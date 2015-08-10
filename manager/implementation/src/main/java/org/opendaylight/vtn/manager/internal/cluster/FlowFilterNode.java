/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.io.Serializable;

import org.opendaylight.vtn.manager.VTenantPath;

/**
 * {@code FlowFilterNode} determines interfaces to be implemented by virtual
 * node classes which can contain flow filter.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public interface FlowFilterNode extends Serializable {
    /**
     * Return the name of the container to which this node belongs.
     *
     * @return  The name of the container.
     */
    String getContainerName();

    /**
     * Return path to this node.
     *
     * @return  Path to the node.
     */
    VTenantPath getPath();
}
