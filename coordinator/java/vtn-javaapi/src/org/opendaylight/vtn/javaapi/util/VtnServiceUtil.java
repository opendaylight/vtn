/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.util;

import java.lang.reflect.Field;
import java.util.Map.Entry;
import java.util.Set;

import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonNull;
import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.annotation.UNCField;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.resources.SessionsResource;
import org.opendaylight.vtn.javaapi.resources.UserResource;
import org.opendaylight.vtn.javaapi.resources.physical.ControllerResource;
import org.opendaylight.vtn.javaapi.resources.physical.ControllersResource;

public class VtnServiceUtil {

	private static final Logger LOG = Logger.getLogger(VtnServiceUtil.class
			.getName());

	/**
	 * Check the validity of Json Object
	 * 
	 * @param valueStr
	 * @return true if String is valid, otherwise false
	 */
	public static boolean isValidString(final String value) {
		boolean status = false;
		if (null != value && !value.isEmpty()) {
			status = true;
		} else {
			status = false;
		}
		return status;
	}

	/**
	 * Trim the values of all the parameters of given Json
	 * 
	 * @param resource
	 * 
	 * @param Json
	 *            object that require to be update
	 * @return Updated Json Object
	 * @throws IllegalAccessException
	 * @throws IllegalArgumentException
	 */
	public static JsonObject trimParamValues(final JsonObject jsonObject,
			AbstractResource resource) throws IllegalArgumentException,
			IllegalAccessException {
		LOG.trace("start VtnServiceUtil#trimParamValues()");
		// extract all the entries in Json Object
		final Set<Entry<String, JsonElement>> jsonSet = jsonObject.entrySet();
		/*
		 * iterate the loop for each entry
		 */
		for (final Entry<String, JsonElement> entry : jsonSet) {
			/*
			 * if entry is type of Json Object
			 */
			if (!(entry.getValue() instanceof JsonNull)
					&& entry.getValue().isJsonObject()) {
				VtnServiceUtil.trimParamValues((JsonObject) entry.getValue(),
						resource);
				continue;
			} else if (!(entry.getValue() instanceof JsonNull)
					&& entry.getValue().isJsonArray()) {
				final JsonArray array = (JsonArray) entry.getValue();
				for (int index = 0; index < array.size(); index++) {
					if (array.get(index).isJsonObject()) {
						VtnServiceUtil.trimParamValues(array.get(index)
								.getAsJsonObject(), resource);
						continue;
					}
				}
				continue;
			} else if (!(entry.getValue() instanceof JsonNull)
					&& !entry.getValue().getAsString()
							.equals(entry.getValue().getAsString().trim())) {
				String paramName = entry.getKey();
				if (!isallParamValuesValid(paramName, resource)) {
					resource.getValidator().setInvalidParameter(paramName);
					LOG.debug("validation failed for : " + paramName);
					throw new IllegalArgumentException(
							"triming validation failed for : " + paramName);
				}
			}
		}
		/*
		 * check URI parameters, leading and trailing spaces are not allowed for
		 * URI parameters as they are always used as key informations
		 */
		for (Field field : resource.getClass().getDeclaredFields()) {
			LOG.info("check annotated resource variables for leading and trailing spaces");
			UNCField uncField = field.getAnnotation(UNCField.class);
			field.setAccessible(true);
			if (uncField != null
					&& !field.get(resource).toString().trim()
							.equals(field.get(resource))) {
				final String value = uncField.value();
				resource.getValidator().setInvalidParameter(value);
				LOG.debug("validation failed for : " + value);
				throw new IllegalArgumentException(
						"triming validation failed for : " + value);
			}
		}
		LOG.trace("complete VtnServiceUtil#trimParamValues()");
		return jsonObject;
	}

	/**
	 * Verify that parameter is allowed or not to contain leading and trailing
	 * spaces in string values
	 * 
	 * @param resource
	 *            - instance of concrete resource class
	 * @param paramName
	 *            - parameter name to be verified
	 * @return true, if parameter is allowed to contain leading and trailing
	 *         spaces. Otherwise return false
	 */
	private static boolean isallParamValuesValid(String paramName,
			AbstractResource resource) {
		LOG.trace("start VtnServiceUtil#isallParamValuesValid()");
		boolean flag;
		if (paramName.equals(VtnServiceJsonConsts.DESCRIPTION)) {
			// description parameter is allowed to contains leading and trailing
			// spaces for all APIs
			flag = true;
		} else if (resource instanceof SessionsResource
				&& (paramName.equals(VtnServiceJsonConsts.LOGIN_NAME) || paramName
						.equals(VtnServiceJsonConsts.INFO) || paramName
						.equals(VtnServiceJsonConsts.PASSWORD))) {
			flag = true;
		} else if ((resource instanceof ControllersResource || resource instanceof ControllerResource)
				&& (paramName.equals(VtnServiceJsonConsts.USERNAME) || paramName
						.equals(VtnServiceJsonConsts.PASSWORD))) {
			flag = true;
		} else if (resource instanceof UserResource
				&& paramName.equals(VtnServiceJsonConsts.PASSWORD)) {
			flag = true;
		} else {
			flag = false;
		}
		LOG.debug("validation status for " + paramName + " is " + flag);
		LOG.trace("complete VtnServiceUtil#isallParamValuesValid()");
		return flag;
	}

	/**
	 * Remove the parameters of given Json which contain empty string
	 * 
	 * @param Json
	 *            object that require to be update
	 * @return Updated Json Object
	 */
	public static JsonObject removeEmptyParamas(final JsonObject jsonObject) {
		// extract all the entries in Json Object
		final Set<Entry<String, JsonElement>> jsonSet = jsonObject.entrySet();
		/*
		 * iterate the loop for each entry
		 */
		for (final Entry<String, JsonElement> entry : jsonSet) {
			/*
			 * if entry is type of Json Object
			 */
			if (!(entry.getValue() instanceof JsonNull)
					&& entry.getValue().isJsonObject()) {
				VtnServiceUtil
						.removeEmptyParamas((JsonObject) entry.getValue());
				continue;
			} else if (!(entry.getValue() instanceof JsonNull)
					&& entry.getValue().isJsonArray()) {
				final JsonArray array = (JsonArray) entry.getValue();
				for (int index = 0; index < array.size(); index++) {
					VtnServiceUtil.removeEmptyParamas(array.get(index)
							.getAsJsonObject());
					continue;
				}
				continue;
			} else if (!(entry.getValue() instanceof JsonNull)
					&& entry.getValue().getAsString()
							.equals(VtnServiceConsts.EMPTY_STRING)) {
				jsonObject.remove(entry.getKey());
			}
		}
		return jsonObject;
	}

	/**
	 * 
	 * @param resource
	 * @return
	 */
	public static boolean isOpenStackResurce(AbstractResource resource) {
		LOG.trace("Start VtnServiceUtil#isOpenStackResurce()");
		boolean status = false;
		if (resource == null) {
			LOG.debug("Decision cannot be taken that request came for UNC core APIs or UNC OpenStack APIs");
		} else if (resource.getClass().getCanonicalName()
				.startsWith(VtnServiceOpenStackConsts.OS_RESOURCE_PKG)) {
			LOG.debug("Request came for UNC OpenStack APIs");
			status = true;
		}
		LOG.trace("Complete VtnServiceUtil#isOpenStackResurce()");
		return status;
	}
}
