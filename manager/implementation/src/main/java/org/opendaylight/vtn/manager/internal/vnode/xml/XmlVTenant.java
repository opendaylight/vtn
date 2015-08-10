/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlElementWrapper;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.vtn.manager.PathMap;

import org.opendaylight.vtn.manager.internal.util.pathmap.PathMapUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantUtils;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.VtnInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtnPathMaps;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnBuilder;

/**
 * {@code XmlVTenant} provides XML binding to the data model for VTN.
 */
@XmlRootElement(name = "vtn")
@XmlAccessorType(XmlAccessType.NONE)
public final class XmlVTenant extends XmlVNode {
    /**
     * A list of VTN path maps.
     */
    @XmlElementWrapper(name = "vtn-path-maps")
    @XmlElement(name = "vtn-path-map")
    private List<PathMap>  pathMaps;

    /**
     * Private constructor only for JAXB.
     */
    @SuppressWarnings("unused")
    private XmlVTenant() {
    }

    /**
     * Construct a new instance.
     *
     * @param vtn  A {@link VtnInfo} instance.
     * @throws RpcException  An error occurred.
     */
    public XmlVTenant(VtnInfo vtn) throws RpcException {
        super(vtn.getName());
        initPathMaps(vtn);
    }

    /**
     * Create a {@link VtnBuilder} instance that contains the VTN
     * configuration.
     *
     * @return  A {@link VtnBuilder} instance.
     * @throws RpcException  An error occurred.
     */
    public VtnBuilder toVtnBuilder() throws RpcException {
        VtnBuilder builder = new VtnBuilder().
            setName(VTenantUtils.checkName(getName()));
        return builder;
    }

    /**
     * Return a list of VTN path map configurations.
     *
     * @return  A list of {@link PathMap} instances or {@code null}.
     */
    public List<PathMap> getPathMaps() {
        return pathMaps;
    }

    /**
     * Initialize VTN path map list.
     *
     * @param vtn  A {@link VtnInfo} instance.
     * @throws RpcException  An error occurred.
     */
    private void initPathMaps(VtnInfo vtn) throws RpcException {
        VtnPathMaps root = vtn.getVtnPathMaps();
        if (root == null) {
            return;
        }

        List<VtnPathMap> vlist = root.getVtnPathMap();
        if (vlist == null || vlist.isEmpty()) {
            return;
        }

        List<PathMap> list = new ArrayList<>(vlist.size());
        pathMaps = list;
        for (VtnPathMap vpm: vlist) {
            list.add(PathMapUtils.toPathMap(vpm));
        }
    }

    // Object

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
        if (!super.equals(o)) {
            return false;
        }

        XmlVTenant xvtn = (XmlVTenant)o;
        return Objects.equals(pathMaps, xvtn.pathMaps);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return super.hashCode() + Objects.hash(pathMaps);
    }
}
