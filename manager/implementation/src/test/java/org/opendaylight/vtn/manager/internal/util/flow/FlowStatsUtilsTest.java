/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import static org.opendaylight.vtn.manager.internal.util.flow.FlowUtilsTest.VTN_FLOW_COOKIE;
import static org.opendaylight.vtn.manager.internal.util.flow.FlowUtilsTest.createDuration;
import static org.opendaylight.vtn.manager.internal.util.flow.FlowUtilsTest.createVtnFlowEntry;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.NavigableMap;
import java.util.Random;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VtnFlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.VtnFlows;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlowKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.FlowStatsHistory;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.FlowStatsHistoryBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.VtnFlowEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.flow.stats.history.FlowStatsRecord;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.flow.stats.history.FlowStatsRecordBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.flows.VtnFlowTable;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.flows.VtnFlowTableKey;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.Table;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.TableKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.table.Flow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.table.FlowKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.FlowStatisticsData;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.GetFlowStatisticsFromFlowTableInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.flow.statistics.FlowStatistics;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.flow.statistics.FlowStatisticsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.FlowCookie;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.Nodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.NodeKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.statistics.types.rev130925.GenericStatistics;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.statistics.types.rev130925.duration.DurationBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.Counter32;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.Counter64;

/**
 * JUnit test for {@link FlowStatsUtils}.
 */
public class FlowStatsUtilsTest extends TestBase {
    /**
     * Test case for
     * {@link FlowStatsUtils#getIdentifier(String,VtnDataFlowKey)}.
     */
    @Test
    public void testGetIdentifier1() {
        String[] tenants = {"vtn1", "vtn_2", "tenant_3"};
        VtnFlowId[] flowIds = {
            new VtnFlowId(BigInteger.valueOf(1L)),
            new VtnFlowId(BigInteger.valueOf(1234567L)),
            new VtnFlowId(BigInteger.valueOf(0xfffffffffL)),
        };

        for (String tname: tenants) {
            VtnFlowTableKey key = new VtnFlowTableKey(tname);
            for (VtnFlowId id: flowIds) {
                VtnDataFlowKey vdfKey = new VtnDataFlowKey(id);
                InstanceIdentifier<FlowStatsHistory> exp = InstanceIdentifier.
                    builder(VtnFlows.class).
                    child(VtnFlowTable.class, key).
                    child(VtnDataFlow.class, vdfKey).
                    child(FlowStatsHistory.class).
                    build();
                assertEquals(exp, FlowStatsUtils.getIdentifier(tname, vdfKey));
            }
        }
    }

    /**
     * Test case for
     * {@link FlowStatsUtils#getIdentifier(NodeId,FlowId)}.
     */
    @Test
    public void testGetIdentifier2() {
        NodeId[] nodes = {
            new NodeId("openflow:1"),
            new NodeId("openflow:7777"),
            new NodeId("openflow:18446744073709551615"),
        };
        FlowId[] flowIds = {
            new FlowId("flow-1"),
            new FlowId("flow_2"),
            new FlowId("flow3"),
        };

        TableKey tableKey = new TableKey((short)0);
        for (NodeId node: nodes) {
            NodeKey nodeKey = new NodeKey(node);
            for (FlowId id: flowIds) {
                InstanceIdentifier<FlowStatistics> exp = InstanceIdentifier.
                    builder(Nodes.class).
                    child(Node.class, nodeKey).
                    augmentation(FlowCapableNode.class).
                    child(Table.class, tableKey).
                    child(Flow.class, new FlowKey(id)).
                    augmentation(FlowStatisticsData.class).
                    child(FlowStatistics.class).
                    build();
                assertEquals(exp, FlowStatsUtils.getIdentifier(node, id));
            }
        }
    }

    /**
     * Test case for
     * {@link FlowStatsUtils#createGetFlowStatsInput(VtnFlowEntry)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreateGetFlowStatsInput1() throws Exception {
        long flowId = 9999999L;
        int order = 0;
        SalPort ingress = new SalPort(12L, 345L);
        Short table = 0;
        VtnFlowEntry vfent = createVtnFlowEntry(flowId, order, ingress);
        FlowCookie cookieMask = new FlowCookie(NumberUtils.getUnsigned(-1L));

        GetFlowStatisticsFromFlowTableInput input =
            FlowStatsUtils.createGetFlowStatsInput(vfent);
        assertEquals(ingress.getNodeRef(), input.getNode());
        assertEquals(vfent.getPriority(), input.getPriority());
        assertEquals(table, input.getTableId());
        assertEquals(vfent.getIdleTimeout(), input.getIdleTimeout());
        assertEquals(vfent.getHardTimeout(), input.getHardTimeout());
        assertEquals(vfent.getCookie(), input.getCookie());
        assertEquals(cookieMask, input.getCookieMask());
        assertEquals(vfent.getMatch(), input.getMatch());
        assertEquals(vfent.getFlags(), input.getFlags());
        assertEquals(vfent.getInstructions(), input.getInstructions());
        assertEquals(null, input.isStrict());
        assertEquals(null, input.isBarrier());
        assertEquals(null, input.getOutPort());
        assertEquals(null, input.getOutGroup());
        assertEquals(null, input.getBufferId());
        assertEquals(null, input.getContainerName());
        assertEquals(null, input.getFlowName());
        assertEquals(null, input.isInstallHw());
    }

    /**
     * Test case for {@link FlowStatsUtils#createGetFlowStatsInput(SalNode)}.
     */
    @Test
    public void testCreateGetFlowStatsInput2() throws Exception {
        SalNode[] nodes = {
            new SalNode(1L),
            new SalNode(7777L),
            new SalNode(-1L),
        };

        FlowCookie cookie =
            new FlowCookie(NumberUtils.getUnsigned(VTN_FLOW_COOKIE));
        FlowCookie cookieMask =
            new FlowCookie(NumberUtils.getUnsigned(0xffff000000000000L));
        Short table = 0;

        for (SalNode snode: nodes) {
            GetFlowStatisticsFromFlowTableInput input =
                FlowStatsUtils.createGetFlowStatsInput(snode);
            assertEquals(snode.getNodeRef(), input.getNode());
            assertEquals(null, input.getPriority());
            assertEquals(table, input.getTableId());
            assertEquals(null, input.getIdleTimeout());
            assertEquals(null, input.getHardTimeout());
            assertEquals(cookie, input.getCookie());
            assertEquals(cookieMask, input.getCookieMask());
            assertEquals(null, input.getMatch());
            assertEquals(null, input.getFlags());
            assertEquals(null, input.getInstructions());
            assertEquals(null, input.isStrict());
            assertEquals(null, input.isBarrier());
            assertEquals(null, input.getOutPort());
            assertEquals(null, input.getOutGroup());
            assertEquals(null, input.getBufferId());
            assertEquals(null, input.getContainerName());
            assertEquals(null, input.getFlowName());
            assertEquals(null, input.isInstallHw());
        }
    }

    /**
     * Test case for {@link FlowStatsUtils#read(ReadTransaction,NodeId,FlowId)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testRead() throws Exception {
        ReadTransaction rtx = mock(ReadTransaction.class);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        NodeId nodeId = new NodeId("openflow:12345");
        FlowId flowId = new FlowId("md-flow:55");
        InstanceIdentifier<FlowStatistics> path = InstanceIdentifier.
            builder(Nodes.class).
            child(Node.class, new NodeKey(nodeId)).
            augmentation(FlowCapableNode.class).
            child(Table.class, new TableKey((short)0)).
            child(Flow.class, new FlowKey(flowId)).
            augmentation(FlowStatisticsData.class).
            child(FlowStatistics.class).
            build();

        // In case where the target MD-SAL flow statistics is not present.
        FlowStatistics fstats = null;
        when(rtx.read(oper, path)).thenReturn(getReadResult(fstats));
        assertEquals(null, FlowStatsUtils.read(rtx, nodeId, flowId));
        verify(rtx).read(oper, path);

        // In case whether the target MD-SAL flow statistics is present.
        reset(rtx);
        fstats = new FlowStatisticsBuilder().build();
        when(rtx.read(oper, path)).thenReturn(getReadResult(fstats));
        assertEquals(fstats, FlowStatsUtils.read(rtx, nodeId, flowId));
        verify(rtx).read(oper, path);
    }

    /**
     * Test case for {@link FlowStatsUtils#check(GenericStatistics)}.
     */
    @Test
    public void testCheck() {
        GenericStatistics gstats = null;
        assertEquals("flow statistics is null.", FlowStatsUtils.check(gstats));

        FlowStatisticsBuilder builder = new FlowStatisticsBuilder();
        gstats = builder.build();
        assertEquals("No packet count.", FlowStatsUtils.check(gstats));

        Counter64 packets = new Counter64(NumberUtils.getUnsigned(1234567L));
        gstats = builder.setPacketCount(packets).build();
        assertEquals("No byte count.", FlowStatsUtils.check(gstats));

        Counter64 bytes = new Counter64(NumberUtils.getUnsigned(2345678L));
        gstats = builder.setByteCount(packets).build();
        assertEquals("No duration.", FlowStatsUtils.check(gstats));

        DurationBuilder db = new DurationBuilder();
        gstats = builder.setDuration(db.build()).build();
        assertEquals("No second in duration.", FlowStatsUtils.check(gstats));

        Counter32 sec = new Counter32(99999L);
        gstats = builder.setDuration(db.setSecond(sec).build()).build();
        assertEquals("No nanosecond in duration.",
                     FlowStatsUtils.check(gstats));

        Counter32 nsec = new Counter32(888888888L);
        gstats = builder.setDuration(db.setNanosecond(nsec).build()).build();
        assertEquals(null, FlowStatsUtils.check(gstats));
    }

    /**
     * Test case for {@link FlowStatsUtils#toNavigableMap(FlowStatsHistory)}.
     */
    @Test
    public void testToNavigableMap() {
        FlowStatsHistory history = null;
        NavigableMap<Long, FlowStatsRecord> map =
            FlowStatsUtils.toNavigableMap(history);
        assertEquals(0, map.size());

        history = new FlowStatsHistoryBuilder().build();
        map = FlowStatsUtils.toNavigableMap(history);
        assertEquals(0, map.size());

        Long time1 = 1L;
        Long time2 = 2L;
        Long time3 = 3L;
        FlowStatsRecord fsr1 = new FlowStatsRecordBuilder().
            setTime(time1).setDuration(createDuration(1L, 100L)).build();
        FlowStatsRecord fsr2 = new FlowStatsRecordBuilder().
            setTime(time2).setDuration(createDuration(2L, 200L)).build();
        FlowStatsRecord fsr3 = new FlowStatsRecordBuilder().
            setTime(time3).setDuration(createDuration(3L, 300L)).build();
        List<FlowStatsRecord> records = new ArrayList<>();
        Collections.addAll(records, fsr2, fsr3, fsr1);
        history = new FlowStatsHistoryBuilder().
            setFlowStatsRecord(records).build();
        map = FlowStatsUtils.toNavigableMap(history);
        assertEquals(3, map.size());
        assertEquals(fsr1, map.get(time1));
        assertEquals(fsr2, map.get(time2));
        assertEquals(fsr3, map.get(time3));
    }

    /**
     * Test case for
     * {@link FlowStatsUtils#addPeriodic(FlowStatsHistory,Long,GenericStatistics)}.
     */
    @Test
    public void testAddPeriodic() {
        // In case where statistics history is not present.
        FlowStatsHistory history = null;
        GenericStatistics gstats = createStatistics(100L, 199L, 1L, 1234567L);
        Long time = 9999L;
        List<FlowStatsRecord> expected = new ArrayList<>();
        expected.add(new FlowStatsRecordBuilder(gstats).setTime(time).
                     setPeriodic(true).build());
        FlowStatsHistory result =
            FlowStatsUtils.addPeriodic(history, time, gstats);
        assertEquals(expected, result.getFlowStatsRecord());

        history = new FlowStatsHistoryBuilder().build();
        result = FlowStatsUtils.addPeriodic(history, time, gstats);
        assertEquals(expected, result.getFlowStatsRecord());

        // Ensure that old entries are removed.
        time = 100000000L;
        long expire = time.longValue() - 60000L;
        Boolean[] booleans = {Boolean.TRUE, Boolean.FALSE};
        for (Boolean periodic: booleans) {
            gstats = createStatistics(333L, 4444L, 100L, 55555555L);
            FlowStatsRecord fsr1 = new FlowStatsRecordBuilder(gstats).
                setTime(expire).setPeriodic(true).build();
            gstats = createStatistics(350L, 4555L, 110L, 55556666L);
            FlowStatsRecord fsr2 = new FlowStatsRecordBuilder(gstats).
                setTime(expire + 10000L).setPeriodic(true).build();

            long latestSec = 113L;
            long latestNsec = 55556666L;
            gstats = createStatistics(1234L, 56788L, latestSec, latestNsec);
            FlowStatsRecord latest = new FlowStatsRecordBuilder(gstats).
                setTime(expire + 13000L).setPeriodic(periodic).build();

            gstats = createStatistics(300L, 3000L, 99L, 55556666L);
            FlowStatsRecord old1 = new FlowStatsRecordBuilder(gstats).
                setTime(expire - 10000L).setPeriodic(true).build();
            FlowStatsRecord old2 = new FlowStatsRecordBuilder(gstats).
                setTime(expire - 1L).build();

            List<FlowStatsRecord> records = new ArrayList<>();
            Collections.addAll(records, fsr2, old2, latest, old1, fsr1);
            history = new FlowStatsHistoryBuilder().
                setFlowStatsRecord(records).build();
            gstats = createStatistics(1245L, 57890L, 120L, 55556677L);
            FlowStatsRecord fsr = new FlowStatsRecordBuilder(gstats).
                setTime(time).setPeriodic(true).build();
            expected.clear();
            Collections.addAll(expected, fsr1, fsr2, latest, fsr);
            result = FlowStatsUtils.addPeriodic(history, time, gstats);
            assertEquals(expected, result.getFlowStatsRecord());

            if (!periodic.booleanValue()) {
                // In case where the latest history is newer than the
                // statistics to be added.
                fsr = new FlowStatsRecordBuilder(latest).
                    setTime(time).setKey(null).setPeriodic(true).build();
                expected.clear();
                Collections.addAll(expected, fsr1, fsr2, fsr);
                gstats = createStatistics(1234L, 56787L, latestSec,
                                          latestNsec - 1);
                result = FlowStatsUtils.addPeriodic(history, time, gstats);
                assertEquals(expected, result.getFlowStatsRecord());
            }
        }
    }

    /**
     * Test case for
     * {@link FlowStatsUtils#addNonPeriodic(FlowStatsHistory,FlowStatsRecord)}.
     */
    @Test
    public void testAddNonPeriodic() {
        // In case where statistics history is not present.
        FlowStatsHistory history = null;
        GenericStatistics gstats = createStatistics(100L, 199L, 1L, 1234567L);
        Long time = 10000000L;
        FlowStatsRecord fsr = new FlowStatsRecordBuilder(gstats).
            setTime(time).build();
        List<FlowStatsRecord> expected = new ArrayList<>();
        expected.add(fsr);
        FlowStatsHistory result = FlowStatsUtils.addNonPeriodic(history, fsr);
        assertEquals(expected, result.getFlowStatsRecord());

        history = new FlowStatsHistoryBuilder().build();
        result = FlowStatsUtils.addNonPeriodic(history, fsr);
        assertEquals(expected, result.getFlowStatsRecord());

        long latestTime = time - 10000L;
        long[] shortIntervals = {1L, 2L, 100L, 999L};
        long[] intervals = {1000L, 1001L, 2000L, 5555L};
        Boolean[] booleans = {Boolean.TRUE, Boolean.FALSE};
        for (Boolean periodic: booleans) {
            gstats = createStatistics(100L, 200L, 4000L, 0L);
            FlowStatsRecord fsr1 = new FlowStatsRecordBuilder(gstats).
                setTime(time - 11000L).build();
            gstats = createStatistics(111L, 222L, 5000L, 10L);
            FlowStatsRecord latest = new FlowStatsRecordBuilder(gstats).
                setTime(latestTime).setPeriodic(periodic).build();
            List<FlowStatsRecord> records = new ArrayList<>();
            Collections.addAll(records, latest, fsr1);
            history = new FlowStatsHistoryBuilder().
                setFlowStatsRecord(records).build();

            // Too old records should be ignored.
            for (long l = 0; l <= 10L; l++) {
                fsr = new FlowStatsRecordBuilder(gstats).
                    setTime(latestTime - l).build();
                assertEquals(null,
                             FlowStatsUtils.addNonPeriodic(history, fsr));
            }

            gstats = createStatistics(111L, 222L, 5001L, 1111L);
            for (long l: shortIntervals) {
                fsr = new FlowStatsRecordBuilder(gstats).
                    setTime(latestTime + l).build();
                result = FlowStatsUtils.addNonPeriodic(history, fsr);
                if (periodic.booleanValue()) {
                    // Too frequent record should be ignored.
                    assertEquals(null, result);
                } else {
                    // Too frequent record should overwrite the latest
                    // non-periodic record.
                    expected.clear();
                    Collections.addAll(expected, fsr1, fsr);
                    assertEquals(expected, result.getFlowStatsRecord());
                }
            }

            for (long l: intervals) {
                fsr = new FlowStatsRecordBuilder(gstats).
                    setTime(latestTime + l).build();
                expected.clear();
                Collections.addAll(expected, fsr1, latest, fsr);
                result = FlowStatsUtils.addNonPeriodic(history, fsr);
                assertEquals(expected, result.getFlowStatsRecord());
            }
        }
    }

    /**
     * Test case for
     * {@link FlowStatsUtils#createTransactionKey(String,BigInteger)}.
     */
    @Test
    public void testCreateTransactionKey() {
        Random rand = new Random();
        Set<String> nodeIds = new HashSet<>();
        nodeIds.add("openflow:1");
        nodeIds.add("openflow:18446744073709551615");
        do {
            long dpid = rand.nextLong();
            nodeIds.add("openflow:" + NumberUtils.getUnsigned(dpid));
        } while (nodeIds.size() < 20);

        Set<BigInteger> xids = new HashSet<>();
        xids.add(BigInteger.ZERO);
        xids.add(BigInteger.ONE);
        do {
            xids.add(NumberUtils.getUnsigned(rand.nextLong()));
        } while (xids.size() < 20);

        Set<String> keys = new HashSet<>();
        for (String nid: nodeIds) {
            for (BigInteger xid: xids) {
                String xkey = FlowStatsUtils.createTransactionKey(nid, xid);
                assertEquals(true, keys.add(xkey));
            }
        }

        assertEquals(nodeIds.size() * xids.size(), keys.size());

        for (String nid: nodeIds) {
            for (BigInteger xid: xids) {
                String xkey = FlowStatsUtils.createTransactionKey(nid, xid);
                assertEquals(false, keys.add(xkey));
                assertEquals(true, keys.contains(xkey));
            }
        }
    }

    /**
     * Create a MD-SAL flow statistics record.
     *
     * @param packets  The number of transmitted packets.
     * @param bytes    The number of transmitted bytes.
     * @param sec      Duration in seconds.
     * @param nsec     Duration in milliseconds.
     */
    private static FlowStatistics createStatistics(long packets, long bytes,
                                                   long sec, long nsec) {
        return new FlowStatisticsBuilder().
            setPacketCount(new Counter64(NumberUtils.getUnsigned(packets))).
            setByteCount(new Counter64(NumberUtils.getUnsigned(bytes))).
            setDuration(createDuration(sec, nsec)).
            build();
    }
}
