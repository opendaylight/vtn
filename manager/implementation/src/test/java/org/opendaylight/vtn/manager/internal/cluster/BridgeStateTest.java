/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.vtn.manager.VNodeState;
import org.opendaylight.vtn.manager.internal.RouteResolver;
import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.core.Edge;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.Path;
import org.opendaylight.controller.sal.routing.IRouting;

/**
 * JUnit test for {@link BridgeState}.
 */
public class BridgeStateTest extends TestBase {
    private Set<ObjectPair<Node, Node>> fp = new HashSet<ObjectPair<Node, Node>>();

    /**
     * Mock-up of the routing service.
     */
    private final class TestRouting implements IRouting, RouteResolver {
        /**
         * Current set of routing.
         */
        private final Map<ObjectPair<Node, Node>, Edge> routingMap =
            new HashMap<ObjectPair<Node, Node>, Edge>();

        /**
         * Add the given path to the current routing set.
         *
         * @param path  A node path.
         * @param edge  An edge.
         */
        private void addPath(ObjectPair<Node, Node> path, Edge edge) {
            routingMap.put(path, edge);
        }

        // RouteResolver

        // IRouting
        @Override
        public Path getRoute(Node src, Node dst) {
            ObjectPair<Node, Node> path = new ObjectPair<Node, Node>(src, dst);
            Edge edge = routingMap.get(path);
            if (edge == null) {
                return null;
            }

            ArrayList<Edge> list = new ArrayList<Edge>();
            list.add(edge);
            try {
                return new Path(list);
            } catch (Exception e) {
                unexpected(e);
            }
            return null;
        }

        @Override
        public Path getMaxThroughputRoute(Node src, Node dst) {
            return getRoute(src, dst);
        }

        @Override
        public Path getRoute(Node src, Node dst, Short bw) {
            return getRoute(src, dst);
        }

        @Override
        public void clear() {
            routingMap.clear();
        }

        @Override
        public void clearMaxThroughput() {
        }

        @Override
        public void initMaxThroughput(Map<Edge, Number> edgeWeightMap) {
        }
    }

    /**
     * Test case for constructor.
     */
    @Test
    public void testConstructor() {
        VNodeState[] states = VNodeState.values();

        for (VNodeState state: states) {
            BridgeState bst = new BridgeState(state);
            assertSame(state, bst.getState());
            assertEquals(bst.toString(), fp, bst.getFaultedPaths());
            assertEquals(bst.toString(), 0, bst.getFaultedPathSize());
            assertFalse(bst.toString(), bst.isDirty());
        }
    }

    /**
     * Test case for getter and setter methods.
     */
    @Test
    public void testGetSet() {
        VNodeState[] states = VNodeState.values();
        BridgeState bst = new BridgeState(states[states.length - 1]);

        // Change bridge state.
        for (VNodeState state: states) {
            assertSame(state, bst.setState(state));
            assertTrue(state.toString(), bst.isDirty());
            assertFalse(state.toString(), bst.isDirty());
        }

        // Add faulted paths.
        Set<ObjectPair<Node, Node>> faulted =
            new HashSet<ObjectPair<Node, Node>>();
        final int numEdges = 10;
        List<Edge> edgeList = createEdges(numEdges);
        for (Edge edge: edgeList) {
            ObjectPair<Node, Node> path = toNodePath(edge);
            Node src = path.getLeft();
            Node dst = path.getRight();
            String emsg = path.toString();
            assertTrue(emsg, faulted.add(path));
            assertTrue(bst.addFaultedPath(src, dst));
            assertTrue(emsg, bst.isDirty());
            assertFalse(emsg, bst.isDirty());
            assertSame(emsg, VNodeState.DOWN, bst.getState());

            // Try to add the same path.
            assertFalse(bst.addFaultedPath(src, dst));
            assertFalse(emsg, bst.isDirty());

            Set<ObjectPair<Node, Node>> paths = bst.getFaultedPaths();
            assertEquals(emsg, faulted, paths);

            // getFaultedPaths() must return a clone of the faulted path set.
            paths.clear();
            assertEquals(emsg, faulted, bst.getFaultedPaths());
        }

        // Resolve faulted paths.
        TestRouting routing = new TestRouting();
        final int half = numEdges >>> 1;
        Iterator<Edge> it = edgeList.iterator();
        for (int i = 0; i < half; i++) {
            assertTrue(bst.removeResolvedPath(routing).isEmpty());
            assertEquals(faulted, bst.getFaultedPaths());

            Edge edge = it.next();
            ObjectPair<Node, Node> path = toNodePath(edge);
            routing.addPath(path, edge);
            List<ObjectPair<Node, Node>> resolved =
                bst.removeResolvedPath(routing);
            String emsg = path.toString();
            assertTrue(emsg, bst.isDirty());
            assertFalse(emsg, bst.isDirty());
            assertEquals(emsg, 1, resolved.size());
            assertEquals(emsg, path, resolved.get(0));
            assertTrue(emsg, faulted.remove(path));
            assertEquals(emsg, faulted, bst.getFaultedPaths());
        }

        // Resolve the rest of faulted paths at a time.
        while (it.hasNext()) {
            Edge edge = it.next();
            ObjectPair<Node, Node> path = toNodePath(edge);
            routing.addPath(path, edge);
        }

        List<ObjectPair<Node, Node>> resolved =
            bst.removeResolvedPath(routing);
        Set<ObjectPair<Node, Node>> resolvedSet =
            new HashSet<ObjectPair<Node, Node>>(resolved);
        assertEquals(resolved.size(), resolvedSet.size());
        assertTrue(bst.isDirty());
        assertFalse(bst.isDirty());
        assertEquals(faulted, resolvedSet);
        assertTrue(bst.getFaultedPaths().isEmpty());
    }

    /**
     * Test case for {@link BridgeState#equals(Object)} and
     * {@link BridgeState#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        VNodeState[] states = VNodeState.values();

        for (VNodeState state: states) {
            BridgeState bst1 = new BridgeState(state);
            BridgeState bst2 = new BridgeState(state);
            testEquals(set, bst1, bst2);
        }

        List<Edge> edgeList = createEdges(10);
        for (int max = 0; max < edgeList.size(); max++) {
            BridgeState bst1 = new BridgeState(VNodeState.DOWN);
            BridgeState bst2 = new BridgeState(VNodeState.DOWN);
            for (int i = 0; i <= max; i++) {
                Edge edge = edgeList.get(i);
                ObjectPair<Node, Node> path = toNodePath(edge);
                Node src = path.getLeft();
                Node dst = path.getRight();
                assertTrue(bst1.addFaultedPath(src, dst));
                assertTrue(bst2.addFaultedPath(copy(src), copy(dst)));
            }
            testEquals(set, bst1, bst2);
        }

        int required = states.length + edgeList.size();
        assertEquals(required, set.size());

    }

    /**
     * Test case for {@link BridgeState#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "BridgeState[";
        String suffix = "]";

        VNodeState[] states = VNodeState.values();
        for (VNodeState state: states) {
            BridgeState bst = new BridgeState(state);
            String s = "state=" + state.toString();
            String f = "faulted=" + fp.toString();
            String required = joinStrings(prefix, suffix, ",", s, f);
            assertEquals(required, bst.toString());
        }

        List<Edge> edgeList = createEdges(10);
        VNodeState state = VNodeState.DOWN;
        for (int max = 0; max < edgeList.size(); max++) {
            BridgeState bst = new BridgeState(state);
            Set<ObjectPair<Node, Node>> faulted =
                new HashSet<ObjectPair<Node, Node>>();
            for (int i = 0; i <= max; i++) {
                Edge edge = edgeList.get(i);
                ObjectPair<Node, Node> path = toNodePath(edge);
                faulted.add(path);
                Node src = path.getLeft();
                Node dst = path.getRight();
                assertTrue(bst.addFaultedPath(src, dst));
            }

            String s = "state=" + state.toString();
            String f = "faulted=" + faulted.toString();
            String required = joinStrings(prefix, suffix, ",", s, f);
            assertEquals(required, bst.toString());
        }
    }

    /**
     * Ensure that {@link BridgeState} is serializable.
     */
    @Test
    public void testSerialize() {
        VNodeState[] states = VNodeState.values();
        for (VNodeState state: states) {
            BridgeState bst1 = new BridgeState(state);
            BridgeState bst2 = (BridgeState)serializeTest(bst1);
            assertFalse(bst1.isDirty());
            assertFalse(bst2.isDirty());
        }

        List<Edge> edgeList = createEdges(10);
        VNodeState state = VNodeState.DOWN;
        for (int max = 0; max < edgeList.size(); max++) {
            BridgeState bst1 = new BridgeState(state);
            Set<ObjectPair<Node, Node>> faulted =
                new HashSet<ObjectPair<Node, Node>>();
            for (int i = 0; i <= max; i++) {
                Edge edge = edgeList.get(i);
                ObjectPair<Node, Node> path = toNodePath(edge);
                faulted.add(path);
                Node src = path.getLeft();
                Node dst = path.getRight();
                assertTrue(bst1.addFaultedPath(src, dst));
            }

            BridgeState bst2 = (BridgeState)serializeTest(bst1);
            assertTrue(bst1.isDirty());
            assertFalse(bst1.isDirty());
            assertFalse(bst2.isDirty());
        }
    }

    /**
     * Convert an edge to node path.
     *
     * @param edge  An edge.
     * @return  A converted node path.
     */
    private ObjectPair<Node, Node> toNodePath(Edge edge) {
        Node src = edge.getTailNodeConnector().getNode();
        Node dst = edge.getHeadNodeConnector().getNode();
        return new ObjectPair<Node, Node>(src, dst);
    }
}
