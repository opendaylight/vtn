/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util;

import org.opendaylight.controller.md.sal.binding.api.ReadOnlyTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnFlowConditionService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.VtnFlowFilterService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.VtnMacMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.VtnPortMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.VtnVlanMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.VtnPathMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicyService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.VtnService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.VtnMacTableService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnVbridgeService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.version.rev150901.VtnVersionService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.VtnVinterfaceService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.VtnVterminalService;

/**
 * {@code VTNServices} describes a set of services used for integration test.
 */
public interface VTNServices {
    /**
     * Return the vtn RPC service.
     *
     * @return  The vtn RPC service.
     */
    VtnService getVtnService();

    /**
     * Return the vtn-vbridge RPC service.
     *
     * @return  The vtn-vbridge RPC service.
     */
    VtnVbridgeService getVbridgeService();

    /**
     * Return the vtn-terminal RPC service.
     *
     * @return  The vtn-terminal RPC service.
     */
    VtnVterminalService getVterminalService();

    /**
     * Return the vtn-vinterface RPC service.
     *
     * @return  The vtn-vinterface RPC service.
     */
    VtnVinterfaceService getVinterfaceService();

    /**
     * Return the vtn-mac-table RPC service.
     *
     * @return  The vtn-mac-table RPC service.
     */
    VtnMacTableService getMacTableService();

    /**
     * Return the vtn-vlan-map RPC service.
     *
     * @return  The vtn-vlan-map RPC service.
     */
    VtnVlanMapService getVlanMapService();

    /**
     * Return the vtn-mac-map RPC service.
     *
     * @return  The vtn-mac-map RPC service.
     */
    VtnMacMapService getMacMapService();

    /**
     * Return the vtn-port-map RPC service.
     *
     * @return  The vtn-port-map RPC service.
     */
    VtnPortMapService getPortMapService();

    /**
     * Return the vtn-flow-filter RPC service.
     *
     * @return  The vtn-flow-filter RPC service.
     */
    VtnFlowFilterService getFlowFilterService();

    /**
     * Return the vtn-flow-condition RPC service.
     *
     * @return  The vtn-flow-condition RPC service.
     */
    VtnFlowConditionService getFlowConditionService();

    /**
     * Return the vtn-path-policy RPC service.
     *
     * @return  The vtn-path-policy RPC service.
     */
    VtnPathPolicyService getPathPolicyService();

    /**
     * Return the vtn-path-map RPC service.
     *
     * @return  The vtn-path-map RPC service.
     */
    VtnPathMapService getPathMapService();

    /**
     * Return the vtn-version RPC service.
     *
     * @return  The vtn-version RPC service.
     */
    VtnVersionService getVersionService();

    /**
     * Create a new read-only transaction for the MD-SAL datastore.
     *
     * @return  A {@link ReadOnlyTransaction} instance.
     */
    ReadOnlyTransaction newReadOnlyTransaction();

    /**
     * Create a new read-write transaction for the MD-SAL datastore.
     *
     * @return  A {@link ReadWriteTransaction} instance.
     */
    ReadWriteTransaction newReadWriteTransaction();
}
