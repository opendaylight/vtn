/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import javax.xml.bind.annotation.adapters.XmlAdapter;

/**
 * This class is used to establish JAXB mapping between {@code VNodePath} and
 * {@link VNodeLocation}.
 *
 * @since  Helium
 */
public class VNodePathAdapter extends XmlAdapter<VNodeLocation, VNodePath> {
    /**
     * Construct a new instance.
     */
    public VNodePathAdapter() {
    }

    /**
     * Convert an instance of {@link VNodeLocation} into {@link VNodePath}.
     *
     * @param loc  A {@link VNodeLocation} instance to be converted.
     * @return  A {@link VNodePath} instance.
     *          {@code null} is returned if the specified {@link VNodeLocation}
     *          does not point any virtual node.
     */
    @Override
    public VNodePath unmarshal(VNodeLocation loc) {
        if (loc == null) {
            return null;
        }

        String tname = loc.getTenantName();
        if (tname == null) {
            return null;
        }

        String ifname = loc.getInterfaceName();
        String bname = loc.getBridgeName();
        if (bname != null) {
            return (ifname == null)
                ? new VBridgePath(tname, bname)
                : new VBridgeIfPath(tname, bname, ifname);
        }

        return null;
    }

    /**
     * Convert an instance of {@link VNodePath} into {@link VNodeLocation}.
     *
     * @param path  A {@link VNodePath} instance to be converted.
     * @return  A {@link VNodeLocation} instance.
     *          {@code null} is returned a value passed to {@code path} is
     *          {@code null} or an instance of unexpected class.
     */
    @Override
    public VNodeLocation marshal(VNodePath path) {
        if (path instanceof VBridgeIfPath) {
            return new VNodeLocation((VBridgeIfPath)path);
        }
        if (path instanceof VBridgePath) {
            return new VNodeLocation((VBridgePath)path);
        }

        return null;
    }
}
