/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron;

import java.net.InetAddress;
import java.util.List;
import java.util.Set;

import org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.packet.address.DataLinkAddress;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.vtn.manager.DataLinkHost;
import org.opendaylight.vtn.manager.IVTNManager;
import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.MacMap;
import org.opendaylight.vtn.manager.MacMapConfig;
import org.opendaylight.vtn.manager.PathMap;
import org.opendaylight.vtn.manager.PathPolicy;
import org.opendaylight.vtn.manager.PortLocation;
import org.opendaylight.vtn.manager.PortMap;
import org.opendaylight.vtn.manager.PortMapConfig;
import org.opendaylight.vtn.manager.VBridge;
import org.opendaylight.vtn.manager.VBridgeConfig;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenant;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VTerminal;
import org.opendaylight.vtn.manager.VTerminalConfig;
import org.opendaylight.vtn.manager.VTerminalPath;
import org.opendaylight.vtn.manager.VTerminalIfPath;
import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.VlanMapConfig;
import org.opendaylight.vtn.manager.flow.DataFlow;
import org.opendaylight.vtn.manager.flow.DataFlowFilter;
import org.opendaylight.vtn.manager.flow.cond.FlowCondition;
import org.opendaylight.vtn.manager.flow.cond.FlowMatch;
import org.opendaylight.vtn.manager.flow.filter.FlowFilter;
import org.opendaylight.vtn.manager.flow.filter.FlowFilterId;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.DataFlowMode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnAclType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;

/**
 * Stub class for unit tests.
 *
 * This stub provides APIs implemented in
 * org.opendaylight.vtn.manager package.
 */
public class VTNManagerStub implements IVTNManager {
    // Boolean identifier for returning valid VInterface on setting this boolean variable
    static boolean isReturnValidVInterface = false;

    // An invalid UUID
    static final String INVALID_UUID = "F6197D54-97A1-44D2-ABFB-6DFED030C30F-";

    // A tenant information which the stub class has.
    static final String TENANT_1_UUID = "4b99cbea5fa7450ab40a81929e40371d";
    static final String TENANT_1_NAME = "4b99cbea5fa750ab40a81929e40371d";

    // A vBridge information which the stub class has.
    static final String BRIDGE_1_UUID = "9CFF065F-44AC-47B9-9E81-5E51CA843307";
    static final String BRIDGE_1_NAME = "9CFF065F44AC7B99E815E51CA843307";

    // A vBridge interface information which stub the class has.
    static final String VBR_IF_1_UUID = "F6197D54-97A1-44D2-ABFB-6DFED030C30F";
    static final String VBR_IF_1_NAME = "F6197D5497A14D2ABFB6DFED030C30F";
    static final String VBR_IF_2_UUID = "F6197D54-97A1-44D2-ABFB-6DFED030C300";
    static final String VBR_IF_2_NAME = "F6197D5497A14D2ABFB6DFED030C300";
    static final String VBR_IF_3_UUID = "F6197D54-97A1-44D2-ABFB-6DFED030C301";
    static final String VBR_IF_3_NAME = "F6197D5497A14D2ABFB6DFED030C301";
    static final String VBR_IF_4_UUID = "F6197D54-97A1-44D2-ABFB-6DFED030C302";
    static final String VBR_IF_4_NAME = "F6197D5497A14D2ABFB6DFED030C302";

    // Following methods are used in UnitTest.
    @Override
    public boolean isActive() {
        return true;
    }

    @Override
    public VTenant getTenant(VTenantPath path) throws VTNException {
        if (path == null) {
            Status status = new Status(StatusCode.BADREQUEST);
            throw new VTNException(status);
        }

        VTenantPath tenant1 = new VTenantPath(TENANT_1_NAME);
        VTenantPath tenant2 = new VTenantPath(TENANT_1_UUID);
        if (path.equals(tenant1)) {
            VTenantConfig conf = new VTenantConfig(null);
            VTenant tenant = new VTenant(TENANT_1_NAME, conf);
            return tenant;
        } else if (path.equals(tenant2)) {
            Status status = new Status(StatusCode.BADREQUEST);
            throw new VTNException(status);
        }

        return null;
    }

    @Override
    public Status addTenant(VTenantPath path, VTenantConfig tconf) {
        if (path == null || tconf == null) {
            return new Status(StatusCode.BADREQUEST);
        }

        VTenantPath tenant1 = new VTenantPath(TENANT_1_NAME);
        if (path.equals(tenant1)) {
            return new Status(StatusCode.CONFLICT);
        }

        return new Status(StatusCode.SUCCESS);
    }

    @Override
    public VBridge getBridge(VBridgePath path) throws VTNException {
        if (path == null) {
            Status status = new Status(StatusCode.BADREQUEST);
            throw new VTNException(status);
        }

        VBridgePath bridge1 = new VBridgePath(TENANT_1_NAME,
                                              BRIDGE_1_NAME);

        VBridgePath bridge2 = new VBridgePath(TENANT_1_UUID,
                                              BRIDGE_1_NAME);

        if (path.equals(bridge1)) {
            VBridgeConfig bconf = new VBridgeConfig(null);
            VBridge bridge = new VBridge(BRIDGE_1_NAME,
                                         VnodeState.UNKNOWN,
                                         0,
                                         bconf);
            return bridge;
        } else if (path.equals(bridge2)) {
            Status status = new Status(StatusCode.BADREQUEST);
            throw new VTNException(status);
        }

        return null;
    }

    @Override
    public Status addBridge(VBridgePath path, VBridgeConfig bconf) {
        if (path == null || bconf == null) {
            return new Status(StatusCode.BADREQUEST);
        }

        VBridgePath bridge1 = new VBridgePath(TENANT_1_NAME,
                                              BRIDGE_1_NAME);
        if (path.equals(bridge1)) {
            return new Status(StatusCode.CONFLICT);
        }

        return new Status(StatusCode.SUCCESS);
    }

    @Override
    public Status modifyBridge(VBridgePath path, VBridgeConfig bconf, boolean all) {
        return new Status(StatusCode.SUCCESS);
    }

    @Override
    public VInterface getInterface(VBridgeIfPath path) throws VTNException {
        if (path == null) {
            Status status = new Status(StatusCode.BADREQUEST);
            throw new VTNException(status);
        }

        VBridgeIfPath if1 = new VBridgeIfPath(TENANT_1_NAME,
                                              BRIDGE_1_NAME,
                                              VBR_IF_1_NAME);

        VBridgeIfPath if2 = new VBridgeIfPath(TENANT_1_NAME,
                                              BRIDGE_1_NAME,
                                              VBR_IF_2_NAME);

        VBridgeIfPath if3 = new VBridgeIfPath(TENANT_1_NAME,
                                              BRIDGE_1_NAME,
                                              VBR_IF_3_NAME);
        if (path.equals(if1)) {
            VInterfaceConfig iconf = new VInterfaceConfig(null, true);
            VInterface vif = new VInterface(VBR_IF_1_NAME,
                                            VnodeState.UNKNOWN,
                                            VnodeState.UNKNOWN,
                                            iconf);
            return vif;
        } else if (path.equals(if2)) {
            VInterfaceConfig iconf = new VInterfaceConfig("br-int config", true);
            VInterface vif = new VInterface(VBR_IF_1_NAME,
                                            VnodeState.UNKNOWN,
                                            VnodeState.UNKNOWN,
                                            iconf);
            return vif;
        } else if (path.equals(if3)) {
            if (isReturnValidVInterface) {
                isReturnValidVInterface = false;

                VInterfaceConfig iconf = new VInterfaceConfig(null, true);
                VInterface vif = new VInterface(VBR_IF_3_NAME,
                                                VnodeState.UNKNOWN,
                                                VnodeState.UNKNOWN,
                                                iconf);
                return vif;
            }
            Status status = new Status(StatusCode.BADREQUEST);
            throw new VTNException(status);
        }

        return null;
    }

    @Override
    public Status addInterface(VBridgeIfPath path, VInterfaceConfig iconf) {
        if (path == null || iconf == null) {
            return new Status(StatusCode.BADREQUEST);
        }

        VBridgeIfPath bif1 = new VBridgeIfPath(TENANT_1_NAME,
                                               BRIDGE_1_NAME,
                                               VBR_IF_2_NAME);

        VBridgeIfPath bif2 = new VBridgeIfPath(TENANT_1_NAME,
                                              BRIDGE_1_NAME,
                                              VBR_IF_4_NAME);

        if ((path.equals(bif1)) || (path.equals(bif2))) {
            return new Status(StatusCode.CONFLICT);
        }

        return new Status(StatusCode.SUCCESS);
    }

    @Override
    public Status modifyInterface(VBridgeIfPath path, VInterfaceConfig iconf, boolean all) {
        if (path == null || iconf == null) {
            return new Status(StatusCode.BADREQUEST);
        }

        VBridgeIfPath bif1 = new VBridgeIfPath(TENANT_1_NAME,
                                               BRIDGE_1_NAME,
                                               VBR_IF_1_NAME);
        if (path.equals(bif1)) {
            return new Status(StatusCode.CONFLICT);
        }

        return new Status(StatusCode.SUCCESS);
    }

    @Override
    public Status removeInterface(VBridgeIfPath path) {
        if (path == null) {
            return new Status(StatusCode.BADREQUEST);
        }

        VBridgeIfPath bif1 = new VBridgeIfPath(TENANT_1_NAME,
                                               BRIDGE_1_NAME,
                                               VBR_IF_1_NAME);
        if (path.equals(bif1)) {
            return new Status(StatusCode.SUCCESS);
        }

        return new Status(StatusCode.NOTFOUND);
    }

    @Override
    public Status setPortMap(VBridgeIfPath path, PortMapConfig pmconf) {
        if (!TestBase.CONFLICTED_NETWORK_UUID.equalsIgnoreCase(path.getBridgeName())) {
            return new Status(StatusCode.CREATED, "desc");
        }
        return new Status(StatusCode.NOTFOUND);
    }

    // Following methods are Unused in UnitTest.
    @Override
    public List<VTenant> getTenants() throws VTNException {
        return null;
    }

    @Override
    public Status modifyTenant(VTenantPath path, VTenantConfig tconf, boolean all) {
        return null;
    }

    @Override
    public Status removeTenant(VTenantPath path) {
        return null;
    }

    @Override
    public List<VBridge> getBridges(VTenantPath path) throws VTNException {
        return null;
    }

    @Override
    public Status removeBridge(VBridgePath path) {
        return null;
    }

    @Override
    public List<VTerminal> getTerminals(VTenantPath path) throws VTNException {
        return null;
    }

    @Override
    public VTerminal getTerminal(VTerminalPath path) throws VTNException {
        return null;
    }

    @Override
    public Status addTerminal(VTerminalPath path, VTerminalConfig vtconf) {
        return null;
    }

    @Override
    public Status modifyTerminal(VTerminalPath path, VTerminalConfig vtconf, boolean all) {
        return null;
    }

    @Override
    public Status removeTerminal(VTerminalPath path) {
        return null;
    }

    @Override
    public List<VInterface> getInterfaces(VBridgePath path) throws VTNException {
        return null;
    }


    @Override
    public List<VInterface> getInterfaces(VTerminalPath path) throws VTNException {
        return null;
    }

    @Override
    public VInterface getInterface(VTerminalIfPath path) throws VTNException {
        return null;
    }

    @Override
    public Status addInterface(VTerminalIfPath path, VInterfaceConfig iconf) {
        return null;
    }

    @Override
    public Status modifyInterface(VTerminalIfPath path, VInterfaceConfig iconf, boolean all) {
        return null;
    }

    @Override
    public Status removeInterface(VTerminalIfPath path) {
        return null;
    }

    @Override
    public List<VlanMap> getVlanMaps(VBridgePath path) throws VTNException {
        return null;
    }

    @Override
    public VlanMap getVlanMap(VBridgePath path, String mapId) throws VTNException {
        return null;
    }

    @Override
    public VlanMap getVlanMap(VBridgePath path, VlanMapConfig vlconf) throws VTNException {
        return null;
    }

    @Override
    public VlanMap addVlanMap(VBridgePath path, VlanMapConfig vlconf) throws VTNException {
        return null;
    }

    @Override
    public Status removeVlanMap(VBridgePath path, String mapId) {
        return null;
    }

    @Override
    public PortMap getPortMap(VBridgeIfPath path) throws VTNException {
        return null;
    }

    @Override
    public PortMap getPortMap(VTerminalIfPath path) throws VTNException {
        return null;
    }

    @Override
    public Status setPortMap(VTerminalIfPath path, PortMapConfig pmconf) {
        return null;
    }

    @Override
    public MacMap getMacMap(VBridgePath path) throws VTNException {
        return null;
    }

    @Override
    public Set<DataLinkHost> getMacMapConfig(VBridgePath path,
                                             VtnAclType aclType)
        throws VTNException {
        return null;
    }

    @Override
    public List<MacAddressEntry> getMacMappedHosts(VBridgePath path) throws VTNException {
        return null;
    }

    @Override
    public MacAddressEntry getMacMappedHost(VBridgePath path, DataLinkAddress addr) throws VTNException {
        return null;
    }

    @Override
    public UpdateType setMacMap(VBridgePath path, VtnUpdateOperationType op,
                                MacMapConfig mcconf) throws VTNException {
        return null;
    }

    @Override
    public UpdateType setMacMap(VBridgePath path, VtnUpdateOperationType op,
                                VtnAclType aclType,
                                Set<? extends DataLinkHost> dlhosts)
        throws VTNException {
        return null;
    }

    @Override
    public void findHost(InetAddress addr, Set<VBridgePath> pathSet) {
    }

    @Override
    public boolean probeHost(HostNodeConnector host) {
        return false;
    }

    @Override
    public List<MacAddressEntry> getMacEntries(VBridgePath path) throws VTNException {
        return null;
    }

    @Override
    public MacAddressEntry getMacEntry(VBridgePath path, DataLinkAddress addr) throws VTNException {
        return null;
    }

    @Override
    public MacAddressEntry removeMacEntry(VBridgePath path, DataLinkAddress addr) throws VTNException {
        return null;
    }

    @Override
    public Status flushMacEntries(VBridgePath path) {
        return null;
    }

    @Override
    public List<DataFlow> getDataFlows(VTenantPath path, DataFlowMode mode,
            DataFlowFilter filter, int interval) throws VTNException {
        return null;
    }

    @Override
    public DataFlow getDataFlow(VTenantPath path, long flowId,
                                DataFlowMode mode, int interval)
        throws VTNException {
        return null;
    }

    @Override
    public int getDataFlowCount(VTenantPath path) throws VTNException {
        return 0;
    }

    @Override
    public List<FlowCondition> getFlowConditions() throws VTNException {
        return null;
    }

    @Override
    public FlowCondition getFlowCondition(String name) throws VTNException {
        return null;
    }

    @Override
    public UpdateType setFlowCondition(String name, FlowCondition fcond) throws VTNException {
        return null;
    }

    @Override
    public Status removeFlowCondition(String name) {
        return null;
    }

    @Override
    public Status clearFlowCondition() {
        return null;
    }

    @Override
    public FlowMatch getFlowConditionMatch(String name, int index)
        throws VTNException {
        return null;
    }

    @Override
    public UpdateType setFlowConditionMatch(String name, int index, FlowMatch match) throws VTNException {
        return null;
    }

    @Override
    public Status removeFlowConditionMatch(String name, int index) {
        return null;
    }

    @Override
    public List<Integer> getPathPolicyIds() throws VTNException {
        return null;
    }

    @Override
    public PathPolicy getPathPolicy(int id) throws VTNException {
        return null;
    }

    @Override
    public UpdateType setPathPolicy(int id, PathPolicy policy) throws VTNException {
        return null;
    }

    @Override
    public Status removePathPolicy(int id) {
        return null;
    }

    @Override
    public Status clearPathPolicy() {
        return null;
    }

    @Override
    public long getPathPolicyDefaultCost(int id) throws VTNException {
        return 0;
    }

    @Override
    public boolean setPathPolicyDefaultCost(int id, long cost)
        throws VTNException {
        return false;
    }

    @Override
    public long getPathPolicyCost(int id, PortLocation ploc)
        throws VTNException {
        return 0;
    }

    @Override
    public UpdateType setPathPolicyCost(int id,
            PortLocation ploc, long cost) throws VTNException {
        return null;
    }

    @Override
    public Status removePathPolicyCost(int id, PortLocation ploc) {
        return null;
    }

    @Override
    public List<PathMap> getPathMaps() throws VTNException {
        return null;
    }

    @Override
    public PathMap getPathMap(int index) throws VTNException {
        return null;
    }

    @Override
    public UpdateType setPathMap(int index, PathMap pmap) throws VTNException {
        return null;
    }

    @Override
    public Status removePathMap(int index) {
        return null;
    }

    @Override
    public Status clearPathMap() {
        return null;
    }

    @Override
    public List<PathMap> getPathMaps(VTenantPath path) throws VTNException {
        return null;
    }

    @Override
    public PathMap getPathMap(VTenantPath path, int index) throws VTNException {
        return null;
    }

    @Override
    public UpdateType setPathMap(VTenantPath path, int index, PathMap pmap) throws VTNException {
        return null;
    }

    @Override
    public Status removePathMap(VTenantPath path, int index) {
        return null;
    }

    @Override
    public Status clearPathMap(VTenantPath path) {
        return null;
    }

    @Override
    public List<FlowFilter> getFlowFilters(FlowFilterId fid) throws VTNException {
        return null;
    }

    @Override
    public FlowFilter getFlowFilter(FlowFilterId fid, int index) throws VTNException {
        return null;
    }

    @Override
    public UpdateType setFlowFilter(FlowFilterId fid, int index, FlowFilter filter) throws VTNException {
        return null;
    }

    @Override
    public Status removeFlowFilter(FlowFilterId fid, int index) {
        return null;
    }

    @Override
    public Status clearFlowFilter(FlowFilterId fid) {
        return null;
    }
}
