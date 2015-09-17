/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import java.math.BigInteger;
import java.net.Inet4Address;
import java.net.InetAddress;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import java.util.Set;

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

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Uri;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.Counter32;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.Counter64;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

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
     * The minimum value of the order value for the first element in the
     * ordered list.
     */
    public static final int  ORDER_MIN = 0;

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
     * Convert the given {@link MacAddress} instance into an
     * {@link EtherAddress} instance.
     *
     * @param mac  A {@link MacAddress} instance.
     * @return  An {@link EtherAddress} instance.
     *          {@code null} if {@code mac} is {@code null}.
     * @throws RpcException
     *    The given MAC address is invalid.
     */
    public static EtherAddress toEtherAddress(MacAddress mac)
        throws RpcException {
        try {
            return EtherAddress.create(mac);
        } catch (RuntimeException e) {
            // This should never happen.
            String msg = "Invalid MAC address: " + mac;
            RpcException re = RpcException.getBadArgumentException(msg);
            re.initCause(e);
            throw re;
        }
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

    /**
     * Cast an object type to the specified type.
     *
     * @param type  A class that indicates the target type.
     * @param obj   An object to be casted.
     * @param <T>   The target type.
     * @return  A casted object if the given object can be casted to the
     *          specified type. Otherwise {@code null}.
     */
    public static <T> T cast(Class<T> type, Object obj) {
        return (type.isInstance(obj)) ? type.cast(obj) : null;
    }

    /**
     * Cast an object type to the specified type.
     *
     * @param type  A class that indicates the target type.
     * @param obj   An object to be casted.
     * @param <T>   The target type.
     * @return  A casted object if the given object can be casted to the
     *          specified type.
     * @throws DataTypeMismatchException
     *    The type of {@code obj} does not match the type specified by
     *    {@code type}.
     */
    public static <T> T checkedCast(Class<T> type, Object obj)
        throws DataTypeMismatchException {
        if (type.isInstance(obj)) {
            return type.cast(obj);
        }

        throw new DataTypeMismatchException(type, obj);
    }

    /**
     * Convert all of the characters in the given string to lower case.
     *
     * @param str  A string to be converted.
     * @return  A converted string.
     */
    public static String toLowerCase(String str) {
        return str.toLowerCase(Locale.ENGLISH);
    }

    /**
     * Return an {@link IllegalStateException} which indicates unexpected
     * method is called.
     *
     * @return  An {@link IllegalStateException} instance.
     */
    public static IllegalStateException unexpected() {
        return new IllegalStateException("Should never be called.");
    }

    /**
     * Return the value in the given 32-bit counter.
     *
     * @param c  The 32-bit counter.
     * @return  A long value configured in the given counter.
     *          Node that zero is returned if {@code c} is invalid.
     */
    public static long longValue(Counter32 c) {
        if (c != null) {
            Long v = c.getValue();
            if (v != null) {
                return v.longValue();
            }
        }

        return 0;
    }

    /**
     * Return the value in the given 64-bit counter.
     *
     * @param c  The 64-bit counter.
     * @return  A long value configured in the given counter.
     *          Node that zero is returned if {@code c} is invalid.
     */
    public static long longValue(Counter64 c) {
        if (c != null) {
            BigInteger v = c.getValue();
            if (v != null) {
                return v.longValue();
            }
        }

        return 0;
    }

    /**
     * Return the value in the given 32-bit counter.
     *
     * @param c  The 32-bit counter.
     * @return  A double value configured in the given counter.
     *          Node that zero is returned if {@code c} is invalid.
     */
    public static double doubleValue(Counter32 c) {
        if (c != null) {
            Long v = c.getValue();
            if (v != null) {
                return v.doubleValue();
            }
        }

        return 0d;
    }

    /**
     * Return the value in the given 64-bit counter.
     *
     * @param c  The 64-bit counter.
     * @return  A double value configured in the given counter.
     *          Node that zero is returned if {@code c} is invalid.
     */
    public static double doubleValue(Counter64 c) {
        if (c != null) {
            BigInteger v = c.getValue();
            if (v != null) {
                return v.doubleValue();
            }
        }

        return 0d;
    }

    /**
     * Return the copy of the given list sorted by the given comparator.
     *
     * @param list  A list to be copied.
     * @param comp  A comparator for the given list.
     * @param <T>   The type of elements in the list.
     * @return  A copy of the given list.
     */
    public static <T> List<T> sortedCopy(List<T> list,
                                         Comparator<? super T> comp) {
        List<T> sorted = new ArrayList<>(list);
        Collections.sort(sorted, comp);
        return sorted;
    }

    /**
     * Determine whether the given two URIs are identical or not.
     *
     * <p>
     *   Note that this method compares only strings configured in the given
     *   URIs. The type of classes are never compared.
     *
     * </p>
     *
     * @param u1  The first instance to be compared.
     * @param u2  The second instance to be compared.
     * @return  {@code true} only if {@code vh1} and {@code vh2} are identical.
     */
    public static boolean equalsUri(Uri u1, Uri u2) {
        if (u1 == null) {
            return (u2 == null);
        } else if (u2 == null) {
            return false;
        }

        return Objects.equals(u1.getValue(), u2.getValue());
    }

    /**
     * Return a string configured in the given URI.
     *
     * @param uri  An {@link Uri} instance.
     * @return  A string configured in the given URI.
     *          Note that {@code null} is returned if {@code uri} is
     *          {@code null}.
     */
    public static String getValue(Uri uri) {
        return (uri == null) ? null : uri.getValue();
    }

    /**
     * Return an unmodifiable list equivalent to the given list.
     *
     * @param src  The source list.
     * @param <T>  The type of elements in the list.
     * @return  An unmodifiable list. An empty list is returned if
     *          {@code src} is {@code null}.
     */
    public static <T> List<T> unmodifiableList(List<T> src) {
        return (src == null)
            ? Collections.<T>emptyList()
            : Collections.unmodifiableList(src);
    }

    /**
     * Return an unmodifiable set equivalent to the given set.
     *
     * @param src  The source set.
     * @param <T>  The type of elements in the set.
     * @return  An unmodifiable set. An empty set is returned if
     *          {@code src} is {@code null}.
     */
    public static <T> Set<T> unmodifiableSet(Set<T> src) {
        return (src == null)
            ? Collections.<T>emptySet()
            : Collections.unmodifiableSet(src);
    }

    /**
     * Return an unmodifiable set equivalent to the key set in the given map.
     *
     * @param src  The source map.
     * @param <T>  The type of keys in the map.
     * @return  An unmodifiable set. An empty set is returned if
     *          {@code src} is {@code null}.
     */
    public static <T> Set<T> unmodifiableKeySet(Map<T, ?> src) {
        return (src == null)
            ? Collections.<T>emptySet()
            : Collections.unmodifiableSet(src.keySet());
    }
}
