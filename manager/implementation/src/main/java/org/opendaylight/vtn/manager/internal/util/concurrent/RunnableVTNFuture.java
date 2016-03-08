/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.concurrent;

/**
 * A {@link VTNFuture} that implements {@link Runnable}.
 *
 * @param <T>  The type of the object to be returned.
 */
public abstract class RunnableVTNFuture<T> extends SettableVTNFuture<T>
    implements Runnable {
}
