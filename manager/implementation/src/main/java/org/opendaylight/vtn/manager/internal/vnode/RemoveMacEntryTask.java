/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.tx.CompositeTxTask;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.RemoveMacEntryOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.RemoveMacEntryOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.remove.mac.entry.output.RemoveMacEntryResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.remove.mac.entry.output.RemoveMacEntryResultBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntry;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * {@code RemoveMacEntryTask} describes the MD-SAL datastore transaction task
 * that removes the given MAC addresses from the MAC address table in the
 * specified vBridge.
 *
 * @see #create(VBridgeIdentifier, List)
 */
public final class RemoveMacEntryTask
    extends CompositeTxTask<VtnUpdateType, RemoveMacTask>
    implements RpcOutputGenerator<List<VtnUpdateType>, RemoveMacEntryOutput> {
    /**
     * The identifier for the target vBridge.
     */
    private final VBridgeIdentifier  identifier;

    /**
     * Construct a new task that removes all the given MAC addresses from
     * the MAC address table.
     *
     * @param ident  A {@link VBridgeIdentifier} instance that specifies the
     *               target vBridge.
     * @param addrs  A list of MAC addresses to be removed.
     *               The caller must ensure that the list is not empty.
     * @return  A {@link RemoveMacEntryTask} instance associated with the task
     *          that removes the given MAC addresses from the MAC address
     *          table.
     * @throws RpcException  An error occurred.
     */
    public static RemoveMacEntryTask create(
        VBridgeIdentifier ident, List<MacAddress> addrs) throws RpcException {
        Set<MacAddress> macSet = new HashSet<>();
        List<RemoveMacTask> taskList = new ArrayList<>();
        for (MacAddress mac: addrs) {
            // Verify the MAC address.
            if (macSet.add(MiscUtils.verify(mac))) {
                InstanceIdentifier<MacTableEntry> path =
                    VBridgeIdentifier.getMacEntryPath(ident, mac);
                taskList.add(new RemoveMacTask(path, mac));
            }
        }

        return new RemoveMacEntryTask(ident, taskList);
    }

    /**
     * Construct a new instance.
     *
     * @param ident  A {@link VBridgeIdentifier} instance that specifies the
     *               target vBridge.
     * @param tasks  A list of tasks that delete MAC address table entries.
     */
    private RemoveMacEntryTask(VBridgeIdentifier ident,
                               List<RemoveMacTask> tasks) {
        super(tasks);
        identifier = ident;
    }

    // CompositeTxTask

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onStarted(TxContext ctx) throws VTNException {
        // Ensure that the target vBridge is present.
        identifier.fetch(ctx.getReadWriteTransaction());
    }

    // RpcOutputGenerator

    /**
     * {@inheritDoc}
     */
    @Override
    public Class<RemoveMacEntryOutput> getOutputType() {
        return RemoveMacEntryOutput.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public RemoveMacEntryOutput createOutput(List<VtnUpdateType> result) {
        List<RemoveMacEntryResult> list = new ArrayList<>();
        Iterator<RemoveMacTask> taskIterator = getSubTasks().iterator();
        for (VtnUpdateType status: result) {
            RemoveMacTask task = taskIterator.next();
            RemoveMacEntryResult res = new RemoveMacEntryResultBuilder().
                setMacAddress(task.getMacAddress()).
                setStatus(status).
                build();
            list.add(res);
        }

        return new RemoveMacEntryOutputBuilder().
            setRemoveMacEntryResult(list).build();
    }
}
