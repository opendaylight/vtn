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

import org.opendaylight.vtn.manager.PathMap;
import org.opendaylight.vtn.manager.VTNException;

/**
 * Implementation of VTN path map.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class VTenantPathMapImpl extends PathMapImpl {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -5489357380659529588L;

    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VTenantPathMapImpl.class);

    /**
     * Virtual tenant which includes this path map.
     */
    private transient VTenantImpl  parent;

    /**
     * Construct a new instance.
     *
     * @param vtn    A virtual tenant to which this path map belongs.
     * @param idx    An index number to be assigned.
     * @param ptmap  A {@link PathMap} instance.
     * @throws VTNException
     *    {@code ptmap} contains invalid value.
     */
    public VTenantPathMapImpl(VTenantImpl vtn, int idx, PathMap ptmap)
        throws VTNException {
        super(idx, ptmap);
        parent = vtn;
    }

    /**
     * Set the virtual tenant to which this path map belongs.
     *
     * @param vtn  The virtual tenant.
     */
    void setVTenant(VTenantImpl vtn) {
        parent = vtn;
    }

    /**
     * Return a logger instance.
     *
     * @return  A logger instance.
     */
    @Override
    protected Logger getLogger() {
        return LOG;
    }

    /**
     * Return a prefix of log record.
     *
     * @return  A prefix of log record.
     */
    @Override
    protected String getLogPrefix() {
        StringBuilder builder = new StringBuilder(":");
        return builder.append(parent.getName()).append('.').
            append(getIndex()).toString();
    }
}
