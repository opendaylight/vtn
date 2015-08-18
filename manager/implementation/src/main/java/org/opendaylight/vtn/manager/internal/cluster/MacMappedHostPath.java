/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import static org.opendaylight.vtn.manager.util.NumberUtils.HASH_PRIME;

import java.util.List;
import java.util.Objects;

import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VTenantPath;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.BridgeMapInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.BridgeMapInfoBuilder;

/**
 * {@code MacMappedHostPath} class describes a layer 2 host mapped by the
 * MAC mapping.
 *
 * <p>
 *   The layer 2 host mapped by the MAC mapping is identified by the following
 *   attributes.
 * </p>
 * <ul>
 *   <li>The name of the VTN</li>
 *   <li>The name of the vBridge</li>
 *   <li>MAC address</li>
 *   <li>VLAN ID</li>
 * </ul>
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public class MacMappedHostPath extends MacMapPath {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -111389535169763497L;

    /**
     * A string which represents that the node type is MAC mapped host.
     */
    private static final String  NODETYPE_MACMAPPEDHOST = "MacMappedHost";

    /**
     * A {@link MacVlan} instance which specifies the layer 2 host mapped by
     * the MAC mapping.
     */
    private final MacVlan  mappedHost;

    /**
     * Construct a path to the layer 2 host mapped by the MAC mapping.
     *
     * @param bridgePath  Path to the virtual bridge.
     * @param host        A {@link MacVlan} instance which indicates the layer
     *                    2 host mapped by the MAC mapping.
     * @throws NullPointerException  {@code bridgePath} is {@code null}.
     */
    public MacMappedHostPath(VBridgePath bridgePath, MacVlan host) {
        super(bridgePath);
        mappedHost = host;
    }

    /**
     * Return the layer 2 host in this instance.
     *
     * @return  A {@link MacVlan} instance which specifies the layer 2 host
     *          if this instance contains the mapped host information.
     *          {@code null} if no mapped host is configured in this instance.
     */
    public MacVlan getMappedHost() {
        return mappedHost;
    }


    // VBridgeMapPath

    /**
     * Return a {@link BridgeMapInfo} instance which represents the virtual
     * network mapping information.
     *
     * @return  A {@link BridgeMapInfo} instance which contains the layer 2
     *          host information mapped by the MAC mapping.
     */
    @Override
    public BridgeMapInfo getBridgeMapInfo() {
        if (mappedHost == null) {
            return super.getBridgeMapInfo();
        }

        return new BridgeMapInfoBuilder().
            setMacMappedHost(mappedHost.getEncodedValue()).build();
    }

    // VTenantPath

    /**
     * {@inheritDoc}
     *
     * @return  {@code "MacMappedHost"} is always returned.
     */
    @Override
    public String getNodeType() {
        return NODETYPE_MACMAPPEDHOST;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected StringBuilder toStringBuilder() {
        StringBuilder builder = super.toStringBuilder().append('.');
        if (mappedHost == null) {
            builder.append("<null>");
        } else {
            builder.append(mappedHost.getEncodedValue());
        }

        return builder;
    }

    /**
     * Determine whether all components in the given path equal to components
     * in this object or not.
     *
     * @param path  An object to be compared.
     *              An instance of {@code MacMappedHostPath} must be specified.
     * @return   {@code true} if all path components in {@code path} are
     *           identical to components in this object.
     *           Otherwise {@code false}.
     */
    @Override
    protected boolean equalsPath(VTenantPath path) {
        if (!super.equalsPath(path)) {
            return false;
        }

        MacMappedHostPath mhpath = (MacMappedHostPath)path;
        return Objects.equals(mappedHost, mhpath.mappedHost);
    }

    /**
     * Calculate the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    protected int getHash() {
        int h = super.getHash();
        if (mappedHost != null) {
            h = h * HASH_PRIME + mappedHost.hashCode();
        }
        return h;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected List<String> getComponents() {
        List<String> components = super.getComponents();
        String host = (mappedHost == null)
            ? null
            : Long.toString(mappedHost.getEncodedValue());
        components.add(host);
        return components;
    }
}
