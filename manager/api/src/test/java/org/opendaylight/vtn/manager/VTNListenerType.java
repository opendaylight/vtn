/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

/**
 * A symbol which indicates events defined by
 * {@link org.opendaylight.vtn.manager.IVTNManagerAware}.
 */
public enum VTNListenerType {
    /**
     * A symbol which indicates VTN event.
     */
    VTN,

    /**
     * A symbol which indicates vBridge event.
     */
    VBRIDGE,

    /**
     * A symbol which indicates vBridge interface event.
     */
    VBRIDGE_IF,

    /**
     * A symbol which indicates vTerminal event.
     */
    VTERMINAL,

    /**
     * A symbol which indicates vTerminal interface event.
     */
    VTERMINAL_IF,

    /**
     * A symbol which indicates VLAN mapping event.
     */
    VLANMAP,

    /**
     * A symbol which indicates port mapping event for bridge interface.
     */
    PORTMAP,

    /**
     * A symbol which indicates port mapping event for vTerminal.
     */
    PORTMAP_VTERM,

    /**
     * A symbol which indicates MAC mapping event.
     */
    MACMAP;
}
