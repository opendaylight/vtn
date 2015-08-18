/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import org.opendaylight.vtn.manager.VNodePath;

/**
 * {@code VTenantNode} class describes virtual node inside the VTN.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public abstract class VTenantNode implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -1522063210471694599L;

    /**
     * Virtual tenant which contains this node.
     */
    private transient VTenantImpl  tenant;

    /**
     * Path to this virtual node.
     */
    private transient VNodePath  nodePath;

    /**
     * Read write lock to synchronize resources in this node.
     */
    private transient ReentrantReadWriteLock  rwLock =
        new ReentrantReadWriteLock();

    /**
     * Construct a new virtual node instance.
     */
    protected VTenantNode() {
    }

    /**
     * Return the name of the container to which this node belongs.
     *
     * @return  The name of the container.
     */
    public final String getContainerName() {
        return tenant.getContainerName();
    }

    /**
     * Return the VTN that contains this node.
     *
     * @return  A {@link VTenantImpl} instance.
     */
    final VTenantImpl getTenant() {
        return tenant;
    }

    /**
     * Return path to this node.
     *
     * @return  Path to the node.
     */
    final VNodePath getNodePath() {
        return nodePath;
    }

    /**
     * Set path to this node.
     *
     * @param vtn   Virtual tenant to which a new node is attached.
     * @param path  Path to this node.
     */
    final void setNodePath(VTenantImpl vtn, VNodePath path) {
        tenant = vtn;
        nodePath = path;
    }

    /**
     * Return the name of the tenant to which this node belongs.
     *
     * @return  The name of the tenant.
     */
    String getTenantName() {
        return tenant.getName();
    }

    /**
     * Return the name of this node.
     *
     * @return  The name of this node.
     */
    final String getName() {
        return nodePath.getTenantNodeName();
    }

    /**
     * Read data from the given input stream and deserialize.
     *
     * @param in  An input stream.
     * @throws IOException
     *    An I/O error occurred.
     * @throws ClassNotFoundException
     *    At least one necessary class was not found.
     */
    @SuppressWarnings("unused")
    private void readObject(ObjectInputStream in)
        throws IOException, ClassNotFoundException {
        // Read field information.
        in.readFields();

        // Reset the lock.
        rwLock = new ReentrantReadWriteLock();
    }

    /**
     * Serialize this object and write it to the given output stream.
     *
     * @param out  An output stream.
     * @throws IOException
     *    An I/O error occurred.
     */
    @SuppressWarnings("unused")
    private void writeObject(ObjectOutputStream out) throws IOException {
        // Write field information.
        out.putFields();
        out.writeFields();
    }

    /**
     * Return a lock instance.
     *
     * @param writer  {@code true} means the writer lock is required.
     * @return  A lock object.
     */
    protected final Lock getLock(boolean writer) {
        return (writer) ? rwLock.readLock() : rwLock.writeLock();
    }

    /**
     * Acquire reader lock of this node.
     *
     * @return  A lock object that holding the reader lock of this node.
     */
    protected final Lock readLock() {
        Lock l = rwLock.readLock();
        l.lock();
        return l;
    }

    /**
     * Acquire writer lock of this node.
     *
     * @return  A lock object that holding the writer lock of this node.
     */
    protected final Lock writeLock() {
        Lock l = rwLock.writeLock();
        l.lock();
        return l;
    }

    /**
     * Destroy this virtual node.
     */
    protected final void destroy() {
        // Unlink VTN for GC.
        tenant = null;
    }

    /**
     * Initialize node path for this instance.
     *
     * @param vtn   Virtual tenant to which a new node is attached.
     * @param name  The name of this node.
     */
    abstract void setPath(VTenantImpl vtn, String name);

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (o == null || !getClass().equals(o.getClass())) {
            return false;
        }

        VTenantNode vtnode = (VTenantNode)o;
        return nodePath.equals(vtnode.nodePath);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return getClass().getName().hashCode() +
            (nodePath.hashCode() * 9);
    }
}
