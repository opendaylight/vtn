/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.opendaylight.vtn.manager.internal.cluster.MacTableEntry;

/**
 * {@link MacTableEntryFilter} provides interfaces to be implemented by
 * classes which filter MAC address table entry.
 */
public interface MacTableEntryFilter {
    /**
     * Test if the specified MAC address table entry should be accepted
     * or not.
     *
     * @param tent  A {@link MacTableEntry} instance to be tested.
     * @return  {@code true} if the specified entry should be accepted.
     *          {@code false} if the specified entry should be filtered out.
     */
    boolean accept(MacTableEntry tent);
}
