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
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetInetDstAction;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetInetDstActionCase;

/**
 * {@code XmlSetInetDstAtion} describes the configuation of flow entry that
 * sets the destination IP address into IP header.
 */
public final class XmlSetInetDstAction
    extends XmlInetAddrAction<VtnSetInetDstActionCase> {
    /**
     * Construct an empty instance.
     */
    public XmlSetInetDstAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param ord   The order of the flow action.
     * @param addr  The IP address to set.
     */
    public XmlSetInetDstAction(Integer ord, IpNetwork addr) {
        super(ord, addr);
    }

    // XmlFlowAction

    /**
     * Return a class that specifies the type of vtn-action.
     *
     * @return  A class of {@link VtnSetInetDstActionCase}.
     */
    @Override
    protected Class<VtnSetInetDstActionCase> getActionType() {
        return VtnSetInetDstActionCase.class;
    }

    /**
     * Return the name of the XML root node.
     *
     * @return  "vtn-set-inet-dst"
     */
    @Override
    protected String getXmlRoot() {
        return "vtn-set-inet-dst";
    }

    /**
     * Create a new vtn-action value.
     *
     * @return  A {@link VtnSetInetDstActionCase} instance.
     */
    @Override
    protected VtnSetInetDstActionCase newVtnAction() {
        return VTNSetInetDstAction.newVtnAction(getMdAddress());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void verifyImpl(FlowFilterAction ffact) {
        assertEquals(VTNSetInetDstAction.class, ffact.getClass());
        VTNSetInetDstAction act = (VTNSetInetDstAction)ffact;
        assertEquals(getAddress(), act.getAddress());
    }
}
