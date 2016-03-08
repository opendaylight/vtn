/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow;

import static org.opendaylight.vtn.manager.internal.util.flow.action.VTNActionList.newInstructions;
import static org.opendaylight.vtn.manager.internal.util.flow.action.VTNOutputAction.newOutputActionCase;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Objects;

import javax.annotation.Nonnull;
import javax.annotation.Nullable;

import org.slf4j.Logger;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.inventory.NodeRpcWatcher;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VtnFlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.FlowIdSet;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.VtnFlows;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.flow.id.set.FlowIdList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.flow.id.set.FlowIdListKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.MatchFlows;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.MatchFlowsKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.NodeFlows;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.NodeFlowsKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.PortFlows;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.PortFlowsKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.SourceHostFlows;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.SourceHostFlowsKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlowKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.VtnFlowEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.flows.VtnFlowTable;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.flows.VtnFlowTableKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnFlowTimeoutConfig;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.ActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.AddFlowInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.AddFlowInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.FlowTableRef;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.Flow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.FlowCookie;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.FlowModFlags;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.FlowRef;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.OutputPortValues;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.Instructions;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.Match;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeRef;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.statistics.types.rev130925.duration.Duration;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Uri;

/**
 * {@code FlowUtils} class is a collection of utility class methods for
 * flow entry management.
 */
public final class FlowUtils {
    /**
     * Flow Table ID to install flow entries.
     */
    public static final short  TABLE_ID = 0;

    /**
     * Bits in a flow cookie that identifies flow entries installed by the
     * VTN Manager.
     */
    private static final long  COOKIE_BITS_VTN = 0x7f56000000000000L;

    /**
     * A bitmask that specifies cookie bits for VTN flows or a table miss
     * flow entries.
     */
    private static final long  COOKIE_MASKVAL_VTN = 0xffff000000000000L;

    /**
     * A fixed cookie value for a table miss flow entry.
     */
    private static final long  COOKIE_VALUE_MISS = 0x7f57ffffffffffffL;

    /**
     * A flow cookie that specifies cookie bits for VTN flows.
     */
    public static final FlowCookie  COOKIE_VTN =
        new FlowCookie(NumberUtils.getUnsigned(COOKIE_BITS_VTN));

    /**
     * A fixed flow cookie for a table miss flow entry.
     */
    public static final FlowCookie  COOKIE_MISS =
        new FlowCookie(NumberUtils.getUnsigned(COOKIE_VALUE_MISS));

    /**
     * A flow cookie mask that specifies all bits in the cookie.
     */
    public static final FlowCookie  COOKIE_MASK_ALL =
        new FlowCookie(NumberUtils.getUnsigned(-1L));

    /**
     * A flow cookie mask that specifies the {@link #COOKIE_MASKVAL_VTN} bits.
     */
    public static final FlowCookie  COOKIE_MASK_VTN =
        new FlowCookie(NumberUtils.getUnsigned(COOKIE_MASKVAL_VTN));

    /**
     * The minimum value of the VTN flow ID.
     */
    public static final long  MIN_FLOW_ID = 1L;

    /**
     * The maximum value of the VTN flow ID.
     */
    public static final long  MAX_FLOW_ID = ~COOKIE_MASKVAL_VTN;

    /**
     * A MD-SAL flow match that matches every packets.
     */
    public static final Match  EMPTY_MATCH = new MatchBuilder().build();

    /**
     * Flags for FLOW_MOD message that contains only SEND_FLOW_REM flag.
     */
    public static final FlowModFlags  FLOW_MOD_FLAGS =
        new FlowModFlagsBuilder().setSendFlowRem(true).build();

    /**
     * A bitmask that represents valid bits in a flow timeout value.
     */
    private static final int  FLOW_TIMEOUT_MASK = 0xffff;

    /**
     * A brief description about flow idle-timeout value.
     */
    private static final String  DESC_IDLE_TIMEOUT = "idle-timeout";

    /**
     * A brief description about flow hard-timeout value.
     */
    private static final String  DESC_HARD_TIMEOUT = "hard-timeout";

    /**
     * A prefix for a MD-SAL flow ID to be associated with a table miss flow
     * entry.
     */
    private static final String  ID_PREFIX_MISS = "vtn:table-miss:";

    /**
     * Private constructor that protects this class from instantiating.
     */
    private FlowUtils() {}

    /**
     * Verify the given flow timeout values.
     *
     * @param idle  The idle-timeout value.
     * @param hard  The hard-timeout value.
     * @throws RpcException  The given flow timeout values are invalid.
     */
    public static void verifyFlowTimeout(Integer idle, Integer hard)
        throws RpcException {
        verifyFlowTimeout(idle, hard, false);
    }

    /**
     * Verify the given flow timeout values.
     *
     * @param idle     The idle-timeout value.
     * @param hard     The hard-timeout value.
     * @param present  {@code true} means that the flow timeout configuration
     *                 must be present.
     * @throws RpcException  The given flow timeout values are invalid.
     */
    public static void verifyFlowTimeout(Integer idle, Integer hard,
                                         boolean present)
        throws RpcException {
        verifyFlowTimeout(idle, DESC_IDLE_TIMEOUT, present);
        verifyFlowTimeout(hard, DESC_HARD_TIMEOUT, present);

        if (hard == null) {
            if (idle != null) {
                String msg = new StringBuilder(DESC_HARD_TIMEOUT).
                    append(" must be specified.").toString();
                throw RpcException.getBadArgumentException(msg);
            }
        } else if (idle == null) {
            String msg = new StringBuilder(DESC_IDLE_TIMEOUT).
                append(" must be specified.").toString();
            throw RpcException.getBadArgumentException(msg);
        } else {
            int it = idle.intValue();
            int ht = hard.intValue();
            if (it != 0 && ht != 0 &&  it >= ht) {
                String msg = new StringBuilder(DESC_IDLE_TIMEOUT).
                    append(" must be less than ").append(DESC_HARD_TIMEOUT).
                    append('.').toString();
                throw RpcException.getBadArgumentException(msg);
            }
        }
    }

    /**
     * Verify timeout values in the given {@link VtnFlowTimeoutConfig}
     * instance.
     *
     * @param vftc  A {@link VtnFlowTimeoutConfig} instance.
     * @throws RpcException
     *    The given instance contains invalid timeout values.
     */
    public static void verifyFlowTimeout(VtnFlowTimeoutConfig vftc)
        throws RpcException {
        verifyFlowTimeout(vftc.getIdleTimeout(), vftc.getHardTimeout(), false);
    }

    /**
     * Verify the given flow timeout value.
     *
     * @param value    A value which indicates the flow timeout.
     * @param desc     A brief description about the given value.
     * @param present  {@code true} means that {@code value} must not be
     *                 {@code null}.
     * @throws RpcException  The given flow timeout value is invalid.
     */
    private static void verifyFlowTimeout(Integer value, String desc,
                                          boolean present)
        throws RpcException {
        if (value != null) {
            if ((value.intValue() & ~FLOW_TIMEOUT_MASK) != 0) {
                String msg = new StringBuilder("Invalid ").append(desc).
                    append(": ").append(value).toString();
                throw RpcException.getBadArgumentException(msg);
            }
        } else if (present) {
            throw RpcException.getNullArgumentException(desc);
        }
    }

    /**
     * Determine whether the given two {@link VtnFlowTimeoutConfig} instances
     * contain the same configuration or not.
     *
     * @param vftc1  The first instance to be compared.
     * @param vftc2  The second instance to be compared.
     * @return  {@code true} only if the given instances contains the same
     *          configuration.
     */
    public static boolean equalsFlowTimeoutConfig(
        VtnFlowTimeoutConfig vftc1, VtnFlowTimeoutConfig vftc2) {
        Integer idle1 = vftc1.getIdleTimeout();
        Integer idle2 = vftc2.getIdleTimeout();
        boolean ret;
        if (Objects.equals(idle1, idle2)) {
            ret = Objects.equals(vftc1.getHardTimeout(),
                                 vftc2.getHardTimeout());
        } else {
            ret = false;
        }

        return ret;
    }

    /**
     * Return the initial value of the VTN flow ID.
     *
     * @return  The initial value of the VTN flow ID.
     */
    public static VtnFlowId getInitialFlowId() {
        return new VtnFlowId(BigInteger.valueOf(MIN_FLOW_ID));
    }

    /**
     * Compare the given two {@link Duration} instances.
     *
     * @param d1  The first instance to be compared.
     * @param d2  The second instance to be compared.
     * @return  A negative integer, zero, or a positive integer as the first
     *          instance is less than, equal to, or greater than the second
     *          instance.
     */
    public static int compare(Duration d1, Duration d2) {
        long s1 = MiscUtils.longValue(d1.getSecond());
        long s2 = MiscUtils.longValue(d2.getSecond());
        int ret = Long.compare(s1, s2);
        if (ret == 0) {
            long n1 = MiscUtils.longValue(d1.getNanosecond());
            long n2 = MiscUtils.longValue(d2.getNanosecond());
            ret = Long.compare(n1, n2);
        }

        return ret;
    }

    /**
     * Create a flow cookie from the given VTN flow ID.
     *
     * @param fid  The identifier of the VTN data flow.
     * @return  A {@link FlowCookie} instance.
     */
    public static FlowCookie createCookie(VtnFlowId fid) {
        long value = fid.getValue().longValue() | COOKIE_BITS_VTN;
        return new FlowCookie(NumberUtils.getUnsigned(value));
    }

    /**
     * Return the VTN flow ID embedded in the given flow cookie.
     *
     * @param cookie  A {@link FlowCookie} instance.
     * @return  A {@link VtnFlowId} instance on success.
     *          {@code null} on failure.
     */
    public static VtnFlowId getVtnFlowId(FlowCookie cookie) {
        if (cookie == null) {
            return null;
        }

        long value = cookie.getValue().longValue();
        if ((value & COOKIE_MASKVAL_VTN) != COOKIE_BITS_VTN) {
            return null;
        }

        long id = (value & ~COOKIE_MASKVAL_VTN);
        return new VtnFlowId(NumberUtils.getUnsigned(id));
    }

    /**
     * Create the instance identifier for the VTN flow table.
     *
     * @param name  The name of the VTN.
     * @return  An {@link InstanceIdentifier} instance.
     */
    public static InstanceIdentifier<VtnFlowTable> getIdentifier(String name) {
        return getIdentifier(new VtnFlowTableKey(name));
    }

    /**
     * Create the instance identifier for the VTN flow table.
     *
     * @param key  A {@link VtnFlowTableKey} which contains the name of the
     *             target VTN.
     * @return  An {@link InstanceIdentifier} instance.
     */
    public static InstanceIdentifier<VtnFlowTable> getIdentifier(
        VtnFlowTableKey key) {
        return InstanceIdentifier.builder(VtnFlows.class).
            child(VtnFlowTable.class, key).
            build();
    }

    /**
     * Create the instance identifier for the specified data flow.
     *
     * @param name    The name of the VTN.
     * @param flowId  The flow identifier.
     * @return  An {@link InstanceIdentifier} instance.
     */
    public static InstanceIdentifier<VtnDataFlow> getIdentifier(
        String name, VtnFlowId flowId) {
        return getIdentifier(name, new VtnDataFlowKey(flowId));
    }

    /**
     * Create the instance identifier for the specified data flow.
     *
     * @param name     The name of the VTN.
     * @param flowKey  A {@link VtnDataFlowKey} instance which contains the
     *                 target flow ID.
     * @return  An {@link InstanceIdentifier} instance.
     */
    public static InstanceIdentifier<VtnDataFlow> getIdentifier(
        String name, VtnDataFlowKey flowKey) {
        return InstanceIdentifier.builder(VtnFlows.class).
            child(VtnFlowTable.class, new VtnFlowTableKey(name)).
            child(VtnDataFlow.class, flowKey).
            build();
    }

    /**
     * Create the instance identifier for the flow index for the given switch.
     *
     * @param name    The name of the VTN.
     * @param nodeId  A {@link NodeId} instance.
     * @return  An {@link InstanceIdentifier} instance.
     */
    public static InstanceIdentifier<NodeFlows> getIdentifier(
        String name, NodeId nodeId) {
        return getIdentifier(new VtnFlowTableKey(name),
                             new NodeFlowsKey(nodeId));
    }

    /**
     * Create the instance identifier for the flow index for the given switch.
     *
     * @param key      A {@link VtnFlowTableKey} which contains the name of the
     *                 target VTN.
     * @param nodeKey  A {@link NodeFlowsKey} instance which contains the
     *                 target node identifier.
     * @return  An {@link InstanceIdentifier} instance.
     */
    public static InstanceIdentifier<NodeFlows> getIdentifier(
        VtnFlowTableKey key, NodeFlowsKey nodeKey) {
        return InstanceIdentifier.builder(VtnFlows.class).
            child(VtnFlowTable.class, key).
            child(NodeFlows.class, nodeKey).
            build();
    }

    /**
     * Create the instance identifier for the flow index for the given switch
     * port.
     *
     * @param name    The name of the VTN.
     * @param portId  A {@link NodeConnectorId} instance.
     * @return  An {@link InstanceIdentifier} instance.
     */
    public static InstanceIdentifier<PortFlows> getIdentifier(
        String name, NodeConnectorId portId) {
        return getIdentifier(new VtnFlowTableKey(name),
                             new PortFlowsKey(portId));
    }

    /**
     * Create the instance identifier for the flow index for the given switch
     * port.
     *
     * @param key      A {@link VtnFlowTableKey} which contains the name of the
     *                 target VTN.
     * @param portKey  A {@link PortFlowsKey} instance which contains the
     *                 target port identifier.
     * @return  An {@link InstanceIdentifier} instance.
     */
    public static InstanceIdentifier<PortFlows> getIdentifier(
        VtnFlowTableKey key, PortFlowsKey portKey) {
        return InstanceIdentifier.builder(VtnFlows.class).
            child(VtnFlowTable.class, key).
            child(PortFlows.class, portKey).
            build();
    }

    /**
     * Create the instance identifier for the flow index for the given source
     * L2 host.
     *
     * @param name    The name of the VTN.
     * @param hostId  A {@link SourceHostFlowsKey} instance.
     * @return  An {@link InstanceIdentifier} instance.
     */
    public static InstanceIdentifier<SourceHostFlows> getIdentifier(
        String name, SourceHostFlowsKey hostId) {
        return InstanceIdentifier.builder(VtnFlows.class).
            child(VtnFlowTable.class, new VtnFlowTableKey(name)).
            child(SourceHostFlows.class, hostId).
            build();
    }

    /**
     * Create the instance identifier for the flow index for the given
     * flow match condition.
     *
     * @param name  The name of the VTN.
     * @param cond  A string which indicates the ingress flow match.
     * @return  An {@link InstanceIdentifier} instance.
     */
    public static InstanceIdentifier<MatchFlows> getIdentifier(
        String name, MatchFlowsKey cond) {
        return InstanceIdentifier.builder(VtnFlows.class).
            child(VtnFlowTable.class, new VtnFlowTableKey(name)).
            child(MatchFlows.class, cond).
            build();
    }

    /**
     * Create a MD-SAL transaction URI for the given flow entryu.
     *
     * @param vfent   A {@link VtnFlowEntry} instance.
     * @param prefix  Prefix of the URI.
     * @return  An {@link Uri} instance.
     */
    public static Uri createTxUri(VtnFlowEntry vfent, String prefix) {
        StringBuilder builder = new StringBuilder(prefix);
        BigInteger cookie = vfent.getCookie().getValue();
        builder.append(Long.toHexString(cookie.longValue())).
            append('-').append(vfent.getOrder());
        return new Uri(builder.toString());
    }

    /**
     * Construct an RPC input to install the specified flow entry.
     *
     * @param vfent  A {@link VtnFlowEntry} instance.
     * @return  A {@link AddFlowInput} instance.
     */
    public static AddFlowInput createAddFlowInput(VtnFlowEntry vfent) {
        SalNode snode = SalNode.create(vfent.getNode());
        Short table = vfent.getTableId();
        FlowTableRef tref =
            new FlowTableRef(snode.getFlowTableIdentifier(table));

        return new AddFlowInputBuilder((Flow)vfent).
            setNode(snode.getNodeRef()).
            setFlowTable(tref).
            setTransactionUri(createTxUri(vfent, "add-flow:")).
            setStrict(true).
            setBarrier(true).
            build();
    }

    /**
     * Create a MD-SAL flow ID to be associated with a table miss flow entry.
     *
     * @param snode  A {@link SalNode} instance that specifies the target
     *               switch.
     * @return  A MD-SAL flow ID.
     */
    public static String createTableMissFlowId(SalNode snode) {
        return ID_PREFIX_MISS + snode;
    }

    /**
     * Construct an RPC input to install a table miss flow entry for the
     * specified switch.
     *
     * @param snode   A {@link SalNode} instance which specifies the target
     *                switch.
     * @return  An {@link AddFlowInput} instance.
     */
    public static AddFlowInput createTableMissInput(SalNode snode) {
        // Create an instruction that tosses packets to the controller.
        Uri port = new Uri(OutputPortValues.CONTROLLER.toString());
        List<Action> actions = Collections.singletonList(
            new ActionBuilder().setAction(newOutputActionCase(port)).
            setOrder(MiscUtils.ORDER_MIN).build());
        Instructions inst = newInstructions(actions);

        // Associate a unique MD-SAL flow ID for a table miss flow entry.
        Short table = TABLE_ID;
        FlowId fid = new FlowId(createTableMissFlowId(snode));
        FlowRef fref = new FlowRef(snode.getFlowIdentifier(table, fid));

        // Create an add-flow input that tosses unmatched packets to the
        // controller.
        FlowTableRef tref =
            new FlowTableRef(snode.getFlowTableIdentifier(table));
        return new AddFlowInputBuilder().
            setNode(snode.getNodeRef()).
            setFlowRef(fref).
            setFlowTable(tref).
            setTransactionUri(new Uri("table-miss:" + table)).
            setCookie(COOKIE_MISS).
            setPriority(0).
            setTableId(table).
            setIdleTimeout(0).
            setHardTimeout(0).
            setMatch(EMPTY_MATCH).
            setFlags(FLOW_MOD_FLAGS).
            setInstructions(inst).
            setBarrier(true).
            build();
    }

    /**
     * Construct an RPC input builder to uninstall all the VTN flow entries
     * from the given switch.
     *
     * <p>
     *   Note that the caller must ensure that the specified switch is present
     *   and supports OF 1.3 protocol.
     * </p>
     *
     * @param snode   A {@link SalNode} instance which specifies the target
     *                switch.
     * @return  A {@link RemoveFlowInputBuilder} instance.
     *          Note that this method never sets barrier flag into the returned
     *          input builder.
     */
    @Nonnull
    public static RemoveFlowInputBuilder createRemoveFlowInputBuilder(
        SalNode snode) {
        Short table = Short.valueOf(TABLE_ID);
        FlowTableRef tref =
            new FlowTableRef(snode.getFlowTableIdentifier(table));

        return new RemoveFlowInputBuilder().
            setNode(snode.getNodeRef()).
            setTableId(table).
            setFlowTable(tref).
            setTransactionUri(new Uri("remove-flow:all")).
            setMatch(EMPTY_MATCH).
            setCookie(COOKIE_VTN).
            setCookieMask(COOKIE_MASK_VTN).
            setStrict(false);
    }

    /**
     * Construct an RPC input builder to uninstall the specified flow entry.
     *
     * @param snode   A {@link SalNode} instance which specifies the target
     *                switch.
     * @param vfent   A {@link VtnFlowEntry} instance.
     * @param reader  A {@link InventoryReader} instance.
     * @return  A {@link RemoveFlowInputBuilder} instance on success.
     *          {@code null} if the target node is not present.
     *          Note that this method never sets barrier flag into the returned
     *          input builder.
     * @throws VTNException  An error occurred.
     */
    @Nullable
    public static RemoveFlowInputBuilder createRemoveFlowInputBuilder(
        SalNode snode, VtnFlowEntry vfent, InventoryReader reader)
        throws VTNException {
        return (reader.get(snode) == null)
            ? null
            : createRemoveFlowInputBuilder(snode, vfent);
    }

    /**
     * Construct an RPC input builder to uninstall the specified flow entry.
     *
     * <p>
     *   Note that the caller must ensure that the specified switch is present.
     * </p>
     *
     * @param snode   A {@link SalNode} instance which specifies the target
     *                switch.
     * @param vfent   A {@link VtnFlowEntry} instance.
     * @return  A {@link RemoveFlowInputBuilder} instance.
     *          Note that this method never sets barrier flag into the returned
     *          input builder.
     */
    @Nonnull
    public static RemoveFlowInputBuilder createRemoveFlowInputBuilder(
        SalNode snode, VtnFlowEntry vfent) {
        Uri uri = createTxUri(vfent, "remove-flow:");
        return createRemoveFlowInputBuilder(snode, vfent, uri);
    }

    /**
     * Construct an RPC input builder to uninstall the specified MD-SAL flow
     * entry.
     *
     * <p>
     *   Note that the caller must ensure that the specified switch is present.
     * </p>
     *
     * @param snode  A {@link SalNode} instance which specifies the target
     *               switch.
     * @param flow   A {@link Flow} instance.
     * @param uri    A {@link Uri} to be assigned to the input.
     * @return  A {@link RemoveFlowInputBuilder} instance.
     *          Note that this method never sets barrier flag into the returned
     *          input builder.
     */
    @Nonnull
    public static RemoveFlowInputBuilder createRemoveFlowInputBuilder(
        SalNode snode, Flow flow, Uri uri) {
        Short table = flow.getTableId();
        FlowTableRef tref =
            new FlowTableRef(snode.getFlowTableIdentifier(table));

        RemoveFlowInputBuilder builder = new RemoveFlowInputBuilder(flow).
            setNode(snode.getNodeRef()).
            setFlowTable(tref).
            setTransactionUri(uri).
            setCookieMask(COOKIE_MASK_ALL).
            setStrict(true);

        if (builder.getMatch() == null) {
            // openflowplugin-li does not allow null match.
            builder.setMatch(EMPTY_MATCH);
        }

        return builder;
    }

    /**
     * Construct an RPC input builder to uninstall flow entries related to the
     * given switch port.
     *
     * <p>
     *   This method returns a bulk flow remove request to uninstall the
     *   following flow entries.
     * </p>
     * <ul>
     *   <li>
     *     Flow entries that match packets received from the given port.
     *   </li>
     *   <li>
     *     Flow entries that transmit packets to the given port.
     *   </li>
     * </ul>
     *
     * <p>
     *   Note that the caller must ensure that the specified switch is present.
     * </p>
     *
     * @param snode   A {@link SalNode} instance which specifies the target
     *                switch.
     * @param sport   A {@link SalPort} instance which specifies the switch
     *                port.
     * @return  A list of {@link RemoveFlowInputBuilder} instances.
     *          Note that this method never sets barrier flag into the returned
     *          input builder.
     */
    @Nonnull
    public static List<RemoveFlowInputBuilder> createRemoveFlowInputBuilder(
        SalNode snode, SalPort sport) {
        assert snode.getNodeNumber() == sport.getNodeNumber();
        StringBuilder ub = new StringBuilder("remove-flow:IN_PORT=").
            append(sport);
        Short table = Short.valueOf(TABLE_ID);
        FlowTableRef tref =
            new FlowTableRef(snode.getFlowTableIdentifier(table));
        NodeRef nref = snode.getNodeRef();

        Match match = new MatchBuilder().
            setInPort(sport.getNodeConnectorId()).
            build();

        List<RemoveFlowInputBuilder> list = new ArrayList<>();
        RemoveFlowInputBuilder ingress = new RemoveFlowInputBuilder().
            setNode(nref).
            setTableId(table).
            setFlowTable(tref).
            setTransactionUri(new Uri(ub.toString())).
            setMatch(match).
            setStrict(false);
        list.add(ingress);

        ub = new StringBuilder("remove-flow:OUT_PORT=").append(sport);
        RemoveFlowInputBuilder egress = new RemoveFlowInputBuilder().
            setNode(nref).
            setTableId(table).
            setFlowTable(tref).
            setTransactionUri(new Uri(ub.toString())).
            setMatch(EMPTY_MATCH).
            setOutPort(NumberUtils.getUnsigned(sport.getPortNumber())).
            setStrict(false);
        list.add(egress);

        return list;
    }

    /**
     * Remove the given flow ID set if it is empty.
     *
     * @param tx    A {@link ReadWriteTransaction} instance.
     * @param path  Path to the flow ID set.
     * @param <T>   The type of the flow ID set.
     * @return  {@code true} if the specified flow ID set was removed.
     *          Otherwise {@code false}.
     * @throws VTNException  An error occurred.
     */
    public static <T extends FlowIdSet> boolean removeFlowIdSet(
        ReadWriteTransaction tx, InstanceIdentifier<T> path)
        throws VTNException {
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Optional<T> opt = DataStoreUtils.read(tx, oper, path);
        boolean removed = false;
        if (opt.isPresent()) {
            List<FlowIdList> list = opt.get().getFlowIdList();
            if (list == null || list.isEmpty()) {
                tx.delete(oper, path);
                removed = true;
            }
        }

        return removed;
    }

    /**
     * Remove the given data flow from all flow indices in the MD-SAL
     * datastore.
     *
     * @param tx     A {@link ReadWriteTransaction} instance.
     * @param path   Path to the VTN flow table in the MD-SAL datastore.
     * @param df     A {@link VTNDataFlow} instance.
     * @throws VTNException  An error occurred.
     */
    public static void removeIndex(ReadWriteTransaction tx,
                                   InstanceIdentifier<VtnFlowTable> path,
                                   VTNDataFlow df) throws VTNException {
        FlowIdListKey idKey = new FlowIdListKey(df.getKey().getFlowId());
        removeIndex(tx, path, df, idKey);
    }

    /**
     * Remove the given data flow from all flow indices in the MD-SAL
     * datastore.
     *
     * @param tx     A {@link ReadWriteTransaction} instance.
     * @param path   Path to the VTN flow table in the MD-SAL datastore.
     * @param df     A {@link VTNDataFlow} instance.
     * @param idKey  A {@link FlowIdListKey} instance which contains the
     *               data flow identifier.
     * @throws VTNException  An error occurred.
     */
    public static void removeIndex(ReadWriteTransaction tx,
                                   InstanceIdentifier<VtnFlowTable> path,
                                   VTNDataFlow df, FlowIdListKey idKey)
        throws VTNException {
        // Remove the given data flow from match-flows.
        String condKey = df.getIngressMatchKey();
        MatchFlowsKey mkey = new MatchFlowsKey(condKey);
        InstanceIdentifier<MatchFlows> mpath =
            path.child(MatchFlows.class, mkey);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        DataStoreUtils.delete(tx, oper, mpath);

        // Remove the given data flow from node-flows.
        for (SalNode snode: df.getFlowNodes()) {
            InstanceIdentifier<FlowIdList> idxPath = path.builder().
                child(NodeFlows.class, new NodeFlowsKey(snode.getNodeId())).
                child(FlowIdList.class, idKey).
                build();
            if (DataStoreUtils.delete(tx, oper, idxPath) != null) {
                // Remove node-flows if empty.
                InstanceIdentifier<NodeFlows> lpath =
                    idxPath.firstIdentifierOf(NodeFlows.class);
                removeFlowIdSet(tx, lpath);
            }
        }

        // Remove the given data flow from port-flows.
        for (SalPort sport: df.getFlowPorts()) {
            InstanceIdentifier<FlowIdList> idxPath = path.builder().
                child(PortFlows.class,
                      new PortFlowsKey(sport.getNodeConnectorId())).
                child(FlowIdList.class, idKey).
                build();
            if (DataStoreUtils.delete(tx, oper, idxPath) != null) {
                // Remove port-flows if empty.
                InstanceIdentifier<PortFlows> lpath =
                    idxPath.firstIdentifierOf(PortFlows.class);
                removeFlowIdSet(tx, lpath);
            }
        }

        // Remove the given data flow from source-host-from.
        SourceHostFlowsKey srcHost = df.getSourceHostFlowsKey();
        if (srcHost != null) {
            InstanceIdentifier<FlowIdList> idxPath = path.builder().
                child(SourceHostFlows.class, srcHost).
                child(FlowIdList.class, idKey).
                build();
            if (DataStoreUtils.delete(tx, oper, idxPath) != null) {
                // Remove source-host-flows if empty.
                InstanceIdentifier<SourceHostFlows> lpath =
                    idxPath.firstIdentifierOf(SourceHostFlows.class);
                removeFlowIdSet(tx, lpath);
            }
        }
    }

    /**
     * Remove the given data flow from the MD-SAL datastore.
     *
     * @param tx     A {@link ReadWriteTransaction} instance.
     * @param tname  The name of the VTN.
     * @param df     A {@link VTNDataFlow} instance.
     * @return  {@code true} if the specified data flow was removed.
     *          {@code false} if the specified data flow is not present.
     * @throws VTNException  An error occurred.
     */
    public static boolean removeDataFlow(ReadWriteTransaction tx, String tname,
                                         VTNDataFlow df) throws VTNException {
        // Ensure that the target flow is still present.
        VtnDataFlowKey key = df.getKey();
        InstanceIdentifier<VtnFlowTable> tpath = getIdentifier(tname);
        InstanceIdentifier<VtnDataFlow> fpath =
            tpath.child(VtnDataFlow.class, key);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Optional<VtnDataFlow> opt = DataStoreUtils.read(tx, oper, fpath);
        boolean present = opt.isPresent();
        if (present) {
            tx.delete(oper, fpath);

            // Clean up indices.
            removeIndex(tx, tpath, df);
        }

        return present;
    }

    /**
     * Remove all flow entries in the given VTN data flows.
     *
     * @param watcher  The node-routed RPC watcher.
     * @param sfs      MD-SAL flow service.
     * @param flows    A list of {@link FlowCache} instances.
     * @param reader   A {@link InventoryReader} instance.
     * @return  A {@link RemoveFlowRpcList} instance.
     * @throws VTNException  An error occurred.
     */
    public static RemoveFlowRpcList removeFlowEntries(
        NodeRpcWatcher watcher, SalFlowService sfs, List<FlowCache> flows,
        InventoryReader reader) throws VTNException {
        RemoveFlowRpcList rpcs = new RemoveFlowRpcList(watcher, sfs);
        for (FlowCache fc: flows) {
            for (VtnFlowEntry vfent: fc.getFlowEntries()) {
                SalNode snode = SalNode.create(vfent.getNode());
                RemoveFlowInputBuilder builder =
                    createRemoveFlowInputBuilder(snode, vfent, reader);
                if (builder != null) {
                    rpcs.invoke(builder);
                }
            }
        }

        return rpcs.flush();
    }

    /**
     * Remove all flow entries in the given VTN data flows.
     *
     * <p>
     *   Flow entries related to the given switch port will be unsinstalled
     *   by a bulk flow remove request.
     * </p>
     *
     * @param watcher  The node-routed RPC watcher.
     * @param sfs      MD-SAL flow service.
     * @param flows    A list of {@link FlowCache} instances.
     * @param sport    A {@link SalPort} instance which specifies the switch
     *                 port.
     * @param reader   A {@link InventoryReader} instance.
     * @return  A {@link RemoveFlowRpcList} instance.
     * @throws VTNException  An error occurred.
     */
    public static RemoveFlowRpcList removeFlowEntries(
        NodeRpcWatcher watcher, SalFlowService sfs, List<FlowCache> flows,
        SalPort sport, InventoryReader reader) throws VTNException {
        RemoveFlowRpcList rpcs = new RemoveFlowRpcList(watcher, sfs);
        long dpid = sport.getNodeNumber();
        boolean done = false;

        for (FlowCache fc: flows) {
            for (VtnFlowEntry vfent: fc.getFlowEntries()) {
                SalNode snode = SalNode.create(vfent.getNode());
                if (reader.get(snode) == null) {
                    continue;
                }

                if (snode.getNodeNumber() != dpid) {
                    RemoveFlowInputBuilder builder =
                        createRemoveFlowInputBuilder(snode, vfent);
                    rpcs.invoke(builder);
                } else if (!done) {
                    for (RemoveFlowInputBuilder builder:
                             createRemoveFlowInputBuilder(snode, sport)) {
                        rpcs.invoke(builder);
                    }
                    done = true;
                }
            }
        }

        return rpcs.flush();
    }

    /**
     * Return a list of all existing VTN flow tables.
     *
     * @param tx  A {@link ReadTransaction} instance.
     * @return  A list of VTN flow tables.
     * @throws VTNException  An error occurred.
     */
    public static List<VtnFlowTable> getFlowTables(ReadTransaction tx)
        throws VTNException {
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        InstanceIdentifier<VtnFlows> path =
            InstanceIdentifier.create(VtnFlows.class);
        Optional<VtnFlows> opt = DataStoreUtils.read(tx, oper, path);
        if (opt.isPresent()) {
            List<VtnFlowTable> tables = opt.get().getVtnFlowTable();
            if (tables != null) {
                return tables;
            }
        }

        return Collections.<VtnFlowTable>emptyList();
    }

    /**
     * Record a debug log message which indicates the specified data flow
     * has been removed.
     *
     * @param logger  A logger instance.
     * @param desc    A brief description about flow remover.
     * @param fc      A {@link FlowCache} instance which contains the removed
     *                data flow.
     */
    public static void removedLog(Logger logger, String desc, FlowCache fc) {
        BigInteger flowId = fc.getDataFlow().getFlowId().getValue();
        logger.debug("VTN data flow has been removed: remover={}, id={}",
                     desc, flowId);
    }
}
