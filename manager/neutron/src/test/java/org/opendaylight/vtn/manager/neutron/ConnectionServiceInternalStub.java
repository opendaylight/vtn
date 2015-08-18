/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron;

import java.util.List;
import java.util.Map;

import org.opendaylight.controller.sal.core.Node;

import org.opendaylight.ovsdb.compatibility.plugin.api.OvsdbConnectionService;
import org.opendaylight.ovsdb.plugin.api.Connection;
import org.opendaylight.ovsdb.plugin.api.ConnectionConstants;

/**
 * Stub class for unit tests.
 *
 * This stub provides APIs implemented in
 * org.opendaylight.ovsdb.plugin package.
 */

public class ConnectionServiceInternalStub implements OvsdbConnectionService{
    // Following Methods are Unused in UnitTest.
    @Override
    public Connection getConnection(Node node) {
        return null;
    }

    @Override
    public List<Node> getNodes() {
        return null;
    }

    @Override
    public Node getNode(String identifier) {
        return null;
    }

    @Override
    public Node connect(String identifier, Map<ConnectionConstants, String> params) {
        return null;
    }
}
