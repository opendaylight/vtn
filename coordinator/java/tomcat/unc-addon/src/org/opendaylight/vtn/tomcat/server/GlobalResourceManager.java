/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.tomcat.server;

import java.util.Properties;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStream;
import org.apache.catalina.Globals;
import org.apache.juli.logging.Log;
import org.apache.juli.logging.LogFactory;
import org.opendaylight.vtn.core.util.HostAddress;
import org.opendaylight.vtn.core.util.LogSystem;
import org.opendaylight.vtn.core.util.LogConfiguration;
import org.opendaylight.vtn.core.util.LogLevel;
import org.opendaylight.vtn.core.util.LogFacility;
import org.opendaylight.vtn.core.util.LogFatalHandler;
import org.opendaylight.vtn.core.ipc.ChannelAddress;
import org.opendaylight.vtn.core.ipc.ChannelDownEvent;
import org.opendaylight.vtn.core.ipc.ChannelState;
import org.opendaylight.vtn.core.ipc.ChannelUpEvent;
import org.opendaylight.vtn.core.ipc.ClientLibrary;
import org.opendaylight.vtn.core.ipc.IpcEvent;
import org.opendaylight.vtn.core.ipc.IpcEventAttribute;
import org.opendaylight.vtn.core.ipc.IpcEventConfiguration;
import org.opendaylight.vtn.core.ipc.IpcEventHandler;
import org.opendaylight.vtn.core.ipc.IpcEventMask;
import org.opendaylight.vtn.core.ipc.IpcEventSystem;
import org.opendaylight.vtn.core.ipc.IpcHostSet;

/**
 * <p>
 *   Manager class of global resources for the UNC service.
 * </p>
 * <p>
 *   Currently this class manages the following resources.
 * </p>
 * <ul>
 *   <li>
 *     Core logging functionality.
 *   </li>
 *   <li>
 *     IPC event subsystem.
 *   </li>
 * </ul>
 */
class GlobalResourceManager implements LogFatalHandler, IpcEventHandler
{
	/**
	 * <p>
	 *   The name of resource file that specifies the configuration of
	 *   global resources.
	 * </p>
	 */
	private final static String  RESOURCE_FILE = "/unc-gres.properties";

	/**
	 * <p>
	 *   Internal state bit that indicates global resources are
	 *   initialized.
	 * </p>
	 */
	private final static byte  RSTATE_INIT  = 0x01;

	/**
	 * <p>
	 *   Internal state bit that indicates the shutdown sequence is
	 *   already triggered.
	 * </p>
	 */
	private final static byte  RSTATE_SHUTDOWN  = 0x02;

	/**
	 * <p>
	 *   Internal state bit that indicates global resources are already
	 *   finalized.
	 * </p>
	 */
	private final static byte  RSTATE_FINI  = 0x04;

	/**
	 * <p>
	 *   Internal state bit that indicates {@link System#exit(int)} is
	 *   already called.
	 * </p>
	 */
	private final static byte  RSTATE_EXITED  = 0x08;

	/**
	 * Priority of IPC event handler which watches the specified
	 * IPC channel.
	 */
	private final static int  IPC_WATCH_PRIORITY = 100000;

	/**
	 * The name of IPC event resources for IPC channel watching.
	 */
	private final static String IPC_WATCH_NAME = "channel_watch";

	/**
	 * Set {@code true} only if the Tomcat instance should not be aborted
	 * even if a fatal level log is recorded.
	 */
	private boolean  ignoreFatal;

	/**
	 * <p>
	 *   Holder class of a single global {@link GlobalResourceManager}
	 *   instance.
	 * </p>
	 */
	private final static class GlobalResourceManagerHolder
	{
		/**
		 * <p>
		 *   A single global instance of
		 *   {@link GlobalResourceManagerHolder}.
		 * </p>
		 */
		private final static GlobalResourceManager  _theInstance =
			new GlobalResourceManager();
	}

	/**
	 * <p>
	 *   Return a single global instance of {@link GlobalResourceManager}.
	 * </p>
	 *
	 * @return	An {@link GlobalResourceManager} instance.
	 */
	static GlobalResourceManager getInstance()
	{
		return GlobalResourceManagerHolder._theInstance;
	}

	/**
	 * <p>
	 *   Instance to record log messages.
	 * </p>
	 */
	private final Log  _log;

	/**
	 * <p>
	 *   Internal state flag bits.
	 * </p>
	 */
	private byte  _state;

	/**
	 * <p>
	 *   Create a single instance of {@link GlobalResourceManager}.
	 * </p>
	 */
	private GlobalResourceManager()
	{
		_log = LogFactory.getLog(getClass());
	}

	/**
	 * <p>
	 *   Invoked when a fatal level log is recorded.
	 * </p>
	 */
	@Override
	public void fatalError()
	{
		if (!ignoreFatal) {
			abort();
		}
	}

	/**
	 * <p>
	 *   Invoked when the target IPC channel is down.
	 * </p>
	 *
	 * @param event  A received IPC event.
	 */
	@Override
	public void eventReceived(IpcEvent event)
	{
		if (event instanceof ChannelDownEvent) {
			_log.fatal("IPC channel is down: " + event);

			// Abort the Tomcat instance on another thread.
			abortAsync();
			return;
		}
		if (event instanceof ChannelUpEvent) {
			_log.info("IPC channel is up: " + event);
			return;
		}

		_log.warn("Received unexpected event: " + event);
	}

	/**
	 * <p>
	 *   Initialize global resources for UNC service.
	 * </p>
	 * <p>
	 *   This method will be called just after the Tomcat server is
	 *   launched.
	 * </p>
	 */
	void initialize()
	{
		Properties prop = load();

		synchronized (this) {
			if (_state != 0) {
				// Already initialized or finalized.
				return;
			}

			_state = RSTATE_INIT;
		}

		initializeLog(prop);
		initializeIpc(prop);
		initializeIpcWatch();
	}

	/**
	 * <p>
	 *   Trigger shutdown sequence of the resource management.
	 * </p>
	 * <p>
	 *   This method will be called just before the Tomcat server triggers
	 *   the shutdown sequence.
	 * </p>
	 */
	void startShutdown()
	{
		synchronized (this) {
			final byte state = _state;

			if (state != RSTATE_INIT) {
				// Not yet initialized, or already finalized.
				return;
			}

			_state = (byte)(state | RSTATE_SHUTDOWN);
		}

		shutdownIpc();
	}

	/**
	 * <p>
	 *   Disable global resources for UNC service.
	 * </p>
	 * <p>
	 *   This method will be called just before the Tomcat server exits.
	 * </p>
	 */
	void disable()
	{
		synchronized (this) {
			final byte state = _state;

			if ((state & (RSTATE_INIT | RSTATE_FINI)) !=
			    RSTATE_INIT) {
				// Not yet initialized, or already finalized.
				return;
			}

			_state = (byte)(state | RSTATE_FINI);
		}

		disableIpc();
		disableLog();
	}

	/**
	 * <p>
	 *   Initialize the core logging functionality.
	 * </p>
	 *
	 * @param prop	A {@code Properties} instance which contains the
	 *		configuration of global resource.
	 */
	private void initializeLog(Properties prop)
	{
		File logDir = getLogDirectory();
		File logFile = new File(logDir, "core.log");
		final LogConfiguration conf =
			new LogConfiguration("tomcat", logFile);

		// Configure logging levels.
		String key = "log.trace.level.default";
		LogLevelSetter setter = new LogLevelSetter(key) {
			@Override
			protected void setLogLevel(LogLevel lvl)
			{
				conf.setDefaultLevel(lvl);
			}
		};
		setter.set(prop);

		key = "log.trace.level";
		setter = new LogLevelSetter(key) {
			@Override
			protected void setLogLevel(LogLevel lvl)
			{
				conf.setLevel(lvl);
			}
		};
		setter.set(prop);

		// Configure trace log file rotation.
		Integer fsize = fetchSize(prop, "log.trace.file.max_size");
		Integer rotate = fetchInteger(prop, "log.trace.file.rotation");
		if (fsize != null && rotate != null) {
			conf.setRotation(rotate.intValue(), fsize.intValue());
		}

		// Configure syslog facility.
		key = "log.system.facility";
		LogFacility fac = fetchLogFacility(prop, key);
		if (fac != null) {
			conf.setFacility(fac);
		}

		if (fetchBoolean(prop, "log.savelevel")) {
			// Configure log level file.
			String path = logDir.getAbsolutePath() + ".level";
			_log.info("level path: " + path);
			File lfile = new File(path);
			conf.setLevelFile(lfile);
		}

		ignoreFatal = fetchBoolean(prop, "log.fatal.ignore");
		conf.setFatalHandler(this);

		// Initialize core logging functionality.
		LogSystem lsys = LogSystem.getInstance();
		try {
			lsys.initialize(conf);
		}
		catch (Exception e) {
			// This should never happen.
			_log.fatal("Unable to initialize core logging.", e);
			abort();
		}

		_log.info("Core logging has been initialized: " + logFile);
	}

	/**
	 * <p>
	 *   Disable the core logging functionality.
	 * </p>
	 */
	private void disableLog()
	{
		LogSystem lsys = LogSystem.getInstance();

		lsys.shutdown();
		_log.info("Core logging has been finalized.");
	}

	/**
	 * <p>
	 *   Initialize the IPC framework.
	 * </p>
	 *
	 * @param prop	A {@code Properties} instance which contains the
	 *		configuration of global resource.
	 */
	private void initializeIpc(Properties prop)
	{
		// Enable the IPC client logging.
		ClientLibrary.setLogEnabled(true);

		final IpcEventConfiguration conf = new IpcEventConfiguration();

		// Configure IPC event subsystem.
		String key = "ipc.event.conn_interval";
		IpcEventConfSetter setter =
			new IpcEventConfSetter(key, 1000, 86400000) {
			@Override
			protected void setParameter(int value)
			{
				conf.setConnectInterval(value);
			}
		};
		setter.set(prop);

		key = "ipc.event.idle_timeout";
		setter = new IpcEventConfSetter(key, 10, 86400000) {
			@Override
			protected void setParameter(int value)
			{
				conf.setIdleTimeout(value);
			}
		};
		setter.set(prop);

		key = "ipc.event.max_threads";
		setter = new IpcEventConfSetter(key, 1, 1024) {
			@Override
			protected void setParameter(int value)
			{
				conf.setMaxThreads(value);
			}
		};
		setter.set(prop);

		key = "ipc.event.timeout";
		setter = new IpcEventConfSetter(key, 1000, 3600000) {
			@Override
			protected void setParameter(int value)
			{
				conf.setTimeout(value);
			}
		};
		setter.set(prop);

		// Initialize the IPC event subsystem.
		IpcEventSystem esys = IpcEventSystem.getInstance();
		try {
			esys.initialize(conf);
		}
		catch (Exception e) {
			// This should never happen.
			_log.fatal("Unable to initialize IPC event subsystem.",
				   e);
			abort();
		}

		_log.info("IPC event subsystem has been initialized.");
	}

	/**
	 * <p>
	 *   Initialize IPC channel watching.
	 * </p>
	 */
	private void initializeIpcWatch()
	{
		String watch = System.getProperty("vtn.ipc.watch");
		if (watch == null) {
			return;
		}

		_log.info("Activating IPC channel watching: " + watch);

		IpcEventSystem esys = IpcEventSystem.getInstance();
		IpcEventAttribute evattr = new IpcEventAttribute();

		try {
			ChannelAddress chaddr =
				setUpEventAttribute(watch, evattr);
			String name = chaddr.getChannelName();
			esys.addHandler(name, this, evattr, IPC_WATCH_NAME);

			HostAddress haddr = chaddr.getHostAddress();
			ChannelState state = esys.getChannelState(name, haddr);
			if (state == ChannelState.DOWN) {
				_log.fatal("IPC channel is down: " + chaddr);
				abort();
				return;
			}

			_log.info(chaddr + ": IPC channel state: " + state);
		}
		catch (Exception e) {
			_log.error("Failed to start IPC channel watching", e);
		}
	}

	/**
	 * <p>
	 *   Abort the Tomcat instance.
	 * </p>
	 */
	private void abort()
	{
		synchronized (this) {
			final byte state = _state;

			if ((state & ~RSTATE_INIT) != 0) {
				// Shutdown sequence is already triggered.
				return;
			}
			
			_state = (byte)(state | RSTATE_EXITED);
		}

		System.exit(2);
		// NOTREACHED
	}

	/**
	 * <p>
	 *   Abort the Tomcat instance on another thread.
	 * </p>
	 */
	private void abortAsync()
	{
		try {
			Thread t = new Thread("Tomcat abort thread") {
				@Override
				public void run()
				{
					abort();
				}
			};
			t.start();
		} catch (Exception e) {
			// This should never happen.
			_log.error("Failed to start abort thread", e);
			abort();
		}
	}

	/**
	 * <p>
	 *   Set up IPC event attribute for IPC channel watching.
	 * </p>
	 *
	 * @param chaddr  IPC channel address to watch.
	 * @param evattr  IPC event attribute.
	 * @return  An IPC channel address instance is returned.
	 */
	private ChannelAddress setUpEventAttribute(String chaddr,
						   IpcEventAttribute evattr)
		throws Exception
	{
		// Split IPC channel name and host address.
		String chname;
		HostAddress haddr = null;
		int idx = chaddr.indexOf('@');
		if (idx > 0) {
			chname = chaddr.substring(0, idx);
			int len = chaddr.length();
			if (idx < len - 1) {
				String h = chaddr.substring(idx + 1, len);
				haddr = HostAddress.getByName(h);
			}
		}
		else if (idx < 0) {
			chname = chaddr;
		}
		else {
			_log.error("No IPC channel name: " + chaddr);
			return null;
		}

		if (haddr == null) {
			haddr = HostAddress.getLocalAddress();
		}

		// Watch IPC channel down and state event.
		IpcEventMask evmask = new IpcEventMask(ChannelDownEvent.TYPE);
		evmask.add(ChannelUpEvent.TYPE);
		evattr.addTarget(null, evmask);

		IpcHostSet hset = new IpcHostSet(IPC_WATCH_NAME);
		hset.create();
		hset.add(haddr);
		evattr.setHostSet(IPC_WATCH_NAME);

		evattr.setPriority(IPC_WATCH_PRIORITY);
		evattr.setLogEnabled(true);

		return new ChannelAddress(chname, haddr);
	}

	/**
	 * <p>
	 *   Trigger the shutdown sequence of the IPC framework.
	 * </p>
	 */
	private void shutdownIpc()
	{
		// Trigger the shutdown sequence of the IPC event subsystem.
		IpcEventSystem esys = IpcEventSystem.getInstance();
		try {
			esys.shutdown();
			_log.info("Shutting down the IPC event subsystem.");
		}
		catch (Exception e) {
			_log.error("Unable to shutdown the IPC event " +
				   "subsystem", e);
		}
	}

	/**
	 * <p>
	 *   Disable the IPC framework.
	 * </p>
	 */
	private void disableIpc()
	{
		// Finalize the IPC event subsystem.
		IpcEventSystem esys = IpcEventSystem.getInstance();
		try {
			esys.disable();
			_log.info("IPC event subsystem has been finalized.");
		}
		catch (Exception e) {
			_log.error("Unable to shutdown the IPC event " +
				   "subsystem", e);
		}

		// Disable the IPC client library.
		ClientLibrary.disable();
		_log.info("IPC client has been disabled.");
	}

	/**
	 * <p>
	 *   Return a {@code File} which represents an absolute path to the
	 *   UNC log directory.
	 * </p>
	 *
	 * @return	A {@code File} instance.
	 */
	private File getLogDirectory()
	{
		String base = getLogBaseDirectory();

		return new File(base, "core");
	}

	/**
	 * <p>
	 *   Return a string which represents an absolute path to the
	 *   UNC logging base directory.
	 * </p>
	 */
	private String getLogBaseDirectory()
	{
		// Try "vtn.logdir" property first.
		String base = System.getProperty("vtn.logdir");
		if (base != null) {
			return base;
		}

		String[] keys = {
			Globals.CATALINA_BASE_PROP,
			Globals.CATALINA_HOME_PROP,
		};

		for (String key: keys) {
			base = System.getProperty(key);
			if (base != null) {
				return base + "/logs";
			}
		}

		// This should never happen.
		throw new IllegalStateException
			("Unable to determine logging base directory.");
	}

	/**
	 * <p>
	 *   Load configuration from the resource file.
	 * </p>
	 *
	 * @return	A {@code Properties} instance which contains the
	 *		configuration of global resource.
	 */
	private Properties load()
	{
		Properties prop = new Properties();

		// Try to open resource file.
		InputStream stream = getClass().
			getResourceAsStream(RESOURCE_FILE);
		if (stream != null) {
			try {
				prop.load(stream);
			}
			catch (Exception e) {
				_log.error("Unable to load resource file: " +
					   RESOURCE_FILE, e);

				return new Properties();
			}

			// Close the stream with ignoring error.
			try {
				stream.close();
			}
			catch (Exception e) {}
		}

		return prop;
	}

	/**
	 * <p>
	 *   Fetch size value, such as the file size, from the given resource
	 *   property.
	 * </p>
	 * <p>
	 *   The value of the given key can have a suffix which determines the
	 *   size of the value, <strong>KB</strong> for kilobytes,
	 *   <strong>MB</strong> for megabytes, and <strong>GB</strong> for
	 *   gigabytes.
	 * </p>
	 *
	 * @param prop	A {@code Properties} instance which contains the
	 *		configuration of global resource.
	 * @param key	The key in the given property.
	 * @return	An {@code Integer} instance associated with the given
	 *		key in the property. {@code null} is returned if
	 *		the given key is not defined, or it is associated
	 *		with an invalid value.
	 */
	private Integer fetchSize(Properties prop, String key)
	{
		String v = prop.getProperty(key);
		if (v == null) {
			return null;
		}

		v = v.trim().toUpperCase();
		String digit;
		long size;
		int idx;

		if ((idx = v.indexOf("KB")) != -1) {
			size = 1024;
			digit = v.substring(0, idx);
		}
		else if ((idx = v.indexOf("MB")) != -1) {
			size = 1024 * 1024;
			digit = v.substring(0, idx);
		}
		else if ((idx = v.indexOf("GB")) != -1) {
			size = 1024 * 1024 * 1024;
			digit = v.substring(0, idx);
		}
		else {
			size = 0;
			digit = v;
		}

		long lvalue;
		try {
			lvalue = Long.parseLong(digit);
			if (lvalue < 0 || lvalue > Integer.MAX_VALUE) {
				throw new NumberFormatException();
			}
		}
		catch (NumberFormatException e) {
			_log.warn(key + ": Ignore invalid value: " + v);

			return null;
		}

		if (size != 0) {
			long newval = lvalue * size;

			if (newval > Integer.MAX_VALUE) {
				_log.warn(key + ": Ignore too large value: " +
					  v);

				return null;
			}

			lvalue = newval;
		}

		Integer iv = new Integer((int)lvalue);
		_log.info(key + "=" + v + " (" + iv + ")");

		return iv;
	}

	/**
	 * <p>
	 *   Fetch integer from the given resource property.
	 * </p>
	 *
	 * @param prop	A {@code Properties} instance which contains the
	 *		configuration of global resource.
	 * @param key	The key in the given property.
	 * @return	An {@code Integer} instance associated with the given
	 *		key in the property. {@code null} is returned if
	 *		the given key is not defined, or it is associated
	 *		with an invalid value.
	 */
	private Integer fetchInteger(Properties prop, String key)
	{
		String v = prop.getProperty(key);
		if (v == null) {
			return null;
		}

		v = v.trim();
		long lvalue;
		try {
			lvalue = Long.parseLong(v);
			if (lvalue < 0 || lvalue > Integer.MAX_VALUE) {
				throw new NumberFormatException();
			}
		}
		catch (NumberFormatException e) {
			_log.warn(key + ": Ignore invalid value: " + v);

			return null;
		}

		Integer iv = new Integer((int)lvalue);
		_log.info(key + "=" + v + " (" + iv + ")");

		return iv;
	}

	/**
	 * <p>
	 *   Fetch boolean from the given resource property.
	 * </p>
	 *
	 * @param prop	A {@code Properties} instance which contains the
	 *		configuration of global resource.
	 * @param key	The key in the given property.
	 * @return	{@code true} only if the value associated with the
	 *		given key and it represents {@code true}.
	 *		Otherwise {@code false}.
	 */
	private boolean fetchBoolean(Properties prop, String key)
	{
		String v = prop.getProperty(key);
		if (v == null) {
			return false;
		}

		v = v.trim();
		_log.info(key + "=" + v);

		return Boolean.parseBoolean(v);
	}

	/**
	 * <p>
	 *   Fetch logging level from the given resource property.
	 * </p>
	 *
	 * @param prop	A {@code Properties} instance which contains the
	 *		configuration of global resource.
	 * @param key	The key in the given property.
	 * @return	A {@code LogLevel} instance associated with the given
	 *		key in the property. {@code null} is returned if
	 *		the given key is not defined, or it is associated
	 *		with an invalid value.
	 */
	private LogLevel fetchLogLevel(Properties prop, String key)
	{
		String v = prop.getProperty(key);
		if (v == null) {
			return null;
		}

		LogLevel lvl = LogLevel.forName(v);
		if (lvl == null) {
			_log.warn(key + ": Ignore invalid logging level: " +
				  v);
		}

		return lvl;
	}

	/**
	 * <p>
	 *   Fetch logging facility from the given resource property.
	 * </p>
	 *
	 * @param prop	A {@code Properties} instance which contains the
	 *		configuration of global resource.
	 * @param key	The key in the given property.
	 * @return	A {@code LogFactory} instance associated with the given
	 *		key in the property. {@code null} is returned if
	 *		the given key is not defined, or it is associated
	 *		with an invalid value.
	 */
	private LogFacility fetchLogFacility(Properties prop, String key)
	{
		String v = prop.getProperty(key);
		if (v == null) {
			return null;
		}

		LogFacility fac = LogFacility.forName(v);
		if (fac == null) {
			_log.warn(key + ": Ignore invalid logging facility: "
				  + v);
		}

		return fac;
	}

	/**
	 * <p>
	 *   Helper class to set logging level to a {@code LogConfiguration}
	 *   instance.
	 * </p>
	 */
	private abstract class LogLevelSetter
	{
		/**
		 * <p>
		 *   The name of property key associated with the logging
		 *   level.
		 * </p>
		 */
		private final String  _key;

		/**
		 * <p>
		 *   Construct a {@code LogLevelSetter} instance.
		 * </p>
		 *
		 * @param key	The name of property associated with the
		 *		logging level.
		 */
		private LogLevelSetter(String key)
		{
			_key = key;
		}

		/**
		 * <p>
		 *   Fetch a logging level from the given propery, and
		 *   set it to a {@code LogConfiguration} instance.
		 * </p>
		 *
		 * @param prop	A {@code Properties} instance which contains
		 *		the configuration of global resource.
		 */
		private void set(Properties prop)
		{
			LogLevel lvl = fetchLogLevel(prop, _key);

			if (lvl == null) {
				return;
			}

			try {
				setLogLevel(lvl);
				_log.info(_key + "=" + lvl);
			}
			catch (Exception e) {
				_log.error(_key + ": Unable to set logging " +
					   "level: " + lvl, e);
			}
		}

		/**
		 * <p>
		 *   Set logging level to a {@code LogConfiguration} instance.
		 * </p>
		 *
		 * @param lvl	A {@code LogLevel} instance to set.
		 */
		protected abstract void setLogLevel(LogLevel lvl);
	}

	/**
	 * <p>
	 *   Helper class to set IPC event tunable parameters to a
	 *   {@code IpcEventConfiguration} instance.
	 * </p>
	 */
	private abstract class IpcEventConfSetter
	{
		/**
		 * <p>
		 *   The name of property key associated with the IPC event
		 *   parameter.
		 * </p>
		 */
		private final String  _key;

		/**
		 * <p>
		 *   The minimum value of the parameter.
		 * </p>
		 */
		private final int  _min;

		/**
		 * <p>
		 *   The maximum value of the parameter.
		 * </p>
		 */
		private final int  _max;

		/**
		 * <p>
		 *   Construct a {@code IpcEventConfSetter} instance.
		 * </p>
		 *
		 * @param key	The name of property associated with the
		 *		IPC event parameter.
		 * @param min	The minimum value of the parameter.
		 * @param max	The minimum value of the parameter.
		 */
		private IpcEventConfSetter(String key, int min, int max)
		{
			_key = key;
			_min = min;
			_max = max;
		}

		/**
		 * <p>
		 *   Fetch an IPC event parameter from the given property,
		 *   and set it to a {@code IpcEventConfiguration} instance.
		 * </p>
		 *
		 * @param prop	A {@code Properties} instance which contains
		 *		the configuration of global resource.
		 */
		private void set(Properties prop)
		{
			Integer v = fetchInteger(prop, _key);

			if (v == null) {
				return;
			}

			int value = v.intValue();
			if (value == 0) {
				// Use default value.
				return;
			}

			if (value < _min) {
				_log.warn(_key + ": Ignore too small value: " +
					  v);

				return;
			}

			if (value > _max) {
				_log.warn(_key + ": Ignore too large value: " +
					  v);

				return;
			}

			try {
				setParameter(value);
				_log.info(_key + "=" + v);
			}
			catch (Exception e) {
				_log.error(_key + ": Unable to set IPC event" +
					   " parameter: " + v, e);
			}
		}

		/**
		 * <p>
		 *   Set the given parameter to an
		 *   {@code IpcEventConfiguration} instance.
		 * </p>
		 *
		 * @param value	An integer value to be set.
		 */
		protected abstract void setParameter(int value);
	}
}
