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
 * {@code DropFilter} class describes the DROP flow filter which discards
 * the specified packets.
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">{}</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "drop")
@XmlAccessorType(XmlAccessType.NONE)
public final class DropFilter extends FilterType {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 7396754685166202723L;

    /**
     * Construct a new instance.
     */
    public DropFilter() {
    }
}
