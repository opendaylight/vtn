/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;
import static org.mockito.MockitoAnnotations.initMocks;

import static org.opendaylight.vtn.manager.neutron.impl.NeutronConfigTest.DEFAULT_BRIDGE_NAME;

import java.util.AbstractMap.SimpleEntry;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Set;
import java.util.UUID;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.mockito.Mock;

import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import org.opendaylight.controller.md.sal.binding.api.DataObjectModification.ModificationType;
import org.opendaylight.controller.md.sal.binding.api.DataObjectModification;
import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.ports.rev150712.ports.attributes.Ports;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.ports.rev150712.ports.attributes.ports.Port;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.ports.rev150712.ports.attributes.ports.PortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.rev150712.Neutron;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev130715.Uuid;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.DatapathId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbBridgeAugmentation;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbBridgeAugmentationBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbBridgeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbNodeAugmentation;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbNodeAugmentationBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbTerminationPointAugmentation;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbTerminationPointAugmentationBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.port._interface.attributes.InterfaceExternalIds;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.port._interface.attributes.InterfaceExternalIdsBuilder;

import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.NodeId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.TpId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.Node;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.NodeBuilder;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.node.TerminationPoint;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.node.TerminationPointBuilder;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.node.attributes.SupportingNode;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.node.attributes.SupportingNodeBuilder;

/**
 * JUnit test for {@link OvsdbNodeChange}.
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({OVSDBEventHandler.class, ReadTransactionHolder.class})
public class OvsdbNodeChangeTest extends TestBase {
    /**
     * The OVSDB external ID associated with the system identifier of the
     * interface.
     */
    private static final String  EXTID_INTERFACE_ID = "iface-id";

    /**
     * Mock-up of {@link ReadTransaction}.
     */
    @Mock
    private ReadTransaction  readTx;

    /**
     * Mock-up of {@link OVSDBEventHandler}.
     */
    private OVSDBEventHandler  ovsdbHandler;

    /**
     * Mock-up of {@link ReadTransactionHolder}.
     */
    private ReadTransactionHolder  txHolder;

    /**
     * The UUID used to create unique UUID.
     */
    private UUID  currentUuid;

    /**
     * Describes port mapping attribtes.
     */
    private final class PortAttr {
        /**
         * The target neutron port.
         */
        private final Port  targetPort;

        /**
         * Path to the target port.
         */
        private InstanceIdentifier<Port>  portPath;

        /**
         * OpenFlow port identifier.
         */
        private final Long  portId;

        /**
         * The name of the port.
         */
        private final String  portName;

        /**
         * The termination point associated with the target port.
         */
        private final TerminationPoint  targetTp;

        /**
         * Set true if the port should be mapped successfully.
         */
        private boolean  succeeded;

        /**
         * Construct a new instance with specifying OpenFlow port ID.
         *
         * @param uuid  UUID that specifies the neutron port.
         * @param id    OpenFlow port identifier.
         */
        private PortAttr(String uuid, Long id) {
            this(uuid, id, null);
        }

        /**
         * Construct a new instance with specifying port name.
         *
         * @param uuid  UUID that specifies the neutron port.
         * @param name  The name of the port.
         */
        private PortAttr(String uuid, String name) {
            this(uuid, null, name);
        }

        /**
         * Construct a new instance with specifying both OpenFlow port ID and
         * port name.
         *
         * @param uuid  UUID that specifies the neutron port.
         * @param id    OpenFlow port identifier.
         * @param name  The name of the port.
         */
        private PortAttr(String uuid, Long id, String name) {
            targetPort = newPort(uuid);
            portPath = InstanceIdentifier.
                builder(Neutron.class).
                child(Ports.class).
                child(Port.class, targetPort.getKey()).
                build();
            portId = id;
            portName = name;
            targetTp = newTerminationPoint(uuid, id, name);
        }

        /**
         * Return the UUID that specifies the neutron port.
         *
         * @return  The UUID that specifies the neutron port.
         */
        private String getUuid() {
            return targetPort.getUuid().getValue();
        }

        /**
         * Return the termination point associated with the target port.
         *
         * @return  The termination point.
         */
        private TerminationPoint getTerminationPoint() {
            return targetTp;
        }

        /**
         * Set the succeeded flag.
         */
        private void setSucceeded() {
            succeeded = true;
        }

        /**
         * Prepare the mock-up for test.
         */
        private void prepare() {
            LogicalDatastoreType store = LogicalDatastoreType.CONFIGURATION;
            when(readTx.read(store, portPath)).
                thenReturn(getReadResult(targetPort));
            succeeded = true;
        }

        /**
         * Prepare the mock-up for test that expects the target port is not
         * found.
         */
        private void prepareNotFound() {
            LogicalDatastoreType store = LogicalDatastoreType.CONFIGURATION;
            when(readTx.read(store, portPath)).
                thenReturn(getReadResult(null));
        }

        /**
         * Prepare the mock-up for test that causes error on port read.
         */
        private void prepareFailure() {
            IllegalStateException cause = new IllegalStateException();
            LogicalDatastoreType store = LogicalDatastoreType.CONFIGURATION;
            when(readTx.read(store, portPath)).
                thenReturn(getReadFailure(Port.class, cause));
        }

        /**
         * Verify the port read operation.
         */
        private void checkRead() {
            checkRead(1);
        }

        /**
         * Verify the port read operation.
         *
         * @param count  The number expected invocations of read operation.
         */
        private void checkRead(int count) {
            LogicalDatastoreType store = LogicalDatastoreType.CONFIGURATION;
            verify(readTx, times(count)).read(store, portPath);
        }

        /**
         * Ensure that the port mapping has been configured.
         *
         * @param onode  An {@link OfNode} instance that specifies the
         *               OpenFlow switch.
         */
        private void checkMapped(OfNode onode) {
            if (succeeded) {
                verify(ovsdbHandler).
                    setPortMap(targetPort, onode, portId, portName);
            }
        }

        /**
         * Ensure that the port mapping has been unconfigured.
         */
        private void checkUnmapped() {
            if (succeeded) {
                verify(ovsdbHandler).deletePortMap(targetPort);
            }
        }
    }

    /**
     * Set up test environment.
     */
    @Before
    public void setUp() {
        initMocks(this);
        ovsdbHandler = PowerMockito.mock(OVSDBEventHandler.class);
        txHolder = PowerMockito.mock(ReadTransactionHolder.class);
        when(txHolder.get()).thenReturn(readTx);
    }

    /**
     * Test case for
     * {@link OvsdbNodeChange#getPortId(OvsdbTerminationPointAugmentation)}.
     */
    @Test
    public void testGetPortId() {
        // No interface external ID list.
        OvsdbTerminationPointAugmentation ovtp =
            new OvsdbTerminationPointAugmentationBuilder().build();
        assertEquals(null, OvsdbNodeChange.getPortId(ovtp));

        // No interface ID in the interface external ID list.
        List<InterfaceExternalIds> list = new ArrayList<>();
        for (int i = 0; i < 10; i++) {
            InterfaceExternalIds extId =
                newIfExternalId("invalid-key-" + i, "invalid-value-" + i);
            list.add(extId);
        }
        ovtp = new OvsdbTerminationPointAugmentationBuilder().
            setInterfaceExternalIds(list).
            build();
        assertEquals(null, OvsdbNodeChange.getPortId(ovtp));

        // Interface ID is present in the interface external ID list.
        String value = "eth2";
        list.add(newIfExternalId(EXTID_INTERFACE_ID, value));
        ovtp = new OvsdbTerminationPointAugmentationBuilder().
            setInterfaceExternalIds(list).
            build();
        assertEquals(value, OvsdbNodeChange.getPortId(ovtp));
    }

    /**
     * Test case for
     * {@link OvsdbNodeChange#isChanged(OvsdbTerminationPointAugmentation,OvsdbTerminationPointAugmentation)}.
     */
    @Test
    public void testIsChanged() {
        // Compare empty instances.
        OvsdbTerminationPointAugmentationBuilder beforeBuilder =
            new OvsdbTerminationPointAugmentationBuilder();
        OvsdbTerminationPointAugmentationBuilder afterBuilder =
            new OvsdbTerminationPointAugmentationBuilder();
        OvsdbTerminationPointAugmentation before = beforeBuilder.build();
        OvsdbTerminationPointAugmentation after = afterBuilder.build();
        assertEquals(false, OvsdbNodeChange.isChanged(before, after));

        // Add interface external IDs that are not interface ID.
        List<InterfaceExternalIds> list = new ArrayList<>();
        Collections.addAll(
            list, newIfExternalId("key1", "value1"),
            newIfExternalId("key2", "value2"));
        after = afterBuilder.setInterfaceExternalIds(list).build();
        assertEquals(false, OvsdbNodeChange.isChanged(before, after));
        before = beforeBuilder.setInterfaceExternalIds(list).build();
        assertEquals(false, OvsdbNodeChange.isChanged(before, after));

        // Add interface ID.
        String ifId = "eth0";
        list = new ArrayList<>(list);
        list.add(newIfExternalId(EXTID_INTERFACE_ID, ifId));
        after = afterBuilder.setInterfaceExternalIds(list).build();
        assertEquals(true, OvsdbNodeChange.isChanged(before, after));
        before = beforeBuilder.setInterfaceExternalIds(list).build();
        assertEquals(false, OvsdbNodeChange.isChanged(before, after));

        // Change OpenFlow port ID.
        Long portId = 123L;
        after = afterBuilder.setOfport(portId).build();
        assertEquals(true, OvsdbNodeChange.isChanged(before, after));
        before = beforeBuilder.setOfport(portId).build();
        assertEquals(false, OvsdbNodeChange.isChanged(before, after));

        // Change port name.
        String portName = "port-1";
        after = afterBuilder.setName(portName).build();
        assertEquals(true, OvsdbNodeChange.isChanged(before, after));
        before = beforeBuilder.setName(portName).build();
        assertEquals(false, OvsdbNodeChange.isChanged(before, after));

        // Change OpenFlow port ID again.
        portId = 124L;
        after = afterBuilder.setOfport(portId).build();
        assertEquals(true, OvsdbNodeChange.isChanged(before, after));
        before = beforeBuilder.setOfport(portId).build();
        assertEquals(false, OvsdbNodeChange.isChanged(before, after));

        // Change port name again.
        portName = "port-2";
        after = afterBuilder.setName(portName).build();
        assertEquals(true, OvsdbNodeChange.isChanged(before, after));
        before = beforeBuilder.setName(portName).build();
        assertEquals(false, OvsdbNodeChange.isChanged(before, after));

        // Change interface ID.
        ifId = "eth1";
        list = Collections.singletonList(
            newIfExternalId(EXTID_INTERFACE_ID, ifId));
        after = afterBuilder.setInterfaceExternalIds(list).build();
        assertEquals(true, OvsdbNodeChange.isChanged(before, after));
        before = beforeBuilder.setInterfaceExternalIds(list).build();
        assertEquals(false, OvsdbNodeChange.isChanged(before, after));
    }

    /**
     * Test case for {@link OvsdbNodeChange#getOfNode(OVSDBEventHandler,Node)}.
     */
    @Test
    public void testGetOfNode() {
        // No OVSDB bridge augmentation.
        Node node = new NodeBuilder().build();
        assertEquals(null, OvsdbNodeChange.getOfNode(ovsdbHandler, node));
        verifyZeroInteractions(ovsdbHandler, txHolder, readTx);

        // No DPID.
        OvsdbBridgeAugmentation ovbr = newBridgeAugmentation(null);
        node = new NodeBuilder().
            addAugmentation(OvsdbBridgeAugmentation.class, ovbr).
            build();
        assertEquals(null, OvsdbNodeChange.getOfNode(ovsdbHandler, node));
        verify(ovsdbHandler).getOvsdbBridgeName();
        verifyNoMoreInteractions(ovsdbHandler, txHolder, readTx);

        // Successful completion.
        Map<String, Long> cases = new HashMap<>();
        cases.put("00:11:22:33:44:55:66:77", 0x0011223344556677L);
        cases.put("ff:11:22:33:ab:cd:ef:89", 0xff112233abcdef89L);
        cases.put("aa:bb:cc:dd:01:23:45:67", 0xaabbccdd01234567L);
        for (Entry<String, Long> entry: cases.entrySet()) {
            OfNode expected = new OfNode(entry.getValue().longValue());

            reset(ovsdbHandler);
            ovbr = newBridgeAugmentation(entry.getKey());
            node = new NodeBuilder().
                addAugmentation(OvsdbBridgeAugmentation.class, ovbr).
                build();
            assertEquals(expected,
                         OvsdbNodeChange.getOfNode(ovsdbHandler, node));
            verify(ovsdbHandler).getOvsdbBridgeName();
            verifyNoMoreInteractions(ovsdbHandler, txHolder, readTx);
        }
    }

    /**
     * Test case for
     * {@link OvsdbNodeChange#getBridgeNode(OVSDBEventHandler,Node)}.
     */
    @Test
    public void testGetBridgeNode() {
        // No OVSDB bridge augmentation.
        Node node = new NodeBuilder().build();
        assertEquals(null, OvsdbNodeChange.getBridgeNode(ovsdbHandler, node));
        verifyZeroInteractions(ovsdbHandler, txHolder, readTx);

        // No bridge name.
        OvsdbBridgeAugmentation ovbr = new OvsdbBridgeAugmentationBuilder().
            build();
        node = new NodeBuilder().
            addAugmentation(OvsdbBridgeAugmentation.class, ovbr).
            build();
        assertEquals(null, OvsdbNodeChange.getBridgeNode(ovsdbHandler, node));
        verifyZeroInteractions(ovsdbHandler, txHolder, readTx);

        // Bridge name does not match.
        String[] bridges = {"bridge_1", "bridge_2", DEFAULT_BRIDGE_NAME + "1"};
        for (String bname: bridges) {
            reset(ovsdbHandler);
            ovbr = newBridgeAugmentation(null, bname);
            node = new NodeBuilder().
                addAugmentation(OvsdbBridgeAugmentation.class, ovbr).
                build();
            assertEquals(null, OvsdbNodeChange.getBridgeNode(
                             ovsdbHandler, node));
            verify(ovsdbHandler).getOvsdbBridgeName();
            verifyNoMoreInteractions(ovsdbHandler, txHolder, readTx);
        }

        // Found.
        reset(ovsdbHandler);
        ovbr = newBridgeAugmentation(null);
        node = new NodeBuilder().
            addAugmentation(OvsdbBridgeAugmentation.class, ovbr).
            build();
        assertEquals(ovbr, OvsdbNodeChange.getBridgeNode(ovsdbHandler, node));
        verify(ovsdbHandler).getOvsdbBridgeName();
        verifyNoMoreInteractions(ovsdbHandler, txHolder, readTx);
    }

    /**
     * Test case for
     * {@link OvsdbNodeChange#nodeCreated(OVSDBEventHandler,ReadTransactionHolder,Node)}.
     *
     * <ul>
     *   <li>Node is empty.</li>
     * </ul>
     */
    @Test
    public void testNodeCreated1() {
        Node node = new NodeBuilder().build();
        assertEquals(null, OvsdbNodeChange.nodeCreated(
                         ovsdbHandler, txHolder, node));
        verifyZeroInteractions(ovsdbHandler, txHolder, readTx);
    }

    /**
     * Test case for
     * {@link OvsdbNodeChange#nodeCreated(OVSDBEventHandler,ReadTransactionHolder,Node)}.
     *
     * <ul>
     *   <li>No OVSDB node augmentation.</li>
     *   <li>Node contains termination points and DPID.</li>
     * </ul>
     */
    @Test
    public void testNodeCreated2() {
        DatapathId dpid = new DatapathId("00:00:11:11:22:22:33:33");
        OvsdbBridgeAugmentation ovbr =
            newBridgeAugmentation("00:00:11:11:22:22:33:33");
        List<TerminationPoint> tps = new ArrayList<>();
        for (long id = 1L; id <= 3L; id++) {
            tps.add(newTerminationPoint(uniqueUuid(), id, "port-" + id));
        }

        Node node = new NodeBuilder().
            setTerminationPoint(tps).
            addAugmentation(OvsdbBridgeAugmentation.class, ovbr).
            build();
        assertEquals(null, OvsdbNodeChange.nodeCreated(
                         ovsdbHandler, txHolder, node));
        verifyZeroInteractions(ovsdbHandler, txHolder, readTx);
    }

    /**
     * Test case for
     * {@link OvsdbNodeChange#nodeCreated(OVSDBEventHandler,ReadTransactionHolder,Node)}.
     *
     * <ul>
     *   <li>OVSDB node augmentation is present.</li>
     *   <li>No termination point in node.</li>
     * </ul>
     */
    @Test
    public void testNodeCreated3() {
        OvsdbBridgeAugmentation ovbr =
            newBridgeAugmentation("01:23:45:67:89:ab:cd:ef");
        OvsdbNodeAugmentation ovnode = new OvsdbNodeAugmentationBuilder().
            build();
        Node node = new NodeBuilder().
            setNodeId(new NodeId("ovsdb:node:created3")).
            addAugmentation(OvsdbNodeAugmentation.class, ovnode).
            addAugmentation(OvsdbBridgeAugmentation.class, ovbr).
            build();
        OvsdbNodeChange ovchg =
            OvsdbNodeChange.nodeCreated(ovsdbHandler, txHolder, node);
        assertNotNull(ovchg);
        verify(ovsdbHandler).getOvsdbBridgeName();
        verifyNoMoreInteractions(ovsdbHandler, txHolder, readTx);
        ovchg.apply();
        verify(ovsdbHandler).nodeAdded(node, ovnode);
        verifyNoMoreInteractions(ovsdbHandler, txHolder, readTx);
    }

    /**
     * Test case for
     * {@link OvsdbNodeChange#nodeCreated(OVSDBEventHandler,ReadTransactionHolder,Node)}.
     *
     * <ul>
     *   <li>OVSDB node augmentation is present.</li>
     *   <li>Node contains termination points but DPID.</li>
     * </ul>
     */
    @Test
    public void testNodeCreated4() {
        OvsdbBridgeAugmentation ovbr = newBridgeAugmentation(null);
        OvsdbNodeAugmentation ovnode = new OvsdbNodeAugmentationBuilder().
            build();
        List<TerminationPoint> tps = new ArrayList<>();
        for (long id = 1L; id <= 3L; id++) {
            tps.add(newTerminationPoint(uniqueUuid(), id, "port-" + id));
        }

        Node node = new NodeBuilder().
            setNodeId(new NodeId("ovsdb:node:created4")).
            setTerminationPoint(tps).
            addAugmentation(OvsdbNodeAugmentation.class, ovnode).
            addAugmentation(OvsdbBridgeAugmentation.class, ovbr).
            build();
        OvsdbNodeChange ovchg =
            OvsdbNodeChange.nodeCreated(ovsdbHandler, txHolder, node);
        assertNotNull(ovchg);
        verify(ovsdbHandler).getOvsdbBridgeName();
        verifyNoMoreInteractions(ovsdbHandler, txHolder, readTx);
        ovchg.apply();
        verify(ovsdbHandler).nodeAdded(node, ovnode);
        verifyNoMoreInteractions(ovsdbHandler, txHolder, readTx);
    }

    /**
     * Test case for
     * {@link OvsdbNodeChange#nodeCreated(OVSDBEventHandler,ReadTransactionHolder,Node)}.
     *
     * <ul>
     *   <li>OVSDB node augmentation is present.</li>
     *   <li>Port mappings are configured for neutron ports.</li>
     * </ul>
     */
    @Test
    public void testNodeCreated5() {
        OvsdbBridgeAugmentation ovbr =
            newBridgeAugmentation("aa:bb:cc:dd:12:34:56:78");
        OvsdbNodeAugmentation ovnode = new OvsdbNodeAugmentationBuilder().
            build();
        List<TerminationPoint> tps = new ArrayList<>();
        Map<String, PortAttr> pattrs = new HashMap<>();
        for (long id = 1L; id <= 10L; id++) {
            String uuid = uniqueUuid();
            PortAttr pattr;
            if ((id % 3) == 0) {
                pattr = new PortAttr(uuid, id);
            } else if ((id % 5) == 0) {
                pattr = new PortAttr(uuid, "port-" + id);
            } else {
                pattr = new PortAttr(uuid, id, "port-" + id);
            }
            assertNull(pattrs.put(uuid, pattr));
            tps.add(pattr.getTerminationPoint());
            pattr.prepare();
        }

        // Below termination points should be ignored.

        // No OVSDB termination point augmentation.
        tps.add(new TerminationPointBuilder().build());

        // No port ID in the OVSDB termination point augmentation.
        tps.add(newTerminationPoint(null, 100L, null));

        // Invalid port UUID.
        tps.add(newTerminationPoint("invalid-uuid", 200L, "port-200"));

        // The target port not found.
        PortAttr pattr = new PortAttr(uniqueUuid(), 300L, "port-300");
        assertNull(pattrs.put(pattr.getUuid(), pattr));
        tps.add(pattr.getTerminationPoint());
        pattr.prepareNotFound();

        // The target port is not readable.
        pattr = new PortAttr(uniqueUuid(), 400L, "port-400");
        assertNull(pattrs.put(pattr.getUuid(), pattr));
        tps.add(pattr.getTerminationPoint());
        pattr.prepareFailure();

        Node node = new NodeBuilder().
            setNodeId(new NodeId("ovsdb:node:created5")).
            setTerminationPoint(tps).
            addAugmentation(OvsdbNodeAugmentation.class, ovnode).
            addAugmentation(OvsdbBridgeAugmentation.class, ovbr).
            build();
        OvsdbNodeChange ovchg =
            OvsdbNodeChange.nodeCreated(ovsdbHandler, txHolder, node);
        assertNotNull(ovchg);
        verify(ovsdbHandler).getOvsdbBridgeName();
        for (PortAttr pa: pattrs.values()) {
            pa.checkRead();
        }
        verifyNoMoreInteractions(ovsdbHandler, readTx);
        ovchg.apply();
        verify(ovsdbHandler).nodeAdded(node, ovnode);
        OfNode onode = new OfNode(0xaabbccdd12345678L);
        for (PortAttr pa: pattrs.values()) {
            pa.checkMapped(onode);
        }
        verifyNoMoreInteractions(ovsdbHandler, readTx);
    }

    /**
     * Test case for
     * {@link OvsdbNodeChange#nodeRemoved(OVSDBEventHandler,ReadTransactionHolder,Node)}.
     *
     * <ul>
     *   <li>Node is empty.</li>
     * </ul>
     */
    @Test
    public void testNodeRemoved1() {
        Node node = new NodeBuilder().build();
        assertEquals(null, OvsdbNodeChange.nodeRemoved(
                         ovsdbHandler, txHolder, node));
        verifyZeroInteractions(ovsdbHandler, txHolder, readTx);
    }

    /**
     * Test case for
     * {@link OvsdbNodeChange#nodeRemoved(OVSDBEventHandler,ReadTransactionHolder,Node)}.
     *
     * <ul>
     *   <li>No OVSDB node augmentation.</li>
     *   <li>Node contains termination points and DPID.</li>
     * </ul>
     */
    @Test
    public void testNodeRemoved2() {
        OvsdbBridgeAugmentation ovbr =
            newBridgeAugmentation("99:88:77:66:55:44:33:22");
        List<TerminationPoint> tps = new ArrayList<>();
        for (long id = 1L; id <= 3L; id++) {
            tps.add(newTerminationPoint(uniqueUuid(), id, "port-" + id));
        }

        Node node = new NodeBuilder().
            setTerminationPoint(tps).
            addAugmentation(OvsdbBridgeAugmentation.class, ovbr).
            build();
        assertEquals(null, OvsdbNodeChange.nodeRemoved(
                         ovsdbHandler, txHolder, node));
        verifyZeroInteractions(ovsdbHandler, txHolder, readTx);
    }

    /**
     * Test case for
     * {@link OvsdbNodeChange#nodeRemoved(OVSDBEventHandler,ReadTransactionHolder,Node)}.
     *
     * <ul>
     *   <li>OVSDB node augmentation is present.</li>
     *   <li>No termination point in node.</li>
     * </ul>
     */
    @Test
    public void testNodeRemoved3() {
        OvsdbBridgeAugmentation ovbr =
            newBridgeAugmentation("aa:bb:cc:dd:44:55:66:77");
        OvsdbNodeAugmentation ovnode = new OvsdbNodeAugmentationBuilder().
            build();
        Node node = new NodeBuilder().
            setNodeId(new NodeId("ovsdb:node:removed3")).
            addAugmentation(OvsdbNodeAugmentation.class, ovnode).
            addAugmentation(OvsdbBridgeAugmentation.class, ovbr).
            build();
        OvsdbNodeChange ovchg =
            OvsdbNodeChange.nodeRemoved(ovsdbHandler, txHolder, node);
        assertNotNull(ovchg);
        verifyZeroInteractions(ovsdbHandler, txHolder, readTx);
        ovchg.apply();
        verify(ovsdbHandler).nodeRemoved(node);
        verifyNoMoreInteractions(ovsdbHandler, txHolder, readTx);
    }

    /**
     * Test case for
     * {@link OvsdbNodeChange#nodeRemoved(OVSDBEventHandler,ReadTransactionHolder,Node)}.
     *
     * <ul>
     *   <li>OVSDB node augmentation is present.</li>
     *   <li>Port mappings are unconfigured for neutron ports.</li>
     * </ul>
     */
    @Test
    public void testNodeRemoved4() {
        OvsdbBridgeAugmentation ovbr =
            newBridgeAugmentation("ff:ee:dd:cc:bb:aa:99:88");
        OvsdbNodeAugmentation ovnode = new OvsdbNodeAugmentationBuilder().
            build();
        List<TerminationPoint> tps = new ArrayList<>();
        Map<String, PortAttr> pattrs = new HashMap<>();
        for (long id = 1L; id <= 10L; id++) {
            String uuid = uniqueUuid();
            PortAttr pattr = new PortAttr(uuid, id, "port-" + id);
            assertNull(pattrs.put(uuid, pattr));
            tps.add(pattr.getTerminationPoint());
            pattr.prepare();
        }

        // Below termination points should be ignored.

        // No OVSDB termination point augmentation.
        tps.add(new TerminationPointBuilder().build());

        // No port ID in the OVSDB termination point augmentation.
        tps.add(newTerminationPoint(null, 1000L, null));

        // Invalid port UUID.
        tps.add(newTerminationPoint("invalid-uuid", 2000L, "port-2000"));

        // The target port not found.
        PortAttr pattr = new PortAttr(uniqueUuid(), 3000L, "port-3000");
        assertNull(pattrs.put(pattr.getUuid(), pattr));
        tps.add(pattr.getTerminationPoint());
        pattr.prepareNotFound();

        // The target port is not readable.
        pattr = new PortAttr(uniqueUuid(), 4000L, "port-4000");
        assertNull(pattrs.put(pattr.getUuid(), pattr));
        tps.add(pattr.getTerminationPoint());
        pattr.prepareFailure();

        Node node = new NodeBuilder().
            setNodeId(new NodeId("ovsdb:node:removed4")).
            setTerminationPoint(tps).
            addAugmentation(OvsdbNodeAugmentation.class, ovnode).
            addAugmentation(OvsdbBridgeAugmentation.class, ovbr).
            build();
        OvsdbNodeChange ovchg =
            OvsdbNodeChange.nodeRemoved(ovsdbHandler, txHolder, node);
        assertNotNull(ovchg);
        for (PortAttr pa: pattrs.values()) {
            pa.checkRead();
        }
        verifyNoMoreInteractions(ovsdbHandler, readTx);
        ovchg.apply();
        verify(ovsdbHandler).nodeRemoved(node);
        for (PortAttr pa: pattrs.values()) {
            pa.checkUnmapped();
        }
        verifyNoMoreInteractions(ovsdbHandler, readTx);
    }

    /**
     * Test case for
     * {@link OvsdbNodeChange#nodeUpdated(OVSDBEventHandler,ReadTransactionHolder,DataObjectModification,Node,Node)}.
     *
     * <ul>
     *   <li>
     *     OVSDB node augmentation is not present before and after
     *     modification.
     *   </li>
     * </ul>
     */
    @Test
    public void testNodeUpdated1() {
        List<TerminationPoint> tps = new ArrayList<>();
        for (long id = 1L; id <= 3L; id++) {
            tps.add(newTerminationPoint(uniqueUuid(), id, "port-" + id));
        }
        NodeId nodeId = new NodeId("ovsdb:node:1");
        Node before = new NodeBuilder().
            setNodeId(nodeId).
            setTerminationPoint(tps).
            build();

        tps = new ArrayList<>(tps);
        TerminationPoint newTp = newTerminationPoint(uniqueUuid(), 10L, null);
        Node after = new NodeBuilder().
            setNodeId(nodeId).
            setTerminationPoint(tps).
            build();

        DataObjectModification<TerminationPoint> tpMod =
            newKeyedModification(null, newTp, null);
        List<DataObjectModification<?>> children =
            Collections.singletonList(tpMod);
        DataObjectModification<Node> mod =
            newKeyedModification(before, after, children);
        assertEquals(null, OvsdbNodeChange.nodeUpdated(
                         ovsdbHandler, txHolder, mod, before, after));
        verifyZeroInteractions(ovsdbHandler, txHolder, readTx);
    }

    /**
     * Test case for
     * {@link OvsdbNodeChange#nodeUpdated(OVSDBEventHandler,ReadTransactionHolder,DataObjectModification,Node,Node)}.
     *
     * <ul>
     *   <li>OVSDB node augmentation has been created.</li>
     *   <li>No termination point in node.</li>
     * </ul>
     */
    @Test
    public void testNodeUpdated2() {
        OvsdbBridgeAugmentation ovbr =
            newBridgeAugmentation("33:44:55:66:cc:dd:ee:ff");
        OvsdbNodeAugmentation ovnode = new OvsdbNodeAugmentationBuilder().
            build();
        NodeId nodeId = new NodeId("ovsdb:node:1");
        Node before = new NodeBuilder().
            setNodeId(nodeId).
            addAugmentation(OvsdbBridgeAugmentation.class, ovbr).
            build();
        Node after = new NodeBuilder().
            setNodeId(nodeId).
            addAugmentation(OvsdbNodeAugmentation.class, ovnode).
            addAugmentation(OvsdbBridgeAugmentation.class, ovbr).
            build();
        DataObjectModification<OvsdbNodeAugmentation> nmod =
            newItemModification(null, ovnode, null);
        List<DataObjectModification<?>> children =
            Collections.singletonList(nmod);
        DataObjectModification<Node> mod =
            newKeyedModification(before, after, children);
        OvsdbNodeChange ovchg = OvsdbNodeChange.nodeUpdated(
            ovsdbHandler, txHolder, mod, before, after);
        assertNotNull(ovchg);
        verify(ovsdbHandler).getOvsdbBridgeName();
        verifyNoMoreInteractions(ovsdbHandler, txHolder, readTx);
        ovchg.apply();
        verify(ovsdbHandler).nodeAdded(after, ovnode);
        verifyNoMoreInteractions(ovsdbHandler, txHolder, readTx);
    }

    /**
     * Test case for
     * {@link OvsdbNodeChange#nodeUpdated(OVSDBEventHandler,ReadTransactionHolder,DataObjectModification,Node,Node)}.
     *
     * <ul>
     *   <li>OVSDB node augmentation has been created.</li>
     *   <li>Port mappings are configured for neutron ports.</li>
     * </ul>
     */
    @Test
    public void testNodeUpdated3() {
        OvsdbBridgeAugmentation ovbr =
            newBridgeAugmentation("aa:ff:bb:ee:cc:dd:01:23");
        OvsdbNodeAugmentation ovnode = new OvsdbNodeAugmentationBuilder().
            build();
        List<TerminationPoint> tps = new ArrayList<>();
        Map<String, PortAttr> pattrs = new HashMap<>();
        for (long id = 1L; id <= 13L; id++) {
            String uuid = uniqueUuid();
            PortAttr pattr;
            if ((id % 3) == 0) {
                pattr = new PortAttr(uuid, id);
            } else if ((id % 5) == 0) {
                pattr = new PortAttr(uuid, "port-" + id);
            } else {
                pattr = new PortAttr(uuid, id, "port-" + id);
            }
            assertNull(pattrs.put(uuid, pattr));
            tps.add(pattr.getTerminationPoint());
            pattr.prepare();
        }

        NodeId nodeId = new NodeId("ovsdb:node:1");
        Node before = new NodeBuilder().
            setNodeId(nodeId).
            setTerminationPoint(tps).
            addAugmentation(OvsdbBridgeAugmentation.class, ovbr).
            build();
        Node after = new NodeBuilder().
            setNodeId(nodeId).
            setTerminationPoint(new ArrayList<TerminationPoint>(tps)).
            addAugmentation(OvsdbNodeAugmentation.class, ovnode).
            addAugmentation(OvsdbBridgeAugmentation.class, ovbr).
            build();
        DataObjectModification<OvsdbNodeAugmentation> nmod =
            newItemModification(null, ovnode, null);
        List<DataObjectModification<?>> children =
            Collections.singletonList(nmod);
        DataObjectModification<Node> mod =
            newKeyedModification(before, after, children);
        OvsdbNodeChange ovchg = OvsdbNodeChange.nodeUpdated(
            ovsdbHandler, txHolder, mod, before, after);
        assertNotNull(ovchg);
        verify(ovsdbHandler).getOvsdbBridgeName();
        for (PortAttr pa: pattrs.values()) {
            pa.checkRead();
        }
        verifyNoMoreInteractions(ovsdbHandler, readTx);
        ovchg.apply();
        verify(ovsdbHandler).nodeAdded(after, ovnode);
        OfNode onode = new OfNode(0xaaffbbeeccdd0123L);
        for (PortAttr pa: pattrs.values()) {
            pa.checkMapped(onode);
        }
        verifyNoMoreInteractions(ovsdbHandler, readTx);
    }

    /**
     * Test case for
     * {@link OvsdbNodeChange#nodeUpdated(OVSDBEventHandler,ReadTransactionHolder,DataObjectModification,Node,Node)}.
     *
     * <ul>
     *   <li>OVSDB node augmentation has been removed.</li>
     *   <li>No termination point in node.</li>
     * </ul>
     */
    @Test
    public void testNodeUpdated4() {
        OvsdbBridgeAugmentation ovbr =
            newBridgeAugmentation("dd:dd:dd:dd:aa:aa:aa:aa");
        OvsdbNodeAugmentation ovnode = new OvsdbNodeAugmentationBuilder().
            build();
        NodeId nodeId = new NodeId("ovsdb:node:1");
        Node before = new NodeBuilder().
            setNodeId(nodeId).
            addAugmentation(OvsdbNodeAugmentation.class, ovnode).
            addAugmentation(OvsdbBridgeAugmentation.class, ovbr).
            build();
        Node after = new NodeBuilder().
            setNodeId(nodeId).
            addAugmentation(OvsdbBridgeAugmentation.class, ovbr).
            build();
        DataObjectModification<OvsdbNodeAugmentation> nmod =
            newItemModification(ovnode, null, null);
        List<DataObjectModification<?>> children =
            Collections.singletonList(nmod);
        DataObjectModification<Node> mod =
            newKeyedModification(before, after, children);
        OvsdbNodeChange ovchg = OvsdbNodeChange.nodeUpdated(
            ovsdbHandler, txHolder, mod, before, after);
        assertNotNull(ovchg);
        verifyZeroInteractions(ovsdbHandler, txHolder, readTx);
        ovchg.apply();
        verify(ovsdbHandler).nodeRemoved(before);
        verifyNoMoreInteractions(ovsdbHandler, txHolder, readTx);
    }

    /**
     * Test case for
     * {@link OvsdbNodeChange#nodeUpdated(OVSDBEventHandler,ReadTransactionHolder,DataObjectModification,Node,Node)}.
     *
     * <ul>
     *   <li>OVSDB node augmentation has been removed.</li>
     *   <li>Port mappings are unconfigured for neutron ports.</li>
     * </ul>
     */
    @Test
    public void testNodeUpdated5() {
        OvsdbBridgeAugmentation ovbr =
            newBridgeAugmentation("55:33:11:88:da:cf:99:04");
        OvsdbNodeAugmentation ovnode = new OvsdbNodeAugmentationBuilder().
            build();
        List<TerminationPoint> beforeTps = new ArrayList<>();
        List<TerminationPoint> afterTps = new ArrayList<>();
        Map<String, PortAttr> pattrs = new HashMap<>();
        for (long id = 1L; id <= 20L; id++) {
            String uuid = uniqueUuid();
            PortAttr pattr = new PortAttr(uuid, id, "port-" + id);
            TerminationPoint tp = pattr.getTerminationPoint();
            if (id >= 4L) {
                beforeTps.add(tp);
                pattr.prepare();
                assertNull(pattrs.put(uuid, pattr));
            }
            if (id < 16L) {
                afterTps.add(tp);
            }
        }

        NodeId nodeId = new NodeId("ovsdb:node:1");
        Node before = new NodeBuilder().
            setNodeId(nodeId).
            setTerminationPoint(beforeTps).
            addAugmentation(OvsdbNodeAugmentation.class, ovnode).
            addAugmentation(OvsdbBridgeAugmentation.class, ovbr).
            build();
        Node after = new NodeBuilder().
            setNodeId(nodeId).
            setTerminationPoint(afterTps).
            addAugmentation(OvsdbBridgeAugmentation.class, ovbr).
            build();
        DataObjectModification<OvsdbNodeAugmentation> nmod =
            newItemModification(ovnode, null, null);
        List<DataObjectModification<?>> children =
            Collections.singletonList(nmod);
        DataObjectModification<Node> mod =
            newKeyedModification(before, after, children);
        OvsdbNodeChange ovchg = OvsdbNodeChange.nodeUpdated(
            ovsdbHandler, txHolder, mod, before, after);
        assertNotNull(ovchg);
        for (PortAttr pa: pattrs.values()) {
            pa.checkRead();
        }
        verifyNoMoreInteractions(ovsdbHandler, readTx);
        ovchg.apply();
        verify(ovsdbHandler).nodeRemoved(before);
        for (PortAttr pa: pattrs.values()) {
            pa.checkUnmapped();
        }
        verifyNoMoreInteractions(ovsdbHandler, readTx);
    }

    /**
     * Test case for
     * {@link OvsdbNodeChange#nodeUpdated(OVSDBEventHandler,ReadTransactionHolder,DataObjectModification,Node,Node)}.
     *
     * <ul>
     *   <li>OVSDB node augmentation is present and not changed.</li>
     *   <li>
     *     DPID is not present in OVSDB bridge augmentation before and after
     *     modification.
     *   </li>
     * </ul>
     */
    @Test
    public void testNodeUpdated6() {
        OvsdbBridgeAugmentation ovbr = newBridgeAugmentation(null);
        OvsdbNodeAugmentation ovnode = new OvsdbNodeAugmentationBuilder().
            build();
        List<TerminationPoint> tps = new ArrayList<>();
        for (long id = 1L; id <= 5L; id++) {
            tps.add(newTerminationPoint(uniqueUuid(), id, "port-" + id));
        }

        NodeId nodeId = new NodeId("ovsdb:node:1");
        Node before = new NodeBuilder().
            setNodeId(nodeId).
            setTerminationPoint(tps).
            addAugmentation(OvsdbNodeAugmentation.class, ovnode).
            build();
        Node after = new NodeBuilder().
            setNodeId(nodeId).
            setTerminationPoint(tps).
            addAugmentation(OvsdbNodeAugmentation.class, ovnode).
            addAugmentation(OvsdbBridgeAugmentation.class, ovbr).
            build();
        DataObjectModification<OvsdbBridgeAugmentation> bmod =
            newItemModification(null, ovbr, null);
        List<DataObjectModification<?>> children =
            Collections.singletonList(bmod);
        DataObjectModification<Node> mod =
            newKeyedModification(before, after, children);
        OvsdbNodeChange ovchg = OvsdbNodeChange.nodeUpdated(
            ovsdbHandler, txHolder, mod, before, after);
        assertNotNull(ovchg);
        verify(ovsdbHandler).getOvsdbBridgeName();
        verifyNoMoreInteractions(ovsdbHandler, readTx);

        // No change should be made to port mapping configuration.
        ovchg.apply();
        verifyNoMoreInteractions(ovsdbHandler, readTx);
    }

    /**
     * Test case for
     * {@link OvsdbNodeChange#nodeUpdated(OVSDBEventHandler,ReadTransactionHolder,DataObjectModification,Node,Node)}.
     *
     * <ul>
     *   <li>OVSDB node augmentation is present and not changed.</li>
     *   <li>OVSDB bridge augmentation has been removed.</li>
     * </ul>
     */
    @Test
    public void testNodeUpdated7() {
        OvsdbBridgeAugmentation ovbr =
            newBridgeAugmentation("91:ab:7c:db:21:59:01:3d");
        OvsdbNodeAugmentation ovnode = new OvsdbNodeAugmentationBuilder().
            build();
        List<TerminationPoint> tps = new ArrayList<>();
        Map<String, PortAttr> pattrs = new HashMap<>();
        for (long id = 1L; id <= 10L; id++) {
            String uuid = uniqueUuid();
            PortAttr pattr = new PortAttr(uuid, null, "port-" + id);
            assertNull(pattrs.put(uuid, pattr));
            tps.add(pattr.getTerminationPoint());
            pattr.prepare();
        }

        NodeId nodeId = new NodeId("ovsdb:node:1");
        Node before = new NodeBuilder().
            setNodeId(nodeId).
            setTerminationPoint(tps).
            addAugmentation(OvsdbNodeAugmentation.class, ovnode).
            addAugmentation(OvsdbBridgeAugmentation.class, ovbr).
            build();
        Node after = new NodeBuilder().
            setNodeId(nodeId).
            setTerminationPoint(tps).
            addAugmentation(OvsdbNodeAugmentation.class, ovnode).
            build();
        DataObjectModification<OvsdbBridgeAugmentation> bmod =
            newItemModification(ovbr, null, null);
        List<DataObjectModification<?>> children =
            Collections.singletonList(bmod);
        DataObjectModification<Node> mod =
            newKeyedModification(before, after, children);
        OvsdbNodeChange ovchg = OvsdbNodeChange.nodeUpdated(
            ovsdbHandler, txHolder, mod, before, after);
        assertNotNull(ovchg);
        for (PortAttr pa: pattrs.values()) {
            pa.checkRead();
        }
        verify(ovsdbHandler).getOvsdbBridgeName();
        verifyNoMoreInteractions(ovsdbHandler, readTx);

        ovchg.apply();

        // All the port mappings in the node should be removed if datapath ID
        // has disappeared.
        for (PortAttr pa: pattrs.values()) {
            pa.checkUnmapped();
        }
        verifyNoMoreInteractions(ovsdbHandler, readTx);
    }

    /**
     * Test case for
     * {@link OvsdbNodeChange#nodeUpdated(OVSDBEventHandler,ReadTransactionHolder,DataObjectModification,Node,Node)}.
     *
     * <ul>
     *   <li>OVSDB node augmentation is present and not changed.</li>
     *   <li>DPID has been added to OVSDB bridge augmentation.</li>
     * </ul>
     */
    @Test
    public void testNodeUpdated8() {
        OvsdbBridgeAugmentation beforeOvbr = newBridgeAugmentation(null);
        OvsdbBridgeAugmentation afterOvbr =
            newBridgeAugmentation("dd:ac:ef:98:31:54:22:10");
        OvsdbNodeAugmentation ovnode = new OvsdbNodeAugmentationBuilder().
            build();
        List<TerminationPoint> tps = new ArrayList<>();
        Map<String, PortAttr> pattrs = new HashMap<>();
        for (long id = 1L; id <= 10L; id++) {
            String uuid = uniqueUuid();
            PortAttr pattr = new PortAttr(uuid, id, null);
            assertNull(pattrs.put(uuid, pattr));
            tps.add(pattr.getTerminationPoint());
            pattr.prepare();
        }

        NodeId nodeId = new NodeId("ovsdb:node:1");
        Node before = new NodeBuilder().
            setNodeId(nodeId).
            setTerminationPoint(tps).
            addAugmentation(OvsdbNodeAugmentation.class, ovnode).
            addAugmentation(OvsdbBridgeAugmentation.class, beforeOvbr).
            build();
        Node after = new NodeBuilder().
            setNodeId(nodeId).
            setTerminationPoint(tps).
            addAugmentation(OvsdbNodeAugmentation.class, ovnode).
            addAugmentation(OvsdbBridgeAugmentation.class, afterOvbr).
            build();
        DataObjectModification<OvsdbBridgeAugmentation> bmod =
            newItemModification(beforeOvbr, afterOvbr, null);
        List<DataObjectModification<?>> children =
            Collections.singletonList(bmod);
        DataObjectModification<Node> mod =
            newKeyedModification(before, after, children);
        OvsdbNodeChange ovchg = OvsdbNodeChange.nodeUpdated(
            ovsdbHandler, txHolder, mod, before, after);
        assertNotNull(ovchg);
        for (PortAttr pa: pattrs.values()) {
            pa.checkRead();
        }
        verify(ovsdbHandler, times(2)).getOvsdbBridgeName();
        verifyNoMoreInteractions(ovsdbHandler, readTx);

        ovchg.apply();

        // Port mappings for all the valid neutron ports should be configured.
        OfNode onode = new OfNode(0xddacef9831542210L);
        for (PortAttr pa: pattrs.values()) {
            pa.checkMapped(onode);
        }
        verifyNoMoreInteractions(ovsdbHandler, readTx);
    }

    /**
     * Test case for
     * {@link OvsdbNodeChange#nodeUpdated(OVSDBEventHandler,ReadTransactionHolder,DataObjectModification,Node,Node)}.
     *
     * <ul>
     *   <li>OVSDB node augmentation is present and not changed.</li>
     *   <li>DPID in OVSDB bridge augmentation has been changed.</li>
     *   <li>Termination points have been changed.</li>
     * </ul>
     */
    @Test
    public void testNodeUpdated9() {
        OvsdbBridgeAugmentation beforeOvbr =
            newBridgeAugmentation("3c:99:31:da:b8:2e:f8:7c");
        OvsdbBridgeAugmentation afterOvbr =
            newBridgeAugmentation("3c:99:31:da:b8:2e:f8:7d");
        OvsdbNodeAugmentation ovnode = new OvsdbNodeAugmentationBuilder().
            build();
        List<TerminationPoint> beforeTps = new ArrayList<>();
        Map<String, PortAttr> beforePattrs = new HashMap<>();
        for (long id = 1L; id <= 10L; id++) {
            String uuid = uniqueUuid();
            PortAttr pattr = new PortAttr(uuid, id, null);
            assertNull(beforePattrs.put(uuid, pattr));
            beforeTps.add(pattr.getTerminationPoint());
            pattr.prepare();
        }

        List<TerminationPoint> afterTps = new ArrayList<>();
        Map<String, PortAttr> afterPattrs = new HashMap<>();
        for (long id = 20L; id <= 35L; id++) {
            String uuid = uniqueUuid();
            PortAttr pattr = new PortAttr(uuid, id, "port-" + id);
            assertNull(afterPattrs.put(uuid, pattr));
            afterTps.add(pattr.getTerminationPoint());
            pattr.prepare();
        }

        long count = 0;
        Set<String> changedPorts = new HashSet<>();
        Map<String, PortAttr> removedPattrs = new HashMap<>();
        for (PortAttr pattr: beforePattrs.values()) {
            count++;
            String uuid = pattr.getUuid();
            if (count <= 4) {
                // 4 termination points are changed.
                assertTrue(changedPorts.add(uuid));
                long id = count * 100L;
                PortAttr newPattr = new PortAttr(uuid, id, "newport-" + id);
                newPattr.setSucceeded();
                assertNull(afterPattrs.put(uuid, newPattr));
                afterTps.add(newPattr.getTerminationPoint());
            } else if (count <= 7) {
                // 3 termination points are unchanged.
                assertNull(afterPattrs.put(uuid, pattr));
                afterTps.add(pattr.getTerminationPoint());
            } else {
                // 3 termination points are removed.
                assertNull(removedPattrs.put(pattr.getUuid(), pattr));
            }
        }

        NodeId nodeId = new NodeId("ovsdb:node:1");
        Node before = new NodeBuilder().
            setNodeId(nodeId).
            setTerminationPoint(beforeTps).
            addAugmentation(OvsdbNodeAugmentation.class, ovnode).
            addAugmentation(OvsdbBridgeAugmentation.class, beforeOvbr).
            build();
        Node after = new NodeBuilder().
            setNodeId(nodeId).
            setTerminationPoint(afterTps).
            addAugmentation(OvsdbNodeAugmentation.class, ovnode).
            addAugmentation(OvsdbBridgeAugmentation.class, afterOvbr).
            build();
        List<DataObjectModification<?>> children = new ArrayList<>();
        children.add(newItemModification(beforeOvbr, afterOvbr, null));

        for (PortAttr pattr: afterPattrs.values()) {
            TerminationPoint tp = pattr.getTerminationPoint();
            children.add(newKeyedModification(null, tp, null));
        }
        for (String uuid: changedPorts) {
            PortAttr bpattr = beforePattrs.get(uuid);
            PortAttr apattr = afterPattrs.get(uuid);
            TerminationPoint beforeTp = bpattr.getTerminationPoint();
            TerminationPoint afterTp = apattr.getTerminationPoint();
            children.add(newKeyedModification(beforeTp, afterTp, null));
        }
        for (PortAttr pattr: removedPattrs.values()) {
            TerminationPoint tp = pattr.getTerminationPoint();
            children.add(newKeyedModification(tp, null, null));
        }

        DataObjectModification<Node> mod =
            newKeyedModification(before, after, children);
        OvsdbNodeChange ovchg = OvsdbNodeChange.nodeUpdated(
            ovsdbHandler, txHolder, mod, before, after);
        assertNotNull(ovchg);
        for (PortAttr pa: beforePattrs.values()) {
            String uuid = pa.getUuid();
            int c = (removedPattrs.containsKey(uuid)) ? 1 : 2;
            pa.checkRead(c);
        }
        for (PortAttr pa: afterPattrs.values()) {
            String uuid = pa.getUuid();
            if (!beforePattrs.containsKey(uuid)) {
                pa.checkRead();
            }
        }
        verify(ovsdbHandler, times(2)).getOvsdbBridgeName();
        verifyNoMoreInteractions(ovsdbHandler, readTx);

        ovchg.apply();

        // Old port mappings should be removed.
        for (PortAttr pa: beforePattrs.values()) {
            pa.checkUnmapped();
        }

        // New port mappings should be configured.
        OfNode onode = new OfNode(0x3c9931dab82ef87dL);
        for (PortAttr pa: afterPattrs.values()) {
            pa.checkMapped(onode);
        }
        verifyNoMoreInteractions(ovsdbHandler, readTx);
    }

    /**
     * Test case for
     * {@link OvsdbNodeChange#nodeUpdated(OVSDBEventHandler,ReadTransactionHolder,DataObjectModification,Node,Node)}.
     *
     * <ul>
     *   <li>OVSDB node augmentation is present and not changed.</li>
     *   <li>DPID is present in OVSDB bridge augmentation and not changed.</li>
     *   <li>No termination point is changed.</li>
     * </ul>
     */
    @Test
    public void testNodeUpdated10() {
        OvsdbBridgeAugmentation ovbr =
            newBridgeAugmentation("77:13:ac:b8:74:19:33:df");
        OvsdbNodeAugmentation ovnode = new OvsdbNodeAugmentationBuilder().
            build();
        List<TerminationPoint> tps = new ArrayList<>();
        for (long id = 1L; id <= 5L; id++) {
            tps.add(newTerminationPoint(uniqueUuid(), id, "port-" + id));
        }

        NodeId nodeId = new NodeId("ovsdb:node:1");
        Node before = new NodeBuilder().
            setNodeId(nodeId).
            setTerminationPoint(tps).
            addAugmentation(OvsdbNodeAugmentation.class, ovnode).
            addAugmentation(OvsdbBridgeAugmentation.class, ovbr).
            build();
        Node after = new NodeBuilder().
            setNodeId(nodeId).
            setTerminationPoint(tps).
            addAugmentation(OvsdbNodeAugmentation.class, ovnode).
            addAugmentation(OvsdbBridgeAugmentation.class, ovbr).
            build();
        DataObjectModification<Node> mod =
            newKeyedModification(before, after, null);
        OvsdbNodeChange ovchg = OvsdbNodeChange.nodeUpdated(
            ovsdbHandler, txHolder, mod, before, after);
        assertNotNull(ovchg);
        verify(ovsdbHandler, times(2)).getOvsdbBridgeName();
        verifyNoMoreInteractions(ovsdbHandler, readTx);

        // No change should be made to port mapping configuration.
        ovchg.apply();
        verifyNoMoreInteractions(ovsdbHandler, readTx);
    }

    /**
     * Test case for
     * {@link OvsdbNodeChange#nodeUpdated(OVSDBEventHandler,ReadTransactionHolder,DataObjectModification,Node,Node)}.
     *
     * <ul>
     *   <li>OVSDB node augmentation is present and not changed.</li>
     *   <li>DPID is present in OVSDB bridge augmentation and not changed.</li>
     *   <li>Termination points are changed.</li>
     * </ul>
     */
    @Test
    public void testNodeUpdated11() {
        OvsdbBridgeAugmentation ovbr =
            newBridgeAugmentation("c0:3b:1a:54:8e:f3:97:33");
        OvsdbNodeAugmentation ovnode = new OvsdbNodeAugmentationBuilder().
            build();
        List<TerminationPoint> beforeTps = new ArrayList<>();
        List<TerminationPoint> afterTps = new ArrayList<>();
        List<DataObjectModification<?>> children = new ArrayList<>();
        Map<String, PortAttr> createdPattrs = new HashMap<>();
        Map<String, PortAttr> removedPattrs = new HashMap<>();
        Map<String, Entry<PortAttr, PortAttr>> changedPattrs = new HashMap<>();

        // 3 termination points are created.
        for (long id = 1L; id <= 3L; id++) {
            String uuid = uniqueUuid();
            PortAttr pattr = new PortAttr(uuid, id, "port-" + id);
            assertNull(createdPattrs.put(uuid, pattr));
            TerminationPoint tp = pattr.getTerminationPoint();
            afterTps.add(tp);
            pattr.prepare();
            children.add(newTpModification(null, tp));
        }

        // 2 termination point augmentations are created.
        for (long id = 4L; id <= 5L; id++) {
            String uuid = uniqueUuid();
            PortAttr pattr = new PortAttr(uuid, id, "port-" + id);
            TerminationPoint afterTp = pattr.getTerminationPoint();
            TerminationPoint beforeTp = new TerminationPointBuilder().
                setTpId(afterTp.getTpId()).
                build();
            Entry<PortAttr, PortAttr> entry = new SimpleEntry<>(null, pattr);
            assertNull(changedPattrs.put(uuid, entry));
            beforeTps.add(beforeTp);
            afterTps.add(afterTp);
            pattr.prepare();
            children.add(newTpModification(beforeTp, afterTp));
        }

        // 3 termination point augmentations are removed.
        for (long id = 6L; id <= 8L; id++) {
            String uuid = uniqueUuid();
            PortAttr pattr = new PortAttr(uuid, id, null);
            TerminationPoint beforeTp = pattr.getTerminationPoint();
            TerminationPoint afterTp = new TerminationPointBuilder().
                setTpId(beforeTp.getTpId()).
                build();
            Entry<PortAttr, PortAttr> entry = new SimpleEntry<>(pattr, null);
            assertNull(changedPattrs.put(uuid, entry));
            beforeTps.add(beforeTp);
            afterTps.add(afterTp);
            pattr.prepare();
            children.add(newTpModification(beforeTp, afterTp));
        }

        // 2 termination points are removed.
        for (long id = 10L; id <= 11L; id++) {
            String uuid = uniqueUuid();
            PortAttr pattr = new PortAttr(uuid, id, "port-" + id);
            assertNull(removedPattrs.put(uuid, pattr));
            TerminationPoint tp = pattr.getTerminationPoint();
            beforeTps.add(tp);
            pattr.prepare();
            children.add(newTpModification(tp, null));
        }

        // UUIDs for 3 neutron ports are changed.
        for (long id = 15L; id <= 17; id++) {
            String beforeUuid = uniqueUuid();
            String afterUuid = uniqueUuid();
            String name = "eth" + id;
            PortAttr beforePattr = new PortAttr(beforeUuid, id, name);
            PortAttr afterPattr = new PortAttr(afterUuid, id, name);
            beforePattr.prepare();
            afterPattr.prepare();
            assertNull(removedPattrs.put(beforeUuid, beforePattr));
            assertNull(createdPattrs.put(afterUuid, afterPattr));
            TerminationPoint beforeTp = beforePattr.getTerminationPoint();
            TerminationPoint afterTp = afterPattr.getTerminationPoint();
            beforeTps.add(beforeTp);
            afterTps.add(afterTp);
            children.add(newTpModification(beforeTp, afterTp));
        }

        // UUIDs for 2 neutron ports are added.
        for (long id = 18L; id <= 19L; id++) {
            String uuid = uniqueUuid();
            String name = "port-" + id;
            PortAttr pattr = new PortAttr(uuid, id, name);
            TerminationPoint beforeTp = newTerminationPoint(null, id, name);
            TerminationPoint afterTp = pattr.getTerminationPoint();
            Entry<PortAttr, PortAttr> entry = new SimpleEntry<>(null, pattr);
            assertNull(changedPattrs.put(uuid, entry));
            beforeTps.add(beforeTp);
            afterTps.add(afterTp);
            pattr.prepare();
            children.add(newTpModification(beforeTp, afterTp));
        }

        // 4 OpenFlow port IDs are changed.
        for (long id = 20L; id <= 23; id++) {
            String uuid = uniqueUuid();
            String name = "ofport-" + id;
            Long beforeId;
            Long afterId;
            if (id == 20L) {
                beforeId = null;
                afterId = id;
            } else if (id == 21L) {
                beforeId = id;
                afterId = null;
            } else {
                beforeId = id;
                afterId = id + 100L;
            }
            PortAttr beforePattr = new PortAttr(uuid, beforeId, name);
            PortAttr afterPattr = new PortAttr(uuid, afterId, name);
            beforePattr.prepare();
            Entry<PortAttr, PortAttr> entry =
                new SimpleEntry<>(beforePattr, afterPattr);
            assertNull(changedPattrs.put(uuid, entry));
            TerminationPoint beforeTp = beforePattr.getTerminationPoint();
            TerminationPoint afterTp = afterPattr.getTerminationPoint();
            TpId tpId = new TpId("ovsdb:node:1:" + id);
            beforeTp = new TerminationPointBuilder(beforeTp).
                setTpId(tpId).
                build();
            afterTp = new TerminationPointBuilder(afterTp).
                setTpId(tpId).
                build();
            beforeTps.add(beforeTp);
            afterTps.add(afterTp);
            children.add(newTpModification(beforeTp, afterTp));
        }

        // UUIDs for 2 neutron ports are removed.
        for (long id = 25L; id <= 26L; id++) {
            String uuid = uniqueUuid();
            PortAttr pattr = new PortAttr(uuid, id, null);
            TerminationPoint beforeTp = pattr.getTerminationPoint();
            TerminationPoint afterTp = newTerminationPoint(null, id, null);
            Entry<PortAttr, PortAttr> entry = new SimpleEntry<>(pattr, null);
            assertNull(changedPattrs.put(uuid, entry));
            beforeTps.add(beforeTp);
            afterTps.add(afterTp);
            pattr.prepare();
            children.add(newTpModification(beforeTp, afterTp));
        }

        // 3 OpenFlow port names are changed.
        for (long id = 31L; id <= 33; id++) {
            String uuid = uniqueUuid();
            String beforeName = "eth" + id;
            String afterName = (id == 31L) ? null : "port-" + id;
            PortAttr beforePattr = new PortAttr(uuid, id, beforeName);
            PortAttr afterPattr = new PortAttr(uuid, id, afterName);
            beforePattr.prepare();
            Entry<PortAttr, PortAttr> entry =
                new SimpleEntry<>(beforePattr, afterPattr);
            assertNull(changedPattrs.put(uuid, entry));
            TerminationPoint beforeTp = beforePattr.getTerminationPoint();
            TerminationPoint afterTp = afterPattr.getTerminationPoint();
            beforeTps.add(beforeTp);
            afterTps.add(afterTp);
            children.add(newTpModification(beforeTp, afterTp));
        }

        // 4 ports are unchanged.
        for (long id = 41L; id <= 44L; id++) {
            String uuid = uniqueUuid();
            PortAttr pattr;
            if (id == 41L) {
                pattr = new PortAttr(uuid, null, "unchanged-" + id);
            } else if (id == 42L) {
                pattr = new PortAttr(uuid, id, null);
            } else {
                pattr = new PortAttr(uuid, id, "unchanged-" + id);
            }
            TerminationPoint tp = pattr.getTerminationPoint();
            if (id == 41L) {
                tp = new TerminationPointBuilder(tp).
                    setTpId(new TpId("tp-" + id)).
                    build();
            }
            children.add(newTpModification(tp, tp));
            beforeTps.add(tp);
            afterTps.add(tp);
        }

        // Below modifications should be ignored.

        // Not a termination point subtree.
        SupportingNode snode = new SupportingNodeBuilder().
            setNodeRef(new NodeId("ovsdb:node:12345")).
            build();
        children.add(newKeyedModification(null, snode, null));

        // OVSDB termination point is not modified.
        TerminationPoint tp1 = new TerminationPointBuilder().
            setTpId(new TpId("ovsdb:node:1:2")).
            build();
        children.add(newKeyedModification(null, tp1, null));
        TerminationPoint tp2 = new TerminationPointBuilder().
            setTpId(new TpId("ovsdb:node:1:3")).
            build();
        children.add(newKeyedModification(tp2, null, null));
        children.add(newKeyedModification(tp2, tp2, null));

        // No deleted OVSDB termination point augmentation.
        OvsdbTerminationPointAugmentation ovtp =
            new OvsdbTerminationPointAugmentationBuilder().
            build();
        DataObjectModification<OvsdbTerminationPointAugmentation> otmod =
            newItemModification(
                OvsdbTerminationPointAugmentation.class,
                ModificationType.DELETE, null, ovtp, null);
        List<DataObjectModification<?>> tchildren =
            Collections.singletonList(otmod);
        TpId tpId = new TpId("ovsdb:node:1:10");
        tp1 = new TerminationPointBuilder().
            setTpId(tpId).
            addAugmentation(OvsdbTerminationPointAugmentation.class, ovtp).
            build();
        tp2 = new TerminationPointBuilder().
            setTpId(tpId).
            build();
        DataObjectModification<TerminationPoint> tmod =
            newKeyedModification(tp1, tp2, null);
        when(tmod.getModifiedAugmentation(
                 OvsdbTerminationPointAugmentation.class)).
            thenReturn(otmod);
        children.add(tmod);

        // No updated OVSDB termination point augmentation.
        otmod = newItemModification(
            OvsdbTerminationPointAugmentation.class, ModificationType.WRITE,
            ovtp, null, null);
        tchildren = Collections.singletonList(otmod);
        tp1 = new TerminationPointBuilder().
            setTpId(tpId).
            build();
        tp2 = new TerminationPointBuilder().
            setTpId(tpId).
            addAugmentation(OvsdbTerminationPointAugmentation.class, ovtp).
            build();
        tmod = newKeyedModification(tp1, tp2, null);
        when(tmod.getModifiedAugmentation(
                 OvsdbTerminationPointAugmentation.class)).
            thenReturn(otmod);
        children.add(tmod);

        otmod = newItemModification(
            OvsdbTerminationPointAugmentation.class, ModificationType.WRITE,
            null, null, null);
        tchildren = Collections.singletonList(otmod);
        tp1 = new TerminationPointBuilder().
            setTpId(tpId).
            addAugmentation(OvsdbTerminationPointAugmentation.class, ovtp).
            build();
        tp2 = new TerminationPointBuilder().
            setTpId(tpId).
            addAugmentation(OvsdbTerminationPointAugmentation.class, ovtp).
            build();
        tmod = newKeyedModification(tp1, tp2, null);
        when(tmod.getModifiedAugmentation(
                 OvsdbTerminationPointAugmentation.class)).
            thenReturn(otmod);
        children.add(tmod);

        NodeId nodeId = new NodeId("ovsdb:node:12345");
        Node before = new NodeBuilder().
            setNodeId(nodeId).
            setTerminationPoint(beforeTps).
            addAugmentation(OvsdbNodeAugmentation.class, ovnode).
            addAugmentation(OvsdbBridgeAugmentation.class, ovbr).
            build();
        Node after = new NodeBuilder().
            setNodeId(nodeId).
            setTerminationPoint(afterTps).
            addAugmentation(OvsdbNodeAugmentation.class, ovnode).
            addAugmentation(OvsdbBridgeAugmentation.class, ovbr).
            build();
        DataObjectModification<Node> mod =
            newKeyedModification(before, after, children);
        OvsdbNodeChange ovchg = OvsdbNodeChange.nodeUpdated(
            ovsdbHandler, txHolder, mod, before, after);
        assertNotNull(ovchg);
        verify(ovsdbHandler, times(2)).getOvsdbBridgeName();
        for (PortAttr pa: createdPattrs.values()) {
            String uuid = pa.getUuid();
            pa.checkRead();
        }
        for (PortAttr pa: removedPattrs.values()) {
            String uuid = pa.getUuid();
            pa.checkRead();
        }
        for (Entry<PortAttr, PortAttr> entry: changedPattrs.values()) {
            PortAttr beforePattr = entry.getKey();
            PortAttr afterPattr = entry.getValue();
            if (beforePattr != null) {
                int c = (afterPattr == null) ? 1 : 2;
                beforePattr.checkRead(c);
            } else if (afterPattr != null) {
                afterPattr.checkRead();
            }
        }
        verifyNoMoreInteractions(ovsdbHandler, readTx);

        // Port mapping configuration should be updated.
        ovchg.apply();

        // Old port mappings should be removed.
        for (PortAttr pa: removedPattrs.values()) {
            pa.checkUnmapped();
        }

        // New port mappings should be configured.
        OfNode onode = new OfNode(0xc03b1a548ef39733L);
        for (PortAttr pa: createdPattrs.values()) {
            pa.setSucceeded();
            pa.checkMapped(onode);
        }

        // Port mapping configuration should be updated.
        for (Entry<PortAttr, PortAttr> entry: changedPattrs.values()) {
            PortAttr beforePattr = entry.getKey();
            PortAttr afterPattr = entry.getValue();
            if (beforePattr != null) {
                beforePattr.checkUnmapped();
            }
            if (afterPattr != null) {
                afterPattr.setSucceeded();
                afterPattr.checkMapped(onode);
            }
        }

        verifyNoMoreInteractions(ovsdbHandler, readTx);
    }

    /**
     * Create a new interface external ID for OVSDB termination point
     * augmentation.
     *
     * @param key    The key of the external ID.
     * @param value  The value  of the external ID.
     * @return  The interface external ID for OVSDB termination point
     *          augmentation.
     */
    private InterfaceExternalIds newIfExternalId(String key, String value) {
        return new InterfaceExternalIdsBuilder().
            setExternalIdKey(key).
            setExternalIdValue(value).
            build();
    }

    /**
     * Construct a new OVSDB bridge augmentation with the default name.
     *
     * @param id  A string representation of datapath ID.
     * @return  A new OVSDB bridge augmentation.
     */
    private OvsdbBridgeAugmentation newBridgeAugmentation(String id) {
        return newBridgeAugmentation(id, DEFAULT_BRIDGE_NAME);
    }

    /**
     * Construct a new OVSDB bridge augmentation.
     *
     * @param id    A string representation of datapath ID.
     * @param name  The name of the OVS bridge.
     * @return  A new OVSDB bridge augmentation.
     */
    private OvsdbBridgeAugmentation newBridgeAugmentation(
        String id, String name) {
        when(ovsdbHandler.getOvsdbBridgeName()).thenReturn(DEFAULT_BRIDGE_NAME);
        DatapathId dpid = (id == null) ? null : new DatapathId(id);
        return new OvsdbBridgeAugmentationBuilder().
            setDatapathId(dpid).
            setBridgeName(new OvsdbBridgeName(new String(name))).
            build();
    }

    /**
     * Construct a new termination point.
     *
     * @param uuid  The UUID that specifies the neutron port.
     * @param id    OpenFlow port ID.
     * @param name  The name of the port.
     * @return  A new termination point.
     */
    private TerminationPoint newTerminationPoint(String uuid, Long id,
                                                 String name) {
        List<InterfaceExternalIds> extIds = new ArrayList<>();
        Collections.addAll(
            extIds,
            newIfExternalId("key-1", "value-1"),
            newIfExternalId("key-2", "value-2"));
        if (uuid != null) {
            extIds.add(newIfExternalId(EXTID_INTERFACE_ID, uuid));
        }

        OvsdbTerminationPointAugmentation ovtp =
            new OvsdbTerminationPointAugmentationBuilder().
            setOfport(id).
            setName(name).
            setInterfaceExternalIds(extIds).
            build();
        return new TerminationPointBuilder().
            setTpId(new TpId("tp-" + id)).
            addAugmentation(OvsdbTerminationPointAugmentation.class, ovtp).
            build();
    }

    /**
     * Create a new neutron port.
     *
     * @param id  The UUID that specifies the neutron port.
     * @return  A new neutron port.
     */
    private Port newPort(String id) {
        return new PortBuilder().
            setUuid(new Uuid(id)).
            build();
    }

    /**
     * Create an unique UUID string.
     *
     * @return  Unique UUID string.
     */
    private String uniqueUuid() {
        UUID uuid = currentUuid;
        if (uuid == null) {
            uuid = UUID.randomUUID();
        } else {
            uuid = new UUID(uuid.getMostSignificantBits(),
                            uuid.getLeastSignificantBits() + 1L);
        }
        currentUuid = uuid;

        return uuid.toString();
    }

    /**
     * Create a data object modification that indicates modification of
     * OVSDB termination point augmentation.
     *
     * @param before  Termination point before modification.
     * @param after   Termination point after modification.
     * @return  A {@link DataObjectModification}.
     */
    private DataObjectModification<TerminationPoint> newTpModification(
        TerminationPoint before, TerminationPoint after) {
        OvsdbTerminationPointAugmentation beforeOvtp = (before == null)
            ? null
            : before.getAugmentation(OvsdbTerminationPointAugmentation.class);
        OvsdbTerminationPointAugmentation afterOvtp = (after == null)
            ? null
            : after.getAugmentation(OvsdbTerminationPointAugmentation.class);
        DataObjectModification<OvsdbTerminationPointAugmentation> otmod =
            newItemModification(beforeOvtp, afterOvtp, null);
        List<DataObjectModification<?>> children =
            Collections.singletonList(otmod);
        DataObjectModification<TerminationPoint> mod =
            newKeyedModification(before, after, children);
        when(mod.getModifiedAugmentation(
                 OvsdbTerminationPointAugmentation.class)).
            thenReturn(otmod);
        return mod;
    }
}
