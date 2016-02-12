/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.junit.Assert.assertEquals;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.util.flow.action.FlowFilterAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetDlDstAction;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetDlDstActionCase;

/**
 * {@code XmlSetDlDstAction} describes the configuration of flow action that
 * sets the destination MAC address into Ethernet header.
 */
public final class XmlSetDlDstAction
    extends XmlDlAddrAction<VtnSetDlDstActionCase> {
    /**
     * Construct an empty instance.
     */
    public XmlSetDlDstAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param ord   The order of the flow action.
     * @param addr  The MAC address to set.
     */
    public XmlSetDlDstAction(Integer ord, EtherAddress addr) {
        super(ord, addr);
    }

    // XmlFlowAction

    /**
     * Return a class that specifies the type of vtn-action.
     *
     * @return  A class of {@link VtnSetDlDstActionCase}.
     */
    @Override
    protected Class<VtnSetDlDstActionCase> getActionType() {
        return VtnSetDlDstActionCase.class;
    }

    /**
     * Return the name of the XML root node.
     *
     * @return  "vtn-set-dl-dst"
     */
    @Override
    protected String getXmlRoot() {
        return "vtn-set-dl-dst";
    }

    /**
     * Create a new vtn-action value.
     *
     * @return  A {@link VtnSetDlDstActionCase} instance.
     */
    @Override
    protected VtnSetDlDstActionCase newVtnAction() {
        return VTNSetDlDstAction.newVtnAction(getMacAddress());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void verifyImpl(FlowFilterAction ffact) {
        assertEquals(VTNSetDlDstAction.class, ffact.getClass());
        VTNSetDlDstAction act = (VTNSetDlDstAction)ffact;
        assertEquals(getAddress(), act.getAddress());
    }
}
