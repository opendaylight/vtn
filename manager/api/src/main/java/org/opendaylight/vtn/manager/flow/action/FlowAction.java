/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.action;

import java.io.Serializable;
import java.net.InetAddress;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlSeeAlso;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.controller.sal.action.Action;
import org.opendaylight.controller.sal.action.ActionType;
import org.opendaylight.controller.sal.action.SetDlDst;
import org.opendaylight.controller.sal.action.SetDlSrc;
import org.opendaylight.controller.sal.action.SetNwDst;
import org.opendaylight.controller.sal.action.SetNwSrc;
import org.opendaylight.controller.sal.action.SetNwTos;
import org.opendaylight.controller.sal.action.SetTpDst;
import org.opendaylight.controller.sal.action.SetTpSrc;
import org.opendaylight.controller.sal.action.SetVlanId;
import org.opendaylight.controller.sal.action.SetVlanPcp;

/**
 * This class describes an abstract information about an action in a flow
 * entry.
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "flowaction")
@XmlAccessorType(XmlAccessType.NONE)
@XmlSeeAlso({
    DropAction.class,
    PopVlanAction.class,
    SetDlDstAction.class,
    SetDlSrcAction.class,
    SetDscpAction.class,
    SetIcmpCodeAction.class,
    SetIcmpTypeAction.class,
    SetInet4DstAction.class,
    SetInet4SrcAction.class,
    SetTpDstAction.class,
    SetTpSrcAction.class,
    SetVlanIdAction.class,
    SetVlanPcpAction.class
})
public abstract class FlowAction implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 6524951361877107145L;

    /**
     * Convert a SAL action into {@code FlowAction} instance.
     *
     * @param act     A SAL action.
     * @param isIcmp  {@code true} means that the given action is applied to
     *                the ICMPv4 packet. If {@code true} is specified,
     *                {@link SetIcmpTypeAction} and {@link SetIcmpCodeAction}
     *                are used to represent SET_TP_SRC and SET_TP_DST actions
     *                respectively. Otherwise {@link SetTpSrcAction} and
     *                {@link SetTpDstAction} are used.
     * @return  A {@link FlowAction} instance converted from the given
     *          SAL action. {@code null} is returned if the given SAL action
     *          is not supported.
     */
    public static final FlowAction create(Action act, boolean isIcmp) {
        if (act != null) {
            ActionType type = act.getType();
            byte[] dladdr;
            int ival;
            InetAddress iaddr;

            switch (type) {
            case DROP:
                return new DropAction();

            case SET_DL_SRC:
                dladdr = ((SetDlSrc)act).getDlAddress();
                return new SetDlSrcAction(dladdr);

            case SET_DL_DST:
                dladdr = ((SetDlDst)act).getDlAddress();
                return new SetDlDstAction(dladdr);

            case SET_VLAN_ID:
                ival = ((SetVlanId)act).getVlanId();
                return new SetVlanIdAction((short)ival);

            case SET_VLAN_PCP:
                ival = ((SetVlanPcp)act).getPcp();
                return new SetVlanPcpAction((byte)ival);

            case POP_VLAN:
                return new PopVlanAction();

            case SET_NW_SRC:
                iaddr = ((SetNwSrc)act).getAddress();
                return new SetInet4SrcAction(iaddr);

            case SET_NW_DST:
                iaddr = ((SetNwDst)act).getAddress();
                return new SetInet4DstAction(iaddr);

            case SET_NW_TOS:
                ival = ((SetNwTos)act).getNwTos();
                return new SetDscpAction((byte)ival);

            case SET_TP_SRC:
                ival = ((SetTpSrc)act).getPort();
                if (isIcmp) {
                    return new SetIcmpTypeAction((short)ival);
                }
                return new SetTpSrcAction(ival);

            case SET_TP_DST:
                ival = ((SetTpDst)act).getPort();
                if (isIcmp) {
                    return new SetIcmpCodeAction((short)ival);
                }
                return new SetTpDstAction(ival);

            default:
                break;
            }
        }

        return null;
    }

    /**
     * Construct a new instance which describes an action in a flow entry.
     */
    FlowAction() {
    }
}
