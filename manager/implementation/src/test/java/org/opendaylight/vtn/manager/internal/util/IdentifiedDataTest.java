/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNodeBuilder;

/**
 * JUnit test for {@link IdentifiedData}.
 */
public class IdentifiedDataTest extends TestBase {
    /**
     * Test case for constructor and getter methods.
     *
     * <ul>
     *   <li>{@link IdentifiedData#create(Class,InstanceIdentifier,DataObject)}</li>
     *   <li>{@link IdentifiedData#create(InstanceIdentifier,DataObject)}</li>
     *   <li>{@link IdentifiedData#IdentifiedData(InstanceIdentifier,DataObject)}</li>
     *   <li>{@link IdentifiedData#getIdentifier()}</li>
     *   <li>{@link IdentifiedData#getValue()}</li>
     *   <li>{@link IdentifiedData#checkType(Class)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testConstructor() throws Exception {
        SalPort sport = new SalPort(1L, 2L);
        InstanceIdentifier<VtnNode> nodePath = sport.getVtnNodeIdentifier();
        InstanceIdentifier<VtnPort> portPath = sport.getVtnPortIdentifier();
        VtnNode vnode = new VtnNodeBuilder().build();
        VtnPort vport = new VtnPortBuilder().build();

        IdentifiedData<VtnNode> nodeData =
            new IdentifiedData<VtnNode>(nodePath, vnode);
        assertSame(nodePath, nodeData.getIdentifier());
        assertSame(vnode, nodeData.getValue());
        assertSame(nodeData, nodeData.checkType(VtnNode.class));
        assertSame(null, nodeData.checkType(VtnPort.class));
        nodeData = IdentifiedData.create(VtnNode.class, nodePath, vnode);
        assertSame(nodePath, nodeData.getIdentifier());
        assertSame(vnode, nodeData.getValue());
        assertSame(nodeData, nodeData.checkType(VtnNode.class));
        assertSame(null, nodeData.checkType(VtnPort.class));
        nodeData = IdentifiedData.create(nodePath, vnode);
        assertSame(nodePath, nodeData.getIdentifier());
        assertSame(vnode, nodeData.getValue());
        assertSame(nodeData, nodeData.checkType(VtnNode.class));
        assertSame(null, nodeData.checkType(VtnPort.class));
        assertSame(null, IdentifiedData.create(VtnNode.class, portPath, null));

        IdentifiedData<VtnPort> portData =
            new IdentifiedData<VtnPort>(portPath, vport);
        assertSame(portPath, portData.getIdentifier());
        assertSame(vport, portData.getValue());
        assertSame(portData, portData.checkType(VtnPort.class));
        assertSame(null, portData.checkType(VtnNode.class));
        portData = IdentifiedData.create(VtnPort.class, portPath, vport);
        assertSame(portPath, portData.getIdentifier());
        assertSame(vport, portData.getValue());
        assertSame(portData, portData.checkType(VtnPort.class));
        assertSame(null, portData.checkType(VtnNode.class));
        portData = IdentifiedData.create(portPath, vport);
        assertSame(portPath, portData.getIdentifier());
        assertSame(vport, portData.getValue());
        assertSame(portData, portData.checkType(VtnPort.class));
        assertSame(null, portData.checkType(VtnNode.class));
        assertSame(null, IdentifiedData.create(VtnPort.class, nodePath, null));

        DataObject[] objs = {null, vport};
        for (DataObject obj: objs) {
            try {
                IdentifiedData.create(nodePath, obj);
                unexpected();
            } catch (DataTypeMismatchException e) {
                assertEquals(VtnNode.class, e.getTargetType());
                assertSame(obj, e.getObject());
            }

            try {
                IdentifiedData.create(VtnNode.class, nodePath, obj);
                unexpected();
            } catch (DataTypeMismatchException e) {
                assertEquals(VtnNode.class, e.getTargetType());
                assertSame(obj, e.getObject());
            }
        }

        objs = new DataObject[]{null, vnode};
        for (DataObject obj: objs) {
            try {
                IdentifiedData.create(portPath, obj);
                unexpected();
            } catch (DataTypeMismatchException e) {
                assertEquals(VtnPort.class, e.getTargetType());
                assertSame(obj, e.getObject());
            }

            try {
                IdentifiedData.create(VtnPort.class, portPath, obj);
                unexpected();
            } catch (DataTypeMismatchException e) {
                assertEquals(VtnPort.class, e.getTargetType());
                assertSame(obj, e.getObject());
            }
        }

        try {
            IdentifiedData.create((InstanceIdentifier<VtnNode>)null, vnode);
            unexpected();
        } catch (IllegalArgumentException e) {
        }
    }
}
