/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtils;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnNodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicies;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicy;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.Nodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnector;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;

/**
 * JUnit test for {@link IdentifierTargetComparator}.
 */
public class IdentifierTargetComparatorTest extends TestBase {
    /**
     * Test case for below methods.
     *
     * <ul>
     *   <li>{@link IdentifierTargetComparator#setOrder(Class, int)}</li>
     *   <li>{@link IdentifierTargetComparator#getOrder(Class)}</li>
     *   <li>
     *     {@link IdentifierTargetComparator#compare(InstanceIdentifier, InstanceIdentifier)}
     *   </li>
     * </ul>
     */
    @Test
    public void testCompare() {
        // Create a list of instance idenfifiers.
        List<InstanceIdentifier<?>> identifiers = new ArrayList<>();
        Map<Class<? extends DataObject>, Integer> counter = new HashMap<>();
        List<Class<? extends DataObject>> containers = new ArrayList<>();
        Collections.addAll(containers, Nodes.class, VtnNodes.class,
                           VtnPathPolicies.class);
        for (Class<? extends DataObject> type: containers) {
            identifiers.add(InstanceIdentifier.create(type));
            assertEquals(null, counter.put(type, Integer.valueOf(1)));
        }

        final int numNodes = 5;
        final int nports = 5;
        for (long i = 1L; i <= (long)numNodes; i++) {
            SalNode snode = new SalNode(i);
            identifiers.add(snode.getNodeIdentifier());
            identifiers.add(snode.getVtnNodeIdentifier());
            for (long j = 1L; j <= nports; j++) {
                SalPort sport = new SalPort(i, j);
                identifiers.add(sport.getNodeConnectorIdentifier());
                identifiers.add(sport.getVtnPortIdentifier());
            }
        }
        final int numPorts = nports * numNodes;
        assertEquals(null, counter.put(Node.class, numNodes));
        assertEquals(null, counter.put(VtnNode.class, numNodes));
        assertEquals(null, counter.put(NodeConnector.class, numPorts));
        assertEquals(null, counter.put(VtnPort.class, numPorts));

        final int numPathPolicies = 5;
        VtnPortDesc[] vdescs = {
            new VtnPortDesc("openflow:1,,"),
            new VtnPortDesc("openflow:1,2,"),
            new VtnPortDesc("openflow:1,2,eth2"),
        };
        for (int i = 1; i <= numPathPolicies; i++) {
            identifiers.add(PathPolicyUtils.getIdentifier(i));
            for (VtnPortDesc vdesc: vdescs) {
                identifiers.add(PathPolicyUtils.getIdentifier(i, vdesc));
            }
        }
        final int numPathCosts = vdescs.length * numPathPolicies;
        assertEquals(null, counter.put(VtnPathPolicy.class, numPathPolicies));
        assertEquals(null, counter.put(VtnPathCost.class, numPathCosts));

        // Create a comparator with specifying order of targets.
        List<Class<? extends DataObject>> orderList = new ArrayList<>();
        IdentifierTargetComparator comp = createComparator(
            orderList, NodeConnector.class, Nodes.class, VtnPathPolicy.class,
            VtnNode.class, VtnPathCost.class, Node.class,
            VtnPathPolicies.class, VtnNodes.class, VtnPort.class);

        // Sort the list in ascending order.
        sortTest(comp, identifiers, counter, orderList);

        // Sort the list in descending order.
        Collections.reverse(orderList);
        sortTest(Collections.reverseOrder(comp), identifiers, counter,
                 orderList);

        // Change the order of target types and test again.
        comp = createComparator(
            orderList, VtnPort.class, VtnPathPolicies.class, VtnPathCost.class,
            NodeConnector.class, VtnNode.class, Node.class,
            VtnPathPolicy.class, Nodes.class, VtnNodes.class);
        sortTest(comp, identifiers, counter, orderList);
        Collections.reverse(orderList);
        sortTest(Collections.reverseOrder(comp), identifiers, counter,
                 orderList);
    }

    /**
     * Test or {@link IdentifierTargetComparator#equals(Object)} and
     * {@link IdentifierTargetComparator#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        testEquals(set, new IdentifierTargetComparator(),
                   new IdentifierTargetComparator());

        List<Class<? extends DataObject>> targets = new ArrayList<>();
        Collections.addAll(targets, Nodes.class, Node.class,
                           NodeConnector.class, VtnNodes.class, VtnNode.class,
                           VtnPort.class, VtnPathPolicies.class,
                           VtnPathPolicy.class, VtnPathCost.class);

        for (int i = 0; i < targets.size(); i++) {
            IdentifierTargetComparator c1 = new IdentifierTargetComparator();
            IdentifierTargetComparator c2 = new IdentifierTargetComparator();
            for (int j = 0; j <= i; j++) {
                Class<? extends DataObject> type = targets.get(j);
                c1.setOrder(type, j);
                c2.setOrder(type, j);
                if (j < i) {
                    assertTrue(set.contains(c1));
                    assertTrue(set.contains(c2));
                }
            }
            testEquals(set, c1, c2);
        }

        assertEquals(targets.size() + 1, set.size());
    }

    /**
     * Create an {@link IdentifierTargetComparator} instance.
     *
     * @param list   A list to store target types in order.
     * @param types  An array of target types.
     * @return  An {@link IdentifierTargetComparator} instance.
     */
    @SafeVarargs
    private static IdentifierTargetComparator createComparator(
        List<Class<? extends DataObject>> list,
        Class<? extends DataObject> ... types) {
        list.clear();

        IdentifierTargetComparator comp = new IdentifierTargetComparator();
        for (int i = 0; i < types.length; i++) {
            Class<? extends DataObject> type = types[i];
            assertEquals(null, comp.getOrder(type));

            // Associate order with target type except the last type.
            if (i != types.length - 1) {
                assertSame(comp, comp.setOrder(type, i));
                assertEquals(Integer.valueOf(i), comp.getOrder(type));
            }
            list.add(type);
        }

        return comp;
    }

    /**
     * Ensure that a list of {@link InstanceIdentifier} can be sorted by
     * the given comparator.
     *
     * @param comp         A {@link Comparator} used to sort the list.
     * @param identifiers  A list of {@link InstanceIdentifier} instances.
     * @param counter      A map that keeps the number of objects per target
     *                     type.
     * @param orderList    A list of target types which indicates expected
     *                     order.
     */
    private void sortTest(Comparator<InstanceIdentifier<?>> comp,
                          List<InstanceIdentifier<?>> identifiers,
                          Map<Class<? extends DataObject>, Integer> counter,
                          List<Class<? extends DataObject>> orderList) {
        Set<InstanceIdentifier<?>> idSet = new HashSet<>(identifiers);
        assertEquals(identifiers.size(), idSet.size());

        // Sort the list using the given comparator.
        Collections.sort(identifiers, comp);

        // Verify the result.
        Class<? extends DataObject> target = null;
        int count = 0;
        Iterator<Class<? extends DataObject>> orderIterator =
            orderList.iterator();
        for (InstanceIdentifier<?> id: identifiers) {
            if (target == null) {
                target = orderIterator.next();
                count = counter.get(target).intValue();
            }

            assertEquals(target, id.getTargetType());
            assertTrue(idSet.remove(id));
            count--;
            if (count == 0) {
                target = null;
            }
        }

        assertEquals(0, idSet.size());
        assertFalse(orderIterator.hasNext());
    }
}
