/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;

import org.slf4j.Logger;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNodeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * JUnit test for {@link ChangedData}.
 */
public class ChangedDataTest extends TestBase {
    /**
     * Test case for constructor and getter methods.
     *
     * <ul>
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

        ChangedData<VtnPort> portData =
            new ChangedData<VtnPort>(portPath, vport1, vport2);
        assertSame(portPath, portData.getIdentifier());
        assertSame(vport1, portData.getValue());
        assertSame(vport2, portData.getOldValue());
        assertSame(portData, portData.checkType(VtnPort.class));
        assertSame(null, portData.checkType(VtnNode.class));
    }

    /**
     * Test case for {@link IdentifiedData#unexpected(Logger, VtnUpdateType)}.
     */
    @Test
    public void testUnexpected() {
        SalPort[] ports = {
            new SalPort(1L, 1L),
            new SalPort(1L, 2L),
            new SalPort(2L, 2L),
        };
        VtnUpdateType[] types = VtnUpdateType.values();
        for (SalPort sport: ports) {
            InstanceIdentifier<VtnPort> path = sport.getVtnPortIdentifier();
            for (VtnUpdateType type: types) {
                VtnPort old = new VtnPortBuilder().build();
                VtnPort vport = new VtnPortBuilder().
                    setId(sport.getNodeConnectorId()).
                    setName("port:" + type).
                    build();
                ChangedData<VtnPort> data =
                    new ChangedData<>(path, vport, old);
                Logger logger = mock(Logger.class);
                data.unexpected(logger, type);
                verify(logger).
                    warn("Unexpected data: type={}, path={}, old={}, new={}",
                         type, path, old, vport);
                verifyNoMoreInteractions(logger);
            }
        }
    }
}
