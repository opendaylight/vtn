/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.junit.Assert.assertEquals;

import org.opendaylight.vtn.manager.util.IpNetwork;

import org.opendaylight.vtn.manager.internal.util.flow.action.FlowFilterAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetInetSrcAction;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetInetSrcActionCase;

/**
 * {@code XmlSetInetSrcAtion} describes the configuation of flow entry that
 * sets the source IP address into IP header.
 */
public final class XmlSetInetSrcAction
    extends XmlInetAddrAction<VtnSetInetSrcActionCase> {
    /**
     * Construct an empty instance.
     */
    public XmlSetInetSrcAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param ord   The order of the flow action.
     * @param addr  The IP address to set.
     */
    public XmlSetInetSrcAction(Integer ord, IpNetwork addr) {
        super(ord, addr);
    }

    // XmlFlowAction

    /**
     * Return a class that specifies the type of vtn-action.
     *
     * @return  A class of {@link VtnSetInetSrcActionCase}.
     */
    @Override
    protected Class<VtnSetInetSrcActionCase> getActionType() {
        return VtnSetInetSrcActionCase.class;
    }

    /**
     * Return the name of the XML root node.
     *
     * @return  "vtn-set-inet-src"
     */
    @Override
    protected String getXmlRoot() {
        return "vtn-set-inet-src";
    }

    /**
     * Create a new vtn-action value.
     *
     * @return  A {@link VtnSetInetSrcActionCase} instance.
     */
    @Override
    protected VtnSetInetSrcActionCase newVtnAction() {
        return VTNSetInetSrcAction.newVtnAction(getMdAddress());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void verifyImpl(FlowFilterAction ffact) {
        assertEquals(VTNSetInetSrcAction.class, ffact.getClass());
        VTNSetInetSrcAction act = (VTNSetInetSrcAction)ffact;
        assertEquals(getAddress(), act.getAddress());
    }
}
