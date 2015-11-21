/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier.getMacTablePath;

import java.util.ArrayList;
import java.util.List;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractRpcTask;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.RemoveMacEntryOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.RemoveMacEntryOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.remove.mac.entry.output.RemoveMacEntryResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.remove.mac.entry.output.RemoveMacEntryResultBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.list.MacAddressTable;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.list.MacAddressTableBuilder;

/**
 * {@code ClearMacEntryTask} describes the MD-SAL datastore transaction task
 * that removes all the MAC addresses learned by the specified vBridge.
 */
public final class ClearMacEntryTask
    extends AbstractRpcTask<List<RemoveMacEntryResult>>
    implements RpcOutputGenerator<List<RemoveMacEntryResult>,
                                  RemoveMacEntryOutput> {
    /**
     * The identifier for the target vBridge.
     */
    private final VBridgeIdentifier  identifier;

    /**
     * Construct a new instance.
     *
     * @param ident  A {@link VBridgeIdentifier} instance that specifies the
     *               target vBridge.
     */
    ClearMacEntryTask(VBridgeIdentifier ident) {
        identifier = ident;
    }

    // AbstractTxTask

    /**
     * Clear the specified MAC address table.
     *
     * @param ctx  A runtime context for transaction task.
     * @return  A list of {@link RemoveMacEntryResult} instances that contains
     *          all the removed MAC addresses.
     *          {@code null} if the specified MAC address table is empty.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected List<RemoveMacEntryResult> execute(TxContext ctx)
        throws VTNException {
        // Ensure that the target vBridge is present.
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        identifier.fetch(tx);

        // Read the MAC address table.
        List<RemoveMacEntryResult> result = null;
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        InstanceIdentifier<MacAddressTable> path = getMacTablePath(identifier);
        Optional<MacAddressTable> opt = DataStoreUtils.read(tx, oper, path);
        if (opt.isPresent()) {
            List<MacTableEntry> entries = opt.get().getMacTableEntry();
            if (!MiscUtils.isEmpty(entries)) {
                result = new ArrayList<>(entries.size());
                for (MacTableEntry mtent: entries) {
                    RemoveMacEntryResult r = new RemoveMacEntryResultBuilder().
                        setMacAddress(mtent.getMacAddress()).
                        setStatus(VtnUpdateType.REMOVED).
                        build();
                    result.add(r);
                }
            }

            // Put an empty MAC address table.
            MacAddressTable mtable = new MacAddressTableBuilder().
                setName(identifier.getBridgeNameString()).
                build();
            tx.put(oper, path, mtable, false);
        }

        return result;
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
    public RemoveMacEntryOutput createOutput(
        List<RemoveMacEntryResult> result) {
        return new RemoveMacEntryOutputBuilder().
            setRemoveMacEntryResult(result).build();
    }
}
