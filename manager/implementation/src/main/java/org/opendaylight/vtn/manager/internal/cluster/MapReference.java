/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.io.Serializable;

import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.VNodeRoute;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VirtualRouteReason;

/**
 * {@code MapReference} class describes a reference to virtual network mapping
 * which maps a physical network to a virtual node.
 *
 * A virtual node referred by a virtual network mapping is represented by
 * a pair of container name and virtual node path in a container.
 */
public class MapReference implements Serializable, Comparable<MapReference> {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 7660028679508618972L;

    /**
     * Mapping type.
     */
    private final MapType  mapType;

    /**
     * The name of the container.
     */
    private final String  containerName;

    /**
     * A path to a virtual node.
     */
    private final VNodePath  vnodePath;

    /**
     * Construct a new object.
     *
     * @param type       A {@link MapType} object which indicates the type
     *                   of virtual network mapping.
     *                   Specifying {@code null} results in undefined behavior.
     * @param container  The name of the container.
     *                   Specifying {@code null} results in undefined behavior.
     * @param path       A path to a virtual node.
     *                   Specifying {@code null} results in undefined behavior.
     */
    public MapReference(MapType type, String container, VNodePath path) {
        mapType = type;
        containerName = container;
        vnodePath = path;
    }

    /**
     * Return the type of the virtual network mapping.
     *
     * @return  A {@link MapType} object which indicates the type of the
     *          virtual network mapping.
     */
    public MapType getMapType() {
        return mapType;
    }

    /**
     * Return the name of the container.
     *
     * @return  The name of the container.
     */
    public String getContainerName() {
        return containerName;
    }

    /**
     * Return a path to a virtual node.
     *
     * @return  A path to a virtual node.
     */
    public VNodePath getPath() {
        return vnodePath;
    }

    /**
     * Return a string which represents an absolute path to the the virtual
     * node where the physical network is mapped.
     *
     * @return  A string which represents an absolute path to the virtual
     *          node.
     */
    public String getAbsolutePath() {
        StringBuilder builder = new StringBuilder(containerName);
        builder.append(':');

        if (vnodePath instanceof VBridgeMapPath) {
            // vnodePath is an instance of internal class.
            // So it should be converted to path to the vBridge.
            VBridgeMapPath bmpath = (VBridgeMapPath)vnodePath;
            builder.append(bmpath.getBridgePath());
        } else {
            builder.append(vnodePath.toString());
        }

        return builder.toString();
    }

    /**
     * Determine whether the target virtual node pointed by this instance is
     * contained in the specified virtual node.
     *
     * @param cname  The name of the container.
     *               Specifying {@code null} results in undefined behavior.
     * @param path   Path to the virtual bridge.
     *               Specifying {@code null} results in undefined behavior.
     * @return  {@code true} is returned if the target virtual node pointed
     *          by this instance is contained in the specified virtual node.
     *          Otherwise {@code false} is returned.
     */
    public boolean isContained(String cname, VNodePath path) {
        return (containerName.equals(cname) && path.contains(vnodePath));
    }

    /**
     * Return a {@link VNodeRoute} instance which represents the ingress
     * virtual node pointed by this instance.
     *
     * @return  A {@link VNodeRoute} instance.
     */
    public VNodeRoute getIngressRoute() {
        VirtualRouteReason reason = mapType.getReason();
        assert reason != null;

        return new VNodeRoute(vnodePath, reason);
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (!(o instanceof MapReference)) {
            return false;
        }

        MapReference ref = (MapReference)o;
        return (mapType == ref.mapType &&
                containerName.equals(ref.containerName) &&
                vnodePath.equals(ref.vnodePath));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return mapType.toString().hashCode() +
            (containerName.hashCode() * 17) +
            (vnodePath.hashCode() * 31);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder(mapType.toString());
        builder.append('@').append(containerName).append(':').
            append(vnodePath.toString());

        return builder.toString();
    }

    // Comparable

    /**
     * Compare two {@code MapReference} instances numerically.
     *
     * @param  ref  A {@code MapReference} instance to be compared.
     * @return   {@code 0} is returned if this instance is equal to
     *           the specified instance.
     *           A value less than {@code 0} is returned if this instance is
     *           numerically less than the specified instance.
     *           A value greater than {@code 0} is returned if this instance is
     *           numerically greater than the specified instance.
     */
    @Override
    public int compareTo(MapReference ref) {
        int ret = mapType.compareTo(ref.mapType);
        if (ret != 0) {
            return ret;
        }

        ret = containerName.compareTo(ref.containerName);
        if (ret != 0) {
            return ret;
        }

        return vnodePath.compareTo(ref.vnodePath);
    }
}
