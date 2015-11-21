/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Set;

import org.opendaylight.vtn.manager.internal.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.internal.util.vnode.MacMapHostIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.MacMapIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIfIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTerminalIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTerminalIfIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VlanMapIdentifier;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.junit.Test;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VirtualRouteReason;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePath;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePathBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.common.VirtualRoute;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.common.VirtualRouteBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.BridgeMapInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.BridgeMapInfoBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;

/**
 * JUnit test for {@link VNodeHop}.
 */
public class VNodeHopTest extends TestBase {
    /**
     * Test case for {@link VNodeHop#toVirtualRouteList(List)}.
     */
    @Test
    public void testToVirtualRouteList() {
        List<VNodeHop> vhops = null;
        assertEquals(null, VNodeHop.toVirtualRouteList(vhops));

        vhops = new ArrayList<>();
        assertEquals(null, VNodeHop.toVirtualRouteList(vhops));

        int order = 0;
        List<VirtualRoute> expected = new ArrayList<>();

        for (Entry<VNodeIdentifier<?>, VirtualNodePath> ent:
                 getIdentifiers().entrySet()) {
            VNodeIdentifier<?> ident = ent.getKey();
            VirtualNodePath vpath = ent.getValue();
            VirtualRouteBuilder builder = new VirtualRouteBuilder().
                setVirtualNodePath(vpath);

            for (VirtualRouteReason reason: VirtualRouteReason.values()) {
                vhops.add(new VNodeHop(ident, reason));
                VirtualRoute vr = builder.setReason(reason).setOrder(order).
                    build();
                expected.add(vr);
                order++;
            }
        }

        vhops.add(new VNodeHop());
        expected.add(new VirtualRouteBuilder().setOrder(order).build());
        assertEquals(expected, VNodeHop.toVirtualRouteList(vhops));
    }

    /**
     * Test case for {@link VNodeHop#getPath()}.
     */
    @Test
    public void testGetPath() {
        VNodeHop vhop = new VNodeHop();
        assertEquals(null, vhop.getPath());

        for (VNodeIdentifier<?> ident: getIdentifiers().keySet()) {
            vhop = new VNodeHop(ident, null);
            assertEquals(ident, vhop.getPath());
        }
    }

    /**
     * Test case for {@link VNodeHop#getReason()}.
     */
    @Test
    public void testGetReason() {
        VNodeHop vhop = new VNodeHop();
        assertEquals(null, vhop.getReason());

        for (VirtualRouteReason reason: VirtualRouteReason.values()) {
            vhop = new VNodeHop(null, reason);
            assertEquals(reason, vhop.getReason());
        }
    }

    /**
     * Test case for {@link VNodeHop#toVirtualRouteBuilder()}.
     */
    @Test
    public void testToVirtualRouteBuilder() {
        VNodeHop vhop = new VNodeHop();
        VirtualRoute expected = new VirtualRouteBuilder().build();
        assertEquals(expected, vhop.toVirtualRouteBuilder().build());

        for (Entry<VNodeIdentifier<?>, VirtualNodePath> ent:
                 getIdentifiers().entrySet()) {
            VNodeIdentifier<?> ident = ent.getKey();
            VirtualNodePath vpath = ent.getValue();
            VirtualRouteBuilder builder = new VirtualRouteBuilder().
                setVirtualNodePath(vpath);

            for (VirtualRouteReason reason: VirtualRouteReason.values()) {
                vhop = new VNodeHop(ident, reason);
                expected = builder.setReason(reason).build();
                assertEquals(expected, vhop.toVirtualRouteBuilder().build());
            }
        }
    }

    /**
     * Test case for {@link VNodeHop#equals(Object)} and
     * {@link VNodeHop#hashCode()}.
     */
    @Test
    public void testEquals() {
        Set<Object> set = new HashSet<>();

        Map<VNodeIdentifier<?>, VirtualNodePath> map = getIdentifiers();
        VirtualRouteReason[] reasons = VirtualRouteReason.values();
        for (VNodeIdentifier<?> ident1: map.keySet()) {
            VNodeIdentifier<?> ident2 =
                VNodeIdentifier.create(ident1.toString());
            for (VirtualRouteReason reason: reasons) {
                VNodeHop vhop1 = new VNodeHop(ident1, reason);
                VNodeHop vhop2 = new VNodeHop(ident2, reason);
                testEquals(set, vhop1, vhop2);
            }
        }

        testEquals(set, new VNodeHop(), new VNodeHop());
        assertEquals(map.size() * reasons.length + 1, set.size());
    }

    /**
     * Create {@link VNodeIdentifier} instances for test.
     *
     * @return  A map that contains pairs of {@link VNodeIdentifier} and
     *          {@link VirtualNodePath} instances.
     */
    private Map<VNodeIdentifier<?>, VirtualNodePath> getIdentifiers() {
        Map<VNodeIdentifier<?>, VirtualNodePath> map = new HashMap<>();

        VnodeName[] vtnNames = {
            new VnodeName("vtn_1"), new VnodeName("vtn_2"),
        };
        VnodeName[] brNames = {
            new VnodeName("bridge_1"), new VnodeName("bridge_2"),
        };
        VnodeName[] ifNames = {
            new VnodeName("if_1"), new VnodeName("if_2"),
        };
        String[] vlanMaps = {"ANY.0", "openflow:3.4095"};
        MacVlan[] hosts = {
            new MacVlan(0x001122334455L, 1),
            new MacVlan(0xa0b0c0d0e0f0L, 1234),
        };

        for (VnodeName vtnName: vtnNames) {
            String tname = vtnName.getValue();

            // VTN
            VNodeIdentifier<?> ident = new VTenantIdentifier(vtnName);
            VirtualNodePathBuilder builder = new VirtualNodePathBuilder().
                setTenantName(tname);
            VirtualNodePath vpath = builder.build();
            assertEquals(null, map.put(ident, vpath));

            for (VnodeName brName: brNames) {
                String bname = brName.getValue();

                // vBridge
                builder = new VirtualNodePathBuilder().
                    setTenantName(tname).
                    setBridgeName(bname);
                ident = new VBridgeIdentifier(vtnName, brName);
                vpath = builder.build();
                assertEquals(null, map.put(ident, vpath));

                // MAC mapping
                ident = new MacMapIdentifier(vtnName, brName);
                BridgeMapInfo bmi = new BridgeMapInfoBuilder().
                    setMacMappedHost(-1L).build();
                vpath = builder.addAugmentation(BridgeMapInfo.class, bmi).
                    build();
                assertEquals(null, map.put(ident, vpath));

                for (String mapId: vlanMaps) {
                    // VLAN mapping
                    ident = new VlanMapIdentifier(vtnName, brName, mapId);
                    bmi = new BridgeMapInfoBuilder().
                        setVlanMapId(mapId).build();
                    vpath = builder.addAugmentation(BridgeMapInfo.class, bmi).
                        build();
                    assertEquals(null, map.put(ident, vpath));
                }

                for (MacVlan host: hosts) {
                    // MAC mapped host
                    ident = new MacMapHostIdentifier(vtnName, brName, host);
                    bmi = new BridgeMapInfoBuilder().
                        setMacMappedHost(host.getEncodedValue()).build();
                    vpath = builder.addAugmentation(BridgeMapInfo.class, bmi).
                        build();
                    assertEquals(null, map.put(ident, vpath));
                }

                // vTerminal.
                ident = new VTerminalIdentifier(vtnName, brName);
                builder = new VirtualNodePathBuilder().
                    setTenantName(tname).
                    setTerminalName(bname);
                vpath = builder.build();
                assertEquals(null, map.put(ident, vpath));

                for (VnodeName ifName: ifNames) {
                    String iname = ifName.getValue();

                    // vBridge interface
                    ident = new VBridgeIfIdentifier(vtnName, brName, ifName);
                    vpath = builder.setBridgeName(bname).
                        setTerminalName(null).
                        setInterfaceName(iname).build();
                    assertEquals(null, map.put(ident, vpath));

                    // vTerminal interface
                    ident = new VTerminalIfIdentifier(vtnName, brName, ifName);
                    vpath = builder.setBridgeName(null).
                        setTerminalName(bname).build();
                    assertEquals(null, map.put(ident, vpath));
                }
            }
        }

        return map;
    }
}
