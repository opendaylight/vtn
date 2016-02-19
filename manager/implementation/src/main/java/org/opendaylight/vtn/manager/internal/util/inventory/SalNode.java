/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import java.math.BigInteger;

import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.InstanceIdentifierBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnNodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNodeKey;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.Table;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.TableKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeRef;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.Nodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.NodeKey;

/**
 * This class represents SAL node identifier corresponding to
 * OpenFlow switch.
 */
public class SalNode {
    /**
     * Separator of Node/NodeConnector ID.
     */
    protected static final char  MD_ID_SEPARATOR = ':';

    /**
     * Node/NodeConnector ID prefix, including separator, that represents
     * OpenFlow.
     */
    private static final String  MD_ID_OPENFLOW = "openflow" + MD_ID_SEPARATOR;

    /**
     * Node number that identifies a node.
     * Datapath ID of an OpenFlow switch is used as an identifier.
     */
    private final long  nodeNumber;

    /**
     * Cache for the identifier string.
     *
     * <p>
     *   Note that this field does not affect object identity.
     * </p>
     */
    private String  identString;

    /**
     * Convert a MD-SAL node ID into a {@code SalNode} instance.
     *
     * @param id  A {@link NodeId} instance.
     * @return  A {@code SalNode} instance on success.
     *          {@code null} on failure.
     */
    public static final SalNode create(NodeId id) {
        if (id == null) {
            return null;
        }

        return create(id.getValue());
    }

    /**
     * Convert a MD-SAL node reference into a {@link SalNode} instance.
     *
     * @param ref  A {@link NodeRef} instance.
     * @return     A {@code SalNode} instance on success.
     *          {@code null} on failure.
     */
    public static final SalNode create(NodeRef ref) {
        if (ref == null) {
            return null;
        }

        InstanceIdentifier<?> id = ref.getValue();
        if (id == null) {
            return null;
        }

        return create(id.firstKeyOf(Node.class));
    }

    /**
     * Convert a MD-SAL node key into a {@link SalNode} instance.
     *
     * @param key  A {@link NodeKey} instance.
     * @return  A {@code SalNode} instance on success.
     *          {@code null} on failure.
     */
    public static final SalNode create(NodeKey key) {
        return (key == null) ? null : create(key.getId());
    }

    /**
     * Convert a string representation of MD-SAL node ID into a {@code SalNode}
     * instance.
     *
     * @param id  A string representation of MD-SAL node ID.
     * @return  A {@code SalNode} instance on success.
     *          {@code null} on failure.
     */
    public static SalNode create(String id) {
        if (id == null || !id.startsWith(MD_ID_OPENFLOW)) {
            return null;
        }

        BigInteger bi;
        try {
            bi = new BigInteger(id.substring(MD_ID_OPENFLOW.length()));
        } catch (Exception e) {
            return null;
        }

        return (bi.bitLength() > Long.SIZE || bi.signum() < 0)
            ? null : new SalNode(bi.longValue(), id);
    }

    /**
     * Convert the given MD-SAL node ID into a {@code SalNode} instance
     * with value checking.
     *
     * @param nid  A {@link NodeId} instance.
     * @return  A {@code SalNode} instance.
     * @throws RpcException
     *    An invalid node ID is passed to {@code nid}.
     */
    public static SalNode checkedCreate(NodeId nid) throws RpcException {
        String id = (nid == null) ? null : nid.getValue();
        return checkedCreate(id);
    }

    /**
     * Convert a string representation of MD-SAL node ID into a {@code SalNode}
     * instance with value checking.
     *
     * @param id  A string representation of MD-SAL node ID.
     * @return  A {@code SalNode} instance.
     * @throws RpcException
     *    An invalid node ID is passed to {@code id}.
     */
    public static SalNode checkedCreate(String id) throws RpcException {
        SalNode snode = create(id);
        if (snode == null) {
            if (id == null) {
                throw RpcException.getNullArgumentException("Node");
            }
            throw RpcException.getBadArgumentException(
                "Invalid node ID: " + id);
        }

        return snode;
    }

    /**
     * Construct a new instance.
     *
     * @param id  A node identifier.
     */
    public SalNode(long id) {
        nodeNumber = id;
    }

    /**
     * Construct a new instance with specifying a string representation of
     * this instance.
     *
     * @param id   A node identifier.
     * @param str  A string representation of this instance.
     */
    public SalNode(long id, String str) {
        nodeNumber = id;
        identString = str;
    }

    /**
     * Return a node number.
     *
     * @return  A node identifier.
     */
    public final long getNodeNumber() {
        return nodeNumber;
    }

    /**
     * Return a {@link NodeId} instance corresponding to this instance.
     *
     * @return  A {@link NodeId} instance.
     */
    public final NodeId getNodeId() {
        return new NodeId(toNodeString());
    }

    /**
     * Return a {@link NodeKey} instance corresponding to this instance.
     *
     * @return  A {@link NodeKey} instance.
     */
    public final NodeKey getNodeKey() {
        return new NodeKey(getNodeId());
    }

    /**
     * Return a {@link NodeRef} instance corresponding to this instance.
     *
     * @return  An {@link NodeRef} instance.
     */
    public NodeRef getNodeRef() {
        return new NodeRef(getNodeIdentifier());
    }

    /**
     * Return an instance identifier that specifies MD-SAL node instance.
     *
     * @return  An instance identifier for a MD-SAL node.
     */
    public final InstanceIdentifier<Node> getNodeIdentifier() {
        return getNodeIdentifierBuilder().build();
    }

    /**
     * Return an instance identifier that specifies flow-capable-node for
     * the node corresponding to this instance.
     *
     * @return An instance identifier for the flow-capable-node.
     */
    public final InstanceIdentifier<FlowCapableNode> getFlowNodeIdentifier() {
        return getNodeIdentifierBuilder().
            augmentation(FlowCapableNode.class).
            build();
    }

    /**
     * Return an instance identifier for the specified flow table in the
     * node corresponding to this instance.
     *
     * @param tid  Identifier for the table.
     * @return  An instance identifier for the specified flow table.
     */
    public final InstanceIdentifier<Table> getFlowTableIdentifier(Short tid) {
        return getNodeIdentifierBuilder().
            augmentation(FlowCapableNode.class).
            child(Table.class, new TableKey(tid)).
            build();
    }

    /**
     * Return a {@link VtnNodeKey} instance corresponding to this instance.
     *
     * @return  A {@link VtnNodeKey} instance.
     */
    public final VtnNodeKey getVtnNodeKey() {
        return new VtnNodeKey(getNodeId());
    }

    /**
     * Return an instance identifier for a VTN node associated with this
     * instance.
     *
     * @return  An instance identifier for a VTN node.
     */
    public final InstanceIdentifier<VtnNode> getVtnNodeIdentifier() {
        return getVtnNodeIdentifierBuilder().build();
    }

    /**
     * Return a string which identifies the switch corresponding to this
     * instance.
     *
     * @return  A string which identifies the switch.
     */
    public String toNodeString() {
        return toString();
    }

    /**
     * Determine whether the given node has the same node number or not.
     *
     * @param snode  A {@link SalNode} instance to be compared.
     * @return  {@code true} only if the given node has the same node number
     *          as this instance.
     */
    public final boolean equalsNode(SalNode snode) {
        return (snode != null && snode.nodeNumber == nodeNumber);
    }

    /**
     * Return a {@link StringBuilder} instance that contains a string
     * representation of this instance.
     *
     * @return  A {@link StringBuilder} instance.
     */
    protected StringBuilder toStringBuilder() {
        return toNodeStringBuilder();
    }

    /**
     * Return a {@link StringBuilder} instance that contains a string
     * representation of a node identifier.
     *
     * @return  A {@link StringBuilder} instance.
     */
    protected final StringBuilder toNodeStringBuilder() {
        StringBuilder builder = new StringBuilder(MD_ID_OPENFLOW);
        if (nodeNumber < 0) {
            // Datapath ID needs to be treated as unsigned.
            BigInteger bi = NumberUtils.getUnsigned(nodeNumber);
            builder.append(bi);
        } else {
            builder.append(nodeNumber);
        }

        return builder;
    }

    /**
     * Return an instance identifier builder that contains an instance
     * identifier for a MD-SAL node.
     *
     * @return  An instance identifier builder for a MD-SAL node.
     *
     */
    protected final InstanceIdentifierBuilder<Node> getNodeIdentifierBuilder() {
        return InstanceIdentifier.builder(Nodes.class).
            child(Node.class, getNodeKey());
    }

    /**
     * Return an instance identifier builder that contains an instance
     * identifier for a VTN node.
     *
     * @return  An instance identifier builder for a VTN node.
     *
     */
    protected final InstanceIdentifierBuilder<VtnNode> getVtnNodeIdentifierBuilder() {
        return InstanceIdentifier.builder(VtnNodes.class).
            child(VtnNode.class, getVtnNodeKey());
    }

    // Object

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (o == null || !getClass().equals(o.getClass())) {
            return false;
        }

        SalNode snode = (SalNode)o;
        return (nodeNumber == snode.nodeNumber);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return getClass().hashCode() + NumberUtils.hashCode(nodeNumber);
    }

    /**
     * Return a string representation of this object.
     *
     * <p>
     *   This method returns a string representation of MD-SAL node identifier.
     * </p>
     *
     * @return  A string representation of this object.
     */
    @Override
    public final String toString() {
        String ident = identString;
        if (ident == null) {
            ident = toStringBuilder().toString();
            identString = ident;
        }

        return ident;
    }
}
