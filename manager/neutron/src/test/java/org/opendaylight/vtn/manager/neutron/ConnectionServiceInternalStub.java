/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.neutron;

import java.util.List;
import java.util.Map;

import org.opendaylight.ovsdb.plugin.Connection;
import org.opendaylight.ovsdb.plugin.IConnectionServiceInternal;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.connection.ConnectionConstants;
import java.util.concurrent.ExecutionException;

/**
 * Stub class for unit tests.
 *
 * This stub provides APIs implemented in
 * org.opendaylight.vtn.manager package.
 */

public class ConnectionServiceInternalStub implements IConnectionServiceInternal {
  @Override
    public Connection getConnection(Node node) {
      return null;
    }

  @Override
    public List<Node> getNodes() {
      return null;
    }

  @Override
    public Node connect(String identifier, Map<ConnectionConstants, String> params) {
      return null;
    }

  @Override
    public Boolean setOFController(Node node, String bridgeUUID) throws InterruptedException, ExecutionException {
      return true;
    }
};
