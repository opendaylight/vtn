/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock.impl;

import static org.opendaylight.vtn.manager.it.ofmock.OfMockService.ID_OPENFLOW;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.Executor;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeoutException;
import java.util.concurrent.atomic.AtomicLong;
import java.util.concurrent.locks.Lock;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.base.Preconditions;
import com.google.common.util.concurrent.FutureCallback;
import com.google.common.util.concurrent.Futures;
import com.google.common.util.concurrent.ListenableFutureTask;

import org.opendaylight.vtn.manager.it.ofmock.OfMockUtils;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.sal.binding.api.BindingAwareBroker.RoutedRpcRegistration;
import org.opendaylight.controller.sal.binding.api.RpcProviderRegistry;

import org.opendaylight.yangtools.concepts.Registration;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.yangtools.yang.binding.RpcService;
import org.opendaylight.yangtools.yang.common.RpcError.ErrorType;
import org.opendaylight.yangtools.yang.common.RpcResult;
import org.opendaylight.yangtools.yang.common.RpcResultBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnOpenflowVersion;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FeatureCapability;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNodeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowFeatureCapabilityFlowStats;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.flow.node.SwitchFeaturesBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.Table;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.TableBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.AddFlowInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.AddFlowOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;
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
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.transaction.rev150304.FlowCapableTransactionService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.transaction.rev150304.SendBarrierInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.transaction.rev150304.TransactionId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.Flow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.Match;
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
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorRef;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeContext;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeRef;
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
               NodeConfigService, FlowCapableTransactionService,
               OpendaylightGroupStatisticsService,
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
     * The flow table ID supported by the ofmock service.
     */
    public static final short  TABLE_ID = 0;

    /**
     * The buffer size to be notified.
     */
    private static final long  NODE_BUFSIZE = 256L;

    /**
     * The number of packets to be set in flow statistics.
     */
    public static final long  PACKET_COUNT = 5555555L;

    /**
     * The number of bytes to be set in flow statistics.
     */
    public static final long  BYTE_COUNT = 1234567890L;

    /**
     * Duration (in seconds) of a flow entry to be set in flow statistics.
     */
    public static final long  DURATION_SEC = 12345L;

    /**
     * Duration (in nanoseconds) of a flow entry to be set in flow statistics.
     */
    public static final long  DURATION_NANOSEC = 666777888L;

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
     * The identifier of the next transaction.
     */
    private final AtomicLong  nextTransactionId = new AtomicLong();

    /**
     * The executor service that updates inventory information.
     */
    private final ExecutorService  inventoryExecutor;

    /**
     * The flow table.
     */
    private final FlowTable  flowTable;

    /**
     * Initialize static configurations.
     */
    static {
        List<Class<? extends FeatureCapability>> cap = new ArrayList<>();
        cap.add(FlowFeatureCapabilityFlowStats.class);
        CAPABILITIES = Collections.unmodifiableList(cap);
    }

    /**
     * Create a dummy flow statistics for the specified flow entry.
     *
     * @param flow  The MD-SAL flow entry.
     * @return  A dummy flow statistics information.
     */
    public static FlowAndStatisticsMapList getStatistics(Flow flow) {
        Duration duration = new DurationBuilder().
            setSecond(new Counter32(DURATION_SEC)).
            setNanosecond(new Counter32(DURATION_NANOSEC)).
            build();

        return new FlowAndStatisticsMapListBuilder(flow).
            setPacketCount(new Counter64(BigInteger.valueOf(PACKET_COUNT))).
            setByteCount(new Counter64(BigInteger.valueOf(BYTE_COUNT))).
            setDuration(duration).
            build();
    }

    /**
     * Construct a new instance.
     *
     * @param provider  The ofmock provider service.
     * @param ver       OpenFlow protocol version.
     * @param prefix    Protocol prefix of the node.
     * @param dpid      Datapath ID of the node.
     */
    public OfNode(OfMockProvider provider, VtnOpenflowVersion ver,
                  String prefix, BigInteger dpid) {
        ofMockProvider = provider;

        // Don't set OpenFlow version if an invalid prefix is specified.
        ofVersion = (ID_OPENFLOW.equals(prefix))
            ? Preconditions.checkNotNull(ver)
            : null;

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
        inventoryExecutor = Executors.newSingleThreadExecutor();
        flowTable = new FlowTable(this, TABLE_ID);
    }

    /**
     * Return the ofmock provider service.
     *
     * @return  The ofmock provider service.
     */
    public OfMockProvider getOfMockProvider() {
        return ofMockProvider;
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
     * Return the path to this node.
     *
     * @return  The path to this node.
     */
    public InstanceIdentifier<Node> getNodePath() {
        return nodePath;
    }

    /**
     * Return the executor that updates inventory information.
     *
     * @return  An {@link Executor} instance.
     */
    public Executor getExecutor() {
        return inventoryExecutor;
    }

    /**
     * Return a reference to this node.
     *
     * @return  A {@link NodeRef} instance.
     */
    public NodeRef getNodeRef() {
        return new NodeRef(nodePath);
    }

    /**
     * Return the OpenFlow version.
     *
     * @return  A {@link VtnOpenflowVersion} instance if this instance
     *          represents an OpenFlow switch. Otherwise {@code null}.
     */
    public VtnOpenflowVersion getOfVersion() {
        return ofVersion;
    }

    /**
     * Return the flow table.
     *
     * @return  A {@link FlowTable} instance.
     */
    public FlowTable getFlowTable() {
        return flowTable;
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
        register(rpcReg, NodeConfigService.class, this);
        register(rpcReg, FlowCapableTransactionService.class, this);
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
        final OfPort port = new OfPort(ofVersion, nodeIdentifier, id);
        final String portId = port.getPortIdentifier();
        if (physicalPorts.put(portId, port) != null) {
            String msg = "Duplicate port number: " + id;
            LOG.error(msg);
            throw new IllegalArgumentException(msg);
        }

        Futures.addCallback(port.publish(this), new FutureCallback<Void>() {
            @Override
            public void onSuccess(Void result) {
                TopologyUtils.addTerminationPoint(ofMockProvider, port);
            }

            @Override
            public void onFailure(Throwable cause) {
            }
        });
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

        port.publishRemoval(this);
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
            port.setPortState(this, state);
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
     * Return the flow entry specified by the given ofmock flow entry.
     *
     * @param target  An {@link OfMockFlowEntry} instance that specifies the
     *                target flow.
     * @return  An {@link OfMockFlowEntry} instance if found.
     *          {@code null} if not found.
     */
    public OfMockFlowEntry getFlow(OfMockFlowEntry target) {
        return flowTable.getFlow(target);
    }

    /**
     * Wait for the specified flow entry to be installed or uninstalled.
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
     *    The specified condition was not satisfied within the timeout.
     */
    public OfMockFlowEntry awaitFlow(OfMockFlowEntry target, boolean installed,
                                     long timeout)
        throws InterruptedException, TimeoutException {
        OfMockFlowEntry result;
        if (installed) {
            result = flowTable.awaitAdded(target, timeout);
        } else {
            flowTable.awaitRemoved(target, timeout);
            result = null;
        }

        return result;
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
    public void awaitFlowCleared(long timeout)
        throws InterruptedException, TimeoutException {
        flowTable.awaitCleared(timeout);
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
    public int getFlowCount() {
        return flowTable.size();
    }

    /**
     * Create a {@link TransactionId} instance which representse the OpenFlow
     * transaction ID.
     *
     * @return  A {@link TransactionId} instance.
     */
    public TransactionId createTransactionId() {
        long id = nextTransactionId.incrementAndGet();
        return new TransactionId(BigInteger.valueOf(id));
    }

    /**
     * Publish a flows-statistics-update notification.
     *
     * @param xid    The transaction ID associated with the flow statistics
     *               request.
     * @param stats  A list of flow statistics.
     */
    public void publishFlowStatsUpdate(TransactionId xid,
                                       List<FlowAndStatisticsMapList> stats) {
        FlowsStatisticsUpdateBuilder builder =
            new FlowsStatisticsUpdateBuilder().
            setId(new NodeId(nodeIdentifier)).
            setMoreReplies(false).
            setTransactionId(xid).
            setFlowAndStatisticsMapList(stats);

        ofMockProvider.publish(builder.build());
    }

    /**
     * Return the flow table ID in the given flow entry.
     *
     * @param flow  The MD-SAL flow entry.
     * @return  The flow table ID in the given flow entry.
     * @throws IllegalArgumentException
     *    The flow table ID in the given flow entry is invalid.
     */
    public Short getTableId(Flow flow) {
        Short table = flow.getTableId();
        if (table == null) {
            throw new IllegalArgumentException("table-id cannot be null.");
        }
        if (table.shortValue() != TABLE_ID) {
            throw new IllegalArgumentException(
                "Unsupported table-id: " + table);
        }
        return table;
    }

    /**
     * Publish a node updated notification.
     */
    private void publish() {
        InstanceIdentifier<FlowCapableNode> path = nodePath.
            augmentation(FlowCapableNode.class);
        FlowCapableNode fcn = createFlowNode();
        UpdateDataTask<FlowCapableNode> task =
            new UpdateDataTask<>(ofMockProvider.getDataBroker(), path, fcn);
        inventoryExecutor.execute(task);

        Futures.addCallback(task.getFuture(), new FutureCallback<Void>() {
            @Override
            public void onSuccess(Void result) {
                TopologyUtils.addTopologyNode(ofMockProvider, OfNode.this);
            }

            @Override
            public void onFailure(Throwable cause) {
            }
        });
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
     * Create a new flow-capable-node.
     *
     * @return  A {@link FlowCapableNode} instance.
     */
    private FlowCapableNode createFlowNode() {
        SwitchFeaturesBuilder sfBuilder = new SwitchFeaturesBuilder().
            setMaxBuffers(Long.valueOf(NODE_BUFSIZE)).
            setMaxTables(Short.valueOf((short)1)).
            setCapabilities(CAPABILITIES);

        // Create table 0.
        Table table0 = new TableBuilder().setId(TABLE_ID).build();
        List<Table> tables = Collections.singletonList(table0);

        String na = "N/A";
        return new FlowCapableNodeBuilder().
            setIpAddress(ipAddress).setSwitchFeatures(sfBuilder.build()).
            setDescription(na).setSerialNumber(na).
            setHardware("OpenFlow mock-up").
            setManufacturer("OpenDaylight").setSoftware("0.1").
            setTable(tables).
            build();
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
     * Create a future that returns an error that indicates an unexpected
     * exception was caught.
     *
     * @param cls    The class that specifies the type of RPC output.
     * @param cause  A throwable that indicates the cause of error.
     * @param <T>    The type of RPC output.
     * @return  A {@link Future} instance.
     */
    private <T> Future<RpcResult<T>> createRpcError(Class<T> cls,
                                                    Throwable cause) {
        RpcResultBuilder<T> builder = RpcResultBuilder.<T>failed().
            withError(ErrorType.RPC, "operation-failed",
                      "Caught an unexpected exception", null, null, cause);
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
            withError(ErrorType.RPC, "operation-not-supported",
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
        }

        if (!inventoryExecutor.isShutdown()) {
            DataBroker broker = ofMockProvider.getDataBroker();
            DeleteTopologyNodeTask topoTask = new DeleteTopologyNodeTask(
                broker, nodeIdentifier);
            ofMockProvider.getTopologyExecutor().execute(topoTask);

            DeleteDataTask<Node> task = new DeleteDataTask<>(broker, nodePath);
            inventoryExecutor.execute(task);
            OfMockProvider.shutdown(inventoryExecutor);
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
    public Future<RpcResult<AddFlowOutput>> addFlow(AddFlowInput input) {
        OfMockFlowEntry ofent = null;
        try {
            // Associate a new flow ID with the given flow entry.
            ofent = flowTable.newFlowId(nodeIdentifier, input);
            AddFlowTask task = new AddFlowTask(this, input, ofent);
            inventoryExecutor.execute(task);
            ofent = null;
            return task.getFuture();
        } catch (IllegalArgumentException e) {
            LOG.error("Attempting to add invalid flow entry: " + input, e);
            return createIllegalArgument(AddFlowOutput.class, e.getMessage());
        } catch (RuntimeException e) {
            LOG.error("Failed to start add-flow task: input=" + input, e);
            return createRpcError(AddFlowOutput.class, e);
        } finally {
            if (ofent != null) {
                flowTable.abort(ofent);
            }
        }
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
        try {
            RemoveFlowTask task = RemoveFlowTask.create(this, input);
            Future<RpcResult<RemoveFlowOutput>> future = task.getFuture();
            if (!future.isDone()) {
                inventoryExecutor.execute(task);
            }
            return future;
        } catch (IllegalArgumentException e) {
            LOG.error("Attempting to remove invalid flow entry: " + input, e);
            return createIllegalArgument(RemoveFlowOutput.class,
                                         e.getMessage());
        } catch (RuntimeException e) {
            LOG.error("Failed to start remove-flow task: input=" + input, e);
            return createIllegalArgument(RemoveFlowOutput.class,
                                         e.getMessage());
        }
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

    // FlowCapableTransactionService

    /**
     * Send a BARRIER_REQUEST message to this node.
     *
     * @param input  An input of this RPC.
     * @return  A {@link Future} instance associated with the RPC task.
     */
    @Override
    public Future<RpcResult<Void>> sendBarrier(SendBarrierInput input) {
        if (ofVersion == null) {
            return createUnsupported(Void.class);
        }

        try {
            ListenableFutureTask<RpcResult<Void>> task = Barrier.create();
            inventoryExecutor.execute(task);
            return task;
        } catch (RuntimeException e) {
            LOG.error("Failed to start send-barrier task: input=" + input, e);
            return createRpcError(Void.class, e);
        }
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
        Short table = input.getTableId();
        if (table == null) {
            return createIllegalArgument(
                GetFlowStatisticsFromFlowTableOutput.class,
                "Table ID cannot be null.");
        } else if (!table.equals(flowTable.getTableId())) {
            return createIllegalArgument(
                GetFlowStatisticsFromFlowTableOutput.class,
                "Invalid table ID: " + table);
        }

        // Start flow statistics read transaction.
        TransactionId xid = createTransactionId();
        FlowMatcher matcher = new FlowMatcher(this, input);
        FlowStatsNotifier notifier = new FlowStatsNotifier(this, matcher, xid);
        inventoryExecutor.execute(notifier);

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
