/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import java.util.UUID;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Base utility functions used by neutron handlers.
 */
public final class VTNNeutronUtils {
    /**
     * Logger instance.
     */
    private static final Logger LOG =
        LoggerFactory.getLogger(VTNNeutronUtils.class);

    /**
     * UUID version number position.
     */
    private static final int UUID_VERSION_POS = 12;

    /**
     * UUID time-low field byte length in hex.
     */
    private static final int UUID_TIME_LOW = 8;

    /**
     * UUID time-mid field byte length in hex.
     */
    private static final int UUID_TIME_MID = 4;

    /**
     * UUID time-high and version field byte length in hex.
     */
    private static final int UUID_TIME_HIGH_VERSION = 4;

    /**
     * UUID clock sequence field byte length in hex.
     */
    private static final int UUID_CLOCK_SEQ = 4;

    /**
     * UUID node field byte length in hex.
     */
    private static final int UUID_NODE = 12;

    /**
     * UUID time field byte length in hex.
     */
    private static final int UUID_TIME_LEN = (UUID_TIME_LOW +
            UUID_TIME_MID + UUID_TIME_HIGH_VERSION);

    /**
     * Neutron UUID identifier length.
     */
    private static final int UUID_LEN = 36;

    /**
     * Tenant id length when keystone identifier is used in neutron.
     */
    private static final int KEYSTONE_ID_LEN = 32;

    /**
     * Private constructor that protects this class from instantiating.
     */
    private VTNNeutronUtils() {}

    /**
     * Convert UUID to VTN key syntax.
     *
     * @param id neutron object UUID.
     * @return key in compliance to VTN object key.
     */
    public static String convertUUIDToKey(String id) {

        String key = null;
        if (id == null) {
            return key;
        }
        LOG.trace("id - {}, length - {}", id, id.length());
        /**
         * VTN ID must be less than 32 bytes,
         * Shorten UUID string length from 36 to 31 as follows:
         * delete UUID Version and hyphen (see RFC4122) field in the UUID
         */
        try {
            StringBuilder tKey = new StringBuilder();
            // remove all the '-'
            for (String retkey: id.split("-")) {
                tKey.append(retkey);
            }
            // remove the version byte
            tKey.deleteCharAt(UUID_VERSION_POS);
            key = tKey.toString();
        } catch (IllegalArgumentException ile) {
            LOG.error("Invalid UUID - {}", id);
            key = null;
        }
        return key;
    }

    /**
     * Convert neutron object id to VTN key syntax.
     *
     * @param neutronID neutron object id.
     * @return key in compliance to VTN object key.
     */
    public static String convertNeutronIDToVTNKey(String neutronID) {
        String key = null;
        if (neutronID == null) {
            return key;
        }

        LOG.trace("neutronID - {}, length - {}",
                  neutronID, neutronID.length());
        if (!isValidNeutronID(neutronID)) {
            return key;
        }

        if (neutronID.length() == UUID_LEN) {
            key = convertUUIDToKey(neutronID);
        } else if (neutronID.length() == KEYSTONE_ID_LEN) {
            key = convertKeystoneIDToVTNKey(neutronID);
        } else {
            key = neutronID;
        }
        return key;
    }

     /**
     * Verify the validity of neutron object identifiers.
     *
     * @param id neutron object id.
     * @return {@code true} neutron identifier is valid.
     *         {@code false} neutron identifier is invalid.
     */
    public static boolean isValidNeutronID(String id) {
        if (id == null) {
            return false;
        }
        boolean isValid = false;
        LOG.trace("id - {}, length - {}", id, id.length());
        /**
         * check the string length
         * if length is 36 its a uuid do uuid validation
         * if length is 32 it can be tenant id form keystone
         * if its less than 32  can be valid VTN ID
         */
        if (id.length() == UUID_LEN) {
            try {
                UUID fromUUID = UUID.fromString(id);
                String toUUID = fromUUID.toString();
                isValid = toUUID.equalsIgnoreCase(id);
            } catch (IllegalArgumentException e) {
                LOG.error("IllegalArgumentExecption for id - {}", id);
                isValid = false;
            }
        } else if ((id.length() > 0) && (id.length() <= KEYSTONE_ID_LEN)) {
            isValid = true;
        } else {
            isValid = false;
        }
        return isValid;
    }

    /**
     * Convert string id to VTN key syntax.
     *
     * @param id neutron object id.
     * @return key in compliance to VTN object key.
     */
    private static String convertKeystoneIDToVTNKey(String id) {
        String key = null;
        if (id == null) {
            return key;
        }

        /**
         * tenant ID if given from openstack keystone does not follow the
         * generic UUID syntax, convert the ID to UUID format for validation
         * and reconvert it to VTN key
         */

        LOG.trace("id - {}, length - {}", id, id.length());
        try {
            StringBuilder tKey = new StringBuilder();
            String tmpStr = id.substring(0, UUID_TIME_LOW);
            tKey.append(tmpStr);
            tKey.append("-");
            tmpStr = id.substring(UUID_TIME_LOW,
                    (UUID_TIME_LOW + UUID_TIME_MID));
            tKey.append(tmpStr);
            tKey.append("-");
            tmpStr = id.substring((UUID_TIME_LOW + UUID_TIME_MID),
                    UUID_TIME_LEN);
            tKey.append(tmpStr);
            tKey.append("-");
            tmpStr = id.substring(UUID_TIME_LEN,
                    (UUID_TIME_LEN + UUID_CLOCK_SEQ));
            tKey.append(tmpStr);
            tKey.append("-");
            tmpStr = id.substring((UUID_TIME_LEN + UUID_CLOCK_SEQ),
                    (UUID_TIME_LEN + UUID_CLOCK_SEQ + UUID_NODE));
            tKey.append(tmpStr);

            tmpStr = tKey.toString();
            UUID fromUUID = UUID.fromString(tmpStr);
            String toUUID = fromUUID.toString();
            if (toUUID.equalsIgnoreCase(tmpStr)) {
                key = convertUUIDToKey(tmpStr);
            }
        } catch (IndexOutOfBoundsException ibe) {
            LOG.error("Execption! Invalid UUID - {}", id);
            key = null;
        } catch (IllegalArgumentException iae) {
            LOG.error("Execption! Invalid object ID - {}", id);
            key = null;
        }
        return key;
    }

}
