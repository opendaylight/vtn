/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import javax.xml.bind.annotation.adapters.XmlAdapter;

/**
 * This class is used to establish JAXB mapping between {@link VNodePath} and
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
        String ifname = loc.getInterfaceName();
        String bname = loc.getBridgeName();
        if (bname != null) {
            return (ifname == null)
                ? new VBridgePath(tname, bname)
                : new VBridgeIfPath(tname, bname, ifname);
        }

        String tmname = loc.getTerminalName();
        if (tmname != null) {
            return (ifname == null)
                ? new VTerminalPath(tname, tmname)
                : new VTerminalIfPath(tname, tmname, ifname);
        }

        return new ErrorVNodePath("Unexpected node location: " + loc);
    }

    /**
     * Convert an instance of {@link VNodePath} into an instance of
     * {@link VNodeLocation}.
     *
     * @param path  A {@link VNodePath} instance to be converted.
     * @return  A {@link VNodeLocation} instance.
     *          {@code null} is returned if {@code null} is passed to
     *          {@code path}.
     */
    @Override
    public VNodeLocation marshal(VNodePath path) {
        return (path == null) ? null : path.toVNodeLocation();
    }
}
