/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.util;

import java.lang.ref.ReferenceQueue;
import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.Map;

/**
 * <p>
 *   An abstract class for logging system implementation.
 * </p>
 * <p>
 *   A subclass of this class should manage {@link Logger} instances
 *   associated with this logging system, and current logging level.
 * </p>
 */
abstract class LogSystemImpl
{
	/**
	 * <p>
	 *   Load native library.
	 * </p>
	 */
	static {
		PfcUtil.load();
	}

	/**
	 * <p>
	 *   Hash map which keeps pairs of the logger name and weak reference
	 *   to the {@link Logger} instance.
	 * <p>
	 */
	private final Map<String, LoggerRef>  _loggers =
		new HashMap<String, LoggerRef>();

	/**
	 * <p>
	 *   {@link Logger} instances which have been collected by the GC will
	 *   be enqueued to this queue.
	 * </p>
	 */
	private final ReferenceQueue<Logger> _queue =
		new ReferenceQueue<Logger>();

	/**
	 * <p>
	 *   Weak reference to {@link Logger} instance.
	 * </p>
	 */
	final class LoggerRef extends WeakReference<Logger>
	{
		/**
		 * <p>
		 *   The name of the logger.
		 * </p>
		 * <p>
		 *   It must be held by this instance in order to remove
		 *   {@code HashMap} entry associated with the name.
		 * </p>
		 */
		private final String  _name;

		/**
		 * <p>
		 *   Set {@code true} only if this reference is removed from
		 *   the map.
		 * </p>
		 */
		private boolean  _removed;

		/**
		 * <p>
		 *   Construct a new weak reference to {@link Logger} instance.
		 * </p>
		 *
		 * @param logger	The logger instance.
		 */
		LoggerRef(Logger logger)
		{
			super(logger, _queue);
			_name = logger.getName();
		}
	}

	/**
	 * <p>
	 *   Construct a new logging system implementation.
	 * </p>
	 */
	LogSystemImpl()
	{
	}

	/**
	 * <p>
	 *   Get the logger associated with the specified name.
	 * </p>
	 * <p>
	 *   If the logger associated with the specified name is held by this
	 *   instance, this method returns it. Otherwise a new logger is
	 *   created.
	 * </p>
	 *
	 * @param name	The name of the logger.
	 * @return	The logger associated with the specified name.
	 */
	synchronized Logger getLogger(String name)
	{
		flushQueue();

		LoggerRef  lref = _loggers.get(name);
		if (lref != null) {
			Logger logger = lref.get();

			if (logger != null) {
				return logger;
			}

			// Remove reference to dead logger.
			_loggers.remove(name);

			// Turn the removed flag on.
			lref._removed = true;
			lref = null;

			// Removed reference may be queued in the reference
			// queue. So we need to drain the queue again.
			flushQueue();
		}

		// Create a new logger.
		Logger logger = newLogger(name);
		lref = new LoggerRef(logger);
		_loggers.put(name, lref);

		return logger;
	}

	/**
	 * <p>
	 *   Return the number of logger instances.
	 * </p>
	 * <p>
	 *   This method always eliminates dead loggers in the map.
	 * </p>
	 *
	 * @return	The number of active loggers.
	 */
	synchronized int getSize()
	{
		flushQueue();

		return _loggers.size();
	}

	/**
	 * <p>
	 *   Remove all map entries associated with references in the
	 *   reference queue {@link #_queue}.
	 * </p>
	 */
	private synchronized void flushQueue()
	{
		LoggerRef  lref;
		while ((lref = (LoggerRef)_queue.poll()) != null) {
			if (!lref._removed) {
				_loggers.remove(lref._name);
			}
		}
	}

	/**
	 * <p>
	 *   Return the current logging level of this implementation.
	 * </p>
	 *
	 * @return	The current logging level of this implementation.
	 */
	LogLevel getLevel()
	{
		return LogLevel.getInstance(getLogLevel());
	}

	/**
	 * <p>
	 *   Change the logging level of this implementation to the specified
	 *   value.
	 * </p>
	 * <p>
	 *   The logging level is reset to initial level if
	 *   {@link LogLevel#NONE} is specified to {@code level}.
	 * </p>
	 *
	 * @param level		New logging level.
	 * @return		{@code true} if the logging level has been
	 *			actually changed.
	 *			{@code false} if the current logging level is
	 *			 identical to {@code level}.
	 * @throws NullPointerException
	 *	{@code level} is {@code null}.
	 */
	public boolean setLevel(LogLevel level)
	{
		return setLogLevel(level.getLevel());
	}

	/**
	 * <p>
	 *   Create a new {@link Logger} instance.
	 * </p>
	 *
	 * @param name	The name of the logger instance.
	 *		{@code null} means an anonymous logger.
	 */
	abstract Logger newLogger(String name);

	/**
	 * <p>
	 *   Return the current logging level of this implementation.
	 * </p>
	 *
	 * @return	The current logging level value of this implementation.
	 */
	abstract int getLogLevel();

	/**
	 * <p>
	 *   Set the logging level of this implementation.
	 * </p>
	 *
	 * @param level		Logging level value to be set.
	 * @return		{@code true} if the level has been actually
	 *			changed.
	 *			{@code  false} if the current level is
	 *			identical to {@code level}.
	 */
	abstract boolean setLogLevel(int level);
}
