/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.filter;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

/**
 * {@code PassFilter} class describes the PASS flow filter which lets the
 * specified packets through the virtual node in the VTN.
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">{}</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "pass")
@XmlAccessorType(XmlAccessType.NONE)
public final class PassFilter extends FilterType {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 1295310703141555376L;

    /**
     * Construct a new instance.
     */
    public PassFilter() {
    }
}
