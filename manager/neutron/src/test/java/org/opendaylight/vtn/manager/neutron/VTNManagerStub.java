/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
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
import org.opendaylight.vtn.manager.MacMapAclType;
import org.opendaylight.vtn.manager.MacMapConfig;
import org.opendaylight.vtn.manager.PortMap;
import org.opendaylight.vtn.manager.PortMapConfig;
import org.opendaylight.vtn.manager.UpdateOperation;
import org.opendaylight.vtn.manager.VBridge;
import org.opendaylight.vtn.manager.VBridgeConfig;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VNodeState;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenant;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.VlanMapConfig;
import org.opendaylight.vtn.manager.flow.DataFlow;
import org.opendaylight.vtn.manager.flow.DataFlowFilter;

/**
 * Stub class for unit tests.
 *
 * This stub provides APIs implemented in
 * org.opendaylight.vtn.manager package.
 */
public class VTNManagerStub implements IVTNManager {
    // A tenant information which the stub class has.
    static final String TENANT_1_UUID = "4b99cbea5fa7450ab40a81929e40371d";
    static final String TENANT_1_NAME = "4b99cbea5fa750ab40a81929e40371d";

    // A vBridge information which the stub class has.
    static final String BRIDGE_1_UUID = "9CFF065F-44AC-47B9-9E81-5E51CA843307";
    static final String BRIDGE_1_NAME = "9CFF065F44AC7B99E815E51CA843307";

    // A vBridge interface information which stub the class has.
    static final String VBR_IF_1_UUID = "F6197D54-97A1-44D2-ABFB-6DFED030C30F";
    static final String VBR_IF_1_NAME = "F6197D5497A14D2ABFB6DFED030C30F";

    // Following methods are used in UnitTest.
    @Override
    public List<VTenant> getTenants() throws VTNException {
        return null;
    }

    @Override
    public VTenant getTenant(VTenantPath path) throws VTNException {
        if (path == null) {
            Status status = new Status(StatusCode.BADREQUEST);
            throw new VTNException(status);
        }

        VTenantPath tenant1 = new VTenantPath(TENANT_1_NAME);
        if (path.equals(tenant1)) {
            VTenantConfig conf = new VTenantConfig(null);
            VTenant tenant = new VTenant(TENANT_1_NAME, conf);
            return tenant;
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
    public Status modifyTenant(VTenantPath path, VTenantConfig tconf,
                               boolean all) {
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
    public VBridge getBridge(VBridgePath path) throws VTNException {
        if (path == null) {
            Status status = new Status(StatusCode.BADREQUEST);
            throw new VTNException(status);
        }

        VBridgePath bridge1 = new VBridgePath(TENANT_1_NAME,
                                              BRIDGE_1_NAME);
        if (path.equals(bridge1)) {
            VBridgeConfig bconf = new VBridgeConfig(null);
            VBridge bridge = new VBridge(BRIDGE_1_NAME,
                                         VNodeState.UNKNOWN,
                                         0,
                                         bconf);
            return bridge;
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
    public Status modifyBridge(VBridgePath path, VBridgeConfig bconf,
                               boolean all) {
        // @Todo Check paramters, and change the return value.
        return new Status(StatusCode.SUCCESS);
    }

    @Override
    public Status removeBridge(VBridgePath path) {
        return null;
    }

    @Override
    public List<VInterface> getBridgeInterfaces(VBridgePath path)
        throws VTNException {
        return null;
    }

    @Override
    public VInterface getBridgeInterface(VBridgeIfPath path)
        throws VTNException {
        if (path == null) {
            Status status = new Status(StatusCode.BADREQUEST);
            throw new VTNException(status);
        }

        VBridgeIfPath if1 = new VBridgeIfPath(TENANT_1_NAME,
                                              BRIDGE_1_NAME,
                                              VBR_IF_1_NAME);
        if (path.equals(if1)) {
            VInterfaceConfig iconf = new VInterfaceConfig(null, true);
            VInterface vif = new VInterface(VBR_IF_1_NAME,
                                            VNodeState.UNKNOWN,
                                            VNodeState.UNKNOWN,
                                            iconf);
            return vif;
        }

        return null;
    }

    @Override
    public Status addBridgeInterface(VBridgeIfPath path,
                                     VInterfaceConfig iconf) {
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
    public Status modifyBridgeInterface(VBridgeIfPath path,
                                        VInterfaceConfig iconf,
                                        boolean all) {
        return null;
    }

    @Override
    public Status removeBridgeInterface(VBridgeIfPath path) {
        return null;
    }

    @Override
    public List<VlanMap> getVlanMaps(VBridgePath path) throws VTNException {
        return null;
    }

    @Override
    public VlanMap getVlanMap(VBridgePath path, String mapId)
        throws VTNException {
        return null;
    }

    @Override
    public VlanMap getVlanMap(VBridgePath path, VlanMapConfig vlconf)
        throws VTNException {
        return null;
    }

    @Override
    public VlanMap addVlanMap(VBridgePath path, VlanMapConfig vlconf)
        throws VTNException {
        return null;
    }

    @Override
    public Status removeVlanMap(VBridgePath path, String mapId) {
        return null;
    }

    @Override
    public Status setPortMap(VBridgeIfPath path, PortMapConfig pmconf) {
        return new Status(StatusCode.CREATED, "desc");
    }

    // Following methods are Unused in UnitTest.
    @Override
    public PortMap getPortMap(VBridgeIfPath path) throws VTNException {
        return null;
    }

    @Override
    public MacMap getMacMap(VBridgePath path) throws VTNException {
        return null;
    }

    @Override
    public Set<DataLinkHost> getMacMapConfig(VBridgePath path,
                                             MacMapAclType aclType)
        throws VTNException {
        return null;
    }

    @Override
    public List<MacAddressEntry> getMacMappedHosts(VBridgePath path)
        throws VTNException {
        return null;
    }

    @Override
    public MacAddressEntry getMacMappedHost(VBridgePath path,
                                            DataLinkAddress addr)
        throws VTNException {
        return null;
    }

    @Override
    public UpdateType setMacMap(VBridgePath path, UpdateOperation op,
                                MacMapConfig mcconf) throws VTNException {
        return null;
    }

    @Override
    public UpdateType setMacMap(VBridgePath path, UpdateOperation op,
                                MacMapAclType aclType,
                                Set<? extends DataLinkHost> dlhosts)
        throws VTNException {
        return null;
    }

    @Override
    public void findHost(InetAddress addr, Set<VBridgePath> pathSet) {
    }

    @Override
    public boolean probeHost(HostNodeConnector host) {
        return true;
    }

    @Override
    public List<MacAddressEntry> getMacEntries(VBridgePath path)
        throws VTNException {
        return null;
    }

    @Override
    public MacAddressEntry getMacEntry(VBridgePath path, DataLinkAddress addr)
        throws VTNException {
        return null;
    }

    @Override
    public MacAddressEntry removeMacEntry(VBridgePath path,
        DataLinkAddress addr)
        throws VTNException {
        return null;
    }

    @Override
    public Status flushMacEntries(VBridgePath path) {
        return null;
    }

    @Override
    public List<DataFlow> getDataFlows(VTenantPath path, DataFlow.Mode mode,
                                       DataFlowFilter filter) {
        return null;
    }

    @Override
    public DataFlow getDataFlow(VTenantPath path, long flowId,
                                DataFlow.Mode mode) {
        return null;
    }

    @Override
    public int getDataFlowCount(VTenantPath path) {
        return 0;
    }

    @Override
    public boolean isActive() {
        return true;
    }

}
