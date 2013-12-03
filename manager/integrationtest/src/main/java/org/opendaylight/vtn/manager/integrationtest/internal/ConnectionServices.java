/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.integrationtest.internal;

import org.opendaylight.controller.sal.connection.IPluginInConnectionService;
import org.opendaylight.controller.sal.connection.ConnectionConstants;

import java.util.Map;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class ConnectionServices implements IPluginInConnectionService {

    private static final Logger logger = LoggerFactory
            .getLogger(ConnectionServices.class);

    void init() {
        logger.debug("openflow stub ConnectionServices init called.");
    }

    void destroy() {
        logger.debug("openflow stub ConnectionServices destroy called.");
    }

    void start() {
        logger.debug("openflow stub ConnectionServices start called.");
    }

    void stop() {
        logger.debug("openflow stub ConnectionServices stop called.");
    }

    // IPluginInConnectionService
    @Override
    public Status disconnect(Node node) {
        logger.debug("openflow stub ConnectionServices disconnect called.");
        return new Status(StatusCode.SUCCESS);
    }

    @Override
    public Node connect(String i, Map<ConnectionConstants, String> p) {
        logger.debug("openflow stub ConnectionServices connect called.");
        return null;
    }

    @Override
    public void notifyClusterViewChanged() {
        logger.debug("openflow stub ConnectionServices notifyClusterViewChanged called.");
        /* No-op */
    }

    @Override
    public void notifyNodeDisconnectFromMaster(Node node) {
        logger.debug("openflow stub ConnectionServices notifyNodeDisconnectFromMaster called.");
        /* No-op */
    }
}
