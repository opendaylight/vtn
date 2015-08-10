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
 * JUnit test for {@link ChangedData}.
 */
public class ChangedDataTest extends TestBase {
    /**
     * Test case for constructor and getter methods.
     *
     * <ul>
     *   <li>{@link ChangedData#create(Class,InstanceIdentifier,DataObject,DataObject)}</li>
     *   <li>{@link ChangedData#create(InstanceIdentifier,DataObject,DataObject)}</li>
     *   <li>{@link ChangedData#ChangedData(InstanceIdentifier,DataObject,DataObject)}</li>
     *   <li>{@link ChangedData#getIdentifier()}</li>
     *   <li>{@link ChangedData#getValue()}</li>
     *   <li>{@link ChangedData#checkType(Class)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testConstructor() throws Exception {
        SalPort sport = new SalPort(1L, 2L);
        InstanceIdentifier<VtnNode> nodePath = sport.getVtnNodeIdentifier();
        InstanceIdentifier<VtnPort> portPath = sport.getVtnPortIdentifier();
        VtnNode vnode1 = new VtnNodeBuilder().build();
        VtnNode vnode2 = new VtnNodeBuilder().build();
        VtnPort vport1 = new VtnPortBuilder().build();
        VtnPort vport2 = new VtnPortBuilder().build();

        ChangedData<VtnNode> nodeData =
            new ChangedData<VtnNode>(nodePath, vnode1, vnode2);
        assertSame(nodePath, nodeData.getIdentifier());
        assertSame(vnode1, nodeData.getValue());
        assertSame(vnode2, nodeData.getOldValue());
        assertSame(nodeData, nodeData.checkType(VtnNode.class));
        assertSame(null, nodeData.checkType(VtnPort.class));
        nodeData = ChangedData.create(VtnNode.class, nodePath, vnode1, vnode2);
        assertSame(nodePath, nodeData.getIdentifier());
        assertSame(vnode1, nodeData.getValue());
        assertSame(vnode2, nodeData.getOldValue());
        assertSame(nodeData, nodeData.checkType(VtnNode.class));
        assertSame(null, nodeData.checkType(VtnPort.class));
        nodeData = ChangedData.create(nodePath, vnode1, vnode2);
        assertSame(nodePath, nodeData.getIdentifier());
        assertSame(vnode1, nodeData.getValue());
        assertSame(vnode2, nodeData.getOldValue());
        assertSame(nodeData, nodeData.checkType(VtnNode.class));
        assertSame(null, nodeData.checkType(VtnPort.class));
        assertSame(null,
                   ChangedData.create(VtnNode.class, portPath, null, null));

        ChangedData<VtnPort> portData =
            new ChangedData<VtnPort>(portPath, vport1, vport2);
        assertSame(portPath, portData.getIdentifier());
        assertSame(vport1, portData.getValue());
        assertSame(vport2, portData.getOldValue());
        assertSame(portData, portData.checkType(VtnPort.class));
        assertSame(null, portData.checkType(VtnNode.class));
        portData = ChangedData.create(VtnPort.class, portPath, vport1, vport2);
        assertSame(portPath, portData.getIdentifier());
        assertSame(vport1, portData.getValue());
        assertSame(vport2, portData.getOldValue());
        assertSame(portData, portData.checkType(VtnPort.class));
        assertSame(null, portData.checkType(VtnNode.class));
        portData = ChangedData.create(portPath, vport1, vport2);
        assertSame(portPath, portData.getIdentifier());
        assertSame(vport1, portData.getValue());
        assertSame(vport2, portData.getOldValue());
        assertSame(portData, portData.checkType(VtnPort.class));
        assertSame(null, portData.checkType(VtnNode.class));
        assertSame(null,
                   ChangedData.create(VtnPort.class, nodePath, null, null));

        DataObject[] objs = {null, vport1, vport2};
        for (DataObject obj: objs) {
            try {
                ChangedData.create(nodePath, obj, vnode1);
                unexpected();
            } catch (DataTypeMismatchException e) {
                assertEquals(VtnNode.class, e.getTargetType());
                assertSame(obj, e.getObject());
            }

            try {
                ChangedData.create(nodePath, vnode1, obj);
                unexpected();
            } catch (DataTypeMismatchException e) {
                assertEquals(VtnNode.class, e.getTargetType());
                assertSame(obj, e.getObject());
            }

            try {
                ChangedData.create(VtnNode.class, nodePath, obj, vnode1);
                unexpected();
            } catch (DataTypeMismatchException e) {
                assertEquals(VtnNode.class, e.getTargetType());
                assertSame(obj, e.getObject());
            }

            try {
                ChangedData.create(VtnNode.class, nodePath, vnode1, obj);
                unexpected();
            } catch (DataTypeMismatchException e) {
                assertEquals(VtnNode.class, e.getTargetType());
                assertSame(obj, e.getObject());
            }
        }

        objs = new DataObject[]{null, vnode1, vnode2};
        for (DataObject obj: objs) {
            try {
                ChangedData.create(portPath, obj, vport1);
                unexpected();
            } catch (DataTypeMismatchException e) {
                assertEquals(VtnPort.class, e.getTargetType());
                assertSame(obj, e.getObject());
            }

            try {
                ChangedData.create(portPath, vport1, obj);
                unexpected();
            } catch (DataTypeMismatchException e) {
                assertEquals(VtnPort.class, e.getTargetType());
                assertSame(obj, e.getObject());
            }

            try {
                ChangedData.create(VtnPort.class, portPath, obj, vport1);
                unexpected();
            } catch (DataTypeMismatchException e) {
                assertEquals(VtnPort.class, e.getTargetType());
                assertSame(obj, e.getObject());
            }

            try {
                ChangedData.create(VtnPort.class, portPath, vport1, obj);
                unexpected();
            } catch (DataTypeMismatchException e) {
                assertEquals(VtnPort.class, e.getTargetType());
                assertSame(obj, e.getObject());
            }
        }

        try {
            ChangedData.create((InstanceIdentifier<VtnNode>)null, vnode1,
                               vnode2);
            unexpected();
        } catch (IllegalArgumentException e) {
        }
    }
}
