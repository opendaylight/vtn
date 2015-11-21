/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

/**
 * {@code DataChangeWaiter} defines the interface to detect changes made to
 * the MD-SAL datastore.
 *
 * @param <T>  Type of the target object.
 */
public interface DataChangeWaiter<T extends DataObject> extends AutoCloseable {
    /**
     * Return the path to the target object.
     *
     * @return  The path to the target object.
     */
    InstanceIdentifier<T> getPath();

    /**
     * Return the latest data object notified by the data change listener.
     *
     * @return  The data object that keeps the latest value.
     *          {@code null} if the data object is not present.
     */
    T getValue();

    /**
     * Wait for the target data object to be updated.
     *
     * @param limit  The system time in milliseconds which specifies the limit
     *               of the wait.
     * @return  {@code true} if the target data has been updated.
     *          {@code false} if the system time passes the specified limit
     *          before the target data is updated.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    boolean await(long limit) throws InterruptedException;

    // AutoCloseable

    /**
     * Close this instance.
     */
    @Override
    void close();
}
