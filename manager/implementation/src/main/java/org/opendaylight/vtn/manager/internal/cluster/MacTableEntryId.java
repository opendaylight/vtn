/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.net.InetAddress;

import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;

/**
 * {@code MacTableEntryId} describes an identifier of a MAC address table
 * entry associated with a virtual L2 bridge.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public class MacTableEntryId extends ClusterEventId {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 7249703870323895498L;

    /**
     * Path to the virtual node which originated the entry.
     */
    private final VBridgePath  mapPath;

    /**
     * A long integer value which represents a MAC address.
     */
    private final long  macAddress;

    /**
     * Construct a new ID for a MAC address table entry.
     *
     * <p>
     *   This constructor uses the IP address of this controller, and assigns
     *   a new event ID.
     * </p>
     *
     * @param path   Path to the virtual node which maps the MAC address.
     *               Specifying {@code null} results in undefined behavior.
     * @param mac    A long value which represents a MAC address.
     */
    public MacTableEntryId(VBridgePath path, long mac) {
        mapPath = path;
        macAddress = mac;
    }

    /**
     * Construct a new ID for a MAC address table entry.
     *
     * @param addr   IP address of the controller.
     * @param id     The event ID.
     * @param path   Path to the virtual node which maps the MAC address.
     *               Specifying {@code null} results in undefined behavior.
     * @param mac    A long value which represents a MAC address.
     */
    public MacTableEntryId(InetAddress addr, long id, VBridgePath path,
                           long mac) {
        super(addr, id);
        mapPath = path;
        macAddress = mac;
    }

    /**
     * Return the path to the virtual node which maps the MAC address.
     *
     * @return  The path to the virtual node.
     */
    public VBridgePath getMapPath() {
        return mapPath;
    }

    /**
     * Return the path to the virtual L2 bridge.
     *
     * @return  The path to the virtual L2 bridge.
     */
    public VBridgePath getBridgePath() {
        return new VBridgePath(mapPath.getTenantName(),
                               mapPath.getBridgeName());
    }

    /**
     * Return the long value which represents the MAC address.
     *
     * @return  The long value which represents the MAC address.
     */
    public long getMacAddress() {
        return macAddress;
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
        if (!(o instanceof MacTableEntryId) || !super.equals(o)) {
            return false;
        }

        MacTableEntryId tid = (MacTableEntryId)o;
        return (mapPath.equals(tid.mapPath) &&
                macAddress == tid.macAddress);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return super.hashCode() ^ mapPath.hashCode() ^
            NumberUtils.hashCode(macAddress);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder(mapPath.toString());
        builder.append(SEPARATOR).
            append(MiscUtils.formatMacAddress(macAddress)).
            append(SEPARATOR).append(super.toString());
        return builder.toString();
    }
}
