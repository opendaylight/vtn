/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.config;

import java.net.NetworkInterface;
import java.util.Enumeration;
import java.util.Map;
import java.util.TreeMap;
import java.util.concurrent.TimeoutException;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicReference;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.base.Optional;
import com.google.common.util.concurrent.FutureCallback;
import com.google.common.util.concurrent.Futures;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxQueue;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.VTNConfig;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.VTNEntityType;
import org.opendaylight.vtn.manager.internal.util.XmlConfigFile;
import org.opendaylight.vtn.manager.internal.util.concurrent.SettableVTNFuture;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;
import org.opendaylight.vtn.manager.internal.util.tx.TxQueueImpl;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.clustering.EntityOwnershipChange;
import org.opendaylight.controller.md.sal.common.api.clustering.EntityOwnershipListener;
import org.opendaylight.controller.md.sal.common.api.clustering.EntityOwnershipListenerRegistration;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.config.rev150209.VtnConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.config.rev150209.VtnConfigBuilder;

/**
 * VTN global configuration manager.
 */
public final class VTNConfigManager implements AutoCloseable, VTNConfig {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VTNConfigManager.class);

    /**
     * Instance identifier of the global configuration.
     */
    static final InstanceIdentifier<VtnConfig> CONFIG_IDENT =
        InstanceIdentifier.create(VtnConfig.class);

    /**
     * The key associated with the global configuration file.
     */
    static final String  KEY_VTN_CONFIG = "vtn-config";

    /**
     * The current configuration.
     */
    private final AtomicReference<VTNConfigImpl>  current;

    /**
     * A MD-SAL datastore transaction queue for the global configuration.
     */
    private final AtomicReference<TxQueueImpl>  configQueue =
        new AtomicReference<TxQueueImpl>();

    /**
     * Listener of VTN configuration for configuration view.
     */
    private final AtomicReference<ConfigListener>  confListener =
        new AtomicReference<ConfigListener>();

    /**
     * Listener of VTN configuration for operational view.
     */
    private final AtomicReference<OperationalListener>  operListener =
        new AtomicReference<OperationalListener>();

    /**
     * Set true if the local node is configuration provider.
     */
    private boolean  configProvider;

    /**
     * MD-SAL transaction task that loads VTN configuration from file.
     *
     * <p>
     *   This task returns {@code null} as the current value of "init-state".
     * </p>
     */
    private static final class ConfigLoadTask extends AbstractTxTask<Boolean> {
        /**
         * MAC address of the local node.
         */
        private final EtherAddress  macAddress;

        /**
         * Construct a new instance.
         *
         * @param mac  MAC address of the local node.
         */
        private ConfigLoadTask(EtherAddress mac) {
            macAddress = mac;
        }

        // AbstractTxTask

        /**
         * {@inheritDoc}
         */
        @Override
        public Boolean execute(TxContext ctx) throws VTNException {
            LogicalDatastoreType cstore = LogicalDatastoreType.CONFIGURATION;
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            VtnConfig vcfg = null;

            // Try to load configuration from file.
            VTNConfigImpl vconf = XmlConfigFile.load(
                XmlConfigFile.Type.CONFIG, KEY_VTN_CONFIG,
                VTNConfigImpl.class);
            if (vconf != null) {
                // Initialize the VTN configuration in configuration view.
                vcfg = vconf.getJaxbValue().build();
                tx.put(cstore, CONFIG_IDENT, vcfg, true);
            } else {
                // Delete configuration.
                DataStoreUtils.delete(tx, cstore, CONFIG_IDENT);
            }

            // Initialize the VTN configuration in operational view.
            VtnConfigBuilder builder = VTNConfigImpl.builder(vcfg, macAddress);

            // Set false to init-state to notify that the configuration
            // is initializing.
            builder.setInitState(Boolean.FALSE);
            LogicalDatastoreType ostore = LogicalDatastoreType.OPERATIONAL;
            tx.put(ostore, CONFIG_IDENT, builder.build(), true);

            return null;
        }
    }

    /**
     * MD-SAL transaction task that saves VTN configuration into file.
     *
     * <p>
     *   This task returns the current value of "init-state".
     * </p>
     */
    private static final class ConfigSaveTask extends AbstractTxTask<Boolean> {
        /**
         * A {@link VTNConfigImpl} instance to be saved into file.
         */
        private VTNConfigImpl  saveConfig;

        // AbstractTxTask

        /**
         * {@inheritDoc}
         */
        @Override
        public Boolean execute(TxContext ctx) throws VTNException {
            saveConfig = null;

            // Read current VTN configuration in configuration view.
            LogicalDatastoreType cstore = LogicalDatastoreType.CONFIGURATION;
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            Optional<VtnConfig> opt =
                DataStoreUtils.read(tx, cstore, CONFIG_IDENT);
            if (opt.isPresent()) {
                // Save current configuration into file.
                saveConfig = new VTNConfigImpl(opt.get());
            }

            // Read current VTN configuration in operational view.
            Boolean ret;
            LogicalDatastoreType ostore = LogicalDatastoreType.OPERATIONAL;
            opt = DataStoreUtils.read(tx, ostore, CONFIG_IDENT);
            if (opt.isPresent()) {
                ret = opt.get().isInitState();
            } else {
                // The owner is now initializing the configuration.
                LOG.debug("VTN configuration is not present.");
                ret = Boolean.FALSE;
            }

            return ret;
        }

        // TxTask

        /**
         * {@inheritDoc}
         */
        @Override
        public void onSuccess(VTNManagerProvider provider, Boolean result) {
            if (saveConfig != null) {
                XmlConfigFile.save(XmlConfigFile.Type.CONFIG, KEY_VTN_CONFIG,
                                   saveConfig);
            }
        }
    }

    /**
     * A futute associated with the task that initializes the VTN
     * configuration.
     *
     * <p>
     *   This future returns the current value of "init-state".
     * </p>
     */
    private static final class ConfigInitFuture
        extends SettableVTNFuture<Boolean>
        implements EntityOwnershipListener, FutureCallback<Boolean> {
        /**
         * A MD-SAL datastore transaction queue for the global configuration.
         */
        private final TxQueue  txQueue;

        /**
         * MAC address of the local node.
         */
        private final EtherAddress  macAddress;

        /**
         * Registration of the VTN configuration entity ownership listener.
         */
        private final EntityOwnershipListenerRegistration  listener;

        /**
         * A boolean value that determines whether the VTN configuration is
         * initialized or not.
         */
        private final AtomicBoolean  initialized = new AtomicBoolean();

        /**
         * Construct a new instance.
         *
         * @param provider  VTN Manager provider service.
         * @param cfq       The DS task queue for the VTN configuration.
         * @param mac       MAC address of the local node.
         */
        private ConfigInitFuture(VTNManagerProvider provider, TxQueue cfq,
                                 EtherAddress mac) {
            txQueue = cfq;
            macAddress = mac;

            // Register a listener in order to determine the ownership of the
            // VTN configuration.
            listener = provider.registerListener(VTNEntityType.CONFIG, this);
        }

        // EntityOwnershipListener

        /**
         * Invoked when the ownership status of the VTN configuration has been
         * changed.
         *
         * @param change  An {@link EntityOwnershipChange} instance.
         */
        @Override
        public void ownershipChanged(EntityOwnershipChange change) {
            LOG.info("Received VTN config ownership change: {}", change);

            if (change.hasOwner() && !initialized.getAndSet(true)) {
                // Start DS task that initializes the VTN configuration.
                TxTask<Boolean> task = (change.isOwner())
                    ? new ConfigLoadTask(macAddress)
                    : new ConfigSaveTask();
                Futures.addCallback(txQueue.postFirst(task), this);
            }
        }

        // FutureCallback

        /**
         * Invoked when the VTN configuration has been initialized
         * successfully.
         *
         * @param result  A {@link Boolean} value returned by the DS task.
         */
        @Override
        public void onSuccess(Boolean result) {
            listener.close();
            set(result);
        }

        /**
         * Invoked when a throwable was caught during initialization.
         *
         * @param cause  A {@link Throwable} that indicates the cause of
         *               failure.
         */
        @Override
        public void onFailure(Throwable cause) {
            listener.close();
            setException(cause);
        }
    }

    /**
     * MD-SAL transaction task to change "init-state" to true.
     */
    private static class ConfigInitDoneTask extends AbstractTxTask<Void> {
        /**
         * {@inheritDoc}
         */
        @Override
        public Void execute(TxContext ctx) throws VTNException {
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            LogicalDatastoreType ostore = LogicalDatastoreType.OPERATIONAL;
            VtnConfigBuilder builder = new VtnConfigBuilder().
                setInitState(Boolean.TRUE);
            tx.merge(ostore, CONFIG_IDENT, builder.build(), true);
            return null;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void onSuccess(VTNManagerProvider provider, Void result) {
            LOG.info("VTN configuration has been initialized.");
        }
    }

    /**
     * Determine the MAC address of the local controller.
     *
     * @return  An {@link EtherAddress} which represents the MAC address of
     *          the controller. {@code null} on failure.
     */
    static EtherAddress getLocalMacAddress() {
        Enumeration<NetworkInterface> nifs = null;
        try {
            nifs = NetworkInterface.getNetworkInterfaces();
        } catch (Exception e) {
            LOG.warn("Failed to get network interfaces.", e);
            return null;
        }

        EtherAddress ea = getMacAddress(nifs);
        if (ea == null) {
            LOG.warn("No network interface was found.");
        }

        return ea;
    }

    /**
     * Get a MAC address configured in the given interface.
     *
     * @param nifs  An enumeration of  {@link NetworkInterface} instances.
     * @return  An {@link EtherAddress} which represents the MAC address of
     *          the controller. {@code null} on failure.
     */
    private static EtherAddress getMacAddress(
        Enumeration<NetworkInterface> nifs) {
        if (nifs == null) {
            return null;
        }

        // Sort interfaces by index.
        // This code expects that lower index is assigned to physical network
        // interface.
        Map<Integer, NetworkInterface> niMap = new TreeMap<>();
        while (nifs.hasMoreElements()) {
            NetworkInterface nif = nifs.nextElement();
            niMap.put(nif.getIndex(), nif);
        }

        NetworkInterface altIf = null;
        EtherAddress altAddr = null;
        for (NetworkInterface nif: niMap.values()) {
            try {
                if (nif.isLoopback() || nif.isVirtual() ||
                    nif.isPointToPoint()) {
                    continue;
                }

                byte[] mac = nif.getHardwareAddress();
                if (mac == null) {
                    continue;
                }

                EtherAddress ea = new EtherAddress(mac);
                if (nif.isUp()) {
                    LOG.debug("Use HW address of {} as local address: {}",
                              nif.getName(), ea.getText());
                    return ea;
                }

                if (altIf == null) {
                    altIf = nif;
                    altAddr = ea;
                }
            } catch (Exception e) {
                LOG.debug("Ignore network interface: " + nif.getName(), e);
            }
        }

        if (altAddr != null) {
            LOG.debug("Use inactive HW address of {} as local address: ",
                      altIf.getName(), altAddr.getText());
            return altAddr;
        }

        return null;
    }

    /**
     * Construct a new instance.
     *
     * @param provider  A VTN Manager provider service.
     */
    public VTNConfigManager(VTNManagerProvider provider) {
        XmlConfigFile.init();

        // Determine MAC address of the local node.
        EtherAddress ea = getLocalMacAddress();
        VTNConfigImpl vconf = new VTNConfigImpl(ea);
        if (ea == null) {
            ea = vconf.getControllerMacAddress();
            LOG.warn("Use custom MAC address as local address: {}",
                     ea.getText());
        } else {
            LOG.info("Local MAC address: {}", ea.getText());
        }
        current = new AtomicReference<VTNConfigImpl>(vconf);

        try {
            initialize(provider, ea);
        } catch (RuntimeException e) {
            LOG.error("Failed to initialize VTN config manager.", e);
            close();
            throw e;
        }
    }

    /**
     * Complete the initialization of global configuration.
     */
    public void initDone() {
        if (configProvider) {
            TxQueueImpl cfq = configQueue.get();
            if (cfq != null) {
                // Change init-state to true.
                ConfigInitDoneTask task = new ConfigInitDoneTask();
                cfq.post(task);
            }
        }
    }

    /**
     * Determine whether the local node is the configuration provider or not.
     *
     * @return  {@code true} if the local node is the configuration provider.
     *          Otherwise {@code false}.
     */
    public boolean isConfigProvider() {
        return configProvider;
    }

    // AutoCloseable

    /**
     * Close the configuration service.
     */
    @Override
    public void close() {
        ConfigListener cfg = confListener.getAndSet(null);
        if (cfg != null) {
            cfg.close();
        }

        OperationalListener op = operListener.getAndSet(null);
        if (op != null) {
            op.close();
        }

        TxQueueImpl cfq = configQueue.getAndSet(null);
        if (cfq != null) {
            cfq.close();
        }
    }

    /**
     * Initialize the VTN configuration.
     *
     * @param provider  A VTN Manager provider service.
     * @param mac       MAC address of the local node.
     */
    private void initialize(VTNManagerProvider provider, EtherAddress mac) {
        DataBroker broker = provider.getDataBroker();
        TxQueueImpl cfq = new TxQueueImpl("VTN Config", provider);
        configQueue.set(cfq);
        confListener.set(new ConfigListener(cfq, broker, mac));
        OperationalListener opl = new OperationalListener(broker, current);
        operListener.set(opl);

        // Post initialization task.
        ConfigInitFuture f = new ConfigInitFuture(provider, cfq, mac);

        // Start transaction queue processing.
        cfq.start();

        // Wait for the initialization task to complete.
        Boolean initState;
        try {
            initState = f.checkedGet();
        } catch (Exception e) {
            String msg = "Failed to initialize global configuration.";
            LOG.error(msg, e);
            throw new IllegalStateException(msg, e);
        }

        if (initState == null) {
            LOG.debug("The local node is configuration provider.");
            configProvider = true;
            return;
        }

        // We need to wait for another controller in the cluster to complete
        // initialization.
        long millis = current.get().getInitTimeout();
        try {
            opl.awaitConfig(millis);
        } catch (InterruptedException e) {
            String msg = "Initialization thread was interrupted.";
            LOG.error(msg, e);
            throw new IllegalStateException(msg, e);
        } catch (TimeoutException e) {
            LOG.warn("Initialization did not complete within {} milliseconds.",
                     millis);
            configProvider = true;
        }
    }

    // VTNConfig

    /**
     * {@inheritDoc}
     */
    @Override
    public int getTopologyWait() {
        return current.get().getTopologyWait();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getL2FlowPriority() {
        return current.get().getL2FlowPriority();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getFlowModTimeout() {
        return current.get().getFlowModTimeout();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getBulkFlowModTimeout() {
        return current.get().getBulkFlowModTimeout();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getInitTimeout() {
        return current.get().getInitTimeout();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getMaxRedirections() {
        return current.get().getMaxRedirections();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean isHostTracking() {
        return current.get().isHostTracking();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public EtherAddress getControllerMacAddress() {
        return current.get().getControllerMacAddress();
    }
}
