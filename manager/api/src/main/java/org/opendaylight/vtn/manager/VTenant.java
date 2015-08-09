/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.vtn.manager.util.VTNIdentifiable;

/**
 * {@code VTenant} class describes the VTN (virtual tenant network)
 * information.
 *
 * <p>
 *   {@code VTenant} class inherits {@link VTenantConfig} class, and
 *   {@code VTenant} object contains the VTN configuration information.
 *   The VTN Manager passes the VTN information to other components by
 *   passing {@code VTenant} object.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"name": "vtn_1",
 * &nbsp;&nbsp;"description": "Description about VTN 1",
 * &nbsp;&nbsp;"idleTimeout": "300",
 * &nbsp;&nbsp;"hardTimeout": "0"
 * }</pre>
 *
 * @see  <a href="package-summary.html#VTN">VTN</a>
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "vtn")
@XmlAccessorType(XmlAccessType.NONE)
public class VTenant extends VTenantConfig implements VTNIdentifiable<String> {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 1029988750868364500L;

    /**
     * The name of the VTN.
     */
    @XmlAttribute(required = true)
    private String  name;

    /**
     * Private constructor used for JAXB mapping.
     */
    @SuppressWarnings("unused")
    private VTenant() {
        super(null);
    }

    /**
     * Construct a new object which represents the
     * {@linkplain <a href="package-summary.html#VTN">VTN</a>} information.
     *
     * @param tenantName  The name of the VTN.
     * @param tconf       A {@link VTenantConfig} object which contains the
     *                    configuration information about the VTN.
     *                    All values in {@code tconf} get copied to a new
     *                    object.
     * @throws NullPointerException
     *    {@code tcode} is {@code null}.
     */
    public VTenant(String tenantName, VTenantConfig tconf) {
        super(tconf.getDescription(), tconf.getIdleTimeout(),
              tconf.getHardTimeout());
        name = tenantName;
    }

    /**
     * Return the name of the
     * {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
     *
     * @return  The name of the VTN.
     */
    public String getName() {
        return name;
    }

    // Object

    /**
     * Determine whether the given object is identical to this object.
     *
     * <p>
     *   {@code true} is returned only if all the following conditions are met.
     * </p>
     * <ul>
     *   <li>
     *     {@code o} is a {@code VTenantConfig} object.
     *   </li>
     *   <li>
     *     The following values stored in {@code o} are the same as in this
     *     object.
     *     <ul>
     *       <li>
     *         The name of the
     *         {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
     *       </li>
     *       <li>The description of the VTN.</li>
     *       <li>
     *         The number of seconds for {@code idle_timeout} of flow entry.
     *       </li>
     *       <li>
     *         The number of seconds for {@code hard_timeout} of flow entry.
     *       </li>
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
        if (!(o instanceof VTenant) || !super.equals(o)) {
            return false;
        }

        VTenant vtenant = (VTenant)o;
        if (name == null) {
            return (vtenant.name == null);
        }

        return name.equals(vtenant.name);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = super.hashCode();
        if (name != null) {
            h ^= name.hashCode();
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
        StringBuilder builder = new StringBuilder("VTenant[");
        String prefix;
        if (name != null) {
            builder.append("name=").append(name);
            prefix = ",";
        } else {
            prefix = "";
        }

        appendContents(builder, prefix).append(']');
        return builder.toString();
    }

    // VTNIdentifiable

    /**
     * Return the identifier of this instance.
     *
     * <p>
     *   This class uses the name of the VTN as the identifier.
     * </p>
     *
     * @return  The name of the VTN.
     * @since   Lithium
     */
    @Override
    public String getIdentifier() {
        return name;
    }
}
