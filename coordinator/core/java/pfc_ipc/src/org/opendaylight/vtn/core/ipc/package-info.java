/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * <p>
 *   This package contains Java bindings for PFC IPC framework APIs.
 *   All classes in this package are provided by the JAR file
 *   <strong>pfc_ipc.jar</strong>.
 * </p>
 * <ul>
 *   <li>
 *     Currently, this package contains APIs only for IPC client.
 *     Java bindings for IPC server is not yet supported.
 *   </li>
 *   <li>
 *     Java bindings for IPC framework supports only one VM in a process.
 *     It may not work if it is used in more than one VM in the same process.
 *   </li>
 * </ul>
 *
 * <h3>IPC Client</h3>
 *
 * <h4>IPC Service</h4>
 * <p>
 *   IPC service is a service procedure provided by IPC server process.
 *   One IPC server can provide more than one IPC services.
 *   IPC client can invoke IPC service on IPC server.
 * </p>
 * <ul>
 *   <li>
 *     The IPC server process is identified by
 *     <strong>IPC channel address</strong>. It is represented by an instance
 *     of {@link org.opendaylight.vtn.core.ipc.ChannelAddress}.
 *   </li>
 *   <li>
 *     The IPC service provided by the IPC server is identified by a pair
 *     of <strong>IPC service name</strong> and
 *     <strong>IPC service ID</strong>. IPC service name is a human readable
 *     string, and IPC service ID is an unsigned 32-bit integer.
 *   </li>
 * </ul>
 * <p>
 *   To invoke IPC service, an {@link org.opendaylight.vtn.core.ipc.IpcConnection}
 *   instance is required. An {@link org.opendaylight.vtn.core.ipc.IpcConnection}
 *   instance represents a pseudo connection between IPC client and IPC server.
 *   It has two implementations.
 * </p>
 * <ul>
 *   <li>
 *     {@link org.opendaylight.vtn.core.ipc.DefaultConnection} is a single global
 *     connection. If your client connects with only one IPC server, and
 *     there is no need to invoke IPC services in parallel, it is recommended
 *     to use {@link org.opendaylight.vtn.core.ipc.DefaultConnection}.
 *   </li>
 *   <li>
 *     Otherwise use of {@link org.opendaylight.vtn.core.ipc.AltConnection} is a
 *     recommended way to connect with IPC server.
 *   </li>
 * </ul>
 * <p>
 *   A {@link org.opendaylight.vtn.core.ipc.ClientSession} instance, which is
 *   a context of IPC service session, is also required to invoke IPC service.
 *   It can be created by
 *   {@link org.opendaylight.vtn.core.ipc.IpcConnection#createSession(String, int)}
 *   or
 *   {@link org.opendaylight.vtn.core.ipc.IpcConnection#createSession(String, int, int)}.
 * </p>
 * <p>
 *   An IPC service request can send one or more than one additional data
 *   represented by array of {@link org.opendaylight.vtn.core.ipc.IpcDataUnit}
 *   instances. And an IPC service on IPC server may also send response
 *   as array of {@link org.opendaylight.vtn.core.ipc.IpcDataUnit}.
 * </p>
 * <p>
 *   Below are pseudo code to invoke an IPC service using an alternative
 *   connection.
 * <p>
 * <blockquote><pre>
 * try {
 *     // Create an alternative connection to connect with the IPC server
 *     // identified by "foo" on the local host.
 *     ChannelAddress chaddr = new ChannelAddress("foo");
 *     AltConnection conn = AltConnection.open(chaddr);
 *
 *     // Create an IPC client session to invoke IPC service identified by
 *     // the IPC service name "bar" and ID 1.
 *     ClientSession sess = conn.createSession("bar", 1);
 *
 *     // Send additional data to the IPC service.
 *     sess.addOutput(new IpcString("a string"));    // String
 *     sess.addOutput(new IpcUint32(0x123));         // Unsigned 32-bit integer
 *
 *     // Invoke an IPC service.
 *     // Return value is a response code sent by the IPC service.
 *     int response = sess.invoke();
 *     if (response == ClientSession.RESP_FATAL) {
 *         // Error.
 *     }
 *
 *     // Get additional data sent by the IPC service.
 *     int count = sess.getResponseCount();   // The number of additional data.
 *     IpcInt32  i32 = (IpcInt32)sess.getResponse(0);  // Signed 32-bit integer
 *     IpcBinary bin = (IpcBinary)sess.getResponse(1); // Binary data
 *
 *     // Destroy client session and connection explicitly.
 *     sess.destroy();
 *     conn.close();
 * }
 * catch (IpcException e) {
 *     // Error.
 * }</pre>
 * </blockquote>
 *
 * <h4>IPC Event</h4>
 * <p>
 *   The IPC event subsystem provides interfaces to listen events on one or
 *   more than one IPC servers asynchronously.
 *   The IPC event subsystem is independent of other features in the IPC
 *   client library. The {@link org.opendaylight.vtn.core.ipc.IpcEventSystem}
 *   class is provided to control the IPC event subsystem.
 * </p>
 * <p>
 *   The IPC event subsystem must be initialized only once before use.
 *   Below pseudo code initializes the IPC event subsystem with default
 *   parameters.
 * </p>
 * <blockquote><pre>
 * IpcEventSystem esys = IpcEventSystem.getInstance();
 * try {
 *     esys.initialize();
 * }
 * catch (IpcExcepeion e) {
 *     // Error.
 * }</pre>
 * </blockquote>
 * <p>
 *   An event on the IPC server is represented by an instance of
 *   {@link org.opendaylight.vtn.core.ipc.IpcEvent} instance.
 *   To listen IPC events, at least one IPC event handler must be added to
 *   the IPC event subsystem. IPC event handler is an instance which implements
 *   {@link org.opendaylight.vtn.core.ipc.IpcEventHandler}.
 *   {@link org.opendaylight.vtn.core.ipc.IpcEventAttribute} can be used to
 *   specify attributes of the IPC event handler.
 *   Below pseudo code illustrates how IPC event handler is added to the
 *   IPC event subsystem.
 * </p>
 * <blockquote><pre>
 * // Create an IPC event handler which listens events on the IPC server.
 * IpcEventHandler handler = new IpcEventHandler() {
 *     public void eventReceived(IpcEvent event)
 *     {
 *         String service = event.getServiceName();
 *         if (service.equals("serv1")) {
 *             // Handle an event generated by IPC service "serv1".
 *             int type = event.getType();
 *             if (type == 0) {
 *                 // Handle event type 0.
 *             }
 *             else {
 *                 // Handle event type 1.
 *             }
 *         }
 *         else {
 *             // Handle an event generated by IPC service "serv2".
 *             // Type of this event must be 3 because other types of events
 *             // are filtered out by the IPC event subsystem.
 *         }
 *
 *         // An IPC event may contain additional data array.
 *         // Additional data can be derived via pseudo ClientSession
 *         // in the IPC event.
 *         ClientSession sess = event.getSession();
 *
 *         IpcFloat f = (IpcFloat)sess.getResponse(0);   // float
 *         String str = (IpcString)sess.getResponse(1);  // String
 *     }
 * };
 *
 * // Create an IPC event handler which listens channel state change events.
 * IpcEventHandler stateHandler = new IpcEventHandler() {
 *     public void eventReceived(IpcEvent event)
 *     {
 *         // Determine IPC channel name.
 *         String channel = event.getChannelName();
 *
 *         // Determine the host address of the IPC server.
 *         HostAddress host = event.getHostAddress();
 *
 *         if (event instanceof ChannelUpEvent) {
 *             // The IPC event listener session has been connected to the
 *             // IPC server.
 *         }
 *         else if (event instanceof ChannelDownEvent) {
 *             // The IPC event listener session has been disconnected.
 *             // Below code determines the cause of disconnection.
 *             ChannelDownEvent dev = (ChannelDownEvent)event;
 *             ChannelDownCause cause = dev.getCause();
 *         }
 *         else if (event instanceof ChannelStateEvent) {
 *             // This is a channel state notification event.
 *             // Below code determines the state of the listener session.
 *             ChannelStateEvent sev = (ChannelStateEvent)event;
 *             ChannelState state = sev.getState();
 *             if (state == ChannelState.UP) {
 *                 // The IPC event listener session is established.
 *             }
 *             else {
 *                 // The IPC event listener session is not established.
 *             }
 *         }
 *     }
 * };
 *
 * try {
 *     // Create an IPC host set named "foo_hosts".
 *     IpcHostSet hostSet = new IpcHostSet("foo_hosts");
 *     hostSet.create();
 *
 *     // Create an IpcEventAttribute instance, and set "foo_hosts".
 *     IpcEventAttribute attr = new IpcEventAttribute();
 *     attr.setHostSet(hostSet.getName());
 *
 *     // Deliver an event to `handler' if it is generated by the IPC service
 *     // named "serv1", and its event type is 0 or 1.
 *     IpcEventMask mask = new IpcEventMask(0);
 *     mask.add(1);
 *     attr.addTarget("serv1", mask);
 *
 *     // Deliver an event to `handler' if it is generated by the IPC service
 *     // named "serv2", and its event type is 3.
 *     mask.clear();
 *     mask.add(3);
 *     attr.addTarget("serv2", mask);
 *
 *     // Assign priority 50 to `handler'.
 *     attr.setPriority(50);
 *
 *     // Add `handler' to the IPC event subsystem, which listens events
 *     // generated by the IPC channel named "foo".
 *     // Note that this call does not start listening because the IPC host
 *     // set named "foo_hosts" is still empty.
 *     IpcEventSystem esys = IpcEventSystem.getInstance();
 *     esys.addHandler("foo", handler, attr, "foo handler");
 *
 *     // Construct attributes for `stateHandler' which listens channel
 *     // state change events.
 *     attr.resetTarget();
 *     attr.addTarget(null);
 *
 *     // Assign priority 150 to `stateHandler'.
 *     attr.setPriority(150);
 *
 *     // Add `stateHandler' to the IPC event subsystem, which listens channel
 *     // state change events on listener sessions connected to the IPC channel
 *     // named "foo".
 *     esys.addHandler("foo", stateHandler, attr, "state change");
 *
 *     // Add local host address to the IPC host set named "foo_hosts".
 *     // This call will create a new IPC event listener session which connects
 *     // to the IPC server "foo" on the localhost, and start event listening.
 *     hostSet.add(HostAddress.getLocalAddress());
 *
 *     // Add 192.168.10.1 to the IPC host set named "foo_hosts".
 *     // This call will create a new IPC event listener session which connects
 *     // to the IPC server "foo" on 192.168.10.1, and start event listening.
 *     hostSet.add(HostAddress.getByName("192.168.10.1"));
 * }
 * catch (IpcException e) {
 *     // Error.
 * }</pre>
 * </blockquote>
 * <p>
 *   To prevent unexpected events on application shutdown, it is strongly
 *   recommended to disable the IPC event subsystem before an application
 *   exits. Below pseudo code illustrates recommended way of application
 *   shutdown.
 * </p>
 * <blockquote><pre>
 * // Start shutdown sequence of the IPC event subsystem.
 * // This call stops receiving IPC events.
 * IpcEventSystem esys = IpcEventSystem.getInstance();
 *
 * try {
 *     esys.shutdown();
 * }
 * catch (IpcException e) {
 *     // Error.
 * }
 *
 * //
 * // Implement application shutdown code here.
 * //
 *
 * // Disable the IPC event subsystem.
 * // This call discards all pending events, and removes all event handlers.
 * try {
 *     esys.disable();
 * }
 * catch (IpcException e) {
 *     // Error.
 * }</pre>
 * </blockquote>
 *
 * @since	C10
 */
package org.opendaylight.vtn.core.ipc;
