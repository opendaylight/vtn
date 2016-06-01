/*
 * Copyright (c) 2013, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import org.slf4j.Logger;

import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.attrs.rev150712.BaseAttributes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.networks.rev150712.networks.attributes.networks.Network;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.ports.rev150712.ports.attributes.ports.Port;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev130715.Uuid;

/**
 * Base utility functions used by neutron handlers.
 */
public final class VTNNeutronUtils {
    /**
     * A format string used to log a pair of message and argument.
     */
    static final String  FORMAT_MSG_ARG = "{}: {}";

    /**
     * UUID version number position.
     */
    private static final int UUID_VERSION_POS = 12;

    /**
     * Private constructor that protects this class from instantiating.
     */
    private VTNNeutronUtils() {}

    /**
     * Convert UUID to VTN key syntax.
     *
     * @param uuid  An {@link Uuid} instance.
     * @return  The converted key string.
     *          {@code null} if the given UUID is invalid.
     */
    public static String convertUUIDToKey(Uuid uuid) {
        String key = null;
        if (uuid != null) {
            String id = uuid.getValue();
            if (id != null) {
                // Delete UUID Version and hyphen (see RFC4122) field in
                // the UUID.
                int len = id.length();
                int pos = 0;
                StringBuilder builder = new StringBuilder();
                for (int i = 0; i < len; i++) {
                    char c = id.charAt(i);
                    // Remove all the hyphens.
                    if (c != '-') {
                        // Remove the version byte.
                        if (pos != UUID_VERSION_POS) {
                            builder.append(c);
                        }
                        pos++;
                    }
                }
                key = builder.toString();
            }
        }

        return key;
    }

    /**
     * Return the virtual tenant ID configured in the specified by the
     * neutron object.
     *
     * @param obj  A {@link BaseAttributes} instance.
     * @return  The virtual tenant ID or {@code null}.
     */
    public static String getTenantId(BaseAttributes obj) {
        return convertUUIDToKey(obj.getTenantId());
    }

    /**
     * Return the virtual bridge ID configured in the specified by the
     * neutron network.
     *
     * @param nw  A neutron network instance.
     * @return  The virtual bridge ID.
     */
    public static String getBridgeId(Network nw) {
        return convertUUIDToKey(nw.getUuid());
    }

    /**
     * Return the virtual bridge ID configured in the specified by the
     * neutron port.
     *
     * @param port  A neutron port instance.
     * @return  The virtual bridge ID.
     */
    public static String getBridgeId(Port port) {
        return convertUUIDToKey(port.getNetworkId());
    }

    /**
     * Return the virtual interface ID configured in the specified by the
     * neutron port.
     *
     * @param port  A neutron port instance.
     * @return  The virtual interface ID.
     */
    public static String getInterfaceId(Port port) {
        return convertUUIDToKey(port.getUuid());
    }

    /**
     * Ensure that the given value is not {@code null}.
     *
     * @param value  The value to be tested.
     * @param def    The default value.
     * @param <T>    The type of the value.
     * @return  {@code value} is returned if it is not {@code null}.
     *          {@code def} is returned if {@code value} is {@code null}.
     */
    public static <T> T getNonNullValue(T value, T def) {
        return (value == null) ? def : value;
    }

    /**
     * Return an UUID string in the given instance.
     *
     * @param uuid  An {@link Uuid} instance.
     * @return  An UUID string in the given instance.
     */
    public static String getUuid(Uuid uuid) {
        return (uuid == null) ? null : uuid.getValue();
    }

    /**
     * Record a log message about the given object.
     *
     * @param logger  A logger instance.
     * @param msg     A log message.
     * @param obj     A {@link BaseAttributes} instance to be logged.
     */
    public static void recordLog(Logger logger, String msg,
                                 BaseAttributes obj) {
        if (logger.isTraceEnabled()) {
            logger.trace(FORMAT_MSG_ARG, msg, obj);
        } else {
            // Record UUID only.
            logger.info(FORMAT_MSG_ARG, msg, obj.getUuid().getValue());
        }
    }

    /**
     * Record a log message that indicates the given object was changed.
     *
     * @param logger  A logger instance.
     * @param msg     A log message.
     * @param before  An object before modification.
     * @param after   An object after modification.
     */
    public static void recordLog(Logger logger, String msg,
                                 BaseAttributes before, BaseAttributes after) {
        if (logger.isTraceEnabled()) {
            logger.trace("{}: before={}, after={}", msg, before, after);
        } else {
            // Record UUID only.
            logger.info(FORMAT_MSG_ARG, msg, after.getUuid().getValue());
        }
    }
}
