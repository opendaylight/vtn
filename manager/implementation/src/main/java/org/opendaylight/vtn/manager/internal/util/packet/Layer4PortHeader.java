/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.packet;

/**
 * {@code Layer4PortHeader} describes the contents of layer 4 protocol header
 * which identifies the service using 16-bit port number.
 */
public interface Layer4PortHeader extends Layer4Header {
    /**
     * Return the source port number.
     *
     * @return  An integer value which represents the source port number.
     */
    int getSourcePort();

    /**
     * Set the source port number.
     *
     * @param port  An integer value which represents the source port number.
     */
    void setSourcePort(int port);

    /**
     * Return the destination port number.
     *
     * @return  An integer value which represents the destination port number.
     */
    int getDestinationPort();

    /**
     * Set the destination port number.
     *
     * @param port  An integer value which represents the destination port
     *              number.
     */
    void setDestinationPort(int port);
}
