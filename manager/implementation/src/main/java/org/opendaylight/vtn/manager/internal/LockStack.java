/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.Deque;
import java.util.LinkedList;
import java.util.concurrent.locks.Lock;

/**
 * A stack to hold acquired locks.
 *
 * <p>
 *   Note that this class is designed to be used by a single thread.
 * </p>
 */
public class LockStack {
    /**
     * A stack of {@link Lock} instances that represents locks acquired by
     * the calling thread.
     */
    private final Deque<Lock>  stack = new LinkedList<Lock>();

    /**
     * Acquire the given lock, and push it to the stack.
     *
     * @param lock  A lock to be acquired.
     */
    public void push(Lock lock) {
        lock.lock();

        boolean succeeded = false;
        try {
            stack.addFirst(lock);
            succeeded = true;
        } finally {
            if (!succeeded) {
                lock.unlock();
            }
        }
    }

    /**
     * Pop a lock from the stack, and release it.
     */
    public void pop() {
        Lock lock = stack.pollFirst();
        if (lock != null) {
            lock.unlock();
        }
    }

    /**
     * Release all locks and clear the stack.
     */
    public void clear() {
        for (Lock lock = stack.pollFirst(); lock != null;
             lock = stack.pollFirst()) {
            lock.unlock();
        }
    }
}
