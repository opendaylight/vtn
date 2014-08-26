/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.action.SetTpSrcAction;

/**
 * Implementation of flow action that modifies source port number in
 * TCP or UDP header.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class SetTpSrcActionImpl extends TpPortActionImpl {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 644126347054787614L;

    /**
     * Construct a new instance.
     *
     * @param act  A {@link SetTpSrcAction} instance.
     * @throws VTNException
     *    {@code act} contains invalid value.
     */
    public SetTpSrcActionImpl(SetTpSrcAction act) throws VTNException {
        super(act);
    }

    // FlowActionImpl

    /**
     * {@inheritDoc}
     */
    @Override
    public SetTpSrcAction getFlowAction() {
        return new SetTpSrcAction(getPort());
    }
}
