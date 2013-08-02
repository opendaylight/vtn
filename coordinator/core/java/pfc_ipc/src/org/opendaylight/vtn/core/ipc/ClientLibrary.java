/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.ipc;

import org.opendaylight.vtn.core.CoreSystem;
import org.opendaylight.vtn.core.util.LogSystem;
import org.opendaylight.vtn.core.util.LogConfiguration;

/**
 * <p>
 *   The {@code ClientLibrary} class provides class methods which controls
 *   the IPC client library. it can not be instantiated.
 * </p>
 *
 * @since	C10
 */
public final class ClientLibrary
{
	/**
	 * Load the IPC client native library.
	 */
	static {
		CoreSystem.loadLibrary("pfc_ipcclnt_jni");
	}

	/**
	 * Ensure that the native library is loaded.
	 */
	static void load() {}

	/**
	 * Private constructor which will never be called.
	 */
	private ClientLibrary()
	{
	}

	/**
	 * <p>
	 *   Cancel all ongoing IPC service requests on IPC client sessions
	 *   which does not ignore global cancellation.
	 * </p>
	 * <p>
	 *   All {@link ClientSession#invoke()} being called will throw
	 *   {@link IpcCanceledException}.
	 * </p>
	 */
	public static native void cancel();

	/**
	 * <p>
	 *   Disable IPC client library.
	 * </p>
	 * <p>
	 *   This method cancels all ongoing IPC service requests.
	 *   All {@link ClientSession#invoke()} being called will throw
	 *   {@link IpcClientDisabledException}.
	 *   After the call of this method, any further IPC service request
	 *   can not be issued.
	 * </p>
	 * <p>
	 *   Note that this method does not affect the IPC event subsystem.
	 *   {@link IpcEventSystem#disable()} must be used to disable the
	 *   IPC event subsystem.
	 * </p>
	 */
	public static native void disable();

	/**
	 * Determine whether the IPC client library is disabled by
	 * {@link #disable()}.
	 *
	 * @return	True only if the IPC client library is disabled.
	 */
	public static native boolean isDisabled();

	/**
	 * <p>
	 *   Enable or disable the IPC client internal logging.
	 *   The internal logging is disabled by default.
	 * </p>
	 * <ul>
	 *   <li>
	 *     If {@code true} is specified to argument, this method enables
	 *     the internal logging by the PFC-Core logging system.
	 *     Note that the PFC-Core logging system must be initialized in
	 *     advance.
	 *   </li>
	 *   <li>
	 *     If {@code false} is specified to argument, this method disables
	 *     the internal logging.
	 *     This method does nothing if the logging system is not enabled.
	 *   </li>
	 * </ul>
	 * <p>
	 *   The internal logging is automatically disabled when
	 *   {@link LogSystem#shutdown()} is called.
	 * </p>
	 *
	 * @param enabled	Enable logging if {@code true}.
	 *			Disable it if {@code false}.
	 * @throws IllegalStateException
	 *	{@code enabled} is {@code true}, and the PFC-Core logging
	 *	system is not yet initialized.
	 * @see	LogSystem#initialize(LogConfiguration)
	 * @see	LogSystem#shutdown()
	 */
	public static native void setLogEnabled(boolean enabled);
}
