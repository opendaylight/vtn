/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock.impl;

import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.it.ofmock.DataStoreUtils;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.link.attributes.Destination;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.link.attributes.Source;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.Topology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.Link;

/**
 * {@code LinkCleaner} describes a task that deletes all the inter-switch
 * links affected by the specified resource.
 */
public abstract class LinkCleaner {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(LinkCleaner.class);

    /**
     * {@code Node} describes a task that deletes all the inter-switch links
     * affected by the specified switch.
     */
    public static final class Node extends LinkCleaner {
        /**
         * The MD-SAL node identifier that specifies the target switch.
         */
        private final String  nodeId;

        /**
         * Construct a new instance.
         *
         * @param nid  The MD-SAL node identifier that specifies the target
         *             switch.
         */
        public Node(String nid) {
            nodeId = nid;
        }

        // LinkCleaner

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean match(Link link) {
            Source src = link.getSource();
            boolean ret = nodeId.equals(src.getSourceNode().getValue());
            if (!ret) {
                Destination dst = link.getDestination();
                ret = nodeId.equals(dst.getDestNode().getValue());
            }
            return ret;
        }
    }

    /**
     * {@code Port} describes a task that deletes all the inter-switch links
     * affected by the specified switch port.
     */
    public static final class Port extends LinkCleaner {
        /**
         * The MD-SAL port identifier that specifies the target switch port.
         */
        private final String  portId;

        /**
         * Construct a new instance.
         *
         * @param pid  The MD-SAL port identifier that specifies the target
         *             switch port.
         */
        public Port(String pid) {
            portId = pid;
        }

        // LinkCleaner

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean match(Link link) {
            Source src = link.getSource();
            boolean ret = portId.equals(src.getSourceTp().getValue());
            if (!ret) {
                Destination dst = link.getDestination();
                ret = portId.equals(dst.getDestTp().getValue());
            }
            return ret;
        }
    }

    /**
     * Construct a new instance.
     */
    protected LinkCleaner() {
    }

    /**
     * Delete all the inter-switch links affected by the specified resource.
     *
     * @param tx  A read-write MD-SAL datastore transaction.
     */
    public final void delete(ReadWriteTransaction tx) {
        // Read the current network topology.
        InstanceIdentifier<Topology> tpath = TopologyUtils.
            getTopologyPathBuilder().
            build();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Optional<Topology> opt = DataStoreUtils.read(tx, oper, tpath);
        if (opt.isPresent()) {
            List<Link> links = opt.get().getLink();
            if (links != null) {
                for (Link link: links) {
                    if (match(link)) {
                        InstanceIdentifier<Link> path = tpath.
                            child(Link.class, link.getKey());
                        tx.delete(oper, path);
                    }
                }
            }
        }
    }

    /**
     * Determine whether the specified inter-switch link should be removed
     * or not.
     *
     * @param link  An inter-switch link to be tested.
     * @return  {@code true} if the given link should be removed.
     *          {@code false} if the given link should be retained.
     */
    protected abstract boolean match(Link link);
}
