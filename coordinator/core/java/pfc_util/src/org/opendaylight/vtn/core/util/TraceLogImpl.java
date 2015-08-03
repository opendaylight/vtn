/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.util;

/**
 * <p>
 *   Logging system implementation for the trace log.
 * </p>
 */
final class TraceLogImpl extends LogSystemImpl
{
	/**
	 * <p>
	 *   Holder class of the trace log implementation.
	 * </p>
	 */
	private final static class TraceLogHolder
	{
		/**
		 * <p>
		 *   A single global trace log implementation.
		 * </p>
		 */
		private final static TraceLogImpl  _theInstance =
			new TraceLogImpl();
	}

	/**
	 * <p>
	 *   Return the global {@code TraceLogImpl} instance.
	 * </p>
	 *
	 * @return	The {@code TraceLogImpl} instance.
	 */
	static TraceLogImpl getInstance()
	{
		return TraceLogHolder._theInstance;
	}

	/**
	 * <p>
	 *   Create a single instance of the trace log implementation.
	 * </p>
	 */
	private TraceLogImpl()
	{
	}

	/**
	 * <p>
	 *   Create a new {@link Logger} instance for the trace log.
	 * </p>
	 *
	 * @param name	The name of the logger instance.
	 *		{@code null} means an anonymous logger.
	 */
	@Override
	Logger newLogger(String name)
	{
		return new Logger.TraceLogger(name);
	}

	/**
	 * <p>
	 *   Return the current logging level of the trace log.
	 * </p>
	 *
	 * @return	The current logging level value of the trace log.
	 */
	@Override
	native int getLogLevel();

	/**
	 * <p>
	 *   Set the logging level of the trace log.
	 * </p>
	 *
	 * @param level		Logging level value to be set.
	 * @return		{@code true} if the level has been actually
	 *			changed.
	 *			{@code  false} if the current level is
	 *			identical to {@code level}.
	 */
	@Override
	native boolean setLogLevel(int level);
}
