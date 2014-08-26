/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.filter.DropFilter;
import org.opendaylight.vtn.manager.flow.filter.FlowFilter;

/**
 * This class describes DROP flow filter, which discards packet.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class DropFlowFilterImpl extends FlowFilterImpl {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 4806340158167710257L;

    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(DropFlowFilterImpl.class);

    /**
     * Construct a new instance.
     *
     * @param idx     An index number to be assigned.
     * @param filter  A {@link FlowFilter} instance.
     * @throws VTNException
     *    {@code filter} contains invalid value.
     */
    protected DropFlowFilterImpl(int idx, FlowFilter filter)
        throws VTNException {
        super(idx, filter);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected DropFilter getFilterType() {
        return new DropFilter();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected Logger getLogger() {
        return LOG;
    }
}
