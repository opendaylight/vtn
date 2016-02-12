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
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetDlSrcAction;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetDlSrcActionCase;

/**
 * {@code XmlSetDlSrcAction} describes the configuration of flow action that
 * sets the source MAC address into Ethernet header.
 */
public final class XmlSetDlSrcAction
    extends XmlDlAddrAction<VtnSetDlSrcActionCase> {
    /**
     * Construct an empty instance.
     */
    public XmlSetDlSrcAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param ord   The order of the flow action.
     * @param addr  The MAC address to set.
     */
    public XmlSetDlSrcAction(Integer ord, EtherAddress addr) {
        super(ord, addr);
    }

    // XmlFlowAction

    /**
     * Return a class that specifies the type of vtn-action.
     *
     * @return  A class of {@link VtnSetDlSrcActionCase}.
     */
    @Override
    protected Class<VtnSetDlSrcActionCase> getActionType() {
        return VtnSetDlSrcActionCase.class;
    }

    /**
     * Return the name of the XML root node.
     *
     * @return  "vtn-set-dl-src"
     */
    @Override
    protected String getXmlRoot() {
        return "vtn-set-dl-src";
    }

    /**
     * Create a new vtn-action value.
     *
     * @return  A {@link VtnSetDlSrcActionCase} instance.
     */
    @Override
    protected VtnSetDlSrcActionCase newVtnAction() {
        return VTNSetDlSrcAction.newVtnAction(getMacAddress());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void verifyImpl(FlowFilterAction ffact) {
        assertEquals(VTNSetDlSrcAction.class, ffact.getClass());
        VTNSetDlSrcAction act = (VTNSetDlSrcAction)ffact;
        assertEquals(getAddress(), act.getAddress());
    }
}
