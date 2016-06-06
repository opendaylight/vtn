/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import java.util.Deque;
import java.util.LinkedList;

import javax.annotation.Nonnull;

import org.opendaylight.controller.md.sal.binding.api.DataObjectModification;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.PathArgument;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

/**
 * The context to be associated with a data tree change notification.
 *
 * @param <D>  The type of the root node.
 */
final class TreeChangeContext<D extends DataObject> {
    /**
     * The path to the root node.
     */
    private InstanceIdentifier<D>  rootPath;

    /**
     * A list of path arguments in the instance identifier that points to
     * the current tree node.
     */
    private Deque<PathArgument>  pathArguments;

    /**
     * The stack for scanning children in the root node.
     */
    private Deque<DataObjectModification<?>>  treeStack;

    /**
     * Construct a new instance.
     */
    TreeChangeContext() {}

    /**
     * Return the path of the root node.
     *
     * @return  The path of the root node.
     */
    InstanceIdentifier<D> getRootPath() {
        return rootPath;
    }

    /**
     * Set the path of the root node.
     *
     * @param root  The path of the root node.
     */
    void setRootPath(@Nonnull InstanceIdentifier<D> root) {
        rootPath = root;
        pathArguments = null;
        treeStack = null;
    }

    /**
     * Switch the tree node pointer to the specified data object.
     *
     * @param mod  The data object modification for the child of the
     *             current tree node.
     */
    void push(DataObjectModification<?> mod) {
        Deque<DataObjectModification<?>> stack = treeStack;
        if (stack == null) {
            stack = new LinkedList<>();
            treeStack = stack;
        }
        stack.addLast(mod);

        if (pathArguments != null) {
            pathArguments.addLast(mod.getIdentifier());
        }
    }

    /**
     * Switch the tree node pointer to the parent node.
     */
    void pop() {
        assert treeStack != null;
        treeStack.removeLast();
        if (pathArguments != null) {
            pathArguments.removeLast();
        }
    }

    /**
     * Return the instance identifier that points the tree node being
     * currently handled.
     *
     * @return  The instance identifier.
     */
    InstanceIdentifier<?> getPath() {
        InstanceIdentifier<?> path;
        if (MiscUtils.isEmpty(treeStack)) {
            // This instance points to the root node.
            path = rootPath;
        } else {
            Deque<PathArgument> args = pathArguments;
            if (args == null) {
                // Initialize the path argument list.
                args = new LinkedList<>();
                for (PathArgument arg: rootPath.getPathArguments()) {
                    args.addLast(arg);
                }
                for (DataObjectModification<?> mod: treeStack) {
                    args.addLast(mod.getIdentifier());
                }
                pathArguments = args;
            }

            path = InstanceIdentifier.create(args);
        }

        return path;
    }
}
