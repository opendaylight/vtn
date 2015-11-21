/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode;

import java.util.EnumMap;
import java.util.EnumSet;
import java.util.Map;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

/**
 * JUnit test for {@link VNodeType}.
 */
public class VNodeTypeTest extends TestBase {
    /**
     * Test case for {@link VNodeType#forName(String)} and
     * {@link VNodeType#checkedForName(String)}.
     */
    @Test
    public void testForName() {
        assertEquals(VNodeType.VTN, VNodeType.forName("VTN"));
        assertEquals(VNodeType.VTN, VNodeType.checkedForName("VTN"));
        assertEquals(VNodeType.VBRIDGE, VNodeType.forName("vBridge"));
        assertEquals(VNodeType.VBRIDGE, VNodeType.checkedForName("vBridge"));
        assertEquals(VNodeType.VTERMINAL, VNodeType.forName("vTerminal"));
        assertEquals(VNodeType.VTERMINAL,
                     VNodeType.checkedForName("vTerminal"));
        assertEquals(VNodeType.VBRIDGE_IF, VNodeType.forName("vBridge-IF"));
        assertEquals(VNodeType.VBRIDGE_IF,
                     VNodeType.checkedForName("vBridge-IF"));
        assertEquals(VNodeType.VTERMINAL_IF,
                     VNodeType.forName("vTerminal-IF"));
        assertEquals(VNodeType.VTERMINAL_IF,
                     VNodeType.checkedForName("vTerminal-IF"));
        assertEquals(VNodeType.VLANMAP, VNodeType.forName("VLAN-map"));
        assertEquals(VNodeType.VLANMAP, VNodeType.checkedForName("VLAN-map"));
        assertEquals(VNodeType.MACMAP, VNodeType.forName("MAC-map"));
        assertEquals(VNodeType.MACMAP, VNodeType.checkedForName("MAC-map"));
        assertEquals(VNodeType.MACMAP_HOST,
                     VNodeType.forName("MAC-map-host"));
        assertEquals(VNodeType.MACMAP_HOST,
                     VNodeType.checkedForName("MAC-map-host"));

        for (VNodeType type: VNodeType.values()) {
            assertEquals(type, VNodeType.forName(type.toString()));
            assertEquals(type, VNodeType.checkedForName(type.toString()));
        }

        String[] invalid = {
            null, "", "vTN", "vTenant", "vbridge", "vterminal", "vinterface",
        };
        for (String str: invalid) {
            assertEquals(null, VNodeType.forName(str));
            try {
                VNodeType.checkedForName(str);
                unexpected();
            } catch (IllegalArgumentException e) {
                assertEquals("Unknown VNode type: " + str, e.getMessage());
            }
        }
    }

    /**
     * Test case for {@link VNodeType#getDescription()}.
     */
    @Test
    public void testGetDescription() {
        assertEquals("VTN", VNodeType.VTN.getDescription());
        assertEquals("vBridge", VNodeType.VBRIDGE.getDescription());
        assertEquals("vTerminal", VNodeType.VTERMINAL.getDescription());
        assertEquals("vInterface", VNodeType.VBRIDGE_IF.getDescription());
        assertEquals("vInterface", VNodeType.VTERMINAL_IF.getDescription());
        assertEquals("VLAN mapping", VNodeType.VLANMAP.getDescription());
        assertEquals("MAC mapping", VNodeType.MACMAP.getDescription());
        assertEquals("MAC mapped host",
                     VNodeType.MACMAP_HOST.getDescription());
    }

    /**
     * Test case for {@link VNodeType#getComponentSize()}.
     */
    @Test
    public void testGetComponentSize() {
        assertEquals(1, VNodeType.VTN.getComponentSize());
        assertEquals(2, VNodeType.VBRIDGE.getComponentSize());
        assertEquals(2, VNodeType.VTERMINAL.getComponentSize());
        assertEquals(3, VNodeType.VBRIDGE_IF.getComponentSize());
        assertEquals(3, VNodeType.VTERMINAL_IF.getComponentSize());
        assertEquals(3, VNodeType.VLANMAP.getComponentSize());
        assertEquals(2, VNodeType.MACMAP.getComponentSize());
        assertEquals(3, VNodeType.MACMAP_HOST.getComponentSize());
    }

    /**
     * Test case for {@link VNodeType#contains(VNodeType)}.
     */
    @Test
    public void testContains() {
        Map<VNodeType, Set<VNodeType>> expected =
            new EnumMap<>(VNodeType.class);
        expected.put(VNodeType.VTN, EnumSet.allOf(VNodeType.class));
        expected.put(VNodeType.VBRIDGE,
                     EnumSet.of(VNodeType.VBRIDGE, VNodeType.VBRIDGE_IF,
                                VNodeType.VLANMAP, VNodeType.MACMAP,
                                VNodeType.MACMAP_HOST));
        expected.put(VNodeType.VTERMINAL,
                     EnumSet.of(VNodeType.VTERMINAL, VNodeType.VTERMINAL_IF));
        expected.put(VNodeType.VBRIDGE_IF, EnumSet.of(VNodeType.VBRIDGE_IF));
        expected.put(VNodeType.VTERMINAL_IF,
                     EnumSet.of(VNodeType.VTERMINAL_IF));
        expected.put(VNodeType.VLANMAP, EnumSet.of(VNodeType.VLANMAP));
        expected.put(VNodeType.MACMAP,
                     EnumSet.of(VNodeType.MACMAP, VNodeType.MACMAP_HOST));
        expected.put(VNodeType.MACMAP_HOST, EnumSet.of(VNodeType.MACMAP_HOST));

        VNodeType[] types = VNodeType.values();
        for (VNodeType type: types) {
            assertEquals(false, type.contains((VNodeType)null));

            Set<VNodeType> set = expected.get(type);
            for (VNodeType arg: types) {
                assertEquals(set.contains(arg), type.contains(arg));
            }
        }
    }

    /**
     * Test case for {@link VNodeType#checkName(String, boolean)} and
     * {@link VNodeType#checkName(VnodeName)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCheckName() throws Exception {
        String[] names = {
            "0",
            "01",
            "0123456789012345678901234567890",
            "a",
            "ab",
            "abcABC_",
            "abcABC_0123_XXXXXXXXXXXXXXXXXXX",
        };
        String[] invalidNames = {
            "01234567890123456789012345678901",
            "abcABC_0123_XXXXXXXXXXXXXXXXXXXX",
            "_flow_cond",
            "flow-cond",
            "flow%cond",
            "_",
            " ",
            "\u3042",
        };
        boolean[] bools = {true, false};

        for (VNodeType type: VNodeType.values()) {
            // Valid name.
            for (String name: names) {
                VnodeName vname = new VnodeName(name);
                for (boolean find: bools) {
                    assertEquals(vname, type.checkName(name, find));
                }
                assertEquals(name, type.checkName(vname));
            }

            // Null name.
            RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
            VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
            String msg = type.getDescription() + " name cannot be null";
            for (boolean find: bools) {
                try {
                    type.checkName((String)null, find);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(etag, e.getErrorTag());
                    assertEquals(vtag, e.getVtnErrorTag());
                    assertEquals(null, e.getCause());
                    assertEquals(msg, e.getMessage());
                }
            }

            try {
                type.checkName((VnodeName)null);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(null, e.getCause());
                assertEquals(msg, e.getMessage());
            }

            // Empty name.
            msg = type.getDescription() + " name cannot be empty";
            etag = RpcErrorTag.BAD_ELEMENT;
            try {
                type.checkName("", false);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(null, e.getCause());
                assertEquals(msg, e.getMessage());
            }

            try {
                type.checkName("", true);
                unexpected();
            } catch (RpcException e) {
                String m = ": " + type.getDescription() + " does not exist.";
                assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
                assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
                assertTrue(e.getCause() instanceof RpcException);
                assertEquals(m, e.getMessage());
            }

            // Invalid name.
            msg = type.getDescription() + " name is invalid";

            for (String name: invalidNames) {
                try {
                    type.checkName(name, false);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(etag, e.getErrorTag());
                    assertEquals(vtag, e.getVtnErrorTag());
                    assertTrue(e.getCause() instanceof
                               IllegalArgumentException);
                    assertEquals(msg, e.getMessage());
                }

                try {
                    type.checkName(name, true);
                    unexpected();
                } catch (RpcException e) {
                    String m = name + ": " + type.getDescription() +
                        " does not exist.";
                    assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
                    assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
                    assertTrue(e.getCause() instanceof RpcException);
                    assertEquals(m, e.getMessage());
                }
            }
        }
    }

    /**
     * Test case for {@link VNodeType#getNotFoundException(String)} and
     * {@link VNodeType#getNotFoundException(String, Throwable)}.
     */
    @Test
    public void testGetNotFoundException() {
        String[] names = {
            "vnode",
            "vnode_1",
        };
        Throwable[] causes = {
            null,
            new NullPointerException(),
            new IllegalArgumentException(),
            new IllegalStateException(),
        };

        RpcErrorTag etag = RpcErrorTag.DATA_MISSING;
        VtnErrorTag vtag = VtnErrorTag.NOTFOUND;
        for (VNodeType type: VNodeType.values()) {
            for (String name: names) {
                String msg = name + ": " + type.getDescription() +
                    " does not exist.";
                RpcException e = type.getNotFoundException(name);
                assertEquals(etag, e.getErrorTag());
                assertEquals(null, e.getCause());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());

                for (Throwable cause: causes) {
                    e = type.getNotFoundException(name, cause);
                    assertEquals(etag, e.getErrorTag());
                    assertEquals(cause, e.getCause());
                    assertEquals(vtag, e.getVtnErrorTag());
                    assertEquals(msg, e.getMessage());
                }
            }
        }
    }

    /**
     * Test case for {@link VNodeType#getDataExistsException(String)}.
     */
    @Test
    public void testGetDataExistsException() {
        String[] names = {
            "vnode",
            "vnode_1",
        };

        RpcErrorTag etag = RpcErrorTag.DATA_EXISTS;
        VtnErrorTag vtag = VtnErrorTag.CONFLICT;
        for (VNodeType type: VNodeType.values()) {
            for (String name: names) {
                String msg = name + ": " + type.getDescription() +
                    " already exists.";
                RpcException e = type.getDataExistsException(name);
                assertEquals(etag, e.getErrorTag());
                assertEquals(null, e.getCause());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }
    }

    /**
     * Test case for {@link VNodeType#getBridgeType()}.
     */
    @Test
    public void testGetBridgeType() {
        assertEquals(null, VNodeType.VTN.getBridgeType());
        assertEquals(VNodeType.VBRIDGE, VNodeType.VBRIDGE.getBridgeType());
        assertEquals(VNodeType.VTERMINAL, VNodeType.VTERMINAL.getBridgeType());
        assertEquals(VNodeType.VBRIDGE, VNodeType.VBRIDGE_IF.getBridgeType());
        assertEquals(VNodeType.VTERMINAL,
                     VNodeType.VTERMINAL_IF.getBridgeType());
        assertEquals(VNodeType.VBRIDGE, VNodeType.VLANMAP.getBridgeType());
        assertEquals(VNodeType.VBRIDGE, VNodeType.MACMAP.getBridgeType());
        assertEquals(VNodeType.VBRIDGE, VNodeType.MACMAP_HOST.getBridgeType());
    }

    /**
     * Test case for {@link VNodeType#isInterface()}.
     */
    @Test
    public void testIsInterface() {
        assertEquals(false, VNodeType.VTN.isInterface());
        assertEquals(false, VNodeType.VBRIDGE.isInterface());
        assertEquals(false, VNodeType.VTERMINAL.isInterface());
        assertEquals(true, VNodeType.VBRIDGE_IF.isInterface());
        assertEquals(true, VNodeType.VTERMINAL_IF.isInterface());
        assertEquals(false, VNodeType.VLANMAP.isInterface());
        assertEquals(false, VNodeType.MACMAP.isInterface());
        assertEquals(false, VNodeType.MACMAP_HOST.isInterface());
    }

    /**
     * Test case for {@link VNodeType#isMacMap()}.
     */
    @Test
    public void testIsMacMap() {
        assertEquals(false, VNodeType.VTN.isMacMap());
        assertEquals(false, VNodeType.VBRIDGE.isMacMap());
        assertEquals(false, VNodeType.VTERMINAL.isMacMap());
        assertEquals(false, VNodeType.VBRIDGE_IF.isMacMap());
        assertEquals(false, VNodeType.VTERMINAL_IF.isMacMap());
        assertEquals(false, VNodeType.VLANMAP.isMacMap());
        assertEquals(true, VNodeType.MACMAP.isMacMap());
        assertEquals(true, VNodeType.MACMAP_HOST.isMacMap());
    }

    /**
     * Test case for {@link VNodeType#toString()}.
     */
    @Test
    public void testToString() {
        assertEquals("VTN", VNodeType.VTN.toString());
        assertEquals("vBridge", VNodeType.VBRIDGE.toString());
        assertEquals("vTerminal", VNodeType.VTERMINAL.toString());
        assertEquals("vBridge-IF", VNodeType.VBRIDGE_IF.toString());
        assertEquals("vTerminal-IF", VNodeType.VTERMINAL_IF.toString());
        assertEquals("VLAN-map", VNodeType.VLANMAP.toString());
        assertEquals("MAC-map", VNodeType.MACMAP.toString());
        assertEquals("MAC-map-host", VNodeType.MACMAP_HOST.toString());
    }

    /**
     * Test case for {@link VNodeType#newIdentifier(String[])}.
     */
    @Test
    public void testNewIdentifier() {
        VnodeName vtnName = new VnodeName("vtn_1");
        VnodeName vbrName = new VnodeName("vbridge_2");
        VnodeName vtermName = new VnodeName("vTerminal_3");
        VnodeName ifName = new VnodeName("if_4");
        String vlanMapId = "ANY.1";
        MacVlan host = new MacVlan(0x001122334455L, (short)4095);

        VNodeType type = VNodeType.VTN;
        String[] comps = {vtnName.getValue()};
        VNodeIdentifier ident = type.newIdentifier(comps);
        assertTrue(ident instanceof VTenantIdentifier);
        assertEquals(vtnName, ident.getTenantName());
        assertEquals(null, ident.getBridgeName());
        assertEquals(null, ident.getInterfaceName());

        type = VNodeType.VBRIDGE;
        comps = new String[] {vtnName.getValue(), vbrName.getValue()};
        ident = type.newIdentifier(comps);
        assertTrue(ident instanceof VBridgeIdentifier);
        assertEquals(vtnName, ident.getTenantName());
        assertEquals(vbrName, ident.getBridgeName());
        assertEquals(null, ident.getInterfaceName());

        type = VNodeType.MACMAP;
        ident = type.newIdentifier(comps);
        assertTrue(ident instanceof MacMapIdentifier);
        assertEquals(vtnName, ident.getTenantName());
        assertEquals(vbrName, ident.getBridgeName());
        assertEquals(null, ident.getInterfaceName());

        type = VNodeType.VTERMINAL;
        ident = type.newIdentifier(comps);
        assertTrue(ident instanceof VTerminalIdentifier);
        assertEquals(vtnName, ident.getTenantName());
        assertEquals(vbrName, ident.getBridgeName());
        assertEquals(null, ident.getInterfaceName());

        type = VNodeType.VBRIDGE_IF;
        comps = new String[] {
            vtnName.getValue(), vbrName.getValue(), ifName.getValue(),
        };
        ident = type.newIdentifier(comps);
        assertTrue(ident instanceof VBridgeIfIdentifier);
        assertEquals(vtnName, ident.getTenantName());
        assertEquals(vbrName, ident.getBridgeName());
        assertEquals(ifName, ident.getInterfaceName());

        type = VNodeType.VTERMINAL_IF;
        ident = type.newIdentifier(comps);
        assertTrue(ident instanceof VTerminalIfIdentifier);
        assertEquals(vtnName, ident.getTenantName());
        assertEquals(vbrName, ident.getBridgeName());
        assertEquals(ifName, ident.getInterfaceName());

        type = VNodeType.VLANMAP;
        comps = new String[] {
            vtnName.getValue(), vbrName.getValue(), vlanMapId,
        };
        ident = type.newIdentifier(comps);
        assertTrue(ident instanceof VlanMapIdentifier);
        assertEquals(vtnName, ident.getTenantName());
        assertEquals(vbrName, ident.getBridgeName());
        assertEquals(vlanMapId, ((VlanMapIdentifier)ident).getMapId());

        type = VNodeType.MACMAP_HOST;
        comps = new String[] {
            vtnName.getValue(),
            vbrName.getValue(),
            Long.toString(host.getEncodedValue()),
        };
        ident = type.newIdentifier(comps);
        assertTrue(ident instanceof MacMapHostIdentifier);
        assertEquals(vtnName, ident.getTenantName());
        assertEquals(vbrName, ident.getBridgeName());
        assertEquals(host, ((MacMapHostIdentifier)ident).getMappedHost());
    }
}
