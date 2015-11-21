/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode.mac;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntry;

/**
 * {@code MacEntryFilter} provides interfaces to be implemented by classes
 * that filter MAC address table entries.
 */
public interface MacEntryFilter {
    /**
     * Test if the specified MAC address table entry should be accepted or not.
     *
     * @param ment  A {@link MacTableEntry} instance to be tested.
     * @return  {@code true} if the specified entry should be accepted.
     *          {@code false} if the specified entry should be filtered out.
     * @throws VTNException  An error occurred.
     */
    boolean accept(MacTableEntry ment) throws VTNException;
}
