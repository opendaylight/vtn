/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock;

import java.math.BigInteger;
import java.util.List;
import java.util.Set;
import java.util.concurrent.TimeoutException;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.controller.md.sal.binding.api.ReadOnlyTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.yangtools.yang.binding.RpcService;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.Match;

/**
 * This interface defines an OSGi service to handle the mock-up of
 * openflowplugin.
 */
public interface OfMockService {
    /**
     * Separator in a MD-SAL node or node connector identifier.
     */
    String  ID_SEPARATOR = ":";

    /**
     * Protocol prefix for OpenFlow switch.
     */
    String  ID_OPENFLOW = "openflow" + ID_SEPARATOR;

    /**
     * Default OpenFlow table ID.
     */
    int  DEFAULT_TABLE = 0;

    /**
     * Initialize the integration test environment.
     *
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    void initialize() throws InterruptedException;

    /**
     * Reset the inventory configuration to the initial state.
     *
     * @throws InterruptedException
     *    The calling thread was interrupted.
     * @throws TimeoutException
     *    At least one flow entry is present in a switch.
     */
    void reset() throws InterruptedException, TimeoutException;

    /**
     * Create a new OpenFlow node.
     *
     * <p>
     *   This method blocks the calling thread until the VTN Manager detects
     *   the change of the inventory information.
     * </p>
     * <p>
     *   All nodes created by this method will be removed by subsequent call
     *   of {@link #reset()}.
     * </p>
     *
     * @param dpid    Datapath ID of the node.
     * @return  The node identifier of the created node.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    String addNode(BigInteger dpid) throws InterruptedException;

    /**
     * Create a new OpenFlow node.
     *
     * <p>
     *   This method blocks the calling thread until the VTN Manager detects
     *   the change of the inventory information.
     * </p>
     * <p>
     *   All nodes created by this method will be removed by subsequent call
     *   of {@link #reset()}.
     * </p>
     *
     * @param dpid  Datapath ID of the node.
     * @param sync  If {@code true}, this method blocks the calling thread
     *              until the VTN Manager detects the created node.
     * @return  The node identifier of the created node.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    String addNode(BigInteger dpid, boolean sync) throws InterruptedException;

    /**
     * Create a new node.
     *
     * <p>
     *   This method blocks the calling thread until the VTN Manager detects
     *   the change of the inventory information.
     * </p>
     * <p>
     *   All nodes created by this method will be removed by subsequent call
     *   of {@link #reset()}.
     * </p>
     *
     * @param prefix  Protocol prefix of the node.
     * @param dpid    Datapath ID of the node.
     * @return  The node identifier of the created node.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    String addNode(String prefix, BigInteger dpid) throws InterruptedException;

    /**
     * Create a new node.
     *
     * <p>
     *   This method blocks the calling thread until the VTN Manager detects
     *   the change of the inventory information.
     * </p>
     * <p>
     *   All nodes created by this method will be removed by subsequent call
     *   of {@link #reset()}.
     * </p>
     *
     * @param prefix  Protocol prefix of the node.
     * @param dpid    Datapath ID of the node.
     * @param sync    If {@code true}, this method blocks the calling thread
     *                until the VTN Manager detects the created node.
     * @return  The node identifier of the created node.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    String addNode(String prefix, BigInteger dpid, boolean sync)
        throws InterruptedException;

    /**
     * Remove the specified node.
     *
     * <p>
     *   This method blocks the calling thread until the VTN Manager detects
     *   the change of the inventory information.
     * </p>
     *
     * @param nid   The identifier of the MD-SAL node.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    void removeNode(String nid) throws InterruptedException;

    /**
     * Add a physical switch port to the given swtich.
     *
     * <p>
     *   This method blocks the calling thread until the VTN Manager detects
     *   the change of the inventory information.
     * </p>
     * <p>
     *   Note that any port cannot be added to nodes created by the call of
     *   {@link #initialize()}.
     * </p>
     *
     * @param nid   The identifier of the MD-SAL node.
     * @param idx   Index of a new port.
     * @return  The MD-SAL node connector identifier for a new port.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    String addPort(String nid, long idx) throws InterruptedException;

    /**
     * Add a physical switch port to the given swtich.
     *
     * <p>
     *   Note that any port cannot be added to nodes created by the call of
     *   {@link #initialize()}.
     * </p>
     *
     * @param nid   The identifier of the MD-SAL node.
     * @param idx   Index of a new port.
     * @param sync  If {@code true}, this method blocks the calling thread
     *              until the VTN Manager detects the change of the
     *              inventory information.
     * @return  The MD-SAL node connector identifier for a new port.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    String addPort(String nid, long idx, boolean sync)
        throws InterruptedException;

    /**
     * Block the calling thread until the VTN Manager detects the creation of
     * the given port.
     *
     * @param pid  The identifier of the MD-SAL node connector.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    void awaitPortCreated(String pid) throws InterruptedException;

    /**
     * Remove the specified switch port.
     *
     * <p>
     *   This method blocks the calling thread until the VTN Manager detects
     *   the change of the inventory information.
     * </p>
     *
     * @param pid  The identifier of the MD-SAL node connector.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    void removePort(String pid) throws InterruptedException;

    /**
     * Remove the specified switch port.
     *
     * @param pid   The identifier of the MD-SAL node connector.
     * @param sync  If {@code true}, this method blocks the calling thread
     *              until the VTN Manager detects the change of the
     *              inventory information.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    void removePort(String pid, boolean sync) throws InterruptedException;

    /**
     * Block the calling thread until the VTN Manager detects the removal of
     * the given port.
     *
     * @param pid  The identifier of the MD-SAL node connector.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    void awaitPortRemoved(String pid) throws InterruptedException;

    /**
     * Return the peer switch port connected to the given port.
     *
     * @param pid  The identifier of the MD-SAL node connector.
     * @return  The identifier for the peer port.
     *          {@code null} if the given port is an edge port.
     */
    String getPeerIdentifier(String pid);

    /**
     * Establish an inter-switch link.
     *
     * <p>
     *   This method blocks the calling thread until the VTN Manager detects
     *   the change of network topology.
     * </p>
     *
     * @param pid   The identifier of the MD-SAL node connector.
     * @param peer  The identifier for the peer port.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    void setPeerIdentifier(String pid, String peer)
        throws InterruptedException;

    /**
     * Establish an inter-switch link.
     *
     * @param pid   The identifier of the MD-SAL node connector.
     * @param peer  The identifier for the peer port.
     * @param sync  If {@code true}, this method blocks the calling thread
     *              until the VTN Manager detects the change of network
     *              topology.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    void setPeerIdentifier(String pid, String peer, boolean sync)
        throws InterruptedException;

    /**
     * Return a list of switches.
     *
     * @return  A list of MD-SAL node identifiers.
     */
    List<String> getNodes();

    /**
     * Return a list of physical ports in the given switch.
     *
     * @param nid   The identifier of the MD-SAL node.
     * @param edge  Return a list of edge ports if {@code true}.
     *              Return a list of ISL ports if {@code false}.
     * @return  A list of MD-SAL node connector identifier.
     */
    List<String> getPorts(String nid, boolean edge);

    /**
     * Return the name of the given port.
     *
     * @param pid  The identifier of the MD-SAL node connector.
     * @return  The name of the given port.
     */
    String getPortName(String pid);

    /**
     * Generate a PACKET_IN message.
     *
     * @param ingress  The port identifier which specifies the ingress port.
     * @param payload  A byte array which represents the packet image.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    void sendPacketIn(String ingress, byte[] payload)
        throws InterruptedException;

    /**
     * Dequeue a packet from the packet transmission queue in the given
     * physical port.
     *
     * @param pid    The identifier of the MD-SAL node connector.
     * @return  A byte array which represents the packet if at least one packet
     *          is present in the packet transmission queue.
     *          {@code null} if no packet is present.
     */
    byte[] getTransmittedPacket(String pid);

    /**
     * Wait for a packet to be put on the packet transmission queue in the
     * given physical port.
     *
     * @param pid    The identifier of the MD-SAL node connector.
     * @return  A byte array which represents the packet if at least one packet
     *          is present in the packet transmission queue.
     *          {@code null} if no packet is present.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    byte[] awaitTransmittedPacket(String pid) throws InterruptedException;

    /**
     * Clear packet transaction queue in all nodes.
     */
    void clearTransmittedPacket();

    /**
     * Set the state of the given physical switch port.
     *
     * <p>
     *   This method blocks the calling thread until the VTN Manager detects
     *   the change of the link state.
     * </p>
     *
     * @param pid    The identifier of the MD-SAL node connector.
     * @param state  The port is up if {@code true} is specified.
     *               The port is down if {@code false} is specified.
     * @return  {@code true} if the port state has been changed.
     *          {@code false} not changed.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    boolean setPortState(String pid, boolean state)
        throws InterruptedException;

    /**
     * Set the state of the given physical switch port.
     *
     * @param pid    The identifier of the MD-SAL node connector.
     * @param state  The port is up if {@code true} is specified.
     *               The port is down if {@code false} is specified.
     * @param sync   If {@code true}, this method blocks the calling thread
     *               until the VTN Manager detects the change of the
     *               inventory information.
     * @return  {@code true} if the port state has been changed.
     *          {@code false} not changed.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    boolean setPortState(String pid, boolean state, boolean sync)
        throws InterruptedException;

    /**
     * Block the calling thread untl the VTN Mamager detects the change of the
     * link state.
     *
     * @param pid    The identifier of the MD-SAL node connector.
     * @param state  The expected link state.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    void awaitLinkState(String pid, boolean state) throws InterruptedException;

    /**
     * Wait for the given inter-switch link to be created or removed.
     *
     * @param src    The source port identifier of the inter-switch link.
     * @param dst    The destination port identifier of the inter-switch link.
     *               {@code null} matches any ports.
     * @param state  If {@code true}, this method waits for the specified
     *               link to be created.
     *               If {@code false}, this method waits for the specified
     *               link to be removed.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     * @throws IllegalStateException
     *    The specified link did not created or removed.
     */
    void awaitLink(String src, String dst, boolean state)
        throws InterruptedException;

    /**
     * Wait for the topology graph to be updated to the given topology.
     *
     * @param topo  A set of {@link OfMockLink} instances which indicates the
     *              expected topology.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     * @throws IllegalStateException
     *    The topology graph was not updated to the given topology.
     */
    void awaitTopology(Set<OfMockLink> topo) throws InterruptedException;

    /**
     * Return the flow entry specified by the given match and priority.
     *
     * @param nid    The identifier of the MD-SAL node.
     * @param table  The flow table identifier.
     * @param match  A {@link Match} instance.
     * @param pri    A priority value.
     * @return  An {@link OfMockFlow} instance if found.
     *          {@code null} if not found.
     */
    OfMockFlow getFlow(String nid, int table, Match match, int pri);

    /**
     * Wait for the flow entry specified by the given match and priority is
     * installed or uninstalled.
     *
     * @param nid        The identifier of the MD-SAL node.
     * @param table      The flow table identifier.
     * @param match      A {@link Match} instance.
     * @param pri        A priority value.
     * @param installed  If {@code true}, this method waits for the given
     *                   flow entry to be installed.
     *                   If {@code false}, this method waits for the given
     *                   flow entry to be uninstalled.
     * @return  An {@link OfMockFlow} instance.
     *          {@code null} if the specified flow entry was not installed.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    OfMockFlow awaitFlow(String nid, int table, Match match, int pri,
                         boolean installed)
        throws InterruptedException;

    /**
     * Return the number of flow entries in the given switch.
     *
     * @param nid    The identifier of the MD-SAL node.
     * @return  The number of flow entries in the given switch.
     */
    int getFlowCount(String nid);

    /**
     * Return MAC address of the controller used as source MAC address of
     * ARP packet.
     *
     * @return  An {@link EtherAddress} that represents a MAC address.
     * @throws Exception  An error occurred.
     */
    EtherAddress getControllerMacAddress() throws Exception;

    /**
     * Return flow priority for layer 2 flow.
     *
     * @return  A flow priority value for layer 2 flow.
     * @throws Exception  An error occurred.
     */
    int getL2FlowPriority() throws Exception;

    /**
     * Return the packet route from the source to the destination switch.
     *
     * @param src  A MD-SAL node identifier corresponding to the source
     *             switch.
     * @param dst  A {MD-SAL node identifier corresponding to the destination
     *             switch.
     * @return     A list of {@link OfMockLink} instances which represents the
     *             packet route.
     *             An empty list is returned if the destination node is the
     *             same as the source.
     *             {@code null} is returned if no route was found.
     */
    List<OfMockLink> getRoute(String src, String dst);

    /**
     * Create a new read-only transaction for the MD-SAL datastore.
     *
     * @return  A {@link ReadOnlyTransaction} instance.
     */
    ReadOnlyTransaction newReadOnlyTransaction();

    /**
     * Create a new read-write transaction for the MD-SAL datastore.
     *
     * @return  A {@link ReadWriteTransaction} instance.
     */
    ReadWriteTransaction newReadWriteTransaction();

    /**
     * Create a new {@link DataChangeWaiter} instance to detect changes made
     * to the specified data object.
     *
     * @param store  The type of the logical data store.
     * @param path   Path to the target data object.
     * @param <T>    The type of the target data object.
     * @return  A {@link DataChangeWaiter} instance.
     */
    <T extends DataObject> DataChangeWaiter<T> newDataChangeWaiter(
        LogicalDatastoreType store, InstanceIdentifier<T> path);

    /**
     * Return an implementation of the specified RPC service.
     *
     * @param type  A class which specifies the RPC service.
     * @param <T>   The type of the RPC service.
     * @return  The proxy for the given RPC service.
     */
    <T extends RpcService> T getRpcService(Class<T> type);
}
