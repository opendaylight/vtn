/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock.impl;

import org.opendaylight.vtn.manager.it.ofmock.OfMockUtils;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnOpenflowVersion;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.Flow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.FlowCookie;

/**
 * {@code CookieMatcher} is used to select MD-SAL flow entries by a pair of
 * flow cookie and cookie mask.
 */
public final class CookieMatcher {
    /**
     * The cookie mask value that indicates the undefined value.
     */
    private static final long  COOKIE_UNDEF = 0L;

    /**
     * Cookie bits to be compared.
     */
    private final long  cookieBits;

    /**
     * The cookie mask value.
     */
    private final long  cookieMask;

    /**
     * {@code true} if this instance matches any cookie.
     */
    private final boolean  wildcard;

    /**
     * Construct a new instance.
     *
     * @param ofver   The OpenFlow protocol version used by the target switch.
     * @param cookie  The flow cookie.
     * @param mask    The flow cookie mask.
     */
    public CookieMatcher(VtnOpenflowVersion ofver, FlowCookie cookie,
                         FlowCookie mask) {
        cookieMask = getMask(ofver, mask);
        wildcard = (cookieMask == COOKIE_UNDEF);
        if (wildcard) {
            // Cookie is not specified.
            cookieBits = 0;
        } else {
            long lexp = OfMockUtils.getCookie(cookie).longValue();
            cookieBits = (lexp & cookieMask);
        }
    }

    /**
     * Determine whether the specified MD-SAL flow entry meets the condition
     * specified by this instance.
     *
     * @param flow  The MD-SAL flow entry to be tested.
     * @return  {@code true} if the specified flow entry meets the conditions.
     *          {@code false} otherwise.
     */
    public boolean match(Flow flow) {
        boolean result = wildcard;
        if (!result) {
            long cookie = OfMockUtils.getCookie(flow.getCookie()).longValue();
            result = ((cookie & cookieMask) == cookieBits);
        }
        return result;
    }

    /**
     * Return the cookie mask value in the specified {@link FlowCookie}
     * instance.
     *
     * @param ofver  The OpenFlow protocol version used by the target switch.
     * @param mask   The flow cookie mask.
     * @return  The flow cookie mask value.
     */
    private long getMask(VtnOpenflowVersion ofver, FlowCookie mask) {
        long lmask;
        if (ofver == VtnOpenflowVersion.OF10) {
            // Cookie mask is not supported.
            lmask = COOKIE_UNDEF;
        } else {
            lmask = OfMockUtils.getCookie(mask).longValue();
        }
        return lmask;
    }
}
