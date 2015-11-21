/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import org.opendaylight.vtn.manager.internal.util.tx.DeleteDataTask;

import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntry;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * {@code RemoveMacTask} describes the MD-SAL datastore transaction task
 * that deletes the specified MAC address table entry.
 *
 * <p>
 *   A task corresponding to this task is used as a sub task for
 *   {@link RemoveMacEntryTask}.
 * </p>
 *
 * @see RemoveMacEntryTask
 */
public final class RemoveMacTask extends DeleteDataTask<MacTableEntry> {
    /**
     * The MAC address to be removed.
     */
    private final MacAddress  macAddress;

    /**
     * Construct a new instance.
     *
     * @param path  Path to the target MAC address table entry.
     * @param mac   The MAC address to be removed.
     */
    RemoveMacTask(InstanceIdentifier<MacTableEntry> path, MacAddress mac) {
        super(LogicalDatastoreType.OPERATIONAL, path);
        macAddress = mac;
    }

    /**
     * Return the target MAC address.
     *
     * @return  A MAC address to be removed.
     */
    public MacAddress getMacAddress() {
        return macAddress;
    }
}
