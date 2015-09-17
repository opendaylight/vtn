/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.junit.Test;

import org.mockito.Mockito;

import org.opendaylight.vtn.manager.VTenantPath;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.Vtns;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

/**
 * JUnit test for {@link VTenantUtils}.
 */
public class VTenantUtilsTest extends TestBase {
    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link VTenantUtils#getNotFoundException(String)}</li>
     *   <li>{@link VTenantUtils#getNotFoundException(String, Throwable)}</li>
     *   <li>{@link VTenantUtils#getNameConflictException(String)}</li>
     * </ul>
     */
    @Test
    public void testException() {
        String[] names = {
            "vtn_1",
            "vtn_2",
            "vtn",
        };

        Throwable cause = new IllegalStateException();
        for (String name: names) {
            String msg = name + ": Tenant does not exist.";
            RpcException e = VTenantUtils.getNotFoundException(name);
            assertEquals(null, e.getCause());
            assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
            assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());

            e = VTenantUtils.getNotFoundException(name, cause);
            assertSame(cause, e.getCause());
            assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
            assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());

            msg = name + ": Tenant name already exists.";
            e = VTenantUtils.getNameConflictException(name);
            assertEquals(null, e.getCause());
            assertEquals(RpcErrorTag.DATA_EXISTS, e.getErrorTag());
            assertEquals(VtnErrorTag.CONFLICT, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link VTenantUtils#checkName(String)}</li>
     *   <li>{@link VTenantUtils#getVnodeName(InstanceIdentifier)}</li>
     *   <li>{@link VTenantUtils#getVnodeName(String)}</li>
     *   <li>{@link VTenantUtils#getVnodeName(VTenantPath)}</li>
     *   <li>{@link VTenantUtils#getName(InstanceIdentifier)}</li>
     *   <li>{@link VTenantUtils#getName(VTenantPath)}</li>
     *   <li>{@link VTenantUtils#getIdentifier(String)}</li>
     *   <li>{@link VTenantUtils#getIdentifier(VnodeName)}</li>
     *   <li>{@link VTenantUtils#getIdentifierBuilder(VnodeName)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testName() throws Exception {
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
            assertEquals(vname, VTenantUtils.checkName(name));
            assertEquals(vname, VTenantUtils.getVnodeName(name));

            VtnKey key = new VtnKey(vname);
            InstanceIdentifier<Vtn> expected = InstanceIdentifier.
                builder(Vtns.class).child(Vtn.class, key).build();
            assertEquals(expected, VTenantUtils.getIdentifier(name));
            assertEquals(expected, VTenantUtils.getIdentifier(vname));
            assertEquals(expected,
                         VTenantUtils.getIdentifierBuilder(vname).build());
            assertEquals(vname, VTenantUtils.getVnodeName(expected));
            assertEquals(name, VTenantUtils.getName(expected));

            VTenantPath tpath = new VTenantPath(name);
            assertEquals(vname, VTenantUtils.getVnodeName(tpath));
            assertEquals(name, VTenantUtils.getName(tpath));
        }

        // Null VTenantPath.
        String msg = "VTenantPath cannot be null";
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        try {
            VTenantUtils.getName((VTenantPath)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(null, e.getCause());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        try {
            VTenantUtils.getVnodeName((VTenantPath)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(null, e.getCause());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        // Null VTN name.
        msg = "Tenant name cannot be null";
        try {
            VTenantUtils.checkName((String)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(null, e.getCause());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        try {
            VTenantUtils.getVnodeName((String)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(null, e.getCause());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        try {
            VTenantUtils.getVnodeName(new VTenantPath((String)null));
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(null, e.getCause());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        try {
            VTenantUtils.getName(new VTenantPath((String)null));
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(null, e.getCause());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        try {
            VTenantUtils.getIdentifier((String)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(null, e.getCause());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        // Empty VTN name.
        msg = "Tenant name cannot be empty";
        etag = RpcErrorTag.BAD_ELEMENT;
        try {
            VTenantUtils.checkName("");
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(null, e.getCause());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        String emsg = ": Tenant does not exist.";
        try {
            VTenantUtils.getVnodeName("");
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
            assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
            Throwable cause = e.getCause();
            assertTrue(cause instanceof RpcException);
            RpcException re = (RpcException)cause;
            assertEquals(etag, re.getErrorTag());
            assertEquals(vtag, re.getVtnErrorTag());
            assertEquals(msg, re.getMessage());
            assertEquals(null, re.getCause());
        }

        try {
            VTenantUtils.getVnodeName(new VTenantPath(""));
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
            assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
            Throwable cause = e.getCause();
            assertTrue(cause instanceof RpcException);
            RpcException re = (RpcException)cause;
            assertEquals(etag, re.getErrorTag());
            assertEquals(vtag, re.getVtnErrorTag());
            assertEquals(msg, re.getMessage());
            assertEquals(null, re.getCause());
        }

        try {
            VTenantUtils.getIdentifier("");
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
            assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
            Throwable cause = e.getCause();
            assertTrue(cause instanceof RpcException);
            RpcException re = (RpcException)cause;
            assertEquals(etag, re.getErrorTag());
            assertEquals(vtag, re.getVtnErrorTag());
            assertEquals(msg, re.getMessage());
            cause = re.getCause();
            assertEquals(null, re.getCause());
        }

        // Invalid VTN name.
        msg = "Tenant name is invalid";
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
                VTenantUtils.checkName(name);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertTrue(e.getCause() instanceof IllegalArgumentException);
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }

            emsg = name + ": Tenant does not exist.";
            try {
                VTenantUtils.getVnodeName(name);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
                assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
                Throwable cause = e.getCause();
                assertTrue(cause instanceof RpcException);
                RpcException re = (RpcException)cause;
                assertEquals(etag, re.getErrorTag());
                assertEquals(vtag, re.getVtnErrorTag());
                assertEquals(msg, re.getMessage());
                cause = re.getCause();
                assertTrue(cause instanceof IllegalArgumentException);
            }

            try {
                VTenantUtils.getVnodeName(new VTenantPath(name));
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
                assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
                Throwable cause = e.getCause();
                assertTrue(cause instanceof RpcException);
                RpcException re = (RpcException)cause;
                assertEquals(etag, re.getErrorTag());
                assertEquals(vtag, re.getVtnErrorTag());
                assertEquals(msg, re.getMessage());
                cause = re.getCause();
                assertTrue(cause instanceof IllegalArgumentException);
            }

            try {
                VTenantUtils.getIdentifier(name);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
                assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
                Throwable cause = e.getCause();
                assertTrue(cause instanceof RpcException);
                RpcException re = (RpcException)cause;
                assertEquals(etag, re.getErrorTag());
                assertEquals(vtag, re.getVtnErrorTag());
                assertEquals(msg, re.getMessage());
                cause = re.getCause();
                assertTrue(cause instanceof IllegalArgumentException);
            }
        }
    }

    /**
     * Test case for
     * {@link VTenantUtils#readVtn(ReadTransaction, VnodeName)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testReadVtn() throws Exception {
        // Set up mock-up of MD-SAL read transaction.
        ReadTransaction rtx = Mockito.mock(ReadTransaction.class);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        List<VnodeName> names = new ArrayList<>();
        List<InstanceIdentifier<Vtn>> paths = new ArrayList<>();
        Map<VnodeName, Vtn> present = new HashMap<>();

        for (int i = 0; i < 10; i++) {
            String name = "present" + i;
            VnodeName vname = new VnodeName(name);
            names.add(vname);
            Vtn vtn = new VtnBuilder().setName(vname).build();
            assertEquals(null, present.put(vname, vtn));
            InstanceIdentifier<Vtn> path = VTenantUtils.getIdentifier(vname);
            paths.add(path);
            Mockito.when(rtx.read(oper, path)).thenReturn(getReadResult(vtn));
        }

        Vtn vtnNull = null;
        for (int i = 0; i < 10; i++) {
            String name = "notfound" + i;
            VnodeName vname = new VnodeName(name);
            names.add(vname);
            assertEquals(null, present.get(vname));
            InstanceIdentifier<Vtn> path = VTenantUtils.getIdentifier(vname);
            paths.add(path);
            Mockito.when(rtx.read(oper, path)).
                thenReturn(getReadResult(vtnNull));
        }

        for (VnodeName vname: names) {
            Vtn expected = present.get(vname);
            try {
                Vtn vtn = VTenantUtils.readVtn(rtx, vname);
                assertNotNull(expected);
                assertEquals(expected, vtn);
            } catch (RpcException e) {
                assertEquals(null, expected);
                assertEquals(null, e.getCause());
                assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
                assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
                String msg = vname.getValue() + ": Tenant does not exist.";
                assertEquals(msg, e.getMessage());
            }
        }

        for (InstanceIdentifier<Vtn> path: paths) {
            Mockito.verify(rtx).read(oper, path);
        }
    }
}
