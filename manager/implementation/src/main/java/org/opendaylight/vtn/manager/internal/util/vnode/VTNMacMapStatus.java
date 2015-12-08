/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode;

import static org.opendaylight.vtn.manager.internal.util.ProtocolUtils.MASK_VLAN_ID;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;
import java.util.NavigableMap;
import java.util.Set;
import java.util.TreeMap;

import org.apache.commons.lang3.tuple.Triple;
import org.apache.commons.lang3.tuple.ImmutableTriple;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.PortFilter;
import org.opendaylight.vtn.manager.internal.util.inventory.PortVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.info.MacMapStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.info.MacMapStatusBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.status.MappedHost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.status.MappedHostBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;
import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;

/**
 * {@code VTNMacMapStatus} describes the runtime information about
 * a MAC mapping configured in a vBridge.
 */
public final class VTNMacMapStatus {
    /**
     * A map that keeps switch ports on which MAC address was detected.
     */
    private final Map<PortVlan, Set<MacVlan>>  mappedPorts = new HashMap<>();

    /**
     * A map which keeps active MAC mappings.
     */
    private final NavigableMap<MacVlan, SalPort>  activeMap = new TreeMap<>();

    /**
     * The mac-map-status container in the MD-SAL datastore.
     */
    private MacMapStatus  mdStatus;

    /**
     * {@code true} if this instance is modified.
     */
    private boolean  dirty;

    /**
     * {@code true} if the mac-map-status container needs to be updated.
     */
    private boolean  updateContainer;

    /**
     * Construct an empty instance.
     */
    public VTNMacMapStatus() {
        mdStatus = new MacMapStatusBuilder().build();
    }

    /**
     * Construct a new instance.
     *
     * @param mms  A {@link MacMapStatus} instance.
     *             Specifying {@code null} results in undefined behavior.
     */
    public VTNMacMapStatus(MacMapStatus mms) {
        if (mms == null) {
            // This should never happen.
            mdStatus = new MacMapStatusBuilder().build();
        } else {
            mdStatus = mms;
            List<MappedHost> hosts = mms.getMappedHost();
            if (hosts != null) {
                init(hosts);
            }
        }
    }

    /**
     * Convert this instance into a {@link MacMapStatus} instance.
     *
     * @return  A {@link MacMapStatus} instance.
     */
    public MacMapStatus toMacMapStatus() {
        MacMapStatus mms;
        if (updateContainer) {
            // Update the mac-map-status container.
            mms = new MacMapStatusBuilder().
                setMappedHost(getMappedHost()).
                build();
            mdStatus = mms;
            updateContainer = false;
        } else {
            mms = mdStatus;
        }

        return mms;
    }

    /**
     * Determine whether this instance is modified or not.
     *
     * <p>
     *   Note that this method always clears the dirty flag in this instance.
     * </p>
     *
     * @return  {@code true} if this instance is modified.
     *          {@code false} if not modified.
     */
    public boolean isDirty() {
        boolean ret = dirty;
        dirty = false;
        return ret;
    }

    /**
     * Put the MAC mapping status only if this instance is modified.
     *
     * @param tx     A transaction for the MD-SAL datastore.
     * @param ident  A {@link MacMapIdentifier} instance that specifies the
     *               target MAC mapping.
     */
    public void submit(ReadWriteTransaction tx, MacMapIdentifier ident) {
        if (isDirty()) {
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            tx.put(oper, ident.getStatusPath(), toMacMapStatus(), false);
        }
    }

    /**
     * Activate MAC mapping for the specified MAC address.
     *
     * @param mapId  The identifier for the target MAC mapping.
     * @param mvlan  A {@link MacVlan} instance corresponding to a host to be
     *               activated.
     * @param sport  A {@link SalPort} instance corresponding to a switch port
     *               where the MAC address is detected.
     * @return
     *   <p>
     *     A {@link Triple} instance is returned if the MAC mapping was
     *     successfully activated.
     *   </p>
     *   <ul>
     *     <li>
     *       A boolean value that indicates the result is set to the left.
     *       {@code true} means that the target MAC mapping has been activated.
     *       {@code false} means that the target MAC mapping is already
     *       activated.
     *     </li>
     *     <li>
     *       A {@link SalPort} instance that specifies the physical switch port
     *       previously associated with the L2 host in the MAC mapping is set
     *       to the middle.
     *     </li>
     *     <li>
     *       A {@link PortVlan} instance that specifies the VLAN to be released
     *       is set to the right.
     *     </li>
     *   </ul>
     *   <p>
     *     {@code null} is returned if the MAC mapping for the specified
     *     host is already activated.
     *   </p>
     * @throws MacMapDuplicateException
     *    The specified MAC address is already mapped by this MAC mapping.
     */
    public Triple<Boolean, SalPort, PortVlan> activate(
        MacMapIdentifier mapId, MacVlan mvlan, SalPort sport)
        throws MacMapDuplicateException {
        int vid = mvlan.getVlanId();
        long mac = mvlan.getAddress();
        boolean empty = activeMap.isEmpty();
        Entry<MacVlan, SalPort> entry = getDuplicateEntry(mac);
        SalPort oldPort;
        PortVlan released = null;
        if (entry == null) {
            activeMap.put(mvlan, sport);
            oldPort = null;
        } else {
            MacVlan dup = entry.getKey();
            if (!dup.equals(mvlan)) {
                throw new MacMapDuplicateException(mvlan, mapId, dup);
            }

            oldPort = entry.getValue();
            if (sport.equals(oldPort)) {
                return null;
            }

            // Update the port associated with the specified host.
            activeMap.put(dup, sport);

            // Remove old port information.
            PortVlan pvlan = new PortVlan(oldPort, vid);
            Set<MacVlan> mvSet = mappedPorts.get(pvlan);
            mvSet.remove(mvlan);
            if (mvSet.isEmpty()) {
                mappedPorts.remove(pvlan);
                released = pvlan;
            }
        }

        PortVlan pvlan = new PortVlan(sport, vid);
        Set<MacVlan> mvSet = mappedPorts.get(pvlan);
        if (mvSet == null) {
            mvSet = new HashSet<>();
            mappedPorts.put(pvlan, mvSet);
        }

        mvSet.add(mvlan);
        setDirty();

        return new ImmutableTriple<Boolean, SalPort, PortVlan>(
            empty, oldPort, released);
    }

    /**
     * Inactivate the MAC mapping for the specified MAC address.
     *
     * @param mvlan  A {@link MacVlan} instance corresponding to a host to be
     *               inactivated.
     * @param rels   A set of {@link PortVlan} to store VLAN networks to be
     *               released.
     * @return  A {@link SalPort} instance which was associated with the
     *          specified {@link MacVlan} instance is returned.
     *          {@code null} is returned if no switch port was associated.
     */
    public SalPort inactivate(MacVlan mvlan, Set<PortVlan> rels) {
        SalPort sport = activeMap.remove(mvlan);
        if (sport != null) {
            removeMappedPort(mvlan, sport, rels);
        }

        return sport;
    }

    /**
     * Inactivate all the MAC mappings detected on the specified VLAN.
     *
     * @param pvlan  A {@link PortVlan} instance which specifies a pair of
     *               physical switch port and VLAN ID.
     * @return  A set of {@link MacVlan} instances that indicates inactivated
     *          MAC mappings. {@code null} if no MAC mapping was inactivated.
     */
    public Set<MacVlan> inactivate(PortVlan pvlan) {
        Set<MacVlan> mvSet = mappedPorts.remove(pvlan);
        if (mvSet != null) {
            for (MacVlan mvlan: mvSet) {
                activeMap.remove(mvlan);
            }
            setDirty();
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
     * @throws VTNException  An error occurred.
     */
    public Map<MacVlan, SalPort> inactivate(
        PortFilter filter, Set<PortVlan> rels) throws VTNException {
        Map<MacVlan, SalPort> result = new HashMap<>();
        for (Iterator<Entry<PortVlan, Set<MacVlan>>> it =
                 mappedPorts.entrySet().iterator(); it.hasNext();) {
            Entry<PortVlan, Set<MacVlan>> entry = it.next();
            PortVlan pvlan = entry.getKey();
            SalPort sport = pvlan.getPort();
            if (filter.accept(sport, null)) {
                for (MacVlan mvlan: entry.getValue()) {
                    result.put(mvlan, sport);
                    activeMap.remove(mvlan);
                }
                rels.add(pvlan);
                it.remove();
            }
        }

        if (!result.isEmpty()) {
            setDirty();
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
     * @param mapId     The identifier for the target MAC mapping.
     * @param allowed   A set of hosts to be mapped by MAC mapping explicitly.
     * @param unmapped  A set of VLAN IDs which was unmmaped.
     * @param rels      A set of {@link PortVlan} to store VLAN networks to be
     *                  released.
     * @return  A map which contains pairs of unmapped host and port is
     *          returned.
     */
    public Map<MacVlan, SalPort> inactivate(
        MacMapIdentifier mapId, Set<MacVlan> allowed, Set<Integer> unmapped,
        Set<PortVlan> rels) {
        Map<MacVlan, SalPort> result = new HashMap<>();
        for (Iterator<Entry<MacVlan, SalPort>> it =
                 activeMap.entrySet().iterator(); it.hasNext();) {
            Entry<MacVlan, SalPort> entry = it.next();
            MacVlan mvlan = entry.getKey();
            Integer vid = Integer.valueOf(mvlan.getVlanId());
            if (unmapped.contains(vid) && !allowed.contains(mvlan)) {
                // This host is no longer mapped.
                SalPort sport = entry.getValue();
                result.put(mvlan, sport);
                removeMappedPort(mvlan, sport, rels);
                it.remove();
            }
        }

        return result;
    }

    /**
     * Determine the switch port where the specified host was detected.
     *
     * @param mvlan  A {@link MacVlan} instance which represents the layer 2
     *               host address.
     * @return  A {@link SalPort} instance corresponding to a switch port
     *          associated with the specified host in this MAC mapping.
     *          {@code null} if the MAC mapping for the specified host is not
     *          active.
     */
    public SalPort getPort(MacVlan mvlan) {
        return activeMap.get(mvlan);
    }

    /**
     * Determine the VLAN associated with the specified MAC address in the
     * MAC mapping.
     *
     * @param mac  A long integer which represents MAC address of the host.
     * @return  A {@link PortVlan} instance which represents VLAN associated
     *          with the specified MAC address.
     *          {@code null} if the specified MAC address is not mapped by the
     *          MAC mapping.
     */
    public PortVlan getPortVlan(long mac) {
        Entry<MacVlan, SalPort> entry = getDuplicateEntry(mac);
        PortVlan pvlan;
        if (entry == null) {
            pvlan = null;
        } else {
            MacVlan mvlan = entry.getKey();
            pvlan = new PortVlan(entry.getValue(), mvlan.getVlanId());
        }

        return pvlan;
    }

    /**
     * Determine whether at least one host is mapped on the specified VLAN
     * network or not.
     *
     * @param pvlan  A {@link PortVlan} instance which specifies a pair of
     *               physical switch port and VLAN ID.
     * @return  {@code true} is returned only if at least one host is mapped
     *          to the specified VLAN.
     */
    public boolean hasMapping(PortVlan pvlan) {
        return mappedPorts.containsKey(pvlan);
    }

    /**
     * Determine whether at least one host is actually mapped by this
     * MAC mapping or not.
     *
     * @return  {@code true} if at least one host is actually mapped by
     *          this MAC mapping. Otherwise {@code false}.
     */
    public boolean hasMapping() {
        return !activeMap.isEmpty();
    }

    /**
     * Return an unmodifiable set of {@link PortVlan} instances corresponding
     * to VLAN where MAC mapping is activated.
     *
     * @return  A set of {@link PortVlan} instances.
     *          {@code null} is returned if no MAC mapping is activated.
     */
    public Set<PortVlan> getNetworks() {
        return (mappedPorts.isEmpty())
            ? null
            : Collections.unmodifiableSet(mappedPorts.keySet());
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
    public MacVlan getDuplicate(MacVlan mvlan) {
        long mac = mvlan.getAddress();
        Entry<MacVlan, SalPort> entry = getDuplicateEntry(mac);
        MacVlan dup;
        if (entry == null) {
            dup = null;
        } else {
            dup = entry.getKey();
            if (dup.equals(mvlan)) {
                dup = null;
            }
        }

        return dup;
    }

    /**
     * Initialize internal maps.
     *
     * @param hosts  A list of {@link MappedHost} instances.
     */
    private void init(List<MappedHost> hosts) {
        try {
            for (MappedHost host: hosts) {
                int vid = host.getVlanId().getValue().intValue();
                MacVlan mv = new MacVlan(host.getMacAddress(), vid);
                SalPort sport = SalPort.create(host.getPortId());
                activeMap.put(mv, sport);

                PortVlan pv = new PortVlan(sport, vid);
                Set<MacVlan> mvSet = mappedPorts.get(pv);
                if (mvSet == null) {
                    mvSet = new HashSet<>();
                    mappedPorts.put(pv, mvSet);
                }
                mvSet.add(mv);
            }
        } catch (RuntimeException e) {
            // This should never happen.
            throw new IllegalStateException(
                "Unable to cache mac-map-status: " + e.getMessage(), e);
        }
    }

    /**
     * Turn the dirty flag on.
     */
    private void setDirty() {
        dirty = true;
        updateContainer = true;
    }

    /**
     * Return a list of {@link MappedHost} instances.
     *
     * @return  A list of {@link MappedHost} instances if this instance
     *          contains at least one host information.
     *          {@code null} if this instance does not contain host
     *          information.
     */
    private List<MappedHost> getMappedHost() {
        List<MappedHost> list = new ArrayList<>(activeMap.size());
        for (Entry<MacVlan, SalPort> entry: activeMap.entrySet()) {
            MacVlan mv = entry.getKey();
            MacAddress mac = mv.getMacAddress();
            VlanId vlanId = new VlanId(mv.getVlanId());
            SalPort sport = entry.getValue();
            MappedHost host = new MappedHostBuilder().
                setMacAddress(mac).
                setVlanId(vlanId).
                setPortId(sport.getNodeConnectorId()).
                build();
            list.add(host);
        }

        return (list.isEmpty()) ? null : list;
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
    private Entry<MacVlan, SalPort> getDuplicateEntry(long mac) {
        MacVlan key = new MacVlan(mac, (int)MASK_VLAN_ID);
        Entry<MacVlan, SalPort> entry = activeMap.floorEntry(key);
        if (entry != null) {
            MacVlan floor = entry.getKey();
            if (floor.getAddress() != mac) {
                entry = null;
            }
        }

        return entry;
    }

    /**
     * Remove the specified host from {@link #mappedPorts}.
     *
     * @param mvlan  A {@link MacVlan} instance.
     * @param sport  A {@link SalPort} instance associated with {@code mvlan}.
     * @param rels   A set of {@link PortVlan} to store VLANs to be released.
     */
    private void removeMappedPort(MacVlan mvlan, SalPort sport,
                                  Set<PortVlan> rels) {
        PortVlan pvlan = new PortVlan(sport, mvlan.getVlanId());
        Set<MacVlan> mvSet = mappedPorts.get(pvlan);
        mvSet.remove(mvlan);
        if (mvSet.isEmpty()) {
            mappedPorts.remove(pvlan);
            rels.add(pvlan);
        }
        setDirty();
    }
}
