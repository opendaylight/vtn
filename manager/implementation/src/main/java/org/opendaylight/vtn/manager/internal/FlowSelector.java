/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.opendaylight.vtn.manager.internal.cluster.VTNFlow;

/**
 * {@code FlowSelector} provides interfaces to be implemented by classes which
 * search for specific flow entries.
 */
public interface FlowSelector {
    /**
     * Test if the specified VTN flow should be accepted or not.
     *
     * @param vflow  A {@link VTNFlow} instance to be tested.
     * @return  {@code true} if the specified flow should be accepted.
     *          {@code false} if the specified flow should be filtered out.
     */
    boolean accept(VTNFlow vflow);

    /**
     * Return a brief description about this filter.
     *
     * @return  A brief description about this filter.
     */
    String getDescription();
}
