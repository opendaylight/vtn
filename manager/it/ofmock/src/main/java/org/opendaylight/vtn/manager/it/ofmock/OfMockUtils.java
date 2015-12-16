/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock;

import java.math.BigInteger;
import java.util.List;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.ActionList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.OutputActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.output.action._case.OutputAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.FlowCookie;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.Instructions;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.instruction.instruction.ApplyActionsCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.instruction.instruction.WriteActionsCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.instruction.list.Instruction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorRef;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.Nodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnector;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnectorKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.NodeKey;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Uri;

/**
 * {@code OfMockUtils} contains static utility methods.
 */
public final class OfMockUtils {
    /**
     * A bitmask for a byte value.
     */
    public static final int  MASK_BYTE = 0xff;

    /**
     * A bitmask for a short value.
     */
    public static final int  MASK_SHORT = 0xffff;

    /**
     * The default value of flow cookie.
     */
    public static final BigInteger  COOKIE_DEFAULT = BigInteger.ZERO;

    /**
     * Private constructor that protects this class from instantiating.
     */
    private OfMockUtils() {}

    /**
     * Create a node identifier from the given port identifier.
     *
     * @param pid  The port identifier.
     * @return  The node identifier string.
     */
    public static String getNodeIdentifier(String pid) {
        int idx = pid.lastIndexOf(OfMockService.ID_SEPARATOR);
        if (idx < 0) {
            throw new IllegalArgumentException(
                "Invalid port identifier: " + pid);
        }

        return pid.substring(0, idx);
    }

    /**
     * Return the port ID in the given MD-SAL node connector identifier.
     *
     * @param pid  The port identifier.
     * @return  The port ID
     */
    public static String getPortId(String pid) {
        int idx = pid.lastIndexOf(OfMockService.ID_SEPARATOR);
        if (idx < 0) {
            throw new IllegalArgumentException(
                "Invalid port identifier: " + pid);
        }

        return pid.substring(idx + 1);
    }

    /**
     * Return the port identifier string for the specified port.
     *
     * @param nid  The node identifier.
     * @param num  The port number.
     * @return  A port identifier string.
     */
    public static String getPortIdentifier(String nid, long num) {
        StringBuilder builder = new StringBuilder(nid).
            append(OfMockService.ID_SEPARATOR).append(num);
        return builder.toString();
    }

    /**
     * Return the port identifier string for the specified port.
     *
     * @param nid  The node identifier.
     * @param num  The port number.
     * @return  A port identifier string.
     */
    public static String getPortIdentifier(String nid, BigInteger num) {
        StringBuilder builder = new StringBuilder(nid).
            append(OfMockService.ID_SEPARATOR).append(num);
        return builder.toString();
    }

    /**
     * Return the port identifier string for the specified port.
     *
     * @param nid  The node identifier.
     * @param num  A string that indicates the port number.
     * @return  A port identifier string.
     */
    public static String getPortIdentifier(String nid, String num) {
        StringBuilder builder = new StringBuilder(nid).
            append(OfMockService.ID_SEPARATOR).append(num);
        return builder.toString();
    }

    /**
     * Return the port identifier string in the given node connector ID.
     *
     * @param id  A {@link NodeConnectorId} instance.
     * @return  A port identifier string.
     *          {@code null} is returned if the port identifier string is
     *          not present.
     */
    public static String getPortIdentifier(NodeConnectorId id) {
        return (id == null) ? null : id.getValue();
    }

    /**
     * Return the MD-SAL node connector indentifier configured in the given
     * node connector reference.
     *
     * @param ref  A {@link NodeConnectorRef} instance.
     * @return  The MD-SAL node connector identifier.
     *          {@code null} is returned if not available.
     */
    public static String getPortIdentifier(NodeConnectorRef ref) {
        if (ref == null) {
            return null;
        }

        InstanceIdentifier<?> path = ref.getValue();
        if (path == null) {
            return null;
        }

        NodeConnectorKey key = path.firstKeyOf(NodeConnector.class);
        if (key == null) {
            return null;
        }

        return getPortIdentifier(key.getId());
    }

    /**
     * Create a instance identifier which specifies the given switch port.
     *
     * @param pid  The port identifier.
     * @return  An {@link InstanceIdentifier} instance.
     */
    public static InstanceIdentifier<NodeConnector> getPortPath(String pid) {
        String nid = OfMockUtils.getNodeIdentifier(pid);
        NodeConnectorId ncid = new NodeConnectorId(pid);

        return InstanceIdentifier.builder(Nodes.class).
            child(Node.class, new NodeKey(new NodeId(nid))).
            child(NodeConnector.class, new NodeConnectorKey(ncid)).build();
    }

    /**
     * Determine whether the given instruction list contains an output action
     * that transmits packets to the given port or not.
     *
     * @param insts  A flow instructions.
     * @param pid    The target port identifier.
     * @return  {@code true} if the given instruction list contains at least
     *          one output action that specifies the given port.
     *          Otherwise {@code false}.
     */
    public static boolean hasOutput(Instructions insts, String pid) {
        if (insts == null) {
            return false;
        }

        List<Instruction> instList = insts.getInstruction();
        if (instList == null) {
            return false;
        }

        for (Instruction inst: instList) {
            org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.
                instruction.Instruction instCase = inst.getInstruction();

            ActionList acList = null;
            if (instCase instanceof ApplyActionsCase) {
                ApplyActionsCase apCase = (ApplyActionsCase)instCase;
                acList = apCase.getApplyActions();
            }  else if (instCase instanceof WriteActionsCase) {
                WriteActionsCase wrCase = (WriteActionsCase)instCase;
                acList = wrCase.getWriteActions();
            }

            if (acList != null && hasOutput(acList.getAction(), pid)) {
                return true;
            }
        }

        return false;
    }

    /**
     * Determine whether the given action list contains an output action that
     * transmits packets to the given port or not.
     *
     * @param actions  An action list.
     * @param pid      The target port identifier.
     * @return  {@code true} if the given action list contains at least
     *          one output action that specifies the given port.
     *          Otherwise {@code false}.
     */
    public static boolean hasOutput(List<Action> actions, String pid) {
        if (actions == null) {
            return false;
        }

        for (Action act: actions) {
            org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.
                rev131112.action.Action a = act.getAction();
            if (a instanceof OutputActionCase) {
                OutputActionCase outCase = (OutputActionCase)a;
                OutputAction out = outCase.getOutputAction();
                if (out == null) {
                    continue;
                }

                Uri uri = out.getOutputNodeConnector();
                if (uri != null && pid.equals(uri.getValue())) {
                    return true;
                }
            }
        }

        return false;
    }

    /**
     * Return the flow cookie configured in the given instance.
     *
     * @param cookie  A flow cookie.
     * @return  The cookie value configured in {@code cookie}.
     */
    public static BigInteger getCookie(FlowCookie cookie) {
        BigInteger value = (cookie == null) ? null : cookie.getValue();
        return (value == null) ? COOKIE_DEFAULT : value;
    }
}
