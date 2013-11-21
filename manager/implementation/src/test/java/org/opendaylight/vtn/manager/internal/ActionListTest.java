/*
 * Copyright (c) 2013 NEC Corporation
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
import org.opendaylight.controller.sal.action.SetVlanId;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
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
        short[] vlans = new short[] {-1, 0, 1, 100, 4095, 4096};

        for (short vlan : vlans) {
            for (Node node : createNodes(4)) {
                ActionList actList = new ActionList(node);
                List<Action> actions = new ArrayList<Action>();
                String emsg = ((node == null) ? "(Node)null" : node.toString())
                              + "(vlan)" + vlan;
                assertEquals(emsg, node, actList.getNode());
                assertEquals(emsg, actions, actList.get());

                if (node == null) {
                    continue;
                }

                actList.addVlanId(vlan);
                int numOutput = 1;

                List<Action> regActs = actList.get();
                regActs = actList.get();
                if (vlan == 0) {
                    assertTrue(emsg, regActs.contains(new PopVlan()));
                } else if (vlan > 0){
                    assertTrue(emsg, regActs.contains(new SetVlanId((int) vlan)));
                } else {
                    numOutput--;
                }
                assertEquals(emsg, numOutput, regActs.size());


                for (short i = 0; i < 5; i++) {
                    Object connId = null;
                    if (node.getType().equals(Node.NodeIDType.OPENFLOW)) {
                        connId = new Short((short)(i + 10));
                    } else {
                        connId = "Node Connector ID: " + i;
                    }
                    NodeConnector port
                        = NodeConnectorCreator.createNodeConnector(node.getType(),
                                                                   connId, node);
                    actList.addOutput(port);
                    numOutput++;

                    emsg = "(NodeConnector)" + port.toString() + ",(vlan)" + vlan;
                    regActs = actList.get();
                    assertTrue(emsg, regActs.contains(new Output(port)));
                    assertEquals(emsg, numOutput, regActs.size());
                }
            }
        }
    }
}
