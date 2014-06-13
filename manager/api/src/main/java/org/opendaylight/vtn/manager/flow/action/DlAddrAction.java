/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.action;

import java.util.Arrays;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.controller.sal.utils.HexEncode;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * This class describes a flow action that takes a data layer address
 * as argument.
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"address": "00:11:22:33:44:55"
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "setdladdr")
@XmlAccessorType(XmlAccessType.NONE)
public abstract class DlAddrAction extends FlowAction {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -20707425482843213L;

    /**
     * Status of JAXB binding validation.
     *
     * <p>
     *   This field does not affect object identity.
     * </p>
     */
    private Status  validationStatus;

    /**
     * A data layer address.
     */
    private byte[]  address;

    /**
     * Dummy constructor only for JAXB and sub classes.
     */
    DlAddrAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param addr  A byte array which represents a data layer address.
     */
    DlAddrAction(byte[] addr) {
        if (addr != null) {
            address = addr.clone();
        }
    }

    /**
     * Return a data layer address configured in this instance.
     *
     * @return  A byte array which represents a data layer address.
     *          {@code null} is returned if no address is configured.
     */
    public final byte[] getAddress() {
        return (address == null) ? null : address.clone();
    }

    /**
     * Return the result of validation.
     *
     * <p>
     *   If this instance is created from XML data, it should be validated
     *   by this method.
     * </p>
     *
     * @return  A {@link Status} which indicates the result of validation.
     *          If this instance keeps invalid contents, a {@link Status}
     *          instance which keeps an error status is returned.
     */
    public final Status getValidationStatus() {
        return validationStatus;
    }

    /**
     * Return a string representation of a data layer address configured
     * in this instance.
     *
     * <p>
     *   {@code null} is returned if no address is configured in this instance.
     * </p>
     * <p>
     *   Note that below description of return value of this method is
     *   written for REST API document.
     * </p>
     *
     * @return
     *   A string representation of the MAC address.
     *   <ul>
     *     <li>
     *       A MAC address is represented by hexadecimal notation with
     *       {@code ':'} inserted between octets.
     *       (e.g. {@code "11:22:33:aa:bb:cc"})
     *     </li>
     *     <li>This attribute is mandatory.</li>
     *   </ul>
     * @deprecated  Only for JAXB.
     *              Use {@link #getAddress()} instead.
     */
    @XmlAttribute(name = "address", required = true)
    public final String getMacAddress() {
        return (address == null) ? null : HexEncode.bytesToHexString(address);
    }

    /**
     * Set a data layer address.
     *
     * <p>
     *   This method is called by JAXB.
     * </p>
     *
     * @param addr  A string representation of a data layer address.
     */
    @SuppressWarnings("unused")
    private void setMacAddress(String addr) {
        if (addr != null) {
            try {
                address = HexEncode.bytesFromHexString(addr);
            } catch (Exception e) {
                StringBuilder builder = new StringBuilder("Invalid address: ");
                builder.append(addr);
                validationStatus =
                    new Status(StatusCode.BADREQUEST, builder.toString());
            }
        }
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public final boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (o == null || !getClass().equals(o.getClass())) {
            return false;
        }

        DlAddrAction dladdr = (DlAddrAction)o;
        return Arrays.equals(address, dladdr.address);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public final int hashCode() {
        return getClass().hashCode() ^ Arrays.hashCode(address);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public final String toString() {
        StringBuilder builder = new StringBuilder(getClass().getSimpleName());
        builder.append('[');
        if (address != null) {
            builder.append("addr=").
                append(HexEncode.bytesToHexString(address));
        }
        builder.append(']');

        return builder.toString();
    }
}
