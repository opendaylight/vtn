/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.io.Serializable;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

/**
 * {@code VNodeLocation} class describes the location of the virtual node
 * inside the VTN.
 *
 * <p>
 *   This class provides JAXB mapping for the virtual node location represented
 *   by {@link VNodePath} instance. Java application does not need to use
 *   this class.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"tenant": "vtn_1",
 * &nbsp;&nbsp;"bridge": "vbridge_1",
 * &nbsp;&nbsp;"interface": "if_1"
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "vnodelocation")
@XmlAccessorType(XmlAccessType.NONE)
public final class VNodeLocation implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -7246059483559637187L;

    /**
     * The name of the VTN.
     */
    @XmlAttribute(name = "tenant")
    private String  tenantName;

    /**
     * The name of the vBridge.
     *
     * <ul>
     *   <li>
     *     This attribute is omitted if the virtual node specified by this
     *     element is neither a vBridge nor a virtual node attached to the
     *     vBridge.
     *   </li>
     * </ul>
     */
    @XmlAttribute(name = "bridge")
    private String  bridgeName;

    /**
     * The name of the vRouter.
     *
     * <ul>
     *   <li>
     *     This attribute is ignored if the <strong>bridge</strong> attribute
     *     is configured.
     *   </li>
     *   <li>
     *     Currently the VTN Manager never set this attribute because the
     *     virtual router is not yet supported.
     *   </li>
     * </ul>
     */
    @XmlAttribute(name = "router")
    private String  routerName;

    /**
     * The name of the vTerminal.
     *
     * <ul>
     *   <li>
     *     This attribute is ignored if either the <strong>bridge</strong> or
     *     <strong>router</strong> attribute is configured.
     *   </li>
     *   <li>
     *     This attribute is omitted if the virtual node specified by this
     *     element is not a vTerminal.
     *   </li>
     * </ul>
     */
    @XmlAttribute(name = "terminal")
    private String  terminalName;

    /**
     * The name of the virtual interface.
     *
     * <ul>
     *   <li>
     *     If the virtual interface specified by this element is attached to
     *     a vBridge, the name of the vBridge is set to the
     *     <strong>bridge</strong> attribute.
     *   </li>
     *   <li>
     *     If the virtual interface specified by this element is attached to
     *     a vRouter, the name of the vRouter is set to the
     *     <strong>router</strong> attribute.
     *   </li>
     *   <li>
     *     This attribute is omitted if the virtual node specified by this
     *     element is not a virtual interace.
     *   </li>
     * </ul>
     */
    @XmlAttribute(name = "interface")
    private String  interfaceName;

    /**
     * Private constructor used for JAXB mapping.
     */
    @SuppressWarnings("unused")
    private VNodeLocation() {
    }

    /**
     * Construct a new {@code VNodeLocation} instance which represents the
     * location of the vBridge.
     *
     * @param path  A {@link VBridgePath} instance which represents the
     *              location of the vBridge.
     * @throws NullPointerException
     *    {@code null} is passed to {@code path}.
     */
    public VNodeLocation(VBridgePath path) {
        tenantName = path.getTenantName();
        bridgeName = path.getBridgeName();
    }

    /**
     * Construct a new {@code VNodeLocation} instance which represents the
     * location of the virtual interface attached to the vBridge.
     *
     * @param path  A {@link VBridgeIfPath} instance which represents the
     *              location of the virtual interface attached to the vBridge.
     * @throws NullPointerException
     *    {@code null} is passed to {@code path}.
     */
    public VNodeLocation(VBridgeIfPath path) {
        this((VBridgePath)path);
        interfaceName = path.getInterfaceName();
    }

    /**
     * Return the name of the VTN configured in this instance.
     *
     * @return  The name of the VTN.
     *          {@code null} is returned if it is not configured.
     */
    public String getTenantName() {
        return tenantName;
    }

    /**
     * Return the name of the vBridge configured in this instance.
     *
     * @return  The name of the vBridge.
     *          {@code null} is returned if it is not configured.
     */
    public String getBridgeName() {
        return bridgeName;
    }

    /**
     * Return the name of the vRouter configured in this instance.
     *
     * @return  The name of the vRouter.
     *          {@code null} is returned if it is not configured.
     */
    public String getRouterName() {
        return routerName;
    }

    /**
     * Return the name of the vTerminal configured in this instance.
     *
     * @return  The name of the vTerminal.
     *          {@code null} is returned if it is not configured.
     */
    public String getTerminalName() {
        return terminalName;
    }

    /**
     * Return the name of the virtual interface configured in this instance.
     *
     * @return  The name of the virtual interface.
     *          {@code null} is returned if it is not configured.
     */
    public String getInterfaceName() {
        return interfaceName;
    }
}
