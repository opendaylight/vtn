/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Set;

import org.opendaylight.vtn.manager.it.ofmock.OfMockService;
import org.opendaylight.vtn.manager.it.ofmock.OfMockUtils;
import org.opendaylight.vtn.manager.it.util.RpcErrorTag;
import org.opendaylight.vtn.manager.it.util.TestPort;
import org.opendaylight.vtn.manager.it.util.VirtualNetwork;
import org.opendaylight.vtn.manager.it.util.vnode.BridgeIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeIfIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VInterfaceConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VInterfaceIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VTNPortMapConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VTerminalIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VTerminalIfIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.SetPortMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.VtnPortMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * Test case for {@link VtnPortMapService} that depends on inventory
 * information.
 */
public final class PortMapStatusTest extends TestMethodBase {
    /**
     * Construct a new instance.
     *
     * @param vit  A {@link VTNManagerIT} instance.
     */
    public PortMapStatusTest(VTNManagerIT vit) {
        super(vit);
    }

    // TestMethodBase

    /**
     * Run the test.
     *
     * @throws Exception  An error occurred.
     */
    @Override
    protected void runTest() throws Exception {
        VTNManagerIT vit = getTest();
        VtnPortMapService pmapSrv = vit.getPortMapService();
        VirtualNetwork vnet = getVirtualNetwork();
        OfMockService ofmock = vit.getOfMockService();

        // Collect edge ports and internal ports.
        List<String> edgePorts = new ArrayList<>();
        Set<String> internalPorts = new HashSet<>();
        Map<String, String> nodeMap = new HashMap<>();
        Map<String, String> portIdMap = new HashMap<>();
        Map<String, String> portNameMap = new HashMap<>();
        for (String nid: ofmock.getNodes()) {
            for (String pid: ofmock.getPorts(nid, true)) {
                nodeMap.put(pid, nid);
                portIdMap.put(pid, OfMockUtils.getPortId(pid));
                portNameMap.put(pid, ofmock.getPortName(pid));
                edgePorts.add(pid);
            }
            for (String pid: ofmock.getPorts(nid, false)) {
                nodeMap.put(pid, nid);
                portIdMap.put(pid, OfMockUtils.getPortId(pid));
                portNameMap.put(pid, ofmock.getPortName(pid));

                String peer = ofmock.getPeerIdentifier(pid);
                if (!internalPorts.contains(peer)) {
                    internalPorts.add(pid);
                }
            }
        }
        assertEquals(false, edgePorts.isEmpty());
        assertEquals(false, internalPorts.isEmpty());

        // Map edge ports by specifying port ID.
        Map<String, Set<VInterfaceIdentifier<?>>> portMaps = new HashMap<>();
        Map<TestPort, VInterfaceIdentifier<?>> mappedPorts = new HashMap<>();
        String tname1 = "vtn_1";
        String iname = "if";
        int vbrCount = 0;
        int vid = 0;
        for (String pid: edgePorts) {
            String vbrName = "vbridge_" + vbrCount;
            vbrCount++;

            VBridgeIfIdentifier ifId =
                new VBridgeIfIdentifier(tname1, vbrName, iname);
            vnet.addInterface(ifId).apply();
            VInterfaceConfig iconf = vnet.getInterface(ifId);

            String nid = nodeMap.get(pid);
            String id = portIdMap.get(pid);
            VTNPortMapConfig pmap = new VTNPortMapConfig(nid, id, null, vid).
                setMappedPort(pid);
            iconf.setPortMap(pmap).
                setState(VnodeState.UP).
                setEntityState(VnodeState.UP);
            assertEquals(VtnUpdateType.CREATED, pmap.update(pmapSrv, ifId));
            vnet.verify();

            Set<VInterfaceIdentifier<?>> ifSet = portMaps.get(pid);
            if (ifSet == null) {
                ifSet = new HashSet<>();
                portMaps.put(pid, ifSet);
            }
            ifSet.add(ifId);

            assertEquals(null, mappedPorts.put(new TestPort(pid, vid), ifId));
            vid++;
        }

        // Map edge ports by specifying port name.
        vid = 4095 - edgePorts.size() + 1;
        int termCount = 0;
        for (String pid: edgePorts) {
            String termName = "vterm_" + termCount;
            termCount++;

            VTerminalIfIdentifier ifId =
                new VTerminalIfIdentifier(tname1, termName, iname);
            vnet.addInterface(ifId).apply();
            VInterfaceConfig iconf = vnet.getInterface(ifId);

            String nid = nodeMap.get(pid);
            String name = portNameMap.get(pid);
            VTNPortMapConfig pmap = new VTNPortMapConfig(nid, null, name, vid).
                setMappedPort(pid);
            iconf.setPortMap(pmap).
                setState(VnodeState.UP).
                setEntityState(VnodeState.UP);
            assertEquals(VtnUpdateType.CREATED, pmap.update(pmapSrv, ifId));
            vnet.verify();

            Set<VInterfaceIdentifier<?>> ifSet = portMaps.get(pid);
            if (ifSet == null) {
                ifSet = new HashSet<>();
                portMaps.put(pid, ifSet);
            }
            ifSet.add(ifId);

            assertEquals(null, mappedPorts.put(new TestPort(pid, vid), ifId));
            vid++;
        }

        // Map edge ports by specifying port ID and port name.
        vid = 123;
        for (String pid: edgePorts) {
            String vbrName = "vbridge_" + vbrCount;
            vbrCount++;

            VBridgeIfIdentifier ifId =
                new VBridgeIfIdentifier(tname1, vbrName, iname);
            vnet.addInterface(ifId).apply();
            VInterfaceConfig iconf = vnet.getInterface(ifId);

            // At first, map the port with specifying only port ID.
            String nid = nodeMap.get(pid);
            String id = portIdMap.get(pid);
            VTNPortMapConfig pmap = new VTNPortMapConfig(nid, id, null, vid).
                setMappedPort(pid);
            iconf.setPortMap(pmap).
                setState(VnodeState.UP).
                setEntityState(VnodeState.UP);
            assertEquals(VtnUpdateType.CREATED, pmap.update(pmapSrv, ifId));
            vnet.verify();

            // Add port name to the port mapping configuration.
            pmap.setPortName(portNameMap.get(pid));
            assertEquals(VtnUpdateType.CHANGED, pmap.update(pmapSrv, ifId));
            vnet.verify();

            Set<VInterfaceIdentifier<?>> ifSet = portMaps.get(pid);
            if (ifSet == null) {
                ifSet = new HashSet<>();
                portMaps.put(pid, ifSet);
            }
            ifSet.add(ifId);

            assertEquals(null, mappedPorts.put(new TestPort(pid, vid), ifId));
            vid++;
        }

        // Map internal ports by specifying port ID.
        vid = 1024;
        for (String pid: internalPorts) {
            String vbrName = "vbridge_" + vbrCount;
            vbrCount++;

            VBridgeIfIdentifier ifId =
                new VBridgeIfIdentifier(tname1, vbrName, iname);
            vnet.addInterface(ifId).apply();
            VInterfaceConfig iconf = vnet.getInterface(ifId);

            String nid = nodeMap.get(pid);
            String id = portIdMap.get(pid);
            VTNPortMapConfig pmap = new VTNPortMapConfig(nid, id, null, vid).
                setMappedPort(pid);
            iconf.setPortMap(pmap).
                setState(VnodeState.DOWN).
                setEntityState(VnodeState.UP);
            assertEquals(VtnUpdateType.CREATED, pmap.update(pmapSrv, ifId));
            vnet.verify();

            Set<VInterfaceIdentifier<?>> ifSet = portMaps.get(pid);
            if (ifSet == null) {
                ifSet = new HashSet<>();
                portMaps.put(pid, ifSet);
            }
            ifSet.add(ifId);

            assertEquals(null, mappedPorts.put(new TestPort(pid, vid), ifId));
            vid++;
        }

        // Ensure that VLANs already mapped cannot be mapped to another
        // virtual interface.
        for (Entry<TestPort, VInterfaceIdentifier<?>> entry:
                 mappedPorts.entrySet()) {
            TestPort tp = entry.getKey();
            VInterfaceIdentifier<?> mapIfId = entry.getValue();
            for (VInterfaceIdentifier<?> ifId: mappedPorts.values()) {
                if (mapIfId.equals(ifId)) {
                    VInterfaceConfig iconf = vnet.getInterface(ifId);
                    VTNPortMapConfig pmap = iconf.getPortMap();
                    assertEquals(null, pmap.update(pmapSrv, ifId));
                } else {
                    String pid = tp.getPortIdentifier();
                    String nid = nodeMap.get(pid);
                    String id = portIdMap.get(pid);
                    VTNPortMapConfig pmap =
                        new VTNPortMapConfig(nid, id, null, tp.getVlanId());
                    SetPortMapInput input = pmap.newInputBuilder(ifId).
                        build();
                    checkRpcError(pmapSrv.setPortMap(input),
                                  RpcErrorTag.DATA_EXISTS,
                                  VtnErrorTag.CONFLICT);
                }
            }
        }

        // Errors should never affect existing port mappings.
        vnet.verify();

        // Down all the edge ports.
        VNodeStateWaiter waiter = new VNodeStateWaiter(ofmock);
        for (String pid: edgePorts) {
            assertEquals(true, ofmock.setPortState(pid, false, false));

            Set<VInterfaceIdentifier<?>> ifSet = portMaps.get(pid);
            for (VInterfaceIdentifier<?> ifId: ifSet) {
                BridgeIdentifier<?> brId = ifId.getBridgeIdentifier();
                waiter.set(brId, VnodeState.DOWN).
                    set(ifId, VnodeState.DOWN, VnodeState.DOWN);
                VInterfaceConfig iconf = vnet.getInterface(ifId);
                iconf.setState(VnodeState.DOWN).
                    setEntityState(VnodeState.DOWN);
            }
            waiter.await();
            vnet.verify();
        }

        // Remove inter-switch links on internalPorts.
        for (String pid: internalPorts) {
            addStaticEdgePort(vit, pid);

            Set<VInterfaceIdentifier<?>> ifSet = portMaps.get(pid);
            for (VInterfaceIdentifier<?> ifId: ifSet) {
                BridgeIdentifier<?> brId = ifId.getBridgeIdentifier();
                waiter.set(brId, VnodeState.UP).
                    set(ifId, VnodeState.UP, VnodeState.UP);
                VInterfaceConfig iconf = vnet.getInterface(ifId);
                iconf.setState(VnodeState.UP).
                    setEntityState(VnodeState.UP);
            }
            waiter.await();
            vnet.verify();
        }

        // Up all the edge ports.
        for (String pid: edgePorts) {
            assertEquals(true, ofmock.setPortState(pid, true, false));

            Set<VInterfaceIdentifier<?>> ifSet = portMaps.get(pid);
            for (VInterfaceIdentifier<?> ifId: ifSet) {
                BridgeIdentifier<?> brId = ifId.getBridgeIdentifier();
                waiter.set(brId, VnodeState.UP).
                    set(ifId, VnodeState.UP, VnodeState.UP);
                VInterfaceConfig iconf = vnet.getInterface(ifId);
                iconf.setState(VnodeState.UP).
                    setEntityState(VnodeState.UP);
            }
            waiter.await();
            vnet.verify();
        }

        // Remove internalPorts from static-edge-ports.
        for (String pid: internalPorts) {
            removeStaticEdgePort(vit, pid);

            Set<VInterfaceIdentifier<?>> ifSet = portMaps.get(pid);
            for (VInterfaceIdentifier<?> ifId: ifSet) {
                BridgeIdentifier<?> brId = ifId.getBridgeIdentifier();
                waiter.set(brId, VnodeState.DOWN).
                    set(ifId, VnodeState.DOWN, VnodeState.UP);
                VInterfaceConfig iconf = vnet.getInterface(ifId);
                iconf.setState(VnodeState.DOWN).
                    setEntityState(VnodeState.UP);
            }
            waiter.await();
            vnet.verify();
        }

        // Remove all the port mappings.
        List<VTNPortMapConfig> allMaps = new ArrayList<>();
        boolean removeIf = false;
        for (VInterfaceIdentifier<?> ifId: mappedPorts.values()) {
            VInterfaceConfig iconf = vnet.getInterface(ifId);
            allMaps.add(iconf.getPortMap());
            if (removeIf) {
                removeVinterface(ifId);
                vnet.removeInterface(ifId);
            } else {
                assertEquals(VtnUpdateType.REMOVED, removePortMap(ifId));
                iconf.setPortMap(null).
                    setState(VnodeState.UNKNOWN).
                    setEntityState(VnodeState.UNKNOWN);
            }

            vnet.verify();
            removeIf = !removeIf;
        }

        for (int i = 0; i < 3; i++) {
            // Restore port mappings.
            VBridgeIdentifier vbrId = new VBridgeIdentifier(tname1, "vbridge");
            vnet.addBridge(vbrId);
            VBridgeConfig bconf = vnet.getBridge(vbrId);

            List<VTerminalIdentifier> vtermIds = new ArrayList<>();
            int count = 0;
            boolean mapBridge = true;
            for (VTNPortMapConfig pmap: allMaps) {
                VInterfaceConfig iconf;
                VInterfaceIdentifier<?> ifId;
                if (mapBridge) {
                    String bifName = "if_" + count;
                    iconf = new VInterfaceConfig();
                    bconf.addInterface(bifName, iconf);
                    ifId = vbrId.childInterface(bifName);
                } else {
                    String termName = "vterm_" + termCount;
                    termCount++;
                    VTerminalIdentifier vtermId =
                        new VTerminalIdentifier(tname1, termName);
                    ifId = vtermId.childInterface(iname);
                    vtermIds.add(vtermId);
                    vnet.addInterface(ifId);
                    iconf = vnet.getInterface(ifId);
                }
                vnet.apply();

                String pid = pmap.getMappedPort();
                assertNotNull(pid);
                VnodeState state = (internalPorts.contains(pid))
                    ? VnodeState.DOWN : VnodeState.UP;
                iconf.setPortMap(pmap).
                    setState(state).
                    setEntityState(VnodeState.UP);
                assertEquals(VtnUpdateType.CREATED,
                             pmap.update(pmapSrv, ifId));
                vnet.verify();

                mapBridge = !mapBridge;
            }

            if (i == 0) {
                // Remove vBridge and all the vTerminals.
                removeVbridge(vbrId);
                vnet.removeBridge(vbrId).verify();

                for (VTerminalIdentifier vtermId: vtermIds) {
                    removeVterminal(vtermId);
                    vnet.removeTerminal(vtermId).verify();
                }
            } else {
                // Remove VTN.
                removeVtn(tname1);
                vnet.removeTenant(tname1).verify();
            }
        }
    }
}
