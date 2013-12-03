package org.opendaylight.vtn.manager.integrationtest.internal;

import org.opendaylight.controller.sal.connection.IPluginInConnectionService;
import org.opendaylight.controller.sal.connection.ConnectionConstants;

import java.util.Map;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

class ConnectionServices implements IPluginInConnectionService {

    private static final Logger logger = LoggerFactory
            .getLogger(ConnectionServices.class);

    void init() {
        logger.debug("openflow stub ConnectionService init called.");
    }

    void destroy() {
        /* No-op */
    }

    void start() {
        /* No-op */
    }

    void stop() {
        /* No-op */
    }

    // IPluginInConnectionService
    @Override
    public Status disconnect(Node node) {
        logger.debug("openflow stub ConnectionService disconnect called.");
        return new Status(StatusCode.SUCCESS);
    }

    @Override
    public Node connect(String i, Map<ConnectionConstants, String> p) {
        logger.debug("openflow stub ConnectionService connect called.");
        return null;
    }

    @Override
    public void notifyClusterViewChanged() {
        logger.debug("openflow stub ConnectionService notifyClusterViewChanged called.");
        /* No-op */
    }

    @Override
    public void notifyNodeDisconnectFromMaster(Node node) {
        logger.debug("openflow stub ConnectionService notifyNodeDisconnectFromMaster called.");
        /* No-op */
    }
}
