/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import javax.annotation.Nonnull;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.base.Optional;
import com.google.common.util.concurrent.FutureCallback;
import com.google.common.util.concurrent.Futures;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.concurrent.SettableVTNFuture;
import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.GetMacMappedHostInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.GetMacMappedHostOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.GetMacMappedHostOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.get.mac.mapped.host.output.MacMappedHost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.get.mac.mapped.host.output.MacMappedHostBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.status.MappedHost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.mappable.MacMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;

/**
 * {@code GetMacMappedHostFuture} describes a future associated with the task
 * that implements get-mac-mapped-host RPC.
 */
public final class GetMacMappedHostFuture
    extends SettableVTNFuture<List<MacMappedHost>>
    implements FutureCallback<Optional<Vbridge>>,
               RpcOutputGenerator<List<MacMappedHost>, GetMacMappedHostOutput> {
    /**
     * A logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(GetMacMappedHostFuture.class);

    /**
     * MD-SAL transaction context.
     */
    private final TxContext  context;

    /**
     * The identifier for the target vBridge.
     */
    private final VBridgeIdentifier  bridgeId;

    /**
     * The number of background tasks that read MAC address table entries.
     */
    private int  taskCount;

    /**
     * A list of hosts mapped by the MAC mapping.
     */
    private List<MacMappedHost>  mappedHosts;

    /**
     * Callback for the read operation that reads host information in the
     * MAC address table.
     */
    private final class ReadMacTableCallback
        implements FutureCallback<Optional<MacTableEntry>> {
        /**
         * A {@link MappedHost} instance that specifies the target host.
         */
        private final MappedHost  targetHost;

        /**
         * Construct a new instance.
         *
         * @param host  A {@link MappedHost} instance.
         */
        private ReadMacTableCallback(MappedHost host) {
            targetHost = host;
        }

        // FutureCallback
        /**
         * Invoked when the operation that reads the target host has completed
         * successfully.
         *
         * @param result  An {@link Optional} instance that contains the target
         *                host information.
         */
        @Override
        public void onSuccess(@Nonnull Optional<MacTableEntry> result) {
            if (result.isPresent()) {
                addHost(result.get());
            } else {
                // This should never happen.
                LOG.warn("{}: MAC mapped host is not found: host={}",
                         bridgeId, targetHost);
                addHost(null);
            }
        }

        /**
         * Invoked when the read operation has failed.
         *
         * @param cause  A {@link Throwable} that indicates the cause of
         *               failure.
         */
        @Override
        public void onFailure(Throwable cause) {
            setFailure(cause);
        }
    }

    /**
     * Construct a new instance.
     *
     * @param ctx    MD-SAL datastore transaction context.
     * @param input  A {@link GetMacMappedHostInput} instance.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    public GetMacMappedHostFuture(TxContext ctx, GetMacMappedHostInput input)
        throws RpcException {
        context = ctx;

        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        // Determine the target vBridge.
        VBridgeIdentifier vbrId = VBridgeIdentifier.create(input, true);
        bridgeId = vbrId;

        // Read the specified vBridge.
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        ReadTransaction rtx = ctx.getTransaction();
        Futures.addCallback(rtx.read(oper, vbrId.getIdentifier()), this);
    }

    /**
     * Set the result of this future.
     *
     * @param hosts  A list of {@link MacMappedHost} instances.
     *               {@code null} indicates that the MAC mapping is not
     *               configured in the target vBridge.
     */
    private void setResult(List<MacMappedHost> hosts) {
        set(hosts);
        cancelTransaction();
    }

    /**
     * Set the cause of failure.
     *
     * @param cause  A {@link Throwable} that indicates the cause of failure.
     */
    private void setFailure(Throwable cause) {
        setException(cause);
        cancelTransaction();
    }

    /**
     * Cancel the transaction if all the background tasks completed.
     */
    private void cancelTransaction() {
        boolean completed;
        synchronized (this) {
            completed = (taskCount == 0);
        }

        if (completed) {
            context.cancelTransaction();
        }
    }

    /**
     * Add the given host information to the list of mapped hosts.
     *
     * @param host  A {@link  MacTableEntry} instance.
     *              {@code null} indicates the target host is not found in the
     *              MAC address table.
     */
    private void addHost(MacTableEntry host) {
        MacMappedHost mhost = (host == null)
            ? null : new MacMappedHostBuilder(host).build();
        List<MacMappedHost> result;

        synchronized (this) {
            if (mhost != null) {
                mappedHosts.add(mhost);
            }

            assert taskCount > 0;
            taskCount--;
            if (taskCount == 0) {
                result = mappedHosts;
                mappedHosts = null;
            } else {
                result = null;
            }
        }

        if (result != null) {
            setResult(result);
        }
    }

    /**
     * Read MAC address table entries in background.
     *
     * @param hosts  A list of {@link MappedHost} instances.
     */
    private void readMacTable(List<MappedHost> hosts) {
        // Set up the number of background tasks.
        int count = hosts.size();
        List<MacMappedHost> result = new ArrayList<>(count);
        synchronized (this) {
            taskCount = count;
            mappedHosts = result;
        }

        // Read MAC address table entries.
        try {
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            ReadTransaction rtx = context.getTransaction();
            for (MappedHost host: hosts) {
                ReadMacTableCallback cb = new ReadMacTableCallback(host);
                InstanceIdentifier<MacTableEntry> path = VBridgeIdentifier.
                    getMacEntryPath(bridgeId, host.getMacAddress());
                Futures.addCallback(rtx.read(oper, path), cb);
                count--;
            }
        } catch (RuntimeException e) {
            context.log(LOG, VTNLogLevel.ERROR, e,
                        "%s: Failed to launch background tasks: %s",
                        bridgeId, e);
            setFailure(e);

            // Adjust task count.
            synchronized (this) {
                assert taskCount >= count;
                taskCount -= count;
            }
            cancelTransaction();
        }
    }

    // FutureCallback

    /**
     * Invoked when the operation that reads the target vBridge has completed
     * successfully.
     *
     * @param result  An {@link Optional} instance that contains the target
     *                vBridge.
     */
    @Override
    public void onSuccess(@Nonnull Optional<Vbridge> result) {
        if (result.isPresent()) {
            MacMap mmp = result.get().getMacMap();
            if (mmp == null) {
                // MAC mapping is not configured in the target vBridge.
                setResult(null);
            } else {
                List<MappedHost> hosts = mmp.getMacMapStatus().getMappedHost();
                if (MiscUtils.isEmpty(hosts)) {
                    // No host is mapped by the MAC mapping.
                    setResult(Collections.<MacMappedHost>emptyList());
                } else {
                    // Read MAC address table entries associated with hosts
                    // mapped by the target MAC mapping.
                    readMacTable(hosts);
                }
            }
        } else {
            setFailure(bridgeId.getNotFoundException());
        }
    }

    /**
     * Invoked when the read operation has failed.
     *
     * @param cause  A {@link Throwable} that indicates the cause of failure.
     */
    @Override
    public void onFailure(Throwable cause) {
        setFailure(cause);
    }

    // RpcOutputGenerator


    /**
     * {@inheritDoc}
     */
    @Override
    public Class<GetMacMappedHostOutput> getOutputType() {
        return GetMacMappedHostOutput.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public GetMacMappedHostOutput createOutput(
        List<MacMappedHost> result) {
        // Null list means that the MAC mapping is not configured.
        // An empty list is passed if the MAC mapping is configured and no host
        // is mapped by the MAC mapping.
        boolean configured = (result != null);

        GetMacMappedHostOutputBuilder builder =
            new GetMacMappedHostOutputBuilder();
        if (!MiscUtils.isEmpty(result)) {
            builder.setMacMappedHost(result);
        }

        return builder.setConfigured(configured).build();
    }
}
