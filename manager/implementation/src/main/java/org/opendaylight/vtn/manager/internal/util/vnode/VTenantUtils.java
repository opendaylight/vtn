/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenantPath;

import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.InstanceIdentifierBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.Vtns;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;

/**
 * {@code VTenantUtils} class is a collection of utility class methods
 * for virtual tenant management.
 */
public final class VTenantUtils {
    /**
     * A brief description about VTN.
     */
    private static final String  DESC_TENANT = "Tenant";

    /**
     * Private constructor that protects this class from instantiating.
     */
    private VTenantUtils() {}

    /**
     * Return a new {@link RpcException} that indicates the specified VTN
     * is not present.
     *
     * @param name  The name of the VTN.
     * @return  An {@link RpcException}.
     */
    public static RpcException getNotFoundException(String name) {
        return getNotFoundException(name, null);
    }

    /**
     * Return a new {@link RpcException} that indicates the specified VTN
     * is not present.
     *
     * @param name   The name of the VTN.
     * @param cause  A {@link Throwable} which indicates the cause of error.
     * @return  An {@link RpcException}.
     */
    public static RpcException getNotFoundException(String name,
                                                    Throwable cause) {
        String msg =
            MiscUtils.joinColon(name, "Tenant does not exist.");
        return RpcException.getNotFoundException(msg, cause);
    }

    /**
     * Return a new {@link RpcException} that indicates the specified VTN
     * already exists.
     *
     * @param name  The name of the VTN.
     * @return  An {@link RpcException}.
     */
    public static RpcException getNameConflictException(String name) {
        String msg =
            MiscUtils.joinColon(name, "Tenant name already exists.");
        return RpcException.getDataExistsException(msg);
    }

    /**
     * Verify the name of the VTN.
     *
     * @param name  The name of the VTN.
     * @return  A {@link VnodeName} instance that contains the given name.
     * @throws RpcException  The specified name is invalid.
     */
    public static VnodeName checkName(String name) throws RpcException {
        return MiscUtils.checkName(DESC_TENANT, name);
    }

    /**
     * Return a {@link VnodeName} instance that contains the given VTN name.
     *
     * <p>
     *   This method is used to retrieve existing VTN.
     * </p>
     *
     * @param name  The name of the VTN.
     * @return  A {@link VnodeName} instance that contains the given name.
     * @throws RpcException  The specified name is invalid.
     */
    public static VnodeName getVnodeName(String name) throws RpcException {
        try {
            return MiscUtils.checkName(DESC_TENANT, name);
        } catch (RpcException e) {
            if (e.getErrorTag() == RpcErrorTag.BAD_ELEMENT) {
                // The specified VTN should not be present because the given
                // name is invalid.
                throw getNotFoundException(name, e);
            }
            throw e;
        }
    }

    /**
     * Return a {@link VnodeName} instance that contains the VTN name in the
     * given tenant path.
     *
     * <p>
     *   This method is used to retrieve existing VTN.
     * </p>
     *
     * @param path  Path to the VTN.
     * @return  A {@link VnodeName} instance that contains the given name.
     * @throws RpcException  The specified name is invalid.
     */
    public static VnodeName getVnodeName(VTenantPath path)
        throws RpcException {
        return getVnodeName(getName(path));
    }

    /**
     * Return the {@link VnodeName} instance that contains the name of the
     * VTN configured in the given instance identifier.
     *
     * @param path  An {@link InstanceIdentifier} instance.
     * @return  A {@link VnodeName} instance that contains the name of the
     *          VTN if found.
     *          {@code null} if not found.
     */
    public static VnodeName getVnodeName(InstanceIdentifier<?> path) {
        VtnKey key = path.firstKeyOf(Vtn.class, VtnKey.class);
        return (key == null) ? null : key.getName();
    }

    /**
     * Return the name of the VTN configured in the given instance identifier.
     *
     * @param path  An {@link InstanceIdentifier} instance.
     * @return  The name of the VTN if found.
     *          {@code null} if not found.
     */
    public static String getTenantName(InstanceIdentifier<?> path) {
        VnodeName vname = getVnodeName(path);
        return (vname == null) ? null : vname.getValue();
    }

    /**
     * Return the name of the VTN configured in the given instance identifier.
     *
     * @param path  An {@link InstanceIdentifier} instance.
     * @return  The name of the VTN if found.
     *          {@code null} if not found.
     */
    public static String getName(InstanceIdentifier<?> path) {
        VnodeName vname = getVnodeName(path);
        return (vname == null) ? null : vname.getValue();
    }

    /**
     * Return the name of the VTN in the given tenant path.
     *
     * @param path  Path to the VTN.
     * @return  The name of the VTN in the given path.
     * @throws RpcException  An error occurred.
     */
    public static String getName(VTenantPath path) throws RpcException {
        if (path == null) {
            throw RpcException.getNullArgumentException("VTenantPath");
        }

        String name = path.getTenantName();
        if (name == null) {
            throw RpcException.getNullArgumentException(DESC_TENANT + " name");
        }

        return name;
    }

    /**
     * Create the instance identifier for the VTN specified by the given name.
     *
     * <p>
     *   This method is used to retrieve existing VTN.
     * </p>
     *
     * @param name  The name of the VTN.
     * @return  An {@link InstanceIdentifier} instance.
     * @throws RpcException  The given VTN name is invalid.
     */
    public static InstanceIdentifier<Vtn> getIdentifier(String name)
        throws RpcException {
        return getIdentifier(getVnodeName(name));
    }

    /**
     * Create the instance identifier for the VTN specified by the given name.
     *
     * @param vname  A {@link VnodeName} instance that contains the name of
     *               the VTN.
     * @return  An {@link InstanceIdentifier} instance.
     */
    public static InstanceIdentifier<Vtn> getIdentifier(VnodeName vname) {
        return getIdentifierBuilder(vname).build();
    }

    /**
     * Return an instance identifier builder that contains the given VTN name.
     *
     * @param vname  A {@link VnodeName} instance that contains the name of
     *               the VTN.
     * @return  An instance identififer builder which specifies data in the
     *          VTN model.
     */
    public static InstanceIdentifierBuilder<Vtn> getIdentifierBuilder(
        VnodeName vname) {
        return InstanceIdentifier.builder(Vtns.class).
            child(Vtn.class, new VtnKey(vname));
    }

    /**
     * Read the VTN specified by the given name.
     *
     * @param rtx    A {@link ReadTransaction} instance associated with the
     *               read transaction for the MD-SAL datastore.
     * @param vname  A {@link VnodeName} instance that contains the name of
     *               the VTN.
     * @return  A {@link Vtn} instance.
     * @throws VTNException  An error occurred.
     */
    public static Vtn readVtn(ReadTransaction rtx, VnodeName vname)
        throws VTNException {
        InstanceIdentifier<Vtn> path = getIdentifier(vname);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Optional<Vtn> opt = DataStoreUtils.read(rtx, oper, path);
        if (!opt.isPresent()) {
            throw getNotFoundException(vname.getValue());
        }
        return opt.get();
    }
}
