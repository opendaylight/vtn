/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Set;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.routing.xml.XmlPathMap;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.XmlConfigFile;
import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;
import org.opendaylight.vtn.manager.internal.util.pathmap.PathMapUtils;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTerminalIdentifier;
import org.opendaylight.vtn.manager.internal.vnode.xml.XmlLogger;
import org.opendaylight.vtn.manager.internal.vnode.xml.XmlVBridge;
import org.opendaylight.vtn.manager.internal.vnode.xml.XmlVTenant;
import org.opendaylight.vtn.manager.internal.vnode.xml.XmlVTerminal;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.VtnMappings;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.VtnMappingsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.Vtns;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.VtnsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtnPathMaps;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtnPathMapsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.MacTables;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.MacTablesBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.mac.tables.TenantMacTable;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.mac.tables.TenantMacTableBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.list.MacAddressTable;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.list.MacAddressTableBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;

/**
 * {@code VTenantLoadTask} describes a MD-SAL datastore transaction task that
 * loads the VTN configurations.
 *
 * <p>
 *   This task returns the root container of all VTNs.
 * </p>
 */
class VTenantLoadTask extends AbstractTxTask<Vtns> implements XmlLogger {
    /**
     * Logger instance.
     */
    private final Logger  logger =
        LoggerFactory.getLogger(VTenantLoadTask.class);

    /**
     * The virtual tenant manager.
     */
    private final VTenantManager  tenantManager;

    /**
     * The vBridge manager instance.
     */
    private final VBridgeManager  vBridgeManager;

    /**
     * Runtime context for MD-SAL datastore transaction.
     */
    private TxContext  context;

    /**
     * A set of loaded vBridge configurations.
     */
    private Map<VBridgeIdentifier, VbridgeConfig>  loadedBridges;

    /**
     * Construct a new instance.
     *
     * @param tmgr   The virtual tenant manager.
     * @param vbmgr  The vBridge manager.
     */
    VTenantLoadTask(VTenantManager tmgr, VBridgeManager vbmgr) {
        tenantManager = tmgr;
        vBridgeManager = vbmgr;
    }

    /**
     * Resume the VTN from the VTN configuration.
     *
     * @param ctx     A runtime context for transaction task.
     * @param tables  A list to store MAC address tables for resumed vBridges.
     * @param name    The name of the VTN.
     * @param xvtn    A {@link XmlVTenant} instance that contains the VTN
     *              configuration.
     * @return  A {@link Vtn} instance corresponding to the given VTN.
     * @throws VTNException  An error occurred.
     */
    private Vtn resume(TxContext ctx, List<TenantMacTable> tables, String name,
                       XmlVTenant xvtn) throws VTNException {
        VtnBuilder builder = xvtn.toVtnBuilder(this, name);

        // Resume vBridges.
        VTenantIdentifier vtnId = xvtn.getIdentifier();
        List<MacAddressTable> macTables = new ArrayList<>();
        resumeBridges(ctx, builder, macTables, vtnId, xvtn);

        if (macTables.isEmpty()) {
            macTables = null;
        }
        TenantMacTable table = new TenantMacTableBuilder().
            setName(name).
            setMacAddressTable(macTables).
            build();
        tables.add(table);

        // Resume vTerminals.
        resumeTerminals(ctx, builder, vtnId, xvtn);

        // Resume VTN path maps.
        resumePathMaps(ctx, builder, vtnId, xvtn);

        return builder.build();
    }

    /**
     * Resume all the vBridges in the given VTN.
     *
     * @param ctx      A runtime context for transaction task.
     * @param builder  A {@link VtnBuilder} instance to store resumed vBridges.
     * @param mtables  A list to store MAC address tables for resumed vBridges.
     * @param vtnId    The identifier for the VTN.
     * @param xvtn     A {@link XmlVTenant} instance that contains the VTN
     *                 configuration.
     */
    private void resumeBridges(TxContext ctx, VtnBuilder builder,
                               List<MacAddressTable> mtables,
                               VTenantIdentifier vtnId, XmlVTenant xvtn) {
        List<XmlVBridge> xvbrs = xvtn.getBridges();
        if (xvbrs != null) {
            List<Vbridge> blist = new ArrayList<>(xvbrs.size());
            Set<VnodeName> nameSet = new HashSet<>();
            for (XmlVBridge xvbr: xvbrs) {
                VnodeName vname = xvbr.getName();
                if (nameSet.add(vname)) {
                    VBridgeIdentifier vbrId =
                        new VBridgeIdentifier(vtnId, vname);
                    try {
                        VBridge vbr = new VBridge(vbrId);
                        Vbridge vbridge = vbr.resume(ctx, this, xvbr);
                        blist.add(vbridge);
                        loadedBridges.put(vbrId, vbridge.getVbridgeConfig());

                        // Prepare MAC address table for this vBridge.
                        MacAddressTable mtable = new MacAddressTableBuilder().
                            setName(vname.getValue()).
                            build();
                        mtables.add(mtable);
                    } catch (VTNException | RuntimeException e) {
                        ctx.log(logger, VTNLogLevel.WARN, e,
                                "%s: Ignore broken vBridge.", vbrId);
                    }
                } else {
                    ctx.log(logger, VTNLogLevel.WARN,
                            "{}: Ignore duplicate vBridge: {}",
                            vtnId, MiscUtils.getValue(vname));
                }
            }

            if (!blist.isEmpty()) {
                builder.setVbridge(blist);
            }
        }
    }

    /**
     * Resume all the vTerminals in the given VTN.
     *
     * @param ctx      A runtime context for transaction task.
     * @param builder  A {@link VtnBuilder} instance to store resumed vBridges.
     * @param vtnId    The identifier for the VTN.
     * @param xvtn     A {@link XmlVTenant} instance that contains the VTN
     *                 configuration.
     */
    private void resumeTerminals(TxContext ctx, VtnBuilder builder,
                                 VTenantIdentifier vtnId, XmlVTenant xvtn) {
        List<XmlVTerminal> xvtms = xvtn.getTerminals();
        if (xvtms != null) {
            List<Vterminal> blist = new ArrayList<>(xvtms.size());
            Set<VnodeName> nameSet = new HashSet<>();
            for (XmlVTerminal xvtm: xvtms) {
                VnodeName vname = xvtm.getName();
                if (nameSet.add(vname)) {
                    VTerminalIdentifier vtmId =
                        new VTerminalIdentifier(vtnId, xvtm.getName());
                    try {
                        VTerminal vtm = new VTerminal(vtmId);
                        blist.add(vtm.resume(ctx, this, xvtm));
                    } catch (VTNException | RuntimeException e) {
                        ctx.log(logger, VTNLogLevel.WARN, e,
                                "%s: Ignore broken vTerminal.", vtmId);
                    }
                } else {
                    ctx.log(logger, VTNLogLevel.WARN,
                            "{}: Ignore duplicate vTerminal: {}",
                            vtnId, MiscUtils.getValue(vname));
                }
            }

            if (!blist.isEmpty()) {
                builder.setVterminal(blist);
            }
        }
    }

    /**
     * Resume all the VTN path maps in the given VTN.
     *
     * @param ctx    A runtime context for transaction task.
     * @param builder  A {@link VtnBuilder} instance to store resumed vBridges.
     * @param vtnId    The identifier for the VTN.
     * @param xvtn     A {@link XmlVTenant} instance that contains the VTN
     *                 configuration.
     */
    private void resumePathMaps(TxContext ctx, VtnBuilder builder,
                                VTenantIdentifier vtnId, XmlVTenant xvtn) {
        List<XmlPathMap> xpms = xvtn.getPathMaps();
        if (xpms != null) {
            String tname = vtnId.getTenantNameString();
            List<VtnPathMap> vlist = new ArrayList<>(xpms.size());
            Set<Integer> idSet = new HashSet<>();
            for (XmlPathMap xpm: xpms) {
                Integer index = xpm.getIndex();
                if (idSet.add(index)) {
                    try {
                        vlist.add(PathMapUtils.toVtnPathMapBuilder(xpm).
                                  build());
                        ctx.log(logger, VTNLogLevel.DEBUG,
                                "{}.{}: VTN path map has been loaded: " +
                                "cond={}, policy={}, idle={}, hard={}",
                                tname, index, xpm.getCondition(),
                                xpm.getPolicy(), xpm.getIdleTimeout(),
                                xpm.getHardTimeout());
                    } catch (Exception e) {
                        ctx.log(logger, VTNLogLevel.WARN, e,
                                "%s.%s: Ignore broken VTN path map.",
                                tname, index);
                    }
                } else {
                    ctx.log(logger, VTNLogLevel.WARN,
                            "{}.{}: Ignore duplicate VTN path map: {}",
                            tname, index, xpm);
                }
            }

            if (!vlist.isEmpty()) {
                VtnPathMaps root = new VtnPathMapsBuilder().
                    setVtnPathMap(vlist).build();
                builder.setVtnPathMaps(root);
            }
        }
    }

    // AbstractTxTask

    /**
     * {@inheritDoc}
     */
    @Override
    public Vtns execute(TxContext ctx) throws VTNException {
        context = ctx;
        loadedBridges = new HashMap<>();
        List<TenantMacTable> tmlist = new ArrayList<>();

        // Prepare the root container for virtual network mappings.
        // This must be done before loading the configuration.
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        InstanceIdentifier<VtnMappings> vmpath = InstanceIdentifier.
            create(VtnMappings.class);
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        tx.put(oper, vmpath, new VtnMappingsBuilder().build(), true);

        // Load configuration from file.
        XmlConfigFile.Type ftype = XmlConfigFile.Type.VTN;
        List<Vtn> vlist = new ArrayList<Vtn>();
        for (String key: XmlConfigFile.getKeys(ftype)) {
            XmlVTenant xvtn = XmlConfigFile.load(ftype, key, XmlVTenant.class);
            if (xvtn != null) {
                try {
                    vlist.add(resume(ctx, tmlist, key, xvtn));
                } catch (VTNException | RuntimeException e) {
                    ctx.log(logger, VTNLogLevel.WARN, e,
                            "%s: Ignore broken VTN.", key);
                }
            }
        }

        VtnsBuilder builder = new VtnsBuilder();
        if (!vlist.isEmpty()) {
            builder.setVtn(vlist);
        }

        // Clear the MAC mapping status cache.
        MacMapStatusReader mReader = ctx.getSpecific(MacMapStatusReader.class);
        mReader.clear();

        // Remove old configuration, and install loaded configuration.
        InstanceIdentifier<Vtns> path = InstanceIdentifier.create(Vtns.class);
        Vtns root = builder.build();
        tx.put(oper, path, root, true);

        // Prepare the root container for MAC address tables.
        InstanceIdentifier<MacTables> mpath = InstanceIdentifier.
            create(MacTables.class);
        if (tmlist.isEmpty()) {
            tmlist = null;
        }
        MacTables mtables = new MacTablesBuilder().
            setTenantMacTable(tmlist).
            build();
        tx.put(oper, mpath, mtables, true);

        return root;
    }

    // TxTask

    /**
     * Invoked when the task has completed successfully.
     *
     * @param provider  VTN Manager provider service.
     * @param result    The loaded VTN tree.
     */
    @Override
    public void onSuccess(VTNManagerProvider provider, Vtns result) {
        // Associate an entity with every loaded vBridge.
        for (Entry<VBridgeIdentifier, VbridgeConfig> ent:
                 loadedBridges.entrySet()) {
            vBridgeManager.updateEntity(ent.getKey(), ent.getValue());
        }
    }

    // XmlLogger

    /**
     * {@inheritDoc}
     */
    @Override
    public void log(VTNLogLevel level, String format, Object ... args) {
        context.log(logger, level, format, args);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void log(VTNLogLevel level, Throwable t, String format,
                    Object ... args) {
        context.log(logger, level, t, format, args);
    }
}
