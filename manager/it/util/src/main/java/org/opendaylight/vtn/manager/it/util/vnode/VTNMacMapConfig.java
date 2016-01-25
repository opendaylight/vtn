/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.vnode;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import static org.opendaylight.vtn.manager.it.util.ModelDrivenTestBase.getRpcOutput;
import static org.opendaylight.vtn.manager.it.util.ModelDrivenTestBase.getRpcResult;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.opendaylight.vtn.manager.it.util.VTNServices;
import org.opendaylight.vtn.manager.it.util.inventory.MacVlan;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.SetMacMapAclInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.SetMacMapAclInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.SetMacMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.SetMacMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.VtnMacMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.info.MacMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.mappable.MacMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VlanHostDesc;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VlanHostDescSet;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnAclType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.vlan.host.desc.set.VlanHostDescList;

/**
 * {@code VTNMacMapConfig} describes the configuration of a MAC mapping.
 */
public final class VTNMacMapConfig implements Cloneable {
    /**
     * A set of hosts to be mapped by MAC mapping.
     */
    private Set<MacVlan>  allowedHosts;

    /**
     * A set of hosts not to be mapped by MAC mapping.
     */
    private Set<MacVlan>  deniedHosts;

    /**
     * The expected status of the MAC mapping.
     */
    private VTNMacMapStatus  mapStatus;

    /**
     * Remove the MAC mapping from the specified vBridge.
     *
     * @param service  The vtn-mac-map RPC service.
     * @param ident    The identifier for the vBridge.
     * @return  A {@link VtnUpdateType} instance.
     */
    public static VtnUpdateType removeMacMap(
        VtnMacMapService service, VBridgeIdentifier ident) {
        SetMacMapInput input = new SetMacMapInputBuilder().
            setTenantName(ident.getTenantNameString()).
            setBridgeName(ident.getBridgeNameString()).
            setOperation(VtnUpdateOperationType.SET).
            build();
        return getRpcResult(service.setMacMap(input));
    }

    /**
     * Remove the specified access control list in the MAC mapping configured
     * in the specified vBridge.
     *
     * @param service  The vtn-mac-map RPC service.
     * @param ident    The identifier for the vBridge.
     * @param aclType  The type of the access control list.
     * @return  A {@link VtnUpdateType} instance.
     */
    public static VtnUpdateType removeMacMapAcl(
        VtnMacMapService service, VBridgeIdentifier ident,
        VtnAclType aclType) {
        SetMacMapAclInput input = new SetMacMapAclInputBuilder().
            setTenantName(ident.getTenantNameString()).
            setBridgeName(ident.getBridgeNameString()).
            setOperation(VtnUpdateOperationType.SET).
            setAclType(aclType).
            build();
        return getRpcResult(service.setMacMapAcl(input));
    }

    /**
     * Convert the given host set into a list of {@link VlanHostDesc}
     * instances.
     *
     * @param set    A set of {@link MacVlan} instances.
     * @param empty  If {@code true}, an empty list is returned if the given
     *               set is {@code null} or empty.
     * @return  A list of {@link VlanHostDesc} instances.
     */
    public static List<VlanHostDesc> toVlanHostDescs(Set<MacVlan> set,
                                                     boolean empty) {
        List<VlanHostDesc> list = new ArrayList<>();
        if (set != null) {
            for (MacVlan mv: set) {
                list.add(mv.getVlanHostDesc());
            }
        }
        if (!empty && list.isEmpty()) {
            list = null;
        }

        return list;
    }

    /**
     * Construct a new empty configuration.
     */
    public VTNMacMapConfig() {
        allowedHosts = new HashSet<>();
        deniedHosts = new HashSet<>();
        mapStatus = new VTNMacMapStatus();
    }

    /**
     * Set the given hosts to the allowed host set.
     *
     * @param hosts  A set of hosts to be set to the allowed host set.
     * @return  This instance.
     */
    public VTNMacMapConfig setAllowed(Set<MacVlan> hosts) {
        allowedHosts.clear();
        if (hosts != null) {
            allowedHosts.addAll(hosts);
        }
        return this;
    }

    /**
     * Add the given hosts to the allowed host set.
     *
     * @param hosts  Hosts to be added to the allowed host set.
     * @return  This instance.
     */
    public VTNMacMapConfig addAllowed(MacVlan ... hosts) {
        for (MacVlan host: hosts) {
            allowedHosts.add(host);
        }
        return this;
    }

    /**
     * Remove the given hosts from the allowed host set.
     *
     * @param hosts  Hosts to be removed from the allowed host set.
     * @return  This instance.
     */
    public VTNMacMapConfig removeAllowed(MacVlan ... hosts) {
        for (MacVlan host: hosts) {
            allowedHosts.remove(host);
        }
        return this;
    }

    /**
     * Clear the allowed host set.
     *
     * @return  This instance.
     */
    public VTNMacMapConfig clearAllowed() {
        allowedHosts.clear();
        return this;
    }

    /**
     * Return an unmodifiable set of hosts to be mapped by MAC mapping.
     *
     * @return  An unmodifiable set of hosts to be mapped by MAC mapping.
     */
    public Set<MacVlan> getAllowed() {
        return Collections.unmodifiableSet(allowedHosts);
    }

    /**
     * Set the given hosts to the denied host set.
     *
     * @param hosts  A set of hosts to be set to the denied host set.
     * @return  This instance.
     */
    public VTNMacMapConfig setDenied(Set<MacVlan> hosts) {
        deniedHosts.clear();
        if (hosts != null) {
            deniedHosts.addAll(hosts);
        }
        return this;
    }

    /**
     * Add the given hosts to the denied host set.
     *
     * @param hosts  Hosts to be added to the denied host set.
     * @return  This instance.
     */
    public VTNMacMapConfig addDenied(MacVlan ... hosts) {
        for (MacVlan host: hosts) {
            deniedHosts.add(host);
        }
        return this;
    }

    /**
     * Remove the given hosts from the denied host set.
     *
     * @param hosts  Hosts to be removed from the denied host set.
     * @return  This instance.
     */
    public VTNMacMapConfig removeDenied(MacVlan ... hosts) {
        for (MacVlan host: hosts) {
            deniedHosts.remove(host);
        }
        return this;
    }

    /**
     * Clear the denied host set.
     *
     * @return  This instance.
     */
    public VTNMacMapConfig clearDenied() {
        deniedHosts.clear();
        return this;
    }

    /**
     * Return an unmodifiable set of hosts to be mapped by MAC mapping.
     *
     * @return  An unmodifiable set of hosts to be mapped by MAC mapping.
     */
    public Set<MacVlan> getDenied() {
        return Collections.unmodifiableSet(deniedHosts);
    }

    /**
     * Clear the specified access control list.
     *
     * @param aclType  The type of the access control list to be cleared.
     * @return  This instance.
     */
    public VTNMacMapConfig clear(VtnAclType aclType) {
        if (aclType == VtnAclType.ALLOW) {
            allowedHosts.clear();
        } else if (aclType == VtnAclType.DENY) {
            deniedHosts.clear();
        }
        return this;
    }

    /**
     * Determine whether this MAC mapping configuration is empty or not.
     *
     * @return  {@code true} if this MAC mapping configuration is empty.
     *          {@code false} otherwise.
     */
    public boolean isEmpty() {
        return (allowedHosts.isEmpty() && deniedHosts.isEmpty());
    }

    /**
     * Return the status of the MAC mapping.
     *
     * @return  A {@link VTNMacMapStatus} instance.
     */
    public VTNMacMapStatus getStatus() {
        return mapStatus;
    }

    /**
     * Create a new input builder for set-mac-map RPC.
     *
     * @return  An {@link SetMacMapInputBuilder} instance.
     */
    public SetMacMapInputBuilder newInputBuilder() {
        return newInputBuilder(false);
    }

    /**
     * Create a new input builder for set-mac-map RPC.
     *
     * @param empty  If {@code true}, an empty host list is set if the host
     *               set is empty.
     * @return  An {@link SetMacMapInputBuilder} instance.
     */
    public SetMacMapInputBuilder newInputBuilder(boolean empty) {
        List<VlanHostDesc> allowed = toVlanHostDescs(allowedHosts, empty);
        List<VlanHostDesc> denied = toVlanHostDescs(deniedHosts, empty);

        return new SetMacMapInputBuilder().
            setAllowedHosts(allowed).
            setDeniedHosts(denied);
    }

    /**
     * Create a new input builder for set-mac-map RPC.
     *
     * @param ident  The identifier for the vBridge.
     * @return  An {@link SetMacMapInputBuilder} instance.
     */
    public SetMacMapInputBuilder newInputBuilder(VBridgeIdentifier ident) {
        return newInputBuilder(ident, false);
    }

    /**
     * Create a new input builder for set-mac-map RPC.
     *
     * @param ident  The identifier for the vBridge.
     * @param empty  If {@code true}, an empty host list is set if the host
     *               set is empty.
     * @return  An {@link SetMacMapInputBuilder} instance.
     */
    public SetMacMapInputBuilder newInputBuilder(VBridgeIdentifier ident,
                                                 boolean empty) {
        return newInputBuilder(empty).
            setTenantName(ident.getTenantNameString()).
            setBridgeName(ident.getBridgeNameString());
    }

    /**
     * Create a new input builder for set-mac-map-acl RPC.
     *
     * @param aclType  A {@link VtnAclType} instance that specifies the access
     *                 control list.
     * @return  A {@link SetMacMapAclInputBuilder} instance.
     */
    public SetMacMapAclInputBuilder newAclInputBuilder(VtnAclType aclType) {
        return newAclInputBuilder(aclType, false);
    }

    /**
     * Create a new input builder for set-mac-map-acl RPC.
     *
     * @param aclType  A {@link VtnAclType} instance that specifies the access
     *                 control list.
     * @param empty    If {@code true}, an empty host list is set if the host
     *                 set is empty.
     * @return  A {@link SetMacMapAclInputBuilder} instance.
     */
    public SetMacMapAclInputBuilder newAclInputBuilder(VtnAclType aclType,
                                                       boolean empty) {
        List<VlanHostDesc> hosts = (aclType == VtnAclType.DENY)
            ? toVlanHostDescs(deniedHosts, empty)
            : toVlanHostDescs(allowedHosts, empty);

        return new SetMacMapAclInputBuilder().
            setAclType(aclType).
            setHosts(hosts);
    }

    /**
     * Create a new input builder for set-mac-map-acl RPC.
     *
     * @param ident    The identifier for the vBridge.
     * @param aclType  A {@link VtnAclType} instance that specifies the access
     *                 control list.
     * @return  A {@link SetMacMapAclInputBuilder} instance.
     */
    public SetMacMapAclInputBuilder newAclInputBuilder(
        VBridgeIdentifier ident, VtnAclType aclType) {
        return newAclInputBuilder(ident, aclType, false);
    }

    /**
     * Create a new input builder for set-mac-map-acl RPC.
     *
     * @param ident    The identifier for the vBridge.
     * @param aclType  A {@link VtnAclType} instance that specifies the access
     *                 control list.
     * @param empty    If {@code true}, an empty host list is set if the host
     *                 set is empty.
     * @return  A {@link SetMacMapAclInputBuilder} instance.
     */
    public SetMacMapAclInputBuilder newAclInputBuilder(
        VBridgeIdentifier ident, VtnAclType aclType, boolean empty) {
        return newAclInputBuilder(aclType, empty).
            setTenantName(ident.getTenantNameString()).
            setBridgeName(ident.getBridgeNameString());
    }

    /**
     * Update the specified MAC mapping.
     *
     * @param service  The vtn-mac-map service.
     * @param ident    The identifier for the vBridge.
     * @param op       A {@link VtnUpdateOperationType} instance.
     * @return  A {@link VtnUpdateType} instance returned by the RPC.
     */
    public VtnUpdateType update(VtnMacMapService service,
                                VBridgeIdentifier ident,
                                VtnUpdateOperationType op) {
        SetMacMapInput input = newInputBuilder(ident).
            setOperation(op).
            build();
        return getRpcResult(service.setMacMap(input));
    }

    /**
     * Update the specified access control list for the MAC mapping.
     *
     * @param service  The vtn-mac-map service.
     * @param ident    The identifier for the vBridge.
     * @param aclType  A {@link VtnAclType} instance that specifies the access
     * @param op       A {@link VtnUpdateOperationType} instance.
     *                 control list.
     * @return  A {@link VtnUpdateType} instance returned by the RPC.
     */
    public VtnUpdateType update(VtnMacMapService service,
                                VBridgeIdentifier ident, VtnAclType aclType,
                                VtnUpdateOperationType op) {
        SetMacMapAclInput input = newAclInputBuilder(ident, aclType).
            setOperation(op).
            build();
        return getRpcResult(service.setMacMapAcl(input));
    }

    /**
     * Verify the given MAC mapping.
     *
     * @param mmap  The MAC mapping to be verified.
     * @return  A {@link VnodeState} instance that indicates the stauts of the
     *          VLAN mapping.
     */
    public VnodeState verify(MacMap mmap) {
        // Verify the configuration.
        MacMapConfig mmc = mmap.getMacMapConfig();
        verifyAcl(mmc.getAllowedHosts(), allowedHosts);
        verifyAcl(mmc.getDeniedHosts(), deniedHosts);

        // Verify the status.
        return mapStatus.verify(mmap.getMacMapStatus());
    }

    /**
     * Apply all the configurations in this instance.
     *
     * @param service  A {@link VTNServices} instance.
     * @param ident    The identifier for the vBridge.
     */
    public void apply(VTNServices service, VBridgeIdentifier ident) {
        SetMacMapInput input = newInputBuilder().
            setTenantName(ident.getTenantNameString()).
            setBridgeName(ident.getBridgeNameString()).
            setOperation(VtnUpdateOperationType.SET).
            build();
        getRpcOutput(service.getMacMapService().setMacMap(input));
    }

    /**
     * Verify the given access control list.
     *
     * @param acl    The access control list in the MAC mapping configuration.
     * @param hosts  A set of host information expected to be present in the
     *               given access control list.
     */
    private void verifyAcl(VlanHostDescSet acl, Set<MacVlan> hosts) {
        if (hosts.isEmpty()) {
            assertEquals(null, acl);
        } else {
            assertNotNull(acl);
            List<VlanHostDescList> list = acl.getVlanHostDescList();
            assertNotNull(list);

            Set<MacVlan> set = new HashSet<>();
            for (VlanHostDescList vhdl: list) {
                assertEquals(true, set.add(new MacVlan(vhdl.getHost())));
            }
            assertEquals(hosts, set);
        }
    }

    // Object

    /**
     * Create a shallow copy of this instance.
     *
     * @return  A shallow copy of this instance.
     */
    @Override
    public VTNMacMapConfig clone() {
        try {
            VTNMacMapConfig mmap = (VTNMacMapConfig)super.clone();
            mmap.allowedHosts = new HashSet<>(allowedHosts);
            mmap.deniedHosts = new HashSet<>(deniedHosts);
            mmap.mapStatus = mapStatus.clone();
            return mmap;
        } catch (CloneNotSupportedException e) {
            // This should never happen.
            throw new IllegalStateException("clone() failed", e);
        }
    }
}
