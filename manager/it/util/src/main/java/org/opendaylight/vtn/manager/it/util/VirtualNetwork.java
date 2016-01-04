/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Set;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.it.ofmock.DataStoreUtils;
import org.opendaylight.vtn.manager.it.util.flow.cond.FlowCondSet;
import org.opendaylight.vtn.manager.it.util.flow.filter.FlowFilterList;
import org.opendaylight.vtn.manager.it.util.pathmap.PathMapSet;
import org.opendaylight.vtn.manager.it.util.pathpolicy.PathPolicySet;
import org.opendaylight.vtn.manager.it.util.vnode.BridgeConfig;
import org.opendaylight.vtn.manager.it.util.vnode.FlowFilterNode;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VInterfaceConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VInterfaceIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VNodeIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VNodeType;
import org.opendaylight.vtn.manager.it.util.vnode.VTenantConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VTerminalConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VTerminalIdentifier;

import org.opendaylight.controller.md.sal.binding.api.ReadOnlyTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.GlobalPathMaps;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.VtnService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.Vtns;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.MacTables;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.mac.tables.TenantMacTable;

/**
 * {@code VirtualNetwork} describes the virtual network configuration.
 */
public final class VirtualNetwork {
    /**
     * VTN service provider.
     */
    private final VTNServices  vtnServices;

    /**
     * A map that keeps VTN configurations.
     */
    private final Map<String, VTenantConfig>  vTenants = new HashMap<>();

    /**
     * A set of path policies.
     */
    private final PathPolicySet  pathPolicies = new PathPolicySet();

    /**
     * A set of global path maps.
     */
    private final PathMapSet  pathMaps = new PathMapSet();

    /**
     * A set of flow conditions.
     */
    private final FlowCondSet  flowConditions = new FlowCondSet();

    /**
     * Construct a new instance.
     *
     * @param service  A {@link VTNServices} instance.
     */
    public VirtualNetwork(VTNServices service) {
        vtnServices = service;
    }

    /**
     * Add the given VTN configuration.
     *
     * @param name   The name of the VTN.
     * @param tconf  The VTN configuration.
     * @return  This instance.
     */
    public VirtualNetwork addTenant(String name, VTenantConfig tconf) {
        vTenants.put(name, tconf);
        return this;
    }

    /**
     * Remove the VTN configuration specified by the name.
     *
     * @param name   The name of the VTN.
     * @return  This instance.
     */
    public VirtualNetwork removeTenant(String name) {
        vTenants.remove(name);
        return this;
    }

    /**
     * Return the VTN configuration specified by the name.
     *
     * @param name   The name of the VTN.
     * @return  A {@link VTenantConfig} instance if found.
     *          {@code null} if not found.
     */
    public VTenantConfig getTenant(String name) {
        return vTenants.get(name);
    }

    /**
     * Return an unmodifiable map that contains all VTNs.
     *
     * @return  An unmodifiable map that contains all VTNs.
     */
    public Map<String, VTenantConfig> getTenants() {
        return Collections.unmodifiableMap(vTenants);
    }

    /**
     * Add vBridge configurations with specifying default parameter.
     *
     * <p>
     *   Parent VTNs will be created if not present.
     * </p>
     *
     * @param ids  An array of vBridge identifiers.
     * @return  This instance.
     */
    public VirtualNetwork addBridge(VBridgeIdentifier ... ids) {
        for (VBridgeIdentifier ident: ids) {
            String tname = ident.getTenantNameString();
            String bname = ident.getBridgeNameString();
            VTenantConfig tconf = vTenants.get(tname);
            VBridgeConfig bconf;
            if (tconf == null) {
                tconf = new VTenantConfig();
                vTenants.put(tname, tconf);
                bconf = null;
            } else {
                bconf = tconf.getBridge(bname);
            }

            if (bconf == null) {
                tconf.addBridge(bname, new VBridgeConfig());
            }
        }

        return this;
    }

    /**
     * Remove vBridge configurations specified by the given identifier.
     *
     * @param ids  An array of vBridge identifiers.
     * @return  This instance.
     */
    public VirtualNetwork removeBridge(VBridgeIdentifier ... ids) {
        for (VBridgeIdentifier ident: ids) {
            String tname = ident.getTenantNameString();
            VTenantConfig tconf = vTenants.get(tname);
            if (tconf != null) {
                tconf.removeBridge(ident.getBridgeNameString());
            }
        }

        return this;
    }

    /**
     * Return the vBridge configuration specified by the given identifier.
     *
     * @param ident  The identifier for the vBridge.
     * @return  A {@link VBridgeConfig} instance if fonud.
     *          {@code null} if not found.
     */
    public VBridgeConfig getBridge(VBridgeIdentifier ident) {
        VTenantConfig tconf = vTenants.get(ident.getTenantNameString());
        return (tconf == null)
            ? null : tconf.getBridge(ident.getBridgeNameString());
    }

    /**
     * Add vTerminal configurations with specifying default parameter.
     *
     * <p>
     *   Parent VTNs will be created if not present.
     * </p>
     *
     * @param ids  An array of vTerminal identifiers.
     * @return  This instance.
     */
    public VirtualNetwork addTerminal(VTerminalIdentifier ... ids) {
        for (VTerminalIdentifier ident: ids) {
            String tname = ident.getTenantNameString();
            String bname = ident.getBridgeNameString();
            VTenantConfig tconf = vTenants.get(tname);
            VTerminalConfig vtconf;
            if (tconf == null) {
                tconf = new VTenantConfig();
                vTenants.put(tname, tconf);
                vtconf = null;
            } else {
                vtconf = tconf.getTerminal(bname);
            }

            if (vtconf == null) {
                tconf.addTerminal(bname, new VTerminalConfig());
            }
        }

        return this;
    }

    /**
     * Remove vTerminal configurations specified by the given identifier.
     *
     * @param ids  An array of vTerminal identifiers.
     * @return  This instance.
     */
    public VirtualNetwork removeTerminal(VTerminalIdentifier ... ids) {
        for (VTerminalIdentifier ident: ids) {
            String tname = ident.getTenantNameString();
            VTenantConfig tconf = vTenants.get(tname);
            if (tconf != null) {
                tconf.removeTerminal(ident.getBridgeNameString());
            }
        }

        return this;
    }

    /**
     * Return the vTerminal configuration specified by the given identifier.
     *
     * @param ident  The identifier for the vTerminal.
     * @return  A {@link VTerminalConfig} instance if fonud.
     *          {@code null} if not found.
     */
    public VTerminalConfig getTerminal(VTerminalIdentifier ident) {
        VTenantConfig tconf = vTenants.get(ident.getTenantNameString());
        return (tconf == null)
            ? null : tconf.getTerminal(ident.getBridgeNameString());
    }

    /**
     * Add virtual interface configurations with specifying default parameter.
     *
     * <p>
     *   Parent virtual nodes will be added if not present.
     * </p>
     *
     * @param ids  An array of virtual interface identifiers.
     * @return  This instance.
     */
    public VirtualNetwork addInterface(VInterfaceIdentifier<?> ... ids) {
        for (VInterfaceIdentifier<?> ident: ids) {
            BridgeConfig<?, ?> vbconf = prepareVirtualBridge(ident);
            String iname = ident.getInterfaceNameString();
            VInterfaceConfig iconf = vbconf.getInterface(iname);
            if (iconf == null) {
                vbconf.addInterface(iname, new VInterfaceConfig());
            }
        }

        return this;
    }

    /**
     * Remove virtual interface configurations specified by the given
     * identifier.
     *
     * @param ids  An array of virtual interface identifiers.
     * @return  This instance.
     */
    public VirtualNetwork removeInterface(VInterfaceIdentifier<?> ... ids) {
        for (VInterfaceIdentifier<?> ident: ids) {
            BridgeConfig<?, ?> vbconf = getVirtualBridge(ident);
            if (vbconf != null) {
                vbconf.removeInterface(ident.getInterfaceNameString());
            }
        }

        return this;
    }

    /**
     * Return the virtual interface configuration specified by the given
     * identifier.
     *
     * @param ident  The identifier for the virtual interface.
     * @return  A {@link VInterfaceConfig} instance if found.
     *          {@code null} if not found.
     */
    public VInterfaceConfig getInterface(VInterfaceIdentifier<?> ident) {
        BridgeConfig<?, ?> vbconf = getVirtualBridge(ident);
        return (vbconf == null)
            ? null
            : vbconf.getInterface(ident.getInterfaceNameString());
    }

    /**
     * Return the path policy configuration.
     *
     * @return  A {@link PathPolicySet} instance.
     */
    public PathPolicySet getPathPolicies() {
        return pathPolicies;
    }

    /**
     * Return the global path map configuration.
     *
     * @return  A {@link PathMapSet} instance.
     */
    public PathMapSet getPathMaps() {
        return pathMaps;
    }

    /**
     * Return the flow condition configuration.
     *
     * @return  A {@link FlowCondSet} instance.
     */
    public FlowCondSet getFlowConditions() {
        return flowConditions;
    }

    /**
     * Return the virtual node configuration that contains flow filters.
     *
     * @param ident   The identifier for the target virtual node.
     * @return  A {@link FlowFilterNode} instance if found.
     *          {@code null} if not found.
     */
    public FlowFilterNode getFlowFilterNode(VNodeIdentifier<?> ident) {
        VNodeType type = ident.getType();
        FlowFilterNode fnode;
        if (type == VNodeType.VTN) {
            fnode = getTenant(ident.getTenantNameString());
        } else if (type == VNodeType.VBRIDGE) {
            @SuppressWarnings("unchecked")
            VBridgeIdentifier vbrId = (VBridgeIdentifier)ident;
            fnode = getBridge(vbrId);
        } else if (type.isInterface()) {
            @SuppressWarnings("unchecked")
            VInterfaceIdentifier<?> ifId = (VInterfaceIdentifier<?>)ident;
            fnode = getInterface(ifId);
        } else {
            fnode = null;
        }

        return fnode;
    }

    /**
     * Return the specified flow filter list.
     *
     * @param ident   The identifier for the target virtual node.
     * @param output  {@code true} indicates the flow filter for outgoing
     *                packets.
     * @return  A {@link FlowFilterList} instance if found.
     *          {@code null} if not found.
     */
    public FlowFilterList getFlowFilterList(VNodeIdentifier<?> ident,
                                            boolean output) {
        FlowFilterNode fnode = getFlowFilterNode(ident);
        FlowFilterList flist;
        if (fnode == null) {
            flist = null;
        } else if (output) {
            flist = fnode.getOutputFilter();
        } else {
            flist = fnode.getInputFilter();
        }

        return flist;
    }

    /**
     * Verify the virtual network configuration.
     *
     * @return  This instance.
     */
    public VirtualNetwork verify() {
        try (ReadOnlyTransaction rtx = vtnServices.newReadOnlyTransaction()) {
            return verify(rtx);
        }
    }

    /**
     * Verify the virtual network configuration.
     *
     * @param rtx  A read-only MD-SAL datastore transaction.
     * @return  This instance.
     */
    public VirtualNetwork verify(ReadTransaction rtx) {
        // Verify the VTN configuration.
        verifyVtn(rtx);

        // Verify the flow conditions.
        flowConditions.verify(rtx);

        // Verify the path policies.
        pathPolicies.verify(rtx);

        // Verify the global path map configuration.
        verifyPathMap(rtx);

        return this;
    }

    /**
     * Apply all the configurations in this instance.
     *
     * @return  This instance.
     */
    public VirtualNetwork apply() {
        try (ReadOnlyTransaction rtx = vtnServices.newReadOnlyTransaction()) {
            return apply(rtx);
        }
    }

    /**
     * Apply all the configurations in this instance.
     *
     * @param rtx  A read-only MD-SAL datastore transaction.
     * @return  This instance.
     */
    public VirtualNetwork apply(ReadTransaction rtx) {
        // Apply the VTN configuration.
        applyVtn(rtx);

        // Apply the flow condition configuration.
        flowConditions.apply(vtnServices, rtx);

        // Apply the path policy configurations.
        pathPolicies.apply(vtnServices, rtx);

        // Apply the global path map configuration.
        applyPathMap(rtx);


        return this;
    }

    /**
     * Remove all the VTNs that are not associated with the configuration
     * in this instance.
     *
     * @param current  The current VTN tree.
     * @return  A map that contains VTNs present in the test environment.
     */
    private Map<String, Vtn> removeUnwanted(Vtns current) {
        List<Vtn> vtnList = current.getVtn();
        Map<String, Vtn> present = new HashMap<>();
        if (vtnList != null) {
            VtnService vtnSrv = vtnServices.getVtnService();
            for (Vtn vtn: vtnList) {
                String tname = vtn.getName().getValue();
                assertEquals(null, present.put(tname, vtn));
                if (!vTenants.containsKey(tname)) {
                    VTenantConfig.removeVtn(vtnSrv, tname);
                }
            }
        }

        return present;
    }

    /**
     * Return the virtual bridge that contains the specified virtual interface.
     *
     * @param ident  The identifier for the virtual interface.
     * @return  A {@link BridgeConfig} instance if found.
     *          {@code null} if not found.
     */
    private BridgeConfig<?, ?> getVirtualBridge(
        VInterfaceIdentifier<?> ident) {
        BridgeConfig<?, ?> vbconf;
        String tname = ident.getTenantNameString();
        VTenantConfig tconf = vTenants.get(tname);
        if (tconf != null) {
            String bname = ident.getBridgeNameString();
            VNodeType type = ident.getType().getBridgeType();
            if (type == VNodeType.VBRIDGE) {
                vbconf = tconf.getBridge(bname);
            } else {
                assertEquals(VNodeType.VTERMINAL, type);
                vbconf = tconf.getTerminal(bname);
            }
        } else {
            vbconf = null;
        }

        return vbconf;
    }

    /**
     * Prepare the virtual bridge that contains the specified virtual
     * interface.
     *
     * @param ident  The identifier for the virtual interface.
     * @return  The configuratio for the virtual bridge that contains the
     *          specified virtual interface.
     */
    private BridgeConfig<?, ?> prepareVirtualBridge(
        VInterfaceIdentifier<?> ident) {
        String tname = ident.getTenantNameString();
        VTenantConfig tconf = vTenants.get(tname);
        if (tconf == null) {
            tconf = new VTenantConfig();
            vTenants.put(tname, tconf);
        }

        String bname = ident.getBridgeNameString();
        VNodeType type = ident.getType().getBridgeType();
        BridgeConfig<?, ?> vbconf;
        if (type == VNodeType.VBRIDGE) {
            VBridgeConfig bconf = tconf.getBridge(bname);
            if (bconf == null) {
                bconf = new VBridgeConfig();
                tconf.addBridge(bname, bconf);
            }
            vbconf = bconf;
        } else {
            assertEquals(VNodeType.VTERMINAL, type);
            VTerminalConfig vtconf = tconf.getTerminal(bname);
            if (vtconf == null) {
                vtconf = new VTerminalConfig();
                tconf.addTerminal(bname, vtconf);
            }
            vbconf = vtconf;
        }

        return vbconf;
    }

    /**
     * Read the VTN tree.
     *
     * @param rtx  A read-only MD-SAL datastore transaction.
     * @return  A {@link Vtns} instance.
     */
    private Vtns readVtns(ReadTransaction rtx) {
        InstanceIdentifier<Vtns> path =
            InstanceIdentifier.create(Vtns.class);
        Optional<Vtns> opt = DataStoreUtils.read(rtx, path);
        assertEquals(true, opt.isPresent());

        return opt.get();
    }

    /**
     * Read the global path map container.
     *
     * @param rtx  A read-only MD-SAL datastore transaction.
     * @return  A {@link GlobalPathMaps} instance.
     */
    private GlobalPathMaps readGlobalPathMap(ReadTransaction rtx) {
        InstanceIdentifier<GlobalPathMaps> path =
            InstanceIdentifier.create(GlobalPathMaps.class);
        Optional<GlobalPathMaps> opt = DataStoreUtils.read(rtx, path);
        assertEquals(true, opt.isPresent());

        return opt.get();
    }

    /**
     * Verify the VTN tree configuration.
     *
     * @param rtx  A read-only MD-SAL datastore transaction.
     */
    private void verifyVtn(ReadTransaction rtx) {
        // Read the MAC address table tree.
        InstanceIdentifier<MacTables> mtPath =
            InstanceIdentifier.create(MacTables.class);
        Optional<MacTables> mopt = DataStoreUtils.read(rtx, mtPath);
        assertEquals(true, mopt.isPresent());

        List<Vtn> vtnList = readVtns(rtx).getVtn();
        List<TenantMacTable> tmList = mopt.get().getTenantMacTable();
        if (!vTenants.isEmpty()) {
            assertNotNull(vtnList);
            assertNotNull(tmList);
            assertEquals(tmList.size(), vtnList.size());
            Map<String, TenantMacTable> tmMap = new HashMap<>();
            for (TenantMacTable tmtable: tmList) {
                String name = tmtable.getName();
                assertEquals(null, tmMap.put(name, tmtable));
            }

            Set<String> checked = new HashSet<>();
            for (Vtn vtn: vtnList) {
                String name = vtn.getName().getValue();
                VTenantConfig tconf = vTenants.get(name);
                assertNotNull(tconf);
                TenantMacTable tmtable = tmMap.get(name);
                assertNotNull(tmtable);
                tconf.verify(rtx, name, vtn, tmtable);
                assertEquals(true, checked.add(name));
            }
            assertEquals(checked, vTenants.keySet());
        } else {
            if (vtnList != null) {
                assertEquals(Collections.<Vtn>emptyList(), vtnList);
            }
            if (tmList != null) {
                assertEquals(Collections.<TenantMacTable>emptyList(), tmList);
            }
        }
    }

    /**
     * Verify the global-path-maps container.
     *
     * @param rtx  A read-only MD-SAL datastore transaction.
     */
    private void verifyPathMap(ReadTransaction rtx) {
        pathMaps.verify(readGlobalPathMap(rtx));
    }

    /**
     * Apply configurations for the VTN tree.
     *
     * @param rtx  A read-only MD-SAL datastore transaction.
     */
    private void applyVtn(ReadTransaction rtx) {
        Map<String, Vtn> vtnPresent = removeUnwanted(readVtns(rtx));
        for (Entry<String, VTenantConfig> entry: vTenants.entrySet()) {
            String tname = entry.getKey();
            VTenantConfig tconf = entry.getValue();
            tconf.apply(vtnServices, tname, vtnPresent.get(tname));
        }
    }

    /**
     * Apply configurations for the global path maps.
     *
     * @param rtx  A read-only MD-SAL datastore transaction.
     */
    private void applyPathMap(ReadTransaction rtx) {
        pathMaps.apply(vtnServices, null, readGlobalPathMap(rtx));
    }
}
