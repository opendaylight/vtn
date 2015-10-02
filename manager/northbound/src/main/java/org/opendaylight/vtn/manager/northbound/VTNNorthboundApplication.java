/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import static com.fasterxml.jackson.databind.DeserializationFeature.FAIL_ON_UNKNOWN_PROPERTIES;

import java.util.Set;

import javax.ws.rs.core.Application;

import com.google.common.collect.ImmutableSet;

import com.fasterxml.jackson.jaxrs.json.JacksonJaxbJsonProvider;

import org.opendaylight.controller.northbound.commons.exception.GenericExceptionMapper;
import org.opendaylight.controller.northbound.commons.query.QueryContextProvider;

/**
 * Describes JAX-RS application for VTN Manager.
 */
public final class VTNNorthboundApplication extends Application {
    /**
     * A set of root resource and provider instances.
     */
    private final Set<Object>  providerInstances;

    /**
     * A set of provider classes.
     */
    private final Set<Class<?>>  providerClasses;

    /**
     * Construct a new instance.
     */
    public VTNNorthboundApplication() {
        JacksonJaxbJsonProvider jsonProvider = new JacksonJaxbJsonProvider();
        jsonProvider.configure(FAIL_ON_UNKNOWN_PROPERTIES, false);
        providerInstances = ImmutableSet.<Object>builder().
            add(jsonProvider).
            add(new JsonExceptionMapper()).
            add(new QueryContextProvider()).
            add(new GenericExceptionMapper()).
            build();

        providerClasses = ImmutableSet.<Class<?>>builder().
            add(ContainerPathMapNorthbound.class).
            add(FlowConditionNorthbound.class).
            add(FlowNorthbound.class).
            add(GlobalNorthbound.class).
            add(MacMapNorthbound.class).
            add(PathPolicyNorthbound.class).
            add(VBridgeFlowFilterNorthbound.class).
            add(VBridgeIfFlowFilterNorthbound.class).
            add(VBridgeInterfaceNorthbound.class).
            add(VBridgeNorthbound.class).
            add(VTenantFlowFilterNorthbound.class).
            add(VTenantNorthbound.class).
            add(VTenantPathMapNorthbound.class).
            add(VTerminalIfFlowFilterNorthbound.class).
            add(VTerminalInterfaceNorthbound.class).
            add(VTerminalNorthbound.class).
            build();
    }

    /**
     * Return a set of root resource and provider instances.
     *
     * @return  A set of root resource and provider instances.
     */
    @Override
    public Set<Object> getSingletons() {
        return providerInstances;
    }

    /**
     * Return a set of root resource and provider classes.
     *
     * @return  A set of root resource and provider classes.
     */
    @Override
    public Set<Class<?>> getClasses() {
        return providerClasses;
    }
}
