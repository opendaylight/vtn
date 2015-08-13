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
 * This class is used to establish JAXB mapping between {@link VInterfacePath}
 * and {@link VNodeLocation}.
 *
 * @since  Helium
 */
public class VInterfacePathAdapter
    extends XmlAdapter<VNodeLocation, VInterfacePath> {
    /**
     * Construct a new instance.
     */
    public VInterfacePathAdapter() {
    }

    /**
     * Convert an instance of {@link VNodeLocation} into
     * {@link VInterfacePath}.
     *
     * @param loc  A {@link VNodeLocation} instance to be converted.
     * @return  A {@link VInterfacePath} instance.
     *          {@code null} is returned if the specified {@link VNodeLocation}
     *          does not point any virtual interface.
     */
    @Override
    public VInterfacePath unmarshal(VNodeLocation loc) {
        if (loc == null) {
            return null;
        }

        String tname = loc.getTenantName();
        String ifname = loc.getInterfaceName();
        if (ifname == null) {
            return new ErrorVNodePath("Interface name is not specified: " +
                                      loc);
        }

        String bname = loc.getBridgeName();
        if (bname != null) {
            return new VBridgeIfPath(tname, bname, ifname);
        }

        String tmname = loc.getTerminalName();
        if (tmname != null) {
            return new VTerminalIfPath(tname, tmname, ifname);
        }

        return new ErrorVNodePath("Unexpected interface location: " + loc);
    }

    /**
     * Convert an instance of {@link VInterfacePath} into an instance of
     * {@link VNodeLocation}.
     *
     * @param path  A {@link VInterfacePath} instance to be converted.
     * @return  A {@link VNodeLocation} instance.
     *          {@code null} is returned a value passed to {@code path} is
     *          {@code null} or an instance of unexpected class.
     */
    @Override
    public VNodeLocation marshal(VInterfacePath path) {
        return (path == null) ? null : path.toVNodeLocation();
    }
}
