/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode;

/**
 * {@code VNodeIdentifierFactory} describes an interface for factory class
 * of {@link VNodeIdentifier}.
 *
 * <p>
 *   This interface is used to instantiate {@link VNodeIdentifier} from a
 *   string representation of {@link VNodeIdentifier}.
 * </p>
 */
public interface VNodeIdentifierFactory {
    /**
     * Construct a new instance.
     *
     * @param comps  An array of strings which represents the path components
     *               of identifier. Note that the caller must guarantee that
     *               {@code comps} contains valid path components.
     * @return  A {@link VNodeIdentifier} instance.
     */
    VNodeIdentifier<?> newIdentifier(String[] comps);
}
