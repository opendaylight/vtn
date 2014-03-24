/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.connection;

import org.opendaylight.vtn.core.ipc.AltConnection;
import org.opendaylight.vtn.core.ipc.ChannelAddress;
import org.opendaylight.vtn.core.ipc.ClientSession;
import org.opendaylight.vtn.core.ipc.IpcException;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.exception.VtnServiceExceptionHandler;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;

/**
 * The Class IpcChannelConnection. Maintains the AltConnection instance and
 * counter for the allocated sessions
 */
public class IpcChannelConnection {

	private static final Logger LOG = Logger
			.getLogger(IpcChannelConnection.class.getName());

	private transient int counter;
	private transient AltConnection altConnections;

	/**
	 * Instantiates a new ipc channel connection.
	 * 
	 * @param channelAddress
	 *            the channel address
	 * @throws IpcException
	 *             the ipc exception
	 */
	IpcChannelConnection(final ChannelAddress channelAddress)
			throws IpcException {
		LOG.trace("Start IpcChannelConnection#IpcChannelConnection()");
		LOG.debug("Channel Address : " + channelAddress.getChannelName());
		counter = 0;
		altConnections = AltConnection.open(channelAddress);
		LOG.trace("Complete IpcChannelConnection#IpcChannelConnection()");
	}

	/**
	 * Gets the counter.
	 * 
	 * @return the counter
	 */
	public final int getCounter() {
		LOG.trace("Return from IpcChannelConnection#getCounter()");
		return counter;
	}

	/**
	 * Sets the alt connections.
	 * 
	 * @param altConnections
	 *            the new alt connections
	 */
	public final void setAltConnections(final AltConnection altConnections) {
		LOG.trace("Start IpcChannelConnection#setAltConnections()");
		this.altConnections = altConnections;
		LOG.trace("Complete IpcChannelConnection#setAltConnections()");
	}

	/**
	 * Gets the session.
	 * 
	 * @param serviceName
	 *            the service name
	 * @param serviceID
	 *            the service id
	 * @param exceptionHandler
	 *            the exception handler
	 * @return the session
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	public final ClientSession getSession(final String serviceName,
			final int serviceID,
			final VtnServiceExceptionHandler exceptionHandler)
			throws VtnServiceException {

		LOG.trace("Start IpcChannelConnection#getSession()");
		ClientSession clientSession = null;
		try {
			clientSession = altConnections.createSession(serviceName,
					serviceID, ClientSession.C_NOGLOBCANCEL);
		} catch (final IpcException e) {
			exceptionHandler.raise(
					Thread.currentThread().getStackTrace()[1].getClassName()
							+ VtnServiceConsts.HYPHEN
							+ Thread.currentThread().getStackTrace()[1]
									.getMethodName(),
					UncJavaAPIErrorCode.IPC_SESS_ERROR.getErrorCode(),
					UncJavaAPIErrorCode.IPC_SESS_ERROR.getErrorMessage(), e);
		}
		// increase the counter if session is allocated successfully
		if (clientSession != null) {
			counter++;
		}
		LOG.debug("Counter for Connection: " + counter);
		LOG.trace("Complete IpcChannelConnection#getSession()");
		return clientSession;
	}

	/**
	 * Destroy session.
	 * 
	 * @param session
	 *            the session
	 */
	public final void destroySession(final ClientSession session) {
		LOG.trace("Start IpcChannelConnection#destroySession()");
		session.destroy();
		// decrease the counter
		counter--;
		LOG.debug("Counter for Connection: " + counter);
		LOG.trace("Complete IpcChannelConnection#destroySession()");
	}
}
