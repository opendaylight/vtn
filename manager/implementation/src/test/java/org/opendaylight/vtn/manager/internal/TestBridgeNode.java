/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.internal.cluster.VBridgeNode;

/**
 * An implementation of {@link VBridgeNode} only for testing.
 */
public class TestBridgeNode implements VBridgeNode {
    /**
     * A {@link VBridgePath} instance.
     */
    private VBridgePath  bridgePath;

    /**
     * Determine whether this node is enabled or not.
     */
    private boolean  enabled = true;

    /**
     * Construct an empty instance.
     */
    public TestBridgeNode() {
        this(null);
    }

    /**
     * Construct a new instance.
     *
     * @param path  A value to be returned by {@link #getPath()}.
     */
    public TestBridgeNode(VBridgePath path) {
        bridgePath = path;
    }

    /**
     * Set value to be returned by {@link #getPath()}.
     *
     * @param path  A value to be returned by {@link #getPath()}.
     */
    public void setPath(VBridgePath path) {
        bridgePath = path;
    }

    /**
     * Set value to be returned by {@link #isEnabled()}.
     *
     * @param en  A value to be returned by {@link #isEnabled()}.
     */
    public void setEnabled(boolean en) {
        enabled = en;
    }

    // VBridgeNode

    /**
     * Return path to this node.
     *
     * @return  Path to the node.
     */
    @Override
    public VBridgePath getPath() {
        return bridgePath;
    }

    /**
     * Determine whether this node is enabled or not.
     *
     * @return  {@code true} is returned only if this node is enabled.
     */
    @Override
    public boolean isEnabled() {
        return enabled;
    }
}
