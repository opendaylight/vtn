/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core;

import java.util.Properties;
import java.io.File;
import java.io.InputStream;

/**
 * <p>
 *   The {@code CoreSystem} class provides several useful class
 *   methods and constants related to PFC-Core system.
 *   This class can not be instantiated.
 * </p>
 *
 * @since	C10
 */
public final class CoreSystem
{
	/**
	 * <p>
	 *   Software product name.
	 * </p>
	 */
	public final static String  PRODUCT_NAME;

	/**
	 * <p>
	 *   Human readable string which represents the software version
	 *   number.
	 * </p>
	 */
	public final static String  VERSION;

	/**
	 * <p>
	 *   Major number of the software version.
	 * </p>
	 */
	public final static int  VERSION_MAJOR;

	/**
	 * <p>
	 *   Minor number of the software version.
	 * </p>
	 */
	public final static int  VERSION_MINOR;

	/**
	 * <p>
	 *   Revision number of the software version.
	 * </p>
	 */
	public final static int  VERSION_REVISION;

	/**
	 * <p>
	 *   Patch level of the software version.
	 * </p>
	 */
	public final static int  VERSION_PATCHLEVEL;

	/**
	 * <p>
	 *   Determine whether the PFC-Core system is built with debugging
	 *   option.
	 * </p>
	 */
	public final static boolean  DEBUG;

	/**
	 * <p>
	 *   Installation directory prefix.
	 * </p>
	 */
	public final static String  INST_PREFIX;

	/**
	 * <p>
	 *   Installation directory for user executables.
	 * </p>
	 */
	public final static String  INST_BINDIR;

	/**
	 * <p>
	 *   Installation directory for read-only data.
	 * </p>
	 */
	public final static String  INST_DATADIR;

	/**
	 * <p>
	 *   Installation directory for library files.
	 * </p>
	 */
	public final static String  INST_LIBDIR;

	/**
	 * <p>
	 *   Installation directory for program executables.
	 * </p>
	 */
	public final static String  INST_LIBEXECDIR;

	/**
	 * <p>
	 *   Installation directory for modifiable system data.
	 * </p>
	 */
	public final static String  INST_LOCALSTATEDIR;

	/**
	 * <p>
	 *   Installation directory for system admin executables.
	 * </p>
	 */
	public final static String  INST_SBINDIR;

	/**
	 * <p>
	 *   Installation directory for read-only system configuration.
	 * </p>
	 */
	public final static String  INST_SYSCONFDIR;

	/**
	 * <p>
	 *   Installation directory for Java libraries.
	 * </p>
	 */
	public final static String  JAVA_LIBDIR;

	/**
	 * <p>
	 *   Installation directory for JNI shared libraries.
	 * </p>
	 */
	public final static String  JNI_LIBDIR;

	/**
	 * <p>
	 *   Library search path specified by system property
	 *   <strong>pflow.core.libpath</strong>.
	 * </p>
	 */
	private final static String[] _libraryPath;

	/**
	 * <p>
	 *   Initialize class variables.
	 * </p>
	 *
	 * @throws IllegalStateException
	 *	Unable to initialize build configuration.
	 */
	static {
		// Load build configuration.
		String conf = "buildconf.properties";
		Properties prop = new Properties();
		InputStream in = CoreSystem.class.getResourceAsStream(conf);
		if (in == null) {
			throw new IllegalStateException
				("Unable to open build configuration: " +
				 conf);
		}

		try {
			prop.load(in);
			in.close();

			// Initialize constants which represent build
			// configuration.
			PRODUCT_NAME = prop.getProperty("product.name");
			VERSION = prop.getProperty("product.version");
			VERSION_MAJOR = getIntProperty
				(prop, "product.version.major");
			VERSION_MINOR = getIntProperty
				(prop, "product.version.minor");
			VERSION_REVISION = getIntProperty
				(prop, "product.version.revision");
			VERSION_PATCHLEVEL = getIntProperty
				(prop, "product.version.patchlevel");

			DEBUG = Boolean.parseBoolean
				(prop.getProperty("debug"));

			INST_PREFIX = prop.getProperty("inst.prefix");
			INST_BINDIR = prop.getProperty("inst.bindir");
			INST_DATADIR = prop.getProperty("inst.datadir");
			INST_LIBDIR = prop.getProperty("inst.libdir");
			INST_LIBEXECDIR = prop.getProperty("inst.libexecdir");
			INST_LOCALSTATEDIR =
				prop.getProperty("inst.localstatedir");
			INST_SBINDIR = prop.getProperty("inst.sbindir");
			INST_SYSCONFDIR = prop.getProperty("inst.sysconfdir");

			JAVA_LIBDIR = prop.getProperty("inst.java.jardir");
			JNI_LIBDIR = prop.getProperty("inst.java.jnidir");
		}
		catch (Exception e) {
			throw new IllegalStateException
				("Unable to load build configuration: " + conf,
				 e);
		}

		if (PRODUCT_NAME == null || VERSION == null ||
		    INST_PREFIX == null || INST_BINDIR == null ||
		    INST_DATADIR == null || INST_LIBDIR == null ||
		    INST_LIBEXECDIR == null || INST_LOCALSTATEDIR == null ||
		    INST_SBINDIR == null || INST_SYSCONFDIR == null ||
		    JAVA_LIBDIR == null || JNI_LIBDIR == null) {
			throw new IllegalStateException
				("Build configuration is broken.");
		}

		String libpath = System.getProperty("pflow.core.libpath");

		_libraryPath = (libpath == null) ? null : libpath.split(":");
	}

	/**
	 * <p>
	 *   Read property value as integer.
	 * </p>
	 *
	 * @param prop	A {@code Properties} instance.
	 * @param key	A property key.
	 * @return	An integer associated with the given key.
	 * @throws NumberFormatException
	 *	A property value associated with the given key is not a
	 *	string representation of integer.
	 */
	private static int getIntProperty(Properties prop, String key)
	{
		return Integer.parseInt(prop.getProperty(key));
	}

	/**
	 * <p>
	 *   Private constructor which will never be called.
	 * </p>
	 */
	private CoreSystem()
	{
	}

	/**
	 * <p>
	 *   Load the JNI shared library associated with the specified name.
	 * </p>
	 * <p>
	 *   {@code name} must be a library name without prefix and
	 *   suffix. For instance, if your JNI library is "libfoo.so",
	 *   you must specify "foo" to {@code name}.
	 * </p>
	 * <p>
	 *   The path to directories which contains JNI library file can be
	 *   specified by the system property {@code pflow.core.libpath}.
	 *   Its value must be the list of directories, separated by
	 *   {@code ":"}.
	 *   Irrespective of whether {@code pflow.core.libpath} is defined,
	 *   the default library directory, {@link #JNI_LIBDIR}, is
	 *   always searched.
	 * </p>
	 *
	 * @param name	The name of JNI library.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws UnsatisfiedLinkError
	 *	The specified library does not exist.
	 * @throws SecurityException
	 *	A security manager does not allow to load library.
	 */
	public static void loadLibrary(String name)
	{
		if (_libraryPath != null) {
			// Library search path is specified.
			for (int idx = 0; idx < _libraryPath.length; idx++) {
				if (loadLibrary(_libraryPath[idx], name)) {
					return;
				}
			}
		}

		// Load the library file under the default directory.
		System.load(createLibraryPath(JNI_LIBDIR, name));
	}

	/**
	 * <p>
	 *   Try to load the JNI shared library.
	 * </p>
	 * <p>
	 *   This method does nothing if the specified library file does not
	 *   exist.
	 * </p>
	 *
	 * @param dir	Path to parent directory.
	 * @param name	The name of JNI library.
	 * @return	{@code true} if the library file has been successfully
	 *		loaded. Otherwise {@code false}.
	 * @throws NullPointerException
	 *	{@code dir} or {@code name} is null.
	 */
	private static boolean loadLibrary(String dir, String name)
	{
		String path = createLibraryPath(dir, name);
		File   file = new File(path);

		// Return false if the path is relative, or the file does
		// not exist.
		if (!file.isAbsolute() || !file.isFile()) {
			return false;
		}

		// Try to load the library.
		try {
			System.load(path);
		}
		catch (Exception e) {
			return false;
		}

		return true;
	}

	/**
	 * <p>
	 *   Construct JNI library file path.
	 * </p>
	 *
	 * @param dir	Path to parent directory.
	 * @param name	The name of JNI library.
	 * @throws NullPointerException
	 *	{@code dir} or {@code name} is null.
	 */
	private static String createLibraryPath(String dir, String name)
	{
		return dir + File.separator + System.mapLibraryName(name) +
			"." + VERSION_MAJOR;
	}
}
