/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.integrationtest.internal;


import org.opendaylight.ovsdb.plugin.OvsdbInventoryListener;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.ovsdb.lib.notation.Row;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class OvsdbServices implements  OvsdbInventoryListener{
    protected static final Logger LOG = LoggerFactory
            .getLogger(OvsdbServices.class);
/**
     * Function called by the dependency manager when all the required
     * dependencies are satisfied
     *
     */
    @Override
    public void nodeAdded(Node node) {
        LOG.info("node added");
    }
    @Override
    public void nodeRemoved(Node node) {
        LOG.info("node removed ");
    }
    @Override
    public void rowAdded(Node node, String tableName, String uuid, Row row) {
        LOG.info("row added ");
    }
    @Override
    public void rowUpdated(Node node, String tableName, String uuid, Row old, Row row) {
        LOG.info("row update ");
    }
    @Override
    public void rowRemoved(Node node, String tableName, String uuid, Row row, Object obj) {
        LOG.info("row removed ");
    }

}
