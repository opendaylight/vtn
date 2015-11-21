/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.VtnVterminalInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.info.VterminalConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.info.VterminalConfigBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.VterminalBuilder;

/**
 * {@code XmlVTerminal} provides XML binding to the data model for vTerminal.
 */
@XmlRootElement(name = "vterminal")
@XmlAccessorType(XmlAccessType.NONE)
public final class XmlVTerminal extends XmlAbstractBridge {
    /**
     * Private constructor only for JAXB.
     */
    private XmlVTerminal() {
    }

    /**
     * Construct a new instance.
     *
     * @param vterm  A {@link VtnVterminalInfo} instance.
     * @throws RpcException
     *    The given instance is invalid.
     */
    public XmlVTerminal(VtnVterminalInfo vterm) throws RpcException {
        super(vterm.getName(), vterm.getVinterface());

        VterminalConfig vtconf = vterm.getVterminalConfig();
        setDescription(vtconf.getDescription());
    }

    /**
     * Convert this instance into a {@link VterminalBuilder} instance.
     *
     * <p>
     *   Note that returned instance contains only configuration for the
     *   vTerminal. Virtual interfaces and flow filters needs to be resumed
     *   by the caller.
     * </p>
     *
     * @return  A {@link VterminalBuilder} instance.
     * @throws RpcException
     *    Failed to convert this instance.
     */
    public VterminalBuilder toVterminalBuilder() throws RpcException {
        VnodeName vname = checkName(VNodeType.VTERMINAL.toString());
        VterminalConfig vtmc = new VterminalConfigBuilder().
            setDescription(getDescription()).
            build();

        return new VterminalBuilder().
            setName(vname).
            setVterminalConfig(vtmc);
    }
}
