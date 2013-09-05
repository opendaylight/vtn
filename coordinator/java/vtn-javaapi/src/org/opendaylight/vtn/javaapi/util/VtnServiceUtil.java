/*
 * Copyright (c) 2012-2013 NEC Corporation
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

public class VtnServiceUtil {

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
		Set<Entry<String, JsonElement>> jsonSet = jsonObject.entrySet();
		/*
		 * iterate the loop for each entry
		 */
		for (Entry<String, JsonElement> entry : jsonSet) {
			/*
			 * if entry is type of Json Object
			 */
			if (!(entry.getValue() instanceof JsonNull)
					&& entry.getValue().isJsonObject()) {
				VtnServiceUtil.trimParamValues((JsonObject) entry.getValue());
				continue;
			}
			/*
			 * if entry is type of Json array
			 */
			else if (!(entry.getValue() instanceof JsonNull)
					&& entry.getValue().isJsonArray()) {
				JsonArray array = (JsonArray) entry.getValue();
				for (int index = 0; index < array.size(); index++) {
					VtnServiceUtil.trimParamValues(array.get(index)
							.getAsJsonObject());
					continue;
				}
				continue;
			}
			/*
			 * if entry is primitive type value
			 */
			else if (!(entry.getValue() instanceof JsonNull)
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
	public static JsonObject removeEmptyParamas(final JsonObject jsonObject){
		// extract all the entries in Json Object
		Set<Entry<String, JsonElement>> jsonSet = jsonObject.entrySet();
		/*
		 * iterate the loop for each entry
		 */
		for (Entry<String, JsonElement> entry : jsonSet) {
			/*
			 * if entry is type of Json Object
			 */
			if (!(entry.getValue() instanceof JsonNull)
					&& entry.getValue().isJsonObject()) {
				VtnServiceUtil.removeEmptyParamas((JsonObject) entry.getValue());
				continue;
			}
			/*
			 * if entry is type of Json array
			 */
			else if (!(entry.getValue() instanceof JsonNull)
					&& entry.getValue().isJsonArray()) {
				JsonArray array = (JsonArray) entry.getValue();
				for (int index = 0; index < array.size(); index++) {
					VtnServiceUtil.removeEmptyParamas(array.get(index)
							.getAsJsonObject());
					continue;
				}
				continue;
			}
			/*
			 * if entry is primitive type value
			 */
			else if (!(entry.getValue() instanceof JsonNull)
					&& entry.getValue().getAsString()
							.equals("")) {
				jsonObject.remove(entry.getKey());
			}
		}
		return jsonObject;
	}
}
