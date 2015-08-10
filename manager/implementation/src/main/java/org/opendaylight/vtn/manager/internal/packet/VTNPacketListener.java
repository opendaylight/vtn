/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.packet;

import org.opendaylight.vtn.manager.VTNException;

/**
 * Interface for classes that listens packets sent by switches.
 */
public interface VTNPacketListener {
    /**
     * Invoked when a packet has been received.
     *
     * @param ev  A {@link PacketInEvent} instance.
     * @throws VTNException  An error occurred.
     */
    void notifyPacket(PacketInEvent ev) throws VTNException;
}
