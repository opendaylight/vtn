/*
 * Copyright (c) 2013 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.integrationtest.internal;

import org.eclipse.osgi.internal.baseadaptor.ArrayMap;
import org.opendaylight.controller.sal.flowprogrammer.Flow;
import org.opendaylight.controller.sal.flowprogrammer.IPluginInFlowProgrammerService;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

import java.util.Map;
import java.util.HashMap;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.CountDownLatch;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


/**
 * Represents the openflow plugin component in charge of programming the flows
 * the flow programming and relay them to functional modules above SAL.
 */
public class FlowProgrammerService implements IPluginInFlowProgrammerService
{
    private static final Logger logger = LoggerFactory
            .getLogger(FlowProgrammerService.class);

    private final Map<Node, CountDownLatch> mapLatch= new HashMap<Node, CountDownLatch>();
    Map<Node, List<Flow>> mapNF = null;

    void init() {
        logger.debug("openflow stub FlowProgrammerService init called.");
        mapNF = new HashMap<Node, List<Flow>>();
    }

    /**
     * Function called by the dependency manager when at least one dependency
     * become unsatisfied or when the component is shutting down because for
     * example bundle is being stopped.
     *
     */
    void destroy() {
    }

    /**
     * Function called by dependency manager after "init ()" is called and after
     * the services provided by the class are registered in the service registry
     *
     */
    void start() {
    }

    /**
     * Function called by the dependency manager before the services exported by
     * the component are unregistered, this will be followed by a "destroy ()"
     * calls
     *
     */
    void stop() {
    }


    /**
     * Synchronously add a flow to the network node
     *
     * @param node
     * @param flow
     */
    public Status addFlow(Node node, Flow flow){
        logger.debug("openflow stub FlowProgrammerService addFlow called.");

        addFlowInternal(node, flow);

        return new Status(StatusCode.SUCCESS);
    }


    /**
     * Synchronously modify existing flow on the switch
     *
     * @param node
     * @param oldFlow
     * @param newFlow
     */
    public Status modifyFlow(Node node, Flow oldFlow, Flow newFlow){
        logger.debug("openflow stub FlowProgrammerService modifyFlow called.");
        return new Status(StatusCode.SUCCESS);
    }
    /**
     * Synchronously remove the flow from the network node
     *
     * @param node
     * @param flow
     */
    public Status removeFlow(Node node, Flow flow){
        logger.debug("openflow stub FlowProgrammerService removeFlow called.");

        removeFlowInternal(node, flow);

        return new Status(StatusCode.SUCCESS);
    }

    /**
     * Asynchronously add a flow to the network node
     *
     * @param node
     * @param flow
     * @param rid
     */
    public Status addFlowAsync(Node node, Flow flow, long rid){
        logger.debug("openflow stub FlowProgrammerService addFlowAsync called.");

        addFlowInternal(node, flow);

        return new Status(StatusCode.SUCCESS);
    }

    /**
     * Asynchronously modify existing flow on the switch
     *
     * @param node
     * @param oldFlow
     * @param newFlow
     * @param rid
     */
    public Status modifyFlowAsync(Node node, Flow oldFlow, Flow newFlow, long rid){
        logger.debug("openflow stub FlowProgrammerService modifyFlowAsync called.");
        return new Status(StatusCode.SUCCESS);
    }

    /**
     * Asynchronously remove the flow from the network node
     *
     * @param node
     * @param flow
     * @param rid
     */
    public Status removeFlowAsync(Node node, Flow flow, long rid){
        logger.debug("openflow stub FlowProgrammerService removeFlowAsync called.");

        removeFlowInternal(node, flow);

        return new Status(StatusCode.SUCCESS);
    }

    /**
     * Remove all flows present on the network node
     *
     * @param node
     */
    public Status removeAllFlows(Node node){
        logger.debug("openflow stub FlowProgrammerService removeAllFlow called.");
        return new Status(StatusCode.SUCCESS);
    }

    /**
     * Send Barrier message synchronously. The caller will be blocked until the
     * Barrier reply arrives.
     *
     * @param node
     */
    public Status syncSendBarrierMessage(Node node){
        logger.debug("openflow stub FlowProgrammerService syncSendBarrierMessage called.");
        return new Status(StatusCode.SUCCESS);
    }

    /**
     * Send Barrier message asynchronously. The caller is not blocked.
     *
     * @param node
     */
    public Status asyncSendBarrierMessage(Node node){
        logger.debug("openflow stub FlowProgrammerService asyncSendBarrierMessage called.");
        return new Status(StatusCode.SUCCESS);
    }

    public CountDownLatch setLatch(Node node, int expectedAddFlows) {
        logger.trace("setLatch called.");
        if (node == null) {
            return null;
        }

       logger.trace("Start waiting for {} flow(s) to add to {}.", expectedAddFlows, node);
        CountDownLatch latch = new CountDownLatch(expectedAddFlows);
        mapLatch.put(node, latch);
        return latch;
    }

    public int getFlowCount(Node node) {
        List<Flow> list = mapNF.get(node);
        if (list == null) {
            return 0;
        }
        return list.size();
    }

    private void addFlowInternal(Node node, Flow flow) {
        List<Flow> list = mapNF.get(node);
        if (list == null) {
            list = new CopyOnWriteArrayList<Flow>();
            mapNF.put(node, list);
        }
        list.add(flow);

        CountDownLatch latch = mapLatch.get(node);
        if (latch != null) {
            latch.countDown();
        }
    }

    private void removeFlowInternal(Node node, Flow flow) {
        List<Flow> list = mapNF.get(node);
        if (list == null) {
            return;
        }
        list.remove(flow);

        CountDownLatch latch = mapLatch.get(node);
        if (latch != null) {
            latch.countDown();
        }
    }

    public void clearFlow(Node node) {
        List<Flow> listFlow = mapNF.get(node);
        if (listFlow == null) {
            return;
        }

        listFlow.clear();
    }

    public void clearFlowAll() {
        mapNF.clear();
        mapLatch.clear();
    }
}
