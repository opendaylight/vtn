/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import static org.opendaylight.vtn.manager.it.ofmock.OfMockService.ID_OPENFLOW;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

import org.opendaylight.vtn.manager.it.util.RpcErrorTag;
import org.opendaylight.vtn.manager.it.util.VirtualNetwork;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeIfIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VInterfaceConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VInterfaceIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VTNPortMapConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VTerminalIfIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.RemovePortMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.RemovePortMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.SetPortMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.SetPortMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.VtnPortMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;

/**
 * Test case for {@link VtnPortMapService}.
 */
public final class PortMapServiceTest extends TestMethodBase {
    /**
     * Construct a new instance.
     *
     * @param vit  A {@link VTNManagerIT} instance.
     */
    public PortMapServiceTest(VTNManagerIT vit) {
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

        // Create virtual interfaces.
        String tname1 = "vtn_1";
        String bname1 = "bridge_1";
        String bname2 = "bridge_2";
        String bname3 = "bridge_3";
        String bname4 = "bridge_4";
        String iname1 = "if_1";
        String iname2 = "if_2";
        String iname3 = "if_3";
        String[] bnames = {bname1, bname2, bname3, bname4};
        String[] inames = {iname1, iname2, iname3};

        List<VInterfaceIdentifier<?>> ifIds = new ArrayList<>();
        for (String bname: bnames) {
            for (String iname: inames) {
                VBridgeIfIdentifier bifId =
                    new VBridgeIfIdentifier(tname1, bname, iname);
                ifIds.add(bifId);
                vnet.addInterface(bifId);
            }

            VTerminalIfIdentifier tifId =
                new VTerminalIfIdentifier(tname1, bname, iname1);
            ifIds.add(tifId);
            vnet.addInterface(tifId);
        }

        vnet.apply().verify();

        // Set port mappings.
        Random rand = new Random(3141592L);
        String[] nodes = {
            ID_OPENFLOW + "12345",
            ID_OPENFLOW + "18446744073709551615",
            ID_OPENFLOW + "1",
            ID_OPENFLOW + "9999999",
        };
        String[] portIds = {
            "1", "2", "45", "4294967040",
        };
        String[] portNames = {
            "port-1", "ether-2", "if-4294967040",
        };
        Integer[] vlanIds = {
            null, 0, 123, 4095,
        };
        for (VInterfaceIdentifier<?> ifId: ifIds) {
            String node = nodes[rand.nextInt(nodes.length)];
            String id = portIds[rand.nextInt(portIds.length)];
            String name = portNames[rand.nextInt(portNames.length)];
            Integer vid = vlanIds[rand.nextInt(vlanIds.length)];

            int i = rand.nextInt(5);
            if (i == 0) {
                id = null;
            } else if (i == 1) {
                name = null;
            }

            VTNPortMapConfig pmap = new VTNPortMapConfig(node, id, name, vid);
            assertEquals(VtnUpdateType.CREATED, pmap.update(pmapSrv, ifId));
            vnet.getInterface(ifId).setPortMap(pmap);
            vnet.verify();

            // Try to set port mapping with the same parameter.
            assertEquals(null, pmap.update(pmapSrv, ifId));
            vnet.verify();
        }

        // Update port mappings.
        String anotherNode = ID_OPENFLOW + "5";
        String anotherPortId = "3";
        String anotherPortName = "port-12345";
        Integer anotherVid = 1;
        for (VInterfaceIdentifier<?> ifId: ifIds) {
            VInterfaceConfig iconf = vnet.getInterface(ifId);
            VTNPortMapConfig pmap = iconf.getPortMap();
            String node = pmap.getNode();
            String id = pmap.getPortId();
            String name = pmap.getPortName();
            Integer vid = pmap.getVlanId();

            pmap.setNode(anotherNode);
            assertEquals(VtnUpdateType.CHANGED, pmap.update(pmapSrv, ifId));
            vnet.verify();

            pmap.setPortId(anotherPortId);
            assertEquals(VtnUpdateType.CHANGED, pmap.update(pmapSrv, ifId));
            vnet.verify();

            pmap.setPortName(anotherPortName);
            assertEquals(VtnUpdateType.CHANGED, pmap.update(pmapSrv, ifId));
            vnet.verify();

            pmap.setVlanId(anotherVid);
            assertEquals(VtnUpdateType.CHANGED, pmap.update(pmapSrv, ifId));
            vnet.verify();

            pmap.setNode(node).
                setPortId(id).
                setPortName(name).
                setVlanId(vid);
            assertEquals(VtnUpdateType.CHANGED, pmap.update(pmapSrv, ifId));
            vnet.verify();
        }

        // Error tests.

        // Null input.
        checkRpcError(pmapSrv.setPortMap(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        checkRpcError(pmapSrv.removePortMap(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Null tenant-name.
        NodeId nodeId = new NodeId(nodes[0]);
        SetPortMapInput input = new SetPortMapInputBuilder().build();
        checkRpcError(pmapSrv.setPortMap(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        input = new SetPortMapInputBuilder().
            setBridgeName(bname1).
            setInterfaceName(iname1).
            setNode(nodeId).
            setPortId(portIds[0]).
            build();
        checkRpcError(pmapSrv.setPortMap(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        RemovePortMapInput rinput = new RemovePortMapInputBuilder().build();
        checkRpcError(pmapSrv.removePortMap(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        rinput = new RemovePortMapInputBuilder().
            setBridgeName(bname1).
            setInterfaceName(iname1).
            build();
        checkRpcError(pmapSrv.removePortMap(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // No bridge name.
        input = new SetPortMapInputBuilder().
            setTenantName(tname1).
            setInterfaceName(iname1).
            setNode(nodeId).
            setPortId(portIds[0]).
            build();
        checkRpcError(pmapSrv.setPortMap(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        rinput = new RemovePortMapInputBuilder().
            setTenantName(tname1).
            setInterfaceName(iname1).
            build();
        checkRpcError(pmapSrv.removePortMap(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Null interface-name.
        input = new SetPortMapInputBuilder().
            setTenantName(tname1).
            setBridgeName(bname1).
            setNode(nodeId).
            setPortId(portIds[0]).
            build();
        checkRpcError(pmapSrv.setPortMap(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        input = new SetPortMapInputBuilder().
            setTenantName(tname1).
            setTerminalName(bname1).
            setNode(nodeId).
            setPortId(portIds[0]).
            build();
        checkRpcError(pmapSrv.setPortMap(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        rinput = new RemovePortMapInputBuilder().
            setTenantName(tname1).
            setBridgeName(bname1).
            build();
        checkRpcError(pmapSrv.removePortMap(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        rinput = new RemovePortMapInputBuilder().
            setTenantName(tname1).
            setTerminalName(bname1).
            build();
        checkRpcError(pmapSrv.removePortMap(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        for (String name: INVALID_VNODE_NAMES) {
            // Invalid tenant-name.
            input = new SetPortMapInputBuilder().
                setTenantName(name).
                setBridgeName(bname1).
                setInterfaceName(iname1).
                setNode(nodeId).
                setPortId(portIds[0]).
                build();
            checkRpcError(pmapSrv.setPortMap(input),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            input = new SetPortMapInputBuilder().
                setTenantName(name).
                setTerminalName(bname1).
                setInterfaceName(iname1).
                setNode(nodeId).
                setPortId(portIds[0]).
                build();
            checkRpcError(pmapSrv.setPortMap(input),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            rinput = new RemovePortMapInputBuilder().
                setTenantName(name).
                setBridgeName(bname1).
                setInterfaceName(iname1).
                build();
            checkRpcError(pmapSrv.removePortMap(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            rinput = new RemovePortMapInputBuilder().
                setTenantName(name).
                setTerminalName(bname1).
                setInterfaceName(iname1).
                build();
            checkRpcError(pmapSrv.removePortMap(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            // Invalid bridge-name.
            input = new SetPortMapInputBuilder().
                setTenantName(tname1).
                setBridgeName(name).
                setInterfaceName(iname1).
                setNode(nodeId).
                setPortId(portIds[0]).
                build();
            checkRpcError(pmapSrv.setPortMap(input),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            rinput = new RemovePortMapInputBuilder().
                setTenantName(tname1).
                setBridgeName(name).
                setInterfaceName(iname1).
                build();
            checkRpcError(pmapSrv.removePortMap(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            // Invalid terminal-name.
            input = new SetPortMapInputBuilder().
                setTenantName(tname1).
                setTerminalName(name).
                setInterfaceName(iname1).
                setNode(nodeId).
                setPortId(portIds[0]).
                build();
            checkRpcError(pmapSrv.setPortMap(input),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            rinput = new RemovePortMapInputBuilder().
                setTenantName(tname1).
                setTerminalName(name).
                setInterfaceName(iname1).
                build();
            checkRpcError(pmapSrv.removePortMap(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            // Invalid interface-name.
            input = new SetPortMapInputBuilder().
                setTenantName(tname1).
                setBridgeName(bname1).
                setInterfaceName(name).
                setNode(nodeId).
                setPortId(portIds[0]).
                build();
            checkRpcError(pmapSrv.setPortMap(input),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            input = new SetPortMapInputBuilder().
                setTenantName(tname1).
                setTerminalName(bname1).
                setInterfaceName(name).
                setNode(nodeId).
                setPortId(portIds[0]).
                build();
            checkRpcError(pmapSrv.setPortMap(input),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            rinput = new RemovePortMapInputBuilder().
                setTenantName(tname1).
                setTerminalName(bname1).
                setInterfaceName(name).
                build();
            checkRpcError(pmapSrv.removePortMap(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            rinput = new RemovePortMapInputBuilder().
                setTenantName(tname1).
                setBridgeName(bname1).
                setInterfaceName(name).
                build();
            checkRpcError(pmapSrv.removePortMap(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);
        }

        // Specifying virtual interface that is not present.
        String unknownName = "unknown";
        input = new SetPortMapInputBuilder().
            setTenantName(unknownName).
            setBridgeName(bname1).
            setInterfaceName(iname1).
            setNode(nodeId).
            setPortId(portIds[0]).
            build();
        checkRpcError(pmapSrv.setPortMap(input),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        input = new SetPortMapInputBuilder().
            setTenantName(unknownName).
            setTerminalName(bname1).
            setInterfaceName(iname1).
            setNode(nodeId).
            setPortId(portIds[0]).
            build();
        checkRpcError(pmapSrv.setPortMap(input),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        rinput = new RemovePortMapInputBuilder().
            setTenantName(unknownName).
            setBridgeName(bname1).
            setInterfaceName(iname1).
            build();
        checkRpcError(pmapSrv.removePortMap(rinput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        rinput = new RemovePortMapInputBuilder().
            setTenantName(unknownName).
            setTerminalName(bname1).
            setInterfaceName(iname1).
            build();
        checkRpcError(pmapSrv.removePortMap(rinput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        input = new SetPortMapInputBuilder().
            setTenantName(tname1).
            setBridgeName(unknownName).
            setInterfaceName(iname1).
            setNode(nodeId).
            setPortId(portIds[0]).
            build();
        checkRpcError(pmapSrv.setPortMap(input),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        input = new SetPortMapInputBuilder().
            setTenantName(tname1).
            setTerminalName(unknownName).
            setInterfaceName(iname1).
            setNode(nodeId).
            setPortId(portIds[0]).
            build();
        checkRpcError(pmapSrv.setPortMap(input),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        rinput = new RemovePortMapInputBuilder().
            setTenantName(tname1).
            setBridgeName(unknownName).
            setInterfaceName(iname1).
            build();
        checkRpcError(pmapSrv.removePortMap(rinput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        rinput = new RemovePortMapInputBuilder().
            setTenantName(tname1).
            setTerminalName(unknownName).
            setInterfaceName(iname1).
            build();
        checkRpcError(pmapSrv.removePortMap(rinput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        input = new SetPortMapInputBuilder().
            setTenantName(tname1).
            setBridgeName(bname1).
            setInterfaceName(unknownName).
            setNode(nodeId).
            setPortId(portIds[0]).
            build();
        checkRpcError(pmapSrv.setPortMap(input),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        input = new SetPortMapInputBuilder().
            setTenantName(tname1).
            setTerminalName(bname1).
            setInterfaceName(unknownName).
            setNode(nodeId).
            setPortId(portIds[0]).
            build();
        checkRpcError(pmapSrv.setPortMap(input),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        rinput = new RemovePortMapInputBuilder().
            setTenantName(tname1).
            setBridgeName(bname1).
            setInterfaceName(unknownName).
            build();
        checkRpcError(pmapSrv.removePortMap(rinput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        rinput = new RemovePortMapInputBuilder().
            setTenantName(tname1).
            setTerminalName(bname1).
            setInterfaceName(unknownName).
            build();
        checkRpcError(pmapSrv.removePortMap(rinput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        // No node.
        input = new SetPortMapInputBuilder().
            setTenantName(tname1).
            setTerminalName(bname1).
            setInterfaceName(iname1).
            setPortId(portIds[0]).
            build();
        checkRpcError(pmapSrv.setPortMap(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Both port-id and port-name are null.
        input = new SetPortMapInputBuilder().
            setTenantName(tname1).
            setTerminalName(bname1).
            setInterfaceName(iname1).
            setNode(nodeId).
            build();
        checkRpcError(pmapSrv.setPortMap(input),
                      RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);

        // Invalid node ID.
        for (String node: INVALID_NODE_IDS) {
            input = new SetPortMapInputBuilder().
                setTenantName(tname1).
                setTerminalName(bname1).
                setInterfaceName(iname1).
                setNode(new NodeId(node)).
                setPortName(portNames[0]).
                build();
            checkRpcError(pmapSrv.setPortMap(input),
                          RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);
        }

        // Invalid port ID.
        for (String port: INVALID_PORT_IDS) {
            input = new SetPortMapInputBuilder().
                setTenantName(tname1).
                setTerminalName(bname1).
                setInterfaceName(iname1).
                setNode(nodeId).
                setPortId(port).
                build();
            checkRpcError(pmapSrv.setPortMap(input),
                          RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);
        }

        // Errors should never affect existing port mappings.
        vnet.verify();

        // Remove all port mappings.
        for (VInterfaceIdentifier<?> ifId: ifIds) {
            assertEquals(VtnUpdateType.REMOVED, removePortMap(ifId));
            VInterfaceConfig iconf = vnet.getInterface(ifId);
            iconf.setPortMap(null);
            vnet.verify();

            // Try to remove port mapping that was already removed.
            assertEquals(null, removePortMap(ifId));
        }

        // Remove VTN.
        removeVtn(tname1);
        vnet.removeTenant(tname1).verify();
    }
}
