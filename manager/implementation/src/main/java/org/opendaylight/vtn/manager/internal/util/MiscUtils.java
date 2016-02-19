/*
 * Copyright (c) 2014, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.EnumMap;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import java.util.Set;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.collect.ImmutableMap;
import com.google.common.collect.ImmutableSet;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnVlanIdField;

import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;

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
     * A map that keeps lower-cased name of {@link VtnUpdateType}.
     */
    private static final Map<VtnUpdateType, String>  UPDATE_TYPE_MAP;

    /**
     * The minimum value of the order value for the first element in the
     * ordered list.
     */
    public static final int  ORDER_MIN = 0;

    /**
     * Default VLAN ID.
     */
    public static final int  DEFAULT_VLAN_ID = 0;

    /**
     * A set of {@link VtnErrorTag} that indicate an error caused by a
     * bad request.
     */
    private static final Set<VtnErrorTag>  BAD_REQUEST_TAGS;

    /**
     * Initialize static fields.
     */
    static {
        Map<VtnUpdateType, String> types = new EnumMap<>(VtnUpdateType.class);
        for (VtnUpdateType type: VtnUpdateType.values()) {
            types.put(type, toLowerCase(type.name()));
        }
        UPDATE_TYPE_MAP = ImmutableMap.copyOf(types);

        Set<VtnErrorTag> set = EnumSet.of(
            VtnErrorTag.BADREQUEST, VtnErrorTag.NOTFOUND,
            VtnErrorTag.CONFLICT);
        BAD_REQUEST_TAGS = ImmutableSet.copyOf(set);
    }

    /**
     * Private constructor that protects this class from instantiating.
     */
    private MiscUtils() {}

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
            throw RpcException.getBadArgumentException(msg, e);
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
            throw RpcException.getNullArgumentException(desc + " name");
        }

        if (name.isEmpty()) {
            throw RpcException.getBadArgumentException(
                desc + " name cannot be empty");
        }

        try {
            return new VnodeName(name);
        } catch (RuntimeException e) {
            String msg = desc + " name is invalid";
            throw RpcException.getBadArgumentException(msg, e);
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
     * Ensure the given vnode-name is not null.
     *
     * @param desc   Brief description of the resource.
     * @param vname  A {@link VnodeName} instance.
     * @throws RpcException  {@code vname} is {@code null}.
     */
    public static void checkPresent(String desc, VnodeName vname)
        throws RpcException {
        if (vname == null) {
            throw RpcException.getNullArgumentException(desc + " name");
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
     * Convert all of the characters in the given {@link VtnUpdateType} name
     * to lower case.
     *
     * @param type  A {@link VtnUpdateType} instance.
     * @return  A converted string.
     */
    public static String toLowerCase(VtnUpdateType type) {
        return UPDATE_TYPE_MAP.get(type);
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
     * Determine whether the given two collections are identical or not.
     *
     * <p>
     *   This method compares the given collections as sets.
     *   Duplicate elements in the given collections are ignored.
     * </p>
     *
     * @param c1   The first collections to be compared.
     *             {@code null} is treated as an empty collection.
     * @param c2   The second collections to be compared.
     *             {@code null} is treated as an empty collection.
     * @param <T>  The type of elements in the given collections.
     * @return  {@code true} only if {@code c1} and {@code c2} are identical.
     */
    public static <T> boolean equalsAsSet(Collection<T> c1, Collection<T> c2) {
        Set<T> set1 = new HashSet<>();
        if (c1 != null) {
            set1.addAll(c1);
        }

        if (c2 != null) {
            Set<T> set2 = new HashSet<>();
            for (T o: c2) {
                if (!set1.remove(o) && !set2.contains(o)) {
                    return false;
                }
                set2.add(o);
            }
        }

        return set1.isEmpty();
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
     * Return a string configured in the given vnode-name.
     *
     * @param vname  An {@link VnodeName} instance.
     * @return  A string configured in the given URI.
     *          Note that {@code null} is returned if {@code vname} is
     *          {@code null}.
     */
    public static String getValue(VnodeName vname) {
        return (vname == null) ? null : vname.getValue();
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

    /**
     * Return a VLAN ID configured in the given {@link VtnVlanIdField}
     * instance.
     *
     * @param field  A {@link VtnVlanIdField} instance.
     * @return  A {@link VlanId} instance.
     * @throws RpcException
     *    {@code field} contains an invalid value.
     */
    public static VlanId getVlanId(VtnVlanIdField field) throws RpcException {
        VlanId vlanId = field.getVlanId();
        if (vlanId == null) {
            // Use default VLAN ID.
            vlanId = new VlanId(DEFAULT_VLAN_ID);
        } else {
            Integer vid = vlanId.getValue();
            if (vid == null) {
                // This should never happen.
                throw RpcException.getNullArgumentException("vlan-id");
            } else {
                ProtocolUtils.checkVlan(vid);
            }
        }

        return vlanId;
    }

    /**
     * Determine whether the given throwable is caused by a bad user request
     * or not.
     *
     * @param t  A throwable to be tested.
     * @return  {@code true} only if the given throwable is caused by a
     *          bad user request.
     */
    public static boolean isBadRequest(Throwable t) {
        boolean bad;
        if (t instanceof VTNException) {
            VTNException e = (VTNException)t;
            bad = BAD_REQUEST_TAGS.contains(e.getVtnErrorTag());
        } else {
            bad = false;
        }

        return bad;
    }

    /**
     * Convert values in the given map into a list.
     *
     * @param map  A map to be converted.
     * @param <T>  The type of values in the given map.
     * @return  A list of values in the given map if the given map contains
     *          at least one value.
     *          {@code null} if the given map is {@code null} or empty.
     */
    public static <T> List<T> toValueList(Map<?, T> map) {
        return (map == null || map.isEmpty())
            ? null
            : new ArrayList<>(map.values());
    }

    /**
     * Determine whether the given collection is empty or not.
     *
     * @param c  A collection to be tested.
     * @return  {@code true} only of the given collection is empty.
     */
    public static boolean isEmpty(Collection<?> c) {
        return (c == null || c.isEmpty());
    }

    /**
     * Convert a boolean value into a {@link VnodeState} instance.
     *
     * @param b  A boolean value.
     * @return  {@link VnodeState#UP} if {@code b} is {@code true}.
     *          {@link VnodeState#DOWN} if {@code b} is {@code false}.
     */
    public static VnodeState toVnodeState(boolean b) {
        return (b) ? VnodeState.UP : VnodeState.DOWN;
    }

    /**
     * Verify the given MAC address.
     *
     * @param mac  The MAC address to be tested.
     * @return  The given MAC address is returned.
     * @throws RpcException
     *    The given MAC address is invalid.
     */
    public static MacAddress verify(MacAddress mac) throws RpcException {
        try {
            EtherAddress eaddr = EtherAddress.create(mac);
            if (eaddr == null) {
                throw RpcException.getNullArgumentException("MAC address");
            }
        } catch (RuntimeException e) {
            throw RpcException.getBadArgumentException(
                "Invalid MAC address: " + mac, e);
        }

        return mac;
    }
}
