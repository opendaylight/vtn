/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.flow.match;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.VtnLayer4Match;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpVersion;

/**
 * {@code VTNLayer4Match} describes the condition for layer 4 protocol header
 * to match against packets.
 */
public abstract class VTNLayer4Match {
    /**
     * Return an IP protocol number assigned to this protocol.
     *
     * @param ver  An {@link IpVersion} instance which describes the
     *             IP version.
     * @return  An IP protocol number.
     * @throws IllegalStateException
     *    This layer 4 protocol is unavailable on the given IP version.
     */
    public abstract short getInetProtocol(IpVersion ver);

    /**
     * Return a {@link VtnLayer4Match} instance which contains the condition
     * represented by this instance.
     *
     * @return  A {@link VtnLayer4Match} instance if this instance contains
     *          the condition.
     */
    public abstract VtnLayer4Match toVtnLayer4Match();

    /**
     * Determine whether this instance is empty or not.
     *
     * @return  {@code true} if this instance is empty.
     *          {@code false} otherwise.
     */
    public abstract boolean isEmpty();

    /**
     * Verify the given layer 4 match.
     *
     * @param vl4  A {@link VtnLayer4Match} instance.
     */
    public abstract void verify(VtnLayer4Match vl4);
}
