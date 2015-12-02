/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.pathmap;

import java.util.Collections;
import java.util.List;
import java.util.Set;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.VtnIndexComparator;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;
import org.opendaylight.vtn.manager.internal.util.flow.cond.FlowCondUtils;
import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantIdentifier;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.GlobalPathMaps;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.VtnPathMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.VtnPathMapList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMapBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMapKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtnPathMaps;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnFlowTimeoutConfig;

/**
 * {@code PathMapUtils} class is a collection of utility class methods
 * for path map.
 */
public final class PathMapUtils {
    /**
     * Private constructor that protects this class from instantiating.
     */
    private PathMapUtils() {}

    /**
     * Return a new {@link RpcException} that indicates the path map ID is
     * missing.
     *
     * @return  An {@link RpcException}.
     */
    public static RpcException getNullMapIndexException() {
        return RpcException.getNullArgumentException("Path map index");
    }

    /**
     * Return a new {@link RpcException} that indicates the given path map
     * index is invalid.
     *
     * @param index  The path map index.
     * @param cause  A throwable that indicates the cause of error.
     * @return  An {@link RpcException}.
     */
    public static RpcException getInvalidMapIndexException(
        Integer index, Throwable cause) {
        String msg = MiscUtils.joinColon("Invalid path map index", index);
        return RpcException.getBadArgumentException(msg, cause);
    }

    /**
     * Convert the given {@link VtnPathMapConfig} instance into a
     * {@link VtnPathMapBuilder} instance.
     *
     * <p>
     *   If the given instance does not contain the path policy ID, this
     *   method set zero as the path policy ID.
     * </p>
     *
     * @param vpmc  A {@link VtnPathMapConfig} instance.
     * @return  A {@link VtnPathMapBuilder} instance that contains values
     *          in the given {@link VtnPathMapConfig} instance.
     * @throws RpcException
     *    The given {@link VtnPathMapConfig} instance contains invalid data.
     */
    public static VtnPathMapBuilder toVtnPathMapBuilder(VtnPathMapConfig vpmc)
        throws RpcException {
        if (vpmc == null) {
            throw RpcException.getNullArgumentException(
                "Path map configuration");
        }

        VtnPathMapBuilder builder =
            new VtnPathMapBuilder((VtnFlowTimeoutConfig)vpmc);
        setIndex(builder, vpmc.getIndex());
        setCondition(builder, vpmc.getCondition());
        Integer policy = vpmc.getPolicy();
        if (policy == null) {
            // Use the system default routing policy.
            policy = Integer.valueOf(PathPolicyUtils.DEFAULT_POLICY);
        }
        setPolicy(builder, policy);
        FlowUtils.verifyFlowTimeout(vpmc);

        return builder;
    }

    /**
     * Ensure that there is not duplicate map index in the path map list.
     *
     * @param set    A set of map indices.
     * @param index  An index to be tested.
     * @throws RpcException  An error occurred.
     */
    public static void verifyMapIndex(Set<Integer> set, Integer index)
        throws RpcException {
        if (!set.add(index)) {
            String msg = "Duplicate map index: " + index;
            throw RpcException.getBadArgumentException(msg);
        }
    }

    /**
     * Ensure that the given map index is valid.
     *
     * @param index  An index to be tested.
     * @throws RpcException
     *    The given map index is invalid.
     */
    public static void verifyMapIndex(Integer index) throws RpcException {
        setIndex(new VtnPathMapBuilder(), index);
    }

    /**
     * Return the path map index in the given instance identifier.
     *
     * @param path  An {@link InstanceIdentifier} instance.
     * @return  The index of the path map if found.
     *          {@code null} if not found.
     */
    public static Integer getIndex(InstanceIdentifier<?> path) {
        VtnPathMapKey key = path.firstKeyOf(VtnPathMap.class);
        return (key == null) ? null : key.getIndex();
    }

    /**
     * Create the instance identifier for the global path map specified by the
     * given map index.
     *
     * @param index  The index assigned to the global path map.
     * @return  An {@link InstanceIdentifier} instance.
     * @throws RpcException
     *    The given map index is invalid.
     */
    public static InstanceIdentifier<VtnPathMap> getIdentifier(Integer index)
        throws RpcException {
        if (index == null) {
            throw getNullMapIndexException();
        }
        return InstanceIdentifier.builder(GlobalPathMaps.class).
            child(VtnPathMap.class, new VtnPathMapKey(index)).build();
    }

    /**
     * Create the instance identifier for the VTN path map specified by the
     * given VTN name and map index.
     *
     * <p>
     *   This method is used to retrieve path map in existing VTN.
     * </p>
     *
     * @param name   The name of the VTN.
     * @param index  The index assigned to the path map in the specified VTN.
     * @return  An {@link InstanceIdentifier} instance.
     * @throws RpcException
     *    The given VTN name or map index is invalid.
     */
    public static InstanceIdentifier<VtnPathMap> getIdentifier(
        String name, Integer index) throws RpcException {
        return getIdentifier(VTenantIdentifier.create(name, true), index);
    }

    /**
     * Create the instance identifier for the VTN path map specified by the
     * given VTN identifier and map index.
     *
     * @param ident  The identifier for the target VTN.
     * @param index  The index assigned to the path map in the specified VTN.
     * @return  An {@link InstanceIdentifier} instance.
     * @throws RpcException
     *    The given map index is invalid.
     */
    public static InstanceIdentifier<VtnPathMap> getIdentifier(
        VTenantIdentifier ident, Integer index) throws RpcException {
        if (index == null) {
            throw getNullMapIndexException();
        }
        return ident.getIdentifierBuilder().
            child(VtnPathMaps.class).
            child(VtnPathMap.class, new VtnPathMapKey(index)).build();
    }

    /**
     * Determine whether the given path map container is empty or not.
     *
     * @param vplist  A {@link VtnPathMapList} instance.
     * @return  {@code true} only if the given path map container is empty.
     */
    public static boolean isEmpty(VtnPathMapList vplist) {
        if (vplist == null) {
            return true;
        }

        List<VtnPathMap> list = vplist.getVtnPathMap();
        return (list == null || list.isEmpty());
    }

    /**
     * Read the path maps in the global path map list.
     *
     * @param rtx  A {@link ReadTransaction} instance associated with the
     *             read transaction for the MD-SAL datastore.
     * @return  A list of all global path maps.
     *          Each element in the list is sorted by index in ascending order.
     *          An empty list is returned if no global path map is configured.
     * @throws VTNException  An error occurred.
     */
    public static List<VtnPathMap> readPathMaps(ReadTransaction rtx)
        throws VTNException {
        InstanceIdentifier<GlobalPathMaps> path =
            InstanceIdentifier.create(GlobalPathMaps.class);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Optional<GlobalPathMaps> opt = DataStoreUtils.read(rtx, oper, path);
        if (opt.isPresent()) {
            GlobalPathMaps maps = opt.get();
            List<VtnPathMap> list = maps.getVtnPathMap();
            if (list != null && !list.isEmpty()) {
                return MiscUtils.sortedCopy(list, new VtnIndexComparator());
            }
        }

        return Collections.<VtnPathMap>emptyList();
    }

    /**
     * Read the path map associated with the given index in the global path
     * map list.
     *
     * @param rtx    A {@link ReadTransaction} instance associated with the
     *               read transaction for the MD-SAL datastore.
     * @param index  The index that specifies the path map.
     * @return  A {@link VtnPathMap} instance if found.
     *          {@code null} if not found.
     * @throws VTNException  An error occurred.
     */
    public static VtnPathMap readPathMap(ReadTransaction rtx, Integer index)
        throws VTNException {
        InstanceIdentifier<VtnPathMap> path = getIdentifier(index);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        return DataStoreUtils.read(rtx, oper, path).orNull();
    }

    /**
     * Read the VTN path maps in the given VTN.
     *
     * @param rtx    A {@link ReadTransaction} instance associated with the
     *               read transaction for the MD-SAL datastore.
     * @param ident  The identifier for the target VTN.
     * @return  A list of all VTN path maps in the given VTN.
     *          Each element in the list is sorted by index in ascending order.
     *          An empty list is returned if no VTN path map is configured.
     * @throws VTNException  An error occurred.
     */
    public static List<VtnPathMap> readPathMaps(
        ReadTransaction rtx, VTenantIdentifier ident) throws VTNException {
        InstanceIdentifier<VtnPathMaps> path = ident.getIdentifierBuilder().
            child(VtnPathMaps.class).build();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Optional<VtnPathMaps> opt = DataStoreUtils.read(rtx, oper, path);
        if (opt.isPresent()) {
            VtnPathMaps maps = opt.get();
            List<VtnPathMap> list = maps.getVtnPathMap();
            if (list != null && !list.isEmpty()) {
                return MiscUtils.sortedCopy(list, new VtnIndexComparator());
            }
        } else {
            // Check to see if the target VTN is present.
            ident.fetch(rtx);
        }

        return Collections.<VtnPathMap>emptyList();
    }

    /**
     * Read the path map associated with the given index in the given VTN.
     *
     * @param rtx    A {@link ReadTransaction} instance associated with the
     *               read transaction for the MD-SAL datastore.
     * @param ident  The identifier for the target VTN.
     * @param index  The index that specifies the path map.
     * @return  A {@link VtnPathMap} instance if found.
     *          {@code null} if not found.
     * @throws VTNException  An error occurred.
     */
    public static VtnPathMap readPathMap(
        ReadTransaction rtx, VTenantIdentifier ident, Integer index)
        throws VTNException {
        InstanceIdentifier<VtnPathMap> path = getIdentifier(ident, index);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Optional<VtnPathMap> opt = DataStoreUtils.read(rtx, oper, path);
        VtnPathMap vpm;
        if (opt.isPresent()) {
            vpm = opt.get();
        } else {
            // Check to see if the target VTN is present.
            ident.fetch(rtx);
            vpm = null;
        }

        return vpm;
    }

    /**
     * Set the map index into the given {@link VtnPathMapBuilder} instance.
     *
     * @param builder  A {@link VtnPathMapBuilder} instance.
     * @param index    A map index to be set.
     * @throws RpcException  The given map index is invalid.
     */
    private static void setIndex(VtnPathMapBuilder builder, Integer index)
        throws RpcException {
        if (index == null) {
            throw getNullMapIndexException();
        }
        try {
            builder.setIndex(index);
        } catch (RuntimeException e) {
            throw getInvalidMapIndexException(index, e);
        }
    }

    /**
     * Set the flow condition name into the given {@link VtnPathMapBuilder}
     * instance.
     *
     * @param builder  A {@link VtnPathMapBuilder} instance.
     * @param vname    A {@link VnodeName} instance that contains the name of
     *                 the flow condition.
     * @throws RpcException  The given flow condition name is invalid.
     */
    private static void setCondition(VtnPathMapBuilder builder,
                                     VnodeName vname) throws RpcException {
        FlowCondUtils.checkName(vname);
        builder.setCondition(vname);
    }

    /**
     * Set the path policy ID into the given {@link VtnPathMapBuilder}
     * instance.
     *
     * @param builder  A {@link VtnPathMapBuilder} instance.
     * @param policy   An {@link Integer} instance which represents the path
     *                 policy ID.
     * @throws RpcException  The given path policy ID is invalid.
     */
    private static void setPolicy(VtnPathMapBuilder builder, Integer policy)
        throws RpcException {
        try {
            builder.setPolicy(policy);
        } catch (RuntimeException e) {
            throw PathPolicyUtils.getInvalidPolicyIdException(policy, e);
        }
    }
}
