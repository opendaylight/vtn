/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.integrationtest.internal;

import java.util.List;
import java.util.ArrayList;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.CountDownLatch;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.IPluginInDataPacketService;
import org.opendaylight.controller.sal.packet.RawPacket;

public class DataPacketServices implements IPluginInDataPacketService {
    private static final Logger LOG = LoggerFactory.
        getLogger(DataPacketServices.class);

    private List<RawPacket> pktList = new CopyOnWriteArrayList<RawPacket>();
    private CountDownLatch latch = null;

    void init() {
        LOG.debug("openflow stub DataPacketServices init called.");
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

    public boolean contains(RawPacket c) {
        if (c == null) {
            return false;
        }

        if (pktList == null || pktList.isEmpty()) {
            return false;
        }

        return pktList.contains(c);
    }

    public CountDownLatch setLatch(int expectedPktCount) {
        LOG.trace("setLatch called. Start waiting for {} packet(s).",
                  expectedPktCount);
        this.latch = new CountDownLatch(expectedPktCount);
        return this.latch;
    }

    public void unsetLatch() {
        if (this.latch != null) {
            this.latch = null;
        }
    }

    public void clearPkt() {
        LOG.debug("openflow stub DataPacketServices clearPkt called.");
        if (pktList == null) {
            return;
        }
        pktList.clear();
    }

    public int getPktCount() {
        LOG.trace("openflow stub DataPacketServices getPktCount called.");
        if (pktList == null || pktList.isEmpty()) {
            return 0;
        }

        return pktList.size();
    }

    public List<RawPacket> getPacketList() {
        List<RawPacket> result = new ArrayList<RawPacket>();
        result.addAll(this.pktList);
        return result;
    }

    public RawPacket getPacket() {
        return getPacket(0);
    }

    public RawPacket getPacket(int index) {
        try {
            return pktList.get(index);
        } catch (IndexOutOfBoundsException e) {
            return null;
        }
    }

    /*
     * Emulates transmitting packet
     */
    @Override
    public void transmitDataPacket(RawPacket outPkt) {
        LOG.debug("openflow stub DataPacketServices transmitDataPacket called.");
        if (outPkt == null) {
            LOG.debug("outPkt is null.");
            return;
        }

        NodeConnector nc = outPkt.getOutgoingNodeConnector();
        if (nc == null) {
            LOG.debug("outPkt's outgoing port is not specified.");
            return;
        }

        LOG.debug("outPkt is added.");
        pktList.add(outPkt);
        if (this.latch != null) {
            this.latch.countDown();
        }
    }
}
