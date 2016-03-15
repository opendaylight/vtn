/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.packet;

import org.opendaylight.vtn.manager.internal.util.inventory.NodeRpcInvocation;
import org.opendaylight.vtn.manager.internal.util.inventory.NodeRpcWatcher;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnector;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnectorKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.packet.service.rev130709.PacketProcessingService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.packet.service.rev130709.TransmitPacketInput;

/**
 * {@code TransmitPacketRpc} describes an invocation of transmit-packet RPC
 * provided by the MD-SAL packet-processing service.
 */
public final class TransmitPacketRpc
    extends NodeRpcInvocation<TransmitPacketInput, Void> {
    /**
     * Issue an transmit-packet RPC request.
     *
     * @param w    The node RPC watcher.
     * @param pps  MD-SAL packet-processing service.
     * @param in   The RPC input.
     */
    public TransmitPacketRpc(NodeRpcWatcher w, PacketProcessingService pps,
                             TransmitPacketInput in) {
        super(w, in, in.getNode(), pps.transmitPacket(in));
    }

    // RpcRequest

    /**
     * Return the name of the RPC.
     *
     * @return  "transmit-packet".
     */
    @Override
    public String getName() {
        return "transmit-packet";
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getInputForLog() {
        InstanceIdentifier<?> path = getInput().getEgress().getValue();
        NodeConnectorKey key = path.firstKeyOf(NodeConnector.class);
        return "{egress=" + key.getId().getValue() + "}";
    }
}
