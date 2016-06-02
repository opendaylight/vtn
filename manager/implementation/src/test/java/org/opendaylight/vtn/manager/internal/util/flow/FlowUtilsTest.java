/*
 * Copyright (c) 2015-2016 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow;

import static org.mockito.Mockito.any;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;

import java.math.BigDecimal;
import java.math.BigInteger;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.Future;

import org.slf4j.Logger;

import org.junit.Test;

import org.opendaylight.vtn.manager.DataLinkHost;
import org.opendaylight.vtn.manager.EthernetHost;
import org.opendaylight.vtn.manager.NodeRoute;
import org.opendaylight.vtn.manager.PortLocation;
import org.opendaylight.vtn.manager.SwitchPort;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.VNodeRoute;
import org.opendaylight.vtn.manager.VTerminalIfPath;
import org.opendaylight.vtn.manager.VTerminalPath;
import org.opendaylight.vtn.manager.flow.AveragedFlowStats;
import org.opendaylight.vtn.manager.flow.DataFlow;
import org.opendaylight.vtn.manager.flow.FlowStats;
import org.opendaylight.vtn.manager.flow.action.FlowAction;
import org.opendaylight.vtn.manager.flow.action.SetDscpAction;
import org.opendaylight.vtn.manager.flow.action.SetVlanIdAction;
import org.opendaylight.vtn.manager.flow.cond.EthernetMatch;
import org.opendaylight.vtn.manager.flow.cond.FlowMatch;
import org.opendaylight.vtn.manager.flow.cond.Inet4Match;
import org.opendaylight.vtn.manager.flow.cond.TcpMatch;
import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.Ip4Network;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.internal.cluster.MacMapPath;
import org.opendaylight.vtn.manager.internal.cluster.MacMappedHostPath;
import org.opendaylight.vtn.manager.internal.cluster.MacVlan;
import org.opendaylight.vtn.manager.internal.cluster.VlanMapPath;
import org.opendaylight.vtn.manager.internal.util.OrderedComparator;
import org.opendaylight.vtn.manager.internal.util.concurrent.SettableVTNFuture;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNActionList;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNFlowAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetDlDstAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetDlSrcAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetInetDscpAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetInetDstAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetInetSrcAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetPortDstAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetPortSrcAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetVlanIdAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetVlanPcpAction;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNEtherMatch;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNInet4Match;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNMatch;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNPortRange;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNTcpMatch;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNUdpMatch;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.pathmap.PathMapUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantUtils;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.TestDataLink;
import org.opendaylight.vtn.manager.internal.TestDataLinkHost;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.yangtools.yang.common.RpcResult;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.DataFlowMode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VirtualRouteReason;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VtnDataFlowInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VtnFlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.get.data.flow.input.DataFlowSource;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.get.data.flow.output.DataFlowInfoBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.physical.route.info.PhysicalEgressPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.physical.route.info.PhysicalEgressPortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.physical.route.info.PhysicalIngressPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.physical.route.info.PhysicalIngressPortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePath;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePathBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.common.VirtualRoute;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.common.VirtualRouteBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.AveragedDataFlowStats;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.AveragedDataFlowStatsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.DataEgressNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.DataEgressNodeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.DataEgressPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.DataEgressPortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.DataFlowAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.DataFlowActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.DataFlowStats;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.DataFlowStatsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.DataIngressNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.DataIngressNodeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.DataIngressPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.DataIngressPortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.PhysicalRoute;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.PhysicalRouteBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.BridgeMapInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.VtnFlows;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.VtnFlowsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.flow.id.set.FlowIdList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.flow.id.set.FlowIdListBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.flow.id.set.FlowIdListKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.MatchFlows;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.MatchFlowsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.MatchFlowsKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.NodeFlows;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.NodeFlowsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.NodeFlowsKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.PortFlows;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.PortFlowsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.PortFlowsKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.SourceHostFlows;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.SourceHostFlowsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.SourceHostFlowsKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlowBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlowKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.VtnFlowEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.VtnFlowEntryBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.flows.VtnFlowTable;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.flows.VtnFlowTableBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.flows.VtnFlowTableKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNodeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnFlowTimeoutConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnSwitchPort;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.Ordered;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.table.FlowBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.AddFlowInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.FlowTableRef;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.Flow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.FlowCookie;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.FlowModFlags;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.GenericFlowAttributes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.Instructions;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.Match;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.statistics.types.rev130925.duration.Duration;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.statistics.types.rev130925.duration.DurationBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Uri;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.Counter32;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.Counter64;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;
import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;

/**
 * JUnit test for {@link FlowUtils}.
 */
public class FlowUtilsTest extends TestBase {
    /**
     * Bits in a flow cookie which identifies the VTN flows.
     */
    public static final long  VTN_FLOW_COOKIE = 0x7f56000000000000L;

    /**
     * An implementation of {@link VtnFlowTimeoutConfig} for test.
     */
    private static class TimeoutConfig implements VtnFlowTimeoutConfig {
        /**
         * Value for idle-timeout field.
         */
        private Integer  idleTimeout;

        /**
         * Value for hard-timeout field.
         */
        private Integer  hardTimeout;

        /**
         * Set the idle-timeout value.
         *
         * @param value  A value for idle-timeout.
         * @return  This instance.
         */
        private TimeoutConfig setIdleTimeout(Integer value) {
            idleTimeout = value;
            return this;
        }

        /**
         * Set the hard-timeout value.
         *
         * @param value  A value for hard-timeout.
         * @return  This instance.
         */
        private TimeoutConfig setHardTimeout(Integer value) {
            hardTimeout = value;
            return this;
        }

        // VtnFlowTimeoutConfig

        /**
         * Return a class that indicates the type of this container.
         *
         * @return  This class.
         */
        @Override
        public Class<TimeoutConfig> getImplementedInterface() {
            return TimeoutConfig.class;
        }

        /**
         * Return idle-timeout value.
         */
        @Override
        public Integer getIdleTimeout() {
            return idleTimeout;
        }

        /**
         * Return hard-timeout value.
         */
        @Override
        public Integer getHardTimeout() {
            return hardTimeout;
        }
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link FlowUtils#verifyFlowTimeout(Integer, Integer)}</li>
     *   <li>{@link FlowUtils#verifyFlowTimeout(VtnFlowTimeoutConfig)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testVerifyFlowTimeout() throws Exception {
        TimeoutConfig tc = new TimeoutConfig();
        Integer[] timeouts = {
            null, 0, 1, 2, 10, 300, 5555, 65534, 65535,
        };
        RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
        StatusCode code = StatusCode.BADREQUEST;
        for (Integer tmout: timeouts) {
            if (tmout == null) {
                tc.setIdleTimeout(tmout).setHardTimeout(tmout);
                FlowUtils.verifyFlowTimeout(tc);
                FlowUtils.verifyFlowTimeout(tmout, tmout);
                continue;
            }

            tc.setIdleTimeout(tmout).setHardTimeout(0);
            FlowUtils.verifyFlowTimeout(tc);
            FlowUtils.verifyFlowTimeout(tmout, 0);
            tc.setIdleTimeout(0).setHardTimeout(tmout);
            FlowUtils.verifyFlowTimeout(tc);
            FlowUtils.verifyFlowTimeout(0, tmout);

            String msg = "idle-timeout must be less than hard-timeout.";
            int tm = tmout.intValue();
            if (tm != 0) {
                tc.setIdleTimeout(tmout).setHardTimeout(tmout);
                try {
                    FlowUtils.verifyFlowTimeout(tc);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(null, e.getCause());
                    assertEquals(etag, e.getErrorTag());
                    Status st = e.getStatus();
                    assertEquals(code, st.getCode());
                    assertEquals(msg, st.getDescription());
                }

                try {
                    FlowUtils.verifyFlowTimeout(tmout, tmout);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(null, e.getCause());
                    assertEquals(etag, e.getErrorTag());
                    Status st = e.getStatus();
                    assertEquals(code, st.getCode());
                    assertEquals(msg, st.getDescription());
                }
            }
            if (tm > 1) {
                Integer idle = Integer.valueOf(tm - 1);
                tc.setIdleTimeout(idle).setHardTimeout(tmout);
                FlowUtils.verifyFlowTimeout(tc);
                FlowUtils.verifyFlowTimeout(idle, tmout);

                tc.setIdleTimeout(tmout).setHardTimeout(idle);
                try {
                    FlowUtils.verifyFlowTimeout(tc);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(null, e.getCause());
                    assertEquals(etag, e.getErrorTag());
                    Status st = e.getStatus();
                    assertEquals(code, st.getCode());
                    assertEquals(msg, st.getDescription());
                }

                try {
                    FlowUtils.verifyFlowTimeout(tmout, idle);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(null, e.getCause());
                    assertEquals(etag, e.getErrorTag());
                    Status st = e.getStatus();
                    assertEquals(code, st.getCode());
                    assertEquals(msg, st.getDescription());
                }
            }

            // Inconsistent flow timeouts.
            msg = "idle-timeout must be specified.";
            tc.setIdleTimeout(null).setHardTimeout(tmout);
            try {
                FlowUtils.verifyFlowTimeout(tc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(null, e.getCause());
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(code, st.getCode());
                assertEquals(msg, st.getDescription());
            }

            try {
                FlowUtils.verifyFlowTimeout(null, tmout);
                unexpected();
            } catch (RpcException e) {
                assertEquals(null, e.getCause());
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(code, st.getCode());
                assertEquals(msg, st.getDescription());
            }

            msg = "hard-timeout must be specified.";
            tc.setIdleTimeout(tmout).setHardTimeout(null);
            try {
                FlowUtils.verifyFlowTimeout(tc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(null, e.getCause());
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(code, st.getCode());
                assertEquals(msg, st.getDescription());
            }

            try {
                FlowUtils.verifyFlowTimeout(tmout, null);
                unexpected();
            } catch (RpcException e) {
                assertEquals(null, e.getCause());
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(code, st.getCode());
                assertEquals(msg, st.getDescription());
            }
        }

        // Invalid idle/hard timeout.
        Integer[] invalidTimeouts = {
            Integer.MIN_VALUE, -99999999, -65535, -3333, -2, -1,
            65536, 65537, 1000000, 33333333, Integer.MAX_VALUE,
        };
        for (Integer tmout: invalidTimeouts) {
            String msg = "Invalid idle-timeout: " + tmout;
            tc.setIdleTimeout(tmout).setHardTimeout(0);
            try {
                FlowUtils.verifyFlowTimeout(tc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(null, e.getCause());
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(code, st.getCode());
                assertEquals(msg, st.getDescription());
            }

            try {
                FlowUtils.verifyFlowTimeout(tmout, 0);
                unexpected();
            } catch (RpcException e) {
                assertEquals(null, e.getCause());
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(code, st.getCode());
                assertEquals(msg, st.getDescription());
            }

            msg = "Invalid hard-timeout: " + tmout;
            tc.setIdleTimeout(0).setHardTimeout(tmout);
            try {
                FlowUtils.verifyFlowTimeout(tc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(null, e.getCause());
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(code, st.getCode());
                assertEquals(msg, st.getDescription());
            }

            try {
                FlowUtils.verifyFlowTimeout(0, tmout);
                unexpected();
            } catch (RpcException e) {
                assertEquals(null, e.getCause());
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(code, st.getCode());
                assertEquals(msg, st.getDescription());
            }
        }
    }

    /**
     * Test case for {@link FlowUtils#getInitialFlowId()}.
     */
    @Test
    public void testGetInitialFlowId() {
        VtnFlowId id = FlowUtils.getInitialFlowId();
        assertEquals(BigInteger.ONE, id.getValue());
    }

    /**
     * Test case for {@link FlowUtils#toDataFlowSource(DataLinkHost)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToDataFlowSource() throws Exception {
        assertEquals(null, FlowUtils.toDataFlowSource((DataLinkHost)null));

        short[] vlans = new short[]{0, 1, 1000, 4095};
        for (EthernetAddress ea: createEthernetAddresses()) {
            MacAddress mac = (ea == null)
                ? null : EtherAddress.create(ea).getMacAddress();
            for (short vlan: vlans) {
                EthernetHost ehost = new EthernetHost(ea, vlan);
                DataFlowSource dfs = FlowUtils.toDataFlowSource(ehost);
                assertEquals(mac, dfs.getMacAddress());
                assertEquals(new VlanId((int)vlan), dfs.getVlanId());
            }
        }

        // Invalid DataLinkHost.
        TestDataLink dladdr = new TestDataLink("addr");
        TestDataLinkHost dlhost = new TestDataLinkHost(dladdr);
        try {
            FlowUtils.toDataFlowSource(dlhost);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals("Unsupported address type: " + dlhost,
                         st.getDescription());
        }
    }

    /**
     * Test case for {@link FlowUtils#toVirtualRouteList(List)}.
     */
    @Test
    public void testToVirtualRouteList() {
        List<VNodeRoute> vroutes = null;
        assertEquals(null, FlowUtils.toVirtualRouteList(vroutes));
        vroutes = new ArrayList<>();
        assertEquals(null, FlowUtils.toVirtualRouteList(vroutes));

        vroutes = createVNodeRouteList();
        List<VirtualRoute> result = FlowUtils.toVirtualRouteList(vroutes);
        int size = vroutes.size();
        assertEquals(size, result.size());
        for (int order = 0; order < size; order++) {
            checkVirtualRoute(result.get(order), vroutes.get(order), order);
        }
    }

    /**
     * Test case for {@link FlowUtils#toDataFlow(VtnDataFlowInfo, DataFlowMode)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToDataFlow() throws Exception {
        long flowId = 12345;
        VtnFlowId vfId = new VtnFlowId(BigInteger.valueOf(flowId));
        long created = System.currentTimeMillis() - 100000000L;
        int idle = 300;
        int hard = 10000;
        EtherAddress srcMac = new EtherAddress(0x001122334455L);
        EtherAddress dstMac = new EtherAddress(0x80aabbccddeeL);
        int srcVlan = 10;
        int dstVlan = 4095;
        VBridgePath inpath = new VBridgePath("vtn_1", "vbr_1");
        VBridgeIfPath rpath = new VBridgeIfPath(inpath, "if_1");
        VBridgeIfPath outpath = new VBridgeIfPath("vtn_1", "vbr_5", "if_10");
        List<VirtualRoute> vrlist = new ArrayList<>();
        List<VNodeRoute> vroutes = new ArrayList<>();
        VirtualNodePath vnp = new VirtualNodePathBuilder().
            setTenantName(inpath.getTenantName()).
            setBridgeName(inpath.getBridgeName()).
            build();
        VirtualRouteReason reason = VirtualRouteReason.MACMAPPED;
        VirtualRoute vr = new VirtualRouteBuilder().
            setOrder(vrlist.size()).
            setVirtualNodePath(vnp).
            setReason(reason).
            build();
        vrlist.add(vr);
        vroutes.add(new VNodeRoute(inpath, reason));
        DataIngressNode inNode = new DataIngressNodeBuilder(vnp).build();

        vnp = new VirtualNodePathBuilder().
            setTenantName(rpath.getTenantName()).
            setBridgeName(rpath.getBridgeName()).
            setInterfaceName(rpath.getInterfaceName()).
            build();
        reason = VirtualRouteReason.FORWARDED;
        vr = new VirtualRouteBuilder().
            setOrder(vrlist.size()).
            setVirtualNodePath(vnp).
            setReason(reason).
            build();
        vrlist.add(vr);
        vroutes.add(new VNodeRoute(rpath, reason));

        vnp = new VirtualNodePathBuilder().
            setTenantName(outpath.getTenantName()).
            setBridgeName(outpath.getBridgeName()).
            setInterfaceName(outpath.getInterfaceName()).
            build();
        vr = new VirtualRouteBuilder().
            setOrder(vrlist.size()).
            setVirtualNodePath(vnp).
            setReason(reason).
            build();
        vrlist.add(vr);
        vroutes.add(new VNodeRoute(outpath, reason));
        DataEgressNode outNode = new DataEgressNodeBuilder(vnp).build();

        List<PhysicalRoute> phlist = new ArrayList<>();
        List<NodeRoute> nroutes = new ArrayList<>();
        createPhysicalRoute(phlist, nroutes, 1L, 2L, "eth-2", 5L, "eth-5");
        createPhysicalRoute(phlist, nroutes, 333L, 19L, "eth-19", 4L, "eth-4");
        createPhysicalRoute(phlist, nroutes, -1L, 7L, "eth-7", 3L, "eth-3");
        createPhysicalRoute(phlist, nroutes, 12345L, 40L, "eth-40", 98L,
                            "eth-98");
        PhysicalRoute pr = phlist.get(0);
        VtnSwitchPort vswp = pr.getPhysicalIngressPort();
        DataIngressPort inPort = new DataIngressPortBuilder(vswp).
            setNode(pr.getNode()).build();
        NodeRoute nr = nroutes.get(0);
        PortLocation inLoc = new PortLocation(nr.getNode(), nr.getInputPort());

        pr = phlist.get(phlist.size() - 1);
        vswp = pr.getPhysicalEgressPort();
        DataEgressPort outPort = new DataEgressPortBuilder(vswp).
            setNode(pr.getNode()).build();
        nr = nroutes.get(nroutes.size() - 1);
        PortLocation outLoc =
            new PortLocation(nr.getNode(), nr.getOutputPort());

        int dstPort = 63;
        VTNTcpMatch vtmatch = new VTNTcpMatch(null, new VTNPortRange(dstPort));
        VTNEtherMatch vematch =
            new VTNEtherMatch(srcMac, dstMac, null, srcVlan, null);
        VTNMatch vmatch = new VTNMatch(vematch, null, vtmatch);
        TcpMatch tmatch = new TcpMatch(null, dstPort);
        Inet4Match imatch = new Inet4Match(null, null, null, null,
                                           (short)6, null);
        EthernetMatch ematch = new EthernetMatch(srcMac, dstMac, 0x800,
                                                 (short)srcVlan, null);
        FlowMatch fmatch = new FlowMatch(ematch, imatch, tmatch);

        List<DataFlowAction> vactions = new ArrayList<>();
        List<FlowAction> factions = new ArrayList<>();
        VtnAction vact = VTNSetVlanIdAction.newVtnAction(dstVlan);
        DataFlowAction dfact = new DataFlowActionBuilder().
            setVtnAction(vact).
            setOrder(vactions.size()).
            build();
        vactions.add(dfact);
        factions.add(new SetVlanIdAction((short)dstVlan));

        short dscp = 59;
        vact = VTNSetInetDscpAction.newVtnAction(dscp);
        dfact = new DataFlowActionBuilder().
            setVtnAction(vact).
            setOrder(vactions.size()).
            build();
        vactions.add(dfact);
        factions.add(new SetDscpAction((byte)dscp));

        long packets = 3456L;
        long bytes = 109876L;
        long duration = System.currentTimeMillis() - created;
        long sec = duration / 1000L;
        long nsec = (duration % 1000L) * 1000000L;
        Duration d = createDuration(sec, nsec);
        DataFlowStats dfs = new DataFlowStatsBuilder().
            setPacketCount(new Counter64(BigInteger.valueOf(packets))).
            setByteCount(new Counter64(BigInteger.valueOf(bytes))).
            setDuration(d).
            build();
        FlowStats fstats = new FlowStats(packets, bytes, duration);

        double avePacktes = 5432.1987D;
        double aveBytes = 1234567.89D;
        long aveStart = created + 5000000L;
        long aveEnd = aveStart + 10000L;
        AveragedDataFlowStats adfs = new AveragedDataFlowStatsBuilder().
            setPacketCount(BigDecimal.valueOf(avePacktes)).
            setByteCount(BigDecimal.valueOf(aveBytes)).
            setStartTime(aveStart).
            setEndTime(aveEnd).
            build();
        AveragedFlowStats aveStats = new AveragedFlowStats(
            avePacktes, aveBytes, aveStart, aveEnd);

        // Reverse lists to ensure that they are sorted by "order" field.
        Collections.reverse(vrlist);
        Collections.reverse(phlist);
        Collections.reverse(vactions);

        DataFlowInfoBuilder builder = new DataFlowInfoBuilder().
            setFlowId(vfId).
            setCreationTime(created).
            setIdleTimeout(idle).
            setHardTimeout(hard).
            setDataIngressNode(inNode).
            setDataEgressNode(outNode).
            setDataIngressPort(inPort).
            setDataEgressPort(outPort).
            setVirtualRoute(vrlist).
            setPhysicalRoute(phlist).
            setDataFlowMatch(vmatch.toDataFlowMatchBuilder().build()).
            setDataFlowAction(vactions);
        VtnDataFlowInfo vdfNoStats = builder.build();
        VtnDataFlowInfo vdfStats = builder.
            setDataFlowStats(dfs).
            setAveragedDataFlowStats(adfs).
            build();

        DataFlowMode[] modes = {
            null,
            DataFlowMode.SUMMARY,
            DataFlowMode.DETAIL,
            DataFlowMode.UPDATESTATS,
        };

        for (DataFlowMode mode: modes) {
            DataFlow df = FlowUtils.toDataFlow(vdfNoStats, mode);
            assertEquals(flowId, df.getFlowId());
            assertEquals(created, df.getCreationTime());
            assertEquals((short)idle, df.getIdleTimeout());
            assertEquals((short)hard, df.getHardTimeout());
            assertEquals(inpath, df.getIngressNodePath());
            assertEquals(inLoc, df.getIngressPort());
            assertEquals(outpath, df.getEgressNodePath());
            assertEquals(outLoc, df.getEgressPort());
            assertEquals(null, df.getStatistics());
            assertEquals(null, df.getAveragedStatistics());

            if (DataFlowMode.SUMMARY.equals(mode)) {
                assertEquals(null, df.getMatch());
                assertEquals(null, df.getActions());
                assertEquals(null, df.getVirtualRoute());
                assertEquals(null, df.getPhysicalRoute());
            } else {
                assertEquals(fmatch, df.getMatch());
                assertEquals(factions, df.getActions());
                assertEquals(vroutes, df.getVirtualRoute());
                assertEquals(nroutes, df.getPhysicalRoute());
            }

            df = FlowUtils.toDataFlow(vdfStats, mode);
            assertEquals(flowId, df.getFlowId());
            assertEquals(created, df.getCreationTime());
            assertEquals((short)idle, df.getIdleTimeout());
            assertEquals((short)hard, df.getHardTimeout());
            assertEquals(inpath, df.getIngressNodePath());
            assertEquals(inLoc, df.getIngressPort());
            assertEquals(outpath, df.getEgressNodePath());
            assertEquals(outLoc, df.getEgressPort());

            if (DataFlowMode.SUMMARY.equals(mode)) {
                assertEquals(null, df.getMatch());
                assertEquals(null, df.getActions());
                assertEquals(null, df.getVirtualRoute());
                assertEquals(null, df.getPhysicalRoute());
                assertEquals(null, df.getStatistics());
                assertEquals(null, df.getAveragedStatistics());
            } else {
                assertEquals(fmatch, df.getMatch());
                assertEquals(factions, df.getActions());
                assertEquals(vroutes, df.getVirtualRoute());
                assertEquals(nroutes, df.getPhysicalRoute());
                assertEquals(fstats, df.getStatistics());
                assertEquals(aveStats, df.getAveragedStatistics());
            }
        }
    }

    /**
     * Test case for {@link FlowUtils#toMillis(Duration)}.
     */
    @Test
    public void testToMillis() {
        assertEquals(0L, FlowUtils.toMillis((Duration)null));

        long[] seconds = {0L, 1L, 100L, 12345678L, 0xffffffffL};
        long[] milli0 = {0L, 1L, 100L, 1000L, 999999L};
        for (long sec: seconds) {
            for (long milli: milli0) {
                long expected = sec * 1000L;
                DurationBuilder builder = new DurationBuilder().
                    setSecond(new Counter32(sec));
                Duration d = builder.setNanosecond(new Counter32(milli)).
                    build();
                assertEquals(expected, FlowUtils.toMillis(d));

                // Round up 1 milliseconds.
                expected++;
                d = builder.setNanosecond(new Counter32(milli + 1000000L)).
                    build();
                assertEquals(expected, FlowUtils.toMillis(d));

                // Round up 10 milliseconds.
                expected += 9;
                d = builder.setNanosecond(new Counter32(milli + 10000000L)).
                    build();
                assertEquals(expected, FlowUtils.toMillis(d));

                // Round up 999 milliseconds.
                expected = sec * 1000L + 999L;
                d = builder.setNanosecond(new Counter32(milli + 999000000L)).
                    build();
                assertEquals(expected, FlowUtils.toMillis(d));
            }
        }
    }

    /**
     * Test case for {@link FlowUtils#compare(Duration,Duration)}.
     */
    @Test
    public void testCompareDuration() {
        Duration min = createDuration(0L, 0L);
        Duration max = createDuration(0xffffffffL, 999999999L);
        assertEquals(0, FlowUtils.compare(min, min));
        assertEquals(0, FlowUtils.compare(max, max));
        assertTrue(FlowUtils.compare(min, max) < 0);
        assertTrue(FlowUtils.compare(max, min) > 0);

        Duration d1 = createDuration(0L, 1L);
        Duration d2 = createDuration(1L, 0L);
        Duration d3 = createDuration(1L, 2L);
        Duration d4 = createDuration(1L, 999999999L);
        Duration d5 = createDuration(99L, 499999998L);
        Duration d6 = createDuration(99L, 499999999L);
        Duration d7 = createDuration(99L, 500000000L);
        Duration d8 = createDuration(99L, 500000001L);
        Duration d9 = createDuration(100L, 500000000L);
        Duration d10 = createDuration(101L, 500000000L);
        Duration d11 = createDuration(101L, 500000001L);
        Duration d12 = createDuration(12345L, 888888888L);
        Duration d13 = createDuration(12345L, 888888889L);
        Duration d14 = createDuration(0xffffffffL, 0L);
        Duration d15 = createDuration(0xffffffffL, 999999998L);
        List<Duration> list = new ArrayList<>();
        Collections.addAll(list, d10, max, d7, d1, d4, d9, min, d5, d2, d13,
                           d15, d6, d14, d8, d11, d3, d12);
        List<Duration> expected = new ArrayList<>();
        Collections.addAll(expected, min, d1, d2, d3, d4, d5, d6, d7, d8, d9,
                           d10, d11, d12, d13, d14, d15, max);

        Comparator<Duration> comp = new Comparator<Duration>() {
            @Override
            public int compare(Duration dr1, Duration dr2) {
                return FlowUtils.compare(dr1, dr2);
            }
        };

        Collections.sort(list, comp);
        assertEquals(expected, list);
    }

    /**
     * Test case for {@link FlowUtils#toVNodeRoutes(List, Comparator)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToVNodeRoutes() throws Exception {
        Comparator<Ordered> comp = new OrderedComparator();
        List<VirtualRoute> vrlist = null;
        assertEquals(null, FlowUtils.toVNodeRoutes(vrlist, comp));
        vrlist = new ArrayList<>();
        assertEquals(0, FlowUtils.toVNodeRoutes(vrlist, comp).size());

        List<VNodeRoute> vroutes = createVNodeRouteList();
        vrlist = FlowUtils.toVirtualRouteList(vroutes);
        Collections.reverse(vrlist);

        assertEquals(vroutes, FlowUtils.toVNodeRoutes(vrlist, comp));
    }

    /**
     * Test case for {@link FlowUtils#toNodeRoutes(List, Comparator)}.
     */
    @Test
    public void testToNodeRoutes() {
        Comparator<Ordered> comp = new OrderedComparator();
        List<PhysicalRoute> phlist = null;
        assertEquals(null, FlowUtils.toNodeRoutes(phlist, comp));
        phlist = new ArrayList<>();
        assertEquals(0, FlowUtils.toNodeRoutes(phlist, comp).size());

        List<NodeRoute> nroutes = new ArrayList<>();
        long[] nodes = {
            1L,
            2L,
            -1L,
            0x123456789abcdL,
            3344455566668788999L,
        };

        for (long node: nodes) {
            long in = (node % 19) & 0xffffL;
            long out = (node % 127) & 0xffffL;
            createPhysicalRoute(phlist, nroutes, node, in, "in-" + in,
                                out, "out-" + out);
        }

        Collections.reverse(phlist);
        assertEquals(nroutes, FlowUtils.toNodeRoutes(phlist, comp));
    }

    /**
     * Test case for {@link FlowUtils#createCookie(VtnFlowId)}.
     */
    @Test
    public void testCreateFlowCookie() {
        long[] ids = {
            1L,
            2L,
            1234567L,
            0xffffffffL,
            0x100000000L,
            0x123456789abcdL,
            0xfffffffffffffL,
        };

        for (long id: ids) {
            VtnFlowId vfId = new VtnFlowId(BigInteger.valueOf(id));
            FlowCookie expected = new FlowCookie(
                BigInteger.valueOf(id | VTN_FLOW_COOKIE));
            assertEquals(expected, FlowUtils.createCookie(vfId));
        }
    }

    /**
     * Test case for {@link FlowUtils#getVtnFlowId(FlowCookie)}.
     */
    @Test
    public void testGetVtnFlowId() {
        assertEquals(null, FlowUtils.getVtnFlowId((FlowCookie)null));

        long[] ids = {
            1L,
            2L,
            1234567L,
            0xffffffffL,
            0x100000000L,
            0x123456789abcL,
            0xffffffffffffL,
        };
        long[] invalidMasks = {
            0L,
            0xffff000000000000L,
            0x0111000000000000L,
            0xff56000000000000L,
            VTN_FLOW_COOKIE + 0x1000000000000L,
            VTN_FLOW_COOKIE - 0x1000000000000L,
        };

        for (long id: ids) {
            BigInteger bid = BigInteger.valueOf(id);
            FlowCookie cookie = new FlowCookie(bid);
            assertEquals(null, FlowUtils.getVtnFlowId(cookie));
            for (long mask: invalidMasks) {
                long c = id | mask;
                cookie = new FlowCookie(NumberUtils.getUnsigned(c));
                assertEquals(null, FlowUtils.getVtnFlowId(cookie));
            }

            long c = id | VTN_FLOW_COOKIE;
            cookie = new FlowCookie(BigInteger.valueOf(c));
            VtnFlowId expected = new VtnFlowId(bid);
            assertEquals(expected, FlowUtils.getVtnFlowId(cookie));
        }
    }

    /**
     * Test case for {@link FlowUtils#getIdentifier(String)}.
     */
    @Test
    public void testGetIdentifier1() {
        String[] tenants = {"vtn1", "vtn2", "tenant_3"};
        for (String tname: tenants) {
            InstanceIdentifier<VtnFlowTable> expected = InstanceIdentifier.
                builder(VtnFlows.class).
                child(VtnFlowTable.class, new VtnFlowTableKey(tname)).
                build();
            assertEquals(expected, FlowUtils.getIdentifier(tname));
        }
    }

    /**
     * Test case for {@link FlowUtils#getIdentifier(VtnFlowTableKey)}.
     */
    @Test
    public void testGetIdentifier2() {
        String[] tenants = {"vtn1", "vtn2", "tenant_3"};
        for (String tname: tenants) {
            VtnFlowTableKey key = new VtnFlowTableKey(tname);
            InstanceIdentifier<VtnFlowTable> expected = InstanceIdentifier.
                builder(VtnFlows.class).
                child(VtnFlowTable.class, key).
                build();
            assertEquals(expected, FlowUtils.getIdentifier(key));
        }
    }

    /**
     * Test case for {@link FlowUtils#getIdentifier(String, VtnFlowId)}.
     */
    @Test
    public void testGetIdentifier3() {
        String[] tenants = {"vtn1", "vtn2", "tenant_3"};
        VtnFlowId[] flowIds = {
            new VtnFlowId(BigInteger.valueOf(1L)),
            new VtnFlowId(BigInteger.valueOf(VTN_FLOW_COOKIE | 12345L)),
            new VtnFlowId(BigInteger.valueOf(VTN_FLOW_COOKIE | 0xfffffffffL)),
        };
        for (String tname: tenants) {
            VtnFlowTableKey key = new VtnFlowTableKey(tname);
            for (VtnFlowId id: flowIds) {
                InstanceIdentifier<VtnDataFlow> expected = InstanceIdentifier.
                    builder(VtnFlows.class).
                    child(VtnFlowTable.class, key).
                    child(VtnDataFlow.class, new VtnDataFlowKey(id)).
                    build();
                assertEquals(expected, FlowUtils.getIdentifier(tname, id));
            }
        }
    }

    /**
     * Test case for {@link FlowUtils#getIdentifier(String, VtnDataFlowKey)}.
     */
    @Test
    public void testGetIdentifier4() {
        String[] tenants = {"vtn1", "vtn2", "tenant_3"};
        VtnFlowId[] flowIds = {
            new VtnFlowId(BigInteger.valueOf(1L)),
            new VtnFlowId(BigInteger.valueOf(VTN_FLOW_COOKIE | 12345L)),
            new VtnFlowId(BigInteger.valueOf(VTN_FLOW_COOKIE | 0xfffffffffL)),
        };
        for (String tname: tenants) {
            VtnFlowTableKey key = new VtnFlowTableKey(tname);
            for (VtnFlowId id: flowIds) {
                VtnDataFlowKey fkey = new VtnDataFlowKey(id);
                InstanceIdentifier<VtnDataFlow> expected = InstanceIdentifier.
                    builder(VtnFlows.class).
                    child(VtnFlowTable.class, key).
                    child(VtnDataFlow.class, fkey).
                    build();
                assertEquals(expected, FlowUtils.getIdentifier(tname, fkey));
            }
        }
    }

    /**
     * Test case for {@link FlowUtils#getIdentifier(String, NodeId)}.
     */
    @Test
    public void testGetIdentifier5() {
        String[] tenants = {"vtn1", "vtn2", "tenant_3"};
        NodeId[] nodes = {
            new NodeId("openflow:1"),
            new NodeId("openflow:2"),
            new NodeId("openflow:18446744073709551615"),
        };
        for (String tname: tenants) {
            VtnFlowTableKey key = new VtnFlowTableKey(tname);
            for (NodeId node: nodes) {
                InstanceIdentifier<NodeFlows> expected = InstanceIdentifier.
                    builder(VtnFlows.class).
                    child(VtnFlowTable.class, key).
                    child(NodeFlows.class, new NodeFlowsKey(node)).
                    build();
                assertEquals(expected, FlowUtils.getIdentifier(tname, node));
            }
        }
    }

    /**
     * Test case for
     * {@link FlowUtils#getIdentifier(VtnFlowTableKey, NodeFlowsKey)}.
     */
    @Test
    public void testGetIdentifier6() {
        String[] tenants = {"vtn1", "vtn2", "tenant_3"};
        NodeId[] nodes = {
            new NodeId("openflow:1"),
            new NodeId("openflow:2"),
            new NodeId("openflow:18446744073709551615"),
        };
        for (String tname: tenants) {
            VtnFlowTableKey key = new VtnFlowTableKey(tname);
            for (NodeId node: nodes) {
                NodeFlowsKey fkey = new NodeFlowsKey(node);
                InstanceIdentifier<NodeFlows> expected = InstanceIdentifier.
                    builder(VtnFlows.class).
                    child(VtnFlowTable.class, key).
                    child(NodeFlows.class, fkey).
                    build();
                assertEquals(expected, FlowUtils.getIdentifier(key, fkey));
            }
        }
    }

    /**
     * Test case for {@link FlowUtils#getIdentifier(String, NodeConnectorId)}.
     */
    @Test
    public void testGetIdentifier7() {
        String[] tenants = {"vtn1", "vtn2", "tenant_3"};
        NodeConnectorId[] ports = {
            new NodeConnectorId("openflow:1:1"),
            new NodeConnectorId("openflow:1:2"),
            new NodeConnectorId("openflow:18446744073709551615:12345"),
        };
        for (String tname: tenants) {
            VtnFlowTableKey key = new VtnFlowTableKey(tname);
            for (NodeConnectorId port: ports) {
                InstanceIdentifier<PortFlows> expected = InstanceIdentifier.
                    builder(VtnFlows.class).
                    child(VtnFlowTable.class, key).
                    child(PortFlows.class, new PortFlowsKey(port)).
                    build();
                assertEquals(expected, FlowUtils.getIdentifier(tname, port));
            }
        }
    }

    /**
     * Test case for
     * {@link FlowUtils#getIdentifier(VtnFlowTableKey, PortFlowsKey)}.
     */
    @Test
    public void testGetIdentifier8() {
        String[] tenants = {"vtn1", "vtn2", "tenant_3"};
        NodeConnectorId[] ports = {
            new NodeConnectorId("openflow:1:1"),
            new NodeConnectorId("openflow:1:2"),
            new NodeConnectorId("openflow:18446744073709551615:12345"),
        };
        for (String tname: tenants) {
            VtnFlowTableKey key = new VtnFlowTableKey(tname);
            for (NodeConnectorId port: ports) {
                PortFlowsKey fkey = new PortFlowsKey(port);
                InstanceIdentifier<PortFlows> expected = InstanceIdentifier.
                    builder(VtnFlows.class).
                    child(VtnFlowTable.class, key).
                    child(PortFlows.class, fkey).
                    build();
                assertEquals(expected, FlowUtils.getIdentifier(key, fkey));
            }
        }
    }

    /**
     * Test case for
     * {@link FlowUtils#getIdentifier(String, SourceHostFlowsKey)}.
     */
    @Test
    public void testGetIdentifier9() {
        String[] tenants = {"vtn1", "vtn2", "tenant_3"};
        SourceHostFlowsKey[] hosts = {
            new MacVlan(1L, (short)0).getSourceHostFlowsKey(),
            new MacVlan(0xaabbccddeeffL, (short)1).getSourceHostFlowsKey(),
            new MacVlan(0xffffffffffffL, (short)4095).getSourceHostFlowsKey(),
        };
        for (String tname: tenants) {
            VtnFlowTableKey key = new VtnFlowTableKey(tname);
            for (SourceHostFlowsKey src: hosts) {
                InstanceIdentifier<SourceHostFlows> exp = InstanceIdentifier.
                    builder(VtnFlows.class).
                    child(VtnFlowTable.class, key).
                    child(SourceHostFlows.class, src).
                    build();
                assertEquals(exp, FlowUtils.getIdentifier(tname, src));
            }
        }
    }

    /**
     * Test case for {@link FlowUtils#getIdentifier(String, MatchFlowsKey)}.
     */
    @Test
    public void testGetIdentifier10() {
        String[] tenants = {"vtn1", "vtn2", "tenant_3"};
        MatchFlowsKey[] keys = {
            new MatchFlowsKey("key1"),
            new MatchFlowsKey("key2"),
            new MatchFlowsKey("key3"),
            new MatchFlowsKey("match-key-4"),
        };
        for (String tname: tenants) {
            VtnFlowTableKey key = new VtnFlowTableKey(tname);
            for (MatchFlowsKey fkey: keys) {
                InstanceIdentifier<MatchFlows> expected = InstanceIdentifier.
                    builder(VtnFlows.class).
                    child(VtnFlowTable.class, key).
                    child(MatchFlows.class, fkey).
                    build();
                assertEquals(expected, FlowUtils.getIdentifier(tname, fkey));
            }
        }
    }

    /**
     * Test case for {@link FlowUtils#getTenantName(InstanceIdentifier)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetTenantName() throws Exception {
        String[] tenants = {"vtn1", "vtn2", "tenant_3"};
        VtnFlowId flowId =
            new VtnFlowId(BigInteger.valueOf(VTN_FLOW_COOKIE | 12345L));
        NodeId node = new NodeId("openflow:2");
        NodeConnectorId port = new NodeConnectorId("openflow:1:2");
        SourceHostFlowsKey src = new MacVlan(0xaabbccddeeffL, (short)1).
            getSourceHostFlowsKey();
        MatchFlowsKey match = new MatchFlowsKey("match-key-1");

        List<InstanceIdentifier<?>> badList = new ArrayList<>();
        SalNode snode = new SalNode(123L);
        SalPort sport = new SalPort(456L, 789L);
        Collections.addAll(
            badList,
            snode.getNodeIdentifier(),
            snode.getFlowNodeIdentifier(),
            snode.getVtnNodeIdentifier(),
            sport.getNodeConnectorIdentifier(),
            sport.getVtnPortIdentifier());

        for (String tname: tenants) {
            List<InstanceIdentifier<?>> paths = new ArrayList<>();
            Collections.addAll(
                paths,
                FlowUtils.getIdentifier(tname),
                FlowUtils.getIdentifier(tname, flowId),
                FlowUtils.getIdentifier(tname, node),
                FlowUtils.getIdentifier(tname, port),
                FlowUtils.getIdentifier(tname, src),
                FlowUtils.getIdentifier(tname, match));
            for (InstanceIdentifier<?> path: paths) {
                assertEquals(tname, FlowUtils.getTenantName(path));
            }

            paths.clear();
            Collections.addAll(
                paths,
                VTenantUtils.getIdentifier(tname),
                PathMapUtils.getIdentifier(tname, 1));
            paths.addAll(badList);
            for (InstanceIdentifier<?> path: paths) {
                assertEquals(null, FlowUtils.getTenantName(path));
            }
        }
    }

    /**
     * Test case for {@link FlowUtils#createTxUri(VtnFlowEntry, String)}.
     */
    @Test
    public void testCreateTxUri() {
        long[] ids = {
            1L,
            2L,
            1234567L,
            0xffffffffL,
            0x100000000L,
            0x123456789abcL,
            0xffffffffffffL,
        };
        Integer[] orders = {0, 1, 3, 5, 1000};
        String[] prefixes = {"add-flow:", "add-flow-1:", "mod-flow:"};

        for (long id: ids) {
            long c = id | VTN_FLOW_COOKIE;
            FlowCookie cookie = new FlowCookie(NumberUtils.getUnsigned(c));
            for (Integer order: orders) {
                for (String prefix: prefixes) {
                    VtnFlowEntry vfent = new VtnFlowEntryBuilder().
                        setCookie(cookie).
                        setOrder(order).
                        build();
                    String value = String.format("%s%x-%s", prefix, c, order);
                    assertEquals(new Uri(value),
                                 FlowUtils.createTxUri(vfent, prefix));
                }
            }
        }
    }

    /**
     * Test case for {@link FlowUtils#createAddFlowInput(VtnFlowEntry)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreateAddFlowInput() throws Exception {
        long flowId = 12345L;
        int order = 3;
        SalPort ingress = new SalPort(123L, 45L);
        Short table = 0;
        VtnFlowEntry vfent = createVtnFlowEntry(flowId, order, ingress);
        long c = flowId | VTN_FLOW_COOKIE;
        FlowCookie cookie = new FlowCookie(NumberUtils.getUnsigned(c));

        AddFlowInput input = FlowUtils.createAddFlowInput(vfent);
        assertEquals(ingress.getNodeRef(), input.getNode());
        assertEquals(vfent.getPriority(), input.getPriority());
        assertEquals(table, input.getTableId());
        assertEquals(vfent.getIdleTimeout(), input.getIdleTimeout());
        assertEquals(vfent.getHardTimeout(), input.getHardTimeout());
        assertEquals(cookie, input.getCookie());
        assertEquals(null, input.getCookieMask());
        assertEquals(vfent.getMatch(), input.getMatch());
        assertEquals(vfent.getFlags(), input.getFlags());
        assertEquals(vfent.getInstructions(), input.getInstructions());
        assertEquals(Boolean.TRUE, input.isStrict());
        assertEquals(Boolean.TRUE, input.isBarrier());
        assertEquals(null, input.getOutPort());
        assertEquals(null, input.getOutGroup());
        assertEquals(null, input.getBufferId());
        assertEquals(null, input.getContainerName());
        assertEquals(null, input.getFlowName());
        assertEquals(null, input.isInstallHw());

        FlowTableRef tref =
            new FlowTableRef(ingress.getFlowTableIdentifier(table));
        assertEquals(tref, input.getFlowTable());

        Uri uri = new Uri(String.format("add-flow:%x-%s", c, order));
        assertEquals(uri, input.getTransactionUri());
    }

    /**
     * Test case for {@link FlowUtils#createRemoveFlowInputBuilder(SalNode)}.
     */
    @Test
    public void testCreateRemoveFlowInputBuilder1() {
        SalNode[] nodes = {
            new SalNode(1L),
            new SalNode(12345L),
            new SalNode(-1L),
        };
        Short table = 0;
        Uri uri = new Uri("remove-flow:all");
        FlowCookie cookie =
            new FlowCookie(NumberUtils.getUnsigned(VTN_FLOW_COOKIE));
        FlowCookie cookieMask =
            new FlowCookie(NumberUtils.getUnsigned(0xffff000000000000L));

        for (SalNode snode: nodes) {
            FlowTableRef tref =
                new FlowTableRef(snode.getFlowTableIdentifier(table));
            RemoveFlowInputBuilder builder =
                FlowUtils.createRemoveFlowInputBuilder(snode);
            assertEquals(snode.getNodeRef(), builder.getNode());
            assertEquals(table, builder.getTableId());
            assertEquals(tref, builder.getFlowTable());
            assertEquals(uri, builder.getTransactionUri());
            assertEquals(cookie, builder.getCookie());
            assertEquals(cookieMask, builder.getCookieMask());
            assertEquals(Boolean.FALSE, builder.isStrict());
            assertEquals(null, builder.isBarrier());
            assertEquals(null, builder.getOutPort());
            assertEquals(null, builder.getOutGroup());
            assertEquals(null, builder.getBufferId());
            assertEquals(null, builder.getContainerName());
            assertEquals(null, builder.getFlowName());
            assertEquals(null, builder.isInstallHw());
        }
    }

    /**
     * Test case for
     * {@link FlowUtils#createRemoveFlowInputBuilder(SalNode,VtnFlowEntry,InventoryReader)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreateRemoveFlowInputBuilder2() throws Exception {
        long flowId = 0xffffffffffffL;
        int order = 12345;
        SalPort ingress = new SalPort(-1L, 0xfffff000L);
        Short table = 0;
        VtnFlowEntry vfent = createVtnFlowEntry(flowId, order, ingress);

        // In case where the target node is not present.
        ReadTransaction rtx = mock(ReadTransaction.class);
        InventoryReader reader = new InventoryReader(rtx);
        SalNode snode = ingress.getSalNode();
        reader.prefetch(snode, null);
        assertEquals(null,
                     FlowUtils.createRemoveFlowInputBuilder(snode, vfent,
                                                            reader));

        // In case where the target node is present.
        long c = flowId | VTN_FLOW_COOKIE;
        FlowCookie cookie = new FlowCookie(NumberUtils.getUnsigned(c));
        FlowCookie cookieMask = new FlowCookie(NumberUtils.getUnsigned(-1L));
        VtnNode vnode = new VtnNodeBuilder().setId(snode.getNodeId()).build();
        reader.prefetch(snode, vnode);

        RemoveFlowInputBuilder builder =
            FlowUtils.createRemoveFlowInputBuilder(snode, vfent, reader);
        assertEquals(ingress.getNodeRef(), builder.getNode());
        assertEquals(vfent.getPriority(), builder.getPriority());
        assertEquals(table, builder.getTableId());
        assertEquals(vfent.getIdleTimeout(), builder.getIdleTimeout());
        assertEquals(vfent.getHardTimeout(), builder.getHardTimeout());
        assertEquals(cookie, builder.getCookie());
        assertEquals(cookieMask, builder.getCookieMask());
        assertEquals(vfent.getMatch(), builder.getMatch());
        assertEquals(vfent.getFlags(), builder.getFlags());
        assertEquals(vfent.getInstructions(), builder.getInstructions());
        assertEquals(Boolean.TRUE, builder.isStrict());
        assertEquals(null, builder.isBarrier());
        assertEquals(null, builder.getOutPort());
        assertEquals(null, builder.getOutGroup());
        assertEquals(null, builder.getBufferId());
        assertEquals(null, builder.getContainerName());
        assertEquals(null, builder.getFlowName());
        assertEquals(null, builder.isInstallHw());

        FlowTableRef tref =
            new FlowTableRef(ingress.getFlowTableIdentifier(table));
        assertEquals(tref, builder.getFlowTable());

        Uri uri = new Uri(String.format("remove-flow:%x-%s", c, order));
        assertEquals(uri, builder.getTransactionUri());

        verifyZeroInteractions(rtx);
    }

    /**
     * Test case for
     * {@link FlowUtils#createRemoveFlowInputBuilder(SalNode,VtnFlowEntry)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreateRemoveFlowInputBuilder3() throws Exception {
        long flowId = 0x123456789aL;
        int order = 99;
        SalPort ingress = new SalPort(777L, 15L);
        SalNode snode = ingress.getSalNode();
        Short table = 0;
        VtnFlowEntry vfent = createVtnFlowEntry(flowId, order, ingress);
        long c = flowId | VTN_FLOW_COOKIE;
        FlowCookie cookie = new FlowCookie(NumberUtils.getUnsigned(c));
        FlowCookie cookieMask = new FlowCookie(NumberUtils.getUnsigned(-1L));

        RemoveFlowInputBuilder builder =
            FlowUtils.createRemoveFlowInputBuilder(snode, vfent);
        assertEquals(ingress.getNodeRef(), builder.getNode());
        assertEquals(vfent.getPriority(), builder.getPriority());
        assertEquals(table, builder.getTableId());
        assertEquals(vfent.getIdleTimeout(), builder.getIdleTimeout());
        assertEquals(vfent.getHardTimeout(), builder.getHardTimeout());
        assertEquals(cookie, builder.getCookie());
        assertEquals(cookieMask, builder.getCookieMask());
        assertEquals(vfent.getMatch(), builder.getMatch());
        assertEquals(vfent.getFlags(), builder.getFlags());
        assertEquals(vfent.getInstructions(), builder.getInstructions());
        assertEquals(Boolean.TRUE, builder.isStrict());
        assertEquals(null, builder.isBarrier());
        assertEquals(null, builder.getOutPort());
        assertEquals(null, builder.getOutGroup());
        assertEquals(null, builder.getBufferId());
        assertEquals(null, builder.getContainerName());
        assertEquals(null, builder.getFlowName());
        assertEquals(null, builder.isInstallHw());

        FlowTableRef tref =
            new FlowTableRef(ingress.getFlowTableIdentifier(table));
        assertEquals(tref, builder.getFlowTable());

        Uri uri = new Uri(String.format("remove-flow:%x-%s", c, order));
        assertEquals(uri, builder.getTransactionUri());
    }

    /**
     * Test case for
     * {@link FlowUtils#createRemoveFlowInputBuilder(SalNode, SalPort)}.
     */
    @Test
    public void testCreateRemoveFlowInputBuilder4() {
        SalPort sport = new SalPort(0xaabbccddeeff1122L, 0xf0000000L);
        SalNode snode = sport.getSalNode();
        Short table = 0;

        List<RemoveFlowInputBuilder> list =
            FlowUtils.createRemoveFlowInputBuilder(snode, sport);
        assertEquals(2, list.size());

        // The first element should remove flows that match packets received
        // from the specified port.
        RemoveFlowInputBuilder builder = list.get(0);
        assertEquals(snode.getNodeRef(), builder.getNode());
        assertEquals(null, builder.getPriority());
        assertEquals(table, builder.getTableId());
        assertEquals(null, builder.getIdleTimeout());
        assertEquals(null, builder.getHardTimeout());
        assertEquals(null, builder.getCookie());
        assertEquals(null, builder.getCookieMask());
        assertEquals(null, builder.getFlags());
        assertEquals(null, builder.getInstructions());
        assertEquals(Boolean.FALSE, builder.isStrict());
        assertEquals(null, builder.isBarrier());
        assertEquals(null, builder.getOutPort());
        assertEquals(null, builder.getOutGroup());
        assertEquals(null, builder.getBufferId());
        assertEquals(null, builder.getContainerName());
        assertEquals(null, builder.getFlowName());
        assertEquals(null, builder.isInstallHw());

        FlowTableRef tref =
            new FlowTableRef(snode.getFlowTableIdentifier(table));
        assertEquals(tref, builder.getFlowTable());

        Uri uri = new Uri(String.format("remove-flow:IN_PORT=%s", sport));
        assertEquals(uri, builder.getTransactionUri());

        Match match = new MatchBuilder().
            setInPort(sport.getNodeConnectorId()).
            build();
        assertEquals(match, builder.getMatch());

        // The second element should remove flows that transmit packets to the
        // specified port.
        builder = list.get(1);
        assertEquals(snode.getNodeRef(), builder.getNode());
        assertEquals(null, builder.getPriority());
        assertEquals(table, builder.getTableId());
        assertEquals(null, builder.getIdleTimeout());
        assertEquals(null, builder.getHardTimeout());
        assertEquals(null, builder.getCookie());
        assertEquals(null, builder.getCookieMask());
        assertEquals(null, builder.getMatch());
        assertEquals(null, builder.getFlags());
        assertEquals(null, builder.getInstructions());
        assertEquals(Boolean.FALSE, builder.isStrict());
        assertEquals(null, builder.isBarrier());
        assertEquals(tref, builder.getFlowTable());
        assertEquals(null, builder.getOutGroup());
        assertEquals(null, builder.getBufferId());
        assertEquals(null, builder.getContainerName());
        assertEquals(null, builder.getFlowName());
        assertEquals(null, builder.isInstallHw());

        uri = new Uri(String.format("remove-flow:OUT_PORT=%s", sport));
        assertEquals(uri, builder.getTransactionUri());

        BigInteger portNum = NumberUtils.getUnsigned(sport.getPortNumber());
        assertEquals(portNum, builder.getOutPort());
    }

    /**
     * Test case for
     * {@link FlowUtils#createRemoveFlowInputBuilder(SalNode,Flow,Uri)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreateRemoveFlowInputBuilder5() throws Exception {
        long flowId = 0x7777777777L;
        int order = 123;
        SalPort ingress = new SalPort(345L, 88L);
        SalNode snode = ingress.getSalNode();
        Short table = 0;
        VtnFlowEntry vfent = createVtnFlowEntry(flowId, order, ingress);
        Flow flow = new FlowBuilder(vfent).build();
        long c = flowId | VTN_FLOW_COOKIE;
        FlowCookie cookie = new FlowCookie(NumberUtils.getUnsigned(c));
        FlowCookie cookieMask = new FlowCookie(NumberUtils.getUnsigned(-1L));
        Uri uri = new Uri("remove-flow-input-5:" + flowId);

        RemoveFlowInputBuilder builder =
            FlowUtils.createRemoveFlowInputBuilder(snode, flow, uri);
        assertEquals(ingress.getNodeRef(), builder.getNode());
        assertEquals(vfent.getPriority(), builder.getPriority());
        assertEquals(table, builder.getTableId());
        assertEquals(vfent.getIdleTimeout(), builder.getIdleTimeout());
        assertEquals(vfent.getHardTimeout(), builder.getHardTimeout());
        assertEquals(cookie, builder.getCookie());
        assertEquals(cookieMask, builder.getCookieMask());
        assertEquals(vfent.getMatch(), builder.getMatch());
        assertEquals(vfent.getFlags(), builder.getFlags());
        assertEquals(vfent.getInstructions(), builder.getInstructions());
        assertEquals(Boolean.TRUE, builder.isStrict());
        assertEquals(null, builder.isBarrier());
        assertEquals(null, builder.getOutPort());
        assertEquals(null, builder.getOutGroup());
        assertEquals(null, builder.getBufferId());
        assertEquals(null, builder.getContainerName());
        assertEquals(null, builder.getFlowName());
        assertEquals(null, builder.isInstallHw());
        assertEquals(uri, builder.getTransactionUri());

        FlowTableRef tref =
            new FlowTableRef(ingress.getFlowTableIdentifier(table));
        assertEquals(tref, builder.getFlowTable());
    }

    /**
     * Test case for
     * {@link FlowUtils#removeFlowIdSet(ReadWriteTransaction,InstanceIdentifier)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testRemoveFlowIdSet() throws Exception {
        String tname = "vtn1";
        NodeId nodeId = new NodeId("openflow:12345");
        InstanceIdentifier<NodeFlows> path = InstanceIdentifier.
            builder(VtnFlows.class).
            child(VtnFlowTable.class, new VtnFlowTableKey(tname)).
            child(NodeFlows.class, new NodeFlowsKey(nodeId)).
            build();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);

        // In case where the target index is not present.
        NodeFlows set = null;
        when(tx.read(oper, path)).thenReturn(getReadResult(set));
        assertEquals(false, FlowUtils.removeFlowIdSet(tx, path));
        verify(tx).read(oper, path);
        verify(tx, never()).delete(oper, path);
        reset(tx);

        // In case where the flow ID list is not present.
        set = new NodeFlowsBuilder().build();
        when(tx.read(oper, path)).thenReturn(getReadResult(set));
        assertEquals(true, FlowUtils.removeFlowIdSet(tx, path));
        verify(tx).read(oper, path);
        verify(tx).delete(oper, path);
        reset(tx);

        // In case where the flow ID list is empty.
        List<FlowIdList> list = Collections.<FlowIdList>emptyList();
        set = new NodeFlowsBuilder().setFlowIdList(list).build();
        when(tx.read(oper, path)).thenReturn(getReadResult(set));
        assertEquals(true, FlowUtils.removeFlowIdSet(tx, path));
        verify(tx).read(oper, path);
        verify(tx).delete(oper, path);
        reset(tx);

        // In case where the flow ID list is not empty.
        VtnFlowId flowId = new VtnFlowId(BigInteger.valueOf(1L));
        FlowIdList fid = new FlowIdListBuilder().setFlowId(flowId).build();
        list = Collections.singletonList(fid);
        set = new NodeFlowsBuilder().setFlowIdList(list).build();
        when(tx.read(oper, path)).thenReturn(getReadResult(set));
        assertEquals(false, FlowUtils.removeFlowIdSet(tx, path));
        verify(tx).read(oper, path);
        verify(tx, never()).delete(oper, path);
    }

    /**
     * Test case for
     * {@link FlowUtils#removeIndex(ReadWriteTransaction,InstanceIdentifier,VTNDataFlow)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testRemoveIndex1() throws Exception {
        String tenant = "vtn";
        VtnFlowTableKey tkey = new VtnFlowTableKey(tenant);
        InstanceIdentifier<VtnFlowTable> tpath = InstanceIdentifier.
            builder(VtnFlows.class).
            child(VtnFlowTable.class, tkey).
            build();
        VtnFlowId flowId = new VtnFlowId(BigInteger.valueOf(99999999L));
        FlowIdListKey fidKey = new FlowIdListKey(flowId);
        FlowIdList indexed = new FlowIdListBuilder().
            setFlowId(flowId).build();
        VtnDataFlowKey dfKey = new VtnDataFlowKey(flowId);
        String condKey = "cond-key";

        VtnFlowId another = new VtnFlowId(BigInteger.valueOf(1L));
        FlowIdList anotherFid = new FlowIdListBuilder().
            setFlowId(another).build();
        List<FlowIdList> fidNull = null;
        List<FlowIdList> fidList = Collections.singletonList(anotherFid);
        List<FlowIdList> fidEmpty = Collections.<FlowIdList>emptyList();

        // fidEmpty means that the index should be removed.
        // fidList means that the index should be retained.
        // fidNull means that the flow is not indexed.
        Map<SalNode, List<FlowIdList>> nodeMap = new HashMap<>();
        nodeMap.put(new SalNode(10L), fidList);
        nodeMap.put(new SalNode(20L), fidList);
        nodeMap.put(new SalNode(-1L), fidEmpty);
        nodeMap.put(new SalNode(12345678L), fidNull);
        Set<SalNode> nodeSet = nodeMap.keySet();

        Map<SalPort, List<FlowIdList>> portMap = new HashMap<>();
        portMap.put(new SalPort(10L, 33L), fidEmpty);
        portMap.put(new SalPort(10L, 7L), fidList);
        portMap.put(new SalPort(20L, 888L), fidEmpty);
        portMap.put(new SalPort(20L, 2345L), fidList);
        portMap.put(new SalPort(-1L, 3L), fidEmpty);
        portMap.put(new SalPort(-1L, 0xffffff00L), fidList);
        portMap.put(new SalPort(12345678L, 77L), fidNull);
        portMap.put(new SalPort(12345678L, 3333L), fidNull);
        Set<SalPort> portSet = portMap.keySet();
        long mac = 0x7777777777L;
        short vlan = 0;
        SourceHostFlowsKey src = new MacVlan(mac, vlan).
            getSourceHostFlowsKey();
        VTNDataFlow df = mock(VTNDataFlow.class);
        when(df.getKey()).thenReturn(dfKey);
        when(df.getIngressMatchKey()).thenReturn(condKey);
        when(df.getFlowNodes()).thenReturn(nodeSet);
        when(df.getFlowPorts()).thenReturn(portSet);
        when(df.getSourceHostFlowsKey()).thenReturn(src);

        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        InstanceIdentifier<MatchFlows> mpath = InstanceIdentifier.
            builder(VtnFlows.class).
            child(VtnFlowTable.class, tkey).
            child(MatchFlows.class, new MatchFlowsKey(condKey)).
            build();
        MatchFlows mflows = new MatchFlowsBuilder().
            setConditionKey(condKey).setFlowId(another).build();
        when(tx.read(oper, mpath)).thenReturn(getReadResult(mflows));

        for (Map.Entry<SalNode, List<FlowIdList>> entry: nodeMap.entrySet()) {
            SalNode snode = entry.getKey();
            NodeId nodeId = snode.getNodeId();
            List<FlowIdList> flist = entry.getValue();
            NodeFlowsKey nkey = new NodeFlowsKey(nodeId);
            InstanceIdentifier<FlowIdList> idxPath = InstanceIdentifier.
                builder(VtnFlows.class).
                child(VtnFlowTable.class, tkey).
                child(NodeFlows.class, nkey).
                child(FlowIdList.class, fidKey).
                build();
            FlowIdList fid = (flist == null) ? null : indexed;
            when(tx.read(oper, idxPath)).thenReturn(getReadResult(fid));
            if (fid != null) {
                InstanceIdentifier<NodeFlows> npath = InstanceIdentifier.
                    builder(VtnFlows.class).
                    child(VtnFlowTable.class, tkey).
                    child(NodeFlows.class, nkey).
                    build();
                NodeFlows nflows = new NodeFlowsBuilder().
                    setNode(nodeId).setFlowIdList(flist).build();
                when(tx.read(oper, npath)).thenReturn(getReadResult(nflows));
            }
        }

        for (Map.Entry<SalPort, List<FlowIdList>> entry: portMap.entrySet()) {
            SalPort sport = entry.getKey();
            NodeConnectorId portId = sport.getNodeConnectorId();
            List<FlowIdList> flist = entry.getValue();
            PortFlowsKey pkey = new PortFlowsKey(portId);
            InstanceIdentifier<FlowIdList> idxPath = InstanceIdentifier.
                builder(VtnFlows.class).
                child(VtnFlowTable.class, tkey).
                child(PortFlows.class, pkey).
                child(FlowIdList.class, fidKey).
                build();
            FlowIdList fid = (flist == null) ? null : indexed;
            when(tx.read(oper, idxPath)).thenReturn(getReadResult(fid));
            if (fid != null) {
                InstanceIdentifier<PortFlows> ppath = InstanceIdentifier.
                    builder(VtnFlows.class).
                    child(VtnFlowTable.class, tkey).
                    child(PortFlows.class, pkey).
                    build();
                PortFlows pflows = new PortFlowsBuilder().
                    setPort(portId).setFlowIdList(flist).build();
                when(tx.read(oper, ppath)).thenReturn(getReadResult(pflows));
            }
        }

        // In case where the source host flow should be retained.
        InstanceIdentifier<FlowIdList> srcIdxPath = InstanceIdentifier.
            builder(VtnFlows.class).
            child(VtnFlowTable.class, tkey).
            child(SourceHostFlows.class, src).
            child(FlowIdList.class, fidKey).
            build();
        when(tx.read(oper, srcIdxPath)).thenReturn(getReadResult(indexed));
        InstanceIdentifier<SourceHostFlows> spath = InstanceIdentifier.
            builder(VtnFlows.class).
            child(VtnFlowTable.class, tkey).
            child(SourceHostFlows.class, src).
            build();
        SourceHostFlows sflows = new SourceHostFlowsBuilder().
            setKey(src).setFlowIdList(fidList).build();
        when(tx.read(oper, spath)).thenReturn(getReadResult(sflows));

        FlowUtils.removeIndex(tx, tpath, df);

        verify(df).getKey();
        verify(df).getIngressMatchKey();
        verify(df).getFlowNodes();
        verify(df).getFlowPorts();
        verify(df).getSourceHostFlowsKey();

        verify(tx).read(oper, mpath);
        verify(tx).delete(oper, mpath);

        for (Map.Entry<SalNode, List<FlowIdList>> entry: nodeMap.entrySet()) {
            SalNode snode = entry.getKey();
            NodeId nodeId = snode.getNodeId();
            List<FlowIdList> flist = entry.getValue();
            NodeFlowsKey nkey = new NodeFlowsKey(nodeId);
            InstanceIdentifier<FlowIdList> idxPath = InstanceIdentifier.
                builder(VtnFlows.class).
                child(VtnFlowTable.class, tkey).
                child(NodeFlows.class, nkey).
                child(FlowIdList.class, fidKey).
                build();
            verify(tx).read(oper, idxPath);

            InstanceIdentifier<NodeFlows> npath = InstanceIdentifier.
                builder(VtnFlows.class).
                child(VtnFlowTable.class, tkey).
                child(NodeFlows.class, nkey).
                build();
            if (flist == null) {
                verify(tx, never()).read(oper, npath);
            } else {
                verify(tx).read(oper, npath);
                if (flist.isEmpty()) {
                    verify(tx).delete(oper, npath);
                } else {
                    verify(tx, never()).delete(oper, npath);
                }
            }
        }

        for (Map.Entry<SalPort, List<FlowIdList>> entry: portMap.entrySet()) {
            SalPort sport = entry.getKey();
            NodeConnectorId portId = sport.getNodeConnectorId();
            List<FlowIdList> flist = entry.getValue();
            PortFlowsKey pkey = new PortFlowsKey(portId);
            InstanceIdentifier<FlowIdList> idxPath = InstanceIdentifier.
                builder(VtnFlows.class).
                child(VtnFlowTable.class, tkey).
                child(PortFlows.class, pkey).
                child(FlowIdList.class, fidKey).
                build();
            verify(tx).read(oper, idxPath);

            InstanceIdentifier<PortFlows> ppath = InstanceIdentifier.
                builder(VtnFlows.class).
                child(VtnFlowTable.class, tkey).
                child(PortFlows.class, pkey).
                build();
            if (flist == null) {
                verify(tx, never()).read(oper, ppath);
            } else {
                verify(tx).read(oper, ppath);
                if (flist.isEmpty()) {
                    verify(tx).delete(oper, ppath);
                } else {
                    verify(tx, never()).delete(oper, ppath);
                }
            }
        }

        verify(tx).read(oper, srcIdxPath);
        verify(tx).read(oper, spath);
        verify(tx, never()).delete(oper, spath);
    }

    /**
     * Test case for
     * {@link FlowUtils#removeIndex(ReadWriteTransaction,InstanceIdentifier,VTNDataFlow,FlowIdListKey)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testRemoveIndex2() throws Exception {
        String tenant = "vtn1";
        VtnFlowTableKey tkey = new VtnFlowTableKey(tenant);
        InstanceIdentifier<VtnFlowTable> tpath = InstanceIdentifier.
            builder(VtnFlows.class).
            child(VtnFlowTable.class, tkey).
            build();
        VtnFlowId flowId = new VtnFlowId(BigInteger.valueOf(12345L));
        FlowIdListKey fidKey = new FlowIdListKey(flowId);
        FlowIdList indexed = new FlowIdListBuilder().
            setFlowId(flowId).build();
        VtnDataFlowKey dfKey = new VtnDataFlowKey(flowId);
        String condKey = "cond-key1";

        VtnFlowId another = new VtnFlowId(BigInteger.valueOf(1L));
        FlowIdList anotherFid = new FlowIdListBuilder().
            setFlowId(another).build();
        List<FlowIdList> fidNull = null;
        List<FlowIdList> fidList = Collections.singletonList(anotherFid);
        List<FlowIdList> fidEmpty = Collections.<FlowIdList>emptyList();

        // fidEmpty means that the index should be removed.
        // fidList means that the index should be retained.
        // fidNull means that the flow is not indexed.
        Map<SalNode, List<FlowIdList>> nodeMap = new HashMap<>();
        nodeMap.put(new SalNode(1L), fidList);
        nodeMap.put(new SalNode(2L), fidEmpty);
        nodeMap.put(new SalNode(-1L), fidList);
        nodeMap.put(new SalNode(12345L), fidNull);
        Set<SalNode> nodeSet = nodeMap.keySet();

        Map<SalPort, List<FlowIdList>> portMap = new HashMap<>();
        portMap.put(new SalPort(1L, 2L), fidEmpty);
        portMap.put(new SalPort(1L, 15L), fidList);
        portMap.put(new SalPort(2L, 33L), fidList);
        portMap.put(new SalPort(2L, 123L), fidList);
        portMap.put(new SalPort(-1L, 3L), fidList);
        portMap.put(new SalPort(-1L, 0xffffff00L), fidEmpty);
        portMap.put(new SalPort(12345L, 77L), fidNull);
        Set<SalPort> portSet = portMap.keySet();
        long mac = 0x123456789L;
        short vlan = 3;
        SourceHostFlowsKey src = new MacVlan(mac, vlan).
            getSourceHostFlowsKey();
        VTNDataFlow df = mock(VTNDataFlow.class);
        when(df.getKey()).thenReturn(dfKey);
        when(df.getIngressMatchKey()).thenReturn(condKey);
        when(df.getFlowNodes()).thenReturn(nodeSet);
        when(df.getFlowPorts()).thenReturn(portSet);
        when(df.getSourceHostFlowsKey()).thenReturn(src);

        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        InstanceIdentifier<MatchFlows> mpath = InstanceIdentifier.
            builder(VtnFlows.class).
            child(VtnFlowTable.class, tkey).
            child(MatchFlows.class, new MatchFlowsKey(condKey)).
            build();
        MatchFlows mflows = new MatchFlowsBuilder().
            setConditionKey(condKey).setFlowId(another).build();
        when(tx.read(oper, mpath)).thenReturn(getReadResult(mflows));

        for (Map.Entry<SalNode, List<FlowIdList>> entry: nodeMap.entrySet()) {
            SalNode snode = entry.getKey();
            NodeId nodeId = snode.getNodeId();
            List<FlowIdList> flist = entry.getValue();
            NodeFlowsKey nkey = new NodeFlowsKey(nodeId);
            InstanceIdentifier<FlowIdList> idxPath = InstanceIdentifier.
                builder(VtnFlows.class).
                child(VtnFlowTable.class, tkey).
                child(NodeFlows.class, nkey).
                child(FlowIdList.class, fidKey).
                build();
            FlowIdList fid = (flist == null) ? null : indexed;
            when(tx.read(oper, idxPath)).thenReturn(getReadResult(fid));
            if (fid != null) {
                InstanceIdentifier<NodeFlows> npath = InstanceIdentifier.
                    builder(VtnFlows.class).
                    child(VtnFlowTable.class, tkey).
                    child(NodeFlows.class, nkey).
                    build();
                NodeFlows nflows = new NodeFlowsBuilder().
                    setNode(nodeId).setFlowIdList(flist).build();
                when(tx.read(oper, npath)).thenReturn(getReadResult(nflows));
            }
        }

        for (Map.Entry<SalPort, List<FlowIdList>> entry: portMap.entrySet()) {
            SalPort sport = entry.getKey();
            NodeConnectorId portId = sport.getNodeConnectorId();
            List<FlowIdList> flist = entry.getValue();
            PortFlowsKey pkey = new PortFlowsKey(portId);
            InstanceIdentifier<FlowIdList> idxPath = InstanceIdentifier.
                builder(VtnFlows.class).
                child(VtnFlowTable.class, tkey).
                child(PortFlows.class, pkey).
                child(FlowIdList.class, fidKey).
                build();
            FlowIdList fid = (flist == null) ? null : indexed;
            when(tx.read(oper, idxPath)).thenReturn(getReadResult(fid));
            if (fid != null) {
                InstanceIdentifier<PortFlows> ppath = InstanceIdentifier.
                    builder(VtnFlows.class).
                    child(VtnFlowTable.class, tkey).
                    child(PortFlows.class, pkey).
                    build();
                PortFlows pflows = new PortFlowsBuilder().
                    setPort(portId).setFlowIdList(flist).build();
                when(tx.read(oper, ppath)).thenReturn(getReadResult(pflows));
            }
        }

        // In case where only the source host is indexed.
        InstanceIdentifier<FlowIdList> srcIdxPath = InstanceIdentifier.
            builder(VtnFlows.class).
            child(VtnFlowTable.class, tkey).
            child(SourceHostFlows.class, src).
            child(FlowIdList.class, fidKey).
            build();
        when(tx.read(oper, srcIdxPath)).thenReturn(getReadResult(indexed));
        InstanceIdentifier<SourceHostFlows> spath = InstanceIdentifier.
            builder(VtnFlows.class).
            child(VtnFlowTable.class, tkey).
            child(SourceHostFlows.class, src).
            build();
        SourceHostFlows sflows = new SourceHostFlowsBuilder().
            setKey(src).setFlowIdList(fidEmpty).build();
        when(tx.read(oper, spath)).thenReturn(getReadResult(sflows));

        FlowUtils.removeIndex(tx, tpath, df, fidKey);

        verify(df, never()).getKey();
        verify(df).getIngressMatchKey();
        verify(df).getFlowNodes();
        verify(df).getFlowPorts();
        verify(df).getSourceHostFlowsKey();

        verify(tx).read(oper, mpath);
        verify(tx).delete(oper, mpath);

        for (Map.Entry<SalNode, List<FlowIdList>> entry: nodeMap.entrySet()) {
            SalNode snode = entry.getKey();
            NodeId nodeId = snode.getNodeId();
            List<FlowIdList> flist = entry.getValue();
            NodeFlowsKey nkey = new NodeFlowsKey(nodeId);
            InstanceIdentifier<FlowIdList> idxPath = InstanceIdentifier.
                builder(VtnFlows.class).
                child(VtnFlowTable.class, tkey).
                child(NodeFlows.class, nkey).
                child(FlowIdList.class, fidKey).
                build();
            verify(tx).read(oper, idxPath);

            InstanceIdentifier<NodeFlows> npath = InstanceIdentifier.
                builder(VtnFlows.class).
                child(VtnFlowTable.class, tkey).
                child(NodeFlows.class, nkey).
                build();
            if (flist == null) {
                verify(tx, never()).read(oper, npath);
            } else {
                verify(tx).read(oper, npath);
                if (flist.isEmpty()) {
                    verify(tx).delete(oper, npath);
                } else {
                    verify(tx, never()).delete(oper, npath);
                }
            }
        }

        for (Map.Entry<SalPort, List<FlowIdList>> entry: portMap.entrySet()) {
            SalPort sport = entry.getKey();
            NodeConnectorId portId = sport.getNodeConnectorId();
            List<FlowIdList> flist = entry.getValue();
            PortFlowsKey pkey = new PortFlowsKey(portId);
            InstanceIdentifier<FlowIdList> idxPath = InstanceIdentifier.
                builder(VtnFlows.class).
                child(VtnFlowTable.class, tkey).
                child(PortFlows.class, pkey).
                child(FlowIdList.class, fidKey).
                build();
            verify(tx).read(oper, idxPath);

            InstanceIdentifier<PortFlows> ppath = InstanceIdentifier.
                builder(VtnFlows.class).
                child(VtnFlowTable.class, tkey).
                child(PortFlows.class, pkey).
                build();
            if (flist == null) {
                verify(tx, never()).read(oper, ppath);
            } else {
                verify(tx).read(oper, ppath);
                if (flist.isEmpty()) {
                    verify(tx).delete(oper, ppath);
                } else {
                    verify(tx, never()).delete(oper, ppath);
                }
            }
        }

        verify(tx).read(oper, srcIdxPath);
        verify(tx).read(oper, spath);
        verify(tx).delete(oper, spath);

        // In case where the flow is not indexed.
        nodeSet = Collections.<SalNode>emptySet();
        portSet = Collections.<SalPort>emptySet();
        src = null;
        reset(df);
        when(df.getKey()).thenReturn(dfKey);
        when(df.getIngressMatchKey()).thenReturn(condKey);
        when(df.getFlowNodes()).thenReturn(nodeSet);
        when(df.getFlowPorts()).thenReturn(portSet);
        when(df.getSourceHostFlowsKey()).thenReturn(src);

        reset(tx);
        mflows = null;
        when(tx.read(oper, mpath)).thenReturn(getReadResult(mflows));

        FlowUtils.removeIndex(tx, tpath, df, fidKey);

        verify(df, never()).getKey();
        verify(df).getIngressMatchKey();
        verify(df).getFlowNodes();
        verify(df).getFlowPorts();
        verify(df).getSourceHostFlowsKey();

        verify(tx, never()).delete(any(LogicalDatastoreType.class),
                                   any(InstanceIdentifier.class));
        verify(tx).read(oper, mpath);

        for (SalNode snode: nodeMap.keySet()) {
            NodeFlowsKey nkey = new NodeFlowsKey(snode.getNodeId());
            InstanceIdentifier<FlowIdList> idxPath = InstanceIdentifier.
                builder(VtnFlows.class).
                child(VtnFlowTable.class, tkey).
                child(NodeFlows.class, nkey).
                child(FlowIdList.class, fidKey).
                build();
            verify(tx, never()).read(oper, idxPath);

            InstanceIdentifier<NodeFlows> npath = InstanceIdentifier.
                builder(VtnFlows.class).
                child(VtnFlowTable.class, tkey).
                child(NodeFlows.class, nkey).
                build();
            verify(tx, never()).read(oper, npath);
        }

        for (SalPort sport: portMap.keySet()) {
            PortFlowsKey pkey = new PortFlowsKey(sport.getNodeConnectorId());
            InstanceIdentifier<FlowIdList> idxPath = InstanceIdentifier.
                builder(VtnFlows.class).
                child(VtnFlowTable.class, tkey).
                child(PortFlows.class, pkey).
                child(FlowIdList.class, fidKey).
                build();
            verify(tx, never()).read(oper, idxPath);

            InstanceIdentifier<PortFlows> ppath = InstanceIdentifier.
                builder(VtnFlows.class).
                child(VtnFlowTable.class, tkey).
                child(PortFlows.class, pkey).
                build();
            verify(tx, never()).read(oper, ppath);
        }

        verify(tx, never()).read(oper, srcIdxPath);
        verify(tx, never()).read(oper, spath);
    }

    /**
     * Test case for
     * {@link FlowUtils#removeDataFlow(ReadWriteTransaction,String,VTNDataFlow)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testRemoveDataFlow() throws Exception {
        // In case where the target data flow is not present.
        String tenant = "vtn2";
        VtnFlowTableKey tkey = new VtnFlowTableKey(tenant);
        VtnFlowId flowId = new VtnFlowId(BigInteger.valueOf(7777777L));
        VtnDataFlowKey dfKey = new VtnDataFlowKey(flowId);
        String condKey = "cond-key2";
        Set<SalNode> nodeSet = Collections.<SalNode>emptySet();
        Set<SalPort> portSet = Collections.<SalPort>emptySet();
        SourceHostFlowsKey src = null;

        VTNDataFlow df = mock(VTNDataFlow.class);
        when(df.getKey()).thenReturn(dfKey);
        when(df.getIngressMatchKey()).thenReturn(condKey);
        when(df.getFlowNodes()).thenReturn(nodeSet);
        when(df.getFlowPorts()).thenReturn(portSet);
        when(df.getSourceHostFlowsKey()).thenReturn(src);

        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        InstanceIdentifier<VtnDataFlow> fpath = InstanceIdentifier.
            builder(VtnFlows.class).
            child(VtnFlowTable.class, tkey).
            child(VtnDataFlow.class, dfKey).
            build();
        VtnDataFlow vdf = null;
        when(tx.read(oper, fpath)).thenReturn(getReadResult(vdf));

        assertEquals(false, FlowUtils.removeDataFlow(tx, tenant, df));
        verify(df).getKey();
        verify(df, never()).getIngressMatchKey();
        verify(df, never()).getFlowNodes();
        verify(df, never()).getFlowPorts();
        verify(df, never()).getSourceHostFlowsKey();

        verify(tx).read(oper, fpath);
        verify(tx, never()).delete(oper, fpath);

        // In case where the target data flow is present but not indexed.
        reset(df);
        when(df.getKey()).thenReturn(dfKey);
        when(df.getIngressMatchKey()).thenReturn(condKey);
        when(df.getFlowNodes()).thenReturn(nodeSet);
        when(df.getFlowPorts()).thenReturn(portSet);
        when(df.getSourceHostFlowsKey()).thenReturn(src);

        reset(tx);
        vdf = new VtnDataFlowBuilder().setFlowId(flowId).build();
        when(tx.read(oper, fpath)).thenReturn(getReadResult(vdf));

        InstanceIdentifier<MatchFlows> mpath = InstanceIdentifier.
            builder(VtnFlows.class).
            child(VtnFlowTable.class, tkey).
            child(MatchFlows.class, new MatchFlowsKey(condKey)).
            build();
        MatchFlows mflows = null;
        when(tx.read(oper, mpath)).thenReturn(getReadResult(mflows));

        assertEquals(true, FlowUtils.removeDataFlow(tx, tenant, df));
        verify(df, times(2)).getKey();
        verify(df).getIngressMatchKey();
        verify(df).getFlowNodes();
        verify(df).getFlowPorts();
        verify(df).getSourceHostFlowsKey();

        // The data flow should be deleted.
        verify(tx).read(oper, fpath);
        verify(tx).delete(oper, fpath);

        // removeIndex() should be called.
        verify(tx).read(oper, mpath);
        verify(tx, never()).delete(oper, mpath);
    }

    /**
     * Test case for
     * {@link FlowUtils#removeFlowEntries(SalFlowService,List,InventoryReader)}.
     */
    @Test
    public void testRemoveFlowEntries1() throws Exception {
        // In case where no data flow is specified.
        List<FlowCache> flows = new ArrayList<>();
        ReadTransaction rtx = mock(ReadTransaction.class);
        InventoryReader reader = new InventoryReader(rtx);
        SalFlowService sfs = mock(SalFlowService.class);
        assertEquals(0, FlowUtils.removeFlowEntries(sfs, flows, reader).size());
        verifyZeroInteractions(sfs, rtx);

        // Create node information.
        long removedNode = 12345L;
        long[] nodeIds = {1L, -1L, removedNode, 33333333L, 99999L};
        for (long nodeId: nodeIds) {
            SalNode snode = new SalNode(nodeId);
            VtnNode vnode = (nodeId == removedNode)
                ? null
                : new VtnNodeBuilder().setId(snode.getNodeId()).build();
            reader.prefetch(snode, vnode);
        }

        // Create 4 data flows.
        long flowId = 1L;
        long portId = 1L;
        Set<RemoveFlowInput> inputs = new HashSet<>();
        Set<Future<RpcResult<RemoveFlowOutput>>> futures = new HashSet<>();
        while (flows.size() < 4) {
            int order = 0;
            List<VtnFlowEntry> entries = new ArrayList<>();
            Boolean barrier = (flows.size() == 3)
                ? Boolean.TRUE : Boolean.FALSE;
            for (long nodeId: nodeIds) {
                SalPort ingress = new SalPort(nodeId, portId);
                VtnFlowEntry vfent =
                    createVtnFlowEntry(flowId, order, ingress);
                entries.add(vfent);
                if (nodeId != removedNode) {
                    SalNode snode = new SalNode(nodeId);
                    RemoveFlowInput input = FlowUtils.
                        createRemoveFlowInputBuilder(snode, vfent).
                        setBarrier(barrier).
                        build();
                    Future<RpcResult<RemoveFlowOutput>> f =
                        new SettableVTNFuture<>();
                    when(sfs.removeFlow(input)).thenReturn(f);
                    inputs.add(input);
                    futures.add(f);
                }
                portId += 17L;
                order++;
            }

            VtnDataFlow vdf = new VtnDataFlowBuilder().
                setFlowId(new VtnFlowId(NumberUtils.getUnsigned(flowId))).
                setVtnFlowEntry(entries).
                build();
            flows.add(new FlowCache(vdf));
            flowId += 127L;
        }

        List<RemoveFlowRpc> result =
            FlowUtils.removeFlowEntries(sfs, flows, reader);
        verifyZeroInteractions(rtx);

        for (RemoveFlowRpc rpc: result) {
            assertEquals(true, inputs.remove(rpc.getInput()));
            assertEquals(true, futures.remove(rpc.getFuture()));
        }
        assertEquals(0, inputs.size());
        assertEquals(0, futures.size());
    }

    /**
     * Test case for
     * {@link FlowUtils#removeFlowEntries(SalFlowService,List,SalPort,InventoryReader)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testRemoveFlowEntries2() throws Exception {
        // In case where no data flow is specified.
        List<FlowCache> flows = new ArrayList<>();
        ReadTransaction rtx = mock(ReadTransaction.class);
        InventoryReader reader = new InventoryReader(rtx);
        SalFlowService sfs = mock(SalFlowService.class);
        long targetNode = 12345L;
        long targetPort = 99999999L;
        SalPort target = new SalPort(targetNode, targetPort);
        assertEquals(0, FlowUtils.removeFlowEntries(sfs, flows, target,
                                                    reader).size());
        verifyZeroInteractions(sfs);
        verifyZeroInteractions(rtx);

        // Create node information.
        long removedNode = 3141592L;
        long[] nodeIds = {1L, -1L, removedNode, targetNode, 9876543L};
        for (long nodeId: nodeIds) {
            SalNode snode = new SalNode(nodeId);
            VtnNode vnode = (nodeId == removedNode)
                ? null
                : new VtnNodeBuilder().setId(snode.getNodeId()).build();
            reader.prefetch(snode, vnode);
        }

        // Create 4 data flows.
        long flowId = 1L;
        long portId = 1L;
        boolean done = false;
        Set<RemoveFlowInput> inputs = new HashSet<>();
        Set<Future<RpcResult<RemoveFlowOutput>>> futures = new HashSet<>();
        while (flows.size() < 4) {
            int order = 0;
            List<VtnFlowEntry> entries = new ArrayList<>();
            Boolean barrier = (flows.size() == 3)
                ? Boolean.TRUE : Boolean.FALSE;
            for (long nodeId: nodeIds) {
                SalPort ingress = new SalPort(nodeId, portId);
                VtnFlowEntry vfent =
                    createVtnFlowEntry(flowId, order, ingress);
                entries.add(vfent);
                if (nodeId == targetNode) {
                    if (!done) {
                        SalNode snode = new SalNode(nodeId);
                        List<RemoveFlowInputBuilder> rfi = FlowUtils.
                            createRemoveFlowInputBuilder(snode, target);
                        Iterator<RemoveFlowInputBuilder> it = rfi.iterator();
                        while (it.hasNext()) {
                            RemoveFlowInputBuilder builder = it.next();
                            Boolean b = !it.hasNext();
                            RemoveFlowInput input = builder.setBarrier(b).
                                build();
                            Future<RpcResult<RemoveFlowOutput>> f =
                                new SettableVTNFuture<>();
                            when(sfs.removeFlow(input)).thenReturn(f);
                            inputs.add(input);
                            futures.add(f);
                        }
                        done = true;
                    }
                } else if (nodeId != removedNode) {
                    SalNode snode = new SalNode(nodeId);
                    RemoveFlowInput input = FlowUtils.
                        createRemoveFlowInputBuilder(snode, vfent).
                        setBarrier(barrier).
                        build();
                    Future<RpcResult<RemoveFlowOutput>> f =
                        new SettableVTNFuture<>();
                    when(sfs.removeFlow(input)).thenReturn(f);
                    inputs.add(input);
                    futures.add(f);
                }
                portId += 17L;
                order++;
            }

            VtnDataFlow vdf = new VtnDataFlowBuilder().
                setFlowId(new VtnFlowId(NumberUtils.getUnsigned(flowId))).
                setVtnFlowEntry(entries).
                build();
            flows.add(new FlowCache(vdf));
            flowId += 127L;
        }

        List<RemoveFlowRpc> result =
            FlowUtils.removeFlowEntries(sfs, flows, target, reader);
        verifyZeroInteractions(rtx);

        for (RemoveFlowRpc rpc: result) {
            assertEquals(true, inputs.remove(rpc.getInput()));
            assertEquals(true, futures.remove(rpc.getFuture()));
        }
        assertEquals(0, inputs.size());
        assertEquals(0, futures.size());

        // In case where the target node is not present.
        reset(sfs);

        for (long nodeId: nodeIds) {
            SalNode snode = new SalNode(nodeId);
            VtnNode vnode = (nodeId == removedNode || nodeId == targetNode)
                ? null
                : new VtnNodeBuilder().setId(snode.getNodeId()).build();
            reader.prefetch(snode, vnode);
        }

        Map<SalNode, RemoveFlowInput> lastInputs = new HashMap<>();
        List<RemoveFlowInput> inputList = new ArrayList<>();
        for (FlowCache fc: flows) {
            for (VtnFlowEntry vfent: fc.getFlowEntries()) {
                SalNode snode = SalNode.create(vfent.getNode());
                long nodeNumber = snode.getNodeNumber();
                if (nodeNumber != removedNode && nodeNumber != targetNode) {
                    RemoveFlowInput input = FlowUtils.
                        createRemoveFlowInputBuilder(snode, vfent).
                        build();
                    lastInputs.put(snode, input);
                    inputList.add(input);
                }
            }
        }

        Set<RemoveFlowInput> lastSet = new HashSet<>(lastInputs.values());
        for (RemoveFlowInput input: inputList) {
            Boolean barrier = lastSet.contains(input);
            RemoveFlowInput in = new RemoveFlowInputBuilder(input).
                setBarrier(barrier).
                build();
            Future<RpcResult<RemoveFlowOutput>> f =
                new SettableVTNFuture<>();
            when(sfs.removeFlow(in)).thenReturn(f);
            inputs.add(in);
            futures.add(f);
        }

        result = FlowUtils.removeFlowEntries(sfs, flows, target, reader);
        verifyZeroInteractions(rtx);

        for (RemoveFlowRpc rpc: result) {
            assertEquals(true, inputs.remove(rpc.getInput()));
            assertEquals(true, futures.remove(rpc.getFuture()));
        }
        assertEquals(0, inputs.size());
        assertEquals(0, futures.size());
    }

    /**
     * Test case for {@link FlowUtils#getFlowTables(ReadTransaction)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetFlowTables() throws Exception {
        InstanceIdentifier<VtnFlows> path =
            InstanceIdentifier.create(VtnFlows.class);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        ReadTransaction rtx = mock(ReadTransaction.class);

        // In case whether the top level container is not present.
        VtnFlows root = null;
        when(rtx.read(oper, path)).thenReturn(getReadResult(root));
        assertEquals(0, FlowUtils.getFlowTables(rtx).size());
        verify(rtx).read(oper, path);
        reset(rtx);

        // In case whether the flow table list is null.
        root = new VtnFlowsBuilder().build();
        when(rtx.read(oper, path)).thenReturn(getReadResult(root));
        assertEquals(0, FlowUtils.getFlowTables(rtx).size());
        verify(rtx).read(oper, path);
        reset(rtx);

        // In case whether the flow table list is empty.
        List<VtnFlowTable> tables = new ArrayList<>();
        root = new VtnFlowsBuilder().setVtnFlowTable(tables).build();
        when(rtx.read(oper, path)).thenReturn(getReadResult(root));
        assertEquals(0, FlowUtils.getFlowTables(rtx).size());
        verify(rtx).read(oper, path);
        reset(rtx);

        // In case whether the 2 flow tables are present.
        Collections.addAll(
            tables,
            new VtnFlowTableBuilder().setTenantName("vtn1").build(),
            new VtnFlowTableBuilder().setTenantName("vtn2").build());
        root = new VtnFlowsBuilder().setVtnFlowTable(tables).build();
        when(rtx.read(oper, path)).thenReturn(getReadResult(root));
        assertEquals(tables, FlowUtils.getFlowTables(rtx));
        verify(rtx).read(oper, path);
        reset(rtx);
    }

    /**
     * Test case for {@link FlowUtils#removedLog(Logger,String,FlowCache)}.
     */
    @Test
    public void testRemovedLog() {
        BigInteger[] flowIds = {
            BigInteger.valueOf(1L),
            BigInteger.valueOf(12345L),
            BigInteger.valueOf(0xfffffffffL),
        };
        String [] descriptions = {
            "desc1", "desc2", "desc3",
        };
        String format = "VTN data flow has been removed: remover={}, id={}";

        for (BigInteger flowId: flowIds) {
            VtnDataFlow vdf = new VtnDataFlowBuilder().
                setFlowId(new VtnFlowId(flowId)).
                build();
            FlowCache fc = new FlowCache(vdf);
            for (String desc: descriptions) {
                Logger logger = mock(Logger.class);
                FlowUtils.removedLog(logger, desc, fc);
                verify(logger).debug(format, desc, flowId);
            }
        }
    }

    /**
     * Create a list of {@link VNodeRoute} instances for test.
     *
     * @return  A list of {@link VNodeRoute} instances.
     */
    private List<VNodeRoute> createVNodeRouteList() {
        List<VNodeRoute> vroutes = new ArrayList<>();

        String[] tenants = {"vtn1", "vtn2"};
        String[] bridges = {"vbr1", "vbr2"};
        String[] interfaces = {"if1", "if2"};
        String[] vlanMaps = {"ANY.0", "OF:00:00:00:00:00:00:00:00.4095"};
        MacVlan[] hosts = {
            new MacVlan(0x1000L), new MacVlan(0xfffffffffffffffL),
        };
        VirtualRouteReason forwarded = VirtualRouteReason.FORWARDED;
        VirtualRouteReason redirected = VirtualRouteReason.REDIRECTED;
        VirtualRouteReason portmapped = VirtualRouteReason.PORTMAPPED;
        VirtualRouteReason vlanmapped = VirtualRouteReason.VLANMAPPED;
        VirtualRouteReason macmapped = VirtualRouteReason.MACMAPPED;

        for (String tname: tenants) {
            for (String bname: bridges) {
                VBridgePath bpath = new VBridgePath(tname, bname);
                vroutes.add(new VNodeRoute(bpath, forwarded));
                vroutes.add(new VNodeRoute(bpath, redirected));

                VNodePath path = new MacMapPath(bpath);
                vroutes.add(new VNodeRoute(path, macmapped));
                vroutes.add(new VNodeRoute(path, forwarded));

                for (MacVlan host: hosts) {
                    path = new MacMappedHostPath(bpath, host);
                    vroutes.add(new VNodeRoute(path, macmapped));
                    vroutes.add(new VNodeRoute(path, forwarded));
                }

                for (String vmap: vlanMaps) {
                    path = new VlanMapPath(bpath, vmap);
                    vroutes.add(new VNodeRoute(path, vlanmapped));
                    vroutes.add(new VNodeRoute(path, forwarded));
                }

                VTerminalPath vtpath = new VTerminalPath(tname, bname);
                vroutes.add(new VNodeRoute(vtpath, forwarded));
                vroutes.add(new VNodeRoute(vtpath, redirected));

                for (String iname: interfaces) {
                    path = new VBridgeIfPath(bpath, iname);
                    vroutes.add(new VNodeRoute(path, portmapped));
                    vroutes.add(new VNodeRoute(path, redirected));
                    vroutes.add(new VNodeRoute(path, forwarded));

                    path = new VTerminalIfPath(vtpath, iname);
                    vroutes.add(new VNodeRoute(path, portmapped));
                    vroutes.add(new VNodeRoute(path, redirected));
                    vroutes.add(new VNodeRoute(path, forwarded));
                }
            }
        }
        vroutes.add(new VNodeRoute());

        return vroutes;
    }

    /**
     * Verify the given {@link VirtualRoute} instance.
     *
     * @param vr      A {@link VirtualRoute} instance to be verified.
     * @param vroute  A {@link VNodeRoute} instance identical to {@code vr}.
     * @param order   The expected order value.
     */
    private void checkVirtualRoute(VirtualRoute vr, VNodeRoute vroute,
                                   int order) {
        assertEquals(Integer.valueOf(order), vr.getOrder());
        assertEquals(vroute.getReason(), vr.getReason());

        VNodePath vpath = vroute.getPath();
        VirtualNodePath vnp = vr.getVirtualNodePath();
        if (vpath == null) {
            assertEquals(null, vnp);
            return;
        }

        assertEquals(vpath.getTenantName(), vnp.getTenantName());
        BridgeMapInfo minfo = vnp.getAugmentation(BridgeMapInfo.class);

        if (vpath instanceof VBridgePath) {
            VBridgePath bpath = (VBridgePath)vpath;
            assertEquals(bpath.getBridgeName(), vnp.getBridgeName());
            assertEquals(null, vnp.getTerminalName());
            assertEquals(null, vnp.getRouterName());

            if (vpath instanceof VBridgeIfPath) {
                VBridgeIfPath ipath = (VBridgeIfPath)vpath;
                assertEquals(ipath.getInterfaceName(), vnp.getInterfaceName());
                assertEquals(null, minfo);
            } else {
                assertEquals(null, vnp.getInterfaceName());
                if (vpath instanceof VlanMapPath) {
                    VlanMapPath vmpath = (VlanMapPath)vpath;
                    assertEquals(vmpath.getMapId(), minfo.getVlanMapId());
                    assertEquals(null, minfo.getMacMappedHost());
                } else if (vpath instanceof MacMapPath) {
                    assertEquals(null, minfo.getVlanMapId());
                    if (vpath instanceof MacMappedHostPath) {
                        MacMappedHostPath mhpath = (MacMappedHostPath)vpath;
                        MacVlan mv = mhpath.getMappedHost();
                        assertEquals(Long.valueOf(mv.getEncodedValue()),
                                     minfo.getMacMappedHost());
                    } else {
                        assertEquals(Long.valueOf(-1L),
                                     minfo.getMacMappedHost());
                    }
                } else {
                    assertEquals(null, minfo);
                }
            }
        } else if (vpath instanceof VTerminalPath) {
            VTerminalPath vtpath = (VTerminalPath)vpath;
            assertEquals(vtpath.getTerminalName(), vnp.getTerminalName());
            assertEquals(null, vnp.getBridgeName());
            assertEquals(null, vnp.getRouterName());
            assertEquals(null, minfo);

            if (vpath instanceof VTerminalIfPath) {
                VTerminalIfPath ipath = (VTerminalIfPath)vpath;
                assertEquals(ipath.getInterfaceName(), vnp.getInterfaceName());
            } else {
                assertEquals(null, vnp.getInterfaceName());
            }
        } else {
            fail("Unexpected path: " + vpath);
        }
    }

    /**
     * Create a list of physical packet route for test.
     *
     * @param phlist   A list to store {@link PhysicalRoute} instance.
     * @param nroutes  A list to store {@link NodeRoute} instance.
     * @param node     The identifier of the switch.
     * @param inPort   The identifier of the ingress switch port.
     * @param inName   The name of the ingress switch port.
     * @param outPort  The identifier of the egress switch port.
     * @param outName  The name of the egress switch port.
     */
    private void createPhysicalRoute(
        List<PhysicalRoute> phlist, List<NodeRoute> nroutes, long node,
        long inPort, String inName, long outPort, String outName) {
        String sin = Long.toString(inPort);
        String sout = Long.toString(outPort);
        PhysicalIngressPort in = new PhysicalIngressPortBuilder().
            setPortId(sin).
            setPortName(inName).
            build();
        PhysicalEgressPort out = new PhysicalEgressPortBuilder().
            setPortId(sout).
            setPortName(outName).
            build();
        BigInteger dpid = NumberUtils.getUnsigned(node);
        NodeId nid = new NodeId("openflow:" + dpid);
        PhysicalRoute pr = new PhysicalRouteBuilder().
            setNode(nid).
            setOrder(phlist.size()).
            setPhysicalIngressPort(in).
            setPhysicalEgressPort(out).
            build();
        phlist.add(pr);

        String type = "OF";
        SwitchPort inSw = new SwitchPort(inName, type, sin);
        SwitchPort outSw = new SwitchPort(outName, type, sout);
        SalNode snode = SalNode.create(nid);
        NodeRoute nr = new NodeRoute(snode.getAdNode(), inSw, outSw);
        nroutes.add(nr);
    }

    /**
     * Test case for {@link FlowUtils#getTableId(GenericFlowAttributes)}.
     */
    @Test
    public void testGetTableId() {
        // Table ID should be zero if it is not present in flow.
        GenericFlowAttributes flow = new FlowBuilder().build();
        assertEquals((short)0, FlowUtils.getTableId(flow));

        short[] tableIds = {0, 1, 34, 67, 145, 255};
        for (short table: tableIds) {
            flow = new FlowBuilder().setTableId(table).build();
            assertEquals(table, FlowUtils.getTableId(flow));
        }
    }

    /**
     * Create a VTN flow entry for test.
     *
     * @param id       The VTN flow identifier.
     * @param order    The order of the flow entry.
     * @param ingress  A {@link SalPort} instance which indicates the ingress
     *                 switch port.
     * @throws Exception  An error occurred.
     */
    public static VtnFlowEntry createVtnFlowEntry(
        long id, int order, SalPort ingress) throws Exception {
        SalPort egress = new SalPort(ingress.getNodeNumber(), 9999L);
        NodeId node = ingress.getNodeId();
        Integer pri = 15;
        Integer idle = 400;
        Integer hard = 0;
        Short table = 0;
        FlowModFlags flags = new FlowModFlags(true, null, false, null, true);

        int srcVlan = 0;
        int dstVlan = 1000;
        EtherAddress srcMac = new EtherAddress(0x001122334455L);
        EtherAddress dstMac = new EtherAddress(0xa0b1c2d3e4f5L);
        Ip4Network srcIp = new Ip4Network("192.168.123.0/24");
        Ip4Network dstIp = new Ip4Network("10.20.30.40");
        VTNUdpMatch vumatch =
            new VTNUdpMatch(new VTNPortRange(12345), new VTNPortRange(23));
        VTNInet4Match vimatch =
            new VTNInet4Match(srcIp, dstIp, null, null);
        VTNEtherMatch vematch =
            new VTNEtherMatch(srcMac, dstMac, null, srcVlan, null);
        VTNMatch vmatch = new VTNMatch(vematch, vimatch, vumatch);
        Match match = vmatch.toMatchBuilder().
            setInPort(ingress.getNodeConnectorId()).
            build();

        EtherAddress newSrcMac = new EtherAddress(0xbaddcaffe123L);
        EtherAddress newDstMac = new EtherAddress(0x000011112222L);
        Ip4Network newSrcIp = new Ip4Network("192.168.10.254");
        Ip4Network newDstIp = new Ip4Network("172.16.99.123");

        List<VTNFlowAction> vactions = new ArrayList<>();
        Collections.addAll(
            vactions,
            new VTNSetDlSrcAction(newSrcMac),
            new VTNSetDlDstAction(newDstMac),
            new VTNSetVlanPcpAction((short)7),
            new VTNSetInetSrcAction(newSrcIp),
            new VTNSetInetDstAction(newDstIp),
            new VTNSetInetDscpAction((short)35),
            new VTNSetPortSrcAction(9999),
            new VTNSetPortDstAction(60000));
        VTNActionList actList = new VTNActionList().
            addVlanAction(srcVlan, dstVlan).
            addAll(vactions).
            addOutputAction(egress);
        Instructions insts = actList.toInstructions();
        long flowId = VTN_FLOW_COOKIE | id;
        FlowCookie cookie = new FlowCookie(NumberUtils.getUnsigned(flowId));

        return new VtnFlowEntryBuilder().
            setNode(node).
            setOrder(order).
            setPriority(pri).
            setTableId(table).
            setIdleTimeout(idle).
            setHardTimeout(hard).
            setCookie(cookie).
            setMatch(match).
            setFlags(flags).
            setInstructions(insts).
            build();
    }

    /**
     * Create a {@link Duration} instance.
     *
     * @param sec   The number of seconds.
     * @param nsec  The number of nanoseconds.
     */
    public static Duration createDuration(Long sec, Long nsec) {
        DurationBuilder builder = new DurationBuilder();
        if (sec != null) {
            builder.setSecond(new Counter32(sec));
        }
        if (nsec != null) {
            builder.setNanosecond(new Counter32(nsec));
        }

        return builder.build();
    }
}
