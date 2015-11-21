/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.packet;

import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang3.tuple.ImmutablePair;
import org.apache.commons.lang3.tuple.Pair;

import org.opendaylight.vtn.manager.packet.Packet;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxHook;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

/**
 * {@code PacketOutQueue} describes a queue for packets to transmit.
 *
 * <p>
 *   An instance of this class must be obtained by
 *   {@link TxContext#getSpecific(Class)}. All the queued packets are
 *   transmitted just after the successful completion of the MD-SAL datastore
 *   transaction associated with this instance. Note that packets will be
 *   discarded when the transaction is canceled.
 * </p>
 */
public final class PacketOutQueue implements TxHook {
    /**
     * The order of the post-submit hook.
     *
     * <p>
     *   PACKET_OUT should be processed prior to other hooks.
     * </p>
     */
    private static final int  HOOK_ORDER = Integer.MIN_VALUE;

    /**
     * A list of packets to transmit.
     */
    private final List<Pair<SalPort, Packet>>  outputs = new ArrayList<>();

    /**
     * Construct a new instance.
     *
     * @param ctx  A runtime context for transaction task.
     */
    public PacketOutQueue(TxContext ctx) {
        // Register this instance as a post-submit hook.
        ctx.addPostSubmitHook(this);
    }

    /**
     * Enqueue the given packet to the transmission queue.
     *
     * @param egress  A {@link SalPort} instance which specifies the egress
     *                switch port.
     * @param packet  A {@link Packet} instance to transmit.
     */
    public void enqueue(SalPort egress, Packet packet) {
        outputs.add(new ImmutablePair<SalPort, Packet>(egress, packet));
    }

    // TxHook

    /**
     * Invoked when the MD-SAL datastore transaction has been submitted
     * successfully.
     *
     * <p>
     *   This method transmits all the packets in the transmission queue.
     * </p>
     *
     * @param ctx   A runtime context for transaction task.
     * @param task  The current transaction task.
     */
    @Override
    public void run(TxContext ctx, TxTask<?> task) {
        ctx.getProvider().transmit(outputs);
    }

    /**
     * {@inheritDoc}
     */
    public int getOrder() {
        return HOOK_ORDER;
    }
}
