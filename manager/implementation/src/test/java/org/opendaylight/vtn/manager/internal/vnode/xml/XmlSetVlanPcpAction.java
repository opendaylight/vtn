/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.junit.Assert.assertEquals;

import static org.opendaylight.vtn.manager.internal.TestBase.createVlanPcp;

import java.util.Random;

import org.opendaylight.vtn.manager.internal.util.flow.action.FlowFilterAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetVlanPcpAction;

import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetVlanPcpActionCase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanPcp;

/**
 * {@code XmlSetVlanPcpAction} describes the configuration of flow action that
 * sets the VLAN priority into a VLAN tag.
 */
public final class XmlSetVlanPcpAction
    extends XmlFlowAction<VtnSetVlanPcpActionCase> {
    /**
     * The VLAN priority to set.
     */
    private Short  priority;

    /**
     * Construct an empty instance.
     */
    public XmlSetVlanPcpAction() {
        this(null, null);
    }

    /**
     * Construct a new instance.
     *
     * @param ord  The order of the flow action.
     * @param pcp  The VLAN priority to set.
     */
    public XmlSetVlanPcpAction(Integer ord, Short pcp) {
        super(ord);
        priority = pcp;
    }

    /**
     * Return the VLAN priority to set.
     *
     * @return  The VLAN priority to set.
     */
    public Short getPriority() {
        return priority;
    }

    // XmlFlowAction

    /**
     * Return a class that specifies the type of vtn-action.
     *
     * @return  A class of {@link VtnSetVlanPcpActionCase}.
     */
    @Override
    protected Class<VtnSetVlanPcpActionCase> getActionType() {
        return VtnSetVlanPcpActionCase.class;
    }

    /**
     * Return the name of the XML root node.
     *
     * @return  "vtn-set-vlan-pcp"
     */
    @Override
    protected String getXmlRoot() {
        return "vtn-set-vlan-pcp";
    }

    /**
     * Create a new vtn-action value.
     *
     * @return  A {@link VtnSetVlanPcpActionCase} instance.
     */
    @Override
    protected VtnSetVlanPcpActionCase newVtnAction() {
        VlanPcp pcp = (priority == null) ? null : new VlanPcp(priority);
        return VTNSetVlanPcpAction.newVtnAction(pcp);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void setImpl(Random rand) {
        priority = createVlanPcp(rand);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void setXml(XmlNode xnode) {
        if (priority != null) {
            xnode.add(new XmlNode("priority", priority));
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void verifyImpl(FlowFilterAction ffact) {
        assertEquals(VTNSetVlanPcpAction.class, ffact.getClass());
        VTNSetVlanPcpAction act = (VTNSetVlanPcpAction)ffact;
        assertEquals(priority.shortValue(), act.getPriority());
    }
}
