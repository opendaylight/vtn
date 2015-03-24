/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import java.net.Inet4Address;
import java.net.InetAddress;
import java.util.Collection;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code MiscUtils} class is a collection of miscellaneous utility class
 * methods.
 */
public final class MiscUtils {
    /**
     * A logger for extra verbose logging.
     */
    public static final Logger  VERBOSE_LOG =
        LoggerFactory.getLogger("ODL-VTN-Manager-verbose");

    /**
     * A prime number used to calculate hash code.
     */
    public static final int  HASH_PRIME = 31;

    /**
     * Private constructor that protects this class from instantiating.
     */
    private MiscUtils() {}

    /**
     * Convert a long value which represents a MAC address into a string.
     *
     * @param mac  A long value which represents a MAC address.
     * @return  A string representation of MAC address.
     */
    public static String formatMacAddress(long mac) {
        return new EtherAddress(mac).getText();
    }

    /**
     * Check the specified resource name.
     *
     * @param desc  Brief description of the resource.
     * @param name  The name of the resource.
     * @return  A {@link VnodeName} instance that contains the given name.
     * @throws RpcException  The specified name is invalid.
     */
    public static VnodeName checkName(String desc, String name)
        throws RpcException {
        if (name == null) {
            throw getNullArgumentException(desc + " name");
        }

        if (name.isEmpty()) {
            throw RpcException.getBadArgumentException(
                desc + " name cannot be empty");
        }

        try {
            return new VnodeName(name);
        } catch (RuntimeException e) {
            String msg = desc + " name is invalid";
            RpcException re = RpcException.getBadArgumentException(msg);
            re.initCause(e);
            throw re;
        }
    }

    /**
     * Check the specified resource name.
     *
     * @param desc   Brief description of the resource.
     * @param vname  A {@link VnodeName} instance.
     * @return  Return the string configured in {@code vname}.
     * @throws RpcException  The specified name is invalid.
     */
    public static String checkName(String desc, VnodeName vname)
        throws RpcException {
        String name = (vname == null) ? null : vname.getValue();
        checkName(desc, name);
        return name;
    }

    /**
     * Return a failure status which represents a {@code null} is specified
     * unexpectedly.
     *
     * @param desc  Brief description of the argument.
     * @return  A failure reason.
     */
    public static Status argumentIsNull(String desc) {
        String msg = desc + " cannot be null";
        return new Status(StatusCode.BADREQUEST, msg);
    }

    /**
     * Return a new {@link RpcException} which indicates a {@code null} is
     * specified as argument unexpectedly.
     *
     * @param desc  Brief description of the argument.
     * @return  An {@link RpcException}.
     */
    public static RpcException getNullArgumentException(String desc) {
        Status st = argumentIsNull(desc);
        return new RpcException(RpcErrorTag.MISSING_ELEMENT, st);
    }

    /**
     * Convert an integer into an IPv4 address.
     *
     * @param address  An integer value which represents an IPv4 address.
     * @return  An {@link InetAddress} instance.
     * @throws IllegalStateException
     *    An error occurred.
     */
    public static InetAddress toInetAddress(int address) {
        byte[] addr = NumberUtils.toBytes(address);
        try {
            return InetAddress.getByAddress(addr);
        } catch (Exception e) {
            // This should never happen.
            StringBuilder builder =
                new StringBuilder("Unexpected exception: addr=");
            builder.append(Integer.toHexString(address));
            throw new IllegalStateException(builder.toString(), e);
        }
    }

    /**
     * Convert an IPv4 address into an integer.
     *
     * @param addr  A {@link InetAddress} instance which represents an IPv4
     *              address.
     * @return  An integer value.
     * @throws IllegalStateException
     *    An error occurred.
     */
    public static int toInteger(InetAddress addr) {
        if (addr instanceof Inet4Address) {
            byte[] bytes = addr.getAddress();
            return NumberUtils.toInteger(bytes);
        }

        StringBuilder builder =
            new StringBuilder("Unexpected InetAddress: addr=");
        builder.append(addr);
        throw new IllegalStateException(builder.toString());
    }

    /**
     * Copy the contents of the given packet.
     *
     * @param src  The source {@link Packet} instance.
     * @param dst  The destination {@link Packet} instance.
     * @param <T>  Type of packet.
     * @return  {@code dst}.
     * @throws VTNException
     *    Failed to copy the packet.
     */
    public static <T extends Packet> T copy(T src, T dst) throws VTNException {
        try {
            byte[] raw = src.serialize();
            int nbits = raw.length * Byte.SIZE;
            dst.deserialize(raw, 0, nbits);
            return dst;
        } catch (Exception e) {
            // This should never happen.
            throw new VTNException("Failed to copy the packet.", e);
        }
    }

    /**
     * Ensure that the given value is not null.
     *
     * @param obj   A reference to the object.
     * @param log   A {@link Logger} instance used to record error message.
     * @param msg   A error message.
     * @param <T>   Type of {@code obj}.
     * @return  {@code obj} is returned if it is not {@code null}.
     * @throws IllegalStateException
     *    {@code obj} is {@code null}.
     */
    public static <T> T checkNotNull(T obj, Logger log, String msg) {
        if (obj != null) {
            return obj;
        }

        IllegalStateException e = new IllegalStateException(msg);
        log.error(msg, e);
        throw e;
    }

    /**
     * Stringify the given objects, and join them with inserting ": " as a
     * separator.
     *
     * @param objs  Objects to be joined.
     * @return  A joined string.
     */
    public static String joinColon(Object ... objs) {
        return join(": ", objs);
    }

    /**
     * Stringify the given objects, and join them with inserting the given
     * separator.
     *
     * @param separator  A separater to be inserted.
     * @param objs       Objects to be joined.
     * @return  A joined string.
     */
    public static String join(String separator, Object ... objs) {
        StringBuilder builder = new StringBuilder();
        String sep = "";
        if (objs != null) {
            for (Object o: objs) {
                builder.append(sep).append(String.valueOf(o));
                sep = separator;
            }
        }

        return builder.toString();
    }

    /**
     * Stringify all the elements in the given collection, and join them with
     * inserting the given separator.
     *
     * @param separator  A separater to be inserted.
     * @param objs       A collection of objects to be joined.
     * @return  A joined string.
     */
    public static String join(String separator, Collection<?> objs) {
        StringBuilder builder = new StringBuilder();
        String sep = "";
        if (objs != null) {
            for (Object o: objs) {
                builder.append(sep).append(String.valueOf(o));
                sep = separator;
            }
        }

        return builder.toString();
    }

    /**
     * Convert the given {@link VtnUpdateType} into {@link UpdateType}.
     *
     * @param vu  A {@link VtnUpdateType} instance.
     * @return  A {@link UpdateType} instance.
     */
    public static UpdateType toUpdateType(VtnUpdateType vu) {
        if (vu == null) {
            return null;
        }
        if (vu.equals(VtnUpdateType.CREATED)) {
            return UpdateType.ADDED;
        }
        if (vu.equals(VtnUpdateType.REMOVED)) {
            return UpdateType.REMOVED;
        }

        return UpdateType.CHANGED;
    }
}
