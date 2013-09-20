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

import org.opendaylight.controller.sal.action.Action;
import org.opendaylight.controller.sal.action.Output;
import org.opendaylight.controller.sal.action.PopVlan;
import org.opendaylight.controller.sal.action.SetVlanId;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;

/**
 * List of actions in a flow entry.
 */
public class ActionList {
    /**
     * List of actions.
     */
    private final ArrayList<Action>  actionList = new ArrayList<Action>();

    /**
     * A node associated with the target SDN switch.
     */
    private final Node  node;

    /**
     * Construct a new action list.
     *
     * @param node  A node associated with the target SDN switch.
     */
    public ActionList(Node node) {
        this.node = node;
    }

    /**
     * Return a node associated with the target switch.
     *
     * @return  A node associated with the target switch.
     */
    public Node getNode() {
        return node;
    }

    /**
     * Return a list of actions.
     *
     * @return  A list of flow actions.
     */
    public List<Action> get() {
        return new ArrayList<Action>(actionList);
    }

    /**
     * Add an output action to the tail of the action list.
     *
     * @param port  A node connector associated with the output switch port.
     * @return  This object is always returned.
     */
    public ActionList addOutput(NodeConnector port) {
        assert node.equals(port.getNode());
        actionList.add(new Output(port));
        return this;
    }

    /**
     * Add an action which sets the specified VLAN ID into the outgoing
     * packet.
     *
     * @param vlan  A VLAN ID. If zero is specified, the VLAN tag in the
     *              outgoing packet will be stripped.
     * @return  This object is always returned.
     */
    public ActionList addVlanId(short vlan) {
        if (vlan == 0) {
            actionList.add(new PopVlan());
        } else if (vlan > 0) {
            actionList.add(new SetVlanId((int)vlan));
        }
        return this;
    }
}
