/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.ipc;

import java.util.Iterator;
import java.util.Stack;

import org.opendaylight.vtn.core.ipc.IpcUint32;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncOperationEnum;

/**
 * The Class IpcRollback.
 */
public class IpcRollback {

	private static final Logger LOG = Logger.getLogger(IpcRollback.class
			.getName());
	private transient final Stack<IpcRequestPacket> ipcRequestPacketStack;

	/**
	 * Instantiates a new ipc rollback.
	 */
	public IpcRollback() {
		LOG.debug("Initialization of IpcRequestPacket Stack.");
		ipcRequestPacketStack = new Stack<IpcRequestPacket>();
	}

	/**
	 * Clear ipc packet stack.
	 */
	private void clearIpcPacketStack() {
		LOG.debug("Clear IpcRequestPacket Stack.");
		ipcRequestPacketStack.clear();
	}

	/**
	 * Pop ipc packet.
	 * 
	 * @return the ipc request packet
	 */
	public final IpcRequestPacket popIpcPacket() {
		LOG.trace("Start IpcRequestPacket#popIpcPacket()");
		IpcRequestPacket requestPacket = null;
		if (!ipcRequestPacketStack.isEmpty()) {
			requestPacket = ipcRequestPacketStack.pop();
		}
		LOG.trace("Complete IpcRequestPacket#popIpcPacket()");
		return requestPacket;
	}

	/**
	 * Push ipc packet.
	 * 
	 * @param requestPacket
	 *            the request packet
	 */
	public final void pushIpcPacket(final IpcRequestPacket requestPacket) {
		LOG.trace("Start IpcRequestPacket#pushIpcPacket()");
		ipcRequestPacketStack.push(requestPacket);
		LOG.trace("Complete IpcRequestPacket#pushIpcPacket()");
	}

	/**
	 * Roll back ipc request.
	 * 
	 * @param requestProcessor
	 *            the request processor
	 * @return true, if successful
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	public final boolean rollBackIpcRequest(
			final IpcRequestProcessor requestProcessor)
			throws VtnServiceException {
		LOG.trace("Start IpcRequestPacket#rollBackIpcRequest()");
		boolean rollBackStatus = false;
		final Iterator<IpcRequestPacket> ipcRequestPacketIterator = ipcRequestPacketStack
				.iterator();
		while (ipcRequestPacketIterator.hasNext()) {
			final IpcRequestPacket requestPacket = ipcRequestPacketIterator
					.next();
			if (requestPacket.getOperation().intValue() == UncOperationEnum.UNC_OP_CREATE
					.ordinal()) {
				LOG.debug("Set Operation type Delete for Create Operation from Stack. Key-Type : "
						+ requestPacket.getKeyType());
				requestPacket.setOperation(new IpcUint32(
						UncOperationEnum.UNC_OP_DELETE.ordinal()));
			}

			try {
				LOG.debug("Invoke operation for roll-back of first item from stack.");
				requestProcessor.processIpcRequest();
			} catch (final VtnServiceException e) {
				requestProcessor
						.getExceptionHandler()
						.raise(Thread.currentThread().getStackTrace()[1]
								.getClassName()
								+ VtnServiceConsts.HYPHEN
								+ Thread.currentThread().getStackTrace()[1]
										.getMethodName(),
								UncJavaAPIErrorCode.ROLLBACK_ERROR
										.getErrorCode(),
								UncJavaAPIErrorCode.ROLLBACK_ERROR
										.getErrorMessage(), e);
				throw e;
			}

			// check should be updated after all the successful rollback
			rollBackStatus = true;
		}
		clearIpcPacketStack();
		LOG.trace("Complete IpcRequestPacket#rollBackIpcRequest()");
		return rollBackStatus;
	}
}
