/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.NavigableSet;
import java.util.Objects;
import java.util.Set;
import java.util.TreeSet;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlElementWrapper;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.ProtocolUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.VtnMacMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.config.AllowedHosts;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.config.AllowedHostsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.config.DeniedHosts;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.config.DeniedHostsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.info.MacMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.info.MacMapConfigBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VlanHostDesc;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VlanHostDescSet;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnAclType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.vlan.host.desc.set.VlanHostDescList;

/**
 * {@code VTNMacMapConfig} describes configuration information about a
 * MAC mapping configured in a vBridge.
 */
@XmlRootElement(name = "vtn-mac-map-config")
@XmlAccessorType(XmlAccessType.NONE)
public final class VTNMacMapConfig {
    /**
     * A set of hosts to be mapped by MAC mapping.
     */
    @XmlElementWrapper(name = "allowed-hosts")
    @XmlElement(name = "host")
    private NavigableSet<MacVlan>  allowedHosts = new TreeSet<>();

    /**
     * A set of hosts not to be mapped by MAC mapping.
     */
    @XmlElementWrapper(name = "denied-hosts")
    @XmlElement(name = "host")
    private Set<MacVlan>  deniedHosts = new HashSet<>();

    /**
     * Create an exception that indicates the same MAC address is configured
     * in the allowed host set.
     *
     * @param mac  A MAC address.
     * @return  A {@link RpcException} instance.
     */
    public static RpcException getDuplicateMacAddressException(long mac) {
        String maddr = MiscUtils.formatMacAddress(mac);
        return RpcException.getBadArgumentException(
            "Duplicate MAC address in allowed set: " + maddr);
    }

    /**
     * Create an exception that indicates the specified MAC address is already
     * mapped to this vBridge.
     *
     * @param mac  A MAC address.
     * @return  A {@link RpcException} instance.
     */
    public static RpcException getAlreadyMappedException(long mac) {
        String maddr = MiscUtils.formatMacAddress(mac);
        return RpcException.getDataExistsException(
            "Already mapped to this vBridge: " + maddr);
    }

    /**
     * Verify a host information in the denied host set.
     *
     * @param host  A {@link VlanHostDesc} instance.
     * @return  A {@link MacVlan} instance converted from {@code host}.
     * @throws RpcException
     *    The given host information cannot be added to the denied host set.
     */
    public static MacVlan verifyDeniedHost(VlanHostDesc host)
        throws RpcException {
        return verifyDeniedHost(new MacVlan(host));
    }

    /**
     * Verify a host information in the denied host set.
     *
     * @param mv  A {@link MacVlan} instance.
     * @return  The given instance.
     * @throws RpcException
     *    The given host information cannot be added to the denied host set.
     */
    public static MacVlan verifyDeniedHost(MacVlan mv) throws RpcException {
        if (mv.getAddress() == MacVlan.UNDEFINED) {
            throw RpcException.getBadArgumentException(
                "MAC address cannot be null in denied host set: vlan=" +
                mv.getVlanId());
        }

        return mv;
    }

    /**
     * Determine whether the specified MAC address is contained in the given
     * set or not.
     *
     * @param set   A set of {@link MacVlan} instances.
     * @param addr  A MAC address to be tested.
     * @return  {@code true} is returned only if the MAC address specified
     *          by {@code addr} is contained in {@code set}.
     */
    public static boolean containsMacAddress(NavigableSet<MacVlan> set,
                                             long addr) {
        MacVlan key = new MacVlan(addr, (int)ProtocolUtils.MASK_VLAN_ID);
        MacVlan floor = set.floor(key);
        return (floor != null && floor.getAddress() == addr);
    }

    /**
     * A base class used to update the configuration of the MAC mapping.
     */
    private abstract class UpdateContext {
        /**
         * Update the MAC mapping configuration.
         *
         * @return  A {@link MacMapChange} instance that represents changes to
         *          the current configuration if changed.
         *          {@code null} if not changed.
         */
        protected final MacMapChange update() {
            MacMapChange change;
            if (hasNoChanges()) {
                change = null;
            } else {
                int flags = (isEmpty()) ? MacMapChange.REMOVING : 0;
                change = getMacMapChange(flags);
            }

            return change;
        }

        /**
         * Determine whether this instance contains host information to be
         * added or removed.
         *
         * @return  {@code true} if this instance does not contain host
         *          information to be added to or removed from the current
         *          configuration. {@code false} otherwise.
         */
        protected abstract boolean hasNoChanges();

        /**
         * Return a {@link MacMapChange} instance that represents changes to
         * the current MAC mapping configuration.
         *
         * @param flags  A bitwise OR-ed flags to be passed to
         *               {@link MacMapChange} instance.
         * @return  A {@link MacMapChange} instance.
         */
        protected abstract MacMapChange getMacMapChange(int flags);
    }

    /**
     * A base class used to update the contents of the access control list
     * of the MAC mapping.
     */
    private abstract class UpdateAclContext extends UpdateContext {
        /**
         * A set of {@link MacVlan} instances to be added to the host set.
         */
        private final Set<MacVlan>  addedSet = new HashSet<>();

        /**
         * A set of {@link MacVlan} instances to be removed from the host set.
         */
        private final Set<MacVlan>  removedSet = new HashSet<>();

        /**
         * Set up the context used to update the target access control list.
         *
         * @param op     A {@link VtnUpdateOperationType} instance which
         *               indicates how to update the MAC mapping
         *               configuration.
         * @param hosts  A list of {@link VlanHostDesc} instances.
         * @throws RpcException  An error occurred.
         */
        private void setUp(VtnUpdateOperationType op, List<VlanHostDesc> hosts)
            throws RpcException {
            if (op == VtnUpdateOperationType.SET) {
                // Change configuration as specified exactly.
                set(hosts);
            } else if (!MiscUtils.isEmpty(hosts)) {
                if (op == VtnUpdateOperationType.REMOVE) {
                    // Remove the specified configuration from the current
                    // configuration.
                    remove(hosts);
                } else {
                    // Add the specified configuration to the current
                    // configuration.
                    add(hosts);
                }
            }
        }

        /**
         * Return a set of {@link MacVlan} that represents host information
         * to be added.
         *
         * @return  A set of {@link MacVlan} to be added to the access control
         *          host set.
         */
        protected final Set<MacVlan> getAddedSet() {
            return addedSet;
        }

        /**
         * Return a set of {@link MacVlan} that represents host information
         * to be removed.
         *
         * @return  A set of {@link MacVlan} to be removed from the access
         *          control host set.
         */
        protected final Set<MacVlan> getRemovedSet() {
            return removedSet;
        }

        /**
         * Perform {@link VtnUpdateOperationType#SET} operation.
         *
         * @param hosts  A list of {@link VlanHostDesc} instances to be exactly
         *               set to the access control host set.
         * @throws RpcException  An error occurred.
         */
        protected final void set(List<VlanHostDesc> hosts)
            throws RpcException {
            Set<MacVlan> target = getTarget();
            if (MiscUtils.isEmpty(hosts)) {
                // Remove all hosts.
                removedSet.addAll(target);
                target.clear();
            } else {
                setImpl(hosts);
                target.removeAll(removedSet);
                target.addAll(addedSet);
            }
        }

        /**
         * Perform {@link VtnUpdateOperationType#ADD} operation.
         *
         * @param hosts  A list of {@link VlanHostDesc} instances to be added
         *               to the access control host set.
         * @throws RpcException  An error occurred.
         */
        protected final void add(List<VlanHostDesc> hosts)
            throws RpcException {
            if (hosts != null) {
                addImpl(hosts);
            }
        }

        /**
         * Perform {@link VtnUpdateOperationType#REMOVE} operation.
         *
         * @param hosts  A list of {@link VlanHostDesc} instances to be removed
         *               from the access control host set.
         * @throws RpcException  An error occurred.
         */
        protected final void remove(List<VlanHostDesc> hosts)
            throws RpcException {
            if (hosts != null) {
                Set<MacVlan> target = getTarget();
                Set<MacVlan> removed = getRemovedSet();
                for (VlanHostDesc host: hosts) {
                    MacVlan mvlan = new MacVlan(host);
                    if (target.remove(mvlan)) {
                        removed.add(mvlan);
                    }
                }
            }
        }

        /**
         * Return the target set of {@link MacVlan} instances.
         *
         * @return  The target set of {@link MacVlan}.
         */
        protected abstract Set<MacVlan> getTarget();

        /**
         * Perform {@link VtnUpdateOperationType#SET} operation.
         *
         * @param hosts  A list of {@link VlanHostDesc} instances to be exactly
         *               set to the access control host set.
         * @throws RpcException  An error occurred.
         */
        protected abstract void setImpl(List<VlanHostDesc> hosts)
            throws RpcException;

        /**
         * Perform {@link VtnUpdateOperationType#ADD} operation.
         *
         * @param hosts  A list of {@link VlanHostDesc} instances to be added
         *               to the access control host set.
         * @throws RpcException  An error occurred.
         */
        protected abstract void addImpl(List<VlanHostDesc> hosts)
            throws RpcException;

        // UpdateContext

        /**
         * {@inheritDoc}
         */
        @Override
        protected final boolean hasNoChanges() {
            return (addedSet.isEmpty() && removedSet.isEmpty());
        }
    }

    /**
     * An internal class used to update {@link #allowedHosts}.
     */
    private final class UpdateAllowedContext extends UpdateAclContext {
        // UpdateAclContext

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
        protected void setImpl(List<VlanHostDesc> hosts) throws RpcException {
            Set<MacVlan> added = getAddedSet();
            Set<MacVlan> retained = new HashSet<>();
            Set<Long> macSet = new HashSet<>();
            for (VlanHostDesc host: hosts) {
                MacVlan mvlan = new MacVlan(host);
                if (allowedHosts.contains(mvlan)) {
                    retained.add(mvlan);
                } else {
                    long mac = mvlan.getAddress();
                    if (!addMacAddress(macSet, mac)) {
                        throw getDuplicateMacAddressException(mac);
                    }
                    added.add(mvlan);
                }
            }

            Set<MacVlan> removed = getRemovedSet();
            for (MacVlan mvlan: allowedHosts) {
                if (retained.contains(mvlan)) {
                    long mac = mvlan.getAddress();
                    if (!addMacAddress(macSet, mac)) {
                        throw getAlreadyMappedException(mac);
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
        protected void addImpl(List<VlanHostDesc> hosts) throws RpcException {
            Set<MacVlan> added = getAddedSet();
            Set<Long> macSet = new HashSet<>();
            for (VlanHostDesc host: hosts) {
                MacVlan mvlan = new MacVlan(host);
                if (!allowedHosts.contains(mvlan)) {
                    long mac = mvlan.getAddress();
                    if (mac != MacVlan.UNDEFINED) {
                        if (!macSet.add(mac)) {
                            throw getDuplicateMacAddressException(mac);
                        }
                        if (containsMacAddress(allowedHosts, mac)) {
                            throw getAlreadyMappedException(mac);
                        }
                    }
                    added.add(mvlan);
                }
            }

            allowedHosts.addAll(added);
        }

        // UpdateContext

        /**
         * {@inheritDoc}
         */
        @Override
        protected MacMapChange getMacMapChange(int flags) {
            Set<MacVlan> empty = Collections.<MacVlan>emptySet();
            return new MacMapChange(
                getAddedSet(), getRemovedSet(), empty, empty, flags);
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
        protected void setImpl(List<VlanHostDesc> hosts) throws RpcException {
            Set<MacVlan> added = getAddedSet();
            Set<MacVlan> retained = new HashSet<>();
            for (VlanHostDesc host: hosts) {
                MacVlan mvlan = verifyDeniedHost(host);
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
        protected void addImpl(List<VlanHostDesc> hosts) throws RpcException {
            Set<MacVlan> added = getAddedSet();
            for (VlanHostDesc host: hosts) {
                MacVlan mvlan = verifyDeniedHost(host);
                if (deniedHosts.add(mvlan)) {
                    added.add(mvlan);
                }
            }
        }

        // UpdateContext

        /**
         * {@inheritDoc}
         */
        @Override
        protected MacMapChange getMacMapChange(int flags) {
            Set<MacVlan> empty = Collections.<MacVlan>emptySet();
            return new MacMapChange(
                empty, empty, getAddedSet(), getRemovedSet(), flags);
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
         * @param op       A {@link VtnUpdateOperationType} instance which
         *                 indicates how to update the MAC mapping
         *                 configuration.
         * @param allowed  A list of {@link VlanHostDesc} instances that
         *                 indicates the changes made to the allowed host set.
         * @param denied   A list of {@link VlanHostDesc} instances that
         *                 indicates the changes made to the denied host set.
         * @throws RpcException  An error occurred.
         */
        private UpdateBothContext(
            VtnUpdateOperationType op, List<VlanHostDesc> allowed,
            List<VlanHostDesc> denied) throws RpcException {
            if (op == VtnUpdateOperationType.SET) {
                // Update configuration as specified exactly.
                allowedContext.set(allowed);
                deniedContext.set(denied);
            } else {
                if (op == VtnUpdateOperationType.REMOVE) {
                    // Remove the specified configuration from the current
                    // configuration.
                    allowedContext.remove(allowed);
                    deniedContext.remove(denied);
                } else {
                    // Add the specified configuration to the current
                    // configuration.
                    allowedContext.add(allowed);
                    deniedContext.add(denied);
                }
            }
        }

        // UpdateContext

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean hasNoChanges() {
            return (allowedContext.hasNoChanges() &&
                    deniedContext.hasNoChanges());
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected MacMapChange getMacMapChange(int flags) {
            Set<MacVlan> allowAdded = allowedContext.getAddedSet();
            Set<MacVlan> allowRemoved = allowedContext.getRemovedSet();
            Set<MacVlan> denyAdded = deniedContext.getAddedSet();
            Set<MacVlan> denyRemoved = deniedContext.getRemovedSet();
            return new MacMapChange(
                allowAdded, allowRemoved, denyAdded, denyRemoved, flags);
        }
    }

    /**
     * Construct an empty MAC mapping configuration.
     */
    public VTNMacMapConfig() {
    }

    /**
     * Construct a new instance.
     *
     * @param vmmc  A {@link VtnMacMapConfig} instance.
     * @throws RpcException
     *    {@code vmmc} contains invalid configuration.
     */
    public VTNMacMapConfig(VtnMacMapConfig vmmc) throws RpcException {
        setAcl(allowedHosts, vmmc.getAllowedHosts());
        setAcl(deniedHosts, vmmc.getDeniedHosts());
    }

    /**
     * Return a set of hosts to be mapped by MAC mapping.
     *
     * @return  A set of hosts to be mapped by MAC mapping.
     */
    public Set<MacVlan> getAllowedHosts() {
        return Collections.unmodifiableSet(allowedHosts);
    }

    /**
     * Return a set of hosts not to be mapped by MAC mapping.
     *
     * @return  A set of hosts not to be mapped by MAC mapping.
     */
    public Set<MacVlan> getDeniedHosts() {
        return Collections.unmodifiableSet(deniedHosts);
    }

    /**
     * Convert this instance into a {@link MacMapConfig} instance.
     *
     * @return  A {@link MacMapConfig} instance.
     */
    public MacMapConfig toMacMapConfig() {
        MacMapConfigBuilder builder = new MacMapConfigBuilder();
        List<VlanHostDescList> hosts = toVlanHostDescList(allowedHosts);
        if (hosts != null) {
            AllowedHosts allowed = new AllowedHostsBuilder().
                setVlanHostDescList(hosts).
                build();
            builder.setAllowedHosts(allowed);
        }

        hosts = toVlanHostDescList(deniedHosts);
        if (hosts != null) {
            DeniedHosts denied = new DeniedHostsBuilder().
                setVlanHostDescList(hosts).
                build();
            builder.setDeniedHosts(denied);
        }

        return builder.build();
    }

    /**
     * Verify the contents of this instance.
     *
     * @throws RpcException  Verification failed.
     */
    public void verify() throws RpcException {
        Set<Long> macSet = new HashSet<>();
        for (MacVlan mv: allowedHosts) {
            mv.checkMacMap();
            long mac = mv.getAddress();
            if (!addMacAddress(macSet, mac)) {
                throw getDuplicateMacAddressException(mac);
            }
        }

        for (MacVlan mv: deniedHosts) {
            mv.checkMacMap();
            verifyDeniedHost(mv);
        }
    }

    /**
     * Determine whether the MAC mapping configuration is empty or not.
     *
     * @return  {@code true} only if this MAC mapping configuration is empty.
     */
    public boolean isEmpty() {
        return (allowedHosts.isEmpty() && deniedHosts.isEmpty());
    }

    /**
     * Update the MAC mapping configuration.
     *
     * <p>
     *   This method updates both the allowed and denied host set.
     *   Note that the caller must discard this instance if this method throws
     *   an exception.
     * </p>
     *
     * @param op       A {@link VtnUpdateOperationType} instance which
     *                 indicates how to update the MAC mapping
     *                 configuration.
     * @param allowed  A list of {@link VlanHostDesc} instances that
     *                 indicates the changes made to the allowed host set.
     * @param denied   A list of {@link VlanHostDesc} instances that
     *                 indicates the changes made to the denied host set.
     * @return  A {@link MacMapChange} instance if the configuration has been
     *          changed. {@code null} if not changed.
     * @throws RpcException  An error occurred.
     */
    public MacMapChange update(
        VtnUpdateOperationType op, List<VlanHostDesc> allowed,
        List<VlanHostDesc> denied) throws RpcException {
        UpdateBothContext context = new UpdateBothContext(op, allowed, denied);
        return context.update();
    }

    /**
     * Update the specified access control list.
     *
     * <p>
     *   Note that the caller must discard this instance if this method throws
     *   an exception.
     * </p>
     *
     * @param op       A {@link VtnUpdateOperationType} instance which
     *                 indicates how to change the MAC mapping configuration.
     * @param aclType  The type of access control list.
     * @param hosts    A list of {@link VlanHostDesc} instances.
     * @return  A {@link MacMapChange} instance if the configuration has been
     *          changed. {@code null} if not changed.
     * @throws RpcException  An error occurred.
     */
    public MacMapChange update(VtnUpdateOperationType op, VtnAclType aclType,
                               List<VlanHostDesc> hosts) throws RpcException {
        UpdateAclContext context = (aclType == VtnAclType.DENY)
            ? new UpdateDeniedContext()
            : new UpdateAllowedContext();

        context.setUp(op, hosts);
        return context.update();
    }

    /**
     * Set host information into the given access control list.
     *
     * @param acl     The target access control list.
     * @param vhdset  A list of host information to be set.
     * @throws RpcException
     *    {@code vhdset} contains invalid configuration.
     */
    private void setAcl(Set<MacVlan> acl, VlanHostDescSet vhdset)
        throws RpcException {
        if (vhdset != null) {
            List<VlanHostDescList> vhdescs = vhdset.getVlanHostDescList();
            if (vhdescs != null) {
                for (VlanHostDescList vhdesc: vhdescs) {
                    if (vhdesc == null) {
                        throw RpcException.getNullArgumentException(
                            "vlan-host-desc-list");
                    }
                    acl.add(new MacVlan(vhdesc.getHost()));
                }
            }
        }
    }

    /**
     * Convert the given access control list into a list of
     * {@link VlanHostDescList} instances.
     *
     * @param acl  The target access control list.
     * @return  A list of {@link VlanHostDescList} instances.
     *          {@code null} if the given access control list is empty.
     */
    private List<VlanHostDescList> toVlanHostDescList(Set<MacVlan> acl) {
        List<VlanHostDescList> list;
        if (acl.isEmpty()) {
            list = null;
        } else {
            list = new ArrayList<>();
            for (MacVlan mv: acl) {
                list.add(mv.getVlanHostDescList());
            }
        }

        return list;
    }

    /**
     * Add the specified MAC address into the specified set of MAC addresses.
     *
     * @param macSet  A set of MAC addresses.
     * @param mac     A long integer value that indicates the MAC address.
     *                Note that {@code MacVlan#UNDEFINED} is never be added
     *                to {@code macSet}.
     * @return  {@code true} if {@code mac} was added to {@code macSet} or
     *          {@code mac} is equal to {@link MacVlan#UNDEFINED}.
     *          {@code false} otherwise.
     */
    private boolean addMacAddress(Set<Long> macSet, long mac) {
        return (mac == MacVlan.UNDEFINED || macSet.add(mac));
    }

    // Object

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        boolean ret = (o == this);
        if (!ret && o != null && getClass().equals(o.getClass())) {
            VTNMacMapConfig mmc = (VTNMacMapConfig)o;
            ret = (allowedHosts.equals(mmc.allowedHosts) &&
                   deniedHosts.equals(mmc.deniedHosts));
        }

        return ret;
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(getClass(), allowedHosts, deniedHosts);
    }
}
