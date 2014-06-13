/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.action;

import java.net.InetAddress;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * This class describes a flow action that takes an IP address as argument.
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"address": "192.168.10.1"
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "setinetaddr")
@XmlAccessorType(XmlAccessType.NONE)
public abstract class InetAddressAction extends FlowAction {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -8930109178154173026L;

    /**
     * Status of JAXB binding validation.
     *
     * <p>
     *   This field does not affect object identity.
     * </p>
     */
    private Status  validationStatus;

    /**
     * An IP address.
     */
    private InetAddress  address;

    /**
     * Dummy constructor only for JAXB and sub classes.
     */
    InetAddressAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param addr  An {@link InetAddress} instance which representse an
     *              IP address.
     * @throws IllegalArgumentException
     *    An invalid IP address is specified to {@code addr}.
     */
    InetAddressAction(InetAddress addr) {
        if (addr != null) {
            Class<?> cls = getAddressClass();
            if (!cls.equals(addr.getClass())) {
                String msg = unexpectedAddress(addr.toString(), cls);
                throw new IllegalArgumentException(msg);
            }
            address = addr;
        }
    }

    /**
     * Return an IP address configured in this instance.
     *
     * @return  An {@link InetAddress} instance which represents an IP address.
     *          {@code null} is returned if no address is configured.
     */
    public final InetAddress getAddress() {
        return address;
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
     * Return a string representation of an IP address configured in this
     * instance.
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
     *   A string representation of the IP address.
     *   <ul>
     *     <li>
     *       Current version supports IPv4 only.
     *       So a string representation of an IPv4 address must be specified.
     *     </li>
     *     <li>This attribute is mandatory.</li>
     *   </ul>
     * @deprecated  Only for JAXB.
     *              Use {@link #getAddress()} instead.
     */
    @XmlAttribute(name = "address", required = true)
    public final String getInetAddress() {
        return (address == null) ? null : address.getHostAddress();
    }

    /**
     * Set an IP address.
     *
     * <p>
     *   This method is called by JAXB.
     * </p>
     *
     * @param addr  A string representation of an IP address.
     */
    @SuppressWarnings("unused")
    private void setInetAddress(String addr) {
        if (addr != null) {
            try {
                InetAddress iaddr = InetAddress.getByName(addr);
                Class<?> cls = getAddressClass();
                if (!cls.equals(iaddr.getClass())) {
                    validationStatus =
                        new Status(StatusCode.BADREQUEST,
                                   unexpectedAddress(addr, cls));
                }
            } catch (Exception e) {
                StringBuilder builder = new StringBuilder("Invalid address: ");
                builder.append(addr);
                validationStatus =
                    new Status(StatusCode.BADREQUEST, builder.toString());
            }
        }
    }

    /**
     * Create a string which indicates unexpected IP address is specified.
     *
     * @param addr  A string representation of the IP address.
     * @param cls   A {@link Class} instance associated with the IP address
     *              class.
     * @return  A string which indicates unexpected IP address is specified.
     */
    private String unexpectedAddress(String addr, Class<?> cls) {
        StringBuilder builder =
            new StringBuilder("Unexpected address type: addr=");
        builder.append(addr).append(", expected=").append(cls.getSimpleName());
        return builder.toString();
    }

    /**
     * Return a {@link Class} instance which represents the type of
     * IP address.
     *
     * @return  A {@link Class} instance which represents the type of
     *          IP address.
     */
    public abstract Class<? extends InetAddress> getAddressClass();

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

        InetAddressAction iaddr = (InetAddressAction)o;
        if (address == null) {
            return (iaddr.address == null);
        }

        return address.equals(iaddr.address);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public final int hashCode() {
        int h = getClass().hashCode();
        if (address != null) {
            h ^= address.hashCode();
        }

        return h;
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
            builder.append("addr=").append(address.getHostAddress());
        }
        builder.append(']');

        return builder.toString();
    }
}
