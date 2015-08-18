/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.HashSet;
import java.util.Set;

import org.opendaylight.vtn.manager.internal.cluster.MacVlan;

/**
 * An instance of {@code MacMapChange} class keeps differences to be applied
 * to MAC mapping configuration.
 */
public final class MacMapChange {
    /**
     * A flag bit which indicates the MAC mapping will be removed.
     */
    public static final int  REMOVING = 0x1;

    /**
     * A flag bit which indicates obsolete network caches do not need to be
     * purged.
     */
    public static final int  DONT_PURGE = 0x2;

    /**
     * A set of {@link MacVlan} instances to be added to the allowed host set.
     */
    private final Set<MacVlan>  allowAddedSet;

    /**
     * A set of {@link MacVlan} instances to be removed from the allowed host
     * set.
     */
    private final Set<MacVlan>  allowRemovedSet;

    /**
     * A set of {@link MacVlan} instances to be added to the denied host set.
     */
    private final Set<MacVlan>  denyAddedSet;

    /**
     * A set of {@link MacVlan} instances to be removed from the denied host
     * set.
     */
    private final Set<MacVlan>  denyRemovedSet;

    /**
     * A bitwise OR-ed flags.
     */
    private final int  flagBits;

    /**
     * Construct a new instance used to change MAC mapping configuration.
     *
     * <ul>
     *   <li>
     *     The same {@link MacVlan} instance must not be contained in
     *     both {@code allowAdded} and {@code allowRemoved}.
     *   </li>
     *   <li>
     *     The same {@link MacVlan} instance must not be contained in
     *     both {@code denyAdded} and {@code denyRemoved}.
     *   </li>
     *   <li>
     *     {@code flags} must be zero or the bitwise OR of oe or more of the
     *     following flags.
     *     <ul>
     *       <li>{@link #REMOVING}</li>
     *       <li>{@link #DONT_PURGE}</li>
     *     </ul>
     *   </li>
     * </ul>
     *
     * @param allowAdded    A set of {@link MacVlan} instances to be added
     *                      to the allowed host set.
     * @param allowRemoved  A set of {@link MacVlan} instances to be removed
     *                      from the allowed host set.
     * @param denyAdded     A set of {@link MacVlan} instances to be added
     *                      to the denied host set.
     * @param denyRemoved   A set of {@link MacVlan} instances to be removed
     *                      from the denied host set.
     * @param flags         A bitwise OR-ed flags.
     */
    public MacMapChange(Set<MacVlan> allowAdded, Set<MacVlan> allowRemoved,
                        Set<MacVlan> denyAdded, Set<MacVlan> denyRemoved,
                        int flags) {
        allowAddedSet = allowAdded;
        allowRemovedSet = allowRemoved;
        denyAddedSet = denyAdded;
        denyRemovedSet = denyRemoved;
        flagBits = flags;
    }

    /**
     * Construct a new instance used to resume or destroy MAC mapping.
     *
     * <ul>
     *   <li>
     *     {@code flags} must be zero or the bitwise OR of oe or more of the
     *     following flags.
     *     <ul>
     *       <li>{@link #REMOVING}</li>
     *       <li>{@link #DONT_PURGE}</li>
     *     </ul>
     *   </li>
     * </ul>
     *
     * @param allow  A set of {@link MacVlan} instances to be added to the
     *               allowed host set.
     * @param deny   A set of {@link MacVlan} instances to be added to the
     *               denied host set.
     * @param flags  A bitwise OR-ed flags.
     */
    public MacMapChange(Set<MacVlan> allow, Set<MacVlan> deny, int flags) {
        flagBits = flags;

        Set<MacVlan> empty = new HashSet<MacVlan>();
        if ((flags & REMOVING) != 0) {
            allowAddedSet = empty;
            allowRemovedSet = allow;
            denyAddedSet = empty;
            denyRemovedSet = deny;
        } else {
            allowAddedSet = allow;
            allowRemovedSet = empty;
            denyAddedSet = deny;
            denyRemovedSet = empty;
        }
    }

    /**
     * Return a set of {@link MacVlan} instances to be added to the allowed
     * host set.
     *
     * @return  A set of {@link MacVlan} instances to be added to the allowed
     *          host set.
     */
    public Set<MacVlan> getAllowAddedSet() {
        return allowAddedSet;
    }

    /**
     * Return a set of {@link MacVlan} instances to be removed from the allowed
     * host set.
     *
     * @return  A set of {@link MacVlan} instances to be removed from the
     *          allowed host set.
     */
    public Set<MacVlan> getAllowRemovedSet() {
        return allowRemovedSet;
    }

    /**
     * Return a set of {@link MacVlan} instances to be added to the denied
     * host set.
     *
     * @return  A set of {@link MacVlan} instances to be added to the denied
     *          host set.
     */
    public Set<MacVlan> getDenyAddedSet() {
        return denyAddedSet;
    }

    /**
     * Return a set of {@link MacVlan} instances to be removed from the denied
     * host set.
     *
     * @return  A set of {@link MacVlan} instances to be removed from the
     *          denied host set.
     */
    public Set<MacVlan> getDenyRemovedSet() {
        return denyRemovedSet;
    }

    /**
     * Determine whether the MAC mapping is going to be removed or not.
     *
     * @return  {@code true} is returned only if the MAC mapping is going to
     *          be removed.
     */
    public boolean isRemoving() {
        return ((flagBits & REMOVING) != 0);
    }

    /**
     * Determine whether obsolete network caches does not need to be purged
     * or not.
     *
     * @return  {@code true} is returned only if network caches does not need
     *          to be purged.
     */
    public boolean dontPurge() {
        return ((flagBits & DONT_PURGE) != 0);
    }
}
