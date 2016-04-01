/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import static org.hamcrest.CoreMatchers.not;
import static org.hamcrest.CoreMatchers.startsWith;

import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnDropActionBuilder;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnPopVlanActionBuilder;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnSetPortDstAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnSetPortDstActionBuilder;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnSetPortSrcAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createVtnSetPortSrcActionBuilder;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.ListIterator;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yangtools.concepts.Builder;
import org.opendaylight.yangtools.yang.binding.DataObject;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnDropFilterCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnDropFilterCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnPassFilterCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnPassFilterCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.vtn.drop.filter._case.VtnDropFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.vtn.pass.filter._case.VtnPassFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnNodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnNodesBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnOpenflowVersion;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNodeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.RemoveVlanMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.RemoveVlanMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.RemoveVlanMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.RemoveVlanMapOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.remove.vlan.map.output.RemoveVlanMapResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.remove.vlan.map.output.RemoveVlanMapResultBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicies;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPoliciesBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicy;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicyBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCostBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.LinkId;

/**
 * JUnit test for {@link DataObjectIdentity}.
 */
public class DataObjectIdentityTest extends TestBase {
    /**
     * Test case for {@link DataObjectIdentity#equals(Object)} and
     * {@link DataObjectIdentity#hashCode()}.
     *
     * <ul>
     *   <li>Test case for keyed list fields.</li>
     * </ul>
     */
    @Test
    public void testEqualsKeyedList() {
        HashSet<Object> set = new HashSet<Object>();

        // Create PortLink instances.
        List<PortLink> portLinks = new ArrayList<>();
        LinkId[] linkIds = {
            null,
            new LinkId("openflow:1:2"),
            new LinkId("openflow:1:3"),
            new LinkId("openflow:3:4"),
            new LinkId("openflow:3:5"),
        };

        int index = 0;
        for (LinkId lid: linkIds) {
            NodeConnectorId peer = (index == 1)
                ? null
                : new NodeConnectorId("openflow:100:200");
            PortLinkBuilder builder = new PortLinkBuilder().
                setLinkId(lid).
                setPeer(peer);
            doTest(set, portLinks, builder);
            index++;
        }

        // Create VtnPort instances.
        List<VtnPort> vtnPorts = new ArrayList<>();
        NodeConnectorId[] portIds = {
            null,
            new NodeConnectorId("openflow:2:3"),
            new NodeConnectorId("openflow:2:4"),
            new NodeConnectorId("openflow:1:1"),
            new NodeConnectorId("openflow:1:2"),
            new NodeConnectorId("openflow:1:5"),
            new NodeConnectorId("openflow:100:200"),
            new NodeConnectorId("openflow:101:200"),
        };

        index = 0;
        for (NodeConnectorId id: portIds) {
            String name;
            Boolean state;
            Long cost;
            if (index == 3) {
                name = null;
                state = null;
                cost = null;
            } else {
                name = "port";
                state = true;
                cost = 10000L;
            }

            List<PortLink> links;
            int idx = index % 4;
            if (idx == 0) {
                links = null;
            } else if (idx == 1) {
                links = Collections.<PortLink>emptyList();
            } else if (idx == 2) {
                links = Collections.singletonList(portLinks.get(1));
            } else {
                links = new ArrayList<PortLink>(portLinks);
            }

            VtnPortBuilder builder = new VtnPortBuilder().
                setId(id).
                setName(name).
                setEnabled(state).
                setCost(cost).
                setPortLink(links);
            doTest(set, vtnPorts, builder);

            index++;
        }

        // Create VtnNode instances.
        List<VtnNode> vtnNodes = new ArrayList<>();
        NodeId[] nodeIds = {
            null,
            new NodeId("openflow:1"),
            new NodeId("openflow:2"),
            new NodeId("openflow:3"),
            new NodeId("openflow:10"),
            new NodeId("openflow:20"),
            new NodeId("openflow:21"),
            new NodeId("openflow:22"),
        };

        index = 0;
        for (NodeId id: nodeIds) {
            VtnOpenflowVersion ver = (index == 4)
                ? null
                : VtnOpenflowVersion.OF13;
            List<VtnPort> ports;
            int idx = index % 4;
            if (idx == 0) {
                ports = null;
            } else if (idx == 1) {
                ports = Collections.<VtnPort>emptyList();
            } else if (idx == 2) {
                ports = Collections.singletonList(vtnPorts.get(2));
            } else {
                ports = new ArrayList<>(vtnPorts);
            }

            VtnNodeBuilder builder = new VtnNodeBuilder().
                setId(id).
                setOpenflowVersion(ver).
                setVtnPort(ports);
            doTest(set, vtnNodes, builder);

            index++;
        }

        // Create VtnNodes instances.
        List<VtnNodes> nodeContainers = new ArrayList<>();
        VtnNodesBuilder nodesBuilder = new VtnNodesBuilder();
        doTest(set, nodeContainers, nodesBuilder);

        List<VtnNode> nodes = new ArrayList<>();
        nodesBuilder.setVtnNode(nodes);
        doTest(set, nodeContainers, nodesBuilder);

        nodes = new ArrayList<VtnNode>();
        nodes.add(vtnNodes.get(0));
        nodesBuilder.setVtnNode(nodes);
        doTest(set, nodeContainers, nodesBuilder);

        nodes = new ArrayList<VtnNode>(vtnNodes);
        nodesBuilder.setVtnNode(nodes);
        doTest(set, nodeContainers, nodesBuilder);

        // Create VtnPathCost instances.
        List<VtnPathCost> pathCosts = new ArrayList<>();
        VtnPortDesc[] portDescs = {
            null,
            new VtnPortDesc("openflow:1,,"),
            new VtnPortDesc("openflow:1,2,"),
            new VtnPortDesc("openflow:1,3,"),
            new VtnPortDesc("openflow:1,2,eth2"),
            new VtnPortDesc("openflow:999,,port-10"),
            new VtnPortDesc("openflow:999,33,port-33"),
            new VtnPortDesc("openflow:12345,67,port-890"),
        };

        index = 0;
        for (VtnPortDesc vdesc: portDescs) {
            Long cost = (index == 2)
                ? null
                : 10000L;
            VtnPathCostBuilder builder = new VtnPathCostBuilder().
                setPortDesc(vdesc).
                setCost(cost);
            doTest(set, pathCosts, builder);
            index++;
        }

        // Create VtnPathPolicy instances.
        List<VtnPathPolicy> pathPolicies = new ArrayList<>();
        Integer[] policyIds = {
            null, 1, 2, 3,
        };

        index = 0;
        for (Integer id: policyIds) {
            Long cost = (index == 1)
                ? null
                : 8000L;
            List<VtnPathCost> vpcosts;
            int idx = index % 4;
            if (id == null) {
                vpcosts = null;
            } else if (idx == 1) {
                vpcosts = Collections.<VtnPathCost>emptyList();
            } else if (idx == 2) {
                vpcosts = Collections.singletonList(pathCosts.get(3));
            } else {
                vpcosts = new ArrayList<>(pathCosts);
            }

            VtnPathPolicyBuilder builder = new VtnPathPolicyBuilder().
                setId(id).
                setDefaultCost(cost).
                setVtnPathCost(vpcosts);
            doTest(set, pathPolicies, builder);

            index++;
        }

        // Create VtnPathPolicies instances.
        List<VtnPathPolicies> policyContainers = new ArrayList<>();
        VtnPathPoliciesBuilder policiesBuilder = new VtnPathPoliciesBuilder();
        doTest(set, policyContainers, policiesBuilder);

        List<VtnPathPolicy> policies = new ArrayList<>();
        policiesBuilder.setVtnPathPolicy(policies);
        doTest(set, policyContainers, policiesBuilder);

        policies = new ArrayList<VtnPathPolicy>();
        policies.add(pathPolicies.get(0));
        policiesBuilder.setVtnPathPolicy(policies);
        doTest(set, policyContainers, policiesBuilder);

        policies = new ArrayList<VtnPathPolicy>(pathPolicies);
        policiesBuilder.setVtnPathPolicy(policies);
        doTest(set, policyContainers, policiesBuilder);

        // Create flow actions.
        List<VtnFlowAction> actions = new ArrayList<>();
        VtnFlowActionBuilder actBuilder = createVtnDropActionBuilder(1);
        doTest(set, actions, actBuilder);

        actBuilder = createVtnDropActionBuilder(2);
        doTest(set, actions, actBuilder);

        actBuilder = createVtnPopVlanActionBuilder(3);
        doTest(set, actions, actBuilder);

        actBuilder = createVtnSetPortSrcActionBuilder(4, 12345);
        doTest(set, actions, actBuilder);

        actBuilder = createVtnSetPortDstActionBuilder(5, 12345);
        doTest(set, actions, actBuilder);

        actBuilder = createVtnSetPortDstActionBuilder(6, 12346);
        doTest(set, actions, actBuilder);

        // Create flow filters.
        List<VtnFlowFilter> filters = new ArrayList<>();
        VtnDropFilterCase drop = new VtnDropFilterCaseBuilder().
            setVtnDropFilter(new VtnDropFilterBuilder().build()).
            build();
        VtnPassFilterCase pass = new VtnPassFilterCaseBuilder().
            setVtnPassFilter(new VtnPassFilterBuilder().build()).
            build();
        VnodeName vcond1 = new VnodeName("cond1");
        VnodeName vcond2 = new VnodeName("cond2");
        VtnFlowFilterBuilder filterBuilder = new VtnFlowFilterBuilder().
            setIndex(1).
            setCondition(vcond1).
            setVtnFlowFilterType(drop);
        doTest(set, filters, filterBuilder);

        filterBuilder = new VtnFlowFilterBuilder().
            setIndex(2).
            setCondition(vcond1).
            setVtnFlowFilterType(drop);
        doTest(set, filters, filterBuilder);

        filterBuilder = new VtnFlowFilterBuilder().
            setIndex(3).
            setCondition(vcond2).
            setVtnFlowFilterType(drop);
        doTest(set, filters, filterBuilder);

        filterBuilder = new VtnFlowFilterBuilder().
            setIndex(4).
            setCondition(vcond1).
            setVtnFlowFilterType(pass);
        doTest(set, filters, filterBuilder);

        filterBuilder = new VtnFlowFilterBuilder().
            setIndex(5).
            setCondition(vcond1).
            setVtnFlowFilterType(pass);
        doTest(set, filters, filterBuilder);

        List<VtnFlowAction> filterActions = new ArrayList<>();
        Collections.addAll(
            filterActions,
            createVtnSetPortSrcAction(100, 9999),
            createVtnSetPortDstAction(200, 9999));

        filterBuilder = new VtnFlowFilterBuilder().
            setIndex(100).
            setCondition(vcond1).
            setVtnFlowFilterType(pass).
            setVtnFlowAction(filterActions);
        doTest(set, filters, filterBuilder);

        filterActions = new ArrayList<>();
        Collections.addAll(
            filterActions,
            createVtnSetPortSrcAction(101, 9999),
            createVtnSetPortDstAction(201, 9999));

        filterBuilder = new VtnFlowFilterBuilder().
            setIndex(101).
            setCondition(vcond1).
            setVtnFlowFilterType(pass).
            setVtnFlowAction(filterActions);
        doTest(set, filters, filterBuilder);

        int expected = portLinks.size() + vtnPorts.size() + vtnNodes.size() +
            nodeContainers.size() + pathCosts.size() + pathPolicies.size() +
            policyContainers.size() + actions.size() + filters.size();
        assertEquals(expected, set.size());

        // Ensure that the order of elements in a list does not affect object
        // identity.
        for (VtnPort vport: vtnPorts) {
            DataObjectIdentity doi = new DataObjectIdentity(reorder(vport));
            assertEquals(true, set.contains(doi));
        }
        for (VtnNode vnode: vtnNodes) {
            DataObjectIdentity doi = new DataObjectIdentity(reorder(vnode));
            assertEquals(true, set.contains(doi));
        }
        for (VtnNodes vnodes: nodeContainers) {
            DataObjectIdentity doi = new DataObjectIdentity(reorder(vnodes));
            assertEquals(true, set.contains(doi));
        }
        for (VtnPathPolicy vpp: pathPolicies) {
            DataObjectIdentity doi = new DataObjectIdentity(reorder(vpp));
            assertEquals(true, set.contains(doi));
        }
        for (VtnPathPolicies vpps: policyContainers) {
            DataObjectIdentity doi = new DataObjectIdentity(reorder(vpps));
            assertEquals(true, set.contains(doi));
        }
        for (VtnFlowFilter vff: filters) {
            DataObjectIdentity doi = new DataObjectIdentity(reorder(vff));
            assertEquals(true, set.contains(doi));
        }

        // Duplicate key is not allowed.
        List<PortLink> badLinks = new ArrayList<>();
        PortLink plink1 = new PortLinkBuilder().
            setLinkId(linkIds[3]).
            setPeer(new NodeConnectorId("openflow:1:10")).
            build();
        PortLink plink2 = new PortLinkBuilder().
            setLinkId(linkIds[2]).
            setPeer(new NodeConnectorId("openflow:1:11")).
            build();
        PortLink plink3 = new PortLinkBuilder().
            setLinkId(linkIds[3]).
            setPeer(new NodeConnectorId("openflow:1:12")).
            build();
        Collections.addAll(badLinks, plink1, plink2, plink3);
        VtnPort badPort = new VtnPortBuilder().
            setId(portIds[2]).
            setPortLink(badLinks).
            build();
        try {
            new DataObjectIdentity(badPort);
            unexpected();
        } catch (IllegalArgumentException e) {
            String msg = "Keyed list in DataObject should have no duplicate:";
            assertThat(e.getMessage(), startsWith(msg));
        }
    }

    /**
     * Test case for {@link DataObjectIdentity#equals(Object)} and
     * {@link DataObjectIdentity#hashCode()}.
     *
     * <ul>
     *   <li>Test case for non-keyed list fields.</li>
     * </ul>
     */
    @Test
    public void testEqualsList() {
        HashSet<Object> set = new HashSet<Object>();

        // Create a list of remove-vlan-map-result.
        List<RemoveVlanMapResult> vmapResults = new ArrayList<>();
        vmapResults.add(null);
        String[] mapIds = {
            null,
            "ANY.0",
            "ANY.1",
            "ANY.4095",
            "openflow:1.0",
            "openflow:1.1",
            "openflow:2.1",
            "openflow:2.2",
        };
        VtnUpdateType[] types = {
            null,
            VtnUpdateType.CREATED,
            VtnUpdateType.REMOVED,
            VtnUpdateType.CHANGED,
        };

        for (String mapId: mapIds) {
            RemoveVlanMapResultBuilder builder =
                new RemoveVlanMapResultBuilder().
                setMapId(mapId);
            for (VtnUpdateType type: types) {
                builder.setStatus(type);
                doTest(set, vmapResults, builder);
            }
        }

        List<RemoveVlanMapOutput> outputs = new ArrayList<>();
        RemoveVlanMapOutputBuilder builder = new RemoveVlanMapOutputBuilder();
        doTest(set, outputs, builder);

        List<RemoveVlanMapResult> results = new ArrayList<>();
        builder.setRemoveVlanMapResult(results);
        doTest(set, outputs, builder);

        for (int i = 0; i < 10; i++) {
            results = Collections.singletonList(vmapResults.get(i));
            builder.setRemoveVlanMapResult(results);
            doTest(set, outputs, builder);
        }

        results = new ArrayList<>(vmapResults);
        builder.setRemoveVlanMapResult(results);
        doTest(set, outputs, builder);

        // Duplicate elements are allowed.
        results = new ArrayList<>(vmapResults);
        for (int i = 0; i < vmapResults.size() / 2; i++) {
            RemoveVlanMapResult org = vmapResults.get(i);
            RemoveVlanMapResult res = (org == null)
                ? null
                : new RemoveVlanMapResultBuilder(org).build();
            results.add(res);
        }
        builder.setRemoveVlanMapResult(results);
        doTest(set, outputs, builder);

        // Ensure that the order of elements in a list does not affect object
        // identity.
        for (RemoveVlanMapOutput out: outputs) {
            DataObjectIdentity doi = new DataObjectIdentity(reorder(out));
            assertEquals(true, set.contains(doi));
        }
    }

    /**
     * Test case for {@link DataObjectIdentity#equals(Object)} and
     * {@link DataObjectIdentity#hashCode()}.
     *
     * <ul>
     *   <li>Test case for leaf-list fields.</li>
     * </ul>
     */
    @Test
    public void testEqualsLeafList() {
        HashSet<Object> set = new HashSet<Object>();

        // Create a list of VLAN mappings IDs.
        List<RemoveVlanMapInput> inputs = new ArrayList<>();
        List<String> vmapIds = new ArrayList<>();
        String[] mapIds = {
            null,
            "ANY.0",
            "ANY.1",
            "ANY.4095",
            "openflow:1.0",
            "openflow:1.1",
            "openflow:2.1",
            "openflow:2.2",
        };

        for (String mapId: mapIds) {
            vmapIds.add(mapId);
            List<String> ids = new ArrayList<>(vmapIds);
            RemoveVlanMapInputBuilder builder =
                new RemoveVlanMapInputBuilder().setMapIds(ids);
            doTest(set, inputs, builder);
        }

        // Duplicate elements are allowed.
        int size = vmapIds.size();
        for (int i = 0; i < size; i++) {
            if ((i & 1) == 0) {
                vmapIds.add(vmapIds.get(i));
                List<String> ids = new ArrayList<>(vmapIds);
                RemoveVlanMapInputBuilder builder =
                    new RemoveVlanMapInputBuilder().setMapIds(ids);
                doTest(set, inputs, builder);
            }
        }

        // Ensure that the order of elements in a list does not affect object
        // identity.
        for (RemoveVlanMapInput in: inputs) {
            DataObjectIdentity doi = new DataObjectIdentity(reorder(in));
            assertEquals(true, set.contains(doi));
        }
    }

    /**
     * Reorder lists in the given object.
     *
     * @param port  A {@link VtnPort} instance.
     * @return  A {@link VtnPort} instance with reordering lists.
     */
    private VtnPort reorder(VtnPort port) {
        List<PortLink> plinks = port.getPortLink();
        int size = (plinks == null) ? 0 : plinks.size();
        boolean reversed = (size > 1);
        if (reversed) {
            List<PortLink> l = new ArrayList<>();
            for (ListIterator<PortLink> it = plinks.listIterator(size);
                 it.hasPrevious();) {
                l.add(new PortLinkBuilder(it.previous()).build());
            }
            assertThat(l, not(plinks));
            plinks = l;
        }

        VtnPort ret = new VtnPortBuilder(port).setPortLink(plinks).build();
        if (reversed) {
            assertThat(ret, not(port));
        }
        return ret;
    }

    /**
     * Reorder lists in the given object.
     *
     * @param node  A {@link VtnNode} instance.
     * @return  A {@link VtnNode} instance with reordering lists.
     */
    private VtnNode reorder(VtnNode node) {
        List<VtnPort> ports = node.getVtnPort();
        int size = (ports == null) ? 0 : ports.size();
        boolean reversed = (size > 1);
        if (reversed) {
            List<VtnPort> l = new ArrayList<>();
            for (ListIterator<VtnPort> it = ports.listIterator(size);
                 it.hasPrevious();) {
                l.add(reorder(it.previous()));
            }
            assertThat(l, not(ports));
            ports = l;
        }

        VtnNode ret = new VtnNodeBuilder(node).setVtnPort(ports).build();
        if (reversed) {
            assertThat(ret, not(node));
        }
        return ret;
    }

    /**
     * Reorder lists in the given object.
     *
     * @param container  A {@link VtnNodes} instance.
     * @return  A {@link VtnNodes} instance with reordering lists.
     */
    private VtnNodes reorder(VtnNodes container) {
        List<VtnNode> nodes = container.getVtnNode();
        int size = (nodes == null) ? 0 : nodes.size();
        boolean reversed = (size > 1);
        if (reversed) {
            List<VtnNode> l = new ArrayList<>();
            for (ListIterator<VtnNode> it = nodes.listIterator(size);
                 it.hasPrevious();) {
                l.add(reorder(it.previous()));
            }
            assertThat(l, not(nodes));
            nodes = l;
        }

        VtnNodes ret = new VtnNodesBuilder().setVtnNode(nodes).build();
        if (reversed) {
            assertThat(ret, not(container));
        }
        return ret;
    }

    /**
     * Reorder lists in the given object.
     *
     * @param vpp  A {@link VtnPathPolicy} instance.
     * @return  A {@link VtnPathPolicy} instance with reordering lists.
     */
    private VtnPathPolicy reorder(VtnPathPolicy vpp) {
        List<VtnPathCost> vpcosts = vpp.getVtnPathCost();
        int size = (vpcosts == null) ? 0 : vpcosts.size();
        boolean reversed = (size > 1);
        if (reversed) {
            List<VtnPathCost> l = new ArrayList<>();
            for (ListIterator<VtnPathCost> it = vpcosts.listIterator(size);
                 it.hasPrevious();) {
                l.add(new VtnPathCostBuilder(it.previous()).build());
            }
            assertThat(l, not(vpcosts));
            vpcosts = l;
        }

        VtnPathPolicy ret = new VtnPathPolicyBuilder(vpp).
            setVtnPathCost(vpcosts).
            build();
        if (reversed) {
            assertThat(ret, not(vpp));
        }
        return ret;
    }

    /**
     * Reorder lists in the given object.
     *
     * @param container  A {@link VtnPathPolicies} instance.
     * @return  A {@link VtnPathPolicies} instance with reordering lists.
     */
    private VtnPathPolicies reorder(VtnPathPolicies container) {
        List<VtnPathPolicy> policies = container.getVtnPathPolicy();
        int size = (policies == null) ? 0 : policies.size();
        boolean reversed = (size > 1);
        if (reversed) {
            List<VtnPathPolicy> l = new ArrayList<>();
            for (ListIterator<VtnPathPolicy> it = policies.listIterator(size);
                 it.hasPrevious();) {
                l.add(reorder(it.previous()));
            }
            assertThat(l, not(policies));
            policies = l;
        }

        VtnPathPolicies ret = new VtnPathPoliciesBuilder().
            setVtnPathPolicy(policies).
            build();
        if (reversed) {
            assertThat(ret, not(container));
        }
        return ret;
    }

    /**
     * Reorder flow action list in the given flow filter.
     *
     * @param filter  A {@link VtnFlowFilter} instance.
     * @return  A {@link VtnFlowFilter} instance with reordering flow action
     *          list.
     */
    private VtnFlowFilter reorder(VtnFlowFilter filter) {
        List<VtnFlowAction> factions = filter.getVtnFlowAction();
        int size = (factions == null) ? 0 : factions.size();
        boolean reversed = (size > 1);
        if (reversed) {
            List<VtnFlowAction> l = new ArrayList<>();
            for (ListIterator<VtnFlowAction> it = factions.listIterator(size);
                 it.hasPrevious();) {
                l.add(new VtnFlowActionBuilder(it.previous()).build());
            }
            assertThat(l, not(factions));
            factions = l;
        }

        VtnFlowFilter ret = new VtnFlowFilterBuilder(filter).
            setVtnFlowAction(factions).
            build();
        if (reversed) {
            assertThat(ret, not(filter));
        }
        return ret;
    }

    /**
     * Reorder remove-vlan-map-result list in the given remove-vlan-map-output.
     *
     * @param out  A {@link RemoveVlanMapOutput} instance.
     * @return  A {@link RemoveVlanMapOutput} instance with reordering
     *          remove-vlan-map-result list.
     */
    private RemoveVlanMapOutput reorder(RemoveVlanMapOutput out) {
        List<RemoveVlanMapResult> results = out.getRemoveVlanMapResult();
        int size = (results == null) ? 0 : results.size();
        boolean reversed = (size > 1);
        if (reversed) {
            List<RemoveVlanMapResult> l = new ArrayList<>();
            for (ListIterator<RemoveVlanMapResult> it =
                     results.listIterator(size); it.hasPrevious();) {
                RemoveVlanMapResult res = it.previous();
                if (res != null) {
                    res = new RemoveVlanMapResultBuilder(res).build();
                }
                l.add(res);
            }
            assertThat(l, not(results));
            results = l;
        }

        RemoveVlanMapOutput ret = new RemoveVlanMapOutputBuilder(out).
            setRemoveVlanMapResult(results).
            build();
        if (reversed) {
            assertThat(ret, not(out));
        }
        return ret;
    }

    /**
     * Reorder map-ids list in the given remove-vlan-map-input.
     *
     * @param in  A {@link RemoveVlanMapInput} instance.
     * @return  A {@link RemoveVlanMapInput} instance with reordering map-ids
     *          list.
     */
    private RemoveVlanMapInput reorder(RemoveVlanMapInput in) {
        List<String> mapIds = in.getMapIds();
        int size = (mapIds == null) ? 0 : mapIds.size();
        boolean reversed = (size > 1);
        if (reversed) {
            List<String> l = new ArrayList<>();
            for (ListIterator<String> it = mapIds.listIterator(size);
                 it.hasPrevious();) {
                String id = it.previous();
                if (id != null) {
                    id = new String(id);
                }
                l.add(id);
            }
            assertThat(l, not(mapIds));
            mapIds = l;
        }

        RemoveVlanMapInput ret = new RemoveVlanMapInputBuilder(in).
            setMapIds(mapIds).
            build();
        if (reversed) {
            assertThat(ret, not(in));
        }
        return ret;
    }

    /**
     * Add a data object for test to the given set.
     *
     * @param set      A set of test instances.
     * @param list     A list to store created object.
     * @param builder  Builder instance.
     * @param <T>      The type of the data object to be created by the given
     *                 builder instance.
     */
    private <T extends DataObject> void doTest(Set<Object> set, List<T> list,
                                               Builder<T> builder) {
        T obj = builder.build();
        list.add(obj);
        DataObjectIdentity doi1 = new DataObjectIdentity(obj);
        DataObjectIdentity doi2 = new DataObjectIdentity(builder.build());
        testEquals(set, doi1, doi2);
    }
}
