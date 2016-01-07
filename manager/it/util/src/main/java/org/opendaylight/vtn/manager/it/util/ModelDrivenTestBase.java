/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util;

import static org.opendaylight.vtn.manager.it.ofmock.OfMockService.ID_OPENFLOW;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.ListIterator;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;

import com.google.common.base.Optional;
import com.google.common.collect.ImmutableList;

import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.EtherTypes;

import org.opendaylight.vtn.manager.it.ofmock.DataStoreUtils;

import org.opendaylight.controller.md.sal.binding.api.ReadOnlyTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.yangtools.yang.common.RpcError;
import org.opendaylight.yangtools.yang.common.RpcResult;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePath;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePathBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.Vtns;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.VtnStaticTopology;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticEdgePorts;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static.edge.ports.StaticEdgePort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static.edge.ports.StaticEdgePortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static.edge.ports.StaticEdgePortKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeUpdateMode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnRpcResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.DropActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.OutputActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.PopVlanActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.PushVlanActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetVlanIdActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.drop.action._case.DropAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.output.action._case.OutputAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.pop.vlan.action._case.PopVlanAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.push.vlan.action._case.PushVlanAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.vlan.id.action._case.SetVlanIdAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.Instructions;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.Match;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.instruction.instruction.ApplyActionsCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.instruction.instruction.apply.actions._case.ApplyActions;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.instruction.list.Instruction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.ethernet.match.fields.EthernetDestination;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.ethernet.match.fields.EthernetDestinationBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.ethernet.match.fields.EthernetSource;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.ethernet.match.fields.EthernetSourceBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.ethernet.match.fields.EthernetTypeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.EthernetMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.EthernetMatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.VlanMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.VlanMatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.vlan.match.fields.VlanIdBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.EtherType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanPcp;

/**
 * Abstract base class for integration tests using JUnit.
 *
 * <p>
 *   This class provides utilities for MD-SAL.
 * </p>
 */
public abstract class ModelDrivenTestBase extends TestBase {
    /**
     * The expected number of flow instructions.
     */
    private static final int  FLOW_INST_SIZE = 1;

    /**
     * A list of invalid node-id.
     */
    public static final List<String>  INVALID_NODE_IDS;

    /**
     * A list of invalid port identifiers.
     */
    public static final List<String>  INVALID_PORT_IDS;

    /**
     * A list of update operations that indicate request for modifying the
     * target data.
     */
    public static final List<VtnUpdateOperationType>  MODIFY_OPERATIONS;

    /**
     * A list of vnode update modes that indicate request for modifying the
     * target data.
     */
    public static final List<VnodeUpdateMode>  VNODE_UPDATE_MODES;

    /**
     * Initialize static field.
     */
    static {
        INVALID_NODE_IDS = ImmutableList.<String>builder().
            add("").
            add(ID_OPENFLOW + "-1").
            add(ID_OPENFLOW + "18446744073709551616").
            add(ID_OPENFLOW + "bad dpid").
            add("unknown:1").
            build();

        INVALID_PORT_IDS = ImmutableList.<String>builder().
            add("").
            add("LOCAL").
            add("bad port ID").
            add("4294967041").
            add("-1").
            build();

        // ImmutableList cannot contain null.
        List<VtnUpdateOperationType> opTypes = Arrays.asList(
            VtnUpdateOperationType.ADD, VtnUpdateOperationType.SET, null);
        MODIFY_OPERATIONS = Collections.unmodifiableList(opTypes);

        List<VnodeUpdateMode> modes = Arrays.asList(
            VnodeUpdateMode.UPDATE, VnodeUpdateMode.MODIFY, null);
        VNODE_UPDATE_MODES = Collections.unmodifiableList(modes);
    }

    /**
     * Create ethernet match builder.
     *
     * @param src  The source MAC address.
     * @param dst  The destination MAC address.
     * @return  A {@link EthernetMatchBuilder} instance.
     */
    public static final EthernetMatchBuilder createEthernetMatch(
        EtherAddress src, EtherAddress dst) {
        EthernetMatchBuilder builder = new EthernetMatchBuilder();
        if (src != null) {
            EthernetSourceBuilder sb = new EthernetSourceBuilder().
                setAddress(src.getMacAddress());
            builder.setEthernetSource(sb.build());
        }
        if (dst != null) {
            EthernetDestinationBuilder db = new EthernetDestinationBuilder().
                setAddress(dst.getMacAddress());
            builder.setEthernetDestination(db.build());
        }

        return builder;
    }

    /**
     * Create ethernet match builder.
     *
     * @param src   The source MAC address.
     * @param dst   The destination MAC address.
     * @param type  The ethernet type. Zero means an undefined type.
     * @return  A {@link EthernetMatchBuilder} instance.
     */
    public static final EthernetMatchBuilder createEthernetMatch(
        EtherAddress src, EtherAddress dst, int type) {
        EthernetMatchBuilder builder = createEthernetMatch(src, dst);
        if (type != 0) {
            EthernetTypeBuilder etb = new EthernetTypeBuilder().
                setType(new EtherType(Long.valueOf((long)type)));
            builder.setEthernetType(etb.build());
        }

        return builder;
    }

    /**
     * Create VLAN match builder.
     *
     * @param vid  VLAN ID. Zero means untagged ethernet frame.
     * @return  A {@link VlanMatchBuilder} instance.
     */
    public static final VlanMatchBuilder createVlanMatch(int vid) {
        return createVlanMatch(vid, null);
    }

    /**
     * Create VLAN match builder.
     *
     * @param vid  VLAN ID. Zero means untagged ethernet frame.
     * @param pcp  VLAN priority. {@code null} means undefined.
     * @return  A {@link VlanMatchBuilder} instance.
     */
    public static final VlanMatchBuilder createVlanMatch(int vid, Short pcp) {
        VlanIdBuilder ib = new VlanIdBuilder();
        boolean present = (vid != 0);
        VlanId vlanId = new VlanId(vid);
        ib.setVlanId(vlanId).setVlanIdPresent(present);
        VlanMatchBuilder builder = new VlanMatchBuilder().
            setVlanId(ib.build());
        if (pcp != null) {
            builder.setVlanPcp(new VlanPcp(pcp));
        }

        return builder;
    }

    /**
     * Create flow match builder to match packet from the given port.
     *
     * @param ingress  The ingress port identifier.
     * @param vid      VLAN ID. Zero means untagged ethernet frame.
     * @return  A {@link MatchBuilder} instance.
     */
    public static final MatchBuilder createMatch(String ingress, int vid) {
        MatchBuilder builder = new MatchBuilder();
        if (vid >= 0) {
            builder.setVlanMatch(createVlanMatch(vid).build());
        }
        if (ingress != null) {
            builder.setInPort(new NodeConnectorId(ingress));
        }

        return builder;
    }

    /**
     * Create flow match builder to match the given packet.
     *
     * @param ingress  The ingress port identifier.
     * @param vid      VLAN ID. Zero means untagged ethernet frame.
     * @param src      The source MAC address.
     * @param dst      The destination MAC address.
     * @return  A {@link MatchBuilder} instance.
     */
    public static final MatchBuilder createMatch(
        String ingress, int vid, EtherAddress src, EtherAddress dst) {
        MatchBuilder builder = createMatch(ingress, vid);
        if (src != null || dst != null) {
            builder.setEthernetMatch(createEthernetMatch(src, dst).build());
        }

        return builder;
    }

    /**
     * Verify flow action.
     *
     * @param it   Action list iterator.
     * @param cls  Expected flow action class.
     * @param <T>  The type of flow action.
     * @return  Flow action instance.
     */
    public static final <T> T verifyAction(ListIterator<Action> it,
                                           Class<T> cls) {
        if (!it.hasNext()) {
            String msg = String.format("action[%d]: Expected %s action.",
                                       it.nextIndex(), cls.getSimpleName());
            fail(msg);
        }

        Action act = it.next();
        Object a = act.getAction();
        if (!cls.isInstance(a)) {
            String msg = String.format("action[%d]: %s is expected, but %s.",
                                       it.previousIndex(), cls.getSimpleName(),
                                       a);
            fail(msg);
        }

        return cls.cast(a);
    }

    /**
     * Vefiry POP_VLAN action.
     *
     * @param it  Action list iterator.
     */
    public static final void verifyPopVlanAction(ListIterator<Action> it) {
        PopVlanActionCase act = verifyAction(it, PopVlanActionCase.class);
        PopVlanAction pva = act.getPopVlanAction();
        assertNotNull(pva);
    }

    /**
     * Verify PUSH_VLAN action.
     *
     * @param it       Action list iterator.
     * @param ethType  Expected ethernet type.
     */
    public static final void verifyPushVlanAction(ListIterator<Action> it,
                                                  int ethType) {
        PushVlanActionCase act = verifyAction(it, PushVlanActionCase.class);
        PushVlanAction pva = act.getPushVlanAction();
        assertNotNull(pva);
        assertEquals(Integer.valueOf(ethType), pva.getEthernetType());
    }

    /**
     * Verify SET_VLAN_ID action.
     *
     * @param it   Action list iterator.
     * @param vid  Expected VLAN ID.
     */
    public static final void verifySetVlanIdAction(ListIterator<Action> it,
                                                   int vid) {
        SetVlanIdActionCase act = verifyAction(it, SetVlanIdActionCase.class);
        SetVlanIdAction sva = act.getSetVlanIdAction();
        assertEquals(vid, sva.getVlanId().getValue().intValue());
    }

    /**
     * Verify the flow action that specifies the VLAN for the outgoing packet.
     *
     * @param it      A list iterator associated with a list of flow actions.
     * @param inVid   The VLAN ID for incoming packet.
     * @param outVid  The VLAN ID for outgoing packet.
     */
    public static final void verifyVlanAction(ListIterator<Action> it,
                                              int inVid, int outVid) {
        if (inVid != outVid) {
            if (outVid == 0) {
                verifyPopVlanAction(it);
            } else {
                if (inVid == 0) {
                    int ethType = EtherTypes.VLAN.intValue();
                    verifyPushVlanAction(it, ethType);
                }
                verifySetVlanIdAction(it, outVid);
            }
        }
    }

    /**
     * Verify DROP action.
     *
     * @param it  Action list iterator.
     */
    public static final void verifyDropAction(ListIterator<Action> it) {
        DropActionCase act = verifyAction(it, DropActionCase.class);
        DropAction da = act.getDropAction();
        assertNotNull(da);
    }

    /**
     * Verify OUTPUT action.
     *
     * @param it      Action list iterator.
     * @param egress  Expected egress port identifier.
     */
    public static final void verifyOutputAction(ListIterator<Action> it,
                                          String egress) {
        OutputActionCase act = verifyAction(it, OutputActionCase.class);
        OutputAction oa = act.getOutputAction();
        assertEquals(egress, oa.getOutputNodeConnector().getValue());
    }

    /**
     * Verify that the given flow instruction forwards the packet to the
     * specified network.
     *
     * @param it      A list iterator associated with a list of flow actions.
     * @param egress  The egress port identifier.
     * @param inVid   The VLAN ID for incoming packet.
     * @param outVid  The VLAN ID for outgoing packet.
     */
    public static final void verifyOutputAction(ListIterator<Action> it,
                                                String egress, int inVid,
                                                int outVid) {
        verifyVlanAction(it, inVid, outVid);
        verifyOutputAction(it, egress);
    }

    /**
     * Return a list of actions configured in the given flow instruction.
     *
     * @param inst  A flow instructions.
     * @return  A list of flow actions.
     */
    public static final List<Action> getActionList(Instructions inst) {
        List<Instruction> instList = inst.getInstruction();
        assertNotNull("Instruction is null.", instList);

        assertEquals(FLOW_INST_SIZE, instList.size());

        org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.instruction.Instruction instCase =
            instList.get(0).getInstruction();
        if (!(instCase instanceof ApplyActionsCase)) {
            fail("Unexpected instruction: " + instCase);
        }

        ApplyActionsCase apCase = (ApplyActionsCase)instCase;
        ApplyActions apActions = apCase.getApplyActions();
        assertNotNull("ApplyActions is null.", apActions);

        List<Action> actList = apActions.getAction();
        assertNotNull("Action list is null.", actList);
        assertFalse("Action list is empty.", actList.isEmpty());

        List<Action> actions = new ArrayList<>();
        actions.addAll(actList);
        Collections.sort(actions, new OrderedComparator());

        return actions;
    }

    /**
     * Verify that the given flow instruction forwards the packet to the
     * specified network.
     *
     * @param inst    A flow instructions.
     * @param egress  The egress port identifier.
     * @param inVid   The VLAN ID for incoming packet.
     * @param outVid  The VLAN ID for outgoing packet.
     */
    public static final void verifyOutputFlow(Instructions inst, String egress,
                                              int inVid, int outVid) {
        ListIterator<Action> it = getActionList(inst).listIterator();
        verifyOutputAction(it, egress, inVid, outVid);
        assertFalse(it.hasNext());
    }

    /**
     * Verify that the given flow instruction drops the packet.
     *
     * @param inst     A flow instructions.
     */
    public static final void verifyDropFlow(Instructions inst) {
        ListIterator<Action> it = getActionList(inst).listIterator();
        verifyDropAction(it);
        assertFalse(it.hasNext());
    }

    /**
     * Return the VLAN ID configured in the given flow match.
     *
     * @param match  A {@link Match} instance.
     * @return  VLAN ID configured in the given match.
     *          Zero is returned if the given instance matches untagged
     *          network.
     */
    public static final int getVlanMatch(Match match) {
        VlanMatch vmatch = match.getVlanMatch();
        org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.vlan.match.fields.VlanId vid =
            vmatch.getVlanId();

        Boolean present = vid.isVlanIdPresent();
        assertNotNull(present);
        if (Boolean.FALSE.equals(present)) {
            return 0;
        }

        return vid.getVlanId().getValue().intValue();
    }

    /**
     * Return the source MAC address configured in the given flow match.
     *
     * @param match  A {@link Match} instance.
     * @return  An {@link EtherAddress} instance that represents the
     *          source MAC address configured in the given match.
     *          {@code null} is returned if the source MAC address is not
     *          configured.
     */
    public static final EtherAddress getEthernetSourceMatch(Match match) {
        EtherAddress eaddr = null;
        EthernetMatch ematch = match.getEthernetMatch();
        if (ematch != null) {
            EthernetSource esrc = ematch.getEthernetSource();
            if (esrc != null) {
                eaddr = EtherAddress.create(esrc.getAddress());
            }
        }

        return eaddr;
    }

    /**
     * Return the destination MAC address configured in the given flow match.
     *
     * @param match  A {@link Match} instance.
     * @return  An {@link EtherAddress} instance that represents the
     *          destination MAC address configured in the given match.
     *          {@code null} is returned if the destination MAC address is not
     *          configured.
     */
    public static final EtherAddress getEthernetDestinationMatch(Match match) {
        EtherAddress eaddr = null;
        EthernetMatch ematch = match.getEthernetMatch();
        if (ematch != null) {
            EthernetDestination edst = ematch.getEthernetDestination();
            if (edst != null) {
                eaddr = EtherAddress.create(edst.getAddress());
            }
        }

        return eaddr;
    }

    /**
     * Return the ingress port identifier configured in the given flow match.
     *
     * @param match  A {@link Match} instance.
     * @return  The ingress port identifier if found.
     *          {@code null} if not found.
     */
    public static final String getInPortMatch(Match match) {
        NodeConnectorId ncId = match.getInPort();
        return (ncId == null) ? null : ncId.getValue();
    }

    /**
     * Determine whether the given flow instructions forwards packets to
     * the given network or not.
     *
     * @param inst   Flow instructions.
     * @param pid    The egress port identifier.
     * @param vid    VLAN ID of the egress network.
     * @param inVid  VLAN ID configured in the incoming packet.
     * @return  {@code true} only if the given flow instructions forwards
     *          packets to the given network.
     */
    public static final boolean hasOutput(Instructions inst, String pid,
                                          int vid, int inVid) {
        int vlan = inVid;
        for (Action act: getActionList(inst)) {
            Object a = act.getAction();
            if (a instanceof PopVlanActionCase) {
                vlan = 0;
            } else if (a instanceof SetVlanIdActionCase) {
                SetVlanIdActionCase s = (SetVlanIdActionCase)a;
                SetVlanIdAction sva = s.getSetVlanIdAction();
                VlanId vlanId = sva.getVlanId();
                if (vlanId != null) {
                    Integer value = vlanId.getValue();
                    if (value != null) {
                        vlan = value.intValue();
                    }
                }
            } else if (a instanceof OutputActionCase) {
                OutputActionCase o = (OutputActionCase)a;
                OutputAction oa = o.getOutputAction();
                if (oa != null) {
                    String out = oa.getOutputNodeConnector().getValue();
                    if (pid.equals(out) && vid == vlan) {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    /**
     * Wait for completion of the RPC task associated with the given future.
     *
     * @param f    A {@link Future} instance associated with the RPC task.
     * @param <T>  The type of the RPC output.
     * @return  The output of the RPC task.
     */
    public static final <T> T getRpcOutput(Future<RpcResult<T>> f) {
        return getRpcOutput(f, false);
    }

    /**
     * Wait for completion of the RPC task associated with the given future.
     *
     * @param f         A {@link Future} instance associated with the RPC task.
     * @param nillable  Set {@code true} if the result can be {@code null}.
     * @param <T>  The type of the RPC output.
     * @return  The output of the RPC task.
     */
    public static final <T> T getRpcOutput(
        Future<RpcResult<T>> f, boolean nillable) {
        try {
            RpcResult<T> result = f.get(TASK_TIMEOUT, TimeUnit.SECONDS);
            assertNotNull(result);
            if (!result.isSuccessful()) {
                fail("RPC failed: " + result);
            }

            T output = result.getResult();
            if (!nillable && output == null) {
                fail("RPC output is null: " + result);
            }

            return output;
        } catch (Exception e) {
            unexpected(e);
        }

        return null;
    }

    /**
     * Return the vtn-update-type that indicates the result of the RPC task.
     *
     * @param f    A {@link Future} instance associated with the RPC task.
     * @param <T>  The type of the RPC output.
     * @return  A {@link VtnUpdateType} instance in the RPC output.
     */
    public static final <T extends VtnRpcResult> VtnUpdateType getRpcResult(
        Future<RpcResult<T>> f) {
        return getRpcOutput(f).getStatus();
    }

    /**
     * Ensure that the given RPC returns the specified error status.
     *
     * @param f     A {@link Future} instance associated with the RPC task.
     * @param etag  A {@link RpcErrorTag} that specifies the expected RPC
     *              error tag.
     * @param vtag  A {@link VtnErrorTag} that specifies the expected RPC
     *              application tag.
     * @param <T>  The type of the RPC output.
     */
    public static final <T> void checkRpcError(
        Future<RpcResult<T>> f, RpcErrorTag etag, VtnErrorTag vtag) {
        try {
            RpcResult<T> result = f.get(TASK_TIMEOUT, TimeUnit.SECONDS);
            assertNotNull(result);
            if (result.isSuccessful()) {
                fail("RPC succeeded unexpectedly: " + result);
            }

            Collection<RpcError> errors = result.getErrors();
            if (errors == null || errors.isEmpty()) {
                fail("RPC did not set error information: " + result);
            }
            if (errors.size() != 1) {
                fail("RPC set more than one error: " + result);
            }

            RpcError rerr = errors.iterator().next();
            assertEquals(etag.getErrorTag(), rerr.getTag());
            assertEquals(vtag.toString(), rerr.getApplicationTag());
        } catch (Exception e) {
            unexpected(e);
        }
    }

    /**
     * Get all the VTNs.
     *
     * @param service  A {@link VTNServices} instance.
     * @return  A list of VTNs.
     */
    public static final List<Vtn> getVtns(VTNServices service) {
        try (ReadOnlyTransaction rtx = service.newReadOnlyTransaction()) {
            return getVtns(rtx);
        }
    }

    /**
     * Get all the VTNs.
     *
     * @param rtx  A read-only MD-SAL datastore transaction.
     * @return  A list of VTNs.
     */
    public static final List<Vtn> getVtns(ReadTransaction rtx) {
        InstanceIdentifier<Vtns> path = InstanceIdentifier.create(Vtns.class);
        Optional<Vtns> opt = DataStoreUtils.read(rtx, path);
        List<Vtn> vtns = null;
        if (opt.isPresent()) {
            vtns = opt.get().getVtn();
        }

        if (vtns == null) {
            vtns = Collections.<Vtn>emptyList();
        }

        return vtns;
    }

    /**
     * Add the given switch port identifier to the static-edge-ports.
     *
     * @param service  A {@link VTNServices} instance.
     * @param pid      The MD-SAL port identifier.
     */
    public static final void addStaticEdgePort(VTNServices service,
                                               String pid) {
        NodeConnectorId ncId = new NodeConnectorId(pid);
        StaticEdgePortKey key = new StaticEdgePortKey(ncId);
        InstanceIdentifier<StaticEdgePort> epath = InstanceIdentifier.
            builder(VtnStaticTopology.class).
            child(StaticEdgePorts.class).
            child(StaticEdgePort.class, key).
            build();
        StaticEdgePort stEdge = new StaticEdgePortBuilder().
            setPort(ncId).
            setKey(key).
            build();
        LogicalDatastoreType config = LogicalDatastoreType.CONFIGURATION;
        ReadWriteTransaction tx = service.newReadWriteTransaction();
        tx.put(config, epath, stEdge, true);
        DataStoreUtils.submit(tx);
    }

    /**
     * Remove the given switch port identifier from the static-edge-ports.
     *
     * @param service  A {@link VTNServices} instance.
     * @param pid      The MD-SAL port identifier.
     */
    public static final void removeStaticEdgePort(VTNServices service,
                                                  String pid) {
        NodeConnectorId ncId = new NodeConnectorId(pid);
        StaticEdgePortKey key = new StaticEdgePortKey(ncId);
        InstanceIdentifier<StaticEdgePort> epath = InstanceIdentifier.
            builder(VtnStaticTopology.class).
            child(StaticEdgePorts.class).
            child(StaticEdgePort.class, key).
            build();
        LogicalDatastoreType config = LogicalDatastoreType.CONFIGURATION;
        ReadWriteTransaction tx = service.newReadWriteTransaction();
        tx.delete(config, epath);
        DataStoreUtils.submit(tx);
    }

    /**
     * Delete the vtn-static-topology container.
     *
     * @param service  A {@link VTNServices} instance.
     */
    public static final void removeVtnStaticTopology(VTNServices service) {
        InstanceIdentifier<VtnStaticTopology> vsPath = InstanceIdentifier.
            create(VtnStaticTopology.class);
        LogicalDatastoreType config = LogicalDatastoreType.CONFIGURATION;
        ReadWriteTransaction tx = service.newReadWriteTransaction();
        tx.delete(config, vsPath);
        DataStoreUtils.submit(tx);
    }

    /**
     * Create a virtual node path that specifies the vBridge or vBridge
     * interface.
     *
     * @param tname  The name of the VTN.
     * @param bname  The name of the vBridge.
     * @param iname  The name of the virtual interface.
     * @return  A {@link VirtualNodePath} instance.
     */
    public static final VirtualNodePath newBridgePath(
        String tname, String bname, String iname) {
        return new VirtualNodePathBuilder().
            setTenantName(tname).
            setBridgeName(bname).
            setInterfaceName(iname).
            build();
    }

    /**
     * Create a virtual node path that specifies the vTerminal or vTerminal
     * interface.
     *
     * @param tname  The name of the VTN.
     * @param bname  The name of the vTerminal.
     * @param iname  The name of the virtual interface.
     * @return  A {@link VirtualNodePath} instance.
     */
    public static final VirtualNodePath newTerminalPath(
        String tname, String bname, String iname) {
        return new VirtualNodePathBuilder().
            setTenantName(tname).
            setTerminalName(bname).
            setInterfaceName(iname).
            build();
    }
}
