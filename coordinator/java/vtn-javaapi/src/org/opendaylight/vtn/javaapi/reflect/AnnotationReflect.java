/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.reflect;

import java.lang.reflect.Field;
import java.net.URLDecoder;
import java.util.HashMap;
import java.util.Map;

import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.annotation.UNCField;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.exception.VtnServiceExceptionHandler;
import org.opendaylight.vtn.javaapi.init.PackageScan;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;

/**
 * The Class AnnotationReflect. Resolve resource class corresponding to URI
 */
public final class AnnotationReflect {

	private static final Logger LOG = Logger.getLogger(AnnotationReflect.class
			.getName());

	private AnnotationReflect() {

	}

	/**
	 * Gets the resource.
	 * 
	 * @param path
	 *            the path
	 * @param exceptionHandler
	 * @return the resource
	 * @throws VtnServiceException 
	 */
	public static AbstractResource getResource(final String path,
			final VtnServiceExceptionHandler exceptionHandler) throws VtnServiceException {
		LOG.trace("Start AnnotationReflect#getResource()");
		LOG.debug("Resource path: " + path);
		AbstractResource resource = null;

		final Map<String, String> fields = new HashMap<String, String>();

		// match the path with scanned resources to get the key corresponding to
		// resource class. Also set the URI parameters
		final String key = matchResource(fields, path);
		if (key != null) {
			PackageScan pscan = PackageScan.getInstance();
			final Class<?> resourceClass = pscan.getResourceClass(key);
			if (resourceClass != null) {
				try {
					// instantiate the resource class
					resource = (AbstractResource) resourceClass.newInstance();

					final Field[] fieldsClass = resourceClass
							.getDeclaredFields();

					/*
					 * set annotated field from URI parameters
					 */
					for (final Field field : fieldsClass) {
						final UNCField uncField = field
								.getAnnotation(UNCField.class);
						if (uncField == null) {
							continue;
						}
						final String value = uncField.value();

						if (!fields.containsKey(value)) {
							continue;
						}

						field.setAccessible(true);
						field.set(resource, URLDecoder.decode(
								fields.get(value), VtnServiceConsts.UTF8));
					}
				} catch (final Exception e) {
					exceptionHandler.raise(
							Thread.currentThread().getStackTrace()[1]
									.getClassName()
									+ VtnServiceConsts.HYPHEN
									+ Thread.currentThread().getStackTrace()[1]
											.getMethodName(),
							UncJavaAPIErrorCode.INTERNAL_ERROR.getErrorCode(),
							UncJavaAPIErrorCode.INTERNAL_ERROR
									.getErrorMessage(), e);
				}
			}
		} else {
			LOG.warning("No resource found");
		}
		LOG.trace("Complete AnnotationReflect#getResource()");
		return resource;
	}

	/**
	 * Match resource corresponding to given URI (path)
	 * 
	 * @param fields
	 *            the fields
	 * @param path
	 *            the path
	 * @return the string
	 */
	private static String matchResource(final Map<String, String> fields,
			final String path) {
		LOG.trace("Start AnnotationReflect#matchResource()");
		String returnKey = null;
		if (fields == null) {
			return returnKey;
		}
		// iterate for each value of KeySet in scanned resource classes
		PackageScan pscan = PackageScan.getInstance();
		for (final String key : pscan.getAllPaths()) {
			if (returnKey != null) {
				break;
			}
			// split key and path by slash
			final String[] urlSeg = key.split(VtnServiceConsts.SLASH);
			final String[] pathSeg = path.split(VtnServiceConsts.SLASH);
			returnKey = key;

			// size must be same to get the proper match
			if (urlSeg.length != pathSeg.length) {
				returnKey = null;
				continue;
			}

			String fieldName = null;
			String fieldValue = null;

			// check each part of key and path
			for (int i = 1; i < urlSeg.length; i++) {
				// if current part is URI parameter
				if (urlSeg[i].startsWith(VtnServiceConsts.OPEN_CURLY_BRACES)
						&& urlSeg[i]
								.endsWith(VtnServiceConsts.CLOSE_CURLY_BRACES)) {
					String regex = null;
					String temp = null;
					int index = -1;

					/*
					 * Get URI parameter values and add them to map, so that
					 * they can be used later to initialize the resource class
					 */
					temp = urlSeg[i].substring(1, urlSeg[i].length() - 1);
					index = temp.indexOf(':');

					if (index == -1) {
						fieldName = temp.trim();
					} else {
						fieldName = temp.substring(0, index).trim();
						regex = temp.substring(index).trim();
					}

					if (regex == null) {
						fieldValue = pathSeg[i];
					} else {
						if (!pathSeg[i].matches(regex)) {
							returnKey = null;
							continue;
						}
						fieldValue = pathSeg[i];
					}
					fields.put(fieldName, fieldValue);
				} else {
					// if current part is not URI parameter, then key and path
					// parameters must be same
					if (!urlSeg[i].equals(pathSeg[i])) {
						returnKey = null;
						continue;
					}
				}
			}
		}
		if (returnKey == null) {
			fields.clear();
		}
		LOG.trace("Complete AnnotationReflect#matchResource()");
		return returnKey;
	}
}
