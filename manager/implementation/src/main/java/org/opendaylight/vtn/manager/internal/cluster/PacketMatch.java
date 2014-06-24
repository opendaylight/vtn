/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.io.Serializable;

import org.opendaylight.vtn.manager.internal.PacketContext;

/**
 * {@code PacketMatch} defines interfaces to be implemented by classes
 * which tests protocol header files in packet.
 *
 * <p>
 *   Although this interface is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public interface PacketMatch extends Serializable {
    /**
     * Determine whether the specified packet matches the condition defined
     * by this instance.
     *
     * @param pctx  The context of the packet to be tested.
     * @return  {@code true} if the specified packet matches the condition.
     *          Otherwise {@code false}.
     */
    boolean match(PacketContext pctx);
}
