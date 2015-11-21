/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.filter;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.VtnFlowFilterType;

/**
 * {@code UnknownFlowFilterCase} describes a flow filter that is never
 * supported.
 */
public final class UnknownFlowFilterCase implements VtnFlowFilterType {
    /**
     * Default constructor.
     */
    public UnknownFlowFilterCase() {
    }

    /**
     * Return a class that indicates the type of this data model.
     *
     * @return  A class for {@link VtnFlowFilterType}.
     */
    public Class<VtnFlowFilterType> getImplementedInterface() {
        return VtnFlowFilterType.class;
    }
}
