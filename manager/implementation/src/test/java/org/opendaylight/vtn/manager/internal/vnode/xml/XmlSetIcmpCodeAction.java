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
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetIcmpCodeAction;

import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetIcmpCodeActionCase;

/**
 * {@code XmlSetIcmpCodeAction} describes the configuration of flow action that
 * sets the ICMP code into ICMP header.
 */
public final class XmlSetIcmpCodeAction
    extends XmlFlowAction<VtnSetIcmpCodeActionCase> {
    /**
     * The ICMP code to set.
     */
    private Short  code;

    /**
     * Construct an empty instance.
     */
    public XmlSetIcmpCodeAction() {
        this(null, null);
    }

    /**
     * Construct a new instance.
     *
     * @param ord  The order of the flow action.
     * @param c    The ICMP code to set.
     */
    public XmlSetIcmpCodeAction(Integer ord, Short c) {
        super(ord);
        code = c;
    }

    /**
     * Return the ICMP code to set.
     *
     * @return  The ICMP code to set.
     */
    public Short getCode() {
        return code;
    }

    // XmlFlowAction

    /**
     * Return a class that specifies the code of vtn-action.
     *
     * @return  A class of {@link VtnSetIcmpCodeActionCase}.
     */
    @Override
    protected Class<VtnSetIcmpCodeActionCase> getActionType() {
        return VtnSetIcmpCodeActionCase.class;
    }

    /**
     * Return the name of the XML root node.
     *
     * @return  "vtn-set-icmp-code"
     */
    @Override
    protected String getXmlRoot() {
        return "vtn-set-icmp-code";
    }

    /**
     * Create a new vtn-action value.
     *
     * @return  A {@link VtnSetIcmpCodeActionCase} instance.
     */
    @Override
    protected VtnSetIcmpCodeActionCase newVtnAction() {
        return VTNSetIcmpCodeAction.newVtnAction(code);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void setImpl(Random rand) {
        code = createUnsignedByte(rand);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void setXml(XmlNode xnode) {
        if (code != null) {
            xnode.add(new XmlNode("code", code));
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void verifyImpl(FlowFilterAction ffact) {
        assertEquals(VTNSetIcmpCodeAction.class, ffact.getClass());
        VTNSetIcmpCodeAction act = (VTNSetIcmpCodeAction)ffact;
        assertEquals(code.shortValue(), act.getCode());
    }
}
