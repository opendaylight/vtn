/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.util.List;

/**
 * {@code VBridgeIfPath} class describes the position of the
 * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}
 * in the {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
 *
 * <p>
 *   This class inherits {@link VBridgePath} and also stores the position
 *   information of the {@linkplain <a href="package-summary.html#VTN">VTN</a>}
 *   and the vBridge to which the virtual interface belongs.
 *   An object of this class is used to identify the vBridge interface while
 *   executing any operations against it.
 * </p>
 *
 * @see  <a href="package-summary.html#vBridge">vBridge</a>
 * @see  <a href="package-summary.html#vInterface">Virtual interface</a>
 */
public class VBridgeIfPath extends VBridgePath {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 8804848224089838313L;

    /**
     * The name of the
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}.
     */
    private final String  ifName;

    /**
     * Construct a new object which represents the position of the
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}
     * inside {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *
     * <p>
     *   Exception will not occur even if incorrect name, such as {@code null},
     *   is specified to {@code tenantName}, {@code bridgeName}, or
     *   {@code ifName}, but there will be error if you specify such
     *   {@code VBridgeIfPath} object in API of {@link IVTNManager} service.
     * </p>
     *
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @param ifName      The name of the virtual interface.
     */
    public VBridgeIfPath(String tenantName, String bridgeName, String ifName) {
        super(tenantName, bridgeName);
        this.ifName = ifName;
    }

    /**
     * Construct a new object which represents the position of the
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}
     * inside {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *
     * <p>
     *   This constructor specifies the vBridge to which the virtual interface
     *   belongs by using {@link VBridgePath}.
     * </p>
     *
     * <p>
     *   Exception will not occur even if incorrect name, such as {@code null},
     *   is specified to {@code ifName}, but there will be error if you
     *   specify such {@code VBridgeIfPath} object in API of
     *   {@link IVTNManager} service.
     * </p>
     *
     * @param bridgePath  A {@link VBridgePath} object that specifies the
     *                    position of the vBridge.
     *                    All the values in the specified object are copied to
     *                    a new object.
     * @param ifName      The name of the virtual interface.
     * @throws NullPointerException  {@code bridgePath} is {@code null}.
     */
    public VBridgeIfPath(VBridgePath bridgePath, String ifName) {
        this(bridgePath.getTenantName(), bridgePath.getBridgeName(), ifName);
    }

    /**
     * Return the name of the
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}.
     *
     * @return  The name of the virtual interface.
     */
    public String getInterfaceName() {
        return ifName;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected StringBuilder toStringBuilder() {
        StringBuilder builder = super.toStringBuilder();
        String name = ifName;
        if (name == null) {
            name = "<null>";
        }
        builder.append('.').append(name);

        return builder;
    }

    /**
     * Determine whether all components in the given path equal to components
     * in this object or not.
     *
     * @param path  An object to be compared.
     *              An instance of {@code VBridgeIfPath} must be specified.
     * @return   {@code true} if all path components in {@code path} are
     *           identical to components in this object.
     *           Otherwise {@code false}.
     */
    @Override
    protected boolean equalsPath(VTenantPath path) {
        if (!super.equalsPath(path)) {
            return false;
        }

        VBridgeIfPath ipath = (VBridgeIfPath)path;
        if (ifName == null) {
            return (ipath.ifName == null);
        }

        return ifName.equals(ipath.ifName);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected List<String> getComponents() {
        List<String> components = super.getComponents();
        components.add(ifName);
        return components;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * <p>
     *   {@code true} is returned only if all the following conditions are met.
     * </p>
     * <ul>
     *   <li>
     *     {@code o} is a {@code VBridgeIfPath} object.
     *     Note that this method returns {@code false} if {@code o} is an
     *     object of subclass of {@code VBridgeIfPath}.
     *   </li>
     *   <li>
     *     The following values stored in {@code o} are the same as in this
     *     object.
     *     <ul>
     *       <li>
     *         The name of the
     *         {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
     *       </li>
     *       <li>
     *         The name of the
     *         {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *       </li>
     *       <li>
     *         The name of the
     *         {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}.
     *       </li>
     *     </ul>
     *   </li>
     * </ul>
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (o == null || !o.getClass().equals(getClass())) {
            return false;
        }

        return equalsPath((VBridgeIfPath)o);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = super.hashCode();
        if (ifName != null) {
            h ^= ifName.hashCode();
        }

        return h;
    }
}
