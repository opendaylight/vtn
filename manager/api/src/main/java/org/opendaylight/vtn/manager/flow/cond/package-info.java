/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
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
package org.opendaylight.vtn.manager.flow.cond;
