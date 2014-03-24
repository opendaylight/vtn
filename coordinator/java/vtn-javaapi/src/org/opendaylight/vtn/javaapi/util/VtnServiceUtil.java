/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.util;

import java.util.Map.Entry;
import java.util.Set;

import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonNull;
import com.google.gson.JsonObject;
import com.google.gson.JsonPrimitive;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;

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
	 * @param Json
	 *            object that require to be update
	 * @return Updated Json Object
	 */
	public static JsonObject trimParamValues(final JsonObject jsonObject) {
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
				VtnServiceUtil.trimParamValues((JsonObject) entry.getValue());
				continue;
			} else if (!(entry.getValue() instanceof JsonNull)
					&& entry.getValue().isJsonArray()) {
				final JsonArray array = (JsonArray) entry.getValue();
				for (int index = 0; index < array.size(); index++) {
					if (array.get(index).isJsonObject()) {
						VtnServiceUtil.trimParamValues(array.get(index)
								.getAsJsonObject());
						continue;
					}
				}
				continue;
			} else if (!(entry.getValue() instanceof JsonNull)
					&& !entry.getValue().getAsString()
							.equals(entry.getValue().getAsString().trim())) {
				entry.setValue(new JsonPrimitive(entry.getValue().getAsString()
						.trim()));
			}
		}
		return jsonObject;
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
