/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.ConfigFileUpdater;
import org.opendaylight.vtn.manager.internal.util.XmlConfigFile;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantIdentifier;
import org.opendaylight.vtn.manager.internal.vnode.xml.XmlVTenant;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.bridge.status.FaultedPaths;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeConfig;

/**
 * {@code VTenantChange} describes changes to the VTN models.
 *
 * <p>
 *   Note that this class is not synchronized.
 * </p>
 */
final class VTenantChange extends ConfigFileUpdater<String, XmlVTenant> {
    /**
     * VTN Manager provider service.
     */
    private final VTNManagerProvider  vtnProvider;

    /**
     * The vBridge manager instance.
     */
    private final VBridgeManager  vBridgeManager;

    /**
     * A map that keeps updated VTN models.
     */
    private Map<String, Vtn>  updatedVtns = new HashMap<>();

    /**
     * A set of created VTN names.
     */
    private Set<String>  createdVtns = new HashSet<>();

    /**
     * A set of VTN names to be saved.
     */
    private Set<String>  targetVtns = new HashSet<>();

    /**
     * A set of resolved faulted paths in a vBridge or vTerminal.
     */
    private Map<VNodeIdentifier<?>, List<FaultedPaths>>  resolvedPathFaults =
        new HashMap<>();

    /**
     * Convert the given {@link Vtn} instance into a {@link XmlVTenant}
     * instance.
     *
     * @param vtn     A {@link Vtn} instance.
     * @param logger  A {@link Logger} instance.
     * @param label   A label only for logging.
     * @return  A {@link XmlVTenant} instance on success.
     *          {@code null} on failure.
     */
    private static XmlVTenant toXmlVTenant(Vtn vtn, Logger logger,
                                           Object label) {
        try {
            return new XmlVTenant(vtn);
        } catch (Exception e) {
            String msg = String.valueOf(label) + ": Ignore broken event.";
            logger.error(msg, e);
        }

        return null;
    }

    /**
     * MD-SAL transaction task to save current VTN configuration.
     */
    private static final class SaveTask extends AbstractTxTask<Void> {
        /**
         * A set of VTN names.
         */
        private final Set<String>  nameSet;

        /**
         * A logger instance.
         */
        private final Logger  logger;

        /**
         * Construct a new instance.
         *
         * @param names  A set of VTN names.
         * @param log  A {@link Logger} instance.
         */
        private SaveTask(Set<String> names, Logger log) {
            nameSet = names;
            logger = log;
        }

        /**
         * Save configuration for the VTN specified by the given name.
         *
         * @param rtx   A {@link ReadTransaction} instance.
         * @param name  The name of the VTN.
         * @throws VTNException  An error occurred.
         */
        private void save(ReadTransaction rtx, String name)
            throws VTNException {
            VTenantIdentifier vtnId = VTenantIdentifier.create(name, true);
            Vtn vtn = vtnId.fetch(rtx);
            XmlVTenant xvtn = VTenantChange.toXmlVTenant(vtn, logger, "ASYNC");
            if (xvtn == null) {
                return;
            }

            XmlConfigFile.Type ftype = XmlConfigFile.Type.VTN;
            if (!XmlConfigFile.save(ftype, name, xvtn)) {
                logger.warn("{}: Failed to save VTN configuration.",
                            name);
            } else if (logger.isTraceEnabled()) {
                logger.trace(
                    "{}: VTN configuration has been saved asynchronously.",
                    name);
            }
        }

        // AbstractTxTask

        /**
         * {@inheritDoc}
         */
        @Override
        public Void execute(TxContext ctx) throws VTNException {
            ReadTransaction rtx = ctx.getTransaction();
            for (String name: nameSet) {
                try {
                    save(rtx, name);
                } catch (Exception e) {
                    logger.error(name + ": Failed to save VTN configuration.",
                                 e);
                }
            }

            return null;
        }
    }

    /**
     * Construct a new instance.
     *
     * @param provider  A VTN Manager provider service.
     * @param vbmgr      The vBridge manager instance.
     */
    VTenantChange(VTNManagerProvider provider, VBridgeManager vbmgr) {
        super(XmlConfigFile.Type.VTN, XmlConfigFile.Type.VTN.name());
        vtnProvider = provider;
        vBridgeManager = vbmgr;
    }

    /**
     * Add an updated VTN.
     *
     * @param name     The name of the VTN.
     * @param vtn      A {@link Vtn} instance.
     * @param created  {@code true} means that the given data has been newly
     *                 created.
     */
    public void addUpdatedVtn(String name, Vtn vtn, boolean created) {
        updatedVtns.put(name, vtn);
        if (created) {
            // VTN configuration should be saved on its creation.
            targetVtns.add(name);
            createdVtns.add(name);
        }
    }

    /**
     * Add the name of the VTN to be saved.
     *
     * @param name  The name of the VTN.
     */
    public void addTargetVtn(String name) {
        targetVtns.add(name);
    }

    /**
     * Update the entity of the specified vBridge.
     *
     * <p>
     *   The entity of the specified vBridge will be created if not present.
     *   If present, the specified configuration will be applied to the
     *   existing entity of the vBridge.
     * </p>
     *
     * @param ident   The identifier for the target vBridge.
     * @param config  The configuration of the vBridge.
     */
    public void updateBridge(VBridgeIdentifier ident, VbridgeConfig config) {
        vBridgeManager.updateEntity(ident, config);
    }

    /**
     * Remove the entity of the specified vBridge.
     *
     * @param ident   The identifier for the target vBridge.
     */
    public void removeBridge(VBridgeIdentifier ident) {
        vBridgeManager.removeEntity(ident);
    }

    /**
     * Add the resolved faulted path in a virtual bridge.
     *
     * @param ident  A {@link VNodeIdentifier} instance that specifies the
     *               virtual bridge.
     * @param path   A {@link FaultedPaths} instance that specifies a resolved
     *               path fault.
     */
    public void addResolvedPathFault(VNodeIdentifier<?> ident,
                                     FaultedPaths path) {
        List<FaultedPaths> list = resolvedPathFaults.get(ident);
        if (list == null) {
            list = new ArrayList<>();
            resolvedPathFaults.put(ident, list);
        }

        list.add(path);
    }

    /**
     * Return a list of resolved path faults on the given virtual bridge.
     *
     * @param ident  A {@link VNodeIdentifier} instance that specifies the
     *               virtual bridge.
     * @return  A list of resolved path faults.
     */
    public List<FaultedPaths> getResolvedPathFaults(VNodeIdentifier<?> ident) {
        List<FaultedPaths> list = resolvedPathFaults.get(ident);
        return (list == null)
            ? Collections.<FaultedPaths>emptyList()
            : Collections.unmodifiableList(list);
    }

    // AbstractConfigFileUpdater

    /**
     * Fix up changes before applying.
     *
     * @param logger  A {@link Logger} instance.
     */
    @Override
    protected void fixUp(Logger logger) {
        // Create VTN configurations to be saved.
        Set<String> pending = new HashSet<>();
        for (Map.Entry<String, Vtn> entry: updatedVtns.entrySet()) {
            String name = entry.getKey();
            if (isRemoved(name) || !targetVtns.contains(name)) {
                continue;
            }

            Vtn vtn = entry.getValue();
            if (vtn == null) {
                pending.add(name);
            } else {
                boolean created = createdVtns.contains(name);
                VtnUpdateType type = (created)
                    ? VtnUpdateType.CREATED : VtnUpdateType.CHANGED;
                XmlVTenant xvtn =
                    VTenantChange.toXmlVTenant(vtn, logger, type);
                if (xvtn != null) {
                    addUpdated(name, xvtn, created);
                }
            }
        }

        if (!pending.isEmpty()) {
            // Save current VTN configurations on background.
            vtnProvider.post(new SaveTask(pending, logger));
        }
    }
}
