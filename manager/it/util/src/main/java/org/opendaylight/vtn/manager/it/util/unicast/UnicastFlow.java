/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.unicast;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.ListIterator;
import java.util.Set;

import org.opendaylight.vtn.manager.it.ofmock.OfMockFlow;
import org.opendaylight.vtn.manager.it.ofmock.OfMockLink;
import org.opendaylight.vtn.manager.it.ofmock.OfMockService;
import org.opendaylight.vtn.manager.it.ofmock.OfMockUtils;
import org.opendaylight.vtn.manager.it.util.ModelDrivenTestBase;
import org.opendaylight.vtn.manager.it.util.TestHost;
import org.opendaylight.vtn.manager.it.util.action.ActionVerifier;
import org.opendaylight.vtn.manager.it.util.match.FlowMatchType;
import org.opendaylight.vtn.manager.it.util.packet.EthernetFactory;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.Match;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;

/**
 * {@code UnicastFlow} describes a unicast flow established by the system test.
 */
public final class UnicastFlow {
    /**
     * Factory instance.
     */
    private final UnicastFlowFactory  factory;

    /**
     * The source host.
     */
    private final TestHost  sourceHost;

    /**
     * The destination host.
     */
    private final TestHost  destinationHost;

    /**
     * A list of switch links that represents a packet route.
     */
    private final List<OfMockLink>  packetRoute;

    /**
     * A list of flow entries that establishes a unicast flow.
     */
    private final List<OfMockFlow>  flowList = new ArrayList<>();

    /**
     * Verify flow entries for the given unicast flows are installed or
     * uninstalled.
     *
     * @param list       A list of {@link UnicastFlow} instances.
     * @param installed  If {@code true}, this method ensures that all flow
     *                   entries are installed.
     *                   If {@code false}, this method ensures that all flow
     *                   entries are uninstalled.
     * @param doWait     If {@code true}, this method blocks the calling
     *                   thread until flow entries are installed or
     *                   uninstalled.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    public static void verifyFlows(List<UnicastFlow> list, boolean installed,
                                   boolean doWait)
        throws InterruptedException {
        for (UnicastFlow unicast: list) {
            unicast.verifyFlows(installed, doWait);
        }
    }

    /**
     * Construct a new instance.
     *
     * @param fc     Factory instance.
     * @param src    The source host.
     * @param dst    The destination host.
     * @param route  A list of switch links that represents the expected
     *               packet route. {@code null} means that a packet should not
     *               be reachable.
     */
    UnicastFlow(UnicastFlowFactory fc, TestHost src, TestHost dst,
                List<OfMockLink> route) {
        factory = fc;
        sourceHost = src;
        destinationHost = dst;
        packetRoute = route;
    }

    /**
     * Return the source host.
     *
     * @return  The source host.
     */
    public TestHost getSourceHost() {
        return sourceHost;
    }

    /**
     * Return the destination host.
     *
     * @return  The destination host.
     */
    public TestHost getDestinationHost() {
        return destinationHost;
    }

    /**
     * Run the unicast flow test.
     *
     * @throws Exception  An error occurred.
     */
    public void runTest() throws Exception {
        runTest(null);
    }

    /**
     * Return a list of flow entries.
     *
     * @return  A list of {@link OfMockFlow} instances.
     */
    public List<OfMockFlow> getFlowList() {
        return Collections.unmodifiableList(flowList);
    }

    /**
     * Return the number of flow entries.
     *
     * @return  The number of flow entries.
     */
    public int getFlowCount() {
        return flowList.size();
    }

    /**
     * Run the unicast flow test.
     *
     * @param hook  A hook to be executed just after a unicast packet is sent.
     * @throws Exception  An error occurred.
     */
    public void runTest(Runnable hook) throws Exception {
        // Send a unicast packet from the source host.
        OfMockService ofmock = factory.getOfMockService();
        EthernetFactory efc =
            factory.createPacketFactory(sourceHost, destinationHost);
        byte[] payload = efc.create();
        String ingress = sourceHost.getPortIdentifier();
        String egress = destinationHost.getPortIdentifier();
        ofmock.sendPacketIn(ingress, payload);

        if (hook != null) {
            hook.run();
        }

        int inVlan = sourceHost.getVlanId();
        int outVlan = destinationHost.getVlanId();
        List<ActionVerifier> actions = factory.getActions();
        if (packetRoute != null) {
            // Ensure that the packet was sent to the egress port.
            byte[] transmitted = ofmock.awaitTransmittedPacket(egress);

            EthernetFactory tmpefc = efc.clone();
            for (ActionVerifier act: actions) {
                act.apply(tmpefc);
            }
            tmpefc.setVlanId(outVlan).verify(ofmock, transmitted);
        }

        // Ensure that no other packet was sent.
        for (String pid: factory.getPortSet()) {
            assertEquals(null, ofmock.getTransmittedPacket(pid));
        }

        if (factory.isAlreadyMapped()) {
            // Flow entries for this unicast flow are already installed by
            // another unicast flow.
            return;
        }

        // Verify flow entries.
        Set<FlowMatchType> matchTypes = factory.getMatchTypes();
        String inPort = ingress;
        String nid = OfMockUtils.getNodeIdentifier(inPort);
        int basePri = ofmock.getL2FlowPriority();
        if (packetRoute == null) {
            // Destination host is not reachable.
            MatchBuilder mb = new MatchBuilder();
            int pri = basePri + efc.initMatch(mb, inPort, matchTypes);
            Match match = mb.build();
            OfMockFlow flow = ofmock.
                getFlow(nid, OfMockService.DEFAULT_TABLE, match, pri);
            assertEquals(null, flow);
            return;
        }

        int idle = factory.getIdleTimeout();
        int hard = factory.getHardTimeout();
        for (OfMockLink link: packetRoute) {
            MatchBuilder mb = new MatchBuilder();
            int pri = basePri + efc.initMatch(mb, inPort, matchTypes);
            Match match = mb.build();
            OfMockFlow flow = ofmock.
                awaitFlow(nid, OfMockService.DEFAULT_TABLE, match, pri, true);
            assertEquals(idle, flow.getIdleTimeout());
            assertEquals(hard, flow.getHardTimeout());
            ModelDrivenTestBase.verifyOutputFlow(
                flow.getInstructions(), link.getSourcePort(), inVlan, inVlan);
            inPort = link.getDestinationPort();
            nid = OfMockUtils.getNodeIdentifier(inPort);
            flowList.add(flow);

            // Only an ingress flow entry should have timeout configuration.
            idle = 0;
            hard = 0;
        }

        // Verify egress flow entry.
        // Note that only egress flow entry modifies VLAN tag.
        MatchBuilder mb = new MatchBuilder();
        int pri = basePri + efc.initMatch(mb, inPort, matchTypes);
        Match match = mb.build();
        OfMockFlow flow = ofmock.
            awaitFlow(nid, OfMockService.DEFAULT_TABLE, match, pri, true);
        assertEquals(idle, flow.getIdleTimeout());
        assertEquals(hard, flow.getHardTimeout());

        ListIterator<Action> it = ModelDrivenTestBase.
            getActionList(flow.getInstructions()).listIterator();

        // The VLAN ID must be changed by the first action.
        ModelDrivenTestBase.verifyVlanAction(it, inVlan, outVlan);

        // Verify actions installed by flow filter.
        for (ActionVerifier act: actions) {
            act.verify(efc, it);
        }

        // The packet should be transmitted by the last action.
        ModelDrivenTestBase.verifyOutputAction(it, egress);
        assertFalse(it.hasNext());
        flowList.add(flow);
    }

    /**
     * Verify flow entries for this unicast flow are installed or uninstalled.
     *
     * @param installed  If {@code true}, this method ensures that all flow
     *                   entries are installed.
     *                   If {@code false}, this method ensures that all flow
     *                   entries are uninstalled.
     * @param doWait     If {@code true}, this method blocks the calling
     *                   thread until flow entries are installed or
     *                   uninstalled.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    public void verifyFlows(boolean installed, boolean doWait)
        throws InterruptedException {
        final int tableId = OfMockService.DEFAULT_TABLE;
        OfMockService ofmock = factory.getOfMockService();
        for (OfMockFlow flow: flowList) {
            Match match = flow.getMatch();
            String nid = flow.getNodeIdentifier();
            int pri = flow.getPriority();
            OfMockFlow expected = (installed) ? flow : null;
            OfMockFlow f = (doWait)
                ? ofmock.awaitFlow(nid, tableId, match, pri, installed)
                : ofmock.getFlow(nid, tableId, match, pri);
            assertEquals(expected, f);
        }
    }
}
