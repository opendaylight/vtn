/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.adsal;

import static org.opendaylight.vtn.manager.internal.util.vnode.VTNVlanMapConfig.NODE_ID_ANY;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.base.Optional;

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
import org.opendaylight.vtn.manager.SwitchPort;
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
import org.opendaylight.vtn.manager.VTerminalIfPath;
import org.opendaylight.vtn.manager.VTerminalPath;
import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.VlanMapConfig;
import org.opendaylight.vtn.manager.flow.DataFlow;
import org.opendaylight.vtn.manager.flow.DataFlowFilter;
import org.opendaylight.vtn.manager.flow.cond.FlowCondition;
import org.opendaylight.vtn.manager.flow.cond.FlowMatch;
import org.opendaylight.vtn.manager.flow.filter.FlowFilter;
import org.opendaylight.vtn.manager.flow.filter.FlowFilterId;
import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.NumberUtils;
import org.opendaylight.vtn.manager.util.VTNIdentifiableComparator;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.concurrent.AbstractVTNFuture;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;
import org.opendaylight.vtn.manager.internal.util.flow.cond.FlowCondUtils;
import org.opendaylight.vtn.manager.internal.util.flow.cond.VTNFlowCondition;
import org.opendaylight.vtn.manager.internal.util.flow.cond.VTNFlowMatch;
import org.opendaylight.vtn.manager.internal.util.flow.filter.VTNFlowFilter;
import org.opendaylight.vtn.manager.internal.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.NodeUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.pathmap.PathMapUtils;
import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathCostConfigBuilder;
import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyConfigBuilder;
import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIfIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeType;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantUtils;
import org.opendaylight.vtn.manager.internal.util.vnode.VTerminalIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTerminalIfIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VlanMapIdentifier;
import org.opendaylight.vtn.manager.internal.vnode.VBridgeManager;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector.NodeConnectorIDType;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.packet.address.DataLinkAddress;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.yangtools.yang.common.RpcError;
import org.opendaylight.yangtools.yang.common.RpcResult;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.ClearFlowConditionOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.RemoveFlowConditionInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.RemoveFlowConditionInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.RemoveFlowConditionMatchInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.RemoveFlowConditionMatchInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.RemoveFlowConditionMatchOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionMatchInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionMatchInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionMatchOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnFlowConditionService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.remove.flow.condition.match.output.RemoveMatchResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.set.flow.condition.match.input.FlowMatchList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.set.flow.condition.match.output.SetMatchResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.RemoveFlowFilterInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.RemoveFlowFilterInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.RemoveFlowFilterOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.SetFlowFilterInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.SetFlowFilterOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.VtnFlowFilterList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.VtnFlowFilterService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilterKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.result.FlowFilterResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.DataFlowMode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.GetDataFlowCountInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.GetDataFlowCountInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.GetDataFlowCountOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.GetDataFlowInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.GetDataFlowInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.GetDataFlowOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VtnFlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VtnFlowService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.get.data.flow.input.DataFlowPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.get.data.flow.input.DataFlowPortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.get.data.flow.output.DataFlowInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.GetMacMappedHostInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.GetMacMappedHostInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.GetMacMappedHostOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.SetMacMapAclOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.SetMacMapAclInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.SetMacMapAclInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.SetMacMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.SetMacMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.SetMacMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.VtnMacMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.VtnMacMapInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.VtnMacMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.get.mac.mapped.host.output.MacMappedHost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.info.MacMapStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.status.MappedHost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.RemovePortMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.RemovePortMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.SetPortMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.SetPortMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.VtnPortMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.VtnPortMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.AddVlanMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.AddVlanMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.AddVlanMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.GetVlanMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.GetVlanMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.GetVlanMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.RemoveVlanMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.RemoveVlanMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.RemoveVlanMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.VtnVlanMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.VtnVlanMapInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.VtnVlanMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.remove.vlan.map.output.RemoveVlanMapResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.ClearPathMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.ClearPathMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.ClearPathMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.RemovePathMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.RemovePathMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.RemovePathMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.SetPathMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.SetPathMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.SetPathMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.VtnPathMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.remove.path.map.output.RemovePathMapResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.set.path.map.input.PathMapList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.set.path.map.output.SetPathMapResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.ClearPathPolicyOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathCostInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathCostInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathCostOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathPolicyInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathPolicyInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathCostInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathCostInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathCostOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathPolicyInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathPolicyOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicyService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.remove.path.cost.output.RemovePathCostResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.set.path.cost.input.PathCostList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.set.path.cost.output.SetPathCostResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicy;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.RemoveVtnInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.RemoveVtnInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.UpdateVtnInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.UpdateVtnInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.VtnService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.Vtns;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtenantConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtnInputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VlanHostDesc;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VlanHostDescSet;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodePathFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeUpdateMode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnAclType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.vlan.host.desc.set.VlanHostDescList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.RemoveMacEntryInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.RemoveMacEntryInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.RemoveMacEntryOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.VtnMacTableService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.remove.mac.entry.output.RemoveMacEntryResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.RemoveVbridgeInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.RemoveVbridgeInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.UpdateVbridgeInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.UpdateVbridgeInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnPortMappableBridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnVbridgeService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.port.mappable.bridge.BridgeStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeInputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeOutputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.RemoveVinterfaceInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.RemoveVinterfaceInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.UpdateVinterfaceInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.UpdateVinterfaceInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.VtnVinterfaceService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.VinterfaceStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceInputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceOutputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.RemoveVterminalInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.RemoveVterminalInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.UpdateVterminalInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.UpdateVterminalInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.VtnVterminalService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.info.VterminalConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;
import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;

/**
 * Implementation of VTN Manager service.
 */
public class VTNManagerImpl implements IVTNManager {
    /**
     * Logger instance.
     */
    static final Logger  LOG = LoggerFactory.getLogger(VTNManagerImpl.class);

    /**
     * The number of seconds to wait for completion of RPC.
     */
    private static final long  RPC_TIMEOUT = 60L;

    /**
     * MD-SAL VTN Manager service provider.
     */
    private VTNManagerProvider  vtnProvider;

    /**
     * Function called by the dependency manager after the VTN Manager service
     * has been registered to the OSGi service repository.
     */
    void started() {
        LOG.info(" VTN Manager has been started");
    }

    /**
     * Function called just before the dependency manager stops the service.
     */
    void stopping() {
        LOG.trace("stopping() called");
    }

    /**
     * Function called by the dependency manager before the services exported
     * by the component are unregistered, this will be followed by a
     * "destroy()" calls.
     */
    void stop() {
        LOG.trace("stop() called");
    }

    /**
     * Function called by the dependency manager when at least one dependency
     * become unsatisfied or when the component is shutting down because for
     * example bundle is being stopped.
     */
    void destroy() {
        LOG.info("VTN Manager has been destroyed");
    }

    /**
     * Invoked when a VTN Manager provider is registered.
     *
     * @param provider  VTN Manager provider service.
     */
    void setVTNProvider(VTNManagerProvider provider) {
        LOG.trace("Set VTN Manager provider: {}", provider);
        vtnProvider = provider;
    }

    /**
     * Invoked when a VTN Manager provider is unregistered.
     *
     * @param provider  VTN Manager provider service.
     */
    void unsetVTNProvider(VTNManagerProvider provider) {
        if (provider != null && provider.equals(vtnProvider)) {
            LOG.trace("Unset VTN Manager provider: {}", provider);
            vtnProvider = null;
        }
    }

    /**
     * Return VTN Manager provider.
     *
     * @return  VTN Manager provider instance.
     */
    public VTNManagerProvider getVTNProvider() {
        return vtnProvider;
    }

    /**
     * Ensure that the given tenant configuration is not null.
     *
     * @param tconf  Tenant configuration
     * @throws RpcException  {@code tconf} is {@code null}.
     */
    private void checkTenantConfig(VTenantConfig tconf) throws RpcException {
        if (tconf == null) {
            throw RpcException.getNullArgumentException(
                "Tenant configuration");
        }
    }

    /**
     * Ensure that the given vBridge configuration is not null.
     *
     * @param bconf  vBridge configuration
     * @throws RpcException  {@code bconf} is {@code null}.
     */
    private void checkBridgeConfig(VBridgeConfig bconf) throws RpcException {
        if (bconf == null) {
            throw RpcException.getNullArgumentException(
                "Bridge configuration");
        }
    }

    /**
     * Ensure that the given vTerminal configuration is not null.
     *
     * @param vtconf  vTerminal configuration
     * @throws RpcException  {@code vtconf} is {@code null}.
     */
    private void checkTerminalConfig(VTerminalConfig vtconf)
        throws RpcException {
        if (vtconf == null) {
            throw RpcException.getNullArgumentException(
                "Terminal configuration");
        }
    }

    /**
     * Ensure that the given virtual interface configuration is not null.
     *
     * @param iconf  Virtual interface configuration
     * @throws RpcException  {@code vtconf} is {@code null}.
     */
    private void checkInterfaceConfig(VInterfaceConfig iconf)
        throws RpcException {
        if (iconf == null) {
            throw RpcException.getNullArgumentException(
                "Interface configuration");
        }
    }

    /**
     * Ensure that the given VLAN mapping configuration is not null.
     *
     * @param vlconf  VLAN mapping configuration
     * @throws RpcException  {@code vlconf} is {@code null}.
     */
    private void checkVlanMapConfig(VlanMapConfig vlconf)
        throws RpcException {
        if (vlconf == null) {
            throw RpcException.getNullArgumentException(
                "VLAN mapping configuration");
        }
    }

    /**
     * Check whether the VTN Manager service is available or not.
     *
     * @return  VTN Manager provider service.
     * @throws VTNException   VTN Manager service is not available.
     */
    private VTNManagerProvider checkService() throws VTNException {
        VTNManagerProvider provider = vtnProvider;
        if (provider == null) {
            throw new VTNException(VtnErrorTag.NOSERVICE,
                                   "VTN service is not available");
        }

        return provider;
    }

    /**
     * Return an exception that indicates the specified VLAN mapping is not
     * present.
     *
     * @param info  Additional information about error.
     * @return  An {@link RpcException} instance.
     */
    private RpcException getVlanMapNotFoundException(String info) {
        return RpcException.getNotFoundException(
            "VLAN mapping does not exist: " + info);
    }

    /**
     * Wait for completion of the RPC task associated with the given future.
     *
     * @param f    A {@link Future} instance associated with the RPC task.
     * @param <T>  The type of the RPC output.
     * @return  The output of the RPC task.
     * @throws VTNException  An error occurred.
     */
    private <T> T getRpcOutput(Future<RpcResult<T>> f) throws VTNException {
        return getRpcOutput(f, false);
    }

    /**
     * Wait for completion of the RPC task associated with the given future.
     *
     * @param f         A {@link Future} instance associated with the RPC task.
     * @param nillable  Set {@code true} if the result can be null.
     * @param <T>  The type of the RPC output.
     * @return  The output of the RPC task.
     * @throws VTNException  An error occurred.
     */
    private <T> T getRpcOutput(Future<RpcResult<T>> f, boolean nillable)
        throws VTNException {
        RpcResult<T> result;
        try {
            result = f.get(RPC_TIMEOUT, TimeUnit.SECONDS);
        } catch (Exception e) {
            throw AbstractVTNFuture.getException(e);
        }

        if (result == null) {
            // This should never happen.
            throw new VTNException("RPC did not set result.");
        }

        if (result.isSuccessful()) {
            T res = result.getResult();
            if (!nillable && res == null) {
                // This should never happen.
                throw new VTNException("RPC did not set output.");
            }

            return res;
        }

        Collection<RpcError> errors = result.getErrors();
        if (errors == null || errors.isEmpty()) {
            // This should never happen.
            String msg = "RPC failed without error information: " + result;
            throw new VTNException(msg);
        }

        // VtnErrorTag should be encoded in application tag.
        RpcError rerr = errors.iterator().next();
        String msg = rerr.getMessage();
        String appTag = rerr.getApplicationTag();
        VtnErrorTag etag;
        try {
            etag = VtnErrorTag.valueOf(appTag);
        } catch (Exception e) {
            LOG.trace("Unknown application error tag in RpcError.", e);

            Throwable cause = rerr.getCause();
            String m = "RPC failed due to unexpected error: type=";
            StringBuilder builder = new StringBuilder(m).
                append(rerr.getErrorType()).
                append(", severity=").append(rerr.getSeverity()).
                append(", msg=").append(msg).
                append(", tag=").append(rerr.getTag()).
                append(", appTag=").append(appTag).
                append(", info=").append(rerr.getInfo());

            String emsg = builder.toString();
            LOG.error(emsg, cause);
            throw new VTNException(emsg, cause);
        }

        throw new VTNException(etag, msg);
    }

    /**
     * Return the element at the given index in the given list.
     *
     * <p>
     *   This method is used to get data fro the list generated by the RPC.
     * </p>
     *
     * @param list      A list generated by the RPC.
     * @param index     Index to the target element.
     * @param nillable  Set {@code true} if the result can be null.
     * @param <T>       The type of elements in the list.
     * @return  An element at the specified index in the list.
     * @throws VTNException  An error occurred.
     */
    private <T> T getRpcOutput(List<T> list, int index, boolean nillable)
        throws VTNException {
        if (list == null) {
            throw new VTNException("RPC did not set result.");
        }

        if (index >= list.size()) {
            throw new VTNException("Unexpected number of RPC results: " +
                                   list.size());
        }

        T ret = list.get(index);
        if (!nillable && ret == null) {
            String msg = String.format("RPC set null into result[%d]", index);
            throw new VTNException(msg);
        }

        return ret;
    }

    /**
     * Set the given data flow filter into the get-data-flow RPC input.
     *
     * @param builder  A input builder for get-data-flow RPC.
     * @param filter   A {@link DataFlowFilter} instance.
     * @return  {@code true} on success.
     *          {@code false} if the given data flow filter contains invalid
     *          value.
     */
    private boolean setInput(GetDataFlowInputBuilder builder,
                             DataFlowFilter filter) {
        // Set condition for physical switch.
        Node node = filter.getNode();
        if (node != null) {
            SalNode snode = SalNode.create(node);
            if (snode == null) {
                // Unsupported node is specified.
                return false;
            }

            builder.setNode(snode.getNodeId());

            SwitchPort swport = filter.getSwitchPort();
            if (swport != null) {
                String type = swport.getType();
                if (type != null &&
                    !NodeConnectorIDType.OPENFLOW.equals(type)) {
                    // Unsupported node-connector type.
                    return false;
                }

                try {
                    DataFlowPort dfp = new DataFlowPortBuilder().
                        setPortId(swport.getId()).
                        setPortName(swport.getName()).
                        build();
                    builder.setDataFlowPort(dfp);
                } catch (IllegalArgumentException e) {
                    LOG.trace("Invalid port name is specified: " + swport, e);
                    return false;
                }
            }
        }

        // Set condition for source L2 host.
        try {
            builder.setDataFlowSource(
                FlowUtils.toDataFlowSource(filter.getSourceHost()));
        } catch (RpcException e) {
            // The source host is specified by unsupported address type,
            // or an invalid VLAN ID is specified.
            LOG.trace("Unsupported source host: " + filter.getSourceHost(), e);
            return false;
        }

        return true;
    }

    /**
     * Invoke get-data-flow RPC.
     *
     * @param provider  VTN Manager provider service.
     * @param input     The input of get-data-flow RPC.
     * @return  A list of {@link DataFlowInfo} instances.
     * @throws VTNException  An error occurred.
     */
    private List<DataFlowInfo> getDataFlows(VTNManagerProvider provider,
                                            GetDataFlowInput input)
        throws VTNException {
        // Invoke RPC and await its completion.
        VtnFlowService rpc = provider.getVtnRpcService(VtnFlowService.class);
        GetDataFlowOutput output = getRpcOutput(rpc.getDataFlow(input));

        List<DataFlowInfo> result = output.getDataFlowInfo();
        if (result == null) {
            // This should never happen.
            throw new VTNException("Data flow list is unavailable.");
        }

        return result;
    }

    /**
     * Convert the given {@link Vtn} instance into a {@link VTenant} instance.
     *
     * @param vtn  A {@link Vtn} instance.
     * @return  A {@link VTenant} instance.
     */
    private VTenant toVTenant(Vtn vtn) {
        String name = vtn.getName().getValue();
        VtenantConfig config = vtn.getVtenantConfig();
        String desc = config.getDescription();
        Integer idle = config.getIdleTimeout();
        Integer hard = config.getHardTimeout();
        VTenantConfig tconf = new VTenantConfig(desc, idle, hard);
        return new VTenant(name, tconf);
    }

    /**
     * Convert the given {@link Vbridge} instance into a {@link VBridge}
     * instance.
     *
     * @param vbridge  A {@link Vbridge} instance.
     * @return  A {@link VBridge} instance.
     */
    private VBridge toVBridge(Vbridge vbridge) {
        String name = vbridge.getName().getValue();
        VbridgeConfig config = vbridge.getVbridgeConfig();
        BridgeStatus bst = vbridge.getBridgeStatus();
        String desc = config.getDescription();
        Integer age = config.getAgeInterval();
        VBridgeConfig bconf = new VBridgeConfig(desc, age);
        return new VBridge(name, bst.getState(), bst.getPathFaults(), bconf);
    }

    /**
     * Convert the given {@link Vterminal} instance into a {@link VTerminal}
     * instance.
     *
     * @param vterminal  A {@link Vterminal} instance.
     * @return  A {@link VTerminal} instance.
     */
    private VTerminal toVTerminal(Vterminal vterminal) {
        String name = vterminal.getName().getValue();
        VterminalConfig config = vterminal.getVterminalConfig();
        BridgeStatus bst = vterminal.getBridgeStatus();
        VTerminalConfig vtconf = new VTerminalConfig(config.getDescription());
        return new VTerminal(name, bst.getState(), bst.getPathFaults(), vtconf);
    }

    /**
     * Convert the given {@link Vinterface} instance into a {@link VInterface}
     * instance.
     *
     * @param vintf  A {@link Vinterface} instance.
     * @return  A {@link VInterface} instance.
     */
    private VInterface toVInterface(Vinterface vintf) {
        String name = vintf.getName().getValue();
        VinterfaceConfig config = vintf.getVinterfaceConfig();
        VinterfaceStatus ist = vintf.getVinterfaceStatus();
        String desc = config.getDescription();
        Boolean enabled = config.isEnabled();
        VInterfaceConfig iconf = new VInterfaceConfig(desc, enabled);
        return new VInterface(name, ist.getState(), ist.getEntityState(),
                              iconf);
    }

    /**
     * Return a list of virtual interfaces in the given virtual bridge.
     *
     * @param bridge  A {@link VtnPortMappableBridge} instance.
     * @return  A list of {@link VInterface} instances.
     */
    private List<VInterface> getVInterfaces(VtnPortMappableBridge bridge) {
        List<Vinterface> interfaces = bridge.getVinterface();
        List<VInterface> list = new ArrayList<>();
        if (interfaces != null) {
            for (Vinterface vintf: interfaces) {
                list.add(toVInterface(vintf));
            }

            // Sort interfaces by their name.
            VTNIdentifiableComparator<String> comparator =
                new VTNIdentifiableComparator<>(String.class);
            Collections.sort(list, comparator);
        }

        return list;
    }

    /**
     * Convert the given {@link VtnVlanMapInfo} instance into a {@link VlanMap}
     * instance.
     *
     * @param vmi  A {@link VtnVlanMapInfo} instance.
     * @return  A {@link VlanMap} instance.
     */
    private VlanMap toVlanMap(VtnVlanMapInfo vmi) {
        String mapId = toAdVlanMapId(vmi.getMapId());
        VtnVlanMapConfig config = vmi.getVlanMapConfig();
        SalNode snode = SalNode.create(config.getNode());
        Node node = (snode == null) ? null : snode.getAdNode();
        Integer vid = config.getVlanId().getValue();
        return new VlanMap(mapId, node, vid.shortValue());
    }

    /**
     * Convert the given AD-SAL VLAN mapping ID into the MD-SAL VLAN mapping
     * ID.
     *
     * @param mapId  The AD-SAL VLAN mapping ID to be converted.
     * @return  The MD-SAL VLAN mapping ID.
     * @throws RpcException  The given VLAN mapping ID is invalid.
     */
    private String toMdVlanMapId(String mapId) throws RpcException {
        if (mapId == null) {
            throw RpcException.getNullArgumentException("VLAN mapping ID");
        }

        try {
            int idx = mapId.lastIndexOf('.');
            String nodeStr = mapId.substring(0, idx);
            String vidStr = mapId.substring(idx + 1);
            if (!NODE_ID_ANY.equals(nodeStr)) {
                int nidx = nodeStr.indexOf('-');
                String type = nodeStr.substring(0, nidx);
                String nodeId = nodeStr.substring(nidx + 1);
                SalNode snode = SalNode.create(Node.fromString(type, nodeId));
                nodeStr = snode.toString();
            }

            return nodeStr + "." + vidStr;
        } catch (RuntimeException e) {
            throw getVlanMapNotFoundException(mapId);
        }
    }

    /**
     * Convert the given MD-SAL VLAN mapping ID into the AD-SAL VLAN mapping
     * ID.
     *
     * @param mapId  The MD-SAL VLAN mapping ID to be converted.
     * @return  The AD-SAL VLAN mapping ID.
     */
    private String toAdVlanMapId(String mapId) {
        int idx = mapId.lastIndexOf('.');
        if (idx <= 0) {
            // This should never happen.
            throw new IllegalStateException(
                "Unexpected VLAN mapping ID: " + mapId);
        }

        String nodeStr = mapId.substring(0, idx);
        String vidStr = mapId.substring(idx + 1);
        if (!NODE_ID_ANY.equals(nodeStr)) {
            SalNode snode = SalNode.create(nodeStr);
            Node node = snode.getAdNode();
            nodeStr = node.getType() + "-" + node.getNodeIDString();
        }

        return nodeStr + "." + vidStr;
    }

    /**
     * Return a port mapping information configured in the given
     * {@link Vinterface} instance.
     *
     * @param vintf A {@link Vinterface} instance.
     * @return  A {@link PortMap} instance if a port mapping is configured.
     *          {@code null} if not configured.
     */
    private PortMap getPortMap(Vinterface vintf) {
        PortMap pmap;
        VtnPortMapConfig vpmc = vintf.getPortMapConfig();
        if (vpmc == null) {
            pmap = null;
        } else {
            SalNode snode = SalNode.create(vpmc.getNode());
            Node node = snode.getAdNode();
            SwitchPort swport = NodeUtils.toSwitchPort(vpmc);
            int vid = vpmc.getVlanId().getValue();
            PortMapConfig pmconf = new PortMapConfig(node, swport, (short)vid);

            VinterfaceStatus ist = vintf.getVinterfaceStatus();
            SalPort mapped = SalPort.create(ist.getMappedPort());
            NodeConnector nc = (mapped == null)
                ? null : mapped.getAdNodeConnector();
            pmap = new PortMap(pmconf, nc);
        }

        return pmap;
    }

    /**
     * Convert the given {@link DataLinkAddress} instance into a
     * {@link MacAddress} instance.
     *
     * @param addr  A {@link DataLinkAddress} instance.
     * @return  A {@link MacAddress} instance on success.
     *          {@code null} on failure.
     * @throws RpcException  {@code addr} is {@code null}.
     */
    private MacAddress toMacAddress(DataLinkAddress addr) throws RpcException {
        MacAddress mac;
        if (addr instanceof EthernetAddress) {
            EtherAddress eaddr = new EtherAddress((EthernetAddress)addr);
            mac = eaddr.getMacAddress();
        } else if (addr == null) {
            throw RpcException.getNullArgumentException("MAC address");
        } else {
            mac = null;
        }

        return mac;
    }

    /**
     * Convert the given {@link VtnMacMapInfo} instance into a {@link MacMap}
     * instance.
     *
     * @param rtx    Read transaction for the MD-SAL datastore.
     * @param ident  The identifier for the target vBridge.
     * @param vmmi   A {@link VtnMacMapInfo} instance.
     * @return  A {@link MacMap} instance if {@code vmmi} is not {@code null}.
     *          {@code null} if {@code vmmi} is {@code null}.
     * @throws VTNException  An error occurred.
     */
    private MacMap toMacMap(ReadTransaction rtx, VBridgeIdentifier ident,
                            VtnMacMapInfo vmmi) throws VTNException {
        MacMap mmap;
        if (vmmi == null) {
            mmap = null;
        } else {
            VtnMacMapConfig config = vmmi.getMacMapConfig();
            Set<DataLinkHost> allowed =
                toDataLinkHosts(config.getAllowedHosts());
            Set<DataLinkHost> denied =
                toDataLinkHosts(config.getDeniedHosts());

            MacMapStatus mst = vmmi.getMacMapStatus();
            List<MacAddressEntry> mapped =
                toMacAddressEntries(rtx, ident, mst.getMappedHost());
            mmap = new MacMap(allowed, denied, mapped);
        }

        return mmap;
    }

    /**
     * Return a list of host information in the specified access control list
     * configured in the given MAC mapping.
     *
     * @param vmmi     A {@link VtnMacMapInfo} instance.
     * @param aclType  The type of access control list.
     * @return  A set of {@link DataLinkHost} instances or {@code null}.
     * @throws RpcException  An error occurred.
     */
    private Set<DataLinkHost> getMacMapConfig(VtnMacMapInfo vmmi,
                                              VtnAclType aclType)
        throws RpcException {
        Set<DataLinkHost> dhset;
        if (vmmi == null) {
            // MAC mapping is not configured.
            dhset = null;
        } else {
            VtnMacMapConfig config = vmmi.getMacMapConfig();
            if (aclType == VtnAclType.ALLOW) {
                dhset = toDataLinkHosts(config.getAllowedHosts());
            } else if (aclType == VtnAclType.DENY) {
                dhset = toDataLinkHosts(config.getDeniedHosts());
            } else {
                throw RpcException.getNullArgumentException("ACL type");
            }
        }

        return dhset;
    }

    /**
     * Convert the given {@link VlanHostDescSet} instance into a set of
     * {@link DataLinkHost} instances.
     *
     * @param vhset  A {@link VlanHostDescSet} instance.
     * @return  A set of {@link DataLinkHost} instances.
     * @throws RpcException
     *    {@code vhset} contains invalid value.
     */
    private Set<DataLinkHost> toDataLinkHosts(VlanHostDescSet vhset)
        throws RpcException {
        Set<DataLinkHost> dhset = new HashSet<>();
        if (vhset != null) {
            List<VlanHostDescList> vhlist = vhset.getVlanHostDescList();
            if (vhlist != null) {
                for (VlanHostDescList vhdl: vhlist) {
                    MacVlan mvlan = new MacVlan(vhdl.getHost());
                    dhset.add(mvlan.getEthernetHost());
                }
            }
        }

        return dhset;
    }

    /**
     * Convert the given list of {@link MappedHost} instances into a list of
     * {@link MacAddressEntry} instances.
     *
     * @param rtx     Read transaction for the MD-SAL datastore.
     * @param ident   The identifier for the target vBridge.
     * @param mhosts  A list of {@link MappedHost} instances.
     * @return  A list of {@link MacAddressEntry} instances.
     * @throws VTNException  An error occurred.
     */
    private List<MacAddressEntry> toMacAddressEntries(
        ReadTransaction rtx, VBridgeIdentifier ident, List<MappedHost> mhosts)
        throws VTNException {
        List<MacAddressEntry> list = new ArrayList<>();
        if (mhosts != null) {
            for (MappedHost mhost: mhosts) {
                MacAddressEntry ment = VBridgeManager.
                    readMacAddressEntry(rtx, ident, mhost.getMacAddress());
                if (ment != null) {
                    list.add(ment);
                }
            }
        }

        return list;
    }

    /**
     * Convert the given set of {@link DataLinkHost} instances into a list of
     * {@link VlanHostDesc} instances.
     *
     * @param dlhosts  A set of {@link DataLinkHost} instances.
     * @return  A list of {@link VlanHostDesc} instances if {@code dlhosts}
     *          is not empty. {@code null} otherwise.
     * @throws RpcException
     *    {@code dlhosts} contains invalid value.
     */
    private List<VlanHostDesc> toVlanHostDescs(
        Set<? extends DataLinkHost> dlhosts) throws RpcException {
        List<VlanHostDesc> vhdescs;
        if (MiscUtils.isEmpty(dlhosts)) {
            vhdescs = null;
        } else {
            vhdescs = new ArrayList<>();
            for (DataLinkHost dlhost: dlhosts) {
                MacVlan mvlan = new MacVlan(dlhost);
                vhdescs.add(mvlan.getVlanHostDesc());
            }
        }

        return vhdescs;
    }

    /**
     * Create a new input builder for update-vtn RPC.
     *
     * @param path   Path to the virtual tenant.
     * @param tconf  Tenant configuration.
     * @return  An input builder for update-vtn RPC.
     * @throws RpcException  An error occurred.
     */
    private UpdateVtnInputBuilder createInputBuilder(
        VTenantPath path, VTenantConfig tconf) throws RpcException {
        String name = VTenantUtils.getName(path);
        checkTenantConfig(tconf);

        UpdateVtnInputBuilder builder = new UpdateVtnInputBuilder().
            setTenantName(name).
            setDescription(tconf.getDescription());

        int timeout = tconf.getIdleTimeout();
        if (timeout >= 0) {
            try {
                builder.setIdleTimeout(timeout);
            } catch (RuntimeException e) {
                throw RpcException.getBadArgumentException(
                    "Invalid idle-timeout: " + timeout, e);
            }
        }

        timeout = tconf.getHardTimeout();
        if (timeout >= 0) {
            try {
                builder.setHardTimeout(timeout);
            } catch (RuntimeException e) {
                throw RpcException.getBadArgumentException(
                    "Invalid hard-timeout: " + timeout, e);
            }
        }

        return builder;
    }

    /**
     * Create a new input builder for update-vbridge RPC.
     *
     * @param path   Path to the vBridge.
     * @param bconf  vBridge configuration.
     * @return  An input builder for update-vbridge RPC.
     * @throws RpcException  An error occurred.
     */
    private UpdateVbridgeInputBuilder createInputBuilder(
        VBridgePath path, VBridgeConfig bconf) throws RpcException {
        String tname = VTenantUtils.getName(path);
        checkBridgeConfig(bconf);

        UpdateVbridgeInputBuilder builder = new UpdateVbridgeInputBuilder().
            setTenantName(tname).
            setBridgeName(path.getBridgeName()).
            setDescription(bconf.getDescription());

        int age = bconf.getAgeInterval();
        if (age >= 0) {
            try {
                builder.setAgeInterval(age);
            } catch (RuntimeException e) {
                throw RpcException.getBadArgumentException(
                    "Invalid age-interval: " + age, e);
            }
        }

        return builder;
    }

    /**
     * Create a new input builder for update-vterminal RPC.
     *
     * @param path    Path to the vTerminal.
     * @param vtconf  vTerminal configuration.
     * @return  An input builder for update-vterminal RPC.
     * @throws RpcException  An error occurred.
     */
    private UpdateVterminalInputBuilder createInputBuilder(
        VTerminalPath path, VTerminalConfig vtconf) throws RpcException {
        String tname = VTenantUtils.getName(path);
        checkTerminalConfig(vtconf);

        return new UpdateVterminalInputBuilder().
            setTenantName(tname).
            setTerminalName(path.getTerminalName()).
            setDescription(vtconf.getDescription());
    }

    /**
     * Create a new input builder for update-vinterface RPC.
     *
     * @param path   Path to the vBridge interface.
     * @param iconf  Virtual interface configuration.
     * @return  An input builder for update-vinterface RPC.
     * @throws RpcException  An error occurred.
     */
    private UpdateVinterfaceInputBuilder createInputBuilder(
        VBridgeIfPath path, VInterfaceConfig iconf) throws RpcException {
        String tname = VTenantUtils.getName(path);
        checkInterfaceConfig(iconf);

        return new UpdateVinterfaceInputBuilder().
            setTenantName(tname).
            setBridgeName(path.getBridgeName()).
            setInterfaceName(path.getInterfaceName()).
            setDescription(iconf.getDescription()).
            setEnabled(iconf.getEnabled());
    }

    /**
     * Create a new input builder for update-vinterface RPC.
     *
     * @param path   Path to the vTerminal interface.
     * @param iconf  Virtual interface configuration.
     * @return  An input builder for update-vinterface RPC.
     * @throws RpcException  An error occurred.
     */
    private UpdateVinterfaceInputBuilder createInputBuilder(
        VTerminalIfPath path, VInterfaceConfig iconf) throws RpcException {
        String tname = VTenantUtils.getName(path);
        checkInterfaceConfig(iconf);

        return new UpdateVinterfaceInputBuilder().
            setTenantName(tname).
            setTerminalName(path.getTerminalName()).
            setInterfaceName(path.getInterfaceName()).
            setDescription(iconf.getDescription()).
            setEnabled(iconf.getEnabled());
    }

    /**
     * Create a new input for set-port-map RPC.
     *
     * @param path    Path to the vBridge interface.
     * @param pmconf  Port mapping configuration.
     * @return  An input for set-port-map RPC.
     * @throws RpcException  An error occurred.
     */
    private SetPortMapInput createInput(
        VBridgeIfPath path, PortMapConfig pmconf) throws RpcException {
        String tname = VTenantUtils.getName(path);
        SetPortMapInputBuilder builder = new SetPortMapInputBuilder().
            setTenantName(tname).
            setBridgeName(path.getBridgeName()).
            setInterfaceName(path.getInterfaceName());
        return setPortMapConfig(builder, pmconf).build();
    }

    /**
     * Create a new input for set-port-map RPC.
     *
     * @param path    Path to the vTerminal interface.
     * @param pmconf  Port mapping configuration.
     * @return  An input for set-port-map RPC.
     * @throws RpcException  An error occurred.
     */
    private SetPortMapInput createInput(
        VTerminalIfPath path, PortMapConfig pmconf) throws RpcException {
        String tname = VTenantUtils.getName(path);
        SetPortMapInputBuilder builder = new SetPortMapInputBuilder().
            setTenantName(tname).
            setTerminalName(path.getTerminalName()).
            setInterfaceName(path.getInterfaceName());
        return setPortMapConfig(builder, pmconf).build();
    }

    /**
     * Create a new input builder for get-mac-mapped-host RPC.
     *
     * @param path   Path to the vBridge.
     * @return  An input builder for get-mac-mapped-host RPC.
     * @throws RpcException  An error occurred.
     */
    private GetMacMappedHostInputBuilder createInputBuilder(VBridgePath path)
        throws RpcException {
        String tname = VTenantUtils.getName(path);

        return new GetMacMappedHostInputBuilder().
            setTenantName(tname).
            setBridgeName(path.getBridgeName());
    }

    /**
     * Configure the given port mapping configuration into the given input
     * builder for set-port-map RPC.
     *
     * @param builder  Input builder for set-port-map RPC.
     * @param pmconf   Port mapping configuration.
     *                 Note that this method does nothing if {@code pmconf}
     *                 is {@code null}.
     * @return  {@code builder} is always returned.
     * @throws RpcException  An error occurred.
     */
    private SetPortMapInputBuilder setPortMapConfig(
        SetPortMapInputBuilder builder, PortMapConfig pmconf)
        throws RpcException {
        if (pmconf != null) {
            Node node = pmconf.getNode();
            NodeUtils.checkNode(node);
            SalNode snode = SalNode.create(node);
            if (snode != null) {
                builder.setNode(snode.getNodeId());
            }

            short vid = pmconf.getVlan();
            try {
                builder.setVlanId(new VlanId((int)vid));
            } catch (RuntimeException e) {
                throw RpcException.getBadArgumentException(
                    "Invalid VLAN ID: " + vid, e);
            }

            SwitchPort swport = pmconf.getPort();
            NodeUtils.checkSwitchPort(swport, node);
            if (swport != null) {
                builder.setPortId(swport.getId()).
                    setPortName(swport.getName());
            }
        }

        return builder;
    }

    /**
     * Remove the port mapping configuration from the specified virtual
     * interface.
     *
     * @param provider  VTN Manager provider service.
     * @param vpath     A {@link VnodePathFields} instance that specifies the
     *                  target virtual interface.
     * @throws VTNException  An error occurred.
     */
    private void removePortMap(
        VTNManagerProvider provider, VnodePathFields vpath)
        throws VTNException {
        RemovePortMapInput input = new RemovePortMapInputBuilder(vpath).
            build();

        // Invoke RPC and await its completion.
        VtnPortMapService rpc =
            provider.getVtnRpcService(VtnPortMapService.class);
        getRpcOutput(rpc.removePortMap(input));
    }

    /**
     * Convert the given {@link VTenantPath} instance into a
     * {@link VNodeIdentifier} instance.
     *
     * @param fid  A {@link FlowFilterId} instance.
     * @return  A {@link VNodeIdentifier} instance.
     * @throws RpcException  An error occurred.
     */
    private VNodeIdentifier<?> toVNodeIdentifier(FlowFilterId fid)
        throws RpcException {
        if (fid == null) {
            throw RpcException.getNullArgumentException("Flow filter ID");
        }

        return VNodeIdentifier.create(fid.getPath(), true);
    }

    /**
     * Convert the given {@link FlowFilterId} instance into an instance
     * identifier for the flow filter list.
     *
     * @param fid  A {@link FlowFilterId} instance.
     * @return  An instance identifier for the flow filter list.
     * @throws RpcException  An error occurred.
     */
    private InstanceIdentifier<? extends VtnFlowFilterList> getFlowFilterPath(
        FlowFilterId fid) throws RpcException {
        if (fid == null) {
            throw RpcException.getNullArgumentException("Flow filter ID");
        }

        VTenantPath tpath = fid.getPath();
        if (tpath == null) {
            throw RpcException.getNullArgumentException("VTenantPath");
        }

        boolean output = fid.isOutput();
        InstanceIdentifier<? extends VtnFlowFilterList>  path;
        if (tpath instanceof VBridgeIfPath) {
            VBridgeIfPath p = (VBridgeIfPath)tpath;
            VBridgeIfIdentifier ifId = VBridgeIfIdentifier.create(
                p.getTenantName(), p.getBridgeName(), p.getInterfaceName(),
                true);
            if (output) {
                path = ifId.getIdentifierBuilder().
                    child(VinterfaceOutputFilter.class).build();
            } else {
                path = ifId.getIdentifierBuilder().
                    child(VinterfaceInputFilter.class).build();
            }
        } else if (tpath instanceof VBridgePath) {
            VBridgePath p = (VBridgePath)tpath;
            VBridgeIdentifier vbrId = VBridgeIdentifier.create(
                p.getTenantName(), p.getBridgeName(), true);
            if (output) {
                path = vbrId.getIdentifierBuilder().
                    child(VbridgeOutputFilter.class).build();
            } else {
                path = vbrId.getIdentifierBuilder().
                    child(VbridgeInputFilter.class).build();
            }
        } else if (tpath instanceof VTerminalIfPath) {
            VTerminalIfPath p = (VTerminalIfPath)tpath;
            VTerminalIfIdentifier ifId = VTerminalIfIdentifier.create(
                p.getTenantName(), p.getTerminalName(), p.getInterfaceName(),
                true);
            if (output) {
                path = ifId.getIdentifierBuilder().
                    child(VinterfaceOutputFilter.class).build();
            } else {
                path = ifId.getIdentifierBuilder().
                    child(VinterfaceInputFilter.class).build();
            }
        } else if (tpath.getClass().equals(VTenantPath.class)) {
            VTenantIdentifier vtnId =
                VTenantIdentifier.create(tpath.getTenantName(), true);
            path = vtnId.getIdentifierBuilder().
                child(VtnInputFilter.class).build();
        } else {
            throw RpcException.getBadArgumentException(
                "Invalid flow filter ID: " + tpath);
        }

        return path;
    }

    /**
     * Construct an input for remove-flow-filter RPC.
     *
     * @param fid    A {@link FlowFilterId} instance.
     * @param index  A flow filter index.
     * @return  A {@link RemoveFlowFilterInput} instance.
     * @throws VTNException  An error occurred.
     */
    private RemoveFlowFilterInput createRemoveFlowFilterInput(
        FlowFilterId fid, Integer index) throws VTNException {
        VNodeIdentifier<?> ident = toVNodeIdentifier(fid);
        RemoveFlowFilterInputBuilder ib = new RemoveFlowFilterInputBuilder().
            setOutput(fid.isOutput()).
            setTenantName(ident.getTenantNameString()).
            setInterfaceName(ident.getInterfaceNameString());
        String bname = ident.getBridgeNameString();
        if (ident.getType().getBridgeType() == VNodeType.VBRIDGE) {
            ib.setBridgeName(bname);
        } else {
            ib.setTerminalName(bname);
        }

        List<Integer> indices = (index == null)
            ? null : Collections.singletonList(index);

        return ib.setIndices(indices).build();
    }

    // IVTNManager

    /**
     * Determine whether the Virtual Tenant Network is active in the container.
     *
     * @return  {@code true} is returned if the VTN is active in the container.
     *          Otherwise {@code false} is returned.
     */
    @Override
    public boolean isActive() {
        TxContext ctx = null;
        InstanceIdentifier<Vtns> path = InstanceIdentifier.create(Vtns.class);
        boolean active = false;
        try {
            VTNManagerProvider provider = checkService();
            ctx = provider.newTxContext();
            ReadTransaction rtx = ctx.getTransaction();
            Optional<Vtns> opt = DataStoreUtils.read(
                rtx, LogicalDatastoreType.OPERATIONAL, path);
            if (opt.isPresent()) {
                active = !MiscUtils.isEmpty(opt.get().getVtn());
            }
        } catch (VTNException e) {
            LOG.error("Failed to read VTN root container.", e);
        } finally {
            if (ctx != null) {
                ctx.cancelTransaction();
            }
        }

        return active;
    }

    /**
     * Return a list of virtual tenant configurations.
     *
     * @return  A list which contains tenant configurations.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<VTenant> getTenants() throws VTNException {
        InstanceIdentifier<Vtns> path = InstanceIdentifier.create(Vtns.class);
        List<VTenant> list = new ArrayList<>();
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            ReadTransaction rtx = ctx.getTransaction();
            Optional<Vtns> opt = DataStoreUtils.read(
                rtx, LogicalDatastoreType.OPERATIONAL, path);
            if (opt.isPresent()) {
                List<Vtn> vtns = opt.get().getVtn();
                if (vtns != null) {
                    for (Vtn vtn: vtns) {
                        list.add(toVTenant(vtn));
                    }
                }
            }
        } finally {
            ctx.cancelTransaction();
        }

        // Sort tenants by their name.
        VTNIdentifiableComparator<String> comparator =
            new VTNIdentifiableComparator<>(String.class);
        Collections.sort(list, comparator);
        return list;
    }

    /**
     * Return the tenant information specified by the given name.
     *
     * @param path  Path to the virtual tenant.
     * @return  Information about the specified tenant.
     * @throws VTNException  An error occurred.
     */
    @Override
    public VTenant getTenant(VTenantPath path) throws VTNException {
        String name = VTenantUtils.getName(path);
        VTenantIdentifier ident = VTenantIdentifier.create(name, true);
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            return toVTenant(ident.fetch(ctx.getTransaction()));
        } finally {
            ctx.cancelTransaction();
        }
    }

    /**
     * Add a new virtual tenant.
     *
     * @param path   Path to the virtual tenant.
     * @param tconf  Tenant configuration
     * @return  "Success" or failure reason.
     */
    @Override
    public Status addTenant(VTenantPath path, VTenantConfig tconf) {
        Status status;
        try {
            VTNManagerProvider provider = checkService();
            UpdateVtnInput input = createInputBuilder(path, tconf).
                setUpdateMode(VnodeUpdateMode.CREATE).
                build();

            // Invoke RPC and await its completion.
            VtnService rpc = provider.getVtnRpcService(VtnService.class);
            getRpcOutput(rpc.updateVtn(input));
            status = new Status(StatusCode.SUCCESS, null);
        } catch (VTNException e) {
            status = e.getStatus();
        }

        return status;
    }

    /**
     * Modify configuration of existing virtual tenant.
     *
     * @param path   Path to the virtual tenant.
     * @param tconf  Tenant configuration
     * @param all    If {@code true} is specified, all attributes of the
     *               tenant are modified. In this case, {@code null} in
     *               {@code tconf} is interpreted as default value.
     *               If {@code false} is specified, an attribute is not
     *               modified if its value in {@code tconf} is {@code null}.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status modifyTenant(VTenantPath path, VTenantConfig tconf,
                               boolean all) {
        Status status;
        try {
            VTNManagerProvider provider = checkService();
            VtnUpdateOperationType op = (all)
                ? VtnUpdateOperationType.SET
                : VtnUpdateOperationType.ADD;
            UpdateVtnInput input = createInputBuilder(path, tconf).
                setUpdateMode(VnodeUpdateMode.MODIFY).
                setOperation(op).
                build();

            // Invoke RPC and await its completion.
            VtnService rpc = provider.getVtnRpcService(VtnService.class);
            getRpcOutput(rpc.updateVtn(input));
            status = new Status(StatusCode.SUCCESS, null);
        } catch (VTNException e) {
            status = e.getStatus();
        }

        return status;
    }

    /**
     * Remove a tenant specified by the given name.
     *
     * @param path  Path to the virtual tenant.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status removeTenant(VTenantPath path) {
        Status status;
        try {
            VTNManagerProvider provider = checkService();
            String name = VTenantUtils.getName(path);
            RemoveVtnInput input = new RemoveVtnInputBuilder().
                setTenantName(name).build();

            // Invoke RPC and await its completion.
            VtnService rpc = provider.getVtnRpcService(VtnService.class);
            getRpcOutput(rpc.removeVtn(input), true);
            status = new Status(StatusCode.SUCCESS, null);
        } catch (VTNException e) {
            status = e.getStatus();
        }

        return status;
    }

    /**
     * Return a list of virtual L2 bridges in the specified tenant.
     *
     * @param path  Path to the virtual tenant.
     * @return  A list of virtual L2 bridges.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<VBridge> getBridges(VTenantPath path) throws VTNException {
        String name = VTenantUtils.getName(path);
        VTenantIdentifier ident = VTenantIdentifier.create(name, true);
        List<VBridge> list = new ArrayList<>();
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            Vtn vtn = ident.fetch(ctx.getTransaction());
            List<Vbridge> vbridges = vtn.getVbridge();
            if (vbridges != null) {
                for (Vbridge vbridge: vbridges) {
                    list.add(toVBridge(vbridge));
                }
            }
        } finally {
            ctx.cancelTransaction();
        }

        // Sort vBridges by their name.
        VTNIdentifiableComparator<String> comparator =
            new VTNIdentifiableComparator<>(String.class);
        Collections.sort(list, comparator);
        return list;
    }

    /**
     * Return information about the specified virtual L2 bridge.
     *
     * @param path  Path to the virtual bridge.
     * @return  Information about the specified L2 bridge.
     * @throws VTNException  An error occurred.
     */
    @Override
    public VBridge getBridge(VBridgePath path) throws VTNException {
        String tname = VTenantUtils.getName(path);
        String bname = path.getBridgeName();
        VBridgeIdentifier ident = VBridgeIdentifier.create(tname, bname, true);
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            return toVBridge(ident.fetch(ctx.getTransaction()));
        } finally {
            ctx.cancelTransaction();
        }
    }

    /**
     * Add a new virtual L2 bridge.
     *
     * @param path   Path to the virtual bridge to be added.
     * @param bconf  Bridge configuration.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status addBridge(VBridgePath path, VBridgeConfig bconf) {
        Status status;
        try {
            VTNManagerProvider provider = checkService();
            UpdateVbridgeInput input = createInputBuilder(path, bconf).
                setUpdateMode(VnodeUpdateMode.CREATE).
                build();

            // Invoke RPC and await its completion.
            VtnVbridgeService rpc =
                provider.getVtnRpcService(VtnVbridgeService.class);
            getRpcOutput(rpc.updateVbridge(input));
            status = new Status(StatusCode.SUCCESS, null);
        } catch (VTNException e) {
            status = e.getStatus();
        }

        return status;
    }

    /**
     * Modify configuration of existing virtual L2 bridge.
     *
     * @param path   Path to the virtual bridge.
     * @param bconf  Bridge configuration.
     * @param all    If {@code true} is specified, all attributes of the
     *               bridge are modified. In this case, {@code null} in
     *               {@code bconf} is interpreted as default value.
     *               If {@code false} is specified, an attribute is not
     *               modified if its value in {@code bconf} is {@code null}.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status modifyBridge(VBridgePath path, VBridgeConfig bconf,
                               boolean all) {
        Status status;
        try {
            VTNManagerProvider provider = checkService();
            VtnUpdateOperationType op = (all)
                ? VtnUpdateOperationType.SET
                : VtnUpdateOperationType.ADD;
            UpdateVbridgeInput input = createInputBuilder(path, bconf).
                setUpdateMode(VnodeUpdateMode.MODIFY).
                setOperation(op).
                build();

            // Invoke RPC and await its completion.
            VtnVbridgeService rpc =
                provider.getVtnRpcService(VtnVbridgeService.class);
            getRpcOutput(rpc.updateVbridge(input));
            status = new Status(StatusCode.SUCCESS, null);
        } catch (VTNException e) {
            status = e.getStatus();
        }

        return status;
    }

    /**
     * Remove the virtual L2 bridge specified by the given name.
     *
     * @param path  Path to the virtual bridge.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status removeBridge(VBridgePath path) {
        Status status;
        try {
            VTNManagerProvider provider = checkService();
            String tname = VTenantUtils.getName(path);
            RemoveVbridgeInput input = new RemoveVbridgeInputBuilder().
                setTenantName(tname).
                setBridgeName(path.getBridgeName()).
                build();

            // Invoke RPC and await its completion.
            VtnVbridgeService rpc =
                provider.getVtnRpcService(VtnVbridgeService.class);
            getRpcOutput(rpc.removeVbridge(input), true);
            status = new Status(StatusCode.SUCCESS, null);
        } catch (VTNException e) {
            status = e.getStatus();
        }

        return status;
    }

    /**
     * Return a list of vTerminals in the specified tenant.
     *
     * @param path  Path to the virtual tenant.
     * @return  A list of vTerminals.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<VTerminal> getTerminals(VTenantPath path)
        throws VTNException {
        String name = VTenantUtils.getName(path);
        VTenantIdentifier ident = VTenantIdentifier.create(name, true);
        List<VTerminal> list = new ArrayList<>();
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            Vtn vtn = ident.fetch(ctx.getTransaction());
            List<Vterminal> vterminals = vtn.getVterminal();
            if (vterminals != null) {
                for (Vterminal vterminal: vterminals) {
                    list.add(toVTerminal(vterminal));
                }
            }
        } finally {
            ctx.cancelTransaction();
        }

        // Sort vTerminals by their name.
        VTNIdentifiableComparator<String> comparator =
            new VTNIdentifiableComparator<>(String.class);
        Collections.sort(list, comparator);
        return list;
    }

    /**
     * Return information about the specified vTerminal.
     *
     * @param path  Path to the vTerminal.
     * @return  Information about the specified vTerminal.
     * @throws VTNException  An error occurred.
     */
    @Override
    public VTerminal getTerminal(VTerminalPath path) throws VTNException {
        String tname = VTenantUtils.getName(path);
        String bname = path.getTerminalName();
        VTerminalIdentifier ident =
            VTerminalIdentifier.create(tname, bname, true);
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            return toVTerminal(ident.fetch(ctx.getTransaction()));
        } finally {
            ctx.cancelTransaction();
        }
    }

    /**
     * Add a new vTerminal.
     *
     * @param path    Path to the vTerminal to be added.
     * @param vtconf  vTerminal configuration.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status addTerminal(VTerminalPath path, VTerminalConfig vtconf) {
        Status status;
        try {
            VTNManagerProvider provider = checkService();
            UpdateVterminalInput input = createInputBuilder(path, vtconf).
                setUpdateMode(VnodeUpdateMode.CREATE).
                build();

            // Invoke RPC and await its completion.
            VtnVterminalService rpc =
                provider.getVtnRpcService(VtnVterminalService.class);
            getRpcOutput(rpc.updateVterminal(input));
            status = new Status(StatusCode.SUCCESS, null);
        } catch (VTNException e) {
            status = e.getStatus();
        }

        return status;
    }

    /**
     * Modify configuration of existing vTerminal.
     *
     * @param path    Path to the vTerminal.
     * @param vtconf  vTerminal configuration.
     * @param all     If {@code true} is specified, all attributes of the
     *                vTerminal are modified. In this case, {@code null} in
     *                {@code vtconf} is interpreted as default value.
     *                If {@code false} is specified, an attribute is not
     *                modified if its value in {@code vtconf} is {@code null}.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status modifyTerminal(VTerminalPath path, VTerminalConfig vtconf,
                               boolean all) {
        Status status;
        try {
            VTNManagerProvider provider = checkService();
            VtnUpdateOperationType op = (all)
                ? VtnUpdateOperationType.SET
                : VtnUpdateOperationType.ADD;
            UpdateVterminalInput input = createInputBuilder(path, vtconf).
                setUpdateMode(VnodeUpdateMode.MODIFY).
                setOperation(op).
                build();

            // Invoke RPC and await its completion.
            VtnVterminalService rpc =
                provider.getVtnRpcService(VtnVterminalService.class);
            getRpcOutput(rpc.updateVterminal(input));
            status = new Status(StatusCode.SUCCESS, null);
        } catch (VTNException e) {
            status = e.getStatus();
        }

        return status;
    }

    /**
     * Remove the vTerminal specified by the given name.
     *
     * @param path  Path to the vTerminal.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status removeTerminal(VTerminalPath path) {
        Status status;
        try {
            VTNManagerProvider provider = checkService();
            String tname = VTenantUtils.getName(path);
            RemoveVterminalInput input = new RemoveVterminalInputBuilder().
                setTenantName(tname).
                setTerminalName(path.getTerminalName()).
                build();

            // Invoke RPC and await its completion.
            VtnVterminalService rpc =
                provider.getVtnRpcService(VtnVterminalService.class);
            getRpcOutput(rpc.removeVterminal(input), true);
            status = new Status(StatusCode.SUCCESS, null);
        } catch (VTNException e) {
            status = e.getStatus();
        }

        return status;
    }

    /**
     * Return a list of virtual interfaces attached to the specified virtual
     * L2 bridge.
     *
     * @param path  Path to the vBridge.
     * @return  A list of virtual interfaces.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<VInterface> getInterfaces(VBridgePath path)
        throws VTNException {
        String name = VTenantUtils.getName(path);
        VBridgeIdentifier ident =
            VBridgeIdentifier.create(name, path.getBridgeName(), true);
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        List<VInterface> list;
        try {
            return getVInterfaces(ident.fetch(ctx.getTransaction()));
        } finally {
            ctx.cancelTransaction();
        }
    }

    /**
     * Return information about the specified virtual interface attached to
     * the virtual L2 bridge.
     *
     * @param path  Path to the vBridge interface.
     * @return  Information about the specified interface.
     * @throws VTNException  An error occurred.
     */
    @Override
    public VInterface getInterface(VBridgeIfPath path)
        throws VTNException {
        String tname = VTenantUtils.getName(path);
        String bname = path.getBridgeName();
        String iname = path.getInterfaceName();
        VBridgeIfIdentifier ident =
            VBridgeIfIdentifier.create(tname, bname, iname, true);
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            return toVInterface(ident.fetch(ctx.getTransaction()));
        } finally {
            ctx.cancelTransaction();
        }
    }

    /**
     * Add a new virtual interface to the virtual L2 bridge.
     *
     * @param path   Path to the vBridge interface to be added.
     * @param iconf  Interface configuration.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status addInterface(VBridgeIfPath path, VInterfaceConfig iconf) {
        Status status;
        try {
            VTNManagerProvider provider = checkService();
            UpdateVinterfaceInput input = createInputBuilder(path, iconf).
                setUpdateMode(VnodeUpdateMode.CREATE).
                build();

            // Invoke RPC and await its completion.
            VtnVinterfaceService rpc =
                provider.getVtnRpcService(VtnVinterfaceService.class);
            getRpcOutput(rpc.updateVinterface(input));
            status = new Status(StatusCode.SUCCESS, null);
        } catch (VTNException e) {
            status = e.getStatus();
        }

        return status;
    }

    /**
     * Modify configuration of existing virtual interface attached to the
     * virtual L2 bridge.
     *
     * @param path   Path to the vBridge interface.
     * @param iconf  Interface configuration.
     * @param all    If {@code true} is specified, all attributes of the
     *               interface are modified. In this case, {@code null} in
     *               {@code iconf} is interpreted as default value.
     *               If {@code false} is specified, an attribute is not
     *               modified if its value in {@code iconf} is {@code null}.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status modifyInterface(VBridgeIfPath path, VInterfaceConfig iconf,
                                  boolean all) {
        Status status;
        try {
            VTNManagerProvider provider = checkService();
            VtnUpdateOperationType op = (all)
                ? VtnUpdateOperationType.SET
                : VtnUpdateOperationType.ADD;
            UpdateVinterfaceInput input = createInputBuilder(path, iconf).
                setUpdateMode(VnodeUpdateMode.MODIFY).
                setOperation(op).
                build();

            // Invoke RPC and await its completion.
            VtnVinterfaceService rpc =
                provider.getVtnRpcService(VtnVinterfaceService.class);
            getRpcOutput(rpc.updateVinterface(input));
            status = new Status(StatusCode.SUCCESS, null);
        } catch (VTNException e) {
            status = e.getStatus();
        }

        return status;
    }

    /**
     * Remove the virtual interface from the virtual L2 bridge.
     *
     * @param path  Path to the interface.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status removeInterface(VBridgeIfPath path) {
        Status status;
        try {
            VTNManagerProvider provider = checkService();
            String tname = VTenantUtils.getName(path);
            RemoveVinterfaceInput input = new RemoveVinterfaceInputBuilder().
                setTenantName(tname).
                setBridgeName(path.getBridgeName()).
                setInterfaceName(path.getInterfaceName()).
                build();

            // Invoke RPC and await its completion.
            VtnVinterfaceService rpc =
                provider.getVtnRpcService(VtnVinterfaceService.class);
            getRpcOutput(rpc.removeVinterface(input), true);
            status = new Status(StatusCode.SUCCESS, null);
        } catch (VTNException e) {
            status = e.getStatus();
        }

        return status;
    }

    /**
     * Return a list of virtual interfaces attached to the specified vTerminal.
     *
     * @param path  Path to the vTerminal.
     * @return  A list of virtual interfaces.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<VInterface> getInterfaces(VTerminalPath path)
        throws VTNException {
        String name = VTenantUtils.getName(path);
        VTerminalIdentifier ident =
            VTerminalIdentifier.create(name, path.getTerminalName(), true);
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        List<VInterface> list;
        try {
            return getVInterfaces(ident.fetch(ctx.getTransaction()));
        } finally {
            ctx.cancelTransaction();
        }
    }

    /**
     * Return information about the specified virtual interface attached to
     * the vTerminal.
     *
     * @param path  Path to the vTerminal interface.
     * @return  Information about the specified interface.
     * @throws VTNException  An error occurred.
     */
    @Override
    public VInterface getInterface(VTerminalIfPath path)
        throws VTNException {
        String tname = VTenantUtils.getName(path);
        String bname = path.getTerminalName();
        String iname = path.getInterfaceName();
        VTerminalIfIdentifier ident =
            VTerminalIfIdentifier.create(tname, bname, iname, true);
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            return toVInterface(ident.fetch(ctx.getTransaction()));
        } finally {
            ctx.cancelTransaction();
        }
    }

    /**
     * Add a new virtual interface to the vTerminal.
     *
     * @param path   Path to the vTerminal interface to be added.
     * @param iconf  Interface configuration.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status addInterface(VTerminalIfPath path, VInterfaceConfig iconf) {
        Status status;
        try {
            VTNManagerProvider provider = checkService();
            UpdateVinterfaceInput input = createInputBuilder(path, iconf).
                setUpdateMode(VnodeUpdateMode.CREATE).
                build();

            // Invoke RPC and await its completion.
            VtnVinterfaceService rpc =
                provider.getVtnRpcService(VtnVinterfaceService.class);
            getRpcOutput(rpc.updateVinterface(input));
            status = new Status(StatusCode.SUCCESS, null);
        } catch (VTNException e) {
            status = e.getStatus();
        }

        return status;
    }

    /**
     * Modify configuration of existing virtual interface attached to the
     * vTerminal.
     *
     * @param path   Path to the vTerminal interface.
     * @param iconf  Interface configuration.
     * @param all    If {@code true} is specified, all attributes of the
     *               interface are modified. In this case, {@code null} in
     *               {@code iconf} is interpreted as default value.
     *               If {@code false} is specified, an attribute is not
     *               modified if its value in {@code iconf} is {@code null}.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status modifyInterface(VTerminalIfPath path, VInterfaceConfig iconf,
                                  boolean all) {
        Status status;
        try {
            VTNManagerProvider provider = checkService();
            VtnUpdateOperationType op = (all)
                ? VtnUpdateOperationType.SET
                : VtnUpdateOperationType.ADD;
            UpdateVinterfaceInput input = createInputBuilder(path, iconf).
                setUpdateMode(VnodeUpdateMode.MODIFY).
                setOperation(op).
                build();

            // Invoke RPC and await its completion.
            VtnVinterfaceService rpc =
                provider.getVtnRpcService(VtnVinterfaceService.class);
            getRpcOutput(rpc.updateVinterface(input));
            status = new Status(StatusCode.SUCCESS, null);
        } catch (VTNException e) {
            status = e.getStatus();
        }

        return status;
    }

    /**
     * Remove the virtual interface from the vTerminal.
     *
     * @param path  Path to the interface.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status removeInterface(VTerminalIfPath path) {
        Status status;
        try {
            VTNManagerProvider provider = checkService();
            String tname = VTenantUtils.getName(path);
            RemoveVinterfaceInput input = new RemoveVinterfaceInputBuilder().
                setTenantName(tname).
                setTerminalName(path.getTerminalName()).
                setInterfaceName(path.getInterfaceName()).
                build();

            // Invoke RPC and await its completion.
            VtnVinterfaceService rpc =
                provider.getVtnRpcService(VtnVinterfaceService.class);
            getRpcOutput(rpc.removeVinterface(input));
            status = new Status(StatusCode.SUCCESS, null);
        } catch (VTNException e) {
            status = e.getStatus();
        }

        return status;
    }

    /**
     * Return a list of VLAN mappings in the specified virtual L2 bridge.
     *
     * @param path  Path to the bridge.
     * @return  A list of VLAN mappings.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<VlanMap> getVlanMaps(VBridgePath path) throws VTNException {
        String name = VTenantUtils.getName(path);
        VBridgeIdentifier ident =
            VBridgeIdentifier.create(name, path.getBridgeName(), true);
        List<VlanMap> list = new ArrayList<>();
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            Vbridge vbridge = ident.fetch(ctx.getTransaction());
            List<? extends VtnVlanMapInfo> vmaps = vbridge.getVlanMap();
            if (vmaps != null) {
                for (VtnVlanMapInfo vmi: vmaps) {
                    list.add(toVlanMap(vmi));
                }
            }
        } finally {
            ctx.cancelTransaction();
        }

        // Sort VLAN mappings by map ID.
        VTNIdentifiableComparator<String> comparator =
            new VTNIdentifiableComparator<>(String.class);
        Collections.sort(list, comparator);
        return list;
    }

    /**
     * Return information about the specified VLAN mapping in the virtual
     * L2 bridge.
     *
     * @param path   Path to the bridge.
     * @param mapId  The identifier of the VLAN mapping.
     * @return  Information about the specified VLAN mapping.
     * @throws VTNException  An error occurred.
     */
    @Override
    public VlanMap getVlanMap(VBridgePath path, String mapId)
        throws VTNException {
        String tname = VTenantUtils.getName(path);
        String bname = path.getBridgeName();
        VlanMapIdentifier ident =
            VlanMapIdentifier.create(tname, bname, toMdVlanMapId(mapId));
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            return toVlanMap(ident.fetch(ctx.getTransaction()));
        } finally {
            ctx.cancelTransaction();
        }
    }

    /**
     * Return information about the VLAN mapping which matches the specified
     * VLAN mapping configuration in the specified virtual L2 bridge.
     *
     * @param path    Path to the bridge.
     * @param vlconf  VLAN mapping configuration.
     * @return  Information about the VLAN mapping which matches the specified
     *          VLAN mapping configuration.
     * @throws VTNException  An error occurred.
     */
    @Override
    public VlanMap getVlanMap(VBridgePath path, VlanMapConfig vlconf)
        throws VTNException {
        String tname = VTenantUtils.getName(path);
        String bname = path.getBridgeName();
        VTNManagerProvider provider = checkService();
        checkVlanMapConfig(vlconf);

        // Create a input for get-vlan-map RPC.
        GetVlanMapInputBuilder builder = new GetVlanMapInputBuilder().
            setTenantName(tname).
            setBridgeName(bname);

        Node node = vlconf.getNode();
        SalNode snode = SalNode.create(node);
        if (snode != null) {
            builder.setNode(snode.getNodeId());
        }

        short vid = vlconf.getVlan();
        try {
            builder.setVlanId(new VlanId((int)vid));
        } catch (RuntimeException e) {
            String info = "node=" + snode + ", vid=" + vid;
            throw getVlanMapNotFoundException(info);
        }

        // Invoke RPC and await its completion.
        VtnVlanMapService rpc =
            provider.getVtnRpcService(VtnVlanMapService.class);
        GetVlanMapInput input = builder.build();
        GetVlanMapOutput output = getRpcOutput(rpc.getVlanMap(input));
        String mapId = output.getMapId();
        if (mapId == null) {
            String info = "node=" + snode + ", vid=" + vid;
            throw getVlanMapNotFoundException(info);
        }

        return new VlanMap(toAdVlanMapId(mapId), node, vid);
    }

    /**
     * Add a new VLAN mapping to the virtual L2 bridge.
     *
     * @param path    Path to the bridge.
     * @param vlconf  VLAN mapping configuration.
     * @return  Information about added VLAN mapping, which includes
     *          VLAN map identifier.
     * @throws VTNException  An error occurred.
     */
    @Override
    public VlanMap addVlanMap(VBridgePath path, VlanMapConfig vlconf)
        throws VTNException {
        String tname = VTenantUtils.getName(path);
        String bname = path.getBridgeName();
        VTNManagerProvider provider = checkService();
        checkVlanMapConfig(vlconf);

        // Create a input for add-vlan-map RPC.
        AddVlanMapInputBuilder builder = new AddVlanMapInputBuilder().
            setTenantName(tname).
            setBridgeName(bname);

        Node node = vlconf.getNode();
        SalNode snode = SalNode.create(node);
        if (snode != null) {
            builder.setNode(snode.getNodeId());
        } else if (node != null) {
            // Unsupported node.
            NodeUtils.checkNode(node);
        }

        short vid = vlconf.getVlan();
        try {
            builder.setVlanId(new VlanId((int)vid));
        } catch (RuntimeException e) {
            throw RpcException.getBadArgumentException(
                "Invalid VLAN ID: " + vid, e);
        }

        // Invoke RPC and await its completion.
        VtnVlanMapService rpc =
            provider.getVtnRpcService(VtnVlanMapService.class);
        AddVlanMapInput input = builder.build();
        AddVlanMapOutput output = getRpcOutput(rpc.addVlanMap(input));
        return new VlanMap(toAdVlanMapId(output.getMapId()), node, vid);
    }

    /**
     * Remove the VLAN mapping from the virtual L2 bridge.
     *
     * @param path   Path to the bridge.
     * @param mapId  The identifier of the VLAN mapping.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status removeVlanMap(VBridgePath path, String mapId) {
        Status status;
        try {
            String tname = VTenantUtils.getName(path);
            String bname = path.getBridgeName();
            VTNManagerProvider provider = checkService();

            // Ensure that the mapping ID is not null.
            String mid = toMdVlanMapId(mapId);
            VlanMapIdentifier.create(tname, bname, mid);

            // Create a input for remove-vlan-map RPC.
            RemoveVlanMapInput input = new RemoveVlanMapInputBuilder().
                setTenantName(tname).
                setBridgeName(bname).
                setMapIds(Collections.singletonList(mid)).
                build();

            // Invoke RPC and await its completion.
            VtnVlanMapService rpc =
                provider.getVtnRpcService(VtnVlanMapService.class);
            RemoveVlanMapOutput output = getRpcOutput(rpc.removeVlanMap(input));
            RemoveVlanMapResult result =
                getRpcOutput(output.getRemoveVlanMapResult(), 0, false);
            if (!mid.equals(result.getMapId())) {
                throw new VTNException("Unexpected map ID in RPC output: " +
                                       output.getRemoveVlanMapResult());
            }
            if (result.getStatus() == null) {
                throw getVlanMapNotFoundException("id=" + mapId);
            }

            status = new Status(StatusCode.SUCCESS, null);
        } catch (VTNException e) {
            status = e.getStatus();
        }

        return status;
    }

    /**
     * Return the port mapping configuration applied to the specified vBridge
     * interface.
     *
     * @param path  Path to the vBridge interface.
     * @return  Port mapping information. {@code null} is returned if the
     *          port mapping is not configured on the specified interface.
     * @throws VTNException  An error occurred.
     */
    @Override
    public PortMap getPortMap(VBridgeIfPath path) throws VTNException {
        String tname = VTenantUtils.getName(path);
        String bname = path.getBridgeName();
        String iname = path.getInterfaceName();
        VBridgeIfIdentifier ident =
            VBridgeIfIdentifier.create(tname, bname, iname, true);
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            return getPortMap(ident.fetch(ctx.getTransaction()));
        } finally {
            ctx.cancelTransaction();
        }
    }

    /**
     * Return the port mapping configuration applied to the specified
     * vTerminal interface.
     *
     * @param path  Path to the vTerminal interface.
     * @return  Port mapping information. {@code null} is returned if the
     *          port mapping is not configured on the specified interface.
     * @throws VTNException  An error occurred.
     */
    @Override
    public PortMap getPortMap(VTerminalIfPath path) throws VTNException {
        String tname = VTenantUtils.getName(path);
        String bname = path.getTerminalName();
        String iname = path.getInterfaceName();
        VTerminalIfIdentifier ident =
            VTerminalIfIdentifier.create(tname, bname, iname, true);
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            return getPortMap(ident.fetch(ctx.getTransaction()));
        } finally {
            ctx.cancelTransaction();
        }
    }

    /**
     * Create or destroy mapping between the physical switch port and the
     * vBridge interface.
     *
     * @param path    Path to the vBridge interface.
     * @param pmconf  Port mapping configuration to be set.
     *                If {@code null} is specified, port mapping on the
     *                specified interface is destroyed.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status setPortMap(VBridgeIfPath path, PortMapConfig pmconf) {
        Status status;
        try {
            VTNManagerProvider provider = checkService();
            SetPortMapInput input = createInput(path, pmconf);
            if (pmconf == null) {
                // Remove port mapping.
                removePortMap(provider, input);
            } else {
                // Invoke RPC and await its completion.
                VtnPortMapService rpc =
                    provider.getVtnRpcService(VtnPortMapService.class);
                getRpcOutput(rpc.setPortMap(input));
            }
            status = new Status(StatusCode.SUCCESS, null);
        } catch (VTNException e) {
            status = e.getStatus();
        }

        return status;
    }

    /**
     * Create or destroy mapping between the physical switch port and the
     * vTerminal interface.
     *
     * @param path    Path to the vTerminal.
     * @param pmconf  Port mapping configuration to be set.
     *                If {@code null} is specified, port mapping on the
     *                specified interface is destroyed.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status setPortMap(VTerminalIfPath path, PortMapConfig pmconf) {
        Status status;
        try {
            VTNManagerProvider provider = checkService();
            SetPortMapInput input = createInput(path, pmconf);
            if (pmconf == null) {
                // Remove port mapping.
                removePortMap(provider, input);
            } else {
                // Invoke RPC and await its completion.
                VtnPortMapService rpc =
                    provider.getVtnRpcService(VtnPortMapService.class);
                getRpcOutput(rpc.setPortMap(input));
            }
            status = new Status(StatusCode.SUCCESS, null);
        } catch (VTNException e) {
            status = e.getStatus();
        }

        return status;
    }

    /**
     * Return information about the MAC mapping configured in the specified
     * vBridge.
     *
     * @param path   Path to the bridge.
     * @return  A {@link MacMap} object which represents information about
     *          the MAC mapping specified by {@code path}.
     *          {@code null} is returned if the MAC mapping is not configured
     *          in the specified vBridge.
     * @throws VTNException  An error occurred.
     */
    @Override
    public MacMap getMacMap(VBridgePath path) throws VTNException {
        String name = VTenantUtils.getName(path);
        VBridgeIdentifier ident =
            VBridgeIdentifier.create(name, path.getBridgeName(), true);
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            ReadTransaction rtx = ctx.getTransaction();
            Vbridge vbridge = ident.fetch(rtx);
            return toMacMap(rtx, ident, vbridge.getMacMap());
        } finally {
            ctx.cancelTransaction();
        }
    }

    /**
     * Return configuration information about MAC mapping in the specified
     * vBridge.
     *
     * @param path     Path to the vBridge.
     * @param aclType  The type of access control list.
     * @return  A set of {@link DataLinkHost} instances which contains host
     *          information in the specified access control list is returned.
     *          {@code null} is returned if MAC mapping is not configured in
     *          the specified vBridge.
     * @throws VTNException  An error occurred.
     */
    @Override
    public Set<DataLinkHost> getMacMapConfig(VBridgePath path,
                                             VtnAclType aclType)
        throws VTNException {
        String name = VTenantUtils.getName(path);
        VBridgeIdentifier ident =
            VBridgeIdentifier.create(name, path.getBridgeName(), true);
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            ReadTransaction rtx = ctx.getTransaction();
            Vbridge vbridge = ident.fetch(rtx);
            return getMacMapConfig(vbridge.getMacMap(), aclType);
        } finally {
            ctx.cancelTransaction();
        }
    }

    /**
     * Return a list of {@link MacAddressEntry} instances corresponding to
     * all the MAC address information actually mapped by MAC mapping
     * configured in the specified vBridge.
     *
     * @param path  Path to the vBridge.
     * @return  A list of {@link MacAddressEntry} instances corresponding to
     *          all the MAC address information actually mapped to the vBridge
     *          specified by {@code path}.
     *          {@code null} is returned if MAC mapping is not configured
     *          in the specified vBridge.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<MacAddressEntry> getMacMappedHosts(VBridgePath path)
        throws VTNException {
        VTNManagerProvider provider = checkService();
        GetMacMappedHostInput input = createInputBuilder(path).build();

        // Invoke RPC and await its completion.
        VtnMacMapService rpc =
            provider.getVtnRpcService(VtnMacMapService.class);
        GetMacMappedHostOutput output =
            getRpcOutput(rpc.getMacMappedHost(input));

        List<MacAddressEntry> mapped;
        if (!Boolean.TRUE.equals(output.isConfigured())) {
            // MAC mapping is not configured.
            mapped = null;
        } else {
            mapped = new ArrayList<>();
            List<MacMappedHost> hosts = output.getMacMappedHost();
            if (hosts != null) {
                for (MacMappedHost host: hosts) {
                    mapped.add(VBridgeManager.toMacAddressEntry(host));
                }
            }
        }

        return mapped;
    }

    /**
     * Determine whether the host specified by the MAC address is actually
     * mapped by MAC mapping configured in the specified vBridge.
     *
     * @param path  Path to the vBridge.
     * @param addr  A {@link DataLinkAddress} instance which represents the
     *              MAC address.
     * @return  A {@link MacAddressEntry} instancw which represents information
     *          about the host corresponding to {@code addr} is returned
     *          if it is actually mapped to the specified vBridge by MAC
     *          mapping.
     *          {@code null} is returned if the MAC address specified by
     *          {@code addr} is not mapped by MAC mapping, or MAC mapping is
     *          not configured in the specified vBridge.
     * @throws VTNException  An error occurred.
     */
    @Override
    public MacAddressEntry getMacMappedHost(VBridgePath path,
                                            DataLinkAddress addr)
        throws VTNException {
        VTNManagerProvider provider = checkService();
        GetMacMappedHostInputBuilder builder = createInputBuilder(path);
        MacAddress mac = toMacAddress(addr);
        MacAddressEntry ment;
        if (mac == null) {
            // Unsupport address type.
            ment = null;
        } else {
            List<MacAddress> maddrs = Collections.singletonList(mac);
            GetMacMappedHostInput input = builder.
                setMacAddresses(maddrs).build();

            // Invoke RPC and await its completion.
            VtnMacMapService rpc =
                provider.getVtnRpcService(VtnMacMapService.class);
            GetMacMappedHostOutput output =
                getRpcOutput(rpc.getMacMappedHost(input));
            if (!Boolean.TRUE.equals(output.isConfigured())) {
                // MAC mapping is not configured.
                ment = null;
            } else {
                List<MacMappedHost> hosts = output.getMacMappedHost();
                ment = (MiscUtils.isEmpty(hosts))
                    ? null
                    : VBridgeManager.toMacAddressEntry(hosts.get(0));
            }
        }

        return ment;
    }

    /**
     * Change MAC mapping configuration as specified by {@link MacMapConfig}
     * instance.
     *
     * @param path    A {@link VBridgePath} object that specifies the position
     *                of the vBridge.
     * @param op      A {@link VtnUpdateOperationType} instance which indicates
     *                how to change the MAC mapping configuration.
     * @param mcconf  A {@link MacMapConfig} instance which contains the MAC
     *                mapping configuration information.
     * @return        A {@link UpdateType} object which represents the result
     *                of the operation is returned.
     *                {@code null} is returned if the configuration was not
     *                changed.
     * @throws VTNException  An error occurred.
     */
    @Override
    public UpdateType setMacMap(VBridgePath path, VtnUpdateOperationType op,
                                MacMapConfig mcconf) throws VTNException {
        VTNManagerProvider provider = checkService();
        String tname = VTenantUtils.getName(path);
        Set<DataLinkHost> allowed;
        Set<DataLinkHost> denied;
        if (mcconf == null) {
            allowed = null;
            denied = null;
        } else {
            allowed = mcconf.getAllowedHosts();
            denied = mcconf.getDeniedHosts();
        }

        if (op == null) {
            // Only for backward compatibility.
            throw RpcException.getNullArgumentException("Operation");
        }

        SetMacMapInput input = new SetMacMapInputBuilder().
            setTenantName(tname).
            setBridgeName(path.getBridgeName()).
            setOperation(op).
            setAllowedHosts(toVlanHostDescs(allowed)).
            setDeniedHosts(toVlanHostDescs(denied)).
            build();

        // Invoke RPC and await its completion.
        VtnMacMapService rpc =
            provider.getVtnRpcService(VtnMacMapService.class);
        SetMacMapOutput output = getRpcOutput(rpc.setMacMap(input));

        return MiscUtils.toUpdateType(output.getStatus());
    }

    /**
     * Change the access controll list for the specified MAC mapping.
     *
     * @param path      A {@link VBridgePath} object that specifies the
     *                  position of the vBridge.
     * @param op        A {@link VtnUpdateOperationType} instance which
     *                  indicates how to change the MAC mapping configuration.
     * @param aclType   The type of access control list.
     * @param dlhosts   A set of {@link DataLinkHost} instances.
     * @return          A {@link UpdateType} object which represents the result
     *                  of the operation is returned.
     *                  {@code null} is returned if the configuration was not
     *                  changed.
     * @throws VTNException  An error occurred.
     */
    @Override
    public UpdateType setMacMap(VBridgePath path, VtnUpdateOperationType op,
                                VtnAclType aclType,
                                Set<? extends DataLinkHost> dlhosts)
        throws VTNException {
        VTNManagerProvider provider = checkService();
        String tname = VTenantUtils.getName(path);

        if (op == null) {
            // Only for backward compatibility.
            throw RpcException.getNullArgumentException("Operation");
        }
        if (aclType == null) {
            // Only for backward compatibility.
            throw RpcException.getNullArgumentException("ACL type");
        }

        SetMacMapAclInput input = new SetMacMapAclInputBuilder().
            setTenantName(tname).
            setBridgeName(path.getBridgeName()).
            setAclType(aclType).
            setOperation(op).
            setHosts(toVlanHostDescs(dlhosts)).
            build();

        // Invoke RPC and await its completion.
        VtnMacMapService rpc =
            provider.getVtnRpcService(VtnMacMapService.class);
        SetMacMapAclOutput output = getRpcOutput(rpc.setMacMapAcl(input));

        return MiscUtils.toUpdateType(output.getStatus());
    }

    /**
     * Initiate the discovery of a host base on its IP address.
     *
     * <p>
     *   If the given IP address is an IPv4 address, this method sends
     *   a broadcast ARP request to the specified virtual L2 bridges.
     *   If a host is found, it is reported to {@code HostTracker} via
     *   {@code IfHostListener}.
     * </p>
     *
     * @param addr     IP address.
     * @param pathSet  A set of destination paths of virtual L2 bridges.
     *                 If {@code null} is specified, a ARP request is sent
     *                 to all existing bridges.
     */
    @Override
    public void findHost(InetAddress addr, Set<VBridgePath> pathSet) {
        // Not implemented.
    }

    /**
     * Send a unicast ARP request to the specified host.
     *
     * <p>
     *   If the specified host sends a ARP reply, it is reported to
     *   {@code HostTracker} via {@code IfHostListener}.
     * </p>
     *
     * @param host  A host to be probed.
     * @return  {@code true} is returned if an ARP request was sent.
     *          Otherwise {@code false} is returned.
     */
    @Override
    public boolean probeHost(HostNodeConnector host) {
        // Not implemented.
        return false;
    }

    /**
     * Return a list of MAC address entries learned by the specified virtual
     * L2 bridge.
     *
     * @param path  Path to the bridge.
     * @return  A list of MAC address entries.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<MacAddressEntry> getMacEntries(VBridgePath path)
        throws VTNException {
        return VBridgeManager.getMacEntries(checkService(), path);
    }

    /**
     * Search for a MAC address entry from the MAC address table in the
     * specified virtual L2 bridge.
     *
     * @param path  Path to the bridge.
     * @param addr  MAC address.
     * @return  A MAC address entry associated with the specified MAC address.
     *          {@code null} is returned if not found.
     * @throws VTNException  An error occurred.
     */
    @Override
    public MacAddressEntry getMacEntry(VBridgePath path, DataLinkAddress addr)
        throws VTNException {
        return VBridgeManager.getMacEntry(
            checkService(), path, toMacAddress(addr));
    }

    /**
     * Remove a MAC address entry from the MAC address table in the virtual L2
     * bridge.
     *
     * @param path  Path to the bridge.
     * @param addr  Ethernet address.
     * @return  A MAC address entry actually removed is returned.
     *          {@code null} is returned if not found.
     * @throws VTNException  An error occurred.
     */
    @Override
    public MacAddressEntry removeMacEntry(VBridgePath path,
                                          DataLinkAddress addr)
        throws VTNException {
        String tname = VTenantUtils.getName(path);
        MacAddress mac = toMacAddress(addr);
        if (mac == null) {
            // Unsupport address type.
            return null;
        }

        // Read the specified MAC address table entry.
        VTNManagerProvider provider = checkService();
        MacAddressEntry ment = VBridgeManager.getMacEntry(provider, path, mac);
        if (ment != null) {
            List<MacAddress> maddrs = Collections.singletonList(mac);
            RemoveMacEntryInput input = new RemoveMacEntryInputBuilder().
                setTenantName(tname).
                setBridgeName(path.getBridgeName()).
                setMacAddresses(maddrs).
                build();

            // Invoke RPC and await its completion.
            VtnMacTableService rpc =
                provider.getVtnRpcService(VtnMacTableService.class);
            RemoveMacEntryOutput output =
                getRpcOutput(rpc.removeMacEntry(input));
            RemoveMacEntryResult result =
                getRpcOutput(output.getRemoveMacEntryResult(), 0, false);
            if (!mac.equals(result.getMacAddress())) {
                throw new VTNException(
                    "Unexpected MAC address in RPC output: " +
                    output.getRemoveMacEntryResult());
            }
            if (result.getStatus() == null) {
                ment = null;
            }
        }

        return ment;
    }

    /**
     * Flush all MAC address table entries in the specified virtual L2 bridge.
     *
     * @param path  Path to the bridge.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status flushMacEntries(VBridgePath path) {
        Status status;
        try {
            String tname = VTenantUtils.getName(path);
            VTNManagerProvider provider = checkService();
            RemoveMacEntryInput input = new RemoveMacEntryInputBuilder().
                setTenantName(tname).
                setBridgeName(path.getBridgeName()).
                build();

            // Invoke RPC and await its completion.
            VtnMacTableService rpc =
                provider.getVtnRpcService(VtnMacTableService.class);
            getRpcOutput(rpc.removeMacEntry(input));
            status = new Status(StatusCode.SUCCESS, null);
        } catch (VTNException e) {
            status = e.getStatus();
        }

        return status;
    }

    /**
     * Return information about all data flows present in the specified VTN.
     *
     * @param path      A {@link VTenantPath} object that specifies the
     *                  position of the VTN.
     * @param mode      A {@link DataFlowMode} instance which specifies
     *                  behavior of this method.
     * @param filter    If a {@link DataFlowFilter} instance is specified,
     *                  only data flows that meet the condition specified by
     *                  {@link DataFlowFilter} instance is returned.
     *                  All data flows in the VTN is returned if {@code null}
     *                  is specified.
     * @param interval  Time interval in seconds for retrieving the average
     *                  statistics.
     * @return  A list of {@link DataFlow} instances which represents
     *          information about data flows.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<DataFlow> getDataFlows(VTenantPath path, DataFlowMode mode,
                                       DataFlowFilter filter, int interval)
        throws VTNException {
        VTNManagerProvider provider = checkService();

        // Construct an RPC input that obtains a list of data flows.
        VnodeName vname = VTenantUtils.getVnodeName(path);
        GetDataFlowInputBuilder builder = new GetDataFlowInputBuilder().
            setTenantName(vname.getValue()).
            setMode(mode).
            setAverageInterval(interval);

        if (filter != null && !setInput(builder, filter)) {
            // Invalid filter condition is specified.
            // Thus no data flow should be selected.
            return Collections.<DataFlow>emptyList();
        }

        // Invoke RPC and await its completion.
        List<DataFlowInfo> result = getDataFlows(provider, builder.build());

        // Convert the result.
        List<DataFlow> list = new ArrayList<>(result.size());
        for (DataFlowInfo dfi: result) {
            list.add(FlowUtils.toDataFlow(dfi, mode));
        }

        return list;
    }

    /**
     * Return information about the specified data flow in the VTN.
     *
     * @param path      A {@link VTenantPath} object that specifies the position
     *                  of the VTN.
     * @param flowId    An identifier of the data flow.
     * @param mode      A {@link DataFlowMode} instance which specifies
     *                  behavior of this method.
     * @param interval  Time interval in seconds for retrieving the average
     *                  statistics.
     * @return  A {@link DataFlow} instance which represents information
     *          about the specified data flow.
     *          {@code null} is returned if the specified data flow was not
     *          found.
     * @throws VTNException  An error occurred.
     */
    @Override
    public DataFlow getDataFlow(VTenantPath path, long flowId,
                                DataFlowMode mode, int interval)
        throws VTNException {
        VTNManagerProvider provider = checkService();

        // Construct an RPC input that obtains information about the specified
        // data flow.
        VnodeName vname = VTenantUtils.getVnodeName(path);
        VtnFlowId fid = new VtnFlowId(NumberUtils.getUnsigned(flowId));
        GetDataFlowInput input = new GetDataFlowInputBuilder().
            setTenantName(vname.getValue()).
            setMode(mode).
            setAverageInterval(interval).
            setFlowId(fid).
            build();

        // Invoke RPC and await its completion.
        List<DataFlowInfo> result = getDataFlows(provider, input);

        // Convert the result.
        if (result.size() > 1) {
            // This should never happen.
            throw new VTNException("Unexpected data flow list: " + result);
        }

        return (result.isEmpty())
            ? null
            : FlowUtils.toDataFlow(result.get(0), mode);
    }

    /**
     * Return the number of data flows present in the specified VTN.
     *
     * @param path  A {@link VTenantPath} object that specifies the position
     *              of the VTN.
     * @return  The number of data flows present in the specified VTN.
     * @throws VTNException  An error occurred.
     */
    @Override
    public int getDataFlowCount(VTenantPath path) throws VTNException {
        VTNManagerProvider provider = checkService();

        // Construct an RPC input that obtains the number of data flows.
        VnodeName vname = VTenantUtils.getVnodeName(path);
        GetDataFlowCountInput input = new GetDataFlowCountInputBuilder().
            setTenantName(vname.getValue()).
            build();

        // Invoke RPC and await its completion.
        VtnFlowService rpc = provider.getVtnRpcService(VtnFlowService.class);
        GetDataFlowCountOutput output =
            getRpcOutput(rpc.getDataFlowCount(input));
        Integer count = output.getCount();
        if (count == null) {
            throw new VTNException("Flow count is unavailable.");
        }

        return count.intValue();
    }

    /**
     * Return a list of flow conditions configured in the container.
     *
     * @return  A list of {@link FlowCondition} instances corresponding to
     *          all flow conditions configured in the container.
     *          An empty list is returned if no flow condition is configured.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<FlowCondition> getFlowConditions() throws VTNException {
        ArrayList<FlowCondition> list = new ArrayList<>();
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            ReadTransaction rtx = ctx.getTransaction();
            List<VTNFlowCondition> vlist =
                FlowCondUtils.readFlowConditions(rtx);
            for (VTNFlowCondition vfcond: vlist) {
                list.add(vfcond.toFlowCondition());
            }
        } finally {
            ctx.cancelTransaction();
        }

        // Sort flow conditions by their name.
        VTNIdentifiableComparator<String> comparator =
            new VTNIdentifiableComparator<>(String.class);
        Collections.sort(list, comparator);

        return list;
    }

    /**
     * Return information about the flow condition specified by the name.
     *
     * @param name  The name of the flow condition.
     * @return  A {@link FlowCondition} instance which represents information
     *          about the flow condition specified by {@code name}.
     * @throws VTNException  An error occurred.
     */
    @Override
    public FlowCondition getFlowCondition(String name) throws VTNException {
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            ReadTransaction rtx = ctx.getTransaction();
            VTNFlowCondition vfcond =
                FlowCondUtils.readFlowCondition(rtx, name);
            return vfcond.toFlowCondition();
        } finally {
            ctx.cancelTransaction();
        }
    }

    /**
     * Create or modify the flow condition.
     *
     * @param name   The name of the flow condition.
     * @param fcond  A {@link FlowCondition} instance which specifies the
     *               configuration of the flow condition.
     * @return  A {@link UpdateType} object which represents the result of the
     *          operation is returned.
     * @throws VTNException  An error occurred.
     */
    @Override
    public UpdateType setFlowCondition(String name, FlowCondition fcond)
        throws VTNException {
        VTNManagerProvider provider = checkService();

        // Construct an RPC input that replaces the flow condition specified
        // by the given name.
        VTNFlowCondition vfcond = new VTNFlowCondition(name, fcond);
        SetFlowConditionInput input = vfcond.toSetFlowConditionInputBuilder().
            setOperation(VtnUpdateOperationType.SET).build();

        // Invoke RPC and await its completion.
        VtnFlowConditionService rpc =
            provider.getVtnRpcService(VtnFlowConditionService.class);
        SetFlowConditionOutput output =
            getRpcOutput(rpc.setFlowCondition(input));

        // Convert the result.
        return MiscUtils.toUpdateType(output.getStatus());
    }

    /**
     * Remove the flow condition specified by the name.
     *
     * @param name  The name of the flow condition to be removed.
     * @return  A {@link Status} object which represents the result of the
     *          operation is returned.
     */
    @Override
    public Status removeFlowCondition(String name) {
        try {
            VTNManagerProvider provider = checkService();
            RemoveFlowConditionInput input =
                new RemoveFlowConditionInputBuilder().setName(name).build();

            // Invoke RPC and await its completion.
            VtnFlowConditionService rpc =
                provider.getVtnRpcService(VtnFlowConditionService.class);
            getRpcOutput(rpc.removeFlowCondition(input), true);
        } catch (VTNException e) {
            return e.getStatus();
        }

        return new Status(StatusCode.SUCCESS, null);
    }

    /**
     * Remove all the flow conditions.
     *
     * @return  A {@link Status} object which represents the result of the
     *          operation is returned.
     */
    @Override
    public Status clearFlowCondition() {
        try {
            VTNManagerProvider provider = checkService();

            // Invoke RPC and await its completion.
            VtnFlowConditionService rpc =
                provider.getVtnRpcService(VtnFlowConditionService.class);
            ClearFlowConditionOutput output =
                getRpcOutput(rpc.clearFlowCondition());
            if (output.getStatus() == null) {
                // The flow condition container is empty.
                return null;
            }
        } catch (VTNException e) {
            return e.getStatus();
        }

        return new Status(StatusCode.SUCCESS, null);
    }

    /**
     * Return a {@link FlowMatch} instance configured in the flow condition
     * specified by the flow condition name and match index.
     *
     * @param name   The name of the flow condition.
     * @param index  The match index that specifies flow match condition
     *               in the flow condition.
     * @return  A {@link FlowMatch} instance which represents a flow match
     *          condition.
     *          {@code null} is returned if no flow match condition is
     *          configured at the specified match index.
     * @throws VTNException  An error occurred.
     */
    @Override
    public FlowMatch getFlowConditionMatch(String name, int index)
        throws VTNException {
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            ReadTransaction rtx = ctx.getTransaction();
            VTNFlowMatch vfmatch =
                FlowCondUtils.readFlowMatch(rtx, name, index);
            return (vfmatch == null) ? null : vfmatch.toFlowMatch();
        } finally {
            ctx.cancelTransaction();
        }
    }

    /**
     * Configure a flow match condition into the flow condition specified
     * by the flow condition name and match index.
     *
     * @param name   The name of the flow condition.
     * @param index  The match index that specifies flow match condition in
     *               the flow condition.
     * @param match  A {@link FlowMatch} instance which represents a flow
     *               match condition to be configured.
     * @return  A {@link UpdateType} object which represents the result of the
     *          operation is returned.
     * @throws VTNException  An error occurred.
     */
    @Override
    public UpdateType setFlowConditionMatch(String name, int index,
                                            FlowMatch match)
        throws VTNException {
        VTNManagerProvider provider = checkService();

        // Complete FlowMatch instance.
        FlowMatch mt;
        if (match == null) {
            // Create an empty flow match.
            mt = new FlowMatch(index, null, null, null);
        } else {
            // Ensure that the match index is assigned.
            mt = match.assignIndex(index);
        }

        VTNFlowMatch vfmatch = new VTNFlowMatch(mt);
        List<FlowMatchList> fml =
            Collections.singletonList(vfmatch.toFlowMatchListBuilder().build());

        // Construct an RPC input that adds the given flow match configuration
        // to the specified flow condition.
        SetFlowConditionMatchInput input = new SetFlowConditionMatchInputBuilder().
            setName(name).setFlowMatchList(fml).build();

        // Invoke RPC and await its completion.
        VtnFlowConditionService rpc =
            provider.getVtnRpcService(VtnFlowConditionService.class);
        SetFlowConditionMatchOutput output =
            getRpcOutput(rpc.setFlowConditionMatch(input));
        SetMatchResult result =
            getRpcOutput(output.getSetMatchResult(), 0, false);
        Integer idx = vfmatch.getIdentifier();
        if (!idx.equals(result.getIndex())) {
            throw new VTNException("Unexpected match index in RPC output: " +
                                   output.getSetMatchResult());
        }

        // Convert the result.
        return MiscUtils.toUpdateType(result.getStatus());
    }

    /**
     * Remove the flow match condition specified by the flow condition name
     * and match index.
     *
     * @param name   The name of the flow condition.
     * @param index  The match index that specifies flow match condition
     *               in the flow condition.
     * @return  A {@link Status} object which represents the result of the
     *          operation is returned.
     */
    @Override
    public Status removeFlowConditionMatch(String name, int index) {
        Integer idx = Integer.valueOf(index);
        try {
            VTNManagerProvider provider = checkService();

            // Construct an RPC input that removes the flow match associated
            // with the given index in the flow condition.
            RemoveFlowConditionMatchInput input =
                new RemoveFlowConditionMatchInputBuilder().
                setName(name).setMatchIndex(Collections.singletonList(idx)).
                build();

            // Invoke RPC and await its completion.
            VtnFlowConditionService rpc =
                provider.getVtnRpcService(VtnFlowConditionService.class);
            RemoveFlowConditionMatchOutput output =
                getRpcOutput(rpc.removeFlowConditionMatch(input));
            RemoveMatchResult result =
                getRpcOutput(output.getRemoveMatchResult(), 0, false);
            if (!idx.equals(result.getIndex())) {
                throw new VTNException(
                    "Unexpected match index in RPC output: " +
                    output.getRemoveMatchResult());
            }
            if (result.getStatus() == null) {
                // The specified match index was not found.
                return null;
            }
        } catch (VTNException e) {
            return e.getStatus();
        }

        return new Status(StatusCode.SUCCESS, null);
    }

    /**
     * Return a list of path policy identifiers present in the container.
     *
     * @return  A list of {@link Integer} instances corresponding to all the
     *          path policies present in the container.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<Integer> getPathPolicyIds() throws VTNException {
        List<Integer> list = new ArrayList<>();
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            ReadTransaction rtx = ctx.getTransaction();
            for (VtnPathPolicy vpp: PathPolicyUtils.readVtnPathPolicies(rtx)) {
                list.add(vpp.getId());
            }
        } finally {
            ctx.cancelTransaction();
        }

        // Sort path policy IDs.
        Collections.sort(list);
        return list;
    }

    /**
     * Return the configuration of the specified path policy.
     *
     * @param id  The identifier of the path policy.
     * @return  A {@link PathPolicy} instance which contains the configuration
     *          of the specified path policy.
     * @throws VTNException  An error occurred.
     */
    @Override
    public PathPolicy getPathPolicy(int id) throws VTNException {
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            ReadTransaction rtx = ctx.getTransaction();
            VtnPathPolicy vpp = PathPolicyUtils.readVtnPathPolicy(rtx, id);
            return PathPolicyUtils.toPathPolicy(vpp);
        } finally {
            ctx.cancelTransaction();
        }
    }

    /**
     * Create or modify the path policy.
     *
     * @param id      The identifier of the path policy.
     * @param policy  A {@link PathPolicy} instance which specifies the
     *                configuration of the path policy.
     * @return  A {@link UpdateType} object which represents the result of the
     *          operation is returned.
     * @throws VTNException  An error occurred.
     */
    @Override
    public UpdateType setPathPolicy(int id, PathPolicy policy)
        throws VTNException {
        VTNManagerProvider provider = checkService();

        // Construct an RPC input that replaces the path policy configuration
        // with the given onfiguration.
        Integer pid = Integer.valueOf(id);
        SetPathPolicyInput input = new PathPolicyConfigBuilder.Rpc().
            set(policy, pid).getBuilder().
            setOperation(VtnUpdateOperationType.SET).build();

        // Invoke RPC and await its completion.
        VtnPathPolicyService rpc =
            provider.getVtnRpcService(VtnPathPolicyService.class);
        SetPathPolicyOutput output = getRpcOutput(rpc.setPathPolicy(input));

        // Convert the result.
        return MiscUtils.toUpdateType(output.getStatus());
    }

    /**
     * Remove the path policy specified by the identifier.
     *
     * @param id  The identifier of the path policy to be removed.
     * @return  A {@link Status} object which represents the result of the
     *          operation is returned.
     */
    @Override
    public Status removePathPolicy(int id) {
        try {
            VTNManagerProvider provider = checkService();
            RemovePathPolicyInput input = new RemovePathPolicyInputBuilder().
                setId(Integer.valueOf(id)).build();

            // Invoke RPC and await its completion.
            VtnPathPolicyService rpc =
                provider.getVtnRpcService(VtnPathPolicyService.class);
            getRpcOutput(rpc.removePathPolicy(input), true);
        } catch (VTNException e) {
            return e.getStatus();
        }

        return new Status(StatusCode.SUCCESS, null);
    }

    /**
     * Remove all the path policies.
     *
     * @return  A {@link Status} object which represents the result of the
     *          operation is returned.
     */
    @Override
    public Status clearPathPolicy() {
        try {
            VTNManagerProvider provider = checkService();

            // Invoke RPC and await its completion.
            VtnPathPolicyService rpc =
                provider.getVtnRpcService(VtnPathPolicyService.class);
            ClearPathPolicyOutput output = getRpcOutput(rpc.clearPathPolicy());
            if (output.getStatus() == null) {
                // The path policy container is empty.
                return null;
            }
        } catch (VTNException e) {
            return e.getStatus();
        }

        return new Status(StatusCode.SUCCESS, null);
    }

    /**
     * Return the default link cost configured in the specified path policy.
     *
     * @param id  The identifier of the path policy.
     * @return    The default link cost configured in the specified path policy
     *            is returned. {@link PathPolicy#COST_UNDEF} means that the
     *            default cost should be determined by link speed.
     * @throws VTNException  An error occurred.
     */
    @Override
    public long getPathPolicyDefaultCost(int id) throws VTNException {
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            ReadTransaction rtx = ctx.getTransaction();
            VtnPathPolicy vpp = PathPolicyUtils.readVtnPathPolicy(rtx, id);
            Long c = vpp.getDefaultCost();
            return (c == null) ? PathPolicy.COST_UNDEF : c.longValue();
        } finally {
            ctx.cancelTransaction();
        }
    }

    /**
     * Change the default link cost for the specified path policy.
     *
     * @param id    The identifier of the path policy.
     * @param cost  The default cost value to be set.
     * @return  {@code true} if the default cost was changed.
     *          {@code false} if not changed.
     * @throws VTNException  An error occurred.
     */
    @Override
    public boolean setPathPolicyDefaultCost(int id, long cost)
        throws VTNException {
        VTNManagerProvider provider = checkService();

        // Construct an RPC input that updates only the default cost.
        PathPolicyConfigBuilder.Rpc builder =
            PathPolicyUtils.createRpcInput(id);
        SetPathPolicyInput input = builder.setDefaultCost(Long.valueOf(cost)).
            getBuilder().build();

        // Invoke RPC and await its completion.
        VtnPathPolicyService rpc =
            provider.getVtnRpcService(VtnPathPolicyService.class);
        SetPathPolicyOutput output = getRpcOutput(rpc.setPathPolicy(input));

        // Convert the result.
        return (output.getStatus() != null);
    }

    /**
     * Return the cost of transmitting a packet from the specified switch port
     * configured in the specified path policy.
     *
     * @param id    The identifier of the path policy.
     * @param ploc  A {@link PortLocation} instance which specifies the
     *              location of the physical switch port.
     * @return  The cost of transmitting a packet from the specified physical
     *          switch port. Zero is returned if {@code ploc} is not configured
     *          in the specified path policy.
     * @throws VTNException  An error occurred.
     */
    @Override
    public long getPathPolicyCost(int id, PortLocation ploc)
        throws VTNException {
        VtnPortDesc vdesc;
        try {
            vdesc = NodeUtils.toVtnPortDesc(ploc);
        } catch (VTNException e) {
            // This means that the given PortLocation contains an invalid
            // value.
            return PathPolicy.COST_UNDEF;
        }

        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            ReadTransaction rtx = ctx.getTransaction();
            VtnPathCost vpc = PathPolicyUtils.readVtnPathCost(rtx, id, vdesc);
            if (vpc != null) {
                Long c = vpc.getCost();
                if (c != null) {
                    return c.longValue();
                }
            }
            return PathPolicy.COST_UNDEF;
        } finally {
            ctx.cancelTransaction();
        }
    }

    /**
     * Associate the cost of transmitting a packet with the specified switch
     * port in the specified path policy.
     *
     * <p>
     *   The specified cost value is used when a packet is transmitted from the
     *   switch port specified by a {@link PortLocation} instance.
     * </p>
     *
     * @param id    The identifier of the path policy to be removed.
     * @param ploc  A {@link PortLocation} instance which specifies the
     *              location of the physical switch port.
     * @param cost  The cost of transmitting a packet.
     * @return  A {@link UpdateType} object which represents the result of the
     *          operation is returned.
     * @throws VTNException  An error occurred.
     */
    @Override
    public UpdateType setPathPolicyCost(int id, PortLocation ploc, long cost)
        throws VTNException {
        VTNManagerProvider provider = checkService();

        // Construct an RPC input that adds the given link cost configuration.
        VtnPortDesc vdesc = NodeUtils.toVtnPortDesc(ploc);
        PathCostList pcl = new PathCostConfigBuilder.Rpc().
            setPortDesc(vdesc).setCost(cost).getBuilder().build();
        SetPathCostInput input = new SetPathCostInputBuilder().
            setId(Integer.valueOf(id)).
            setPathCostList(Collections.singletonList(pcl)).build();

        // Invoke RPC and await its completion.
        VtnPathPolicyService rpc =
            provider.getVtnRpcService(VtnPathPolicyService.class);
        SetPathCostOutput output = getRpcOutput(rpc.setPathCost(input));
        SetPathCostResult result =
            getRpcOutput(output.getSetPathCostResult(), 0, false);
        if (!vdesc.equals(result.getPortDesc())) {
            throw new VTNException("Unexpected port desc in RPC output: " +
                                   output.getSetPathCostResult());
        }

        // Convert the result.
        return MiscUtils.toUpdateType(result.getStatus());
    }

    /**
     * Remove the cost associated with the specified switch port in the
     * specified path policy.
     *
     * @param id    The identifier of the path policy to be removed.
     * @param ploc  A {@link PortLocation} instance which specifies the
     *              location of the physical switch port.
     * @return  A {@link Status} object which represents the result of the
     *          operation is returned.
     */
    @Override
    public Status removePathPolicyCost(int id, PortLocation ploc) {
        TxContext ctx = null;
        try {
            VTNManagerProvider provider = checkService();

            // Construct an RPC input that removes the given link cost
            // configuration.
            VtnPortDesc vdesc;
            try {
                vdesc = NodeUtils.toVtnPortDesc(ploc);
            } catch (VTNException e) {
                // This means that the given PortLocation contains an invalid
                // value.
                ctx = provider.newTxContext();
                ReadTransaction rtx = ctx.getTransaction();
                PathPolicyUtils.readVtnPathPolicy(rtx, id);
                return null;
            }

            RemovePathCostInput input = new RemovePathCostInputBuilder().
                setId(Integer.valueOf(id)).
                setPortDesc(Collections.singletonList(vdesc)).build();

            // Invoke RPC and await its completion.
            VtnPathPolicyService rpc =
                provider.getVtnRpcService(VtnPathPolicyService.class);
            RemovePathCostOutput output =
                getRpcOutput(rpc.removePathCost(input));
            RemovePathCostResult result =
                getRpcOutput(output.getRemovePathCostResult(), 0, false);
            if (!vdesc.equals(result.getPortDesc())) {
                throw new VTNException("Unexpected port desc in RPC output: " +
                                       output.getRemovePathCostResult());
            }
            if (result.getStatus() == null) {
                // The specified path cost configuration did not exist.
                return null;
            }
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            if (ctx != null) {
                ctx.cancelTransaction();
            }
        }

        return new Status(StatusCode.SUCCESS, null);
    }

    /**
     * Return a list of container path maps configured in the container.
     *
     * @return  A list of {@link PathMap} instances corresponding to all
     *          container path maps configured in the container.
     *          An empty list is returned if no container path map is
     *          configured.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<PathMap> getPathMaps() throws VTNException {
        VTNManagerProvider provider = checkService();
        List<PathMap> list = new ArrayList<>();
        TxContext ctx = provider.newTxContext();
        try {
            ReadTransaction rtx = ctx.getTransaction();
            for (VtnPathMap vpm: PathMapUtils.readPathMaps(rtx)) {
                list.add(PathMapUtils.toPathMap(vpm));
            }
        } finally {
            ctx.cancelTransaction();
        }

        return list;
    }

    /**
     * Return information about the container path map specified by the index
     * number.
     *
     * @param index  The index value which specifies the path map in the
     *               container.
     * @return  A {@link PathMap} instance corresponding to the specified
     *          container path map is returned.
     *          {@code null} is returned if the specified path map does not
     *          exist in the container.
     * @throws VTNException  An error occurred.
     */
    @Override
    public PathMap getPathMap(int index) throws VTNException {
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            ReadTransaction rtx = ctx.getTransaction();
            VtnPathMap vpm = PathMapUtils.readPathMap(rtx, index);
            return PathMapUtils.toPathMap(vpm);
        } finally {
            ctx.cancelTransaction();
        }
    }

    /**
     * Create or modify the container path map specified by the index number.
     *
     * @param index  The index value which specifies the path map in the
     *               container.
     * @param pmap   A {@link PathMap} instance which specifies the
     *               configuration of the path map.
     * @return  A {@link UpdateType} object which represents the result of the
     *          operation is returned.
     * @throws VTNException  An error occurred.
     */
    @Override
    public UpdateType setPathMap(int index, PathMap pmap) throws VTNException {
        VTNManagerProvider provider = checkService();

        // Construct an RPC input that adds the given path map configuration
        // to the global path map list.
        Integer idx = Integer.valueOf(index);
        PathMapList pml =
            PathMapUtils.toPathMapListBuilder(idx, pmap).build();
        List<PathMapList> pmlist = Collections.singletonList(pml);
        SetPathMapInput input = new SetPathMapInputBuilder().
            setPathMapList(pmlist).build();

        // Invoke RPC and await its completion.
        VtnPathMapService rpc =
            provider.getVtnRpcService(VtnPathMapService.class);
        SetPathMapOutput output = getRpcOutput(rpc.setPathMap(input));
        SetPathMapResult result =
            getRpcOutput(output.getSetPathMapResult(), 0, false);
        if (!idx.equals(result.getIndex())) {
            throw new VTNException("Unexpected map index in RPC output: " +
                                   output.getSetPathMapResult());
        }

        // Convert the result.
        return MiscUtils.toUpdateType(result.getStatus());
    }

    /**
     * Remove the container path map specified by the index number.
     *
     * @param index  The index value which specifies the path map in the
     *               container.
     * @return  A {@link Status} object which represents the result of the
     *          operation is returned.
     */
    @Override
    public Status removePathMap(int index) {
        Integer idx = Integer.valueOf(index);
        try {
            VTNManagerProvider provider = checkService();

            // Construct an RPC input that removes the global path map
            // associated with the given index in the global path map list.
            RemovePathMapInput input = new RemovePathMapInputBuilder().
                setMapIndex(Collections.singletonList(idx)).build();

            // Invoke RPC and await its completion.
            VtnPathMapService rpc =
                provider.getVtnRpcService(VtnPathMapService.class);
            RemovePathMapOutput output =
                getRpcOutput(rpc.removePathMap(input));
            RemovePathMapResult result =
                getRpcOutput(output.getRemovePathMapResult(), 0, false);
            if (!idx.equals(result.getIndex())) {
                throw new VTNException("Unexpected map index in RPC output: " +
                                       output.getRemovePathMapResult());
            }
            if (result.getStatus() == null) {
                // The specified global path map was not found.
                return null;
            }
        } catch (VTNException e) {
            return e.getStatus();
        }

        return new Status(StatusCode.SUCCESS, null);
    }

    /**
     * Remove all the container path maps.
     *
     * @return  A {@link Status} object which represents the result of the
     *          operation is returned.
     */
    @Override
    public Status clearPathMap() {
        try {
            VTNManagerProvider provider = checkService();

            // Construct an RPC input that removes all the global path maps.
            ClearPathMapInput input = new ClearPathMapInputBuilder().build();

            // Invoke RPC and await its completion.
            VtnPathMapService rpc =
                provider.getVtnRpcService(VtnPathMapService.class);
            ClearPathMapOutput output =
                getRpcOutput(rpc.clearPathMap(input));
            if (output.getStatus() == null) {
                // The global path map container is empty.
                return null;
            }
        } catch (VTNException e) {
            return e.getStatus();
        }

        return new Status(StatusCode.SUCCESS, null);
    }

    /**
     * Return a list of VTN path maps configured in the VTN.
     *
     * @param path  A {@link VTenantPath} object that specifies the position
     *              of the VTN.
     * @return  A list of {@link PathMap} instances corresponding to all
     *          VTN path maps configured in the VTN.
     *          An empty list is returned if no VTN path map is configured.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<PathMap> getPathMaps(VTenantPath path) throws VTNException {
        VTNManagerProvider provider = checkService();
        String tname = VTenantUtils.getName(path);
        VTenantIdentifier ident = VTenantIdentifier.create(tname, true);
        List<PathMap> list = new ArrayList<>();
        TxContext ctx = provider.newTxContext();
        try {
            ReadTransaction rtx = ctx.getTransaction();
            for (VtnPathMap vpm: PathMapUtils.readPathMaps(rtx, ident)) {
                list.add(PathMapUtils.toPathMap(vpm));
            }
        } finally {
            ctx.cancelTransaction();
        }

        return list;
    }

    /**
     * Return information about the VTN path map specified by the index number.
     *
     * @param path   A {@link VTenantPath} object that specifies the position
     *               of the VTN.
     * @param index  The index value which specifies the path map in the VTN.
     * @return  A {@link PathMap} instance corresponding to the specified
     *          VTN path map is returned.
     *          {@code null} is returned if the specified path map does not
     *          exist in the VTN.
     * @throws VTNException  An error occurred.
     */
    @Override
    public PathMap getPathMap(VTenantPath path, int index)
        throws VTNException {
        VTNManagerProvider provider = checkService();
        String tname = VTenantUtils.getName(path);
        VTenantIdentifier ident = VTenantIdentifier.create(tname, true);
        TxContext ctx = provider.newTxContext();
        try {
            ReadTransaction rtx = ctx.getTransaction();
            VtnPathMap vpm = PathMapUtils.readPathMap(rtx, ident, index);
            return PathMapUtils.toPathMap(vpm);
        } finally {
            ctx.cancelTransaction();
        }
    }

    /**
     * Create or modify the VTN path map specified by the index number.
     *
     * @param path   A {@link VTenantPath} object that specifies the position
     *               of the VTN.
     * @param index  The index value which specifies the path map in the VTN.
     * @param pmap   A {@link PathMap} instance which specifies the
     *               configuration of the path map.
     * @return  A {@link UpdateType} object which represents the result of the
     *          operation is returned.
     * @throws VTNException  An error occurred.
     */
    @Override
    public UpdateType setPathMap(VTenantPath path, int index, PathMap pmap)
        throws VTNException {
        VTNManagerProvider provider = checkService();

        // Construct an RPC input that adds the given path map configuration
        // to the path map list in the specified VTN.
        String tname = VTenantUtils.getName(path);
        VTenantIdentifier ident = VTenantIdentifier.create(tname, true);
        Integer idx = Integer.valueOf(index);
        PathMapList pml;
        try {
            pml = PathMapUtils.toPathMapListBuilder(idx, pmap).build();
        } catch (VTNException e) {
            // Check to see if the target VTN is present.
            TxContext ctx = provider.newTxContext();
            try {
                ident.fetch(ctx.getTransaction());
            } finally {
                ctx.cancelTransaction();
            }
            throw e;
        }

        List<PathMapList> pmlist = Collections.singletonList(pml);
        SetPathMapInput input = new SetPathMapInputBuilder().
            setTenantName(tname).setPathMapList(pmlist).build();

        // Invoke RPC and await its completion.
        VtnPathMapService rpc =
            provider.getVtnRpcService(VtnPathMapService.class);
        SetPathMapOutput output = getRpcOutput(rpc.setPathMap(input));
        SetPathMapResult result =
            getRpcOutput(output.getSetPathMapResult(), 0, false);
        if (!idx.equals(result.getIndex())) {
            throw new VTNException("Unexpected map index in RPC output: " +
                                   output.getSetPathMapResult());
        }

        // Convert the result.
        return MiscUtils.toUpdateType(result.getStatus());
    }

    /**
     * Remove the VTN path map specified by the index number.
     *
     * @param path   A {@link VTenantPath} object that specifies the position
     *               of the VTN.
     * @param index  The index value which specifies the path map in the VTN.
     * @return  A {@link Status} object which represents the result of the
     *          operation is returned.
     */
    @Override
    public Status removePathMap(VTenantPath path, int index) {
        Integer idx = Integer.valueOf(index);
        try {
            VTNManagerProvider provider = checkService();
            String tname = VTenantUtils.getName(path);

            // Construct an RPC input that removes the VTN path map
            // associated with the given index in the given VTN.
            RemovePathMapInput input = new RemovePathMapInputBuilder().
                setTenantName(tname).
                setMapIndex(Collections.singletonList(idx)).build();

            // Invoke RPC and await its completion.
            VtnPathMapService rpc =
                provider.getVtnRpcService(VtnPathMapService.class);
            RemovePathMapOutput output =
                getRpcOutput(rpc.removePathMap(input));
            RemovePathMapResult result =
                getRpcOutput(output.getRemovePathMapResult(), 0, false);
            if (!idx.equals(result.getIndex())) {
                throw new VTNException("Unexpected map index in RPC output: " +
                                       output.getRemovePathMapResult());
            }
            if (result.getStatus() == null) {
                // The specified VTN path map was not found.
                return null;
            }
        } catch (VTNException e) {
            return e.getStatus();
        }

        return new Status(StatusCode.SUCCESS, null);
    }

    /**
     * Remove all the VTN path maps configured in the specified VTN.
     *
     * @param path   A {@link VTenantPath} object that specifies the position
     *               of the VTN.
     * @return  A {@link Status} object which represents the result of the
     *          operation is returned.
     */
    @Override
    public Status clearPathMap(VTenantPath path) {
        try {
            VTNManagerProvider provider = checkService();
            String tname = VTenantUtils.getName(path);

            // Construct an RPC input that removes all the VTN path maps
            // in the given VTN.
            ClearPathMapInput input = new ClearPathMapInputBuilder().
                setTenantName(tname).build();

            // Invoke RPC and await its completion.
            VtnPathMapService rpc =
                provider.getVtnRpcService(VtnPathMapService.class);
            ClearPathMapOutput output =
                getRpcOutput(rpc.clearPathMap(input));
            if (output.getStatus() == null) {
                // The VTN path map container is empty.
                return null;
            }
        } catch (VTNException e) {
            return e.getStatus();
        }

        return new Status(StatusCode.SUCCESS, null);
    }

    /**
     * Return a list of flow filters configured in the specified flow filter
     * list.
     *
     * @param fid  A {@link FlowFilterId} instance which specifies the
     *             flow filter list in the virtual node.
     * @return  A list of {@link FlowFilter} instances corresponding to all
     *          flow filters configured in the list specified by {@code fid}.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<FlowFilter> getFlowFilters(FlowFilterId fid)
        throws VTNException {
        InstanceIdentifier<? extends VtnFlowFilterList> path =
            getFlowFilterPath(fid);
        List<FlowFilter> list = new ArrayList<>();
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            ReadTransaction rtx = ctx.getTransaction();
            Optional<? extends VtnFlowFilterList> opt = DataStoreUtils.read(
                rtx, LogicalDatastoreType.OPERATIONAL, path);
            if (opt.isPresent()) {
                List<VtnFlowFilter> flist = opt.get().getVtnFlowFilter();
                if (flist != null) {
                    for (VtnFlowFilter vff: flist) {
                        list.add(VTNFlowFilter.create(vff).toFlowFilter());
                    }
                }
            } else {
                // Ensure that the virtual node is present.
                VNodeIdentifier<?> ident = toVNodeIdentifier(fid);
                ident.fetch(rtx);
            }
        } finally {
            ctx.cancelTransaction();
        }

        // Sort flow filters by indices.
        VTNIdentifiableComparator<Integer> comparator =
            new VTNIdentifiableComparator<>(Integer.class);
        Collections.sort(list, comparator);

        return list;
    }

    /**
     * Return information about the flow filter specified by the index number.
     *
     * @param fid    A {@link FlowFilterId} instance which specifies the
     *               flow filter list in the virtual node.
     * @param index  The index value which specifies the flow filter in the
     *               flow filter list specified by {@code fid}.
     * @return  A {@link FlowFilter} instance corresponding to the flow filter
     *          specified by {@code fid} and {@code index}.
     *          {@code null} is returned if the specified flow filter does not
     *          exist.
     * @throws VTNException  An error occurred.
     */
    @Override
    public FlowFilter getFlowFilter(FlowFilterId fid, int index)
        throws VTNException {
        InstanceIdentifier<? extends VtnFlowFilterList> lpath =
            getFlowFilterPath(fid);
        InstanceIdentifier<VtnFlowFilter> path = lpath.
            child(VtnFlowFilter.class, new VtnFlowFilterKey(index));
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            ReadTransaction rtx = ctx.getTransaction();
            Optional<VtnFlowFilter> opt = DataStoreUtils.read(
                rtx, LogicalDatastoreType.OPERATIONAL, path);
            FlowFilter ff;
            if (opt.isPresent()) {
                ff = VTNFlowFilter.create(opt.get()).toFlowFilter();
            } else {
                // Ensure that the virtual node is present.
                VNodeIdentifier<?> ident = toVNodeIdentifier(fid);
                ident.fetch(rtx);
                ff = null;
            }

            return ff;
        } finally {
            ctx.cancelTransaction();
        }
    }

    /**
     * Create or modify the flow filter specified by the index number.
     *
     * @param fid     A {@link FlowFilterId} instance which specifies the
     *                flow filter list in the virtual node.
     * @param index   The index value which specifies the flow filter in the
     *                flow filter list.
     * @param filter  A {@link FlowFilter} instance which specifies the
     *                configuration of the flow filter.
     * @return  A {@link UpdateType} object which represents the result of the
     *          operation is returned.
     * @throws VTNException  An error occurred.
     */
    @Override
    public UpdateType setFlowFilter(FlowFilterId fid, int index,
                                    FlowFilter filter) throws VTNException {
        VTNManagerProvider provider = checkService();
        VNodeIdentifier<?> ident = toVNodeIdentifier(fid);
        VTNFlowFilter ff = VTNFlowFilter.create(index, filter);
        SetFlowFilterInputBuilder ib = new SetFlowFilterInputBuilder().
            setOutput(fid.isOutput()).
            setTenantName(ident.getTenantNameString()).
            setInterfaceName(ident.getInterfaceNameString()).
            setVtnFlowFilter(Collections.singletonList(ff.toVtnFlowFilter()));
        String bname = ident.getBridgeNameString();
        if (ident.getType().getBridgeType() == VNodeType.VBRIDGE) {
            ib.setBridgeName(bname);
        } else {
            ib.setTerminalName(bname);
        }

        // Invoke RPC and await its completion.
        VtnFlowFilterService rpc =
            provider.getVtnRpcService(VtnFlowFilterService.class);
        SetFlowFilterOutput output =
            getRpcOutput(rpc.setFlowFilter(ib.build()));

        // Convert the result.
        FlowFilterResult result =
            getRpcOutput(output.getFlowFilterResult(), 0, false);
        Integer ridx = result.getIndex();
        if (ridx == null || ridx.intValue() != index) {
            throw new VTNException("Unexpected filter index in RPC output: " +
                                   output.getFlowFilterResult());
        }

        return MiscUtils.toUpdateType(result.getStatus());
    }

    /**
     * Remove the flow filter specified by the index number.
     *
     * @param fid    A {@link FlowFilterId} instance which specifies the
     *               flow filter list in the virtual node.
     * @param index  The index value which specifies the flow filter in the
     *               flow filter list specified by {@code fid}.
     * @return  A {@link Status} object which represents the result of the
     *          operation is returned.
     */
    @Override
    public Status removeFlowFilter(FlowFilterId fid, int index) {
        Integer idx = Integer.valueOf(index);
        try {
            VTNManagerProvider provider = checkService();
            RemoveFlowFilterInput input = createRemoveFlowFilterInput(fid, idx);

            // Invoke RPC and await its completion.
            VtnFlowFilterService rpc =
                provider.getVtnRpcService(VtnFlowFilterService.class);
            RemoveFlowFilterOutput output =
                getRpcOutput(rpc.removeFlowFilter(input));
            FlowFilterResult result =
                getRpcOutput(output.getFlowFilterResult(), 0, false);
            if (!idx.equals(result.getIndex())) {
                throw new VTNException("Unexpected filter index in " +
                                       "RPC output: " +
                                       output.getFlowFilterResult());
            }
            if (result.getStatus() == null) {
                // The specified flow filter was not found.
                return null;
            }
        } catch (VTNException e) {
            return e.getStatus();
        }

        return new Status(StatusCode.SUCCESS, null);
    }

    /**
     * Remove all the flow filters present in the specified flow filter list.
     *
     * @param fid  A {@link FlowFilterId} instance which specifies the
     *             flow filter list in the virtual node.
     * @return  A {@link Status} object which represents the result of the
     *          operation is returned.
     */
    @Override
    public Status clearFlowFilter(FlowFilterId fid) {
        try {
            VTNManagerProvider provider = checkService();
            RemoveFlowFilterInput input =
                createRemoveFlowFilterInput(fid, null);

            // Invoke RPC and await its completion.
            VtnFlowFilterService rpc =
                provider.getVtnRpcService(VtnFlowFilterService.class);
            RemoveFlowFilterOutput output =
                getRpcOutput(rpc.removeFlowFilter(input));
            List<FlowFilterResult> results = output.getFlowFilterResult();
            if (results == null) {
                // The specified flow filter list was empty.
                return null;
            }
        } catch (VTNException e) {
            return e.getStatus();
        }

        return new Status(StatusCode.SUCCESS, null);
    }
}
