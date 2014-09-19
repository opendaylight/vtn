/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.PathMap;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.internal.ContainerConfig;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * Implementation of container path map.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class ContainerPathMapImpl extends PathMapImpl {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -2543964758272756372L;

    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(ContainerPathMapImpl.class);

    /**
     * Construct a new instance.
     *
     * @param idx    An index number to be assigned.
     * @param ptmap  A {@link PathMap} instance.
     * @throws VTNException
     *    {@code ptmap} contains invalid value.
     */
    public ContainerPathMapImpl(int idx, PathMap ptmap) throws VTNException {
        super(idx, ptmap);
    }

    /**
     * Save the configuration of this container path map to the
     * configuration file.
     *
     * @param mgr  VTN Manager service.
     * @return     "Success" or failure reason.
     */
    public Status saveConfig(VTNManagerImpl mgr) {
        String container = mgr.getContainerName();
        ContainerConfig cfg = new ContainerConfig(container);
        String name = Integer.toString(getIndex());
        Status status = cfg.save(ContainerConfig.Type.PATHMAP, name, this);
        if (status.isSuccess()) {
            if (LOG.isTraceEnabled()) {
                LOG.trace("{}.{}: Path map was saved.", container, name);
            }
            return status;
        }

        String msg = "Failed to save path map configuration";
        LOG.error("{}.{}: {}: {}",
                  container, name, msg, status);
        return new Status(StatusCode.INTERNALERROR, msg);
    }

    /**
     * Destroy this container path map.
     *
     * @param mgr  VTN Manager service.
     */
    public void destroy(VTNManagerImpl mgr) {
        String container = mgr.getContainerName();
        ContainerConfig cfg = new ContainerConfig(container);
        String name = Integer.toString(getIndex());
        cfg.delete(ContainerConfig.Type.PATHMAP, name);
    }

    /**
     * Return a logger instance.
     *
     * @return  A logger instance.
     */
    @Override
    protected Logger getLogger() {
        return LOG;
    }

    /**
     * Return a prefix of log record.
     *
     * @return  A prefix of log record.
     */
    @Override
    protected String getLogPrefix() {
        StringBuilder builder = new StringBuilder(".");
        return builder.append(Integer.toString(getIndex())).toString();
    }
}
