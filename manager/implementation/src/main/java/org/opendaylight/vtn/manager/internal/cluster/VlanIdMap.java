/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.io.Serializable;
import java.util.Map;
import java.util.HashMap;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VlanMapConfig;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * {@code VlanIdMap} class describes a set of VLAN mappings that map networks
 * specified by the same VLAN ID.
 *
 * <p>
 *   This class is not synchronized. If multiple threads acccess the same
 *   {@code VlanIdMap} object concurrently, it must be synchronized externally.
 * </p>
 */
public abstract class VlanIdMap implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -8096283940215754161L;

    /**
     * Create a new {@code VlanIdMap} object and put the given VLAN mapping
     * instance into a new object.
     *
     * @param vmap  VLAN mapping instance to be added to this object.
     * @return  A {@code VlanIdMap} object that contains the specified VLAN
     *          mapping instance is returned.
     */
    static VlanIdMap create(VlanMapImpl vmap) {
        VlanMapConfig vlc = vmap.getVlanMapConfig();
        Node node = vlc.getNode();

        return (node == null) ? new GlobalMap(vmap) : new NodeMap(node, vmap);
    }

    /**
     * A constructor to protect from instantiation by others.
     */
    private VlanIdMap() {
    }

    /**
     * Return the number of VLAN mappings in this object.
     *
     * @return  The number of VLAN mappings in this object.
     */
    protected abstract int getSize();

    /**
     * Determine whether this object is empty or not.
     *
     * @return  {@code true} is returned if this object no longer keeps
     *          VLAN mapping.
     *          {@code false} is returned if this object still keeps at least
     *          one VLAN mapping.
     */
    protected abstract boolean isEmpty();

    /**
     * Add a new VLAN mapping instance to this object.
     *
     * @param vmap  VLAN mapping instance to be added to this object.
     * @throws VTNException
     *     The VLAN associated with the given VLAN mapping overlaps with one of
     *     VLAN mappings in this object.
     */
    protected abstract void add(VlanMapImpl vmap) throws VTNException;

    /**
     * Remove the VLAN mapping associated with the given node.
     *
     * @param node  A node associated with the VLAN mapping to be removed.
     * @return  {@code true} is returned if this object no longer keeps VLAN
     *          mapping. If this method returns {@code true}, the caller
     *          must discard this object immediately.
     *          {@code false} is returned if this object still keeps at least
     *          VLAN mapping.
     */
    protected abstract boolean remove(Node node);

    /**
     * Return the VLAN mapping instance associated with the given node
     * in this object.
     *
     * @param node  A node.
     * @return  A {@link VlanMapImpl} instance is returned if found.
     *          {@code null} is returned if not found.
     */
    protected abstract VlanMapImpl get(Node node);

    /**
     * Return the VLAN mapping instance which maps the given node.
     *
     * @param node  A node. Specifying {@code null} results in undefined
     *              behavior.
     * @return  A {@link VlanMapImpl} instance is returned if found.
     *          {@code null} is returned if not found.
     */
    protected abstract VlanMapImpl match(Node node);

    /**
     * Throw an exception which indicates the specified VLAN mapping overlaps
     * with one of VLAN mappings in this object.
     *
     * @throws VTNException   Always thrown.
     */
    protected void conflict() throws VTNException {
        String msg = "Already mapped to this bridge";
        throw new VTNException(StatusCode.CONFLICT, msg);
    }

    /**
     * Implementation of {@link VlanIdMap} for a global VLAN mapping.
     *
     * <p>
     *   A global VLAN mapping maps VLAN network without specifying node.
     *   So an object of this class can include only one global VLAN mapping.
     * </p>
     */
    public static final class GlobalMap extends VlanIdMap {
        /**
         * Version number for serialization.
         */
        private static final long serialVersionUID = -9074749074869336346L;

        /**
         * A global VLAN mapping instance.
         */
        private VlanMapImpl  globalMap;

        /**
         * Construct a new {@link VlanIdMap} object for a global VLAN mapping.
         *
         * @param vmap  VLAN mapping instance to be added to this object.
         */
        private GlobalMap(VlanMapImpl vmap) {
            globalMap = vmap;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected int getSize() {
            return (globalMap == null) ? 0 : 1;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean isEmpty() {
            return (globalMap == null);
        }

        /**
         * Add a new VLAN mapping instance to this object.
         *
         * <p>
         *   Node that this method always throws an {@code VTNException}
         *   because a global VLAN mapping always overlaps with all VLAN
         *   mappings with the same VLAN ID.
         * </p>
         *
         * @param vmap  Unused.
         * @throws VTNException   Always thrown.
         */
        @Override
        protected void add(VlanMapImpl vmap) throws VTNException {
            conflict();
            // NOTREACHED
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean remove(Node node) {
            if (node == null) {
                globalMap = null;
                return true;
            }

            return (globalMap == null);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected VlanMapImpl get(Node node) {
            return (node == null) ? globalMap : null;
        }

        /**
         * Return the VLAN mapping instance which maps the given node.
         *
         * <p>
         *   This method always returns a global VLAN mapping instance in this
         *   object because a global VLAN mapping always overlaps with all
         *   VLAN mappings with the same VLAN ID.
         * </p>
         *
         * @param node  Unused.
         * @return  A {@link VlanMapImpl} instance in this object is always
         *          returned.
         */
        @Override
        protected VlanMapImpl match(Node node) {
            return globalMap;
        }

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
            if (!(o instanceof GlobalMap)) {
                return false;
            }

            GlobalMap gmap = (GlobalMap)o;
            if (globalMap == null) {
                return (gmap.globalMap == null);
            }

            return globalMap.equals(gmap.globalMap);
        }

        /**
         * Return the hash code of this object.
         *
         * @return  The hash code.
         */
        @Override
        public int hashCode() {
            int h = 0;

            if (globalMap != null) {
                h ^= globalMap.hashCode();
            }

            return h;
        }
    }

    /**
     * Implementation of {@link VlanIdMap} for VLAN mappings with specifying
     * nodes.
     *
     * <p>
     *   An object of this class can more than one VLAN mappings as long as
     *   all target nodes are different.
     * </p>
     */
    public static final class NodeMap extends VlanIdMap {
        /**
         * Version number for serialization.
         */
        private static final long serialVersionUID = 7523430032456522653L;

        /**
         * Pairs of nodes and VLAN mapping instances.
         * A node specified by VLAN mapping configuration is used as map key.
         */
        private final Map<Node, VlanMapImpl>  vlanMaps =
            new HashMap<Node, VlanMapImpl>();

        /**
         * Construct a new {@link VlanIdMap} object for a global VLAN mapping.
         *
         * @param node  A node specified by the VLAN mapping configuration.
         * @param vmap  VLAN mapping instance to be added to this object.
         */
        private NodeMap(Node node, VlanMapImpl vmap) {
            vlanMaps.put(node, vmap);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected int getSize() {
            return vlanMaps.size();
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean isEmpty() {
            return vlanMaps.isEmpty();
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void add(VlanMapImpl vmap) throws VTNException {
            // Ensure that the specified VLAN mapping is not overlapped.
            VlanMapConfig vlc = vmap.getVlanMapConfig();
            Node node = vlc.getNode();
            if (node != null) {
                VlanMapImpl old = vlanMaps.put(node, vmap);
                if (old == null) {
                    return;
                }

                node = old.getVlanMapConfig().getNode();
                vlanMaps.put(node, old);
            }

            conflict();
            // NOTREACHED
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean remove(Node node) {
            vlanMaps.remove(node);
            return vlanMaps.isEmpty();
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected VlanMapImpl get(Node node) {
            return vlanMaps.get(node);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected VlanMapImpl match(Node node) {
            return vlanMaps.get(node);
        }

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
            if (!(o instanceof NodeMap)) {
                return false;
            }

            NodeMap nmap = (NodeMap)o;
            return vlanMaps.equals(nmap.vlanMaps);
        }

        /**
         * Return the hash code of this object.
         *
         * @return  The hash code.
         */
        @Override
        public int hashCode() {
            return vlanMaps.hashCode();
        }
    }
}
