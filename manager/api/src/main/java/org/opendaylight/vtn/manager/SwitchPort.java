/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.io.Serializable;
import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.controller.sal.core.NodeConnector;

/**
 * {@code SwitchPort} class describes the location of port of physical switch.
 *
 * <p>
 *   This class is used to specify the physical switch port to be mapped during
 *   configuration of port mapping.
 * </p>
 * <p>
 *   {@code SwitchPort} class maintains only the information that identifies
 *   the port within a physical switch, and does not maintain the information
 *   that identifies the physical switch. A port of physical switch to be
 *   mapped by port mapping is specified by the combination of
 *   {@link org.opendaylight.controller.sal.core.Node} object
 *   (<strong>node</strong> element for REST API), which identifies
 *   the physical switch managed by the OpenDaylight controller, and
 *   {@code SwitchPort} object.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;"id": "1",
 * &nbsp;&nbsp;"name": "s1-eth1"
 * }</pre>
 *
 * @see  <a href="package-summary.html#port-map">Port mapping</a>
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "switchport")
@XmlAccessorType(XmlAccessType.NONE)
public class SwitchPort implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -6749678358471810294L;

    /**
     * The name of the physical switch port.
     *
     * <ul>
     *   <li>
     *     It is necessary to specify a character string with 1 or more
     *     characters.
     *   </li>
     *   <li>
     *     If this attribute is omitted, then it is necessary to specify
     *     both <strong>type</strong> and <strong>id</strong> attributes.
     *   </li>
     * </ul>
     */
    @XmlAttribute
    private String  name;

    /**
     * Type of the node connector corresponding to the physical switch port.
     *
     * <ul>
     *   <li>
     *     Specify <strong>"OF"</strong> for physical port of OpenFlow switch.
     *   </li>
     *   <li>
     *     It should be specified along with the attribute <strong>id</strong>.
     *   </li>
     *   <li>
     *     If this attribute is omitted, then it is necessary to specify the
     *     attribute <strong>name</strong>.
     *   </li>
     * </ul>
     */
    @XmlAttribute
    private String  type;

    /**
     * A string which represents identifier of the node connector corresponding
     * to the physical switch port.
     *
     * <ul>
     *   <li>
     *     Specify a string representation of the port number for physical port
     *     of OpenFlow switch.
     *   </li>
     *   <li>
     *     It should be specified along with the attribute
     *     <strong>type</strong>.
     *   </li>
     *   <li>
     *     If this attribute is omitted, then it is necessary to specify the
     *     attribute <strong>name</strong>.
     *   </li>
     * </ul>
     */
    @XmlAttribute
    private String  id;

    /**
     * Private constructor used for JAXB mapping.
     */
    @SuppressWarnings("unused")
    private SwitchPort() {
    }

    /**
     * Construct a new {@code SwitchPort} object which specifies the physical
     * switch port by its name.
     *
     * @param name
     *   The name of the physical switch port.
     *   <ul>
     *     <li>
     *       A string containing 1 or more characters must be specified.
     *     </li>
     *     <li>
     *       Exception will not occur even if {@code null} or an empty string
     *       is specified, but there will be error if you specify such
     *       {@code SwitchPort} object in API of {@link IVTNManager} service.
     *     </li>
     *   </ul>
     */
    public SwitchPort(String name) {
        this.name = name;
    }

    /**
     * Construct a new {@code SwitchPort} object which specifies the physical
     * switch port by type and identifier of the node connector.
     *
     * <p>
     *   Exception will not occur even if {@code null} or an empty string is
     *   specified to {@code type} or {@code id}, but there will be error
     *   if you specify such {@code SwitchPort} object in API of
     *   {@link IVTNManager} service.
     * </p>
     *
     * @param type  The type of the node connector.
     *              Specify <strong>"OF"</strong> for physical port of OpenFlow
     *              switch.
     * @param id    The identifier of the node connector.
     *              Specify a string representation of the port number for
     *              physical port of OpenFlow switch.
     */
    public SwitchPort(String type, String id) {
        this.type = type;
        this.id = id;
    }

    /**
     * Construct a new {@code SwitchPort} object which specifies the physical
     * switch port by its name, and a pair of type and identifier of the node
     * connector.
     *
     * <p>
     *   {@linkplain <a href="package-summary.html#port-map">Port mapping</a>}
     *   specified by an object created by this constructor is enabled only
     *   when the physical switch port specified by {@code type} and {@code id}
     *   exists and when the name of that port matches with the string
     *   specified by {@code name}.
     * </p>
     * <p>
     *   Specifying {@code null} to argument is treated as the conditions
     *   for that argument were not specified. However, if following conditions
     *   are not met, there will be error if you specify such
     *   {@code SwitchPort} object in API of {@link IVTNManager} service.
     * </p>
     * <ul>
     *   <li>
     *     If {@code null} is specified in {@code name}, then you need to
     *     specify value other than {@code null} in both {@code type} and
     *     {@code id} so that the node connector can be identified.
     *   </li>
     *   <li>
     *     If {@code null} is specified in both {@code type} and {@code id},
     *     then you need to specify a string containing 1 or more characters
     *     in {@code name} so that the name of the physical switch port can
     *     be identified.
     *   </li>
     *   <li>
     *     If value other than {@code null} is specified in {@code type},
     *     then you need to specify value other than {@code null} to
     *     {@code id}, and vice versa.
     *   </li>
     * </ul>
     *
     * @param name  The name of the physical switch port.
     * @param type  The type of the node connector.
     *              Specify <strong>"OF"</strong> for physical port of OpenFlow
     *              switch.
     * @param id    The identifier of the node connector.
     *              Specify a string representation of the port number for
     *              physical port of OpenFlow switch.
     */
    public SwitchPort(String name, String type, String id) {
        this.name = name;
        this.type = type;
        this.id = id;
    }

    /**
     * Construct a new {@code SwitchPort} instance which represents the
     * location of the specified switch port.
     *
     * @param nc    A {@link NodeConnector} instance corresponding to the
     *              switch port.
     * @param name  The name of the switch port specified by {@code nc}.
     *              {@code null} means that the port name is unavailable.
     * @throws NullPointerException
     *    {@code nc} is {@code null}.
     * @since  Helium
     */
    public SwitchPort(NodeConnector nc, String name) {
        this.name = name;
        this.type = nc.getType();
        this.id = nc.getNodeConnectorIDString();
    }

    /**
     * Return the name of the switch port.
     *
     * @return  The name of the switch port.
     *          {@code null} is returned if name is not set.
     */
    public String getName() {
        return name;
    }

    /**
     * Return the type of the node connector.
     *
     * @return  The type of the node connector.
     *          {@code null} is returned if node connector type is not set.
     */
    public String getType() {
        return type;
    }

    /**
     * Return a string representation of the node connector ID.
     *
     * @return  The identifier of the node connector.
     *          {@code null} is returned if node connector ID is not set.
     */
    public String getId() {
        return id;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * <p>
     *   {@code true} is returned only if all the following conditions are met.
     * </p>
     * <ul>
     *   <li>
     *     {@code o} is a {@code SwitchPort} object.
     *   </li>
     *   <li>
     *     The following values stored in {@code o} are the same as in this
     *     object.
     *     <ul>
     *       <li>The name of the physical switch port.</li>
     *       <li>The type of the node connector.</li>
     *       <li>The identifier of the node connector.</li>
     *     </ul>
     *   </li>
     * </ul>
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (!(o instanceof SwitchPort)) {
            return false;
        }

        SwitchPort swport = (SwitchPort)o;
        return (Objects.equals(name, swport.name) &&
                Objects.equals(type, swport.type) &&
                Objects.equals(id, swport.id));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = 0;
        if (name != null) {
            h ^= name.hashCode();
        }
        if (type != null) {
            h ^= type.hashCode();
        }
        if (id != null) {
            h ^= id.hashCode();
        }

        return h;
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("SwitchPort[");
        char sep = 0;
        if (name != null) {
            builder.append("name=").append(name);
            sep = ',';
        }

        if (type != null) {
            if (sep != 0) {
                builder.append(sep);
            }
            builder.append("type=").append(type);
            sep = ',';
        }

        if (id != null) {
            if (sep != 0) {
                builder.append(sep);
            }
            builder.append("id=").append(id);
        }

        builder.append(']');

        return builder.toString();
    }
}
