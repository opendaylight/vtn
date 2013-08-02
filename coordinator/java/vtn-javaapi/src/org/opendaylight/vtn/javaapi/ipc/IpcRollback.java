/*
 * Copyright (c) 2012-2013 NEC Corporation
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
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncOperationEnum;

/**
 * The Class IpcRollback.
 */
public class IpcRollback {

	// private static final Logger LOG =
	// Logger.getLogger(IpcRollback.class.getName());

	private transient final Stack<IpcRequestPacket> ipcRequestPacketStack;

	/**
	 * Instantiates a new ipc rollback.
	 */
	public IpcRollback() {
		ipcRequestPacketStack = new Stack<IpcRequestPacket>();
	}

	/**
	 * Clear ipc packet stack.
	 */
	private void clearIpcPacketStack() {
		ipcRequestPacketStack.clear();
	}

	/**
	 * Pop ipc packet.
	 * 
	 * @return the ipc request packet
	 */
	public IpcRequestPacket popIpcPacket() {
		IpcRequestPacket requestPacket = null;
		if (!ipcRequestPacketStack.isEmpty()) {
			requestPacket = ipcRequestPacketStack.pop();
		}
		return requestPacket;
	}

	/**
	 * Push ipc packet.
	 * 
	 * @param requestPacket
	 *            the request packet
	 */
	public void pushIpcPacket(final IpcRequestPacket requestPacket) {
		ipcRequestPacketStack.push(requestPacket);
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
	public boolean rollBackIpcRequest(final IpcRequestProcessor requestProcessor)
			throws VtnServiceException {
		boolean rollBackStatus = false;
		final Iterator<IpcRequestPacket> ipcRequestPacketIterator = ipcRequestPacketStack
				.iterator();
		while (ipcRequestPacketIterator.hasNext()) {
			final IpcRequestPacket requestPacket = ipcRequestPacketIterator
					.next();
			if (requestPacket.getOperation().intValue() == UncOperationEnum.UNC_OP_CREATE
					.ordinal()) {
				requestPacket.setOperation(new IpcUint32(
						UncOperationEnum.UNC_OP_DELETE.ordinal()));
			}

			try {
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
		return rollBackStatus;
	}
}
