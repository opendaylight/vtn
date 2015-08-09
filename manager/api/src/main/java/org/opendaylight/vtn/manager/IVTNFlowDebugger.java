/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import org.opendaylight.controller.sal.utils.Status;

/**
 * This interface defines OSGi service to handle flow entries for the
 * VTN Manager.
 *
 * <p>
 *   This service is provided only for debugging purpose, and it is exported
 *   only if the system property {@code vtn.debug} is defined as {@code true}.
 * </p>
 * <p>
 *   Although this interface is public to other packages, this interface does
 *   not provide any API. Applications other than the VTN Manager must not use
 *   this interface.
 * </p>
 */
public interface IVTNFlowDebugger {
    /**
     * Remove all flow entries in the specified VTN.
     *
     * @param path   A {@link VTenantPath} object that specifies the position
     *               of the VTN.
     * @return  A {@link Status} object which indicates the result of the
     *          operation.
     */
    Status removeAllFlows(VTenantPath path);
}
