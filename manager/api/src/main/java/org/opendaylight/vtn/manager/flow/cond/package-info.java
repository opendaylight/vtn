/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * This package provides public APIs for flow condition, which tests whether
 * the packet satisfies the specified conditions.
 *
 * <p>
 *   A flow condition is a named list of flow match conditions, and it is
 *   used to select packets. Each flow match condition must have a match index,
 *   which is an unique index in a flow condition. When a flow condition
 *   tests a packet, flow match conditions in a flow condition are evaluated
 *   in ascending order of match indices. A packet is selected if at least one
 *   flow match condition matches the packet.
 * </p>
 * <p>
 *   Flow conditions configured in the container is shared with all VTNs
 *   in the container.
 * </p>
 *
 * @since  Helium
 */
@XmlJavaTypeAdapters({
    @XmlJavaTypeAdapter(value = ByteAdapter.class, type = Byte.class),
    @XmlJavaTypeAdapter(value = ByteAdapter.class, type = byte.class),
    @XmlJavaTypeAdapter(value = ShortAdapter.class, type = Short.class),
    @XmlJavaTypeAdapter(value = ShortAdapter.class, type = short.class),
    @XmlJavaTypeAdapter(value = IntegerAdapter.class, type = Integer.class),
    @XmlJavaTypeAdapter(value = IntegerAdapter.class, type = int.class),
    @XmlJavaTypeAdapter(value = LongAdapter.class, type = Long.class),
    @XmlJavaTypeAdapter(value = LongAdapter.class, type = long.class),
    @XmlJavaTypeAdapter(value = DoubleAdapter.class, type = double.class),
    @XmlJavaTypeAdapter(value = DoubleAdapter.class, type = Double.class),
})
package org.opendaylight.vtn.manager.flow.cond;

import javax.xml.bind.annotation.adapters.XmlJavaTypeAdapter;
import javax.xml.bind.annotation.adapters.XmlJavaTypeAdapters;

import org.opendaylight.vtn.manager.util.xml.adapters.ByteAdapter;
import org.opendaylight.vtn.manager.util.xml.adapters.DoubleAdapter;
import org.opendaylight.vtn.manager.util.xml.adapters.IntegerAdapter;
import org.opendaylight.vtn.manager.util.xml.adapters.LongAdapter;
import org.opendaylight.vtn.manager.util.xml.adapters.ShortAdapter;
