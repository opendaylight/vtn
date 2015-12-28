/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.unicast;

import java.util.ArrayList;
import java.util.Collections;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.it.ofmock.OfMockLink;
import org.opendaylight.vtn.manager.it.ofmock.OfMockService;
import org.opendaylight.vtn.manager.it.util.TestHost;
import org.opendaylight.vtn.manager.it.util.action.ActionVerifier;
import org.opendaylight.vtn.manager.it.util.flow.match.FlowMatchType;
import org.opendaylight.vtn.manager.it.util.packet.EthernetFactory;

/**
 * {@code UnicastFlowFactory} describes a test environment for configuring a
 * unicast flow.
 */
public abstract class UnicastFlowFactory {
    /**
     * Default value of flow idle timeout.
     */
    private static final int  DEFAULT_IDLE_TIMEOUT = 300;

    /**
     * Default value of flow hard timeout.
     */
    private static final int  DEFAULT_HARD_TIMEOUT = 0;

    /**
     * openflowplugin mock-up service.
     */
    private final OfMockService  ofMockService;

    /**
     * A set of port identifiers that specifies all the switch ports detected
     * by the controller.
     */
    private final Set<String>  portSet = new HashSet<>();

    /**
     * A list of {@link ActionVerifier} instances which specifies flow actions
     * to be applied.
     */
    private final List<ActionVerifier>  flowActions = new ArrayList<>();

    /**
     * A set of {@link FlowMatchType} which specifies additional flow match
     * types to be configured.
     */
    private final Set<FlowMatchType>  matchTypes =
        EnumSet.noneOf(FlowMatchType.class);

    /**
     * Idle timeout for an ingress flow entry.
     */
    private int  idleTimeout = DEFAULT_IDLE_TIMEOUT;

    /**
     * Hard timeout for an ingress flow entry.
     */
    private int  hardTimeout = DEFAULT_HARD_TIMEOUT;

    /**
     * Determine whether flow entries for unicast flow should be present
     * or not.
     */
    private boolean  alreadyMapped;

    /**
     * Construct a new instance.
     *
     * @param ofmock  openflowplugin mock-up service.
     */
    UnicastFlowFactory(OfMockService ofmock) {
        ofMockService = ofmock;
    }

    /**
     * Return the openflowplugin mock-up service.
     *
     * @return  An {@link OfMockService} instance.
     */
    public final OfMockService getOfMockService() {
        return ofMockService;
    }

    /**
     * Return an idle timeout value for an ingress flow entry.
     *
     * @return  An idle timeout for an ingress flow entry.
     */
    public final int getIdleTimeout() {
        return idleTimeout;
    }

    /**
     * Return a hard timeout value for an ingress flow entry.
     *
     * @return  A hard timeout for an ingress flow entry.
     */
    public final int getHardTimeout() {
        return hardTimeout;
    }

    /**
     * Set an idle timeout value for an ingress flow entry.
     *
     * @param value  An idle timeout for an ingress flow entry.
     * @return  This instance.
     */
    public final UnicastFlowFactory setIdleTimeout(int value) {
        idleTimeout = value;
        return this;
    }

    /**
     * Set a hard timeout value for an ingress flow entry.
     *
     * @param value  A hard timeout for an ingress flow entry.
     * @return  This instance.
     */
    public final UnicastFlowFactory setHardTimeout(int value) {
        hardTimeout = value;
        return this;
    }

    /**
     * Reset idle and hard timeout for an ingress flow entry.
     *
     * @return  This instance.
     */
    public final UnicastFlowFactory resetTimeout() {
        idleTimeout = DEFAULT_IDLE_TIMEOUT;
        hardTimeout = DEFAULT_HARD_TIMEOUT;
        return this;
    }

    /**
     * Add port identifiers that specifies all the switch ports detected by
     * the VTN Manager.
     *
     * @param pset  A set of port identifiers.
     * @return  This instance.
     */
    public final UnicastFlowFactory addPortSet(Set<String> pset) {
        portSet.addAll(pset);
        return this;
    }

    /**
     * Return an immutable set of port identifiers that specifies all the
     * switch ports detected by the controller.
     *
     * @return  A set of port identifiers.
     */
    public final Set<String> getPortSet() {
        return Collections.unmodifiableSet(portSet);
    }

    /**
     * Append the specified {@link ActionVerifier} instance to the tail of
     * the flow action list.
     *
     * @param act  An {@link ActionVerifier} instance.
     * @return  This instance.
     */
    public final UnicastFlowFactory addAction(ActionVerifier act) {
        flowActions.add(act);
        return this;
    }

    /**
     * Clear the list of flow actions.
     *
     * @return  This instance.
     */
    public final UnicastFlowFactory clearAction() {
        flowActions.clear();
        return this;
    }

    /**
     * Return an immutable list of flow actions to be applied.
     *
     * @return  A list of {@link ActionVerifier} instances.
     */
    public final List<ActionVerifier> getActions() {
        return Collections.unmodifiableList(flowActions);
    }

    /**
     * Append the specified {@link FlowMatchType} instance to the set of
     * flow match types to be configured into a flow entry.
     *
     * @param type  A {@link FlowMatchType} instance.
     * @return  This instance.
     */
    public final UnicastFlowFactory addMatchType(FlowMatchType type) {
        matchTypes.add(type);
        return this;
    }

    /**
     * Clear the set of flow match types.
     *
     * @return  This instance.
     */
    public final UnicastFlowFactory clearMatchType() {
        matchTypes.clear();
        return this;
    }

    /**
     * Return an immutable set of flow match types to be configured into a
     * flow entry.
     *
     * @return  A set of {@link FlowMatchType} instances.
     */
    public final Set<FlowMatchType> getMatchTypes() {
        return Collections.unmodifiableSet(matchTypes);
    }

    /**
     * Determine whether flow entries for unicast flow should be present
     * or not.
     *
     * @return  {@code true} only if flow entries for unicast flow should be
     *          present.
     */
    public final boolean isAlreadyMapped() {
        return alreadyMapped;
    }

    /**
     * Set a boolean value which indicates whether flow entries for unicast
     * packet should be present or not.
     *
     * @param mapped  {@code true} means that flow entries for unicast flow
     *                should be present.
     * @return  This instance.
     */
    public final UnicastFlowFactory setAlreadyMapped(boolean mapped) {
        alreadyMapped = mapped;
        return this;
    }

    /**
     * Create a unicast flow for test.
     *
     * @param src    The source host.
     * @param dst    The destination host.
     * @param route  A list of switch links that represents the expected
     *               packet route. {@code null} means that a packet should not
     *               be reachable.
     * @return  A {@link UnicastFlow} instance.
     */
    public final UnicastFlow create(TestHost src, TestHost dst,
                                    List<OfMockLink> route) {
        return new UnicastFlow(this, src, dst, route);
    }

    /**
     * Create a new ethernet factory for a unicast flow test.
     *
     * @param src  The source host.
     * @param dst  The destination host.
     * @return  A {@link EthernetFactory} instance.
     */
    public final EthernetFactory createEthernetFactory(TestHost src,
                                                       TestHost dst) {
        EtherAddress smac = src.getEtherAddress();
        EtherAddress dmac = dst.getEtherAddress();
        return new EthernetFactory(smac, dmac).setVlanId(src.getVlanId());
    }

    /**
     * Create a new packet factory for a unicast flow test.
     *
     * @param src  The source host.
     * @param dst  The destination host.
     * @return  A {@link EthernetFactory} instance.
     */
    public abstract EthernetFactory createPacketFactory(TestHost src,
                                                        TestHost dst);
}
