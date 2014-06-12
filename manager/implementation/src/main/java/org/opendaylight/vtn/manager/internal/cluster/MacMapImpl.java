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
import java.net.InetAddress;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;
import java.util.NavigableSet;
import java.util.Set;
import java.util.TreeSet;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.DataLinkHost;
import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.MacMap;
import org.opendaylight.vtn.manager.MacMapAclType;
import org.opendaylight.vtn.manager.MacMapConfig;
import org.opendaylight.vtn.manager.UpdateOperation;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VNodeState;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.internal.EdgeUpdateState;
import org.opendaylight.vtn.manager.internal.IVTNResourceManager;
import org.opendaylight.vtn.manager.internal.MacAddressTable;
import org.opendaylight.vtn.manager.internal.MacMapChange;
import org.opendaylight.vtn.manager.internal.MacMapConflictException;
import org.opendaylight.vtn.manager.internal.MacMapDuplicateException;
import org.opendaylight.vtn.manager.internal.MacMapGoneException;
import org.opendaylight.vtn.manager.internal.MacMapPortBusyException;
import org.opendaylight.vtn.manager.internal.NodePortFilter;
import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.SpecificPortFilter;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.address.DataLinkAddress;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * Implementation of MAC mapping to the virtual L2 bridge.
 *
 * <p>
 *   This class keeps VLAN mapping configuration and MAC addresses actually
 *   mapped by MAC mapping.
 * </p>
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class MacMapImpl implements VBridgeNode, Serializable, Cloneable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 8543149423569724022L;

    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(MacMapImpl.class);

    /**
     * The access control list which determines hosts to be mapped by
     * MAC mapping.
     */
    private NavigableSet<MacVlan>  allowedHosts = new TreeSet<MacVlan>();

    /**
     * The access control list which determines hosts not to be mapped by
     * MAC mapping.
     */
    private Set<MacVlan>  deniedHosts = new HashSet<MacVlan>();

    /**
     * Path to the MAC mapping.
     */
    private transient MacMapPath  mapPath;

    /**
     * A base class used to update the configuration of the MAC mapping.
     */
    protected abstract class UpdateContext {
        /**
         * Update the MAC mapping configuration.
         *
         * @param mgr  VTN Manager service.
         * @return     A {@link MacMapConfig} instance to be delivered as
         *             manager event is returned if the configuration for the
         *             MAC mapping was actually updated.
         *             Otherwise {@code null} is returned.
         * @throws VTNException  An error occurred.
         */
        protected final MacMapConfig update(VTNManagerImpl mgr)
            throws VTNException {
            if (!isUpdated()) {
                // Nothing to do.
                return null;
            }

            MacMapConfig ret;
            if (isAddedSetEmpty()) {
                // The MAC mapping may be removed by this operation.
                // So the current configuration needs to be preserved for
                // manager event.
                ret = getMacMapConfig();
            } else {
                ret = null;
            }

            boolean removed = register(mgr);
            apply();

            if (ret == null || !removed) {
                // Return new configuration.
                ret = getMacMapConfig();
            }

            return ret;
        }

        /**
         * Determine whether the MAC mapping configuration needs to be updated
         * or not.
         *
         * @return  {@code true} is returned only if the MAC mapping
         *          configuration needs to be updated.
         */
        protected abstract boolean isUpdated();

        /**
         * Determine whether a set of {@link MacVlan} instances which contains
         * a host information to be added to the access control list is
         * empty or not.
         *
         * @return  {@code true} is returned only if the set is empty.
         */
        protected abstract boolean isAddedSetEmpty();

        /**
         * Register the MAC mapping configuration to the global resource
         * manager.
         *
         * @param mgr  VTN Manager service.
         * @return  {@code true} is returned if this MAC mapping was removed.
         *          Otherwise {@code false} is returned.
         * @throws VTNException  An error occurred.
         */
        protected abstract boolean register(VTNManagerImpl mgr)
            throws VTNException;

        /**
         * Apply changes to the MAC mapping configuration.
         */
        protected abstract void apply();
    }

    /**
     * A base class used to update the contents of the access control list
     * of the MAC mapping.
     */
    protected abstract class UpdateAclContext extends UpdateContext {
        /**
         * A set of {@link MacVlan} instances to be added to the host set.
         */
        private final Set<MacVlan>  addedSet = new HashSet<MacVlan>();

        /**
         * A set of {@link MacVlan} instances to be removed from the host set.
         */
        private final Set<MacVlan>  removedSet = new HashSet<MacVlan>();

        /**
         * Set up the context used to update the target access control list.
         *
         * @param op       A {@link UpdateOperation} instance which indicates
         *                 how to update the MAC mapping configuration.
         * @param dlhosts  A {@link MacMapConfig} instance which contains the
         *                 MAC mapping configuration information.
         * @throws VTNException  An error occurred.
         */
        private void setUp(UpdateOperation op,
                           Set<? extends DataLinkHost> dlhosts)
            throws VTNException {
            if (op == UpdateOperation.SET) {
                // Change configuration as specified exactly.
                set(dlhosts);
            } else if (op == null) {
                throw new VTNException(operationCannotBeNull());
            } else {
                if (dlhosts == null || dlhosts.isEmpty()) {
                    return;
                }

                if (op == UpdateOperation.ADD) {
                    // Add the specified configuration to the current
                    // configuration.
                    add(dlhosts);
                } else {
                    // Remove the specified configuration from the current
                    // configuration.
                    assert op == UpdateOperation.REMOVE;
                    remove(dlhosts);
                }
            }
        }

        /**
         * Determine whether the target access control list will be empty
         * after change or not.
         *
         * @return  {@code true} is returned only if the target access
         *          control list will be empty after change.
         */
        private boolean willBeEmpty() {
            int sz = getTarget().size() + addedSet.size() - removedSet.size();
            return (sz == 0);
        }

        /**
         * Return a set of {@link MacVlan} that keeps {@link MacVlan} instances
         * to be added.
         *
         * @return  A set of {@link MacVlan} that keeps {@link MacVlan}
         *          instances to be added to the access control host set.
         */
        protected final Set<MacVlan> getAddedSet() {
            return addedSet;
        }

        /**
         * Return a set of {@link MacVlan} that keeps {@link MacVlan} instances
         * to be removed.
         *
         * @return  A set of {@link MacVlan} that keeps {@link MacVlan}
         *          instances to be removed from the access control host set.
         */
        protected final Set<MacVlan> getRemovedSet() {
            return removedSet;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected final boolean isUpdated() {
            return (!(addedSet.isEmpty() && removedSet.isEmpty()));
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected final boolean isAddedSetEmpty() {
            return addedSet.isEmpty();
        }

        /**
         * Perform {@link UpdateOperation#SET} operation.
         *
         * @param dlhosts  A set of {@link DataLinkHost} instances to be
         *                 exactly set to the access control host set.
         * @throws VTNException  An error occurred.
         */
        protected final void set(Set<? extends DataLinkHost> dlhosts)
            throws VTNException {
            if (dlhosts == null || dlhosts.isEmpty()) {
                // Remove all hosts.
                removedSet.addAll(getTarget());
            } else {
                setImpl(dlhosts);
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected final void apply() {
            Set<MacVlan> target = getTarget();
            target.removeAll(removedSet);
            target.addAll(addedSet);
        }

        /**
         * Return the target set of {@link MacVlan} instances.
         *
         * @return  The target set of {@link MacVlan}.
         */
        protected abstract Set<MacVlan> getTarget();

        /**
         * Perform {@link UpdateOperation#SET} operation.
         *
         * @param dlhosts  A set of {@link DataLinkHost} instances to be
         *                 exactly set to the access control host set.
         * @throws VTNException  An error occurred.
         */
        protected abstract void setImpl(Set<? extends DataLinkHost> dlhosts)
            throws VTNException;

        /**
         * Perform {@link UpdateOperation#ADD} operation.
         *
         * @param dlhosts  A set of {@link DataLinkHost} instances to be
         *                 added to the access control host set.
         * @throws VTNException  An error occurred.
         */
        protected abstract void add(Set<? extends DataLinkHost> dlhosts)
            throws VTNException;

        /**
         * Perform {@link UpdateOperation#REMOVE} operation.
         *
         * @param dlhosts  A set of {@link DataLinkHost} instances to be
         *                 removed from the access control host set.
         * @throws VTNException  An error occurred.
         */
        protected abstract void remove(Set<? extends DataLinkHost> dlhosts)
            throws VTNException;
    }

    /**
     * An internal class used to update {@link #allowedHosts}.
     */
    private final class UpdateAllowedContext extends UpdateAclContext {
        /**
         * {@inheritDoc}
         */
        @Override
        protected Set<MacVlan> getTarget() {
            return allowedHosts;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void setImpl(Set<? extends DataLinkHost> dlhosts)
            throws VTNException {
            Set<MacVlan> added = getAddedSet();
            HashSet<MacVlan> retained = new HashSet<MacVlan>();
            HashSet<Long> macSet = new HashSet<Long>();
            for (DataLinkHost dlhost: dlhosts) {
                MacVlan mvlan = new MacVlan(dlhost);
                if (allowedHosts.contains(mvlan)) {
                    retained.add(mvlan);
                } else {
                    long mac = mvlan.getMacAddress();
                    if (mac != MacVlan.UNDEFINED && !macSet.add(mac)) {
                        throw new VTNException(duplicatedMacAddress(mac));
                    }
                    added.add(mvlan);
                }
            }

            Set<MacVlan> removed = getRemovedSet();
            for (MacVlan mvlan: allowedHosts) {
                if (retained.contains(mvlan)) {
                    long mac = mvlan.getMacAddress();
                    if (mac != MacVlan.UNDEFINED && !macSet.add(mac)) {
                        throw new VTNException(alreadyMapped(mac));
                    }
                } else {
                    removed.add(mvlan);
                }
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void add(Set<? extends DataLinkHost> dlhosts)
            throws VTNException {
            Set<MacVlan> added = getAddedSet();
            HashSet<Long> macSet = new HashSet<Long>();
            for (DataLinkHost dlhost: dlhosts) {
                MacVlan mvlan = new MacVlan(dlhost);
                if (!allowedHosts.contains(mvlan)) {
                    long mac = mvlan.getMacAddress();
                    if (mac != MacVlan.UNDEFINED) {
                        if (!macSet.add(mac)) {
                            throw new VTNException(duplicatedMacAddress(mac));
                        }
                        if (containsMacAddress(allowedHosts, mac)) {
                            throw new VTNException(alreadyMapped(mac));
                        }
                    }
                    added.add(mvlan);
                }
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void remove(Set<? extends DataLinkHost> dlhosts)
            throws VTNException {
            Set<MacVlan> removed = getRemovedSet();
            HashSet<Long> macSet = new HashSet<Long>();
            for (DataLinkHost dlhost: dlhosts) {
                MacVlan mvlan = new MacVlan(dlhost);
                if (allowedHosts.contains(mvlan)) {
                    removed.add(mvlan);
                }
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean  register(VTNManagerImpl mgr) throws VTNException {
            return register(mgr, this, new UpdateDeniedContext());
        }
    }

    /**
     * An internal class used to update {@link #deniedHosts}.
     */
    private final class UpdateDeniedContext extends UpdateAclContext {
        /**
         * {@inheritDoc}
         */
        @Override
        protected Set<MacVlan> getTarget() {
            return deniedHosts;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void setImpl(Set<? extends DataLinkHost> dlhosts)
            throws VTNException {
            Set<MacVlan> added = getAddedSet();
            HashSet<MacVlan> retained = new HashSet<MacVlan>();
            for (DataLinkHost dlhost: dlhosts) {
                MacVlan mvlan = createMacVlan(dlhost);
                if (deniedHosts.contains(mvlan)) {
                    retained.add(mvlan);
                } else {
                    added.add(mvlan);
                }
            }

            Set<MacVlan> removed = getRemovedSet();
            for (MacVlan mvlan: deniedHosts) {
                if (!retained.contains(mvlan)) {
                    removed.add(mvlan);
                }
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void add(Set<? extends DataLinkHost> dlhosts)
            throws VTNException {
            Set<MacVlan> added = getAddedSet();
            for (DataLinkHost dlhost: dlhosts) {
                MacVlan mvlan = createMacVlan(dlhost);
                if (!deniedHosts.contains(mvlan)) {
                    added.add(mvlan);
                }
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void remove(Set<? extends DataLinkHost> dlhosts)
            throws VTNException {
            Set<MacVlan> removed = getRemovedSet();
            for (DataLinkHost dlhost: dlhosts) {
                MacVlan mvlan = new MacVlan(dlhost);
                if (deniedHosts.contains(mvlan)) {
                    removed.add(mvlan);
                }
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean register(VTNManagerImpl mgr) throws VTNException {
            return register(mgr, new UpdateAllowedContext(), this);
        }

        /**
         * Create a new {@link MacVlan} instance for denied host list.
         *
         * @param dlhost  A {@link DataLinkHost} instance.
         * @return  A {@link MacVlan} instance which represents the specified
         *          host.
         * @throws VTNException  An error occurred.
         */
        private MacVlan createMacVlan(DataLinkHost dlhost)
            throws VTNException {
            MacVlan mvlan = new MacVlan(dlhost);
            if (mvlan.getMacAddress() == MacVlan.UNDEFINED) {
                throw new VTNException
                    (StatusCode.BADREQUEST,
                     "MAC address cannot be null in denied hosts: " + dlhost);
            }

            return mvlan;
        }
    }

    /**
     * An internal class used to update both {@link #allowedHosts} and
     * {@link #deniedHosts}.
     */
    private final class UpdateBothContext extends UpdateContext {
        /**
         * A context used to update <strong>allow</strong> access control list.
         */
        private final UpdateAclContext  allowedContext =
            new UpdateAllowedContext();

        /**
         * A context used to update <strong>deny</strong> access control list.
         */
        private final UpdateAclContext  deniedContext =
            new UpdateDeniedContext();

        /**
         * Set up the context used to update both <strong>allow</strong> and
         * <strong>deny</strong> access control lists.
         *
         * @param op      A {@link UpdateOperation} instance which indicates
         *                how to update the MAC mapping configuration.
         * @param mcconf  A {@link MacMapConfig} instance which contains the
         *                MAC mapping configuration information.
         * @throws VTNException  An error occurred.
         */
        private UpdateBothContext(UpdateOperation op, MacMapConfig mcconf)
            throws VTNException {
            if (op == UpdateOperation.SET) {
                // Update configuration as specified exactly.
                Set<DataLinkHost> allow, deny;
                if (mcconf == null) {
                    allow = null;
                    deny = null;
                } else {
                    allow = mcconf.getAllowedHosts();
                    deny = mcconf.getDeniedHosts();
                }
                allowedContext.set(allow);
                deniedContext.set(deny);
            } else if (op == null) {
                throw new VTNException(operationCannotBeNull());
            } else {
                if (mcconf == null) {
                    return;
                }

                Set<DataLinkHost> allow = mcconf.getAllowedHosts();
                Set<DataLinkHost> deny = mcconf.getDeniedHosts();
                if (op == UpdateOperation.ADD) {
                    // Add the specified configuration to the current
                    // configuration.
                    allowedContext.add(allow);
                    deniedContext.add(deny);
            } else {
                    // Remove the specified configuration from the current
                    // configuration.
                    assert op == UpdateOperation.REMOVE;
                    allowedContext.remove(allow);
                    deniedContext.remove(deny);
                }
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean isUpdated() {
            return (allowedContext.isUpdated() || deniedContext.isUpdated());
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean isAddedSetEmpty() {
            return (allowedContext.isAddedSetEmpty() &&
                    deniedContext.isAddedSetEmpty());
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean register(VTNManagerImpl mgr) throws VTNException {
            return register(mgr, allowedContext, deniedContext);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void apply() {
            allowedContext.apply();
            deniedContext.apply();
        }
    }

    /**
     * Construct a MAC mapping instance.
     *
     * @param path  Path to the virtual L2 bridge which includes this mapping.
     */
    MacMapImpl(VBridgePath path) {
        setPath(path);
    }

    /**
     * Set vBridge path which includes this MAC mapping.
     *
     * @param path  Path to the virtual L2 bridge which includes this mapping.
     */
    void setPath(VBridgePath path) {
        mapPath = new MacMapPath(path);
    }

    /**
     * Determine whether the configuration of MAC mapping is empty or not.
     *
     * @return  {@code true} is returned if the configuration of this MAC
     *          mapping is empty().
     */
    boolean isEmpty() {
        return (allowedHosts.isEmpty() && deniedHosts.isEmpty());
    }

    /**
     * Return information about the MAC mapping configuration.
     *
     * <p>
     *   Note that this method must be called with holding the bridge lock.
     * </p>
     *
     * @return  A {@link MacMapConfig} object which represents the
     *          configuration information about the MAC mapping.
     */
    MacMapConfig getMacMapConfig() {
        Set<DataLinkHost> allowed = toDataLinkHostSet(allowedHosts);
        Set<DataLinkHost> denied = toDataLinkHostSet(deniedHosts);

        return new MacMapConfig(allowed, denied);
    }

    /**
     * Return information about the MAC mapping configured in this vBridge.
     *
     * <p>
     *   Note that this method must be called with holding the bridge lock.
     * </p>
     *
     * @param mgr   VTN Manager service.
     * @return  A {@link MacMap} object which represents information about
     *          the MAC mapping configured in this vBridge.
     * @throws VTNException  An error occurred.
     */
    MacMap getMacMap(VTNManagerImpl mgr) throws VTNException {
        Set<DataLinkHost> allowed = toDataLinkHostSet(allowedHosts);
        Set<DataLinkHost> denied = toDataLinkHostSet(deniedHosts);
        List<MacAddressEntry> mapped = getMacMappedHosts(mgr);

        return new MacMap(allowed, denied, mapped);
    }

    /**
     * Return configuration information about MAC mapping in the specified
     * vBridge.
     *
     * <p>
     *   Note that this method must be called with holding the bridge lock.
     * </p>
     *
     * @param aclType  The type of access control list.
     * @return  A set of {@link DataLinkHost} instances which contains host
     *          information in the specified access control list is returned.
     * @throws VTNException  An error occurred.
     */
    Set<DataLinkHost> getMacMapConfig(MacMapAclType aclType)
        throws VTNException {
        if (aclType == MacMapAclType.ALLOW) {
            return toDataLinkHostSet(allowedHosts);
        }
        if (aclType == MacMapAclType.DENY) {
            return toDataLinkHostSet(deniedHosts);
        }

        assert aclType == null;
        throw new VTNException(aclCannotBeNull());
    }

    /**
     * Return a list of {@link MacAddressEntry} instances corresponding to
     * all the MAC address information actually mapped by MAC mapping
     * configured in the specified vBridge.
     *
     * <p>
     *   Note that this method must be called with holding the bridge lock.
     * </p>
     *
     * @param mgr   VTN Manager service.
     * @return  A list of {@link MacAddressEntry} instances corresponding to
     *          all the MAC address information actually mapped to the vBridge
     *          specified by {@code path}.
     * @throws VTNException  An error occurred.
     */
    List<MacAddressEntry> getMacMappedHosts(VTNManagerImpl mgr)
        throws VTNException {
        ArrayList list = new ArrayList<MacAddressEntry>();
        IVTNResourceManager resMgr = mgr.getResourceManager();
        Map<MacVlan, NodeConnector> map =
            resMgr.getMacMappedHosts(mgr, mapPath);
        if (map != null) {
            for (Entry<MacVlan, NodeConnector> entry: map.entrySet()) {
                MacVlan mvlan = entry.getKey();
                NodeConnector port = entry.getValue();
                list.add(createMacAddressEntry(mgr, mvlan, port));
            }
        }
        list.trimToSize();

        return list;
    }

    /**
     * Determine whether the host specified by the MAC address is actually
     * mapped by MAC mapping configured in the specified vBridge.
     *
     * @param mgr   VTN Manager service.
     * @param addr  A {@link DataLinkAddress} instance which represents the
     *              MAC address.
     * @return  A {@link MacAddressEntry} instancw which represents information
     *          about the host corresponding to {@code addr} is returned
     *          if it is actually mapped to the specified vBridge by MAC
     *          mapping.
     *          {@code null} is returned if the MAC address specified by
     *          {@code addr} is not mapped by MAC mapping.
     * @throws VTNException  An error occurred.
     */
    MacAddressEntry getMacMappedHost(VTNManagerImpl mgr, DataLinkAddress addr)
        throws VTNException {
        if (!(addr instanceof EthernetAddress)) {
            return null;
        }

        EthernetAddress eaddr = (EthernetAddress)addr;
        long mac = NetUtils.byteArray6ToLong(eaddr.getValue());

        IVTNResourceManager resMgr = mgr.getResourceManager();
        PortVlan pvlan = resMgr.getMacMappedNetwork(mgr, mapPath, mac);
        if (pvlan == null) {
            return null;
        }

        return createMacAddressEntry(mgr, eaddr, mac, pvlan.getVlan(),
                                     pvlan.getNodeConnector());
    }

    /**
     * Change MAC mapping configuration as specified by {@link MacMapConfig}
     * instance.
     *
     * <p>
     *   Note that this method must be called with holding the bridge write
     *   lock.
     * </p>
     *
     * @param mgr     VTN Manager service.
     * @param op      A {@link UpdateOperation} instance which indicates
     *                how to change the MAC mapping configuration.
     * @param mcconf  A {@link MacMapConfig} instance which contains the MAC
     *                mapping configuration information.
     * @return        A {@link MacMapConfig} instance to be delivered as
     *                manager event is returned if the configuration for the
     *                MAC mapping was actually changed.
     *                Otherwise {@code null} is returned.
     * @throws VTNException  An error occurred.
     */
    MacMapConfig setMacMap(VTNManagerImpl mgr, UpdateOperation op,
                           MacMapConfig mcconf) throws VTNException {
        UpdateBothContext context = new UpdateBothContext(op, mcconf);
        return context.update(mgr);
    }

    /**
     * Change MAC mapping configuration as specified by a set of
     * {@link DataLinkHost} instances.
     *
     * <p>
     *   Note that this method must be called with holding the bridge write
     *   lock.
     * </p>
     *
     * @param mgr      VTN Manager service.
     * @param op       A {@link UpdateOperation} instance which indicates
     *                 how to change the MAC mapping configuration.
     * @param aclType  The type of access control list.
     * @param dlhosts  A {@link MacMapConfig} instance which contains the MAC
     *                 mapping configuration information.
     * @return         A {@link MacMapConfig} instance to be delivered as
     *                 manager event is returned if the configuration for the
     *                 MAC mapping was actually changed.
     *                 Otherwise {@code null} is returned.
     * @throws VTNException  An error occurred.
     */
    MacMapConfig setMacMap(VTNManagerImpl mgr, UpdateOperation op,
                           MacMapAclType aclType,
                           Set<? extends DataLinkHost> dlhosts)
        throws VTNException {
        UpdateAclContext context = createUpdateContext(aclType);
        context.setUp(op, dlhosts);
        return context.update(mgr);
    }

    /**
     * Determine vBridge state from current MAC mapping state.
     *
     * @param mgr     VTN Manager service.
     * @param bstate  Current bridge state value.
     * @return  New bridge state value.
     */
    VNodeState getBridgeState(VTNManagerImpl mgr, VNodeState bstate) {
        IVTNResourceManager resMgr = mgr.getResourceManager();
        return getBridgeState(mgr, resMgr, bstate);
    }

    /**
     * Resume MAC mapping.
     *
     * <p>
     *   This method is called just after this bridge is instantiated from
     *   the configuration file.
     * </p>
     *
     * @param mgr     VTN Manager service.
     * @param bstate  Current bridge state value.
     * @return  New bridge state value.
     */
    VNodeState resume(VTNManagerImpl mgr, VNodeState bstate) {
        IVTNResourceManager resMgr = mgr.getResourceManager();
        MacMapChange change = new MacMapChange(allowedHosts, deniedHosts,
                                               MacMapChange.DONT_PURGE);

        try {
            resMgr.registerMacMap(mgr, mapPath, change);
        } catch (Exception e) {
            StringBuilder builder =
                createLog(mgr, "Failed to resume MAC mapping");
            LOG.error(builder.toString(), e);
            // FALLTHROUGH
        }

        return getBridgeState(mgr, resMgr, bstate);
    }

    /**
     * Destroy MAC mapping.
     *
     * @param mgr     VTN manager service.
     * @param bpath   Path to the parent bridge.
     * @param mcconf  A {@link MacMapConfig} instance to be delivered as
     *                manager event. The current configuration is used if
     *                {@code null} is specified.
     * @param retain  {@code true} means that the parent bridge will be
     *                retained. {@code false} means that the parent bridge
     *                is being destroyed.
     */
    void destroy(VTNManagerImpl mgr, VBridgePath bpath, MacMapConfig mcconf,
                 boolean retain) {
        MacMapConfig conf = mcconf;
        if (conf == null) {
            conf =  getMacMapConfig();
        }

        if (!isEmpty()) {
            // Unregister MAC mapping from the global resource manager.
            int flags = MacMapChange.REMOVING;
            if (!retain) {
                flags |= MacMapChange.DONT_PURGE;
            }

            IVTNResourceManager resMgr = mgr.getResourceManager();
            MacMapChange change = new MacMapChange(allowedHosts, deniedHosts,
                                                   flags);
            try {
                resMgr.registerMacMap(mgr, mapPath, change);
            } catch (Exception e) {
                StringBuilder builder =
                    createLog(mgr, "Failed to unregister MAC mapping");
                LOG.error(builder.toString(), e);
                // FALLTHROUGH
            }

            allowedHosts.clear();
            deniedHosts.clear();
        }

        // Generate a MAC mapping event.
        MacMapEvent.removed(mgr, bpath, conf, retain);
    }

    /**
     * Invoked when a node is added, removed, or changed.
     *
     * @param mgr     VTN Manager service.
     * @param bstate  Current bridge state value.
     * @param node    Node being updated.
     * @param type    Type of update.
     * @return  New bridge state value.
     */
    VNodeState notifyNode(VTNManagerImpl mgr, VNodeState bstate, Node node,
                          UpdateType type) {
        IVTNResourceManager resMgr = mgr.getResourceManager();
        if (type != UpdateType.REMOVED) {
            return getBridgeState(mgr, resMgr, bstate);
        }

        NodePortFilter filter = new NodePortFilter(node);
        try {
            boolean active = resMgr.inactivateMacMap(mgr, mapPath, filter);
            return getBridgeState(active, bstate);
        } catch (Exception e) {
            StringBuilder builder =
                createLog(mgr, "Failed to inactivate MAC mapping on switch: ").
                append(node.toString());
            LOG.error(builder.toString(), e);
        }

        return bstate;
    }

    /**
     * This method is called when some properties of a node connector are
     * added/deleted/changed.
     *
     * @param mgr     VTN Manager service.
     * @param bstate  Current bridge state value.
     * @param nc      Node connector being updated.
     * @param pstate  The state of the node connector.
     * @param type    Type of update.
     * @return  New bridge state value.
     */
    VNodeState notifyNodeConnector(VTNManagerImpl mgr, VNodeState bstate,
                                   NodeConnector nc, VNodeState pstate,
                                   UpdateType type) {
        IVTNResourceManager resMgr = mgr.getResourceManager();
        if (type != UpdateType.REMOVED && pstate == VNodeState.UP) {
            return getBridgeState(mgr, resMgr, bstate);
        }

        SpecificPortFilter filter = new SpecificPortFilter(nc);
        try {
            boolean active = resMgr.inactivateMacMap(mgr, mapPath, filter);
            return getBridgeState(active, bstate);
        } catch (Exception e) {
            StringBuilder builder =
                createLog(mgr, "Failed to inactivate MAC mapping on " +
                          "switch port: ").
                append(nc.toString());
            LOG.error(builder.toString(), e);
        }

        return bstate;
    }

    /**
     * This method is called when topology graph is changed.
     *
     * @param mgr     VTN Manager service.
     * @param bstate  Current bridge state value.
     * @param estate  A {@link EdgeUpdateState} instance which contains
     *                information reported by the controller.
     * @return  New bridge state value.
     */
    VNodeState edgeUpdate(VTNManagerImpl mgr, VNodeState bstate,
                          EdgeUpdateState estate) {
        // Inactivate MAC mappings associated with ISL ports.
        IVTNResourceManager resMgr = mgr.getResourceManager();
        try {
            boolean active = resMgr.inactivateMacMap(mgr, mapPath, estate);
            return getBridgeState(active, bstate);
        } catch (Exception e) {
            StringBuilder builder =
                createLog(mgr, "Failed to inactivate MAC mapping on " +
                          "ISL ports");
            LOG.error(builder.toString(), e);
        }

        return bstate;
    }

    /**
     * Transmit the specified packet to the network established by this
     * MAC mapping.
     *
     * @param mgr   VTN manager service.
     * @param pctx  The context of the packet.
     * @param sent  A set of {@link PortVlan} which indicates the network
     *              already processed.
     */
    void transmit(VTNManagerImpl mgr, PacketContext pctx, Set<PortVlan> sent) {
        IVTNResourceManager resMgr = mgr.getResourceManager();
        Set<PortVlan> networks = resMgr.getMacMappedNetworks(mgr, mapPath);
        if (networks == null) {
            LOG.trace("{}:{}: transmit: MAC mapping is not active",
                      mgr.getContainerName(), mapPath);
            return;
        }

        for (PortVlan pvlan: networks) {
            if (!sent.add(pvlan)) {
                continue;
            }

            NodeConnector port = pvlan.getNodeConnector();
            short vlan = pvlan.getVlan();
            Ethernet frame = pctx.createFrame(vlan);
            if (LOG.isTraceEnabled()) {
                LOG.trace("{}:{}: Transmit packet to MAC mapping: {}",
                          mgr.getContainerName(), mapPath,
                          pctx.getDescription(frame, port, vlan));
            }
            mgr.transmit(port, frame);
        }
    }

    /**
     * Determine whether the specified host is activated in the MAC mapping
     * or not.
     * @param mgr    VTN manager service.
     * @param mvlan  A {@link MacVlan} instance which specifies the L2 host.
     * @param port   A {@link NodeConnector} instance corresponding to a
     *               switch port where the host was detected.
     * @return  {@code true} is returned if the specified host is activated
     *          in the MAC mapping. Otherwise {@code false} is returned.
     */
    boolean isActive(VTNManagerImpl mgr, MacVlan mvlan, NodeConnector port) {
        IVTNResourceManager resMgr = mgr.getResourceManager();
        NodeConnector mapped = resMgr.getMacMappedPort(mgr, mapPath, mvlan);
        if (port.equals(mapped)) {
            return true;
        }

        if (mapped != null) {
            StringBuilder builder = new StringBuilder();
            appendDescription(builder, mvlan, port);
            builder.append(", mapped=").append(mapped.toString());
            LOG.warn("{}:{}: Unexpected port is mapped by MAC mapping: {}",
                     mgr.getContainerName(), mapPath, builder);
        }

        return false;
    }

    /**
     * Activate the specified host in the MAC mapping.
     *
     * @param mgr    VTN manager service.
     * @param mvlan  A {@link MacVlan} instance which specifies the L2 host.
     * @param port   A {@link NodeConnector} instance corresponding to a
     *               switch port where the host was detected.
     * @return  {@link Boolean#TRUE} is returned if this MAC mapping has been
     *          activated.
     *          {@link Boolean#FALSE} is returned if this MAC mapping is
     *          already active.
     *          {@code null} is returned on failure.
     */
    Boolean activate(VTNManagerImpl mgr, MacVlan mvlan, NodeConnector port) {
        IVTNResourceManager resMgr = mgr.getResourceManager();
        String emsg;
        try {
            boolean ret = resMgr.activateMacMap(mgr, mapPath, mvlan, port);
            return Boolean.valueOf(ret);
        } catch (MacMapGoneException e) {
            // This may happen if the virtual network mapping is changed
            // by another cluster node.
            emsg = "No longer mapped by MAC mapping";
        } catch (MacMapPortBusyException e) {
            StringBuilder builder =
                new StringBuilder("Switch port is exclusively used by ");
            builder.append(e.getAnotherMapping().toString());
            emsg = builder.toString();
        } catch (MacMapDuplicateException e) {
            MacVlan dup = e.getDuplicate();
            long mac = mvlan.getMacAddress();
            StringBuilder builder = new StringBuilder("MAC address(");
            builder.append(VTNManagerImpl.formatMacAddress(mac)).
                append(") on VLAN ").append((int)dup.getVlan()).
                append(" is already mapped");
            emsg = builder.toString();
        } catch (Exception e) {
            StringBuilder builder =
                createLog(mgr, "Failed to activate MAC mapping: ");
            appendDescription(builder, mvlan, port);
            LOG.error(builder.toString(), e);
            return null;
        }

        StringBuilder builder = new StringBuilder();
        appendDescription(builder, mvlan, port);
        LOG.error("{}:{}: {}: {}", mgr.getContainerName(), mapPath,
                  emsg, builder);

        return null;
    }

    /**
     * Register MAC mapping configuration to the global resource service.
     *
     * @param mgr      VTN manager service.
     * @param allowed  A {@link UpdateAclContext} instance which keeps changes
     *                 to be applied to {@link #allowedHosts}.
     * @param denied   A {@link UpdateAclContext} instance which keeps changes
     *                 to be applied to {@link #deniedHosts}.
     * @return  {@code true} is returned if this MAC mapping was removed.
     *          Otherwise {@code false} is returned.
     * @throws VTNException  An error occurred.
     */
    private boolean register(VTNManagerImpl mgr, UpdateAclContext allowed,
                             UpdateAclContext denied) throws VTNException {
        IVTNResourceManager resMgr = mgr.getResourceManager();
        Set<MacVlan> allowAdded = allowed.getAddedSet();
        Set<MacVlan> allowRemoved = allowed.getRemovedSet();
        Set<MacVlan> denyAdded = denied.getAddedSet();
        Set<MacVlan> denyRemoved = denied.getRemovedSet();
        boolean removing = (allowed.willBeEmpty() && denied.willBeEmpty());
        int flags = (removing) ? MacMapChange.REMOVING : 0;
        MacMapChange change = new MacMapChange(allowAdded, allowRemoved,
                                               denyAdded, denyRemoved, flags);

        try {
            resMgr.registerMacMap(mgr, mapPath, change);
        } catch (MacMapConflictException e) {
            MacVlan host = e.getHost();
            MapReference ref = e.getMapReference();
            assert ref.getMapType() == MapType.MAC;
            throw new VTNException(alreadyMapped(host, ref.getAbsolutePath()));
        }

        return removing;
    }

    /**
     * Create a context to update the access control host set.
     *
     * @param aclType  The type of access control list.
     * @return  A {@link UpdateAclContext} instance.
     * @throws VTNException  An error occurred.
     */
    private UpdateAclContext createUpdateContext(MacMapAclType aclType)
        throws VTNException {
        if (aclType == MacMapAclType.ALLOW) {
            return new UpdateAllowedContext();
        }
        if (aclType == MacMapAclType.DENY) {
            return new UpdateDeniedContext();
        }

        assert aclType == null;
        throw new VTNException(aclCannotBeNull());
    }

    /**
     * Convert a set of {@link MacVlan} instances into a set of
     * {@link DataLinkHost} instances.
     *
     * @param set  A set of {@link MacVlan} instances.
     * @return     A set of {@link DataLinkHost} instances.
     */
    private Set<DataLinkHost> toDataLinkHostSet(Set<MacVlan> set) {
        Set<DataLinkHost> hostSet = new HashSet<DataLinkHost>();
        for (MacVlan mvlan: set) {
            hostSet.add(mvlan.getEthernetHost());
        }

        return hostSet;
    }

    /**
     * Determine whether the specified MAC address is contained in a set
     * of {@link MacVlan} instances.
     *
     * @param set   A set of {@link MacVlan} instances.
     * @param addr  A MAC address to be tested.
     * @return      {@code true} is returned only if the MAC address specified
     *              by {@code addr} is contained in {@code set}.
     */
    private boolean containsMacAddress(NavigableSet<MacVlan> set, long addr) {
        MacVlan key = new MacVlan(addr, (short)MacVlan.MASK_VLAN_ID);
        MacVlan floor = set.floor(key);
        return (floor != null && floor.getMacAddress() == addr);
    }

    /**
     * Create an error status that indicates {@link UpdateOperation}
     * cannot be null.
     *
     * @return  An error status.
     */
    private Status operationCannotBeNull() {
        return VTNManagerImpl.argumentIsNull("Operation");
    }

    /**
     * Create an error status that indicates {@link MacMapAclType}
     * cannot be null.
     *
     * @return  An error status.
     */
    private Status aclCannotBeNull() {
        return VTNManagerImpl.argumentIsNull("ACL type");
    }

    /**
     * Create an error status that indicates the same MAC addresses cannot
     * be specified to allowed hosts.
     *
     * @param mac  A MAC address.
     * @return  An error status.
     */
    private Status duplicatedMacAddress(long mac) {
        String maddr = VTNManagerImpl.formatMacAddress(mac);
        StringBuilder builder = new StringBuilder("MAC address(");
        builder.append(maddr).append(" is duplicated in allowed set");
        return new Status(StatusCode.BADREQUEST, builder.toString());
    }

    /**
     * Create an error status that indicates the specified host is
     * already mapped to this vBridge.
     *
     * @param mvlan  A {@link MacVlan} instance.
     * @param path   A string representaiton of path to the MAC mapping
     *               which maps the specified host.
     *               {@code null} means this MAC mapping.
     * @return  An error status.
     */
    private Status alreadyMapped(MacVlan mvlan, String path) {
        StringBuilder builder = new StringBuilder("Host(");
        mvlan.appendContents(builder);
        builder.append(") is already mapped to ").append(path);
        return new Status(StatusCode.CONFLICT, builder.toString());
    }

    /**
     * Create an error status that indicates the specified MAC address is
     * already mapped to this vBridge.
     *
     * @param mac  A MAC address.
     * @return  An error status.
     */
    private Status alreadyMapped(long mac) {
        String maddr = VTNManagerImpl.formatMacAddress(mac);
        StringBuilder builder = new StringBuilder("MAC address(");
        builder.append(maddr).append(") is already mapped to this vBridge");
        return new Status(StatusCode.CONFLICT, builder.toString());
    }

    /**
     * Create a {@link MacAddressEntry} instance which represents the
     * specified host.
     *
     * @param mgr    VTN manager service.
     * @param mvlan  A {@link MacVlan} instance which specifies the L2 host.
     * @param port   A {@link NodeConnector} instance corresponding to a
     *               switch port to which the host is connected.
     * @return  A {@link MacAddressEntry} instance.
     * @throws VTNException  An error occurred.
     */
    private MacAddressEntry createMacAddressEntry(VTNManagerImpl mgr,
                                                  MacVlan mvlan,
                                                  NodeConnector port)
        throws VTNException {
        long mac = mvlan.getMacAddress();
        byte[] addr = NetUtils.longToByteArray6(mac);
        EthernetAddress eaddr;
        try {
            eaddr = new EthernetAddress(addr);
        } catch (Exception e) {
            // This should never happen.
            throw new VTNException("Unable to create MAC address etnry", e);
        }

        return createMacAddressEntry(mgr, eaddr, mac, mvlan.getVlan(), port);
    }

    /**
     * Create a {@link MacAddressEntry} instance which represents the
     * specified host.
     *
     * @param mgr    VTN manager service.
     * @param eaddr  A {@link EthernetAddress} instance which specifies
     *               the MAC address.
     * @param mac    A long integer which represents the MAC address specified
     *               by {@code eaddr}.
     * @param vlan   VLAN ID.
     * @param port   A {@link NodeConnector} instance corresponding to a
     *               switch port to which the host is connected.
     * @return  A {@link MacAddressEntry} instance.
     * @throws VTNException  An error occurred.
     */
    private MacAddressEntry createMacAddressEntry(VTNManagerImpl mgr,
                                                  EthernetAddress eaddr,
                                                  long mac, short vlan,
                                                  NodeConnector port)
        throws VTNException {
        VBridgePath bpath = new VBridgePath(mapPath.getTenantName(),
                                            mapPath.getBridgeName());
        MacAddressTable table = mgr.getMacAddressTable(bpath);

        Set<InetAddress> ipaddrs;
        if (table == null) {
            ipaddrs = null;
        } else {
            // Retrieve IP addresses associated with the specified MAC address
            // in the MAC address table.
            ipaddrs = table.getInetAddresses(mac);
        }

        return new MacAddressEntry(eaddr, vlan, port, ipaddrs);
    }

    /**
     * Determine vBridge state from current MAC mapping state.
     *
     * @param mgr     VTN Manager service.
     * @param resMgr  Global resource manager service.
     * @param bstate  Current bridge state value.
     * @return  New bridge state value.
     */
    private VNodeState getBridgeState(VTNManagerImpl mgr,
                                      IVTNResourceManager resMgr,
                                      VNodeState bstate) {
        return getBridgeState(resMgr.hasMacMappedHost(mgr, mapPath), bstate);
    }

    /**
     * Determine vBridge state from current MAC mapping state.
     *
     * @param active  Specify {@code true} only if this MAC mapping maps
     *                at least one host.
     * @param bstate  Current bridge state value.
     * @return  New bridge state value.
     */
    private VNodeState getBridgeState(boolean active, VNodeState bstate) {
        if (!active) {
            return VNodeState.DOWN;
        }

        if (bstate == VNodeState.UNKNOWN) {
            return VNodeState.UP;
        }

        return bstate;
    }

    /**
     * Append human readable strings which describes the specified L2 host
     * and switch port to a {@link StringBuilder} instance.
     *
     * @param builder  A {@link StringBuilder} instance.
     * @param mvlan    A {@link MacVlan} instance which specifies the L2 host.
     * @param port     A {@link NodeConnector} instance corresponding to a
     *                 switch port.
     */
    private void appendDescription(StringBuilder builder, MacVlan mvlan,
                                   NodeConnector port) {
        builder.append("host={");
        mvlan.appendContents(builder);
        builder.append("}, port=").append(port.toString());
    }

    /**
     * Create a {@link StringBuilder} instance which contains a log record.
     *
     * @param mgr  VTN Manager service.
     * @param msg  A log message.
     * @return  A {@link StringBuilder} instance.
     */
    private StringBuilder createLog(VTNManagerImpl mgr, String msg) {
        StringBuilder builder = new StringBuilder(mgr.getContainerName());
        builder.append(':').append(mapPath).append(" :").append(msg);
        return builder;
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
        if (!(o instanceof MacMapImpl)) {
            return false;
        }

        MacMapImpl mmap = (MacMapImpl)o;
        if (!mapPath.equals(mmap.mapPath)) {
            return false;
        }

        return (allowedHosts.equals(mmap.allowedHosts) &&
                deniedHosts.equals(mmap.deniedHosts));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return mapPath.hashCode() ^ allowedHosts.hashCode() ^
            deniedHosts.hashCode();
    }

    // VBridgeNode

    /**
     * Return path to this interface.
     *
     * @return  Path to the interface.
     */
    @Override
    public MacMapPath getPath() {
        return mapPath;
    }

    /**
     * Determine whether this MAC mapping is administravely enabled or not.
     *
     * @return {@code true} is always returned because the MAC mapping can not
     *         be disabled.
     */
    @Override
    public boolean isEnabled() {
        return true;
    }

    // Cloneable

    /**
     * Return a clone of this instance.
     *
     * @return  A clone of this instance.
     */
    @Override
    public MacMapImpl clone() {
        try {
            MacMapImpl mmap = (MacMapImpl)super.clone();
            mmap.allowedHosts = new TreeSet<MacVlan>(allowedHosts);
            mmap.deniedHosts = new TreeSet<MacVlan>(deniedHosts);

            return mmap;
        } catch (CloneNotSupportedException e) {
            // This should never happen.
            throw new IllegalStateException("clone() failed", e);
        }
    }
}
