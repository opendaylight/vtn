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
 * JUnit test for {@link IdentifiedData}.
 */
public class IdentifiedDataTest extends TestBase {
    /**
     * Test case for constructor and getter methods.
     *
     * <ul>
     *   <li>{@link IdentifiedData#IdentifiedData(InstanceIdentifier,DataObject)}</li>
     *   <li>{@link IdentifiedData#getIdentifier()}</li>
     *   <li>{@link IdentifiedData#getValue()}</li>
     *   <li>{@link IdentifiedData#checkType(Class)}</li>
     * </ul>
     */
    @Test
    public void testConstructor() {
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

        IdentifiedData<VtnPort> portData =
            new IdentifiedData<VtnPort>(portPath, vport);
        assertSame(portPath, portData.getIdentifier());
        assertSame(vport, portData.getValue());
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
                VtnPort vport = new VtnPortBuilder().build();
                IdentifiedData<VtnPort> data =
                    new IdentifiedData<>(path, vport);
                Logger logger = mock(Logger.class);
                data.unexpected(logger, type);
                verify(logger).
                    warn("Unexpected data: type={}, path={}, value={}",
                         type, path, vport);
                verifyNoMoreInteractions(logger);
            }
        }
    }
}
