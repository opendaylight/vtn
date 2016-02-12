/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.junit.Assert.assertEquals;

import static org.opendaylight.vtn.manager.internal.TestBase.createDscp;

import java.util.Random;

import org.opendaylight.vtn.manager.internal.util.flow.action.FlowFilterAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetInetDscpAction;

import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetInetDscpActionCase;

/**
 * {@code XmlSetInetDscpAction} describes the configuration of flow action that
 * sets the DSCP value into IP header.
 */
public final class XmlSetInetDscpAction
    extends XmlFlowAction<VtnSetInetDscpActionCase> {
    /**
     * The DSCP value to set.
     */
    private Short  dscp;

    /**
     * Construct an empty instance.
     */
    public XmlSetInetDscpAction() {
        this(null, null);
    }

    /**
     * Construct a new instance.
     *
     * @param ord  The order of the flow action.
     * @param d    The DSCP value to set.
     */
    public XmlSetInetDscpAction(Integer ord, Short d) {
        super(ord);
        dscp = d;
    }

    /**
     * Return the DSCP value to set.
     *
     * @return  The DSCP value to set.
     */
    public Short getDscp() {
        return dscp;
    }

    // XmlFlowAction

    /**
     * Return a class that specifies the type of vtn-action.
     *
     * @return  A class of {@link VtnSetInetDscpActionCase}.
     */
    @Override
    protected Class<VtnSetInetDscpActionCase> getActionType() {
        return VtnSetInetDscpActionCase.class;
    }

    /**
     * Return the name of the XML root node.
     *
     * @return  "vtn-set-inet-dscp"
     */
    @Override
    protected String getXmlRoot() {
        return "vtn-set-inet-dscp";
    }

    /**
     * Create a new vtn-action value.
     *
     * @return  A {@link VtnSetInetDscpActionCase} instance.
     */
    @Override
    protected VtnSetInetDscpActionCase newVtnAction() {
        return VTNSetInetDscpAction.newVtnAction(dscp);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void setImpl(Random rand) {
        dscp = createDscp(rand);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void setXml(XmlNode xnode) {
        if (dscp != null) {
            xnode.add(new XmlNode("dscp", dscp));
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void verifyImpl(FlowFilterAction ffact) {
        assertEquals(VTNSetInetDscpAction.class, ffact.getClass());
        VTNSetInetDscpAction act = (VTNSetInetDscpAction)ffact;
        assertEquals(dscp.shortValue(), act.getDscp());
    }
}
