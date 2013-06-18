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

	private static final Logger LOG = Logger.getLogger(PackageScan.class
			.getName());

	private static final Map<String, Class<?>> CACHEDRESOURCES = new HashMap<String, Class<?>>();

	private PackageScan() {
	}

	/**
	 * Load resources.
	 * 
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	public static void loadResources() throws VtnServiceException {

		LOG.trace("Start PackageScan#loadResources()");

		List<String> classNameList = null;
		// create the list of resource classes from a given package
		try {
			//classNameList = getClassNameList(VtnServiceConsts.TEST_RESOURCES);
			classNameList = getFromJARFile(
					VtnServiceInitManager.getConfigurationMap().getConfigValue(
							VtnServiceConsts.JAVAAPI_JARPATH),
					VtnServiceConsts.RESOURCES);

			if (classNameList == null) {
				return;
			}

			for (final String className : classNameList) {
				Class<?> classResource = null;

				// check the class validity for each resource class
				classResource = Class.forName(className);

				// continue if resource class is not derived from AbstractClass
				if (classResource == null
						|| AbstractResource.class != classResource
								.getSuperclass()) {
					continue;
				}

				final UNCVtnService annotion = classResource
						.getAnnotation(UNCVtnService.class);
				if (annotion == null) {
					continue;
				}

				// map the annotation path with resource class
				final String path = annotion.path();
				if (path != null
						&& !VtnServiceConsts.EMPTY_STRING.equals(path.trim())) {
					CACHEDRESOURCES.put(path, classResource);
				}
			}
		} catch (final Exception e) {
			VtnServiceInitManager
					.getExceptionHandler()
					.raise(Thread.currentThread().getStackTrace()[1]
							.getClassName()
							+ VtnServiceConsts.HYPHEN
							+ Thread.currentThread().getStackTrace()[1]
									.getMethodName(),
							UncJavaAPIErrorCode.RESOURCE_LOAD_ERROR
									.getErrorCode(),
							UncJavaAPIErrorCode.RESOURCE_LOAD_ERROR
									.getErrorMessage(), e);
		}
		LOG.trace("Complete PackageScan#loadResources()");
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
	 * @throws ClassNotFoundException
	 */
	static List<String> getFromJARFile(final String jar,
			final String packageName) throws FileNotFoundException,
			IOException, ClassNotFoundException {
		final List<String> classes = new ArrayList<String>();
		final JarInputStream jarFile = new JarInputStream(new FileInputStream(
				jar));
		JarEntry jarEntry;
		/*
		 * iterate for all the class files in Jar file
		 */
		do {
			jarEntry = jarFile.getNextJarEntry();
			if (jarEntry != null) {
				String className = jarEntry.getName();
				if (className.endsWith(VtnServiceConsts.CLASS_EXT)) {
					className = stripFilenameExtension(className);
					if (className.startsWith(packageName)) {
						classes.add(className.replaceAll(
								VtnServiceConsts.SLASH, VtnServiceConsts.DOT));
					}
				}
			}
		} while (jarEntry != null);
		jarFile.close();
		return classes;
	}

	/**
	 * Stripping the class name
	 * 
	 * @param className
	 * @return
	 */
	public static String stripFilenameExtension(final String className) {
		final String str = className.substring(0, className.lastIndexOf('.'));
		return str;
	}

	/**
	 * Gets the class name list.
	 * 
	 * @param packageName
	 *            the package name
	 * @return the class name list
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	private static List<String> getClassNameList(final String packageName)
			throws VtnServiceException {

		LOG.trace("Start PackageScan#getClassNameList()");
		LOG.debug("Input value for package name : " + packageName);
		final List<String> classNames = new ArrayList<String>();
		final ClassLoader loader = Thread.currentThread()
				.getContextClassLoader();
		try {
			// load all the classes from given package
			final String resourceName = packageName.replaceAll(
					VtnServiceConsts.DOT_REGEX, VtnServiceConsts.SLASH);
			final URL url = loader.getResource(resourceName);
			final File urlFile = new File(url.toURI());
			final File[] files = urlFile.listFiles();
			// get class name of each file present in package
			for (final File f : files) {
				getClassName(packageName, f, classNames);
			}
		} catch (final URISyntaxException e) {
			VtnServiceInitManager
					.getExceptionHandler()
					.raise(Thread.currentThread().getStackTrace()[1]
							.getClassName()
							+ VtnServiceConsts.HYPHEN
							+ Thread.currentThread().getStackTrace()[1]
									.getMethodName(),
							UncJavaAPIErrorCode.INIT_ERROR.getErrorCode(),
							UncJavaAPIErrorCode.INIT_ERROR.getErrorCode(), e);
		}
		LOG.debug("Class list:" + classNames.toArray());
		LOG.trace("Complete PackageScan#getClassNameList()");
		return classNames;
	}

	/**
	 * Gets the class name.
	 * 
	 * @param packageName
	 *            the package name
	 * @param packageFile
	 *            the package file
	 * @param list
	 *            the list
	 * @return the class name
	 */
	private static void getClassName(final String packageName,
			final File packageFile, final List<String> list) {

		LOG.trace("Start PackageScan#getClassName()");
		if (packageFile.isFile()) {
			list.add(packageName
					+ VtnServiceConsts.DOT
					+ packageFile.getName().replace(VtnServiceConsts.CLASS_EXT,
							VtnServiceConsts.EMPTY_STRING));
		} else {
			final File[] files = packageFile.listFiles();
			final String tmPackageName = packageName + VtnServiceConsts.DOT
					+ packageFile.getName();
			for (final File file : files) {
				getClassName(tmPackageName, file, list);
			}
		}
		LOG.trace("Complete PackageScan#getClassName()");
	}

	/**
	 * Gets the cached resources.
	 * 
	 * @return the cached resources
	 */
	public static Map<String, Class<?>> getCachedResources() {
		LOG.trace("Return from PackageScan#getCachedResources()");
		return CACHEDRESOURCES;
	}
}
