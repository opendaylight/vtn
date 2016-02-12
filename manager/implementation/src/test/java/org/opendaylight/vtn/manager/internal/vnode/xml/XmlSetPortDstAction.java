/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.junit.Assert.assertEquals;

import org.opendaylight.vtn.manager.internal.util.flow.action.FlowFilterAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetPortDstAction;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetPortDstActionCase;

/**
 * {@code XmlSetPortDstAction} describes the configuration of flow action that
 * sets the destination port number for IP transport layer protocol into packet.
 */
public final class XmlSetPortDstAction
    extends XmlPortAction<VtnSetPortDstActionCase> {
    /**
     * Construct an empty instance.
     */
    public XmlSetPortDstAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param ord   The order of the flow action.
     * @param pnum  The port number to set.
     */
    public XmlSetPortDstAction(Integer ord, Integer pnum) {
        super(ord, pnum);
    }

    // XmlFlowAction

    /**
     * Return a class that specifies the type of vtn-action.
     *
     * @return  A class of {@link VtnSetPortDstActionCase}.
     */
    @Override
    protected Class<VtnSetPortDstActionCase> getActionType() {
        return VtnSetPortDstActionCase.class;
    }

    /**
     * Return the name of the XML root node.
     *
     * @return  "vtn-set-port-dst"
     */
    @Override
    protected String getXmlRoot() {
        return "vtn-set-port-dst";
    }

    /**
     * Create a new vtn-action value.
     *
     * @return  A {@link VtnSetPortDstActionCase} instance.
     */
    @Override
    protected VtnSetPortDstActionCase newVtnAction() {
        return VTNSetPortDstAction.newVtnAction(getPortNumber());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void verifyImpl(FlowFilterAction ffact) {
        assertEquals(VTNSetPortDstAction.class, ffact.getClass());
        VTNSetPortDstAction act = (VTNSetPortDstAction)ffact;
        assertEquals(getPort().intValue(), act.getPort());
    }
}
