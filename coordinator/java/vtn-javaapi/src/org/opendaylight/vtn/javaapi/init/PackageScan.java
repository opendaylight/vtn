/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.init;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.URISyntaxException;
import java.net.URL;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.jar.JarEntry;
import java.util.jar.JarInputStream;

import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;

/**
 * The Class PackageScan. Scans the resource classes for VTN Service
 */
public final class PackageScan {
	/**
	 * Pairs of REST resource path and associated resource class.
	 */
	private final HashMap<String, Class<?>> _cachedClasses =
		new HashMap<String, Class<?>>();

	/**
	 * Private class that keeps a single global instance of
	 * {@code PackageScan}.
	 */
	private final static class PackageScanHolder {
		/**
		 * A single global instance of {@code PackageScan}.
		 */
		private final static PackageScan _instance = new PackageScan();
	}

	/**
	 * Return a single global instance of {@code PackageScan}.
	 * 
	 * @return A single global instance of {@code PackageScan}.
	 */
	public final static PackageScan getInstance() {
		return PackageScanHolder._instance;
	}

	/**
	 * Create a single global instance of {@code PackageScan}.
	 * 
	 * @throws IllegalStateException
	 *             Failed to load properties file that defines resource clas
ses.
        */
	private PackageScan() {
		Logger log = Logger.getLogger(PackageScan.class.getName());
		log.trace("Start PackageScan#PackageScan()");

		// create the list of resource classes from a given package
		String jar = VtnServiceInitManager.
			getConfigurationMap().
			getConfigValue(VtnServiceConsts.JAVAAPI_JARPATH);
		List<String> classNameList;
		try {
			classNameList =
				getFromJARFile(jar, VtnServiceConsts.RESOURCES);
		} catch (FileNotFoundException e) {
			String msg = "Java API JAR file not found: " + jar;
			log.error(e, msg + ": " + e);
			throw new IllegalStateException(msg, e);
		} catch (IOException e) {
			String msg = "Failed to load Java API JAR file: " + jar;
			log.error(e, msg + ": " + e);
			throw new IllegalStateException(msg, e);
		}

		for (final String className : classNameList) {
			// check the class validity for each resource
			// class
			Class<?> cls = null;
			try {
				cls = Class.forName(className);
			} catch (ClassNotFoundException e) {
				log.warning(e,
					    "Failed to load resource class: " +
					    className + ": " + e);
				continue;
			}

			// continue if resource class is not derived
			// from AbstractClass
			if (cls == null ||
			    AbstractResource.class != cls.getSuperclass()) {
				continue;
			}

			final UNCVtnService annotion =
				cls.getAnnotation(UNCVtnService.class);
			if (annotion == null) {
				continue;
			}

			// map the annotation path with resource class
			final String path = annotion.path();
			if (path != null && path.trim().length() != 0) {
				_cachedClasses.put(path, cls);
			}
		}
		log.trace("Complete PackageScan#PackageScan()");
	}

	/**
	 * Load the resources from vtn-javaapi jar
	 * 
	 * @param jar
	 *            : javaapi jar file name
	 * @param packageName
	 * @return list of class names
	 * @throws FileNotFoundException
	 * @throws IOException
	 */
	private List<String> getFromJARFile(final String jar,
					    final String packageName)
		throws FileNotFoundException, IOException {
		final List<String> classes = new ArrayList<String>();
		final JarInputStream jarFile = 
			new JarInputStream(new FileInputStream(jar));

		/*
		 * iterate for all the class files in Jar file
		 */
		for (JarEntry jarEntry = jarFile.getNextJarEntry();
		     jarEntry != null; jarEntry = jarFile.getNextJarEntry()) {
			String className = jarEntry.getName();
			if (!className.endsWith(VtnServiceConsts.CLASS_EXT)) {
				continue;
			}

			className = stripFilenameExtension(className);
			if (className.startsWith(packageName)) {
				classes.add(className.replaceAll
					    (VtnServiceConsts.SLASH,
					     VtnServiceConsts.DOT));
			}
		}
		jarFile.close();
		return classes;
	}

	/**
	 * Return a resource class associated with the given path.
	 * 
	 * @param path  A path.
	 * @return A {@code Class} instance associated with the given path
	 *         if found. {@code null} if not found.
	 */
	public Class<?> getResourceClass(String path) {
		return _cachedClasses.get(path);
	}

	/**
	 * Return a string array which contains all REST paths.
	 * 
	 * @return A string array which contains all REST paths.
	 */
	public String[] getAllPaths() {
		String[] array = new String[_cachedClasses.size()];

		return _cachedClasses.keySet().toArray(array);
	}

	/**
	 * Stripping the class name
	 * 
	 * @param className
	 * @return
	 */
	private static String stripFilenameExtension(final String className) {
		return className.substring(0, className.lastIndexOf('.'));
	}
}
