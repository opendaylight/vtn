/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.packet;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.fail;

import java.util.Set;

import org.opendaylight.vtn.manager.util.ByteUtils;

import org.opendaylight.vtn.manager.it.ofmock.OfMockUtils;
import org.opendaylight.vtn.manager.it.util.match.FlowMatchType;

import org.opendaylight.controller.sal.packet.Packet;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;

/**
 * {@code PacketFactory} is a utility class used to create or to verify
 * a packet.
 */
public abstract class PacketFactory implements Cloneable {
    /**
     * A packet generator for the next layer.
     */
    private PacketFactory  nextFactory;

    /**
     * A byte array that represents the raw payload.
     */
    private byte[]  rawPayload;

    /**
     * Construct a new instance.
     */
    PacketFactory() {}

    /**
     * Create a packet.
     *
     * @return  A byte array that represents the packet image.
     * @throws Exception  An error occurred.
     */
    public final byte[] create() throws Exception {
        Packet root = createPacket();
        Packet parent = root;
        PacketFactory f = nextFactory;
        PacketFactory last = this;
        while (f != null) {
            last = f;
            Packet p = parent.getPayload();
            while (p != null) {
                parent = p;
                p = p.getPayload();
            }

            p = f.createPacket();
            parent.setPayload(p);
            parent = p;
            f = f.getNextFactory();
        }

        byte[] b = last.getRawPayload();
        if (b != null && b.length != 0) {
            parent.setRawPayload(b);
        }

        return root.serialize();
    }

    /**
     * Return the packet factory for the next protocol layer.
     *
     * @return  A {@link PacketFactory} instance or {@code null}.
     */
    public final PacketFactory getNextFactory() {
        return nextFactory;
    }

    /**
     * Return the packet factory for the next protocol layer.
     *
     * @param cls  A {@link Class} instance which specifies the type of
     *             expected factory class.
     * @param <T>  The type of factory class.
     * @return  The next factory class or {@code null}.
     */
    public final <T extends PacketFactory> T getNextFactory(Class<T> cls) {
        if (nextFactory != null) {
            if (cls.equals(nextFactory.getClass())) {
                return cls.cast(nextFactory);
            }
        }

        return null;
    }

    /**
     * Set the packet factory for the next protocol layer.
     *
     * @param f  A {@link PacketFactory} instance.
     * @return  This instance.
     */
    public final PacketFactory setNextFactory(PacketFactory f) {
        nextFactory = f;
        return this;
    }

    /**
     * Return a raw payload.
     *
     * @return  A byte array that represents the raw payload of the packet.
     *          {@code null} if not configured.
     */
    public final byte[] getRawPayload() {
        return getBytes(rawPayload);
    }

    /**
     * Set a raw payload of the packet.
     *
     * @param b  A byte array that represents the raw payload of the packet.
     * @return  This instance.
     */
    public final PacketFactory setRawPayload(byte[] b) {
        rawPayload = getBytes(b);
        return this;
    }

    /**
     * Verify the raw payload.
     *
     * @param payload  A byte array that representse the raw payload to be
     *                 verified.
     */
    public final void assertRawPayload(byte[] payload) {
        if (rawPayload == null) {
            if (payload != null && payload.length != 0) {
                fail("Unexpected raw payload: " +
                     ByteUtils.toHexString(payload));
            }
        } else {
            assertArrayEquals(rawPayload, payload);
        }
    }

    /**
     * Create a flow match specified by the given match type.
     *
     * @param builder  A {@link MatchBuilder} instance.
     * @param ingress  The ingress port identifier.
     * @param types    A set of {@link FlowMatchType} instances that specifies
     *                 flow match types to be configured.
     * @return  Return the number of additional flow match types configured
     *          into the given match builder.
     */
    public final int initMatch(MatchBuilder builder, String ingress,
                               Set<FlowMatchType> types) {
        builder.setInPort(new NodeConnectorId(ingress));

        int count = 0;
        for (PacketFactory f = this; f != null; f = f.getNextFactory()) {
            count += f.initMatch(builder, types);
        }

        return count;
    }

    /**
     * Return a clone of the give byte array.
     *
     * @param b  A byte array.
     * @return   A clone of the given byte array.
     *           {@code null} is returned if {@code null} is specified.
     */
    byte[] getBytes(byte[] b) {
        return (b == null) ? null : b.clone();
    }

    /**
     * Convert the given byte value into a short integer value.
     *
     * @param b  A byte value.
     * @return   A {@link Short} instance.
     */
    Short toShort(byte b) {
        return Short.valueOf((short)(b & OfMockUtils.MASK_BYTE));
    }

    /**
     * Create a packet instance.
     *
     * @return  A {@link Packet} instance.
     */
    abstract Packet createPacket();

    /**
     * Verify the given packet.
     *
     * @param packet  A {@link Packet} instance.
     */
    abstract void verify(Packet packet);

    /**
     * Construct a flow match specified by the given match type.
     *
     * @param builder  A {@link MatchBuilder} instance.
     * @param types    A set of {@link FlowMatchType} instances that specifies
     *                 flow match types to be configured.
     * @return  Return the number of additional flow match types configured
     *          into the given match builder.
     */
    abstract int initMatch(MatchBuilder builder, Set<FlowMatchType> types);

    // Object

    /**
     * Return a deep copy of this instance.
     *
     * @return  A deep copy of this instance.
     */
    @Override
    public PacketFactory clone() {
        try {
            PacketFactory c = (PacketFactory)super.clone();
            PacketFactory next = nextFactory;
            if (next != null) {
                c.nextFactory = next.clone();
            }
            c.rawPayload = getBytes(rawPayload);

            return c;
        } catch (CloneNotSupportedException e) {
            throw new IllegalStateException("Unable to clone.", e);
        }
    }
}
