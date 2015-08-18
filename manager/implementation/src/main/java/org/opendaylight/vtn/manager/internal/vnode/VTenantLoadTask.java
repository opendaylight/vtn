/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.PathMap;
import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.FixedLogger;
import org.opendaylight.vtn.manager.internal.util.LogRecord;
import org.opendaylight.vtn.manager.internal.util.XmlConfigFile;
import org.opendaylight.vtn.manager.internal.util.pathmap.PathMapUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantUtils;
import org.opendaylight.vtn.manager.internal.vnode.xml.XmlVTenant;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.Vtns;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.VtnsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtnPathMaps;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtnPathMapsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;

/**
 * {@code VTenantLoadTask} describes a MD-SAL datastore transaction task that
 * loads the VTN configurations.
 *
 * <p>
 *   This task returns the root container of all VTNs.
 * </p>
 */
class VTenantLoadTask extends AbstractTxTask<Vtns> {
    /**
     * Logger instance.
     */
    private final Logger  logger =
        LoggerFactory.getLogger(VTenantLoadTask.class);

    /**
     * A logger instance bound to the info level.
     */
    private final FixedLogger  infoLogger = new FixedLogger.Info(logger);

    /**
     * The virtual tenant manager.
     */
    private final VTenantManager  tenantManager;

    /**
     * A set of paths to the loaded objects.
     */
    private ConcurrentMap<InstanceIdentifier<?>, Boolean>  loadedPaths;

    /**
     * Log messages to indicate loaded objects.
     */
    private List<LogRecord>  loadedLogs;

    /**
     * Construct a new instance.
     *
     * @param tmgr  The virtual tenant manager.
     */
    VTenantLoadTask(VTenantManager tmgr) {
        tenantManager = tmgr;
    }

    /**
     * Add the path to the loaded object.
     *
     * @param path    Path to the loaded object.
     * @param format  A format string used to create a log message.
     * @param args    Arguments used to create a log message.
     */
    private void addLoadedPath(InstanceIdentifier<?> path, String format,
                               Object ... args) {
        loadedPaths.put(path, Boolean.TRUE);
        if (loadedLogs != null) {
            loadedLogs.add(new LogRecord(infoLogger, format, args));
        }
    }

    /**
     * Resume the VTN from the VTN configuration.
     *
     * @param name  The name of the VTN.
     * @param xvtn  A {@link XmlVTenant} instance that contains the VTN
     *              configuration.
     * @return  A {@link Vtn} instance corresponding to the given VTN.
     * @throws VTNException  An error occurred.
     */
    private Vtn resume(String name, XmlVTenant xvtn)
        throws VTNException {
        VtnBuilder builder = xvtn.toVtnBuilder();
        VnodeName vname = builder.getName();
        String tname = vname.getValue();
        if (!name.equals(tname)) {
            String msg = new StringBuilder(name).
                append(": Ignore broken VTN: Unexpected name: ").
                append(tname).toString();
            logger.warn(msg);
            throw RpcException.getBadArgumentException(msg);
        }

        // Resume VTN path maps.
        List<VtnPathMap> vpmaps = resumePathMaps(vname, xvtn.getPathMaps());
        if (vpmaps != null) {
            VtnPathMaps root = new VtnPathMapsBuilder().
                setVtnPathMap(vpmaps).build();
            builder.setVtnPathMaps(root);
        }

        // Remarks:
        //   Use addLoadedPath() when VTN model is fully migrated to MD-SAL.
        loadedPaths.put(VTenantUtils.getIdentifier(vname), Boolean.TRUE);

        return builder.build();
    }

    /**
     * Resume VTN path map configuration.
     *
     * @param vname  A {@link VnodeName} instance that contains the VTN name.
     * @param pmaps  A list of {@link PathMap} instances.
     * @return  A list of {@link VtnPathMap} instances if at least one VTN
     *          path map is present. {@code null} if no path map is present.
     */
    private List<VtnPathMap> resumePathMaps(VnodeName vname,
                                            List<PathMap> pmaps) {
        if (pmaps == null || pmaps.isEmpty()) {
            return null;
        }

        List<VtnPathMap> vlist = new ArrayList<>(pmaps.size());
        for (PathMap pmap: pmaps) {
            try {
                vlist.add(PathMapUtils.toVtnPathMapBuilder(pmap).build());
                Integer index = pmap.getIndex();
                addLoadedPath(PathMapUtils.getIdentifier(vname, index),
                              "%s.%s: VTN path map has been loaded: " +
                              "cond=%s, policy=%s, idle=%s, hard=%s",
                              vname.getValue(), pmap.getIndex(),
                              pmap.getFlowConditionName(),
                              pmap.getPathPolicyId(), pmap.getIdleTimeout(),
                              pmap.getHardTimeout());
            } catch (Exception e) {
                String msg = new StringBuilder(vname.getValue()).
                    append('.').append(pmap.getIndex()).
                    append(": Ignore broken VTN path map.").toString();
                logger.warn(msg, e);
            }
        }

        return (vlist.isEmpty()) ? null : vlist;
    }

    // AbstractTxTask

    /**
     * {@inheritDoc}
     */
    @Override
    public Vtns execute(TxContext ctx) throws VTNException {
        loadedPaths = new ConcurrentHashMap<InstanceIdentifier<?>, Boolean>();
        loadedLogs = (logger.isInfoEnabled())
            ? new ArrayList<LogRecord>() : null;

        // Load configuration from file.
        XmlConfigFile.Type ftype = XmlConfigFile.Type.VTN;
        List<Vtn> vlist = new ArrayList<Vtn>();
        for (String key: XmlConfigFile.getKeys(ftype)) {
            XmlVTenant xvtn = XmlConfigFile.load(ftype, key, XmlVTenant.class);
            if (xvtn != null) {
                try {
                    Vtn vtn = resume(key, xvtn);
                    vlist.add(vtn);
                } catch (Exception e) {
                    logger.warn(key + ": Ignore broken VTN configuration.", e);
                }
            }
        }

        VtnsBuilder builder = new VtnsBuilder();
        if (!vlist.isEmpty()) {
            builder.setVtn(vlist);
        }

        // Remove old configuration, and install loaded configuration.
        InstanceIdentifier<Vtns> path = InstanceIdentifier.create(Vtns.class);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        DataStoreUtils.delete(tx, oper, path);

        Vtns container = builder.build();
        tx.put(oper, path, container, true);
        if (!loadedPaths.isEmpty()) {
            tenantManager.setLoadedPaths(loadedPaths.keySet());
        }

        return container;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void onSuccess(VTNManagerProvider provider, Vtns result) {
        if (loadedLogs != null) {
            for (LogRecord record: loadedLogs) {
                record.log();
            }
        }
    }
}
