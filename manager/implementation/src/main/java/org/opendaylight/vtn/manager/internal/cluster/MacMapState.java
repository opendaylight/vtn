/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;
import java.util.NavigableMap;
import java.util.Set;
import java.util.TreeMap;

import org.opendaylight.vtn.manager.internal.MacMapActivation;
import org.opendaylight.vtn.manager.internal.MacMapDuplicateException;
import org.opendaylight.vtn.manager.internal.PortFilter;
import org.opendaylight.vtn.manager.internal.util.ProtocolUtils;

import org.opendaylight.controller.sal.core.NodeConnector;

/**
 * {@code MacMapState} class keeps runtime state of the MAC mapping.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public class MacMapState implements Serializable, Cloneable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -3834019576239068711L;

    /**
     * A map that keeps switch ports on which MAC addresses detected.
     */
    private transient Map<PortVlan, Set<MacVlan>>  mappedPorts =
        new HashMap<PortVlan, Set<MacVlan>>();

    /**
     * A map which keeps active MAC mappings.
     */
    private transient NavigableMap<MacVlan, NodeConnector> activeMap =
        new TreeMap<MacVlan, NodeConnector>();

    /**
     * Dirty flag.
     *
     * <p>
     *   Dirty flag is set when the contents of this instance is changed.
     *   This value never affects identity of the object.
     * </p>
     */
    private transient boolean  dirty = false;

    /**
     * Activate MAC mapping for the specified MAC address.
     *
     * @param ref    A reference to the target MAC mapping.
     * @param mvlan  A {@link MacVlan} instance corresponding to a host to be
     *               activated.
     * @param port   A {@link NodeConnector} instance corresponding to a
     *               switch port where the MAC address is detected.
     * @return  A {@link MacMapActivation} instance is returned if the
     *          MAC mapping was successfully activated.
     *          {@code null} is returned if the MAC mapping for the specified
     *          host is already activated.
     * @throws MacMapDuplicateException
     *    The specified MAC address is already mapped by this MAC mapping.
     */
    public synchronized MacMapActivation activate(MapReference ref,
                                                  MacVlan mvlan,
                                                  NodeConnector port)
        throws MacMapDuplicateException {
        short vlan = mvlan.getVlan();
        long mac = mvlan.getMacAddress();
        boolean empty = activeMap.isEmpty();
        Entry<MacVlan, NodeConnector> entry = getDuplicateEntry(mac);
        NodeConnector oldPort;
        PortVlan released = null;
        if (entry == null) {
            activeMap.put(mvlan, port);
            oldPort = null;
        } else {
            MacVlan dup = entry.getKey();
            if (!dup.equals(mvlan)) {
                throw new MacMapDuplicateException(mvlan, ref, dup);
            }

            oldPort = entry.getValue();
            if (port.equals(oldPort)) {
                return null;
            }

            // Update the port associated with the specified host.
            activeMap.put(dup, port);

            // Remove old port information.
            PortVlan pvlan = new PortVlan(oldPort, vlan);
            Set<MacVlan> mvSet = mappedPorts.get(pvlan);
            mvSet.remove(mvlan);
            if (mvSet.isEmpty()) {
                mappedPorts.remove(pvlan);
                released = pvlan;
            }
        }

        PortVlan pvlan = new PortVlan(port, vlan);
        Set<MacVlan> mvSet = mappedPorts.get(pvlan);
        if (mvSet == null) {
            mvSet = new HashSet<MacVlan>();
            mappedPorts.put(pvlan, mvSet);
        }

        mvSet.add(mvlan);
        dirty = true;

        return new MacMapActivation(oldPort, released, empty);
    }

    /**
     * Inactivate the MAC mapping for the specified MAC address.
     *
     * @param mvlan  A {@link MacVlan} instance corresponding to a host to be
     *               inactivated.
     * @param rels   A set of {@link PortVlan} to store VLAN networks to be
     *               released.
     * @return  A {@link NodeConnector} instance which was associated with
     *          the specified {@link MacVlan} instance is returned.
     *          {@code null} is returned if no switch port was associated.
     */
    public synchronized NodeConnector inactivate(MacVlan mvlan,
                                                 Set<PortVlan> rels) {
        NodeConnector port = activeMap.remove(mvlan);
        if (port != null) {
            removeMappedPort(mvlan, port, rels);
        }

        return port;
    }

    /**
     * Inactivate all MAC mappings associated with the specified VLAN network
     * on the switch port.
     *
     * @param pvlan  A {@link PortVlan} instance corresponding to a
     *               VLAN network on a switch port.
     * @return  A set of {@link MacVlan} instances which was associated with
     *          the specified VLAN network.
     *          {@code null} is returned if no MAC mapping was assocaited.
     */
    public synchronized Set<MacVlan> inactivate(PortVlan pvlan) {
        Set<MacVlan> mvSet = mappedPorts.remove(pvlan);
        if (mvSet != null) {
            for (MacVlan mvlan: mvSet) {
                activeMap.remove(mvlan);
            }
            dirty = true;
        }

        return mvSet;
    }

    /**
     * Inactivate all MAC mappings detected on switch port accepted by the
     * specified port filter.
     *
     * @param filter  A {@link PortFilter} instance which selects switch ports.
     * @param rels    A set of {@link PortVlan} to store VLAN networks to be
     *                released.
     * @return  A map which contains pairs of unmapped host and port is
     *          returned.
     */
    public synchronized Map<MacVlan, NodeConnector> inactivate(
        PortFilter filter, Set<PortVlan> rels) {
        Map<MacVlan, NodeConnector> result =
            new HashMap<MacVlan, NodeConnector>();
        for (Iterator<Entry<PortVlan, Set<MacVlan>>> it =
                 mappedPorts.entrySet().iterator(); it.hasNext();) {
            Entry<PortVlan, Set<MacVlan>> entry = it.next();
            PortVlan pvlan = entry.getKey();
            NodeConnector nc = pvlan.getNodeConnector();
            if (filter.accept(nc, null)) {
                for (MacVlan mvlan: entry.getValue()) {
                    result.put(mvlan, nc);
                    activeMap.remove(mvlan);
                }
                rels.add(pvlan);
                it.remove();
            }
        }

        if (!result.isEmpty()) {
            dirty = true;
        }

        return result;
    }

    /**
     * Inactivate all MAC mappings which are no longer mapped.
     *
     * <p>
     *   This method is used to inactivate MAC mappings which was mapped by
     *   an allowed host entry with a wildcard MAC address.
     * </p>
     *
     * @param allowed   A map which keeps hosts to be mapped by MAC mapping
     *                  explicitly.
     * @param ref       A reference to the MAC mapping.
     * @param unmapped  A set of VLAN IDs which was unmmaped.
     * @param rels      A set of {@link PortVlan} to store VLAN networks to be
     *                  released.
     * @return  A map which contains pairs of unmapped host and port is
     *          returned.
     */
    public synchronized Map<MacVlan, NodeConnector> inactivate(
        Map<MacVlan, MapReference> allowed, MapReference ref,
        Set<Short> unmapped, Set<PortVlan> rels) {
        Map<MacVlan, NodeConnector> result =
            new HashMap<MacVlan, NodeConnector>();
        for (Iterator<Entry<MacVlan, NodeConnector>> it =
                 activeMap.entrySet().iterator(); it.hasNext();) {
            Entry<MacVlan, NodeConnector> entry = it.next();
            MacVlan mvlan = entry.getKey();
            Short vid = Short.valueOf(mvlan.getVlan());
            if (!unmapped.contains(vid)) {
                continue;
            }

            MapReference mref = allowed.get(mvlan);
            if (!ref.equals(mref)) {
                // This host is no longer mapped.
                NodeConnector port = entry.getValue();
                result.put(mvlan, port);
                removeMappedPort(mvlan, port, rels);
                it.remove();
            }
        }

        return result;
    }

    /**
     * Determine a switch port where the specified host was detected.
     *
     * @param mvlan  A {@link MacVlan} instance which represents the layer 2
     *               host address.
     * @return  A {@link NodeConnector } instance corresponding to a switch
     *          port associated with the specified host in this MAC mapping
     *          is returned. {@code null} is returned if the MAC mapping for
     *          the specified host is not active.
     */
    public synchronized NodeConnector getPort(MacVlan mvlan) {
        return activeMap.get(mvlan);
    }

    /**
     * Determine a VLAN network associated with the specified MAC address
     * in the MAC mapping.
     *
     * @param mac  A long integer which represents MAC address of the host.
     * @return  A {@link PortVlan} instance which represents VLAN network
     *          associated with the specified MAC address is returned.
     *          {@code null} is returned if the specified MAC address is not
     *          mapped by the MAC mapping.
     */
    public synchronized PortVlan getPortVlan(long mac) {
        Entry<MacVlan, NodeConnector> entry = getDuplicateEntry(mac);
        if (entry == null) {
            return null;
        }

        MacVlan mvlan = entry.getKey();
        NodeConnector port = entry.getValue();
        return new PortVlan(port, mvlan.getVlan());
    }

    /**
     * Determine whether at least one host is mapped on the specified VLAN
     * network or not.
     *
     * @param pvlan  A {@link NodeConnector} instance corresponding to a VLAN
     *               network on a switch port.
     * @return  {@code true} is returned only if at least one host is mapped
     *          to the specified VLAN network.
     */
    public synchronized boolean hasMapping(PortVlan pvlan) {
        return mappedPorts.containsKey(pvlan);
    }

    /**
     * Determine whether at least one host is actually mapped by this
     * MAC mapping or not.
     *
     * @return  {@code true} is returned if at least one host is actually
     *          mapped by the MAC mapping. Otherwise {@code false} is returned.
     */
    public synchronized boolean hasMapping() {
        return !activeMap.isEmpty();
    }

    /**
     * Return a set of {@link PortVlan} instances corresponding to VLAN network
     * where MAC mapping is activated.
     *
     * @return  A set of {@link PortVlan} instances.
     *          {@code null} is returned if no MAC mapping is activated.
     */
    public synchronized Set<PortVlan> getNetworks() {
        if (mappedPorts.isEmpty()) {
            return null;
        }

        return new HashSet<PortVlan>(mappedPorts.keySet());
    }

    /**
     * Return a map which keeps information about the mapped hosts is returned.
     *
     * @return
     *    If at least one host is actually mapped by the MAC mapping. a map
     *    which contains information about hosts actually mapped by the
     *    MAC mapping is returned.
     *    The key is a {@link MacVlan} instance which indicates the L2 host,
     *    and the value is a {@link NodeConnector} instance corresponding to
     *    a switch port to which the host is connected.
     *    <p>
     *      {@code null} is returned if no host is mapped by the MAC mapping.
     *    </p>
     */
    public synchronized Map<MacVlan, NodeConnector> getActiveHosts() {
        if (activeMap.isEmpty()) {
            return null;
        }

        return new HashMap<MacVlan, NodeConnector>(activeMap);
    }

    /**
     * Determine whether the MAC address same as the specified host is already
     * mapped by the MAC mapping or not.
     *
     * @param mvlan  A {@link MacVlan} instance to be tested.
     * @return  A {@link MacVlan} instance is returned if another host which
     *          has the same MAC address as {@code mvlan} is already mapped.
     *          Otherwise {@code null} is returned.
     */
    public synchronized MacVlan getDuplicate(MacVlan mvlan) {
        long mac = mvlan.getMacAddress();
        Entry<MacVlan, NodeConnector> entry = getDuplicateEntry(mac);
        if (entry == null) {
            return null;
        }

        MacVlan dup = entry.getKey();
        return (dup.equals(mvlan)) ? null : dup;
    }

    /**
     * Test and clear dirty flag.
     *
     * @return  {@code true} is returned only if this object is dirty.
     */
    public synchronized boolean isDirty() {
        boolean ret = dirty;
        dirty = false;
        return ret;
    }

    /**
     * Return an entry in {@link #activeMap} which keeps the specified
     * MAC address.
     *
     * <p>
     *   Note that an {@link Entry} instance returned by this method is
     *   immutable.
     * </p>
     *
     * @param mac  A long integer which represents the MAC address.
     * @return  A {@link Entry} in {@link #activeMap} if found.
     *          {@code null} if not found.
     */
    private Entry<MacVlan, NodeConnector> getDuplicateEntry(long mac) {
        MacVlan key = new MacVlan(mac, (short)ProtocolUtils.MASK_VLAN_ID);
        Entry<MacVlan, NodeConnector> entry = activeMap.floorEntry(key);
        if (entry != null) {
            MacVlan floor = entry.getKey();
            if (floor.getMacAddress() == mac) {
                return entry;
            }
        }

        return null;
    }

    /**
     * Create a copy of {@link #mappedPorts}.
     *
     * @return  A copy of {@link #mappedPorts}.
     */
    private synchronized Map<PortVlan, Set<MacVlan>> copyMappedPorts() {
        Map<PortVlan, Set<MacVlan>> copy = new HashMap<>();
        for (Entry<PortVlan, Set<MacVlan>> entry: mappedPorts.entrySet()) {
            PortVlan pvlan = entry.getKey();
            Set<MacVlan> mvSet = new HashSet<MacVlan>(entry.getValue());
            copy.put(pvlan, mvSet);
        }

        return copy;
    }

    /**
     * Remove the specified host from {@link #mappedPorts}.
     *
     * @param mvlan  A {@link MacVlan} instance.
     * @param port   A {@link NodeConnector} instance associated with
     *               {@code mvlan}.
     * @param rels   A set of {@link PortVlan} to store VLAN networks to be
     *               released.
     */
    private synchronized void removeMappedPort(MacVlan mvlan,
                                               NodeConnector port,
                                               Set<PortVlan> rels) {
        PortVlan pvlan = new PortVlan(port, mvlan.getVlan());
        Set<MacVlan> mvSet = mappedPorts.get(pvlan);
        mvSet.remove(mvlan);
        if (mvSet.isEmpty()) {
            mappedPorts.remove(pvlan);
            rels.add(pvlan);
        }
        dirty = true;
    }

    /**
     * Read data from the given input stream and deserialize.
     *
     * @param in  An input stream.
     * @throws IOException
     *    An I/O error occurred.
     * @throws ClassNotFoundException
     *    At least one necessary class was not found.
     */
    @SuppressWarnings("unused")
    private void readObject(ObjectInputStream in)
        throws IOException, ClassNotFoundException {
        // Read field information.
        in.readFields();

        mappedPorts = new HashMap<PortVlan, Set<MacVlan>>();
        activeMap = new TreeMap<MacVlan, NodeConnector>();

        // Read the number of keys in mappedPorts.
        int size = in.readInt();

        for (int i = 0; i < size; i++) {
            // Read a entry for mappedPorts.
            PortVlan pvlan = (PortVlan)in.readObject();
            Set<MacVlan> mvSet = (Set<MacVlan>)in.readObject();
            mappedPorts.put(pvlan, mvSet);

            // Set entries for activeMap.
            for (MacVlan mvlan: mvSet) {
                activeMap.put(mvlan, pvlan.getNodeConnector());
            }
        }
    }

    /**
     * Serialize this object and write it to the given output stream.
     *
     * @param out  An output stream.
     * @throws IOException
     *    An I/O error occurred.
     */
    @SuppressWarnings("unused")
    private synchronized void writeObject(ObjectOutputStream out)
        throws IOException {
        // Write field information.
        out.putFields();
        out.writeFields();

        // Write the number of keys in mappedPorts.
        out.writeInt(mappedPorts.size());

        // Write contents of mappedPorts.
        for (Entry<PortVlan, Set<MacVlan>> entry: mappedPorts.entrySet()) {
            out.writeObject(entry.getKey());
            out.writeObject(entry.getValue());
        }
    }

    /**
     * Returns a shallow copy of this instance.
     *
     * <p>
     *   {@link MacVlan} and {@link NodeConnector} instances configured in
     *   this instance are not cloned.
     * </p>
     *
     * @return  A shallow copy of this instance.
     */
    @Override
    public synchronized MacMapState clone() {
        try {
            MacMapState mst = (MacMapState)super.clone();
            Map<PortVlan, Set<MacVlan>> mapped = new HashMap<>();
            NavigableMap<MacVlan, NodeConnector> active = new TreeMap<>();

            for (Entry<PortVlan, Set<MacVlan>> entry: mappedPorts.entrySet()) {
                PortVlan pvlan = entry.getKey();
                Set<MacVlan> mvSet = new HashSet<MacVlan>(entry.getValue());
                mapped.put(pvlan, mvSet);

                for (MacVlan mvlan: mvSet) {
                    active.put(mvlan, pvlan.getNodeConnector());
                }
            }

            mst.mappedPorts = mapped;
            mst.activeMap = active;
            return mst;
        } catch (CloneNotSupportedException e) {
            // This should never happen.
            throw new IllegalStateException("clone() failed", e);
        }
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (!(o instanceof MacMapState)) {
            return false;
        }

        MacMapState mst = (MacMapState)o;

        // Compare with copy of mappedPorts in order to avoid deadlock.
        // Note that we don't need to compare activeMap.
        Map<PortVlan, Set<MacVlan>> mapped = copyMappedPorts();
        Map<PortVlan, Set<MacVlan>> otherMapped = mst.copyMappedPorts();
        return mapped.equals(otherMapped);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public synchronized int hashCode() {
        return mappedPorts.hashCode();
    }
}
