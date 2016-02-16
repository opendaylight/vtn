/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import javax.xml.bind.Marshaller;
import javax.xml.bind.Unmarshaller;

import com.google.common.base.Optional;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlNode;
import org.opendaylight.vtn.manager.internal.XmlDataType;
import org.opendaylight.vtn.manager.internal.XmlValueType;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilterKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.result.FlowFilterResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.result.FlowFilterResultBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.redirect.filter.config.RedirectDestination;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePath;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePathBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.BridgeMapInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.BridgeMapInfoBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.Vtns;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.VinterfaceStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.VinterfaceBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.VinterfaceKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceInputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceOutputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.VterminalKey;

/**
 * JUnit test for {@link VTerminalIfIdentifier}.
 */
public class VTerminalIfIdentifierTest extends TestBase {
    /**
     * Root XML element name associated with {@link VTerminalIfIdentifier}
     * class.
     */
    private static final String  XML_ROOT = "vterminal-if-path";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link VTerminalIfIdentifier} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        ArrayList<XmlDataType> dlist = new ArrayList<>();
        Collections.addAll(
            dlist,
            new XmlValueType("tenant-name", VnodeName.class).add(name).
            prepend(parent),
            new XmlValueType("bridge-name", VnodeName.class).add(name).
            prepend(parent),
            new XmlValueType("interface-name", VnodeName.class).add(name).
            prepend(parent));
        return dlist;
    }

    /**
     * Test case for
     * {@link VTerminalIfIdentifier#create(String, String, String, boolean)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreate() throws Exception {
        String[] tnames = {
            "vtn", "vtn_1", "vtn_2",
        };
        String[] bnames = {
            "vtm", "vtm_1", "vterm_2",
        };
        String[] inames = {
            "if", "if_1", "vif_2",
        };
        boolean[] bools = {true, false};

        for (String tname: tnames) {
            for (String bname: bnames) {
                for (String iname: inames) {
                    for (boolean find: bools) {
                        VTerminalIfIdentifier ifId = VTerminalIfIdentifier.
                            create(tname, bname, iname, find);
                        assertEquals(tname, ifId.getTenantNameString());
                        assertEquals(bname, ifId.getBridgeNameString());
                        assertEquals(iname, ifId.getInterfaceNameString());
                    }
                }
            }
        }

        // Null name.
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = "VTN name cannot be null";
        for (boolean find: bools) {
            try {
                VTerminalIfIdentifier.
                    create((String)null, "vterm", "vif", find);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(null, e.getCause());
                assertEquals(msg, e.getMessage());
            }
        }

        msg = "vTerminal name cannot be null";
        for (boolean find: bools) {
            try {
                VTerminalIfIdentifier.
                    create("vtn", (String)null, "vif", find);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(null, e.getCause());
                assertEquals(msg, e.getMessage());
            }
        }

        msg = "vInterface name cannot be null";
        for (boolean find: bools) {
            try {
                VTerminalIfIdentifier.
                    create("vtn", "vterm", (String)null, find);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(null, e.getCause());
                assertEquals(msg, e.getMessage());
            }
        }

        String badName = "bad name";
        etag = RpcErrorTag.DATA_MISSING;
        vtag = VtnErrorTag.NOTFOUND;
        for (boolean find: bools) {
            // Empty VTN name.
            msg = ": VTN does not exist.";
            try {
                VTerminalIfIdentifier.create("", "vterm", "vif", find);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertTrue(e.getCause() instanceof RpcException);
                assertEquals(msg, e.getMessage());
            }

            // Empty vTerminal name.
            msg = ": vTerminal does not exist.";
            try {
                VTerminalIfIdentifier.create("vtn", "", "vif", find);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertTrue(e.getCause() instanceof RpcException);
                assertEquals(msg, e.getMessage());
            }

            // Invalid VTN name.
            msg = badName + ": VTN does not exist.";
            try {
                VTerminalIfIdentifier.create(badName, "vterm", "vif", find);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertTrue(e.getCause() instanceof RpcException);
                assertEquals(msg, e.getMessage());
            }

            // Invalid vTerminal name.
            msg = badName + ": vTerminal does not exist.";
            try {
                VTerminalIfIdentifier.create("vtn", badName, "vif", find);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertTrue(e.getCause() instanceof RpcException);
                assertEquals(msg, e.getMessage());
            }
        }

        // Empty interface name (read).
        msg = ": vInterface does not exist.";
        try {
            VTerminalIfIdentifier.create("vtn", "vterm", "", true);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertTrue(e.getCause() instanceof RpcException);
            assertEquals(msg, e.getMessage());
        }

        // Invalid interface name (read).
        msg = badName + ": vInterface does not exist.";
        try {
            VTerminalIfIdentifier.create("vtn", "vterm", badName, true);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertTrue(e.getCause() instanceof RpcException);
            assertEquals(msg, e.getMessage());
        }

        // Empty interface name (write).
        msg = "vInterface name cannot be empty";
        etag = RpcErrorTag.BAD_ELEMENT;
        vtag = VtnErrorTag.BADREQUEST;
        try {
            VTerminalIfIdentifier.create("vtn", "vterm", "", false);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(null, e.getCause());
            assertEquals(msg, e.getMessage());
        }

        // Invalid interface name (write).
        msg = "vInterface name is invalid";
        try {
            VTerminalIfIdentifier.create("vtn", "vterm", badName, false);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertTrue(e.getCause() instanceof IllegalArgumentException);
            assertEquals(msg, e.getMessage());
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#getTenantName()} and
     * {@link VNodeIdentifier#getTenantNameString()}.
     */
    @Test
    public void testGetTenantName() {
        VnodeName bname = new VnodeName("bridge_1");
        VnodeName iname = new VnodeName("if_1");

        String[] names = {
            null, "vtn", "vtn_1", "0123456789012345678901234567890",
        };
        for (String name: names) {
            VnodeName tname = (name == null) ? null : new VnodeName(name);
            VTerminalIfIdentifier ifId =
                new VTerminalIfIdentifier(tname, bname, iname);
            assertEquals(tname, ifId.getTenantName());
            assertEquals(name, ifId.getTenantNameString());

            VTerminalIdentifier vtmId = new VTerminalIdentifier(tname, bname);
            ifId = new VTerminalIfIdentifier(vtmId, iname);
            assertEquals(tname, ifId.getTenantName());
            assertEquals(name, ifId.getTenantNameString());
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#getBridgeName()} and
     * {@link VNodeIdentifier#getBridgeNameString()}.
     */
    @Test
    public void testGetBridgeName() {
        VnodeName tname = new VnodeName("vtn_1");
        VnodeName iname = new VnodeName("if_1");

        String[] names = {
            null, "vbr", "vterm_1", "0123456789012345678901234567890",
        };
        for (String name: names) {
            VnodeName bname = (name == null) ? null : new VnodeName(name);
            VTerminalIfIdentifier ifId =
                new VTerminalIfIdentifier(tname, bname, iname);
            assertEquals(bname, ifId.getBridgeName());
            assertEquals(name, ifId.getBridgeNameString());

            VTerminalIdentifier vtmId = new VTerminalIdentifier(tname, bname);
            ifId = new VTerminalIfIdentifier(vtmId, iname);
            assertEquals(bname, ifId.getBridgeName());
            assertEquals(name, ifId.getBridgeNameString());
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#getInterfaceName()} and
     * {@link VNodeIdentifier#getInterfaceNameString()}.
     */
    @Test
    public void testGetInterfaceName() {
        VnodeName tname = new VnodeName("vtn_1");
        VnodeName bname = new VnodeName("bridge_1");

        String[] names = {
            null, "if", "vif_1", "0123456789012345678901234567890",
        };
        for (String name: names) {
            VnodeName iname = (name == null) ? null : new VnodeName(name);
            VTerminalIfIdentifier ifId =
                new VTerminalIfIdentifier(tname, bname, iname);
            assertEquals(iname, ifId.getInterfaceName());
            assertEquals(name, ifId.getInterfaceNameString());

            VTerminalIdentifier vtmId = new VTerminalIdentifier(tname, bname);
            ifId = new VTerminalIfIdentifier(vtmId, iname);
            assertEquals(iname, ifId.getInterfaceName());
            assertEquals(name, ifId.getInterfaceNameString());
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#getIdentifier()} and
     * {@link VTerminalIfIdentifier#getIdentifierBuilder()}.
     */
    @Test
    public void testGetIdentifier() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vterm_1")};
        VnodeName[] inames = {new VnodeName("if"), new VnodeName("vif_1")};
        for (VnodeName tname: tnames) {
            VtnKey vtnKey = new VtnKey(tname);
            for (VnodeName bname: bnames) {
                VterminalKey vtermKey = new VterminalKey(bname);
                for (VnodeName iname: inames) {
                    VinterfaceKey vifKey = new VinterfaceKey(iname);
                    InstanceIdentifier<Vinterface> exp = InstanceIdentifier.
                        builder(Vtns.class).
                        child(Vtn.class, vtnKey).
                        child(Vterminal.class, vtermKey).
                        child(Vinterface.class, vifKey).
                        build();
                    VTerminalIfIdentifier ifId =
                        new VTerminalIfIdentifier(tname, bname, iname);
                    InstanceIdentifier<Vinterface> path = ifId.getIdentifier();
                    assertEquals(exp, path);

                    // Result should be cached.
                    for (int i = 0; i < 5; i++) {
                        assertSame(path, ifId.getIdentifier());
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#getComponents()} and
     * {@link VInterfaceIdentifier#newComponents()}.
     */
    @Test
    public void testGetComponents() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vterm_1")};
        VnodeName[] inames = {new VnodeName("if"), new VnodeName("vif_1")};
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                for (VnodeName iname: inames) {
                    VTerminalIfIdentifier ifId =
                        new VTerminalIfIdentifier(tname, bname, iname);
                    List<String> comps = ifId.getComponents();
                    assertEquals(3, comps.size());
                    assertEquals(tname.getValue(), comps.get(0));
                    assertEquals(bname.getValue(), comps.get(1));
                    assertEquals(iname.getValue(), comps.get(2));

                    // Result should be cached.
                    for (int i = 0; i < 5; i++) {
                        assertSame(comps, ifId.getComponents());
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#contains(VNodeIdentifier)}.
     */
    @Test
    public void testContains() {
        // VTerminalIfIdentifier.contains() returns true only if the given
        // instance is equal to the instance.
        List<VNodeIdentifier<?>> falseList = new ArrayList<>();
        falseList.add(null);

        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vterm_1")};
        VnodeName[] inames = {new VnodeName("if"), new VnodeName("vif_1")};
        MacVlan[] hosts = {
            new MacVlan(0xa0b0c0d0e0f0L, (short)0),
            new MacVlan(0x001122334455L, (short)1024),
        };
        for (VnodeName tname: tnames) {
            falseList.add(new VTenantIdentifier(tname));
            for (VnodeName bname: bnames) {
                Collections.addAll(
                    falseList,
                    new VBridgeIdentifier(tname, bname),
                    new VTerminalIdentifier(tname, bname),
                    new MacMapIdentifier(tname, bname));
                for (MacVlan host: hosts) {
                    falseList.add(
                        new MacMapHostIdentifier(tname, bname, host));
                }
                for (VnodeName iname: inames) {
                    VTerminalIfIdentifier ifId1 =
                        new VTerminalIfIdentifier(tname, bname, iname);
                    VTerminalIfIdentifier ifId2 =
                        new VTerminalIfIdentifier(tname, bname, iname);
                    assertEquals(true, ifId1.contains(ifId1));
                    assertEquals(true, ifId1.contains(ifId2));
                    assertEquals(true, ifId2.contains(ifId1));
                    assertEquals(true, ifId2.contains(ifId2));

                    Collections.addAll(
                        falseList,
                        new VBridgeIfIdentifier(tname, bname, iname),
                        new VlanMapIdentifier(tname, bname, iname.getValue()));

                    for (VNodeIdentifier<?> ident: falseList) {
                        assertEquals(false, ifId1.contains(ident));
                        assertEquals(false, ifId2.contains(ident));
                    }

                    falseList.add(ifId1);
                }
            }
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#contains(VirtualNodePath)}.
     */
    @Test
    public void testContainsVirtualNodePath() {
        // VTerminalIfIdentifier.contains(VirtualNodePath) returns true only if
        // the given instance specifies the same virtual interface.
        List<VirtualNodePath> falseList = new ArrayList<>();

        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        VnodeName[] inames = {new VnodeName("if"), new VnodeName("vif_1")};
        MacVlan[] hosts = {
            new MacVlan(0xa0b0c0d0e0f0L, (short)0),
            new MacVlan(0x001122334455L, (short)1024),
        };
        for (VnodeName vtnName: tnames) {
            String tname = vtnName.getValue();
            VirtualNodePath vtpath = new VirtualNodePathBuilder().
                setTenantName(tname).build();
            falseList.add(vtpath);

            for (VnodeName brName: bnames) {
                VBridgeIdentifier brId =
                    new VBridgeIdentifier(vtnName, brName);
                String bname = brName.getValue();
                VirtualNodePath vbpath = new VirtualNodePathBuilder().
                    setTenantName(tname).setBridgeName(bname).build();
                VirtualNodePath vtmpath = new VirtualNodePathBuilder().
                    setTenantName(tname).setTerminalName(bname).build();
                BridgeMapInfo bmi = new BridgeMapInfoBuilder().
                    setMacMappedHost(-1L).build();
                VirtualNodePath mpath = new VirtualNodePathBuilder().
                    setTenantName(tname).setBridgeName(bname).
                    addAugmentation(BridgeMapInfo.class, bmi).
                    build();
                Collections.addAll(falseList, vbpath, vtmpath, mpath);

                for (MacVlan host: hosts) {
                    bmi = new BridgeMapInfoBuilder().
                        setMacMappedHost(host.getEncodedValue()).build();
                    VirtualNodePath mhpath = new VirtualNodePathBuilder().
                        setTenantName(tname).setBridgeName(bname).
                        addAugmentation(BridgeMapInfo.class, bmi).
                        build();
                    falseList.add(mhpath);
                }
                for (VnodeName ifName: inames) {
                    VTerminalIfIdentifier ifId =
                        new VTerminalIfIdentifier(vtnName, brName, ifName);
                    String iname = ifName.getValue();
                    VirtualNodePath ipath1 = new VirtualNodePathBuilder().
                        setTenantName(tname).setBridgeName(bname).
                        setInterfaceName(iname).build();
                    VirtualNodePath ipath2 = new VirtualNodePathBuilder().
                        setTenantName(tname).setTerminalName(bname).
                        setInterfaceName(iname).build();
                    bmi = new BridgeMapInfoBuilder().
                        setVlanMapId(iname).build();
                    VirtualNodePath vmpath = new VirtualNodePathBuilder().
                        setTenantName(tname).setBridgeName(bname).
                        addAugmentation(BridgeMapInfo.class, bmi).
                        build();
                    Collections.addAll(falseList, ipath1, vmpath);
                    assertEquals(true, ifId.contains(ipath2));

                    for (VirtualNodePath vpath: falseList) {
                        assertEquals(false, ifId.contains(vpath));
                    }

                    falseList.add(ipath2);
                }
            }
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#getVirtualNodePath()}.
     */
    @Test
    public void testGetVirtualNodePath() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vterm_1")};
        VnodeName[] inames = {new VnodeName("if"), new VnodeName("vif_1")};
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                for (VnodeName iname: inames) {
                    VTerminalIfIdentifier ifId =
                        new VTerminalIfIdentifier(tname, bname, iname);
                    VirtualNodePath vpath = new VirtualNodePathBuilder().
                        setTenantName(tname.getValue()).
                        setTerminalName(bname.getValue()).
                        setInterfaceName(iname.getValue()).
                        build();
                    assertEquals(vpath, ifId.getVirtualNodePath());
                }
            }
        }
    }

    /**
     * Test case for {@link VInterfaceIdentifier#getStatusPath()}.
     */
    @Test
    public void testGetStatusPath() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vterm_1")};
        VnodeName[] inames = {new VnodeName("if"), new VnodeName("vif_1")};
        for (VnodeName tname: tnames) {
            VtnKey vtnKey = new VtnKey(tname);
            for (VnodeName bname: bnames) {
                VterminalKey vtermKey = new VterminalKey(bname);
                for (VnodeName iname: inames) {
                    VinterfaceKey vifKey = new VinterfaceKey(iname);
                    VTerminalIfIdentifier ifId =
                        new VTerminalIfIdentifier(tname, bname, iname);
                    InstanceIdentifier<VinterfaceStatus> expected =
                        InstanceIdentifier.builder(Vtns.class).
                        child(Vtn.class, vtnKey).
                        child(Vterminal.class, vtermKey).
                        child(Vinterface.class, vifKey).
                        child(VinterfaceStatus.class).
                        build();
                    InstanceIdentifier<VinterfaceStatus> path =
                        ifId.getStatusPath();
                    assertEquals(expected, path);

                    // The status path should be cached.
                    for (int i = 0; i < 10; i++) {
                        assertSame(path, ifId.getStatusPath());
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link VTerminalIfIdentifier#getBridgeIdentifier()}.
     */
    @Test
    public void testGetBridgeIdentifier() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vterm_1")};
        VnodeName[] inames = {new VnodeName("if"), new VnodeName("vif_1")};
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                VTerminalIdentifier expected =
                    new VTerminalIdentifier(tname, bname);
                for (VnodeName iname: inames) {
                    VTerminalIfIdentifier ifId =
                        new VTerminalIfIdentifier(tname, bname, iname);
                    VTerminalIdentifier ident = ifId.getBridgeIdentifier();
                    assertEquals(expected, ident);
                }
            }
        }
    }

    /**
     * Test case for {@link VTerminalIfIdentifier#getType()}.
     */
    @Test
    public void testGetType() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vterm_1")};
        VnodeName[] inames = {new VnodeName("if"), new VnodeName("vif_1")};
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                for (VnodeName iname: inames) {
                    VTerminalIfIdentifier ifId =
                        new VTerminalIfIdentifier(tname, bname, iname);
                    for (int i = 0; i < 5; i++) {
                        assertEquals(VNodeType.VTERMINAL_IF, ifId.getType());
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link VTerminalIfIdentifier#getVNodeIdentifier()}.
     */
    @Test
    public void testGetVNodeIdentifier() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vterm_1")};
        VnodeName[] inames = {new VnodeName("if"), new VnodeName("vif_1")};
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                for (VnodeName iname: inames) {
                    VTerminalIfIdentifier ifId =
                        new VTerminalIfIdentifier(tname, bname, iname);
                    for (int i = 0; i < 5; i++) {
                        VTerminalIfIdentifier ident =
                            ifId.getVNodeIdentifier();
                        assertSame(ifId, ident);
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link VTerminalIfIdentifier#replaceTenantName(VnodeName)}.
     */
    @Test
    public void testReplaceTenantName() {
        VnodeName[] tnames = {
            null, new VnodeName("vtn"), new VnodeName("vtn_1"),
        };
        VnodeName[] bnames = {new VnodeName("vtm"), new VnodeName("vterm_1")};
        VnodeName[] inames = {new VnodeName("if"), new VnodeName("vif_1")};
        VnodeName newName = new VnodeName("new_vtn_name");
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                for (VnodeName iname: inames) {
                    VTerminalIfIdentifier ifId =
                        new VTerminalIfIdentifier(tname, bname, iname);
                    VTerminalIfIdentifier ident =
                        ifId.replaceTenantName(newName);
                    assertEquals(newName, ident.getTenantName());
                    assertEquals(bname, ident.getBridgeName());
                    assertEquals(iname, ident.getInterfaceName());
                }
            }
        }
    }

    /**
     * Test case for {@link VTerminalIfIdentifier#toRedirectDestination()}.
     */
    @Test
    public void testToRedirectDestination() {
        String[] tnames = {null, "vtn", "vtn_1"};
        String[] bnames = {"vtm", "vterminal_1"};
        String[] inames = {"if", "vif_1"};
        for (String tname: tnames) {
            VnodeName vtnName = (tname == null) ? null : new VnodeName(tname);
            for (String bname: bnames) {
                VnodeName vtmName = new VnodeName(bname);
                for (String iname: inames) {
                    VnodeName ifName = new VnodeName(iname);
                    VTerminalIfIdentifier ifId =
                        new VTerminalIfIdentifier(vtnName, vtmName, ifName);
                    RedirectDestination rd = ifId.toRedirectDestination();
                    assertEquals(null, rd.getTenantName());
                    assertEquals(null, rd.getBridgeName());
                    assertEquals(bname, rd.getTerminalName());
                    assertEquals(null, rd.getRouterName());
                    assertEquals(iname, rd.getInterfaceName());
                }
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

        String[] tnames = {null, "vtn", "vtn_1"};
        String[] bnames = {null, "vbr", "vterm_1"};
        String[] inames = {null, "if", "vif_1"};
        for (String tname: tnames) {
            VnodeName vtnName1;
            VnodeName vtnName2;
            if (tname == null) {
                vtnName1 = null;
                vtnName2 = null;
            } else {
                vtnName1 = new VnodeName(tname);
                vtnName2 = copy(vtnName1);
            }
            for (String bname: bnames) {
                VnodeName vbrName1;
                VnodeName vbrName2;
                if (bname == null) {
                    vbrName1 = null;
                    vbrName2 = null;
                } else {
                    vbrName1 = new VnodeName(bname);
                    vbrName2 = copy(vbrName1);
                }
                for (String iname: inames) {
                    VnodeName ifName1;
                    VnodeName ifName2;
                    if (iname == null) {
                        ifName1 = null;
                        ifName2 = null;
                    } else {
                        ifName1 = new VnodeName(iname);
                        ifName2 = copy(ifName1);
                    }

                    VTerminalIfIdentifier ifId1 =
                        new VTerminalIfIdentifier(vtnName1, vbrName1, ifName1);
                    VTerminalIfIdentifier ifId2 =
                        new VTerminalIfIdentifier(vtnName2, vbrName2, ifName2);
                    testEquals(set, ifId1, ifId2);
                }
            }
        }

        int expected = tnames.length * bnames.length * inames.length;
        assertEquals(expected, set.size());
    }

    /**
     * Test case for {@link VNodeIdentifier#toString()} and
     * {@link VNodeIdentifier#create(String)}.
     */
    @Test
    public void testToString() {
        String[] tnames = {null, "vtn", "vtn_1"};
        String[] bnames = {null, "vbr", "vterm_1"};
        String[] inames = {null, "if", "vif_1"};
        for (String tname: tnames) {
            VnodeName vtnName;
            String tnameStr;
            if (tname == null) {
                vtnName = null;
                tnameStr = "<null>";
            } else {
                vtnName = new VnodeName(tname);
                tnameStr = tname;
            }
            for (String bname: bnames) {
                VnodeName vbrName;
                String bnameStr;
                if (bname == null) {
                    vbrName = null;
                    bnameStr = "<null>";
                } else {
                    vbrName = new VnodeName(bname);
                    bnameStr = bname;
                }
                for (String iname: inames) {
                    VnodeName ifName;
                    String inameStr;
                    if (iname == null) {
                        ifName = null;
                        inameStr = "<null>";
                    } else {
                        ifName = new VnodeName(iname);
                        inameStr = iname;
                    }

                    VTerminalIfIdentifier ifId =
                        new VTerminalIfIdentifier(vtnName, vbrName, ifName);
                    String expected = joinStrings(
                        "vTerminal-IF:", null, "/", tnameStr, bnameStr,
                        inameStr);
                    assertEquals(expected, ifId.toString());

                    // VNodeIdentifier.create() works only if path components
                    // are valid.
                    boolean valid = (tname != null && bname != null &&
                                     iname != null);
                    try {
                        VNodeIdentifier<?> ident =
                            VNodeIdentifier.create(expected);
                        assertTrue(valid);
                        assertEquals(ifId, ident);

                        // The given string should be cached.
                        assertSame(expected, ident.toString());
                    } catch (IllegalArgumentException e) {
                        assertFalse(valid);
                        assertEquals("Invalid identifier format: " + expected,
                                     e.getMessage());
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link VTerminalIfIdentifier} is mapped to XML root element.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        Marshaller[] marshallers = {
            createMarshaller(VNodeIdentifier.class),
            createMarshaller(VInterfaceIdentifier.class),
            createMarshaller(VTerminalIfIdentifier.class),
        };
        Unmarshaller[] unmarshallers = {
            createUnmarshaller(VNodeIdentifier.class),
            createUnmarshaller(VInterfaceIdentifier.class),
            createUnmarshaller(VTerminalIfIdentifier.class),
        };

        Class<VTerminalIfIdentifier> type = VTerminalIfIdentifier.class;
        String[] tnames = {null, "vtn", "vtn_1"};
        String[] bnames = {null, "vbr", "vterm_1"};
        String[] inames = {null, "if", "vif_1"};
        for (String tname: tnames) {
            for (String bname: bnames) {
                for (String iname: inames) {
                    XmlNode root = new XmlNode(XML_ROOT);
                    if (tname != null) {
                        root.add(new XmlNode("tenant-name", tname));
                    }
                    if (bname != null) {
                        root.add(new XmlNode("bridge-name", bname));
                    }
                    if (iname != null) {
                        root.add(new XmlNode("interface-name", iname));
                    }

                    String xml = root.toString();
                    for (Unmarshaller um: unmarshallers) {
                        VTerminalIfIdentifier ifId = unmarshal(um, xml, type);
                        assertEquals(tname, ifId.getTenantNameString());
                        assertEquals(bname, ifId.getBridgeNameString());
                        assertEquals(iname, ifId.getInterfaceNameString());

                        for (Marshaller m: marshallers) {
                            xml = marshal(m, ifId, type, XML_ROOT);
                            assertEquals(ifId, unmarshal(um, xml, type));
                        }
                    }
                }
            }
        }

        // Ensure that broken values in XML can be detected.
        List<XmlDataType> dlist = getXmlDataTypes(XML_ROOT);
        for (Unmarshaller um: unmarshallers) {
            jaxbErrorTest(um, type, dlist);
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#read(ReadTransaction)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testRead() throws Exception {
        VnodeName tname = new VnodeName("vtn_1");
        VnodeName bname = new VnodeName("vterminal_1");
        VnodeName iname = new VnodeName("if_1");
        VTerminalIfIdentifier ifId =
            new VTerminalIfIdentifier(tname, bname, iname);
        ReadTransaction rtx = mock(ReadTransaction.class);
        InstanceIdentifier<Vinterface> path = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, new VtnKey(tname)).
            child(Vterminal.class, new VterminalKey(bname)).
            child(Vinterface.class, new VinterfaceKey(iname)).
            build();
        Vinterface vif = new VinterfaceBuilder().build();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        when(rtx.read(oper, path)).thenReturn(getReadResult(vif));
        Optional<Vinterface> opt = ifId.read(rtx);
        assertEquals(vif, opt.get());
        verify(rtx).read(oper, path);
        verifyNoMoreInteractions(rtx);
        reset(rtx);

        vif = null;
        when(rtx.read(oper, path)).thenReturn(getReadResult(vif));
        opt = ifId.read(rtx);
        assertEquals(false, opt.isPresent());
        verify(rtx).read(oper, path);
        verifyNoMoreInteractions(rtx);
    }

    /**
     * Test case for {@link VNodeIdentifier#fetch(ReadTransaction)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testFetch() throws Exception {
        VnodeName tname = new VnodeName("vtn_1");
        VnodeName bname = new VnodeName("vterminal_1");
        VnodeName iname = new VnodeName("if_1");
        VTerminalIfIdentifier ifId =
            new VTerminalIfIdentifier(tname, bname, iname);
        ReadTransaction rtx = mock(ReadTransaction.class);
        InstanceIdentifier<Vinterface> path = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, new VtnKey(tname)).
            child(Vterminal.class, new VterminalKey(bname)).
            child(Vinterface.class, new VinterfaceKey(iname)).
            build();
        Vinterface vif = new VinterfaceBuilder().build();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        when(rtx.read(oper, path)).thenReturn(getReadResult(vif));
        assertEquals(vif, ifId.fetch(rtx));
        verify(rtx).read(oper, path);
        verifyNoMoreInteractions(rtx);
        reset(rtx);

        vif = null;
        when(rtx.read(oper, path)).thenReturn(getReadResult(vif));
        try {
            ifId.fetch(rtx);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
            assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
            assertEquals(ifId.toString() + ": vInterface does not exist.",
                         e.getMessage());
            assertEquals(null, e.getCause());
        }
        verify(rtx).read(oper, path);
        verifyNoMoreInteractions(rtx);
    }

    /**
     * Test case for {@link VNodeIdentifier#getNotFoundException()}.
     */
    @Test
    public void testGetNotFoundException() {
        RpcErrorTag etag = RpcErrorTag.DATA_MISSING;
        VtnErrorTag vtag = VtnErrorTag.NOTFOUND;

        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vterm_1")};
        VnodeName[] inames = {new VnodeName("if"), new VnodeName("vif_1")};
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                for (VnodeName iname: inames) {
                    VTerminalIfIdentifier ifId =
                        new VTerminalIfIdentifier(tname, bname, iname);
                    String msg = ifId.toString() +
                        ": vInterface does not exist.";
                    RpcException e = ifId.getNotFoundException();
                    assertEquals(etag, e.getErrorTag());
                    assertEquals(vtag, e.getVtnErrorTag());
                    assertEquals(null, e.getCause());
                    assertEquals(msg, e.getMessage());
                }
            }
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#getDataExistsException()}.
     */
    @Test
    public void testGetDataExistsException() {
        RpcErrorTag etag = RpcErrorTag.DATA_EXISTS;
        VtnErrorTag vtag = VtnErrorTag.CONFLICT;

        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vterm_1")};
        VnodeName[] inames = {new VnodeName("if"), new VnodeName("vif_1")};
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                for (VnodeName iname: inames) {
                    VTerminalIfIdentifier ifId =
                        new VTerminalIfIdentifier(tname, bname, iname);
                    String msg = ifId.toString() +
                        ": vInterface already exists.";
                    RpcException e = ifId.getDataExistsException();
                    assertEquals(etag, e.getErrorTag());
                    assertEquals(vtag, e.getVtnErrorTag());
                    assertEquals(null, e.getCause());
                    assertEquals(msg, e.getMessage());
                }
            }
        }
    }

    /**
     * Test case for
     * {@link VNodeIdentifier#getFlowFilterIdentifier(boolean, Integer)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetFlowFilterIdentifier() throws Exception {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vtm"), new VnodeName("vterm_1")};
        VnodeName[] inames = {new VnodeName("if"), new VnodeName("vif_1")};
        Integer[] indices = {1, 100, 65535};
        InstanceIdentifier<VtnFlowFilter> expected;
        InstanceIdentifier<VtnFlowFilter> path;
        for (VnodeName tname: tnames) {
            VtnKey vtnKey = new VtnKey(tname);
            for (VnodeName bname: bnames) {
                VterminalKey vtermKey = new VterminalKey(bname);
                for (VnodeName iname: inames) {
                    VinterfaceKey vifKey = new VinterfaceKey(iname);
                    VTerminalIfIdentifier ifId =
                        new VTerminalIfIdentifier(tname, bname, iname);
                    for (Integer index: indices) {
                        VtnFlowFilterKey filterKey =
                            new VtnFlowFilterKey(index);
                        expected = InstanceIdentifier.
                            builder(Vtns.class).
                            child(Vtn.class, vtnKey).
                            child(Vterminal.class, vtermKey).
                            child(Vinterface.class, vifKey).
                            child(VinterfaceInputFilter.class).
                            child(VtnFlowFilter.class, filterKey).
                            build();
                        path = ifId.getFlowFilterIdentifier(false, index);
                        assertEquals(expected, path);

                        expected = InstanceIdentifier.
                            builder(Vtns.class).
                            child(Vtn.class, vtnKey).
                            child(Vterminal.class, vtermKey).
                            child(Vinterface.class, vifKey).
                            child(VinterfaceOutputFilter.class).
                            child(VtnFlowFilter.class, filterKey).
                            build();
                        path = ifId.getFlowFilterIdentifier(true, index);
                        assertEquals(expected, path);
                    }
                }
            }
        }
    }

    /**
     * Test case for
     * {@link VNodeIdentifier#clearFlowFilter(ReadWriteTransaction, boolean)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testClearFlowFilter() throws Exception {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vtm"), new VnodeName("vterm_1")};
        VnodeName[] inames = {new VnodeName("if"), new VnodeName("vif_1")};
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;

        List<VtnFlowFilter> vfilters = new ArrayList<>();
        List<FlowFilterResult> results = new ArrayList<>();
        for (int i = 1; i <= 4; i++) {
            VtnFlowFilter vff = new VtnFlowFilterBuilder().
                setIndex(i).build();
            FlowFilterResult res = new FlowFilterResultBuilder().
                setIndex(i).setStatus(VtnUpdateType.REMOVED).build();
            vfilters.add(vff);
            results.add(res);
        }

        for (VnodeName tname: tnames) {
            VtnKey vtnKey = new VtnKey(tname);
            for (VnodeName bname: bnames) {
                VterminalKey vtermKey = new VterminalKey(bname);
                for (VnodeName iname: inames) {
                    VinterfaceKey vifKey = new VinterfaceKey(iname);
                    VTerminalIfIdentifier ifId =
                        new VTerminalIfIdentifier(tname, bname, iname);

                    InstanceIdentifier<VinterfaceInputFilter> ipath =
                        InstanceIdentifier.builder(Vtns.class).
                        child(Vtn.class, vtnKey).
                        child(Vterminal.class, vtermKey).
                        child(Vinterface.class, vifKey).
                        child(VinterfaceInputFilter.class).
                        build();

                    // INPUT: The target list is missing.
                    ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
                    VinterfaceInputFilter ilist = null;
                    when(tx.read(oper, ipath)).thenReturn(getReadResult(ilist));
                    assertEquals(null, ifId.clearFlowFilter(tx, false));
                    verify(tx).read(oper, ipath);
                    verifyNoMoreInteractions(tx);

                    // INPUT: Flow filter list is null.
                    List<VtnFlowFilter> filters = null;
                    ilist = mock(VinterfaceInputFilter.class);
                    when(ilist.getVtnFlowFilter()).thenReturn(filters);
                    tx = mock(ReadWriteTransaction.class);
                    when(tx.read(oper, ipath)).thenReturn(getReadResult(ilist));
                    assertEquals(null, ifId.clearFlowFilter(tx, false));
                    verify(tx).read(oper, ipath);
                    verify(tx).delete(oper, ipath);
                    verifyNoMoreInteractions(tx);

                    // INPUT: Flow filter list is empty.
                    filters = Collections.<VtnFlowFilter>emptyList();
                    ilist = mock(VinterfaceInputFilter.class);
                    when(ilist.getVtnFlowFilter()).thenReturn(filters);
                    tx = mock(ReadWriteTransaction.class);
                    when(tx.read(oper, ipath)).thenReturn(getReadResult(ilist));
                    assertEquals(null, ifId.clearFlowFilter(tx, false));
                    verify(tx).read(oper, ipath);
                    verify(tx).delete(oper, ipath);
                    verifyNoMoreInteractions(tx);

                    // INPUT: Flow filter list is not empty.
                    ilist = mock(VinterfaceInputFilter.class);
                    when(ilist.getVtnFlowFilter()).thenReturn(vfilters);
                    tx = mock(ReadWriteTransaction.class);
                    when(tx.read(oper, ipath)).thenReturn(getReadResult(ilist));
                    assertEquals(results, ifId.clearFlowFilter(tx, false));
                    verify(tx).read(oper, ipath);
                    verify(tx).delete(oper, ipath);
                    verifyNoMoreInteractions(tx);

                    InstanceIdentifier<VinterfaceOutputFilter> opath =
                        InstanceIdentifier.builder(Vtns.class).
                        child(Vtn.class, vtnKey).
                        child(Vterminal.class, vtermKey).
                        child(Vinterface.class, vifKey).
                        child(VinterfaceOutputFilter.class).
                        build();

                    // OUTPUT: The target list is missing.
                    tx = mock(ReadWriteTransaction.class);
                    VinterfaceOutputFilter olist = null;
                    when(tx.read(oper, opath)).thenReturn(getReadResult(olist));
                    assertEquals(null, ifId.clearFlowFilter(tx, true));
                    verify(tx).read(oper, opath);
                    verifyNoMoreInteractions(tx);

                    // OUTPUT: Flow filter list is null.
                    filters = null;
                    olist = mock(VinterfaceOutputFilter.class);
                    when(olist.getVtnFlowFilter()).thenReturn(filters);
                    tx = mock(ReadWriteTransaction.class);
                    when(tx.read(oper, opath)).thenReturn(getReadResult(olist));
                    assertEquals(null, ifId.clearFlowFilter(tx, true));
                    verify(tx).read(oper, opath);
                    verify(tx).delete(oper, opath);
                    verifyNoMoreInteractions(tx);

                    // OUTPUT: Flow filter list is empty.
                    filters = Collections.<VtnFlowFilter>emptyList();
                    olist = mock(VinterfaceOutputFilter.class);
                    when(olist.getVtnFlowFilter()).thenReturn(filters);
                    tx = mock(ReadWriteTransaction.class);
                    when(tx.read(oper, opath)).thenReturn(getReadResult(olist));
                    assertEquals(null, ifId.clearFlowFilter(tx, true));
                    verify(tx).read(oper, opath);
                    verify(tx).delete(oper, opath);
                    verifyNoMoreInteractions(tx);

                    // OUTPUT: Flow filter list is not empty.
                    olist = mock(VinterfaceOutputFilter.class);
                    when(olist.getVtnFlowFilter()).thenReturn(vfilters);
                    tx = mock(ReadWriteTransaction.class);
                    when(tx.read(oper, opath)).thenReturn(getReadResult(olist));
                    assertEquals(results, ifId.clearFlowFilter(tx, true));
                    verify(tx).read(oper, opath);
                    verify(tx).delete(oper, opath);
                    verifyNoMoreInteractions(tx);
                }
            }
        }
    }
}
