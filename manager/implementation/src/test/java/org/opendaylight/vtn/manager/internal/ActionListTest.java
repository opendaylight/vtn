/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal;

import java.util.ArrayList;
import java.util.List;

import org.junit.Test;
import org.opendaylight.controller.sal.action.Action;
import org.opendaylight.controller.sal.action.Output;
import org.opendaylight.controller.sal.action.PopVlan;
import org.opendaylight.controller.sal.action.PushVlan;
import org.opendaylight.controller.sal.action.SetVlanId;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;

/**
 * JUnit test for {@link ActionList}
 */
public class ActionListTest extends TestBase {

    /**
     * Test method for methods implemented in {@link ActionList}.
     */
    @Test
    public void testActionList() {
        short[] vlans = new short[] {0, 1, 100, 4095};

        for (short vlan : vlans) {
            short newVlan = (short)((vlan + 1) & 0xfff);
            for (Node node : createNodes(4)) {
                ActionList actList = new ActionList(node, vlan);
                List<Action> actions = new ArrayList<Action>();
                String emsg = ((node == null) ? "(Node)null" : node.toString())
                              + "(vlan)" + vlan;
                assertEquals(emsg, node, actList.getNode());
                assertEquals(emsg, vlan, actList.getOriginalVlan());
                assertEquals(emsg, actions, actList.get());

                if (node == null) {
                    continue;
                }

                // This should never add any action.
                actList.addVlanId(vlan);
                assertEquals(emsg, actions, actList.get());

                actList.addVlanId(newVlan);
                List<Action> expected = new ArrayList<Action>();
                if (newVlan == 0) {
                    expected.add(new PopVlan());
                } else {
                    if (vlan == 0) {
                        expected.add(new PushVlan(EtherTypes.VLANTAGGED));
                    }
                    expected.add(new SetVlanId((int)newVlan));
                }
                assertEquals(emsg, expected, actList.get());

                for (short i = 0; i < 5; i++) {
                    Object connId = null;
                    if (node.getType().equals(Node.NodeIDType.OPENFLOW)) {
                        connId = new Short((short)(i + 10));
                    } else {
                        connId = "Node Connector ID: " + i;
                    }
                    NodeConnector port = NodeConnectorCreator.
                        createNodeConnector(node.getType(), connId, node);
                    actList.addOutput(port);

                    emsg = "(NodeConnector)" + port.toString() + ",(vlan)" +
                        vlan;
                    expected.add(new Output(port));
                    assertEquals(emsg, expected, actList.get());
                }
            }
        }
    }
}
