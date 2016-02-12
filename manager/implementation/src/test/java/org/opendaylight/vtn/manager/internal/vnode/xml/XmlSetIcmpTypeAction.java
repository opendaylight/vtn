/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.junit.Assert.assertEquals;

import static org.opendaylight.vtn.manager.internal.TestBase.createUnsignedByte;

import java.util.Random;

import org.opendaylight.vtn.manager.internal.util.flow.action.FlowFilterAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetIcmpTypeAction;

import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetIcmpTypeActionCase;

/**
 * {@code XmlSetIcmpTypeAction} describes the configuration of flow action that
 * sets the ICMP type into ICMP header.
 */
public final class XmlSetIcmpTypeAction
    extends XmlFlowAction<VtnSetIcmpTypeActionCase> {
    /**
     * The ICMP type to set.
     */
    private Short  type;

    /**
     * Construct an empty instance.
     */
    public XmlSetIcmpTypeAction() {
        this(null, null);
    }

    /**
     * Construct a new instance.
     *
     * @param ord  The order of the flow action.
     * @param t    The ICMP type to set.
     */
    public XmlSetIcmpTypeAction(Integer ord, Short t) {
        super(ord);
        type = t;
    }

    /**
     * Return the ICMP type to set.
     *
     * @return  The ICMP type to set.
     */
    public Short getType() {
        return type;
    }

    // XmlFlowAction

    /**
     * Return a class that specifies the type of vtn-action.
     *
     * @return  A class of {@link VtnSetIcmpTypeActionCase}.
     */
    @Override
    protected Class<VtnSetIcmpTypeActionCase> getActionType() {
        return VtnSetIcmpTypeActionCase.class;
    }

    /**
     * Return the name of the XML root node.
     *
     * @return  "vtn-set-icmp-type"
     */
    @Override
    protected String getXmlRoot() {
        return "vtn-set-icmp-type";
    }

    /**
     * Create a new vtn-action value.
     *
     * @return  A {@link VtnSetIcmpTypeActionCase} instance.
     */
    @Override
    protected VtnSetIcmpTypeActionCase newVtnAction() {
        return VTNSetIcmpTypeAction.newVtnAction(type);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void setImpl(Random rand) {
        type = createUnsignedByte(rand);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void setXml(XmlNode xnode) {
        if (type != null) {
            xnode.add(new XmlNode("type", type));
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void verifyImpl(FlowFilterAction ffact) {
        assertEquals(VTNSetIcmpTypeAction.class, ffact.getClass());
        VTNSetIcmpTypeAction act = (VTNSetIcmpTypeAction)ffact;
        assertEquals(type.shortValue(), act.getType());
    }
}
