/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock.impl;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.Future;
import java.util.concurrent.TimeoutException;
import java.util.concurrent.atomic.AtomicLong;
import java.util.concurrent.locks.Lock;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.util.concurrent.Futures;

import org.opendaylight.vtn.manager.it.ofmock.OfMockUtils;

import org.opendaylight.controller.sal.binding.api.BindingAwareBroker.RoutedRpcRegistration;
import org.opendaylight.controller.sal.binding.api.NotificationProviderService;
import org.opendaylight.controller.sal.binding.api.RpcProviderRegistry;

import org.opendaylight.yangtools.concepts.Registration;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.yangtools.yang.binding.RpcService;
import org.opendaylight.yangtools.yang.common.RpcError.ErrorType;
import org.opendaylight.yangtools.yang.common.RpcResult;
import org.opendaylight.yangtools.yang.common.RpcResultBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnOpenflowVersion;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FeatureCapability;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNodeUpdated;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNodeUpdatedBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowFeatureCapabilityFlowStats;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.flow.node.SwitchFeaturesBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.AddFlowInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.AddFlowOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.AddFlowOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SwitchFlowRemovedBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.UpdateFlowInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.UpdateFlowOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.UpdateFlowOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.FlowsStatisticsUpdateBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.GetAggregateFlowStatisticsFromFlowTableForAllFlowsInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.GetAggregateFlowStatisticsFromFlowTableForAllFlowsOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.GetAggregateFlowStatisticsFromFlowTableForGivenMatchInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.GetAggregateFlowStatisticsFromFlowTableForGivenMatchOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.GetAllFlowStatisticsFromFlowTableInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.GetAllFlowStatisticsFromFlowTableOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.GetAllFlowsStatisticsFromAllFlowTablesInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.GetAllFlowsStatisticsFromAllFlowTablesOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.GetFlowStatisticsFromFlowTableInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.GetFlowStatisticsFromFlowTableOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.GetFlowStatisticsFromFlowTableOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.OpendaylightFlowStatisticsService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.flow.and.statistics.map.list.FlowAndStatisticsMapList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.flow.and.statistics.map.list.FlowAndStatisticsMapListBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.table.statistics.rev131215.GetFlowTablesStatisticsInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.table.statistics.rev131215.GetFlowTablesStatisticsOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.table.statistics.rev131215.OpendaylightFlowTableStatisticsService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.transaction.rev150304.TransactionId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.Flow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.FlowCookie;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.FlowModFlags;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.RemovedReasonFlags;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.Match;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.group.service.rev130918.AddGroupInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.group.service.rev130918.AddGroupOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.group.service.rev130918.RemoveGroupInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.group.service.rev130918.RemoveGroupOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.group.service.rev130918.SalGroupService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.group.service.rev130918.UpdateGroupInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.group.service.rev130918.UpdateGroupOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.group.statistics.rev131111.GetAllGroupStatisticsInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.group.statistics.rev131111.GetAllGroupStatisticsOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.group.statistics.rev131111.GetGroupDescriptionInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.group.statistics.rev131111.GetGroupDescriptionOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.group.statistics.rev131111.GetGroupFeaturesInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.group.statistics.rev131111.GetGroupFeaturesOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.group.statistics.rev131111.GetGroupStatisticsInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.group.statistics.rev131111.GetGroupStatisticsOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.group.statistics.rev131111.OpendaylightGroupStatisticsService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorRef;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeContext;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeRef;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeRemoved;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeRemovedBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeUpdated;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeUpdatedBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.Nodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.NodeKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.meter.service.rev130918.AddMeterInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.meter.service.rev130918.AddMeterOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.meter.service.rev130918.RemoveMeterInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.meter.service.rev130918.RemoveMeterOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.meter.service.rev130918.SalMeterService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.meter.service.rev130918.UpdateMeterInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.meter.service.rev130918.UpdateMeterOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.meter.statistics.rev131111.GetAllMeterConfigStatisticsInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.meter.statistics.rev131111.GetAllMeterConfigStatisticsOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.meter.statistics.rev131111.GetAllMeterStatisticsInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.meter.statistics.rev131111.GetAllMeterStatisticsOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.meter.statistics.rev131111.GetMeterFeaturesInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.meter.statistics.rev131111.GetMeterFeaturesOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.meter.statistics.rev131111.GetMeterStatisticsInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.meter.statistics.rev131111.GetMeterStatisticsOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.meter.statistics.rev131111.OpendaylightMeterStatisticsService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.statistics.types.rev130925.duration.Duration;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.statistics.types.rev130925.duration.DurationBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.module.config.rev141015.NodeConfigService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.module.config.rev141015.SetConfigInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.module.config.rev141015.SetConfigOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.module.config.rev141015.SetConfigOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.packet.service.rev130709.PacketProcessingService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.packet.service.rev130709.TransmitPacketInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.port.service.rev131107.SalPortService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.port.service.rev131107.UpdatePortInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.port.service.rev131107.UpdatePortOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.port.service.rev131107.UpdatePortOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.port.statistics.rev131214.GetAllNodeConnectorsStatisticsInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.port.statistics.rev131214.GetAllNodeConnectorsStatisticsOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.port.statistics.rev131214.GetNodeConnectorStatisticsInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.port.statistics.rev131214.GetNodeConnectorStatisticsOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.port.statistics.rev131214.OpendaylightPortStatisticsService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.queue.statistics.rev131216.GetAllQueuesStatisticsFromAllPortsInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.queue.statistics.rev131216.GetAllQueuesStatisticsFromAllPortsOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.queue.statistics.rev131216.GetAllQueuesStatisticsFromGivenPortInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.queue.statistics.rev131216.GetAllQueuesStatisticsFromGivenPortOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.queue.statistics.rev131216.GetQueueStatisticsFromGivenPortInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.queue.statistics.rev131216.GetQueueStatisticsFromGivenPortOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.queue.statistics.rev131216.OpendaylightQueueStatisticsService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.table.service.rev131026.SalTableService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.table.service.rev131026.UpdateTableInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.table.service.rev131026.UpdateTableOutput;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpAddress;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Ipv4Address;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.Counter32;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.Counter64;

/**
 * {@code OfNode} describes a mock-up of OpenFlow switch.
 */
public final class OfNode
    implements AutoCloseable, SalFlowService, SalGroupService, SalMeterService,
               SalTableService, SalPortService, PacketProcessingService,
               NodeConfigService, OpendaylightGroupStatisticsService,
               OpendaylightMeterStatisticsService,
               OpendaylightFlowStatisticsService,
               OpendaylightPortStatisticsService,
               OpendaylightFlowTableStatisticsService,
               OpendaylightQueueStatisticsService {
    /**
     * Logger instance.
     */
    private static final Logger  LOG = LoggerFactory.getLogger(OfNode.class);

    /**
     * The buffer size to be notified.
     */
    private static final long  NODE_BUFSIZE = 256L;

    /**
     * The number of packets to be set in flow statistics.
     */
    private static final long  PACKET_COUNT = 5555555L;

    /**
     * The number of bytes to be set in flow statistics.
     */
    private static final long  BYTE_COUNT = 1234567890L;

    /**
     * Duration (in seconds) of a flow entry to be set in flow statistics.
     */
    private static final long  DURATION_SEC = 12345L;

    /**
     * Duration (in nanoseconds) of a flow entry to be set in flow statistics.
     */
    private static final long  DURATION_NANOSEC = 666777888L;

    /**
     * Capabilities of the switch.
     */
    private static final List<Class<? extends FeatureCapability>> CAPABILITIES;

    /**
     * The ofmock provider service.
     */
    private final OfMockProvider  ofMockProvider;

    /**
     * OpenFlow protocol version.
     */
    private final VtnOpenflowVersion  ofVersion;

    /**
     * Datapath ID of this node.
     */
    private final BigInteger  datapathId;

    /**
     * The identifier string of this node.
     */
    private final String  nodeIdentifier;

    /**
     * The instance identifier of this node.
     */
    private final InstanceIdentifier<Node>  nodePath;

    /**
     * IP address of this node.
     */
    private final IpAddress  ipAddress;

    /**
     * A list of registrations of SAL services.
     */
    private final List<Registration>  registrations = new ArrayList<>();

    /**
     * Physical ports inside this node.
     */
    private final Map<String, OfPort>  physicalPorts = new HashMap<>();

    /**
     * A list of installed flow entries.
     */
    private final List<OfMockFlowEntry>  flowEntries = new LinkedList<>();

    /**
     * The identifier of the next transaction.
     */
    private final AtomicLong  nextTransactionId = new AtomicLong();

    /**
     * Initialize static configurations.
     */
    static {
        List<Class<? extends FeatureCapability>> cap = new ArrayList<>();
        cap.add(FlowFeatureCapabilityFlowStats.class);
        CAPABILITIES = Collections.unmodifiableList(cap);
    }

    /**
     * A thread that notifies flow statistics.
     */
    private final class FlowStatsThread extends Thread
        implements FlowTableScanner {
        /**
         * The transaction ID associated with the flow statistics request.
         */
        private final TransactionId  transactionId;

        /**
         * The condition to select flow entries to be notified.
         */
        private final Flow  condition;

        /**
         * A list of flow statistics.
         */
        private final List<FlowAndStatisticsMapList>  flowStats =
            new ArrayList<>();

        /**
         * Construct a new instance.
         *
         * @param xid   OpenFlow transaction ID.
         * @param cond  The MD-SAL flow entry which specifies the condition to
         *              select flow entries.
         */
        private FlowStatsThread(TransactionId xid, Flow cond) {
            super("FlowStatsThread: xid=" + xid.getValue());
            transactionId = xid;
            condition = cond;
        }

        // Runnable

        /**
         * Scan the flow table, and notify flow statistics.
         */
        @Override
        public void run() {
            try {
                scanFlowTable(condition, this);

                // Publish a flows-statistics-update notification.
                FlowsStatisticsUpdateBuilder builder =
                    new FlowsStatisticsUpdateBuilder();
                builder.setId(new NodeId(nodeIdentifier)).
                    setMoreReplies(false).
                    setTransactionId(transactionId).
                    setFlowAndStatisticsMapList(flowStats);

                ofMockProvider.publish(builder.build());
            } catch (RuntimeException e) {
                BigInteger xid = transactionId.getValue();
                LOG.error("Unexpected exception: xid=" + xid, e);
            }
        }

        // FlowTableScanner

        /**
         * Invoked when a flow entry to be notified has been found.
         *
         * @param ofent  The flow entry to be notified.
         * @return  {@code false}.
         */
        @Override
        public boolean flowEntryFound(OfMockFlowEntry ofent) {
            Duration duration = new DurationBuilder().
                setSecond(new Counter32(DURATION_SEC)).
                setNanosecond(new Counter32(DURATION_NANOSEC)).
                build();

            FlowAndStatisticsMapList fs = new FlowAndStatisticsMapListBuilder().
                setPacketCount(new Counter64(BigInteger.valueOf(PACKET_COUNT))).
                setByteCount(new Counter64(BigInteger.valueOf(BYTE_COUNT))).
                setDuration(duration).
                setPriority(Integer.valueOf(ofent.getPriority())).
                setTableId(Short.valueOf((short)ofent.getTableId())).
                setCookie(new FlowCookie(ofent.getCookie())).
                setMatch(ofent.getMatch()).
                setInstructions(ofent.getInstructions()).
                setFlags(ofent.getFlowModFlags()).
                setIdleTimeout(Integer.valueOf(ofent.getIdleTimeout())).
                setHardTimeout(Integer.valueOf(ofent.getHardTimeout())).
                build();
            flowStats.add(fs);

            return false;
        }
    }

    /**
     * Construct a new instance.
     *
     * @param provider  The ofmock provider service.
     * @param ver       OpenFlow protocol version.
     * @param nsv       A {@link NotificationProviderService} service instance.
     * @param prefix    Protocol prefix of the node.
     * @param dpid      Datapath ID of the node.
     */
    public OfNode(OfMockProvider provider, VtnOpenflowVersion ver,
                  NotificationProviderService nsv, String prefix,
                  BigInteger dpid) {
        ofMockProvider = provider;
        ofVersion = ver;
        datapathId = dpid;
        nodeIdentifier = prefix + dpid;
        nodePath = InstanceIdentifier.builder(Nodes.class).
            child(Node.class, new NodeKey(new NodeId(nodeIdentifier))).
            build();

        int ip = (int)(dpid.longValue() & OfMockUtils.MASK_BYTE);
        if (ip == 0 || ip == OfMockUtils.MASK_BYTE) {
            ip = 1;
        }
        ipAddress = new IpAddress(new Ipv4Address("192.168.111." + ip));
    }

    /**
     * Return the node identifier.
     *
     * @return  The node identifier.
     */
    public String getNodeIdentifier() {
        return nodeIdentifier;
    }

    /**
     * Register this node to MD-SAL.
     *
     * @param rpcReg  A {@link RpcProviderRegistry} service instance.
     */
    public void register(RpcProviderRegistry rpcReg) {
        register(rpcReg, SalFlowService.class, this);
        register(rpcReg, SalPortService.class, this);
        register(rpcReg, SalMeterService.class, this);
        register(rpcReg, SalGroupService.class, this);
        register(rpcReg, SalTableService.class, this);
        register(rpcReg, PacketProcessingService.class, this);
        register(rpcReg, OpendaylightFlowStatisticsService.class, this);
        register(rpcReg, OpendaylightGroupStatisticsService.class, this);
        register(rpcReg, OpendaylightMeterStatisticsService.class, this);
        register(rpcReg, OpendaylightPortStatisticsService.class, this);
        register(rpcReg, OpendaylightFlowTableStatisticsService.class, this);

        LOG.debug("Node has been created: {}", nodeIdentifier);
        publish();
    }

    /**
     * Add a physical switch port to this node.
     *
     * <p>
     *   This method must be called with holding the global writer lock.
     * </p>
     *
     * @param id  The port number.
     * @return    An {@link OfPort} instance.
     */
    public OfPort addPort(long id) {
        OfPort port = new OfPort(ofVersion, nodeIdentifier, id);
        String portId = port.getPortIdentifier();
        if (physicalPorts.put(portId, port) != null) {
            String msg = "Duplicate port number: " + id;
            LOG.error(msg);
            throw new IllegalArgumentException(msg);
        }

        port.publish(ofMockProvider);
        return port;
    }

    /**
     * Remove the given physical switch port.
     *
     * <p>
     *   This method must be called with holding the global writer lock.
     * </p>
     *
     * @param pid  The port identifier.
     */
    public void removePort(String pid) {
        OfPort port = physicalPorts.remove(pid);
        if (port == null) {
            throw new IllegalArgumentException("Unknown port: " + pid);
        }

        ofMockProvider.publish(port.getNodeConnectorRemoved());
    }

    /**
     * Return an {@link OfPort} instance associated with the given port
     * identifier string.
     *
     * <p>
     *   This method must be called with holding the global lock.
     * </p>
     *
     * @param pid  The port identifier.
     * @return  A {@link OfPort} instance if found.
     *          {@code null} if not found.
     */
    public OfPort getPort(String pid) {
        return physicalPorts.get(pid);
    }

    /**
     * Return a list of physical ports.
     *
     * <p>
     *   This method must be called with holding the global lock.
     * </p>
     *
     * @param edge  Return a list of edge ports if {@code true}.
     *              Return a list of ISL ports if {@code false}.
     * @return  A list of MD-SAL node identifiers.
     */
    public List<String> getPorts(boolean edge) {
        List<String> ports = new ArrayList<>();
        for (OfPort port: physicalPorts.values()) {
            boolean e = (port.getPeerIdentifier() == null);
            if (e == edge) {
                ports.add(port.getPortIdentifier());
            }
        }

        return ports;
    }

    /**
     * Return a list of all the physical ports.
     *
     * <p>
     *   This method must be called with holding the global lock.
     * </p>
     *
     * @return  A list of {@link OfPort} instances.
     */
    public List<OfPort> getOfPorts() {
        return new ArrayList<OfPort>(physicalPorts.values());
    }

    /**
     * Return a list of physical ports.
     *
     * <p>
     *   This method must be called with holding the global lock.
     * </p>
     *
     * @param edge  Return a list of edge ports if {@code true}.
     *              Return a list of ISL ports if {@code false}.
     * @return  A list of {@link OfPort} instances.
     */
    public List<OfPort> getOfPorts(boolean edge) {
        List<OfPort> ports = new ArrayList<>();
        for (OfPort port: physicalPorts.values()) {
            boolean e = (port.getPeerIdentifier() == null);
            if (e == edge) {
                ports.add(port);
            }
        }

        return ports;
    }

    /**
     * Set the link state for all ports in this node.
     *
     * <p>
     *   This method must be called with holding the global lock.
     * </p>
     *
     * @param state     New link state.
     */
    public void setPortState(boolean state) {
        for (OfPort port: physicalPorts.values()) {
            port.setPortState(ofMockProvider, state);
        }
    }

    /**
     * Clear packet transmission queue in all ports.
     *
     * <p>
     *   This method must be called with holding the global lock.
     * </p>
     */
    public void clearTransmittedPacket() {
        for (OfPort port: physicalPorts.values()) {
            port.clearTransmittedPacket();
        }
    }

    /**
     * Return the flow entry specified by the given match and priority.
     *
     * @param table  The flow table identifier.
     * @param match  A {@link Match} instance.
     * @param pri    A priority value.
     * @return  An {@link OfMockFlowEntry} instance if found.
     *          {@code null} if not found.
     */
    public OfMockFlowEntry getFlow(int table, Match match, int pri) {
        OfMockFlowEntry target =
            new OfMockFlowEntry(nodeIdentifier, table, match, pri);
        return getFlow(target);
    }

    /**
     * Wait for the flow entry specified by the given match and priority is
     * installed.
     *
     * @param target  An {@link OfMockFlowEntry} instance which represents
     *                the target flow.
     * @return  An {@link OfMockFlowEntry} instance if found.
     *          {@code null} if not found.
     */
    public synchronized OfMockFlowEntry getFlow(OfMockFlowEntry target) {
        for (OfMockFlowEntry ofent: flowEntries) {
            if (ofent.equals(target)) {
                return ofent;
            }
        }
        return null;
    }

    /**
     * Wait for the flow entry specified by the given match and priority is
     * installed.
     *
     * @param target     An {@link OfMockFlowEntry} instance which represents
     *                   the target flow.
     * @param installed  If {@code true}, this method waits for the given
     *                   flow entry to be installed.
     *                   If {@code false}, this method waits for the given
     *                   flow entry to be uninstalled.
     * @param timeout    The number of milliseconds to wait.
     * @return  An {@link OfMockFlowEntry} instance if found.
     *          {@code null} if not found.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     * @throws TimeoutException
     *    {@code timeout} is zero or negative.
     */
    public synchronized OfMockFlowEntry awaitFlow(OfMockFlowEntry target,
                                                  boolean installed,
                                                  long timeout)
        throws InterruptedException, TimeoutException {
        OfMockFlowEntry ofent = getFlow(target);
        if ((ofent != null) == installed) {
            return ofent;
        }

        if (timeout <= 0) {
            throw new TimeoutException();
        }
        wait(timeout);

        return getFlow(target);
    }

    /**
     * Wait for the flow table to be cleared.
     *
     * @param timeout    The number of milliseconds to wait.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     * @throws TimeoutException
     *    At least one flow entry is present after the wait.
     */
    public synchronized void awaitFlowCleared(long timeout)
        throws InterruptedException, TimeoutException {
        if (flowEntries.isEmpty()) {
            return;
        }

        long tmout = OfMockProvider.TASK_TIMEOUT;
        long deadline = System.currentTimeMillis() + tmout;
        do {
            wait(tmout);
            if (flowEntries.isEmpty()) {
                return;
            }
            tmout = deadline - System.currentTimeMillis();
        } while (tmout > 0);

        StringBuilder builder = new StringBuilder("Flow table is not empty: ").
            append(flowEntries);
        throw new TimeoutException(builder.toString());
    }

    /**
     * Return the number of flow entries.
     *
     * <p>
     *   This method must be called with holding the global lock.
     * </p>
     *
     * @return  The number of flow entries.
     */
    public synchronized int getFlowCount() {
        return flowEntries.size();
    }

    /**
     * Publish a node updated notification.
     */
    public void publish() {
        ofMockProvider.publish(createNodeUpdated());
    }

    /**
     * Register the given routed RPC service.
     *
     * @param rpcReg  A {@link RpcProviderRegistry} service instance.
     * @param clz     A class corresponding to routed RPC service.
     * @param impl    This instance must be specified.
     * @param <T>     The type of routed RPC service.
     */
    private <T extends RpcService> void register(
        RpcProviderRegistry rpcReg, Class<T> clz, T impl) {
        RoutedRpcRegistration<T> reg =
            rpcReg.addRoutedRpcImplementation(clz, impl);
        reg.registerPath(NodeContext.class, nodePath);
        registrations.add(reg);
    }

    /**
     * Create a {@link NodeUpdated} notification.
     *
     * @return  A {@link NodeUpdated} instance.
     */
    private NodeUpdated createNodeUpdated() {
        NodeUpdatedBuilder builder = new NodeUpdatedBuilder();
        builder.setId(new NodeId(nodeIdentifier)).
            setNodeRef(new NodeRef(nodePath));

        SwitchFeaturesBuilder sfBuilder = new SwitchFeaturesBuilder().
            setMaxBuffers(Long.valueOf(NODE_BUFSIZE)).
            setMaxTables(Short.valueOf((short)1)).
            setCapabilities(CAPABILITIES);

        String na = "N/A";
        FlowCapableNodeUpdatedBuilder fcBuilder = new FlowCapableNodeUpdatedBuilder().
            setIpAddress(ipAddress).setSwitchFeatures(sfBuilder.build()).
            setDescription(na).setSerialNumber(na).
            setHardware("OpenFlow mock-up").
            setManufacturer("OpenDaylight").setSoftware("0.1");
        builder.addAugmentation(FlowCapableNodeUpdated.class,
                                fcBuilder.build());

        return builder.build();
    }

    /**
     * Create a {@link NodeRemoved} notification.
     *
     * @return  A {@link NodeRemoved} instance.
     */
    private NodeRemoved createNodeRemoved() {
        NodeRemovedBuilder builder = new NodeRemovedBuilder();
        return builder.setNodeRef(new NodeRef(nodePath)).build();
    }

    /**
     * Create a future that has no output.
     *
     * @return  A {@link Future} instance.
     */
    private Future<RpcResult<Void>> createRpcResult() {
        RpcResultBuilder<Void> builder = RpcResultBuilder.<Void>success();
        return Futures.immediateFuture(builder.build());
    }

    /**
     * Create a future that returns the specified output.
     *
     * @param out  An output of the RPC.
     * @param <T>  The type of RPC output.
     * @return  A {@link Future} instance.
     */
    private <T> Future<RpcResult<T>> createRpcResult(T out) {
        RpcResultBuilder<T> builder = RpcResultBuilder.success(out);
        return Futures.immediateFuture(builder.build());
    }

    /**
     * Create a future that returns an error that indicates the requested
     * feature is not supported.
     *
     * @param cls  The class which indicates the type of RPC output.
     * @param <T>  The type of RPC output.
     * @return  A {@link Future} instance.
     */
    private <T> Future<RpcResult<T>> createUnsupported(Class<T> cls) {
        RpcResultBuilder<T> builder = RpcResultBuilder.<T>failed().
            withError(ErrorType.RPC, "unsupported",
                      "The requested RPC is not supported.");
        return Futures.immediateFuture(builder.build());
    }

    /**
     * Create a future that returns an error that indicates an illegal argument
     * has been passed to RPC.
     *
     * @param cls  The class which indicates the type of RPC output.
     * @param msg  An error message.
     * @param <T>  The type of RPC output.
     * @return  A {@link Future} instance.
     */
    private <T> Future<RpcResult<T>> createIllegalArgument(Class<T> cls,
                                                           String msg) {
        RpcResultBuilder<T> builder = RpcResultBuilder.<T>failed().
            withError(ErrorType.RPC, "invalid-value", msg);
        return Futures.immediateFuture(builder.build());
    }

    /**
     * Create a error response for transmit-packet RPC request.
     *
     * @param msg  An error message.
     * @return  A {@link Future} instance.
     */
    private Future<RpcResult<Void>> createTransmitPacketError(String msg) {
        LOG.error("{}: transmitPacket: {}", nodeIdentifier, msg);
        return createIllegalArgument(Void.class, msg);
    }

    /**
     * Create a {@link TransactionId} instance which representse the OpenFlow
     * transaction ID.
     *
     * @return  A {@link TransactionId} instance.
     */
    private TransactionId createTransactionId() {
        long id = nextTransactionId.incrementAndGet();
        return new TransactionId(BigInteger.valueOf(id));
    }

    /**
     * Check whether the MD-SAL match contains another MD-SAL match.
     *
     * @param match1  The first MD-SAL match to be tested.
     * @param match2  The second MD-SAL match to be tested.
     * @return  {@code true} only if {@code match1} contains {@code match2}.
     */
    private boolean contains(Match match1, Match match2) {
        if (match1 == null) {
            return true;
        }

        Match m = (match2 == null)
            ? new MatchBuilder().build()
            : match2;

        // Currently, only IN_PORT match is supported.
        NodeConnectorId inPort1 = match1.getInPort();
        if (inPort1 != null && !inPort1.equals(m.getInPort())) {
            return false;
        }

        // Return false if unsupported filed is specified.
        return (match1.getInPhyPort() == null &&
                match1.getMetadata() == null &&
                match1.getTunnel() == null &&
                match1.getEthernetMatch() == null &&
                match1.getVlanMatch() == null &&
                match1.getIpMatch() == null &&
                match1.getLayer3Match() == null &&
                match1.getLayer4Match() == null &&
                match1.getIcmpv4Match() == null &&
                match1.getIcmpv6Match() == null &&
                match1.getProtocolMatchFields() == null &&
                match1.getTcpFlagMatch() == null);
    }

    /**
     * Check whether the given flow cookie satisfies the given condition.
     *
     * @param cookie    The flow cookie to be tested.
     * @param expected  The expected flow cookie.
     * @param mask      The flow cookie mask.
     * @return  {@code true} if the given flow cookie satisfies the condition.
     *          Otherwise {@code false}.
     */
    private boolean checkCookie(BigInteger cookie, FlowCookie expected,
                                FlowCookie mask) {
        if (ofVersion == VtnOpenflowVersion.OF10) {
            // Cookie mask is not supported.
            return true;
        }

        long lmask = OfMockUtils.getCookie(mask).longValue();
        if (lmask == 0) {
            // Cookie mask is not specified.
            return true;
        }

        long lexp = OfMockUtils.getCookie(expected).longValue();
        long lcookie = cookie.longValue();

        return ((lcookie & lmask) == (lexp & lmask));
    }

    /**
     * Scan the flow table.
     *
     * @param cond     The MD-SAL flow entry which specifies the condition to
     *                 select flow entries.
     * @param scanner  A {@link FlowTableScanner} instance.
     *                 Flow entries that satisfies the condition specified by
     *                 {@code flow} will be passed to this instance.
     */
    private synchronized void scanFlowTable(Flow cond,
                                            FlowTableScanner scanner) {
        Short table = cond.getTableId();
        Match match = cond.getMatch();
        BigInteger oport = cond.getOutPort();
        String outPort = (oport == null)
            ? null
            : OfMockUtils.getPortIdentifier(nodeIdentifier, oport);
        FlowCookie cookie = cond.getCookie();
        FlowCookie cookieMask = cond.getCookieMask();

        for (Iterator<OfMockFlowEntry> it = flowEntries.iterator();
             it.hasNext();) {
            OfMockFlowEntry ofent = it.next();
            if (table != null && table.intValue() != ofent.getTableId()) {
                continue;
            }

            if (!contains(match, ofent.getMatch())) {
                continue;
            }

            if (outPort != null &&
                !OfMockUtils.hasOutput(ofent.getInstructions(), outPort)) {
                continue;
            }

            if (!checkCookie(ofent.getCookie(), cookie, cookieMask)) {
                continue;
            }

            if (scanner.flowEntryFound(ofent)) {
                it.remove();
            }
        }
    }

    /**
     * Process a bulk flow remove request.
     *
     * @param input  An input of this RPC.
     * @return  A list of flow entries removed from the flow table.
     */
    private synchronized List<OfMockFlowEntry> removeFlows(
        RemoveFlowInput input) {
        FlowEntryRemover remover = new FlowEntryRemover();
        scanFlowTable(input, remover);

        List<OfMockFlowEntry> removed = remover.getRemovedFlows();
        if (!removed.isEmpty()) {
            notifyAll();
        }

        return removed;
    }

    /**
     * Publish a switch-flow-removed notification.
     *
     * @param ofent   A {@link OfMockFlowEntry} instance.
     * @param reason  A {@link RemovedReasonFlags} instance.
     */
    private void publishFlowRemoved(OfMockFlowEntry ofent,
                                    RemovedReasonFlags reason) {
        FlowModFlags flags = ofent.getFlowModFlags();
        if (flags == null || !Boolean.TRUE.equals(flags.isSENDFLOWREM())) {
            // FLOW_REMOVED is not requested.
            return;
        }

        org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.mod.removed.Match match =
            new org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.mod.removed.MatchBuilder(ofent.getMatch()).build();

        SwitchFlowRemovedBuilder builder = new SwitchFlowRemovedBuilder().
            setPacketCount(BigInteger.valueOf(PACKET_COUNT)).
            setByteCount(BigInteger.valueOf(BYTE_COUNT)).
            setDurationSec(Long.valueOf(DURATION_SEC)).
            setDurationNsec(Long.valueOf(DURATION_NANOSEC)).
            setPriority(Integer.valueOf(ofent.getPriority())).
            setTableId(Short.valueOf((short)ofent.getTableId())).
            setIdleTimeout(Integer.valueOf(ofent.getIdleTimeout())).
            setHardTimeout(Integer.valueOf(ofent.getHardTimeout())).
            setRemovedReason(reason).setMatch(match).
            setNode(new NodeRef(nodePath));

        BigInteger cookie = ofent.getCookie();
        if (cookie != null) {
            builder.setCookie(new FlowCookie(cookie));
        }

        ofMockProvider.publish(builder.build());
    }

    // AutoCloseable

    /**
     * Close the mock-up of OpenFlow switch.
     */
    @Override
    public void close() {
        if (!registrations.isEmpty()) {
            for (Registration reg: registrations) {
                try {
                    reg.close();
                } catch (Exception e) {
                    LOG.error("Failed to close registration: " + reg, e);
                }
            }
            registrations.clear();

            LOG.debug("Node has been removed: {}", nodeIdentifier);
            ofMockProvider.publish(createNodeRemoved());
        }
    }

    // SalFlowService

    /**
     * Add a new flow entry.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public synchronized Future<RpcResult<AddFlowOutput>> addFlow(
        AddFlowInput input) {
        try {
            OfMockFlowEntry ofent = new OfMockFlowEntry(nodeIdentifier, input);
            flowEntries.add(ofent);
        } catch (IllegalArgumentException e) {
            LOG.error("addFlow: Invalid input: " + input, e);
            return createIllegalArgument(AddFlowOutput.class, e.getMessage());
        }

        notifyAll();

        AddFlowOutput output = new AddFlowOutputBuilder().
            setTransactionId(createTransactionId()).build();
        return createRpcResult(output);
    }

    /**
     * Remove the specified flow entry.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<RemoveFlowOutput>> removeFlow(
        RemoveFlowInput input) {
        List<OfMockFlowEntry> removed;
        if (Boolean.TRUE.equals(input.isStrict())) {
            removed = new ArrayList<>();
            FlowCookie cookie = input.getCookie();
            FlowCookie cookieMask = input.getCookieMask();
            OfMockFlowEntry target;
            try {
                target = new OfMockFlowEntry(nodeIdentifier, input);
            } catch (IllegalArgumentException e) {
                LOG.error("removeFlow: Invalid input: " + input, e);
                return createIllegalArgument(RemoveFlowOutput.class,
                                             e.getMessage());
            }

            synchronized (this) {
                for (Iterator<OfMockFlowEntry> it = flowEntries.iterator();
                     it.hasNext();) {
                    OfMockFlowEntry ofent = it.next();
                    if (ofent.equals(target) &&
                        checkCookie(ofent.getCookie(), cookie, cookieMask)) {
                        it.remove();
                        removed.add(ofent);
                    }
                }

                if (!removed.isEmpty()) {
                    notifyAll();
                }
            }
        } else {
            removed = removeFlows(input);
        }

        RemovedReasonFlags reason =
            new RemovedReasonFlags(true, false, false, false);
        for (OfMockFlowEntry ofent: removed) {
            publishFlowRemoved(ofent, reason);
        }

        RemoveFlowOutput output = new RemoveFlowOutputBuilder().
            setTransactionId(createTransactionId()).build();
        return createRpcResult(output);
    }

    /**
     * Update the specified flow entry.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<UpdateFlowOutput>> updateFlow(
        UpdateFlowInput input) {
        // Do nothing.
        UpdateFlowOutput output = new UpdateFlowOutputBuilder().
            setTransactionId(createTransactionId()).build();
        return createRpcResult(output);
    }

    // SalGroupService
    /**
     * Add a flow group.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<AddGroupOutput>> addGroup(AddGroupInput input) {
        return createUnsupported(AddGroupOutput.class);
    }

    /**
     * Remove the given flow group.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<RemoveGroupOutput>> removeGroup(
        RemoveGroupInput input) {
        return createUnsupported(RemoveGroupOutput.class);
    }

    /**
     * Update the given flow group.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<UpdateGroupOutput>> updateGroup(
        UpdateGroupInput input) {
        return createUnsupported(UpdateGroupOutput.class);
    }

    // SalMeterService

    /**
     * Add a new meter.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<AddMeterOutput>> addMeter(AddMeterInput input) {
        return createUnsupported(AddMeterOutput.class);
    }

    /**
     * Remove the given meter.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<RemoveMeterOutput>> removeMeter(
        RemoveMeterInput input) {
        return createUnsupported(RemoveMeterOutput.class);
    }

    /**
     * Update the given meter.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<UpdateMeterOutput>> updateMeter(
        UpdateMeterInput input) {
        return createUnsupported(UpdateMeterOutput.class);
    }

    // SalTableService

    /**
     * Update the given flow table.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<UpdateTableOutput>> updateTable(
        UpdateTableInput input) {
        return createUnsupported(UpdateTableOutput.class);
    }

    // SalPortService

    /**
     * Update the given port.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<UpdatePortOutput>> updatePort(
        UpdatePortInput input) {
        TransactionId txid = createTransactionId();
        UpdatePortOutputBuilder builder = new UpdatePortOutputBuilder().
            setTransactionId(txid);
        return createRpcResult(builder.build());
    }

    // PacketProcessingService

    /**
     * Transmit the given packet.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<Void>> transmitPacket(TransmitPacketInput input) {
        if (input == null) {
            return createTransmitPacketError("Input cannot be null.");
        }

        NodeConnectorRef egress = input.getEgress();
        String pid = OfMockUtils.getPortIdentifier(egress);
        if (pid == null) {
            return createTransmitPacketError("Egress port cannot be null.");
        }

        byte[] payload = input.getPayload();
        if (payload == null || payload.length == 0) {
            return createTransmitPacketError("Payload cannot be empty.");
        }

        InstanceIdentifier<?> path = egress.getValue();
        Lock lock = ofMockProvider.acquire(false);
        try {
            OfPort port = physicalPorts.get(pid);
            if (port == null || !path.equals(port.getPortPath())) {
                String msg = "Egress port not found: " + egress;
                return createTransmitPacketError(msg);
            }

            port.transmit(payload);
        } finally {
            lock.unlock();
        }

        return createRpcResult();
    }

    // NodeConfigService

    /**
     * Set the given configuration to this node.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<SetConfigOutput>> setConfig(SetConfigInput input) {
        TransactionId txid = new TransactionId(BigInteger.valueOf(1L));
        SetConfigOutputBuilder builder = new SetConfigOutputBuilder().
            setTransactionId(txid);
        return createRpcResult(builder.build());
    }

    // OpendaylightGroupStatisticsService

    /**
     * Return statistics information about all groups.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<GetAllGroupStatisticsOutput>> getAllGroupStatistics(
        GetAllGroupStatisticsInput input) {
        return createUnsupported(GetAllGroupStatisticsOutput.class);
    }

    /**
     * Return the description about the given group.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<GetGroupDescriptionOutput>> getGroupDescription(
        GetGroupDescriptionInput input) {
        return createUnsupported(GetGroupDescriptionOutput.class);
    }

    /**
     * Return features of the given group.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<GetGroupFeaturesOutput>> getGroupFeatures(
        GetGroupFeaturesInput input) {
        return createUnsupported(GetGroupFeaturesOutput.class);
    }

    /**
     * Return statistics information about the given group.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<GetGroupStatisticsOutput>> getGroupStatistics(
        GetGroupStatisticsInput input) {
        return createUnsupported(GetGroupStatisticsOutput.class);
    }

    // OpendaylightMeterStatisticsService

    /**
     * Return statistics information about all meter configurations.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<GetAllMeterConfigStatisticsOutput>> getAllMeterConfigStatistics(
        GetAllMeterConfigStatisticsInput input) {
        return createUnsupported(GetAllMeterConfigStatisticsOutput.class);
    }

    /**
     * Return statistics information about all meters.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<GetAllMeterStatisticsOutput>> getAllMeterStatistics(
        GetAllMeterStatisticsInput input) {
        return createUnsupported(GetAllMeterStatisticsOutput.class);
    }

    /**
     * Return features of the given meter.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<GetMeterFeaturesOutput>> getMeterFeatures(
        GetMeterFeaturesInput input) {
        return createUnsupported(GetMeterFeaturesOutput.class);
    }

    /**
     * Return statistics information about the given meter.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<GetMeterStatisticsOutput>> getMeterStatistics(
        GetMeterStatisticsInput input) {
        return createUnsupported(GetMeterStatisticsOutput.class);
    }

    // OpendaylightFlowStatisticsService

    /**
     * Return aggregate statistics for all flow entries.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<GetAggregateFlowStatisticsFromFlowTableForAllFlowsOutput>> getAggregateFlowStatisticsFromFlowTableForAllFlows(
        GetAggregateFlowStatisticsFromFlowTableForAllFlowsInput input) {
        return createUnsupported(
            GetAggregateFlowStatisticsFromFlowTableForAllFlowsOutput.class);
    }

    /**
     * Return aggregate statistics for flow entries that match to the given
     * match.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<GetAggregateFlowStatisticsFromFlowTableForGivenMatchOutput>> getAggregateFlowStatisticsFromFlowTableForGivenMatch(
        GetAggregateFlowStatisticsFromFlowTableForGivenMatchInput input) {
        return createUnsupported(
            GetAggregateFlowStatisticsFromFlowTableForGivenMatchOutput.class);
    }

    /**
     * Return statistics information about all flow entries in the given table.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<GetAllFlowStatisticsFromFlowTableOutput>> getAllFlowStatisticsFromFlowTable(
        GetAllFlowStatisticsFromFlowTableInput input) {
        return createUnsupported(
            GetAllFlowStatisticsFromFlowTableOutput.class);
    }

    /**
     * Return statistics information about all flow entries in all flow tables.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<GetAllFlowsStatisticsFromAllFlowTablesOutput>> getAllFlowsStatisticsFromAllFlowTables(
        GetAllFlowsStatisticsFromAllFlowTablesInput input) {
        return createUnsupported(
            GetAllFlowsStatisticsFromAllFlowTablesOutput.class);
    }

    /**
     * Return statistics information about the given flow entry in the flow
     * table.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<GetFlowStatisticsFromFlowTableOutput>> getFlowStatisticsFromFlowTable(
        GetFlowStatisticsFromFlowTableInput input) {
        // Start flow statistics read transaction.
        TransactionId xid = createTransactionId();
        FlowStatsThread t = new FlowStatsThread(xid, input);
        t.start();

        // Return the transaction ID only.
        GetFlowStatisticsFromFlowTableOutputBuilder builder =
            new GetFlowStatisticsFromFlowTableOutputBuilder();
        builder.setTransactionId(xid);
        return createRpcResult(builder.build());
    }

    // OpendaylightPortStatisticsService

    /**
     * Return statistics information about all node connectors in the given
     * node.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<GetAllNodeConnectorsStatisticsOutput>> getAllNodeConnectorsStatistics(
        GetAllNodeConnectorsStatisticsInput input) {
        return createUnsupported(GetAllNodeConnectorsStatisticsOutput.class);
    }

    /**
     * Return statistics information abotu the given node connector.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<GetNodeConnectorStatisticsOutput>> getNodeConnectorStatistics(
        GetNodeConnectorStatisticsInput input) {
        return createUnsupported(GetNodeConnectorStatisticsOutput.class);
    }

    // OpendaylightFlowTableStatisticsService

    /**
     * Return statistics information about all flow tables in the given node.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<GetFlowTablesStatisticsOutput>> getFlowTablesStatistics(
        GetFlowTablesStatisticsInput input) {
        return createUnsupported(GetFlowTablesStatisticsOutput.class);
    }

    // OpendaylightQueueStatisticsService

    /**
     * Return statistics information abou all queues attached to all ports
     * in the given node.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<GetAllQueuesStatisticsFromAllPortsOutput>> getAllQueuesStatisticsFromAllPorts(
        GetAllQueuesStatisticsFromAllPortsInput input) {
        return createUnsupported(
            GetAllQueuesStatisticsFromAllPortsOutput.class);
    }

    /**
     * Return statistics information about all queues for the given port.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<GetAllQueuesStatisticsFromGivenPortOutput>> getAllQueuesStatisticsFromGivenPort(
        GetAllQueuesStatisticsFromGivenPortInput input) {
        return createUnsupported(
            GetAllQueuesStatisticsFromGivenPortOutput.class);
    }

    /**
     * Return statistics information about the given queue.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<GetQueueStatisticsFromGivenPortOutput>> getQueueStatisticsFromGivenPort(
        GetQueueStatisticsFromGivenPortInput input) {
        return createUnsupported(
            GetQueueStatisticsFromGivenPortOutput.class);
    }
}
