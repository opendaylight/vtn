/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.opendaylight.vtn.manager.internal.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodePathFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

/**
 * JUnit test for {@link VNodeIdentifier}.
 */
public class VNodeIdentifierTest extends TestBase {
    /**
     * A set of VNode identifier classes.
     */
    public static final Set<Class<? extends VNodeIdentifier<?>>> ALL_TYPES;

    /**
     * Initialize static field.
     */
    static {
        Set<Class<? extends VNodeIdentifier<?>>> set = new HashSet<>();
        Collections.addAll(
            set,
            VTenantIdentifier.class,
            VBridgeIdentifier.class,
            VBridgeIfIdentifier.class,
            VTerminalIdentifier.class,
            VTerminalIfIdentifier.class,
            VlanMapIdentifier.class,
            MacMapIdentifier.class,
            MacMapHostIdentifier.class);
        ALL_TYPES = set;
    }

    /**
     * Test case for {@link VNodeIdentifier#create(String)} and
     * {@link VNodeIdentifier#create(String, Class)}.
     */
    @Test
    public void testCreate() {
        assertEquals(null, VNodeIdentifier.create(null));
        try {
            VNodeIdentifier.create(null, VTenantIdentifier.class);
            unexpected();
        } catch (IllegalStateException e) {
            assertEquals("Unexpected VNode identifier type: null",
                         e.getMessage());
        }

        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        VnodeName[] inames = {new VnodeName("if"), new VnodeName("vif_1")};
        String[] mapIds = {"ANY.1", "openflow:3.0", "openflow:12345678.4095"};
        MacVlan[] hosts = {
            new MacVlan(0xa0b0c0d0e0f0L, (short)0),
            new MacVlan(0x001122334455L, (short)1024),
        };

        List<VNodeIdentifier<?>> allIds = new ArrayList<>();
        for (VnodeName tname: tnames) {
            VTenantIdentifier vtnId = new VTenantIdentifier(tname);
            String str = vtnId.toString();
            assertEquals(vtnId, VNodeIdentifier.create(str));
            VTenantIdentifier vtnId1 =
                VNodeIdentifier.create(str, VTenantIdentifier.class);
            assertEquals(vtnId, vtnId1);
            allIds.add(vtnId);

            for (VnodeName bname: bnames) {
                VBridgeIdentifier vbrId = new VBridgeIdentifier(tname, bname);
                str = vbrId.toString();
                assertEquals(vbrId, VNodeIdentifier.create(str));
                VBridgeIdentifier vbrId1 =
                    VNodeIdentifier.create(str, VBridgeIdentifier.class);
                assertEquals(vbrId, vbrId1);
                allIds.add(vbrId);

                VTerminalIdentifier vtermId =
                    new VTerminalIdentifier(tname, bname);
                str = vtermId.toString();
                assertEquals(vtermId, VNodeIdentifier.create(str));
                VTerminalIdentifier vtermId1 =
                    VNodeIdentifier.create(str, VTerminalIdentifier.class);
                assertEquals(vtermId, vtermId1);
                allIds.add(vtermId);

                MacMapIdentifier macId = new MacMapIdentifier(tname, bname);
                str = macId.toString();
                assertEquals(macId, VNodeIdentifier.create(str));
                MacMapIdentifier macId1 =
                    VNodeIdentifier.create(str, MacMapIdentifier.class);
                assertEquals(macId, macId1);
                allIds.add(macId);

                for (MacVlan host: hosts) {
                    MacMapHostIdentifier hostId =
                        new MacMapHostIdentifier(tname, bname, host);
                    str = hostId.toString();
                    assertEquals(hostId, VNodeIdentifier.create(str));
                    MacMapHostIdentifier hostId1 =
                        VNodeIdentifier.create(str, MacMapHostIdentifier.class);
                    assertEquals(hostId, hostId1);
                    allIds.add(hostId);
                }

                for (VnodeName iname: inames) {
                    VBridgeIfIdentifier bifId =
                        new VBridgeIfIdentifier(tname, bname, iname);
                    str = bifId.toString();
                    assertEquals(bifId, VNodeIdentifier.create(str));
                    VBridgeIfIdentifier bifId1 =
                        VNodeIdentifier.create(str, VBridgeIfIdentifier.class);
                    assertEquals(bifId, bifId1);
                    allIds.add(bifId);

                    VTerminalIfIdentifier tifId =
                        new VTerminalIfIdentifier(tname, bname, iname);
                    str = tifId.toString();
                    VTerminalIfIdentifier tifId1 = VNodeIdentifier.
                        create(str, VTerminalIfIdentifier.class);
                    assertEquals(tifId, tifId1);
                    allIds.add(tifId);
                }

                for (String mapId: mapIds) {
                    VlanMapIdentifier vlanId =
                        new VlanMapIdentifier(tname, bname, mapId);
                    str = vlanId.toString();
                    assertEquals(vlanId, VNodeIdentifier.create(str));
                    VlanMapIdentifier vlanId1 =
                        VNodeIdentifier.create(str, VlanMapIdentifier.class);
                    assertEquals(vlanId, vlanId1);
                    allIds.add(vlanId);
                }
            }
        }

        for (VNodeIdentifier<?> ident: allIds) {
            Class<?> type = ident.getClass();
            for (Class<? extends VNodeIdentifier<?>> cls: ALL_TYPES) {
                if (type.equals(cls)) {
                    continue;
                }

                // Type mismatch.
                String str = ident.toString();
                try {
                    VNodeIdentifier.create(str, cls);
                    unexpected();
                } catch (IllegalStateException e) {
                    assertEquals("Unexpected VNode identifier type: " + ident,
                                 e.getMessage());
                }

                // Cast to super class.
                try {
                    VInterfaceIdentifier id = VNodeIdentifier.
                        create(str, VInterfaceIdentifier.class);
                    assertTrue(ident instanceof VInterfaceIdentifier);
                    assertEquals(ident, id);
                } catch (IllegalStateException e) {
                    assertFalse(ident instanceof VInterfaceIdentifier);
                    assertEquals("Unexpected VNode identifier type: " + ident,
                                 e.getMessage());
                }
            }
        }

        // Bad VNode type in string.
        String[] badTypes = {
            "Path-Map", "Flow-Filter", "Port-Map",
        };
        for (String type: badTypes) {
            String str = type + ":vtn_1";
            try {
                VNodeIdentifier.create(str);
                unexpected();
            } catch (IllegalArgumentException e) {
                assertEquals("Invalid identifier format: " + str,
                             e.getMessage());
                Throwable cause = e.getCause();
                assertTrue(cause instanceof IllegalArgumentException);
                assertEquals("Unknown VNode type: " + type,
                             cause.getMessage());
            }
        }

        // Invalid strings.
        String[] badArgs = {
            // No type separator.
            "", "a", "01234", "This is a test string.",

            // Invalid number of path components.
            "VTN:vtn_1/vbr_1",
            "vBridge:vtn_2",
            "vBridge:vtn_2/vbr_1/if_222",
            "vTerminal:vtn_3",
            "vTerminal:vtn_3/vterm_2/if_3",
            "vBridge-IF:vtn_4/vbr_4",
            "vBridge-IF:vtn_4/vbr_4/if_3/portmap",
            "vTerminal-IF:vtn_5",
            "vTerminal-IF:vtn_5/vterm_2/if_14/portmap",
            "VLAN-map:vtn_6/vbr_3",
            "VLAN-map:vtn_6/vbr_3/if_3/portmap",
            "MAC-map:vtn_7",
            "MAC-map:vtn_7/vbr_33/if_4/portmap",
            "MAC-map-host:vtn_8/vbr10",
            "MAC-map-host:vtn_8/vbr10/macmap/allowed/host",
        };
        for (String str: badArgs) {
            try {
                VNodeIdentifier.create(str);
                unexpected();
            } catch (IllegalArgumentException e) {
                assertEquals("Invalid identifier format: " + str,
                             e.getMessage());
                assertEquals(null, e.getCause());
            }
        }

        // Invalid path components.
        String[] badComps = {
            // Invalid VTN name.
            "VTN:",
            "VTN:<null>",
            "vBridge:-tenant-1/vbr_1",
            "vTerminal:01234567890123456789012345678901/vterm",
            "vBridge-IF:/vbr_1/if_1",
            "vTerminal-IF:vtn 1/vterm/if_1",

            // Invalid bridge name.
            "vBridge:vtn/_vbridge_1",
            "vBridge:vtn/<null>",
            "vTerminal:vtn/",
            "vBridge-IF:vtn/01234567890123456789012345678901/if",
            "VLAN-map:vtn//ANY.1",
            "MAC-map:vtn/vtn_(1)",

            // Invalid interface name.
            "vBridge-IF:vtn/vbr/01234567890123456789012345678901",
            "vBridge-IF:vtn/vbr/",
            "vTerminal-IF:vtn/vterm/^if1",
            "vTerminal-IF:vtn/vterm/<null>",

            // No VLAN map ID.
            "VLAN-map:vtn/vbr/<null>",

            // Invalid MAC-mapped host.
            "MAC-map-host:vtn/vbr/mac maped host",
            "MAC-map-host:vtn/vbr/",
            "MAC-map-host:vtn/vbr/<null>",
            "MAC-map-host:vtn/vbr/99999999999999999999999999999999999999999999",
        };
        for (String str: badComps) {
            try {
                VNodeIdentifier.create(str);
                unexpected();
            } catch (IllegalArgumentException e) {
                assertEquals("Invalid identifier format: " + str,
                             e.getMessage());
                Throwable cause = e.getCause();
                assertTrue(cause instanceof IllegalArgumentException);
            }
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#create(VnodePathFields, boolean)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreatePath() throws Exception {
        String[] tnames = {"vtn", "vtn1"};
        String[] bnames = {"bridge", "bridge_1"};
        String[] inames = {"if", "if_1"};
        boolean[] bools = {true, false};

        for (boolean find: bools) {
            VnodePathFields vpath = null;
            assertEquals(null, VNodeIdentifier.create(vpath, find));

            for (String tname: tnames) {
                vpath = mock(VnodePathFields.class);
                when(vpath.getTenantName()).thenReturn(tname);
                VNodeIdentifier ident = VNodeIdentifier.create(vpath, find);
                assertEquals(VTenantIdentifier.class, ident.getClass());
                assertEquals(tname, ident.getTenantNameString());

                for (String bname: bnames) {
                    vpath = mock(VnodePathFields.class);
                    when(vpath.getTenantName()).thenReturn(tname);
                    when(vpath.getBridgeName()).thenReturn(bname);
                    ident = VNodeIdentifier.create(vpath, find);
                    assertEquals(VBridgeIdentifier.class, ident.getClass());
                    assertEquals(tname, ident.getTenantNameString());
                    assertEquals(bname, ident.getBridgeNameString());

                    vpath = mock(VnodePathFields.class);
                    when(vpath.getTenantName()).thenReturn(tname);
                    when(vpath.getTerminalName()).thenReturn(bname);
                    ident = VNodeIdentifier.create(vpath, find);
                    assertEquals(VTerminalIdentifier.class, ident.getClass());
                    assertEquals(tname, ident.getTenantNameString());
                    assertEquals(bname, ident.getBridgeNameString());

                    // bridge-name field should precede terminal-name field.
                    vpath = mock(VnodePathFields.class);
                    when(vpath.getTenantName()).thenReturn(tname);
                    when(vpath.getBridgeName()).thenReturn(bname);
                    when(vpath.getTerminalName()).thenReturn("vterminal");
                    ident = VNodeIdentifier.create(vpath, find);
                    assertEquals(VBridgeIdentifier.class, ident.getClass());
                    assertEquals(tname, ident.getTenantNameString());
                    assertEquals(bname, ident.getBridgeNameString());

                    for (String iname: inames) {
                        vpath = mock(VnodePathFields.class);
                        when(vpath.getTenantName()).thenReturn(tname);
                        when(vpath.getBridgeName()).thenReturn(bname);
                        when(vpath.getInterfaceName()).thenReturn(iname);
                        ident = VNodeIdentifier.create(vpath, find);
                        assertEquals(VBridgeIfIdentifier.class,
                                     ident.getClass());
                        assertEquals(tname, ident.getTenantNameString());
                        assertEquals(bname, ident.getBridgeNameString());
                        assertEquals(iname, ident.getInterfaceNameString());

                        vpath = mock(VnodePathFields.class);
                        when(vpath.getTenantName()).thenReturn(tname);
                        when(vpath.getTerminalName()).thenReturn(bname);
                        when(vpath.getInterfaceName()).thenReturn(iname);
                        ident = VNodeIdentifier.create(vpath, find);
                        assertEquals(VTerminalIfIdentifier.class,
                                     ident.getClass());
                        assertEquals(tname, ident.getTenantNameString());
                        assertEquals(bname, ident.getBridgeNameString());
                        assertEquals(iname, ident.getInterfaceNameString());

                        // bridge-name field should precede terminal-name
                        // field.
                        vpath = mock(VnodePathFields.class);
                        when(vpath.getTenantName()).thenReturn(tname);
                        when(vpath.getBridgeName()).thenReturn(bname);
                        when(vpath.getTerminalName()).thenReturn("vterminal");
                        when(vpath.getInterfaceName()).thenReturn(iname);
                        ident = VNodeIdentifier.create(vpath, find);
                        assertEquals(VBridgeIfIdentifier.class,
                                     ident.getClass());
                        assertEquals(tname, ident.getTenantNameString());
                        assertEquals(bname, ident.getBridgeNameString());
                        assertEquals(iname, ident.getInterfaceNameString());
                    }
                }
            }
        }

        // Virtual bridge name is not specified though VTN and interface name
        // are specified.
        tnames = new String[]{null, "vtn", "vtn1"};
        for (String tname: tnames) {
            VnodePathFields noBridge = mock(VnodePathFields.class);
            when(noBridge.getTenantName()).thenReturn(tname);
            when(noBridge.getInterfaceName()).thenReturn(inames[0]);
            String msg = (tname == null)
                ? "VTN name cannot be null"
                : "Virtual bridge name cannot be null";
            for (boolean find: bools) {
                try {
                    VNodeIdentifier.create(noBridge, find);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
                    assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
                    assertEquals(msg, e.getMessage());
                }
            }
        }

        // Specifying invalid name (read).
        String badName = "bad name";
        Map<VnodePathFields, String> cases = new HashMap<>();
        RpcErrorTag etag = RpcErrorTag.DATA_MISSING;
        VtnErrorTag vtag = VtnErrorTag.NOTFOUND;
        VnodePathFields vtnPath =  mock(VnodePathFields.class);
        when(vtnPath.getTenantName()).thenReturn(badName);
        assertNull(cases.put(vtnPath, badName + ": VTN does not exist."));

        VnodePathFields vbrPath =  mock(VnodePathFields.class);
        when(vbrPath.getTenantName()).thenReturn("vtn");
        when(vbrPath.getBridgeName()).thenReturn(badName);
        assertNull(cases.put(vbrPath, badName + ": vBridge does not exist."));

        VnodePathFields vtmPath =  mock(VnodePathFields.class);
        when(vtmPath.getTenantName()).thenReturn("vtn");
        when(vtmPath.getTerminalName()).thenReturn(badName);
        assertNull(cases.put(vtmPath, badName + ": vTerminal does not exist."));

        VnodePathFields ifPath1 =  mock(VnodePathFields.class);
        when(ifPath1.getTenantName()).thenReturn("vtn");
        when(ifPath1.getBridgeName()).thenReturn("vbr");
        when(ifPath1.getInterfaceName()).thenReturn(badName);
        assertNull(cases.put(ifPath1,
                             badName + ": vInterface does not exist."));

        VnodePathFields ifPath2 =  mock(VnodePathFields.class);
        when(ifPath2.getTenantName()).thenReturn("vtn");
        when(ifPath2.getTerminalName()).thenReturn("vterm");
        when(ifPath2.getInterfaceName()).thenReturn(badName);
        assertNull(cases.put(ifPath2,
                             badName + ": vInterface does not exist."));

        for (Map.Entry<VnodePathFields, String> entry: cases.entrySet()) {
            VnodePathFields vp = entry.getKey();
            try {
                VNodeIdentifier.create(vp, true);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertTrue(e.getCause() instanceof RpcException);
                assertEquals(entry.getValue(), e.getMessage());
            }
        }

        // Specifying invalid VTN name (write).
        etag = RpcErrorTag.BAD_ELEMENT;
        vtag = VtnErrorTag.BADREQUEST;
        cases.clear();
        assertNull(cases.put(vtnPath, "VTN name is invalid"));
        assertNull(cases.put(vbrPath, "vBridge name is invalid"));
        assertNull(cases.put(vtmPath, "vTerminal name is invalid"));
        assertNull(cases.put(ifPath1, "vInterface name is invalid"));
        assertNull(cases.put(ifPath2, "vInterface name is invalid"));

        for (Map.Entry<VnodePathFields, String> entry: cases.entrySet()) {
            VnodePathFields vp = entry.getKey();
            try {
                VNodeIdentifier.create(vp, false);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertTrue(e.getCause() instanceof IllegalArgumentException);
                assertEquals(entry.getValue(), e.getMessage());
            }
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#equals(Object)} and
     * {@link VNodeIdentifier#hashCode()}.
     */
    @Test
    public void testEquals() {
        Set<Object> set = new HashSet<>();
        List<VNodeIdentifier<?>> allIds = new ArrayList<>();

        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        VnodeName[] inames = {new VnodeName("if"), new VnodeName("vif_1")};
        String[] mapIds = {"ANY.1", "openflow:3.0", "openflow:12345678.4095"};
        MacVlan[] hosts = {
            new MacVlan(0xa0b0c0d0e0f0L, (short)0),
            new MacVlan(0x001122334455L, (short)1024),
        };

        for (VnodeName tname: tnames) {
            VnodeName tname1 = copy(tname);
            VTenantIdentifier vtnId = new VTenantIdentifier(tname);
            VTenantIdentifier vtnId1 = new VTenantIdentifier(tname1);
            testEquals(set, vtnId, vtnId1);
            allIds.add(vtnId1);

            VBridgeIdentifier vbrId = new VBridgeIdentifier(tname, null);
            VBridgeIdentifier vbrId1 = new VBridgeIdentifier(tname1, null);
            testEquals(set, vbrId, vbrId1);
            allIds.add(vbrId1);

            for (VnodeName bname: bnames) {
                VnodeName bname1 = copy(bname);
                vbrId = new VBridgeIdentifier(tname, bname);
                vbrId1 = new VBridgeIdentifier(tname1, bname1);
                testEquals(set, vbrId, vbrId1);
                allIds.add(vbrId1);

                VTerminalIdentifier vtermId =
                    new VTerminalIdentifier(tname, bname);
                VTerminalIdentifier vtermId1 =
                    new VTerminalIdentifier(tname1, bname1);
                testEquals(set, vtermId, vtermId1);
                allIds.add(vtermId1);

                MacMapIdentifier macId = new MacMapIdentifier(tname, bname);
                MacMapIdentifier macId1 = new MacMapIdentifier(tname1, bname1);
                testEquals(set, macId, macId1);
                allIds.add(macId1);

                for (MacVlan host: hosts) {
                    MacMapHostIdentifier hostId =
                        new MacMapHostIdentifier(tname, bname, host);
                    MacMapHostIdentifier hostId1 =
                        new MacMapHostIdentifier(tname1, bname1, copy(host));
                    testEquals(set, hostId, hostId1);
                    allIds.add(hostId1);
                }

                for (VnodeName iname: inames) {
                    VnodeName iname1 = copy(iname);
                    VBridgeIfIdentifier bifId =
                        new VBridgeIfIdentifier(tname, bname, iname);
                    VBridgeIfIdentifier bifId1 =
                        new VBridgeIfIdentifier(tname1, bname1, iname1);
                    testEquals(set, bifId, bifId1);
                    allIds.add(bifId1);

                    VTerminalIfIdentifier tifId =
                        new VTerminalIfIdentifier(tname, bname, iname);
                    VTerminalIfIdentifier tifId1 =
                        new VTerminalIfIdentifier(tname1, bname1, iname1);
                    testEquals(set, tifId, tifId1);
                    allIds.add(tifId1);
                }

                for (String mapId: mapIds) {
                    VlanMapIdentifier vlanId =
                        new VlanMapIdentifier(tname, bname, mapId);
                    VlanMapIdentifier vlanId1 =
                        new VlanMapIdentifier(tname1, bname1, copy(mapId));
                    testEquals(set, vlanId, vlanId1);
                    allIds.add(vlanId1);
                }
            }
        }

        assertEquals(allIds.size(), set.size());
        for (VNodeIdentifier<?> ident: allIds) {
            assertEquals(true, set.contains(ident));
            assertEquals(true, set.remove(ident));
            assertEquals(false, set.remove(ident));
        }

        assertTrue(set.isEmpty());
    }
}
