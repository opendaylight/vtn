/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi.connection;

import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

import org.opendaylight.vtn.core.ipc.ChannelAddress;
import org.opendaylight.vtn.core.ipc.ClientSession;
import org.opendaylight.vtn.core.ipc.IpcException;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.core.util.TimeSpec;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.exception.VtnServiceExceptionHandler;
import org.opendaylight.vtn.javaapi.init.VtnServiceConfiguration;
import org.opendaylight.vtn.javaapi.init.VtnServiceInitManager;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncSYSMGEnums;
import org.opendaylight.vtn.javaapi.ipc.enums.UncSessionEnums;
import org.opendaylight.vtn.javaapi.ipc.enums.UncTCEnums;
import org.opendaylight.vtn.javaapi.ipc.enums.UncUPLLEnums;
import org.opendaylight.vtn.javaapi.ipc.enums.UncUPPLEnums;
import org.opendaylight.vtn.javaapi.util.VtnServiceUtil;

/**
 * The Class IpcConnPool. IPC connection pooling initialization and providing
 * the sessions are main responsibility of this class
 */
public final class IpcConnPool {

	private static final Logger LOG = Logger.getLogger(IpcConnPool.class
			.getName());

	/* map containing ChannelAddress instance corresponding to Channel Address */
	private transient Map<String, ChannelAddress> channelAddressesMap = null;
	/*
	 * map containing set of IpcChannelConnection corresponding to
	 * ChannelAddress
	 */
	private transient Map<ChannelAddress, Set<IpcChannelConnection>> channelConnectionMap = null;
	/* map containing IpcChannelConnection corresponding to ClientSession */
	private transient Map<ClientSession, IpcChannelConnection> sessionMap = null;

	/**
	 * Initialize the connection pooling for IPC servers. Open the connections,
	 * with no session allocated during initialization.
	 * 
	 * @param exceptionHandler
	 *            the exception handler
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	public void init(final VtnServiceExceptionHandler exceptionHandler)
			throws VtnServiceException {

		LOG.trace("Start IpcConnPool#init()");
		final VtnServiceConfiguration configuration = VtnServiceInitManager
				.getConfigurationMap();
		/*
		 * Create the ChaneelAddress instances for each of the IPC server after
		 * reading from configuration file
		 */
		if (channelAddressesMap == null) {
			channelAddressesMap = new HashMap<String, ChannelAddress>();

			// IPC Server : TC
			final ChannelAddress tcChannelAddress = new ChannelAddress(
					configuration
							.getCommonConfigValue(UncTCEnums.UNC_CHANNEL_NAME));
			channelAddressesMap.put(UncTCEnums.UNC_CHANNEL_NAME,
					tcChannelAddress);

			// IPC Server : UPPL
			final ChannelAddress upplChannelAddress = new ChannelAddress(
					configuration
							.getCommonConfigValue(UncUPPLEnums.UPPL_IPC_CHN_NAME));
			channelAddressesMap.put(UncUPPLEnums.UPPL_IPC_CHN_NAME,
					upplChannelAddress);

			// IPC Server : UPLL
			final ChannelAddress upllChannelAddress = new ChannelAddress(
					configuration
							.getCommonConfigValue(UncUPLLEnums.UPLL_IPC_CHANNEL_NAME));
			channelAddressesMap.put(UncUPLLEnums.UPLL_IPC_CHANNEL_NAME,
					upllChannelAddress);

			// IPC Server : Node Manager
			final ChannelAddress nmChannelAddress = new ChannelAddress(
					configuration
							.getCommonConfigValue(UncSYSMGEnums.SYSMG_IPC_CHANNEL_NAME));
			channelAddressesMap.put(UncSYSMGEnums.SYSMG_IPC_CHANNEL_NAME,
					nmChannelAddress);

			// IPC Server : Session Manager
			final ChannelAddress sessmChannelAddress = new ChannelAddress(
					configuration
							.getCommonConfigValue(UncSessionEnums.UNCD_IPC_CHANNEL));
			channelAddressesMap.put(UncSessionEnums.UNCD_IPC_CHANNEL,
					sessmChannelAddress);

			/*
			 * Create the connection for each of the above IPC server
			 */
			try {
				createConnections(configuration, exceptionHandler);
			} catch (final VtnServiceException e) {
				exceptionHandler.raise(
						Thread.currentThread().getStackTrace()[1]
								.getClassName()
								+ VtnServiceConsts.HYPHEN
								+ Thread.currentThread().getStackTrace()[1]
										.getMethodName(),
						UncJavaAPIErrorCode.CONN_INIT_ERROR.getErrorCode(),
						UncJavaAPIErrorCode.CONN_INIT_ERROR.getErrorMessage(),
						e);
				throw e;
			}
		}
		LOG.trace("Complete IpcConnPool#init()");
	}

	/**
	 * Open the connection for IPC servers, allocated session count for each of
	 * the IPC server will be zero.
	 * 
	 * @param exceptionHandler
	 *            the exception handler
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	private void createConnections(final VtnServiceConfiguration configuration,
			final VtnServiceExceptionHandler exceptionHandler)
			throws VtnServiceException {
		LOG.trace("Start IpcConnPool#createConnections()");
		// Check that ChannelAddress has been read successfully
		if (channelAddressesMap != null && !channelAddressesMap.isEmpty()) {

			channelConnectionMap = new HashMap<ChannelAddress, Set<IpcChannelConnection>>();
			sessionMap = Collections.synchronizedMap(new HashMap<ClientSession, IpcChannelConnection>());
			int poolSize = 0;

			// retrieve the key set for Channel Address
			final Iterator<String> channelKeySet = channelAddressesMap.keySet()
					.iterator();
			IpcChannelConnection ipcChannelConnection = null;

			/*
			 * Read the pool size for each of the IPC server and open multiple
			 * connections
			 */
			while (channelKeySet.hasNext()) {
				final String key = channelKeySet.next();

				// read pool size from configuration file
				try {
					poolSize = Integer.parseInt(configuration
							.getConfigValue(VtnServiceConsts.CONN_POOL_SIZE
									+ key));
				} catch (final NumberFormatException e) {
					exceptionHandler.raise(
							Thread.currentThread().getStackTrace()[1]
									.getClassName()
									+ VtnServiceConsts.HYPHEN
									+ Thread.currentThread().getStackTrace()[1]
											.getMethodName(),
							UncJavaAPIErrorCode.POOL_SIZE_ERROR.getErrorCode(),
							UncJavaAPIErrorCode.POOL_SIZE_ERROR
									.getErrorMessage(), e);
				}

				LOG.debug("Connection Pool Size for " + key + " : " + poolSize);
				final Set<IpcChannelConnection> connectionSet = new HashSet<IpcChannelConnection>();
				final ChannelAddress channelAdd = channelAddressesMap.get(key);
				// open multiple connections, as per the pool size
				for (int i = 0; i < poolSize; i++) {
					try {
						ipcChannelConnection = new IpcChannelConnection(
								channelAdd);
					} catch (final IpcException e) {
						exceptionHandler.raise(
								Thread.currentThread().getStackTrace()[1]
										.getClassName()
										+ VtnServiceConsts.HYPHEN
										+ Thread.currentThread()
												.getStackTrace()[1]
												.getMethodName(),
								UncJavaAPIErrorCode.IPC_CONN_ERROR
										.getErrorCode(),
								UncJavaAPIErrorCode.IPC_CONN_ERROR
										.getErrorMessage(), e);
					}
					connectionSet.add(ipcChannelConnection);
				}
				channelConnectionMap.put(channelAdd, connectionSet);
			}
		}
		LOG.trace("Complete IpcConnPool#createConnections()");
	}

	/**
	 * Gets the session for the IPC server specified by Channel ID, Service ID
	 * and name
	 * 
	 * @param channelID
	 *            the channel id
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
	public ClientSession getSession(final String channelID,
			final String serviceName, final int serviceID,
			final VtnServiceExceptionHandler exceptionHandler)
			throws VtnServiceException {
		LOG.trace("Start IpcConnPool#getSession()");
		LOG.debug("Channel ID : " + channelID);
		LOG.debug("Service ID : " + serviceID);
		LOG.debug("Service Name : " + serviceName);

		ClientSession clientSession = null;
		if (VtnServiceUtil.isValidString(channelID)
				&& VtnServiceUtil.isValidString(serviceName)) {
			// Get the list of connections for the specified IPC Channel
			final Iterator<IpcChannelConnection> channelConnections = channelConnectionMap
					.get(channelAddressesMap.get(channelID)).iterator();

			// Get the least used connection, from the above connections
			int minimumCounter = Integer.MAX_VALUE;
			IpcChannelConnection ipcChannelConnection = null;
			IpcChannelConnection leastUsedIpcChannelConnection = null;

			/*
			 * At a time only one thread can get the session
			 */
			if (channelConnections != null) {
				synchronized (channelConnections) {
					// algorithm to find out the least used connection
					while (channelConnections.hasNext()) {
						ipcChannelConnection = channelConnections.next();
						final int currentCounter = ipcChannelConnection
								.getCounter();
						// if no session is allocated, then break from loop and
						// use the
						// current connection
						if (currentCounter == 0) {
							leastUsedIpcChannelConnection = ipcChannelConnection;
							break;
						} else if (currentCounter < minimumCounter) {
							minimumCounter = currentCounter;
							leastUsedIpcChannelConnection = ipcChannelConnection;
						}
					}
					// Once found least used connection, then create the session
					// and
					// return
					try {
						TimeSpec timeout = new TimeSpec(600L, 0);
						clientSession = leastUsedIpcChannelConnection
								.getSession(serviceName, serviceID,
										exceptionHandler);
						if (serviceID == UncUPLLEnums.ServiceID.UPLL_READ_SVC_ID
								.ordinal()
								|| serviceID == UncUPPLEnums.ServiceID.UPPL_SVC_READREQ
										.ordinal()) {
							clientSession.setTimeout(null);
						} else if (serviceID == UncUPLLEnums.ServiceID.UPLL_EDIT_SVC_ID
								.ordinal()) {
							clientSession.setTimeout(timeout);
						}
					} catch (final VtnServiceException e) {
						exceptionHandler.raise(
								Thread.currentThread().getStackTrace()[1]
										.getClassName()
										+ VtnServiceConsts.HYPHEN
										+ Thread.currentThread()
												.getStackTrace()[1]
												.getMethodName(),
								UncJavaAPIErrorCode.SESS_ERROR.getErrorCode(),
								UncJavaAPIErrorCode.SESS_ERROR
										.getErrorMessage(), e);
						throw e;
					} catch (final IpcException e) {
						LOG.error(e, "Error occured while performing addOutput operation");
						exceptionHandler.raise(
								Thread.currentThread().getStackTrace()[1]
										.getClassName()
										+ VtnServiceConsts.HYPHEN
										+ Thread.currentThread()
												.getStackTrace()[1]
												.getMethodName(),
								UncJavaAPIErrorCode.IPC_SERVER_ERROR
										.getErrorCode(),
								UncJavaAPIErrorCode.IPC_SERVER_ERROR
										.getErrorMessage(), e);
					}
					// maintain the session map, that will be used at the time
					// of
					// destroying the session
					sessionMap
							.put(clientSession, leastUsedIpcChannelConnection);
				}
			}
			LOG.trace("Complete IpcConnPool#getSession()");
		}
		return clientSession;
	}

	/**
	 * Destroy session.
	 * 
	 * @param session
	 *            the session
	 */
	public void destroySession(final ClientSession session) {
		LOG.trace("Start IpcConnPool#destroySession()");
		// Get the connection instance for which session needs to be closed
		if (session != null) {
			final IpcChannelConnection ipcChannelConnection = sessionMap
					.get(session);
			if (ipcChannelConnection != null) {
				// at a time only one thread can destroy the session
				synchronized (ipcChannelConnection) {
					ipcChannelConnection.destroySession(session);
					sessionMap.remove(session);
				}
			}
		}
		LOG.trace("Complete IpcConnPool#destroySession()");
	}
}
