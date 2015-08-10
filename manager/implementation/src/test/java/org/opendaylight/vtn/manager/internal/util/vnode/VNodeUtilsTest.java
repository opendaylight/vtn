/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode;

import java.util.List;

import org.junit.Test;

import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.VTerminalIfPath;
import org.opendaylight.vtn.manager.VTerminalPath;

import org.opendaylight.vtn.manager.internal.cluster.MacMapPath;
import org.opendaylight.vtn.manager.internal.cluster.MacMappedHostPath;
import org.opendaylight.vtn.manager.internal.cluster.MacVlan;
import org.opendaylight.vtn.manager.internal.cluster.VlanMapPath;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePath;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePathBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.BridgeMapInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.BridgeMapInfoBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodePathFields;

/**
 * JUnit test for {@link VTenantUtils}.
 */
public class VNodeUtilsTest extends TestBase {
    /**
     * Test case for {@link VNodeUtils#toVirtualNodePath(VNodePath)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToVirtualNodePath() throws Exception {
        assertEquals(null, VNodeUtils.toVirtualNodePath((VNodePath)null));
        Class<BridgeMapInfo> bmClass = BridgeMapInfo.class;

        List<String> bnames = createStrings("vbr");
        List<String> inames = createStrings("vif");
        List<String> mapIds = createStrings("map", false);
        MacVlan[] hosts = {
            null,
            new MacVlan(0L, (short)0),
            new MacVlan(0xffffffffffffL, (short)4095),
            new MacVlan(0x123456789abcL, (short)123),
        };

        for (String tname: createStrings("vtn")) {
            for (String bname: bnames) {
                VBridgePath bpath = new VBridgePath(tname, bname);
                VirtualNodePath vnp = VNodeUtils.toVirtualNodePath(bpath);
                assertEquals(tname, vnp.getTenantName());
                assertEquals(bname, vnp.getBridgeName());
                assertEquals(null, vnp.getTerminalName());
                assertEquals(null, vnp.getInterfaceName());
                assertEquals(null, vnp.getAugmentation(bmClass));
                if (bname != null) {
                    assertEquals(bpath, VNodeUtils.toVNodePath(vnp));
                }

                for (String iname: inames) {
                    VBridgeIfPath path = new VBridgeIfPath(bpath, iname);
                    vnp = VNodeUtils.toVirtualNodePath(path);
                    assertEquals(tname, vnp.getTenantName());
                    assertEquals(bname, vnp.getBridgeName());
                    assertEquals(null, vnp.getTerminalName());
                    assertEquals(iname, vnp.getInterfaceName());
                    assertEquals(null, vnp.getAugmentation(bmClass));
                    if (bname != null && iname != null) {
                        assertEquals(path, VNodeUtils.toVNodePath(vnp));
                    }
                }

                for (String mapId: mapIds) {
                    VlanMapPath path = new VlanMapPath(bpath, mapId);
                    vnp = VNodeUtils.toVirtualNodePath(path);
                    assertEquals(tname, vnp.getTenantName());
                    assertEquals(bname, vnp.getBridgeName());
                    assertEquals(null, vnp.getTerminalName());
                    assertEquals(null, vnp.getInterfaceName());

                    BridgeMapInfo minfo = vnp.getAugmentation(bmClass);
                    assertEquals(mapId, minfo.getVlanMapId());
                    assertEquals(null, minfo.getMacMappedHost());
                    if (bname != null) {
                        assertEquals(path, VNodeUtils.toVNodePath(vnp));
                    }
                }

                for (MacVlan mv: hosts) {
                    MacMappedHostPath path = new MacMappedHostPath(bpath, mv);
                    vnp = VNodeUtils.toVirtualNodePath(path);
                    assertEquals(tname, vnp.getTenantName());
                    assertEquals(bname, vnp.getBridgeName());
                    assertEquals(null, vnp.getTerminalName());
                    assertEquals(null, vnp.getInterfaceName());

                    BridgeMapInfo minfo = vnp.getAugmentation(bmClass);
                    assertEquals(null, minfo.getVlanMapId());
                    long l = (mv == null) ? -1L : mv.getEncodedValue();
                    assertEquals(Long.valueOf(l), minfo.getMacMappedHost());
                    if (bname != null) {
                        MacMapPath expected = (mv == null)
                            ? new MacMapPath(bpath) : path;
                        assertEquals(expected, VNodeUtils.toVNodePath(vnp));
                    }
                }

                MacMapPath mpath = new MacMapPath(bpath);
                vnp = VNodeUtils.toVirtualNodePath(mpath);
                assertEquals(tname, vnp.getTenantName());
                assertEquals(bname, vnp.getBridgeName());
                assertEquals(null, vnp.getTerminalName());
                assertEquals(null, vnp.getInterfaceName());

                BridgeMapInfo minfo = vnp.getAugmentation(bmClass);
                assertEquals(null, minfo.getVlanMapId());
                assertEquals(Long.valueOf(-1L), minfo.getMacMappedHost());
                if (bname != null) {
                    assertEquals(mpath, VNodeUtils.toVNodePath(vnp));
                }
            }

            for (String vtname: bnames) {
                VTerminalPath vtpath = new VTerminalPath(tname, vtname);
                VirtualNodePath vnp = VNodeUtils.toVirtualNodePath(vtpath);
                assertEquals(tname, vnp.getTenantName());
                assertEquals(null, vnp.getBridgeName());
                assertEquals(vtname, vnp.getTerminalName());
                assertEquals(null, vnp.getInterfaceName());
                assertEquals(null, vnp.getAugmentation(bmClass));
                if (vtname != null) {
                    assertEquals(vtpath, VNodeUtils.toVNodePath(vnp));
                }

                for (String iname: inames) {
                    VTerminalIfPath path = new VTerminalIfPath(vtpath, iname);
                    vnp = VNodeUtils.toVirtualNodePath(path);
                    assertEquals(tname, vnp.getTenantName());
                    assertEquals(null, vnp.getBridgeName());
                    assertEquals(vtname, vnp.getTerminalName());
                    assertEquals(iname, vnp.getInterfaceName());
                    assertEquals(null, vnp.getAugmentation(bmClass));
                    if (vtname != null && iname != null) {
                        assertEquals(path, VNodeUtils.toVNodePath(vnp));
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link VNodeUtils#toVNodePath(VnodePathFields)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToVNodePath1() throws Exception {
        assertEquals(null, VNodeUtils.toVNodePath((VnodePathFields)null));
        Class<BridgeMapInfo> bmClass = BridgeMapInfo.class;

        List<String> bnames = createStrings("vbr", false);
        List<String> inames = createStrings("vif", false);
        List<String> mapIds = createStrings("map", false);
        Long[] hosts = {
            -1L,
            Long.MIN_VALUE,
            0L,
            0xfffff010L,
            0xfffffffffffffffL,
        };

        for (String tname: createStrings("vtn")) {
            VirtualNodePathBuilder builder = new VirtualNodePathBuilder().
                setTenantName(tname);
            for (String bname: bnames) {
                VnodePathFields vpf = builder.setBridgeName(bname).
                    setInterfaceName(null).
                    setTerminalName(null).
                    removeAugmentation(bmClass).
                    build();
                VBridgePath bpath = new VBridgePath(tname, bname);
                assertEquals(bpath, VNodeUtils.toVNodePath(vpf));

                for (String mapId: mapIds) {
                    BridgeMapInfo minfo = new BridgeMapInfoBuilder().
                        setVlanMapId(mapId).build();
                    vpf = builder.addAugmentation(bmClass, minfo).build();
                    assertEquals(bpath, VNodeUtils.toVNodePath(vpf));
                }

                for (Long host: hosts) {
                    BridgeMapInfo minfo = new BridgeMapInfoBuilder().
                        setMacMappedHost(host).build();
                    vpf = builder.addAugmentation(bmClass, minfo).build();
                    assertEquals(bpath, VNodeUtils.toVNodePath(vpf));
                }

                for (String iname: inames) {
                    vpf = builder.setInterfaceName(iname).build();
                    assertEquals(new VBridgeIfPath(bpath, iname),
                                 VNodeUtils.toVNodePath(vpf));
                }

                vpf = builder.setBridgeName(null).
                    setTerminalName(bname).
                    setInterfaceName(null).
                    build();
                VTerminalPath vtpath = new VTerminalPath(tname, bname);
                assertEquals(vtpath, VNodeUtils.toVNodePath(vpf));

                for (String iname: inames) {
                    vpf = builder.setInterfaceName(iname).build();
                    assertEquals(new VTerminalIfPath(vtpath, iname),
                                 VNodeUtils.toVNodePath(vpf));
                }
            }

            VnodePathFields[] bad = {
                builder.setTerminalName(null).build(),
                builder.setInterfaceName(null).build(),
            };
            for (VnodePathFields path: bad) {
                try {
                    VNodeUtils.toVNodePath(path);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                    Status st = e.getStatus();
                    assertEquals(StatusCode.BADREQUEST, st.getCode());
                    assertEquals("Unexpected virtual node path: " + path,
                                 st.getDescription());
                }
            }
        }
    }

    /**
     * Test case for {@link VNodeUtils#toVNodePath(VirtualNodePath)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToVNodePath2() throws Exception {
        assertEquals(null, VNodeUtils.toVNodePath((VirtualNodePath)null));
        Class<BridgeMapInfo> bmClass = BridgeMapInfo.class;

        List<String> bnames = createStrings("vbr", false);
        List<String> inames = createStrings("vif", false);
        List<String> mapIds = createStrings("map", false);
        Long[] hosts = {
            -1L,
            Long.MIN_VALUE,
            0L,
            0xfffff010L,
            0xfffffffffffffffL,
        };

        for (String tname: createStrings("vtn")) {
            VirtualNodePathBuilder builder = new VirtualNodePathBuilder().
                setTenantName(tname);
            for (String bname: bnames) {
                VirtualNodePath vnp = builder.setBridgeName(bname).
                    setInterfaceName(null).
                    setTerminalName(null).
                    removeAugmentation(bmClass).
                    build();
                VBridgePath bpath = new VBridgePath(tname, bname);
                assertEquals(bpath, VNodeUtils.toVNodePath(vnp));

                for (String mapId: mapIds) {
                    BridgeMapInfo minfo = new BridgeMapInfoBuilder().
                        setVlanMapId(mapId).build();
                    vnp = builder.addAugmentation(bmClass, minfo).build();
                    assertEquals(new VlanMapPath(bpath, mapId),
                                 VNodeUtils.toVNodePath(vnp));
                }

                for (Long host: hosts) {
                    BridgeMapInfo minfo = new BridgeMapInfoBuilder().
                        setMacMappedHost(host).build();
                    vnp = builder.addAugmentation(bmClass, minfo).build();
                    Long l = host.longValue();
                    MacMapPath expected = (l >= 0)
                        ? new MacMappedHostPath(bpath, new MacVlan(l))
                        : new MacMapPath(bpath);
                    assertEquals(expected, VNodeUtils.toVNodePath(vnp));
                }

                for (String iname: inames) {
                    vnp = builder.setInterfaceName(iname).build();
                    assertEquals(new VBridgeIfPath(bpath, iname),
                                 VNodeUtils.toVNodePath(vnp));
                }

                vnp = builder.setBridgeName(null).
                    setTerminalName(bname).
                    setInterfaceName(null).
                    build();
                VTerminalPath vtpath = new VTerminalPath(tname, bname);
                assertEquals(vtpath, VNodeUtils.toVNodePath(vnp));

                for (String iname: inames) {
                    vnp = builder.setInterfaceName(iname).build();
                    assertEquals(new VTerminalIfPath(vtpath, iname),
                                 VNodeUtils.toVNodePath(vnp));
                }
            }

            VirtualNodePath[] bad = {
                builder.setTerminalName(null).build(),
                builder.setInterfaceName(null).build(),
            };
            for (VirtualNodePath path: bad) {
                try {
                    VNodeUtils.toVNodePath(path);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                    Status st = e.getStatus();
                    assertEquals(StatusCode.BADREQUEST, st.getCode());
                    assertEquals("Unexpected virtual node path: " + path,
                                 st.getDescription());
                }
            }
        }
    }
}
