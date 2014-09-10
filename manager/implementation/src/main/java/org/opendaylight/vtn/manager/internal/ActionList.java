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
import java.util.Collection;
import java.util.List;

import org.opendaylight.controller.sal.action.Action;
import org.opendaylight.controller.sal.action.Output;
import org.opendaylight.controller.sal.action.PopVlan;
import org.opendaylight.controller.sal.action.PushVlan;
import org.opendaylight.controller.sal.action.SetVlanId;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.utils.EtherTypes;

/**
 * List of actions in a flow entry.
 */
public class ActionList {
    /**
     * List of actions.
     */
    private final List<Action>  actionList = new ArrayList<Action>();

    /**
     * A node associated with the target SDN switch.
     */
    private final Node  node;

    /**
     * VLAN ID of the original Ethernet frame.
     */
    private final short originalVlan;

    /**
     * Construct a new action list.
     *
     * @param node  A node associated with the target SDN switch.
     * @param vlan  VLAN ID of the original Ethernet frame.
     *              {@link MatchType#DL_VLAN_NONE} means that the original
     *              Ethernet frame has no VLAN tag.
     */
    public ActionList(Node node, short vlan) {
        this.node = node;
        originalVlan = vlan;
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
     * Return a VLAN ID of the original Ethernet frame.
     *
     * @return  A VLAN ID of the original Ethernet frame.
     *          {@link MatchType#DL_VLAN_NONE} is returned if the original
     *          Ethernet frame has no VLAN tag.
     */
    public short getOriginalVlan() {
        return originalVlan;
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
     * Append all SAL actions in the given collection to the tail of the
     * action list.
     *
     * @param c  A collection of SAL actions.
     * @return  This object is always returned.
     */
    public ActionList addAll(Collection<? extends Action> c) {
        if (c != null) {
            actionList.addAll(c);
        }
        return this;
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
        // Don't append action if not needed.
        if (vlan != originalVlan) {
            if (vlan == MatchType.DL_VLAN_NONE) {
                actionList.add(new PopVlan());
            } else {
                if (originalVlan == MatchType.DL_VLAN_NONE) {
                    // Add a new VLAN tag.
                    actionList.add(new PushVlan(EtherTypes.VLANTAGGED));
                }
                actionList.add(new SetVlanId((int)vlan));
            }
        }
        return this;
    }
}
