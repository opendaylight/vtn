/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.redirect.filter.config.RedirectDestination;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.redirect.filter.config.RedirectDestinationBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

/**
 * JUnit test for {@link VInterfaceIdentifier}.
 */
public class VInterfaceIdentifierTest extends TestBase {
    /**
     * Test case for {@link VInterfaceIdentifier#checkName(String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCheckName() throws Exception {
        // Valid name.
        String[] names = {
            "0",
            "01",
            "012",
            "0123",
            "012345678901234567890123456789",
            "0123456789012345678901234567890",
            "a",
            "ab",
            "abc",
            "abcABC",
            "abcABC_",
            "abcABC_0123",
            "abcABC_0123_XXXXXXXXXXXXXXXXXXX",
        };

        for (String name: names) {
            VnodeName vname = new VnodeName(name);
            assertEquals(vname, VInterfaceIdentifier.checkName(name));
        }

        // Null name.
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = "vInterface name cannot be null";
        try {
            VInterfaceIdentifier.checkName((String)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(null, e.getCause());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        // Empty name.
        msg = "vInterface name cannot be empty";
        etag = RpcErrorTag.BAD_ELEMENT;
        try {
            VInterfaceIdentifier.checkName("");
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(null, e.getCause());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        // Invalid name.
        msg = "vInterface name is invalid";
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

        for (String name: invalidNames) {
            try {
                VInterfaceIdentifier.checkName(name);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertTrue(e.getCause() instanceof IllegalArgumentException);
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }
    }

    /**
     * Test case for
     * {@link VInterfaceIdentifier#getNoRedirectDestinationException()}.
     */
    @Test
    public void getGetNoRedirectDestinationException() {
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = "redirect-destination cannot be null";

        RpcException e = VInterfaceIdentifier.
            getNoRedirectDestinationException();
        assertEquals(etag, e.getErrorTag());
        assertEquals(null, e.getCause());
        assertEquals(vtag, e.getVtnErrorTag());
        assertEquals(msg, e.getMessage());
    }

    /**
     * Test case for {@link VInterfaceIdentifier#create(RedirectDestination)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreate() throws Exception {
        String[] tnames = {null, "vtn", "vtn_1"};
        String[] bnames = {"b", "bridge"};
        String[] inames = {"if", "vif_1"};
        for (String tname: tnames) {
            VnodeName vtnName = (tname == null) ? null : new VnodeName(tname);
            for (String bname: bnames) {
                VnodeName brName = new VnodeName(bname);
                for (String iname: inames) {
                    VnodeName ifName = new VnodeName(iname);

                    // vBridge
                    RedirectDestination rd = new RedirectDestinationBuilder().
                        setTenantName(tname).
                        setBridgeName(bname).
                        setInterfaceName(iname).
                        build();
                    VInterfaceIdentifier ifId =
                        VInterfaceIdentifier.create(rd);
                    assertTrue(ifId instanceof VBridgeIfIdentifier);
                    assertEquals(VNodeType.VBRIDGE_IF, ifId.getType());

                    // VTN name should be always ignored.
                    assertEquals(null, ifId.getTenantName());
                    assertEquals(brName, ifId.getBridgeName());
                    assertEquals(ifName, ifId.getInterfaceName());

                    // vTerminal
                    rd = new RedirectDestinationBuilder().
                        setTenantName(tname).
                        setTerminalName(bname).
                        setInterfaceName(iname).
                        build();
                    ifId = VInterfaceIdentifier.create(rd);
                    assertTrue(ifId instanceof VTerminalIfIdentifier);
                    assertEquals(VNodeType.VTERMINAL_IF, ifId.getType());

                    // VTN name should be always ignored.
                    assertEquals(null, ifId.getTenantName());
                    assertEquals(brName, ifId.getBridgeName());
                    assertEquals(ifName, ifId.getInterfaceName());

                }
            }
        }

        // No destination.
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = "redirect-destination cannot be null";
        RedirectDestination rdest = null;
        try {
            VInterfaceIdentifier.create(rdest);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(null, e.getCause());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        // No vInterface name.
        String vbrName = "vbr";
        String vtmName = "vtm";
        RedirectDestination[] dests = {
            new RedirectDestinationBuilder().setBridgeName(vbrName).build(),
            new RedirectDestinationBuilder().setTerminalName(vtmName).build(),
        };
        msg = "vInterface name cannot be null";
        for (RedirectDestination rd: dests) {
            try {
                VInterfaceIdentifier.create(rd);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals("Invalid redirect-destination: " + msg,
                             e.getMessage());

                Throwable cause = e.getCause();
                assertTrue(cause instanceof RpcException);
                RpcException re = (RpcException)cause;
                assertEquals(etag, re.getErrorTag());
                assertEquals(vtag, re.getVtnErrorTag());
                assertEquals(msg, re.getMessage());
                assertEquals(null, re.getCause());
            }
        }

        // No bridge name.
        String ifName = "if_1";
        etag = RpcErrorTag.BAD_ELEMENT;
        rdest = new RedirectDestinationBuilder().
            setInterfaceName(ifName).build();
        msg = "Unexpected virtual node path: " + rdest;
        try {
            VInterfaceIdentifier.create(rdest);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(null, e.getCause());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        // Empty vBridge name.
        String empty = "";
        msg = "vBridge name cannot be empty";
        rdest = new RedirectDestinationBuilder().
            setBridgeName(empty).setInterfaceName(ifName).build();
        try {
            VInterfaceIdentifier.create(rdest);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals("Invalid redirect-destination: " + msg,
                         e.getMessage());

            Throwable cause = e.getCause();
            assertTrue(cause instanceof RpcException);
            RpcException re = (RpcException)cause;
            assertEquals(etag, re.getErrorTag());
            assertEquals(vtag, re.getVtnErrorTag());
            assertEquals(msg, re.getMessage());
            assertEquals(null, re.getCause());
        }

        // Empty vTerminal name.
        msg = "vTerminal name cannot be empty";
        rdest = new RedirectDestinationBuilder().
            setTerminalName(empty).setInterfaceName(ifName).build();
        try {
            VInterfaceIdentifier.create(rdest);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals("Invalid redirect-destination: " + msg,
                         e.getMessage());

            Throwable cause = e.getCause();
            assertTrue(cause instanceof RpcException);
            RpcException re = (RpcException)cause;
            assertEquals(etag, re.getErrorTag());
            assertEquals(vtag, re.getVtnErrorTag());
            assertEquals(msg, re.getMessage());
            assertEquals(null, re.getCause());
        }

        // Empty vInterface name.
        msg = "vInterface name cannot be empty";
        dests = new RedirectDestination[] {
            new RedirectDestinationBuilder().setBridgeName(vbrName).
            setInterfaceName(empty).build(),
            new RedirectDestinationBuilder().setTerminalName(vtmName).
            setInterfaceName(empty).build(),
        };
        for (RedirectDestination rd: dests) {
            try {
                VInterfaceIdentifier.create(rd);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals("Invalid redirect-destination: " + msg,
                             e.getMessage());

                Throwable cause = e.getCause();
                assertTrue(cause instanceof RpcException);
                RpcException re = (RpcException)cause;
                assertEquals(etag, re.getErrorTag());
                assertEquals(vtag, re.getVtnErrorTag());
                assertEquals(msg, re.getMessage());
                assertEquals(null, re.getCause());
            }
        }

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
        for (String invalid: invalidNames) {
            // Invalid vBridge name.
            msg = "vBridge name is invalid";
            rdest = new RedirectDestinationBuilder().
                setBridgeName(invalid).setInterfaceName(ifName).build();
            try {
                VInterfaceIdentifier.create(rdest);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals("Invalid redirect-destination: " + msg,
                             e.getMessage());

                Throwable cause = e.getCause();
                assertTrue(cause instanceof RpcException);
                RpcException re = (RpcException)cause;
                assertEquals(etag, re.getErrorTag());
                assertEquals(vtag, re.getVtnErrorTag());
                assertEquals(msg, re.getMessage());
                assertTrue(re.getCause() instanceof IllegalArgumentException);
            }

            // Invalid vTerminal name.
            msg = "vTerminal name is invalid";
            rdest = new RedirectDestinationBuilder().
                setTerminalName(invalid).setInterfaceName(ifName).build();
            try {
                VInterfaceIdentifier.create(rdest);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals("Invalid redirect-destination: " + msg,
                             e.getMessage());

                Throwable cause = e.getCause();
                assertTrue(cause instanceof RpcException);
                RpcException re = (RpcException)cause;
                assertEquals(etag, re.getErrorTag());
                assertEquals(vtag, re.getVtnErrorTag());
                assertEquals(msg, re.getMessage());
                assertTrue(re.getCause() instanceof IllegalArgumentException);
            }

            // Invalid vInterface name.
            msg = "vInterface name is invalid";
            dests = new RedirectDestination[] {
                new RedirectDestinationBuilder().setBridgeName(vbrName).
                setInterfaceName(invalid).build(),
                new RedirectDestinationBuilder().setTerminalName(vtmName).
                setInterfaceName(invalid).build(),
            };
            for (RedirectDestination rd: dests) {
                try {
                    VInterfaceIdentifier.create(rd);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(etag, e.getErrorTag());
                    assertEquals(vtag, e.getVtnErrorTag());
                    assertEquals("Invalid redirect-destination: " + msg,
                                 e.getMessage());

                    Throwable cause = e.getCause();
                    assertTrue(cause instanceof RpcException);
                    RpcException re = (RpcException)cause;
                    assertEquals(etag, re.getErrorTag());
                    assertEquals(vtag, re.getVtnErrorTag());
                    assertEquals(msg, re.getMessage());
                    assertTrue(re.getCause() instanceof
                               IllegalArgumentException);
                }
            }
        }
    }
}
