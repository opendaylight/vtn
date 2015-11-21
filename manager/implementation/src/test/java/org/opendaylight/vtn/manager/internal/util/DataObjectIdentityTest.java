/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

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
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicies;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPoliciesBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicy;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicyBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCostBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;

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
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();

        // Create PortLink instances.
        List<PortLink> portLinks = new ArrayList<>();
        LinkId[] linkIds = {
            null,
            new LinkId("openflow:1:2"),
            new LinkId("openflow:3:4"),
        };
        NodeConnectorId[] peers = {
            null,
            new NodeConnectorId("openflow:1:3"),
            new NodeConnectorId("openflow:3:3"),
        };
        for (LinkId lid: linkIds) {
            PortLinkBuilder builder = new PortLinkBuilder().setLinkId(lid);
            for (NodeConnectorId peer: peers) {
                builder.setPeer(peer);
                doTest(set, portLinks, builder);
            }
        }

        // Create VtnPort instances.
        List<VtnPort> vtnPorts = new ArrayList<>();
        NodeConnectorId[] portIds = {
            null,
            new NodeConnectorId("openflow:2:4"),
            new NodeConnectorId("openflow:1:3"),
        };
        String[] portNames = {
            null,
            "port-1",
            "port-2",
        };
        Boolean[] portStates = {
            null,
            Boolean.TRUE,
            Boolean.FALSE,
        };
        Long[] linkCosts = {
            null,
            Long.valueOf(100L),
            Long.valueOf(10000L),
        };
        for (NodeConnectorId id: portIds) {
            VtnPortBuilder builder = new VtnPortBuilder().setId(id);
            for (String name: portNames) {
                builder.setName(name);
                for (Boolean en: portStates) {
                    builder.setEnabled(en);
                    for (Long cost: linkCosts) {
                        builder.setCost(cost);

                        List<PortLink> links = null;
                        builder.setPortLink(links);
                        doTest(set, vtnPorts, builder);

                        links = new ArrayList<PortLink>();
                        builder.setPortLink(links);
                        doTest(set, vtnPorts, builder);

                        links = new ArrayList<PortLink>();
                        links.add(portLinks.get(0));
                        builder.setPortLink(links);
                        doTest(set, vtnPorts, builder);

                        links = new ArrayList<PortLink>(portLinks);
                        builder.setPortLink(links);
                        doTest(set, vtnPorts, builder);
                    }
                }
            }
        }

        // Create VtnNode instances.
        List<VtnNode> vtnNodes = new ArrayList<>();
        NodeId[] nodeIds = {
            null,
            new NodeId("openflow:1"),
            new NodeId("openflow:10"),
        };
        VtnOpenflowVersion[] protocols = {
            null,
            VtnOpenflowVersion.OF10,
            VtnOpenflowVersion.OF13,
        };

        for (NodeId id: nodeIds) {
            VtnNodeBuilder builder = new VtnNodeBuilder().setId(id);
            for (VtnOpenflowVersion proto: protocols) {
                builder.setOpenflowVersion(proto);

                List<VtnPort> ports = null;
                builder.setVtnPort(ports);
                doTest(set, vtnNodes, builder);

                ports = new ArrayList<VtnPort>();
                builder.setVtnPort(ports);
                doTest(set, vtnNodes, builder);

                ports = new ArrayList<VtnPort>();
                ports.add(vtnPorts.get(0));
                builder.setVtnPort(ports);
                doTest(set, vtnNodes, builder);

                ports = new ArrayList<VtnPort>(vtnPorts);
                builder.setVtnPort(ports);
                doTest(set, vtnNodes, builder);
            }
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
            new VtnPortDesc("openflow:1,2,eth2"),
        };
        for (VtnPortDesc vdesc: portDescs) {
            VtnPathCostBuilder builder = new VtnPathCostBuilder().
                setPortDesc(vdesc);
            for (Long cost: linkCosts) {
                builder.setCost(cost);
                doTest(set, pathCosts, builder);
            }
        }

        // Create VtnPathPolicy instances.
        List<VtnPathPolicy> pathPolicies = new ArrayList<>();
        Integer[] policyIds = {
            null,
            Integer.valueOf(1),
            Integer.valueOf(3),
        };
        for (Integer id: policyIds) {
            VtnPathPolicyBuilder builder = new VtnPathPolicyBuilder().
                setId(id);
            for (Long cost: linkCosts) {
                builder.setDefaultCost(cost);

                List<VtnPathCost> vpcosts = null;
                builder.setVtnPathCost(vpcosts);
                doTest(set, pathPolicies, builder);

                vpcosts = new ArrayList<VtnPathCost>();
                builder.setVtnPathCost(vpcosts);
                doTest(set, pathPolicies, builder);

                vpcosts = new ArrayList<VtnPathCost>();
                vpcosts.add(pathCosts.get(0));
                builder.setVtnPathCost(vpcosts);
                doTest(set, pathPolicies, builder);

                vpcosts = new ArrayList<VtnPathCost>(pathCosts);
                builder.setVtnPathCost(vpcosts);
                doTest(set, pathPolicies, builder);
            }
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

        actBuilder = createVtnPopVlanActionBuilder(1);
        doTest(set, actions, actBuilder);

        actBuilder = createVtnSetPortSrcActionBuilder(1, 12345);
        doTest(set, actions, actBuilder);

        actBuilder = createVtnSetPortDstActionBuilder(1, 12345);
        doTest(set, actions, actBuilder);

        actBuilder = createVtnSetPortDstActionBuilder(1, 12346);
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
            setIndex(1).
            setCondition(vcond2).
            setVtnFlowFilterType(drop);
        doTest(set, filters, filterBuilder);

        filterBuilder = new VtnFlowFilterBuilder().
            setIndex(1).
            setCondition(vcond1).
            setVtnFlowFilterType(pass);
        doTest(set, filters, filterBuilder);

        List<VtnFlowAction> filterActions = new ArrayList<>();
        Collections.addAll(
            filterActions,
            createVtnSetPortSrcAction(1, 9999),
            createVtnSetPortDstAction(2, 9999));

        filterBuilder = new VtnFlowFilterBuilder().
            setIndex(1).
            setCondition(vcond1).
            setVtnFlowFilterType(pass).
            setVtnFlowAction(filterActions);
        doTest(set, filters, filterBuilder);

        filterActions = new ArrayList<>();
        Collections.addAll(
            filterActions,
            createVtnSetPortSrcAction(2, 9999),
            createVtnSetPortDstAction(1, 9999));

        filterBuilder = new VtnFlowFilterBuilder().
            setIndex(1).
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
        for (Object o: set) {
            assertTrue(set.contains(o));
            if (o instanceof VtnNodes) {
                assertTrue(set.contains(reorder((VtnNodes)o)));
            } else if (o instanceof VtnNode) {
                assertTrue(set.contains(reorder((VtnNode)o)));
            } else if (o instanceof VtnPort) {
                assertTrue(set.contains(reorder((VtnPort)o)));
            } else if (o instanceof VtnPathPolicies) {
                assertTrue(set.contains(reorder((VtnPathPolicies)o)));
            } else if (o instanceof VtnPathPolicy) {
                assertTrue(set.contains(reorder((VtnPathPolicy)o)));
            } else if (o instanceof VtnFlowFilter) {
                assertTrue(set.contains(reorder((VtnFlowFilter)o)));
            }
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
        if (plinks != null && !plinks.isEmpty()) {
            List<PortLink> l = new ArrayList<>();
            for (ListIterator<PortLink> it = plinks.listIterator(plinks.size());
                 it.hasPrevious();) {
                l.add(it.previous());
            }
            assertFalse(l.equals(plinks));
            return new VtnPortBuilder(port).setPortLink(l).build();
        }

        return port;
    }

    /**
     * Reorder lists in the given object.
     *
     * @param node  A {@link VtnNode} instance.
     * @return  A {@link VtnNode} instance with reordering lists.
     */
    private VtnNode reorder(VtnNode node) {
        List<VtnPort> ports = node.getVtnPort();
        if (ports != null && !ports.isEmpty()) {
            List<VtnPort> l = new ArrayList<>();
            for (ListIterator<VtnPort> it = ports.listIterator(ports.size());
                 it.hasPrevious();) {
                VtnPort port = reorder(it.previous());
                l.add(port);
            }
            assertFalse(l.equals(ports));
            return new VtnNodeBuilder(node).setVtnPort(l).build();
        }

        return node;
    }

    /**
     * Reorder lists in the given object.
     *
     * @param container  A {@link VtnNodes} instance.
     * @return  A {@link VtnNodes} instance with reordering lists.
     */
    private VtnNodes reorder(VtnNodes container) {
        List<VtnNode> nodes = container.getVtnNode();
        if (nodes != null && !nodes.isEmpty()) {
            List<VtnNode> l = new ArrayList<>();
            for (ListIterator<VtnNode> it = nodes.listIterator(nodes.size());
                 it.hasPrevious();) {
                VtnNode node = reorder(it.previous());
                l.add(node);
            }
            assertFalse(l.equals(nodes));
            return new VtnNodesBuilder().setVtnNode(l).build();
        }

        return container;
    }

    /**
     * Reorder lists in the given object.
     *
     * @param vpp  A {@link VtnPathPolicy} instance.
     * @return  A {@link VtnPathPolicy} instance with reordering lists.
     */
    private VtnPathPolicy reorder(VtnPathPolicy vpp) {
        List<VtnPathCost> vpcosts = vpp.getVtnPathCost();
        if (vpcosts != null && !vpcosts.isEmpty()) {
            List<VtnPathCost> l = new ArrayList<>();
            for (ListIterator<VtnPathCost> it =
                     vpcosts.listIterator(vpcosts.size()); it.hasPrevious();) {
                l.add(it.previous());
            }
            assertFalse(l.equals(vpcosts));
            return new VtnPathPolicyBuilder(vpp).setVtnPathCost(l).build();
        }

        return vpp;
    }

    /**
     * Reorder lists in the given object.
     *
     * @param container  A {@link VtnPathPolicies} instance.
     * @return  A {@link VtnPathPolicies} instance with reordering lists.
     */
    private VtnPathPolicies reorder(VtnPathPolicies container) {
        List<VtnPathPolicy> policies = container.getVtnPathPolicy();
        if (policies != null && !policies.isEmpty()) {
            List<VtnPathPolicy> l = new ArrayList<>();
            for (ListIterator<VtnPathPolicy> it =
                     policies.listIterator(policies.size());
                 it.hasPrevious();) {
                VtnPathPolicy vpp = reorder(it.previous());
                l.add(vpp);
            }
            assertFalse(l.equals(policies));
            return new VtnPathPoliciesBuilder().setVtnPathPolicy(l).build();
        }

        return container;
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
        if (factions != null && !factions.isEmpty()) {
            int size = factions.size();
            List<VtnFlowAction> l = new ArrayList<>();
            for (ListIterator<VtnFlowAction> it = factions.listIterator(size);
                 it.hasPrevious();) {
                l.add(it.previous());
            }
            assertFalse(l.equals(factions));
            return new VtnFlowFilterBuilder(filter).
                setVtnFlowAction(l).build();
        }

        return filter;
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
