/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import static org.junit.Assert.fail;

import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.it.ofmock.DataChangeWaiter;
import org.opendaylight.vtn.manager.it.ofmock.DataStoreUtils;
import org.opendaylight.vtn.manager.it.ofmock.OfMockLink;
import org.opendaylight.vtn.manager.it.ofmock.OfMockService;

import org.opendaylight.controller.md.sal.binding.api.ReadOnlyTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.VtnTopology;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.IgnoredLinks;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.ignored.links.IgnoredLink;

/**
 * {@code SwitchLinkSet} describes a set of inter-switch links.
 */
public final class SwitchLinkMap {
    /**
     * Logger instance.
     */
    private static final Logger  LOG = LoggerFactory.
        getLogger(SwitchLinkMap.class);

    /**
     * Path to the vtn-topology container.
     */
    private static final InstanceIdentifier<VtnTopology> TOPO_PATH =
        InstanceIdentifier.create(VtnTopology.class);

    /**
     * Path to the ignored-links container.
     */
    private static final InstanceIdentifier<IgnoredLinks> IGNORED_PATH =
        InstanceIdentifier.create(IgnoredLinks.class);

    /**
     * The number of milliseconds to wait for topology change.
     */
    private static final long  TIMEOUT = 10000L;

    /**
     * ofmock service.
     */
    private final OfMockService  ofMock;

    /**
     * A set if inter-switch links.
     *
     * <p>
     *   Each map entry describes an inter-switch link. This map takes
     *   source port of inter-switch link as a key. Destination port
     *   of inter-switch link is associated with a key in this map.
     * </p>
     */
    private final Map<String, String>  linkMap;

    /**
     * A set of VTN links in the vtn-topology container.
     */
    private final Map<String, SwitchLink>  vtnLinks;

    /**
     * A set of VTN links in the ignored-links container.
     */
    private final Map<String, SwitchLink>  ignoredLinks;

    /**
     * Runtime context to verify VTN topology.
     */
    private static final class TopologyContext {
        /**
         * A set of expected VTN links.
         */
        private final Set<SwitchLink>  expectedVtnLinks;

        /**
         * A set of unexpected VTN links.
         */
        private final Set<SwitchLink>  unexpectedVtnLinks = new HashSet<>();

        /**
         * A set of expected ignored links.
         */
        private final Set<SwitchLink>  expectedIgnoredLinks;

        /**
         * A set of unexpected ignored links.
         */
        private final Set<SwitchLink>  unexpectedIgnoredLinks = new HashSet<>();

        /**
         * Construct a new instance.
         *
         * @param mock   ofmock service.
         * @param vlinks  A set of expected VTN links.
         * @param ilinks  A set of expected ignored links.
         */
        private TopologyContext(OfMockService mock,
                                Collection<SwitchLink> vlinks,
                                Collection<SwitchLink> ilinks) {
            expectedVtnLinks = new HashSet<>(vlinks);
            expectedIgnoredLinks = new HashSet<>(ilinks);
            try (ReadOnlyTransaction rtx = mock.newReadOnlyTransaction()) {
                checkVtnTopology(rtx);
                checkIgnoredLinks(rtx);
            }
        }

        /**
         * Check the contents of vtn-topology container.
         *
         * @param rtx  MD-SAL datastore transaction.
         */
        private void checkVtnTopology(ReadTransaction rtx) {
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            VtnTopology vtopo = DataStoreUtils.read(rtx, oper, TOPO_PATH).
                orNull();

            if (vtopo != null) {
                List<VtnLink> list = vtopo.getVtnLink();
                if (list != null) {
                    for (VtnLink vlink: list) {
                        unexpectedVtnLinks.add(new SwitchLink(vlink));
                    }
                }
            }

            for (Iterator<SwitchLink> it = unexpectedVtnLinks.iterator();
                 it.hasNext();) {
                SwitchLink swl = it.next();
                if (expectedVtnLinks.remove(swl)) {
                    it.remove();
                }
            }
        }

        /**
         * Check the contents of ignored-links container.
         *
         * @param rtx  MD-SAL datastore transaction.
         */
        private void checkIgnoredLinks(ReadTransaction rtx) {
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            IgnoredLinks igls = DataStoreUtils.read(rtx, oper, IGNORED_PATH).
                orNull();

            if (igls != null) {
                List<IgnoredLink> list = igls.getIgnoredLink();
                if (list != null) {
                    for (IgnoredLink ilink: list) {
                        unexpectedIgnoredLinks.add(new SwitchLink(ilink));
                    }
                }
            }

            for (Iterator<SwitchLink> it = unexpectedIgnoredLinks.iterator();
                 it.hasNext();) {
                SwitchLink swl = it.next();
                if (expectedIgnoredLinks.remove(swl)) {
                    it.remove();
                }
            }
        }

        /**
         * Determine whether the vtn-topology container is valid or not.
         *
         * @return  {@code true} only if the vtn-topology container is valid.
         */
        private boolean isVtnTopologyValid() {
            return (expectedVtnLinks.isEmpty() &&
                    unexpectedVtnLinks.isEmpty());
        }

        /**
         * Determine whether the ignored-links container is valid or not.
         *
         * @return  {@code true} only if the ignored-links container is valid.
         */
        private boolean isIgnoredLinksValid() {
            return (expectedIgnoredLinks.isEmpty() &&
                    unexpectedIgnoredLinks.isEmpty());
        }

        /**
         * Throw an error if the verification failed.
         */
        private void assertTopology() {
            boolean failed = false;
            for (SwitchLink swl: expectedVtnLinks) {
                LOG.error("Expected vtn-link is not present: {}", swl);
                failed = true;
            }
            for (SwitchLink swl: unexpectedVtnLinks) {
                LOG.error("Unexpected vtn-link is present: {}", swl);
                failed = true;
            }
            for (SwitchLink swl: expectedIgnoredLinks) {
                LOG.error("Expected ignored-link is not present: {}", swl);
                failed = true;
            }
            for (SwitchLink swl: unexpectedIgnoredLinks) {
                LOG.error("Unexpected ignored-link is present: {}", swl);
                failed = true;
            }

            if (failed) {
                fail("Unexpected VTN topology.");
            }
        }
    }

    /**
     * Construct a new instance.
     *
     * @param mock  ofmock service.
     */
    public SwitchLinkMap(OfMockService mock) {
        ofMock = mock;
        linkMap = new HashMap<>();
        vtnLinks = new HashMap<>();
        ignoredLinks = new HashMap<>();
    }

    /**
     * Construct a copy of the given instance.
     *
     * @param slm  A {@link SwitchLinkMap} instance to be copied.
     */
    public SwitchLinkMap(SwitchLinkMap slm) {
        ofMock = slm.ofMock;
        linkMap = new HashMap<>(slm.linkMap);
        vtnLinks = new HashMap<>(slm.vtnLinks);
        ignoredLinks = new HashMap<>(slm.ignoredLinks);
    }

    /**
     * Add a new inter-switch link detected by topology-manager.
     *
     * @param src  The port identifier which specifies the source port of
     *             the link.
     * @param dst  The port identifier which specifies the destination port of
     *             the link.
     */
    public void add(String src, String dst) {
        add(src, dst, false);
    }

    /**
     * Add a new inter-switch link.
     *
     * @param src  The port identifier which specifies the source port of
     *             the link.
     * @param dst  The port identifier which specifies the destination port of
     *             the link.
     * @param st   {@code true} means that the given link is a static link.
     *             {@code false} means that the given link is a dynamic link
     *             detected by topology-manager.
     */
    public void add(String src, String dst, boolean st) {
        linkMap.put(src, dst);
        vtnLinks.put(src, new SwitchLink(src, dst, st));
    }

    /**
     * Remove the inter-switch link specified by the given source port.
     *
     * @param src  The port identifier which specifies the source port of
     *             the link to be removed.
     */
    public void remove(String src) {
        linkMap.remove(src);
        vtnLinks.remove(src);
    }

    /**
     * Add an inter-switch link to be ignored.
     *
     * @param src  The port identifier which specifies the source port of
     *             the link.
     * @param dst  The port identifier which specifies the destination port of
     *             the link.
     */
    public void addIgnored(String src, String dst) {
        ignoredLinks.put(src, new SwitchLink(src, dst, false));
    }

    /**
     * Remove the given inter-switch link to be ignored.
     *
     * @param src  The port identifier which specifies the source port of
     *             the link.
     */
    public void removeIgnored(String src) {
        ignoredLinks.remove(src);
    }

    /**
     * Verify the current network topology and routing table.
     *
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    public void verify() throws InterruptedException {
        long abstime = System.currentTimeMillis() + TIMEOUT;
        ofMock.awaitTopology(getTopology());

        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        try (DataChangeWaiter<VtnTopology> tdcw = ofMock.newDataChangeWaiter(
                 oper, TOPO_PATH);
             DataChangeWaiter<IgnoredLinks> idcw = ofMock.newDataChangeWaiter(
                 oper, IGNORED_PATH)) {
            TopologyContext ctx = new TopologyContext(
                ofMock, vtnLinks.values(), ignoredLinks.values());
            for (;;) {
                if (ctx.isVtnTopologyValid()) {
                    if (ctx.isIgnoredLinksValid()) {
                        return;
                    } else if (!idcw.await(abstime)) {
                        break;
                    }
                } else if (!tdcw.await(abstime)) {
                    break;
                }

                ctx = new TopologyContext(ofMock, vtnLinks.values(),
                                          ignoredLinks.values());
            }

            ctx.assertTopology();
        }
    }

    /**
     * Return a set of inter-switch links.
     *
     * @return  A set of {@link OfMockLink} instances.
     */
    private Set<OfMockLink> getTopology() {
        Set<OfMockLink> set = new HashSet<>();
        for (Map.Entry<String, String> entry: linkMap.entrySet()) {
            String src = entry.getKey();
            String dst = entry.getValue();
            set.add(new OfMockLink(src, dst));
        }

        return set;
    }
}
