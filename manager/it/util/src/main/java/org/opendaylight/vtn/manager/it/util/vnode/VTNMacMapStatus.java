/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.vnode;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import static org.opendaylight.vtn.manager.it.util.ModelDrivenTestBase.getRpcOutput;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.it.util.vnode.mac.MacEntry;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.GetMacMappedHostInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.GetMacMappedHostInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.GetMacMappedHostOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.VtnMacMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.get.mac.mapped.host.output.MacMappedHost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.info.MacMapStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.status.MappedHost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * {@code VTNMacMapStatus} descrbies the status of the MAC mapping.
 */
public final class VTNMacMapStatus implements Cloneable {
    /**
     * A set of mapped hosts indexed by MAC address.
     */
    private Map<EtherAddress, MacEntry>  mappedHosts;

    /**
     * Construct a new instance.
     */
    public VTNMacMapStatus() {
        mappedHosts = new HashMap<>();
    }

    /**
     * Return the number of hosts in this instance.
     *
     * @return  The number of hosts in this instance.
     */
    public int size() {
        return mappedHosts.size();
    }

    /**
     * Add the given hosts to this instance.
     *
     * @param hosts  Host information to be added.
     * @return  This instance.
     */
    public VTNMacMapStatus add(MacEntry ... hosts) {
        for (MacEntry ment: hosts) {
            mappedHosts.put(ment.getMacAddress(), ment);
        }
        return this;
    }

    /**
     * Remove the given MAC addresses from this instance.
     *
     * @param eaddrs  MAC addresses to be removed.
     * @return  This instance.
     */
    public VTNMacMapStatus remove(EtherAddress ... eaddrs) {
        for (EtherAddress mac: eaddrs) {
            mappedHosts.remove(mac);
        }
        return this;
    }

    /**
     * Remove all the host information connected to the specified switch.
     *
     * @param nid  The MD-SAL node identifier.
     * @return  A list of removed host information.
     */
    public List<MacEntry> removeByNode(String nid) {
        List<MacEntry> removed = new ArrayList<>();
        for (Iterator<MacEntry> it = mappedHosts.values().iterator();
             it.hasNext();) {
            MacEntry ment = it.next();
            if (nid.equals(ment.getNodeIdentifier())) {
                removed.add(ment);
                it.remove();
            }
        }

        return removed;
    }

    /**
     * Remove all the host information connected to the specified switch port.
     *
     * @param pid  The MD-SAL port identifier.
     * @return  A list of removed host information.
     */
    public List<MacEntry> removeByPort(String pid) {
        List<MacEntry> removed = new ArrayList<>();
        for (Iterator<MacEntry> it = mappedHosts.values().iterator();
             it.hasNext();) {
            MacEntry ment = it.next();
            if (pid.equals(ment.getPortIdentifier())) {
                removed.add(ment);
                it.remove();
            }
        }

        return removed;
    }

    /**
     * Remove all the host information on the specified VLAN.
     *
     * @param vid  The VLAN ID.
     * @return  A list of removed host information.
     */
    public List<MacEntry> removeByVlan(int vid) {
        List<MacEntry> removed = new ArrayList<>();
        for (Iterator<MacEntry> it = mappedHosts.values().iterator();
             it.hasNext();) {
            MacEntry ment = it.next();
            if (ment.getVlanId() == vid) {
                removed.add(ment);
                it.remove();
            }
        }

        return removed;
    }

    /**
     * Return an unmodifiable collection that contains all the host information
     * in this instance.
     *
     * @return  An unmodifiable collection of host information.
     */
    public Collection<MacEntry> get() {
        return Collections.unmodifiableCollection(mappedHosts.values());
    }

    /**
     * Return the host associated with the given MAC address.
     *
     * @param mac  The MAC address.
     * @return  A {@link MacEntry} instance if found.
     *          {@code null}if not found.
     */
    public MacEntry get(EtherAddress mac) {
        return mappedHosts.get(mac);
    }

    /**
     * Determine whether this instance is empty or not.
     *
     * @return  {@code true} if this instance is empty.
     *          {@code false} otherwise.
     */
    public boolean isEmpty() {
        return mappedHosts.isEmpty();
    }

    /**
     * Return a set of VLANs mapped by the MAC mapping.
     *
     * @param ports  A collection of MD-SAL port identifiers for all the
     *               existing switch ports.
     * @return  A map that indicates a set of VLANs mapped by the MAC mapping.
     *          The returned map associates a set of VLAN IDs with a MD-SAL
     *          switch port identifier.
     */
    public Map<String, Set<Integer>> getMappedVlans(Collection<String> ports) {
        Map<String, Set<Integer>> map = new HashMap<>();
        for (String pid: ports) {
            map.put(pid, null);
        }

        for (MacEntry ment: mappedHosts.values()) {
            String pid = ment.getPortIdentifier();
            Integer vid = ment.getVlanId();
            Set<Integer> vidSet = map.get(pid);
            if (vidSet == null) {
                vidSet = new HashSet<>();
                map.put(pid, vidSet);
            }
            vidSet.add(vid);
        }
        return map;
    }

    /**
     * Verify the MAC mapped hosts using get-mac-mapped-host RPC.
     *
     * @param mmapSrv  vtn-mac-map service.
     * @param vbrId    The identifier for the target vBridge.
     */
    public void checkMappedHost(VtnMacMapService mmapSrv,
                                VBridgeIdentifier vbrId) {
        checkMappedHost(mmapSrv, vbrId, false);
    }

    /**
     * Verify the MAC mapped hosts using get-mac-mapped-host RPC.
     *
     * @param mmapSrv  vtn-mac-map service.
     * @param vbrId    The identifier for the target vBridge.
     * @param empty    If {@code true}, set an empty MAC address list to the
     *                 get-mac-mapped-host input.
     */
    public void checkMappedHost(VtnMacMapService mmapSrv,
                                VBridgeIdentifier vbrId, boolean empty) {
        List<MacAddress> maddrs = (empty)
            ? Collections.<MacAddress>emptyList()
            : null;
        checkMappedHost(mmapSrv, vbrId, maddrs);
    }

    /**
     * Verify the MAC mapped hosts using get-mac-mapped-host RPC.
     *
     * @param mmapSrv  vtn-mac-map service.
     * @param vbrId    The identifier for the target vBridge.
     * @param eaddr    A MAC address to be searched.
     */
    public void checkMappedHost(VtnMacMapService mmapSrv,
                                VBridgeIdentifier vbrId, EtherAddress eaddr) {
        checkMappedHost(mmapSrv, vbrId,
                        Collections.singletonList(eaddr.getMacAddress()));
    }

    /**
     * Verify the MAC mapped hosts using get-mac-mapped-host RPC.
     *
     * @param mmapSrv  vtn-mac-map service.
     * @param vbrId    The identifier for the target vBridge.
     * @param eaddrs   A collection of MAC addresses to be searched.
     */
    public void checkMappedHost(VtnMacMapService mmapSrv,
                                VBridgeIdentifier vbrId,
                                Collection<EtherAddress> eaddrs) {
        List<MacAddress> maddrs;
        if (eaddrs == null) {
            maddrs = null;
        } else {
            maddrs = new ArrayList<>(eaddrs.size());
            for (EtherAddress eaddr: eaddrs) {
                maddrs.add(eaddr.getMacAddress());
            }
        }

        checkMappedHost(mmapSrv, vbrId, maddrs);
    }

    /**
     * Verify the status of the MAC mapping.
     *
     * @param mst  The status of the MAC mapping.
     * @return  A {@link VnodeState} instance that indicates the stauts of the
     *          VLAN mapping.
     */
    public VnodeState verify(MacMapStatus mst) {
        assertNotNull(mst);

        List<MappedHost> mapped = mst.getMappedHost();
        VnodeState state;
        if (mappedHosts.isEmpty()) {
            state = VnodeState.DOWN;
            if (mapped != null) {
                assertEquals(Collections.<MappedHost>emptyList(), mapped);
            }
        } else {
            assertNotNull(mapped);
            state = VnodeState.UP;
            Set<EtherAddress> checked = new HashSet<>();
            for (MappedHost mh: mapped) {
                EtherAddress eaddr = new EtherAddress(mh.getMacAddress());
                MacEntry ment = mappedHosts.get(eaddr);
                assertNotNull(ment);
                assertEquals(ment.getMacAddress(), eaddr);
                assertEquals(ment.getVlanId(),
                             mh.getVlanId().getValue().intValue());
                assertEquals(ment.getPortIdentifier(),
                             mh.getPortId().getValue());
                assertEquals(true, checked.add(eaddr));
            }
            assertEquals(checked, mappedHosts.keySet());
        }

        return state;
    }

    /**
     * Verify the MAC mapped hosts using get-mac-mapped-host RPC.
     *
     * @param mmapSrv  vtn-mac-map service.
     * @param vbrId    The identifier for the target vBridge.
     * @param maddrs   A list of MAC addresses to be set in RPC input.
     */
    public void checkMappedHost(VtnMacMapService mmapSrv,
                                VBridgeIdentifier vbrId,
                                List<MacAddress> maddrs) {
        Map<EtherAddress, MacEntry> expected = getMappedHosts(maddrs);
        GetMacMappedHostInput input = new GetMacMappedHostInputBuilder().
            setTenantName(vbrId.getTenantNameString()).
            setBridgeName(vbrId.getBridgeNameString()).
            setMacAddresses(maddrs).
            build();
        GetMacMappedHostOutput output =
            getRpcOutput(mmapSrv.getMacMappedHost(input));
        assertEquals(Boolean.TRUE, output.isConfigured());
        List<MacMappedHost> hosts = output.getMacMappedHost();
        if (expected.isEmpty()) {
            assertEquals(null, hosts);
        } else {
            Map<EtherAddress, MacEntry> result = new HashMap<>();
            for (MacMappedHost mhost: hosts) {
                MacEntry ment = new MacEntry(mhost);
                EtherAddress eaddr = ment.getMacAddress();
                assertEquals(null, result.put(eaddr, ment));
            }
            assertEquals(expected, result);
        }
    }

    /**
     * Return a map that indicates the expected result of get-mac-mapped-host
     * RPC.
     *
     * @param maddrs  A list of MAC addresses to be set in the input of
     *                get-mac-mapped-host RPC.
     * @return  A map that indicates the expected result of get-mac-mapped-host
     *          RPC.
     */
    private Map<EtherAddress, MacEntry> getMappedHosts(
        List<MacAddress> maddrs) {
        Map<EtherAddress, MacEntry> expected;
        if (maddrs == null || maddrs.isEmpty()) {
            expected = mappedHosts;
        } else {
            Set<MacAddress> mset = new HashSet<>(maddrs);
            expected = new HashMap<>();
            for (MacEntry ment: mappedHosts.values()) {
                EtherAddress eaddr = ment.getMacAddress();
                if (mset.contains(eaddr.getMacAddress())) {
                    expected.put(eaddr, ment);
                }
            }
        }

        return expected;
    }

    // Object

    /**
     * Create a shallow copy of this instance.
     *
     * @return  A shallow copy of this instance.
     */
    @Override
    public VTNMacMapStatus clone() {
        try {
            VTNMacMapStatus vmst = (VTNMacMapStatus)super.clone();
            vmst.mappedHosts = new HashMap<>(mappedHosts);
            return vmst;
        } catch (CloneNotSupportedException e) {
            // This should never happen.
            throw new IllegalStateException("clone() failed", e);
        }
    }
}
