/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.cond;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Set;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnFlowConditions;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.cond.config.VtnFlowMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.cond.config.VtnFlowMatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.cond.config.VtnFlowMatchKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.conditions.VtnFlowCondition;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.conditions.VtnFlowConditionKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;

/**
 * {@code FlowCondUtils} class is a collection of utility class methods
 * for flow condition.
 */
public final class FlowCondUtils {
    /**
     * A brief description about flow condition.
     */
    private static final String  DESC_FLOW_COND = "Flow condition";

    /**
     * Private constructor that protects this class from instantiating.
     */
    private FlowCondUtils() {}

    /**
     * Return a new {@link RpcException} that indicates the specified flow
     * condition is not present.
     *
     * @param name  The name of the flow condition.
     * @return  An {@link RpcException}.
     */
    public static RpcException getNotFoundException(String name) {
        return getNotFoundException(name, null);
    }

    /**
     * Return a new {@link RpcException} that indicates the specified flow
     * condition is not present.
     *
     * @param name   The name of the flow condition.
     * @param cause  A {@link Throwable} which indicates the cause of error.
     * @return  An {@link RpcException}.
     */
    public static RpcException getNotFoundException(String name,
                                                    Throwable cause) {
        String msg =
            MiscUtils.joinColon(name, "Flow condition does not exist.");
        return RpcException.getNotFoundException(msg, cause);
    }

    /**
     * Return a new {@link RpcException} that indicates the flow match index
     * is missing.
     *
     * @return  An {@link RpcException}.
     */
    public static RpcException getMatchIndexMissingException() {
        return RpcException.getNullArgumentException("Match index");
    }

    /**
     * Verify the name of the flow condition.
     *
     * @param name  The name of the flow condition.
     * @return  A {@link VnodeName} instance that contains the given name.
     * @throws RpcException  The specified name is invalid.
     */
    public static VnodeName checkName(String name) throws RpcException {
        return MiscUtils.checkName(DESC_FLOW_COND, name);
    }

    /**
     * Verify the name of the flow condition.
     *
     * @param vname  A {@link VnodeName} instance.
     * @return  Return the string in {@code vname}.
     * @throws RpcException  The specified name is invalid.
     */
    public static String checkName(VnodeName vname) throws RpcException {
        return MiscUtils.checkName(DESC_FLOW_COND, vname);
    }

    /**
     * Ensure the given vnode-name is not null.
     *
     * @param vname  A {@link VnodeName} instance.
     * @throws RpcException  {@code vname} is {@code null}.
     */
    public static void checkPresent(VnodeName vname) throws RpcException {
        if (vname == null) {
            throw RpcException.getNullArgumentException(
                DESC_FLOW_COND + " name");
        }
    }

    /**
     * Create the instance identifier for the flow condition specified by the
     * given name.
     *
     * <p>
     *   This method is used to retrieve existing flow condition.
     * </p>
     *
     * @param name  The name of the flow condition.
     * @return  An {@link InstanceIdentifier} instance.
     * @throws RpcException
     *    The given flow condition name is invalid.
     */
    public static InstanceIdentifier<VtnFlowCondition> getIdentifier(
        String name) throws RpcException {
        return getIdentifier(getVnodeName(name));
    }

    /**
     * Create the instance identifier for the flow condition specified by the
     * given name.
     *
     * @param vname  A {@link VnodeName} instance that contains the name of
     *               the flow condition.
     * @return  An {@link InstanceIdentifier} instance.
     */
    public static InstanceIdentifier<VtnFlowCondition> getIdentifier(
        VnodeName vname) {
        return InstanceIdentifier.builder(VtnFlowConditions.class).
            child(VtnFlowCondition.class, new VtnFlowConditionKey(vname)).
            build();
    }

    /**
     * Create the instance identifier for the flow match specified by the
     * given flow condition name and match index.
     *
     * <p>
     *   This method is used to retrieve flow match in existing flow condition.
     * </p>
     *
     * @param name   The name of the flow condition.
     * @param index  The index assigned to the flow match in a flow condition.
     * @return  An {@link InstanceIdentifier} instance.
     * @throws RpcException
     *    The given flow condition name or match index is invalid.
     */
    public static InstanceIdentifier<VtnFlowMatch> getIdentifier(
        String name, Integer index) throws RpcException {
        return getIdentifier(getVnodeName(name), index);
    }

    /**
     * Create the instance identifier for the flow match specified by the
     * given flow condition name and match index.
     *
     * <p>
     *   This method is used to retrieve flow match in existing flow condition.
     * </p>
     *
     * @param vname  A {@link VnodeName} instance that contains the name of
     *               the flow condition.
     * @param index  The index assigned to the flow match in a flow condition.
     * @return  An {@link InstanceIdentifier} instance.
     * @throws RpcException
     *    The given flow condition name or match index is invalid.
     */
    public static InstanceIdentifier<VtnFlowMatch> getIdentifier(
        VnodeName vname, Integer index) throws RpcException {
        if (index == null) {
            throw getMatchIndexMissingException();
        }

        return InstanceIdentifier.builder(VtnFlowConditions.class).
            child(VtnFlowCondition.class, new VtnFlowConditionKey(vname)).
            child(VtnFlowMatch.class, new VtnFlowMatchKey(index)).build();
    }

    /**
     * Return a {@link VnodeName} instance that contains the given flow
     * condition name.
     *
     * <p>
     *   This method is used to retrieve existing flow condition.
     * </p>
     *
     * @param name  The name of the flow condition.
     * @return  A {@link VnodeName} instance that contains the given name.
     * @throws RpcException  The specified name is invalid.
     */
    public static VnodeName getVnodeName(String name) throws RpcException {
        try {
            return MiscUtils.checkName(DESC_FLOW_COND, name);
        } catch (RpcException e) {
            if (e.getErrorTag() == RpcErrorTag.BAD_ELEMENT) {
                // The specified flow condition should not be present because
                // the given name is invalid.
                throw getNotFoundException(name, e);
            }
            throw e;
        }
    }

    /**
     * Return the name of the flow condition configured in the given
     * instance identifier.
     *
     * @param path  An {@link InstanceIdentifier} instance.
     * @return  The name of the flow condition if found.
     *          {@code null} if not found.
     */
    public static String getName(InstanceIdentifier<?> path) {
        VtnFlowConditionKey key = path.firstKeyOf(VtnFlowCondition.class);
        if (key == null) {
            return null;
        }

        VnodeName vname = key.getName();
        return (vname == null) ? null : vname.getValue();
    }

    /**
     * Ensure that there is no duplicate match index in the match list.
     *
     * @param set    A set of match indices.
     * @param index  An index to be tested.
     * @throws RpcException   An error occurred.
     */
    public static void verifyMatchIndex(Set<Integer> set, Integer index)
        throws RpcException {
        if (!set.add(index)) {
            String msg = "Duplicate match index: " + index;
            throw RpcException.getBadArgumentException(msg);
        }
    }

    /**
     * Verify the given index number for a flow match in a flow condition.
     *
     * @param index  A match index to be verified.
     * @throws RpcException
     *    The given match index is invalid.
     */
    public static void verifyMatchIndex(Integer index) throws RpcException {
        if (index == null) {
            throw getMatchIndexMissingException();
        }

        try {
            new VtnFlowMatchBuilder().setIndex(index);
        } catch (RuntimeException e) {
            String msg = "Invalid match index: " + index;
            RpcException re = RpcException.getBadArgumentException(msg);
            re.initCause(e);
            throw re;
        }
    }

    /**
     * Determine whether the given flow condition container is empty or not.
     *
     * @param root  A {@link VtnFlowConditions} instance.
     * @return  {@code true} only if the given flow condition container is
     *          empty.
     */
    public static boolean isEmpty(VtnFlowConditions root) {
        if (root == null) {
            return true;
        }

        List<VtnFlowCondition> vlist = root.getVtnFlowCondition();
        return (vlist == null || vlist.isEmpty());
    }

    /**
     * Determine whether the specified flow condition is present or not.
     *
     * @param rtx    A {@link ReadTransaction} instance associated with the
     *               read transaction for the MD-SAL datastore.
     * @param vname  A {@link VnodeName} instance that contains the name of the
     *               target flow condition.
     * @throws RpcException
     *    The specified flow condition is not present.
     * @throws VTNException
     *    Failed to read the MD-SAL datastore.
     */
    public static void checkPresent(ReadTransaction rtx, VnodeName vname)
        throws VTNException {
        InstanceIdentifier<VtnFlowCondition> path = getIdentifier(vname);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Optional<VtnFlowCondition> opt = DataStoreUtils.read(rtx, oper, path);
        if (!opt.isPresent()) {
            throw getNotFoundException(vname.getValue());
        }
    }

    /**
     * Read all the flow conditions from the MD-SAL datastore.
     *
     * @param rtx  A {@link ReadTransaction} instance.
     * @return  A list of {@link VTNFlowCondition} instances.
     * @throws VTNException  An error occurred.
     */
    public static List<VTNFlowCondition> readFlowConditions(ReadTransaction rtx)
        throws VTNException {
        InstanceIdentifier<VtnFlowConditions> path =
            InstanceIdentifier.create(VtnFlowConditions.class);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Optional<VtnFlowConditions> opt = DataStoreUtils.read(rtx, oper, path);
        List<VtnFlowCondition> vlist = null;
        if (opt.isPresent()) {
            vlist = opt.get().getVtnFlowCondition();
        }
        if (vlist == null || vlist.isEmpty()) {
            return Collections.<VTNFlowCondition>emptyList();
        }

        List<VTNFlowCondition> list = new ArrayList<>(vlist.size());
        for (VtnFlowCondition vfc: vlist) {
            list.add(new VTNFlowCondition(vfc));
        }

        return list;
    }

    /**
     * Read the flow condition specified by the given name from the MD-SAL
     * datastore.
     *
     * @param rtx   A {@link ReadTransaction} instance.
     * @param name  The name of the flow condition.
     * @return  A {@link VTNFlowCondition} instance.
     * @throws VTNException  An error occurred.
     */
    public static VTNFlowCondition readFlowCondition(ReadTransaction rtx,
                                                     String name)
        throws VTNException {
        InstanceIdentifier<VtnFlowCondition> path = getIdentifier(name);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Optional<VtnFlowCondition> opt = DataStoreUtils.read(rtx, oper, path);
        if (opt.isPresent()) {
            return new VTNFlowCondition(opt.get());
        }

        throw getNotFoundException(name);
    }

    /**
     * Read the flow match specified by the given index in the given flow
     * condition from the MD-SAL datastore.
     *
     * @param rtx   A {@link ReadTransaction} instance.
     * @param name  The name of the flow condition.
     * @param idx   The match index that specifies the flow match in the
     *              flow condition.
     * @return  A {@link VTNFlowMatch} instance.
     *          {@code null} is returned if no flow match is associated
     *          with the given match index in the flow condition.
     * @throws VTNException  An error occurred.
     */
    public static VTNFlowMatch readFlowMatch(ReadTransaction rtx, String name,
                                             int idx) throws VTNException {
        VnodeName vname = getVnodeName(name);
        InstanceIdentifier<VtnFlowMatch> path = getIdentifier(vname, idx);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Optional<VtnFlowMatch> opt = DataStoreUtils.read(rtx, oper, path);
        if (opt.isPresent()) {
            return new VTNFlowMatch(opt.get());
        }

        // Check to see if the flow condition is present.
        checkPresent(rtx, vname);

        return null;
    }
}
