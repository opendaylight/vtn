/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.openstack.convertor;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;

/**
 * Provides Request-Response Conversion methods for Router and Router Interface
 * Requests
 */
public class VrtResourcesGenerator {

	/**
	 * Generated Create vRouter request body from OpenStack request body
	 * 
	 * @param openStackResourceBody
	 *            - OpenStack request body for tenant
	 * @return - vRouter request body
	 */
	public static JsonObject getCreateVrtRequestBody(
			JsonObject openStackResourceBody) {
		final JsonObject root = new JsonObject();
		final JsonObject vrt = new JsonObject();

		vrt.addProperty(VtnServiceJsonConsts.VRTNAME, openStackResourceBody
				.get(VtnServiceOpenStackConsts.ID).getAsString());

		if (openStackResourceBody.has(VtnServiceOpenStackConsts.DESCRIPTION)
				&& !openStackResourceBody.get(
						VtnServiceOpenStackConsts.DESCRIPTION).isJsonNull()) {
			vrt.addProperty(
					VtnServiceJsonConsts.DESCRIPTION,
					openStackResourceBody.get(
							VtnServiceOpenStackConsts.DESCRIPTION)
							.getAsString());

		}

		vrt.addProperty(VtnServiceJsonConsts.CONTROLLERID,
				openStackResourceBody.get(VtnServiceJsonConsts.CONTROLLERID)
						.getAsString());

		vrt.addProperty(VtnServiceJsonConsts.DOMAINID,
				VtnServiceJsonConsts.DEFAULT_DOMAIN_ID);

		root.add(VtnServiceJsonConsts.VROUTER, vrt);
		return root;
	}

	/**
	 * Generated Update vRouter request body from OpenStack request body
	 * 
	 * @param openStackResourceBody
	 *            - OpenStack request body for tenant
	 * @return - vRouter request body
	 */
	public static JsonObject getUpdateVrtRequestBody(
			JsonObject openStackResourceBody) {
		final JsonObject root = new JsonObject();
		final JsonObject vrt = new JsonObject();

		if (!openStackResourceBody.has(VtnServiceOpenStackConsts.DESCRIPTION)
				|| openStackResourceBody.get(
						VtnServiceOpenStackConsts.DESCRIPTION).isJsonNull()
				|| openStackResourceBody
						.get(VtnServiceOpenStackConsts.DESCRIPTION)
						.getAsString().isEmpty()) {
			vrt.addProperty(VtnServiceJsonConsts.DESCRIPTION,
					VtnServiceConsts.EMPTY_STRING);
		} else {
			vrt.addProperty(
					VtnServiceJsonConsts.DESCRIPTION,
					openStackResourceBody.get(
							VtnServiceOpenStackConsts.DESCRIPTION)
							.getAsString());

		}

		root.add(VtnServiceJsonConsts.VROUTER, vrt);
		return root;
	}

	/**
	 * Generated Create vRouter interface request body from OpenStack request
	 * body
	 * 
	 * @param openStackResourceBody
	 *            - OpenStack request body for tenant
	 * @return - vRouter interface request body
	 */
	public static JsonObject getCreateVrtIfRequestBody(
			JsonObject openStackResourceBody) {
		final JsonObject root = new JsonObject();
		final JsonObject interfaceJson = new JsonObject();

		interfaceJson.addProperty(VtnServiceJsonConsts.IFNAME,
				openStackResourceBody.get(VtnServiceOpenStackConsts.ID)
						.getAsString());
		setIpAndMacAddress(openStackResourceBody, interfaceJson);
		root.add(VtnServiceJsonConsts.INTERFACE, interfaceJson);
		return root;
	}

	/**
	 * Generated Update vRouter interface request body from OpenStack request
	 * body
	 * 
	 * @param openStackResourceBody
	 *            - OpenStack request body for tenant
	 * @return - vRouter interface request body
	 */
	public static JsonObject getUpdateVrtIfRequestBody(
			JsonObject openStackResourceBody) {
		final JsonObject root = new JsonObject();
		final JsonObject interfaceJson = new JsonObject();
		setIpAndMacAddress(openStackResourceBody, interfaceJson);

		// special case handling for IP address
		if (openStackResourceBody.has(VtnServiceOpenStackConsts.IP_ADDRESS)
				&& VtnServiceOpenStackConsts.DEFAULT_CIDR_IP
						.equals(openStackResourceBody.get(
								VtnServiceOpenStackConsts.IP_ADDRESS)
								.getAsString())) {
			interfaceJson.addProperty(VtnServiceJsonConsts.IPADDR,
					VtnServiceConsts.EMPTY_STRING);
			interfaceJson.addProperty(VtnServiceJsonConsts.PREFIX,
					VtnServiceConsts.EMPTY_STRING);
		}

		// special case handling for MAC address
		if (openStackResourceBody.has(VtnServiceOpenStackConsts.MAC_ADDRESS)
				&& VtnServiceOpenStackConsts.DEFAULT_MAC
						.equals(openStackResourceBody.get(
								VtnServiceOpenStackConsts.MAC_ADDRESS)
								.getAsString())) {
			interfaceJson.addProperty(VtnServiceJsonConsts.MACADDR,
					VtnServiceConsts.EMPTY_STRING);
		}

		root.add(VtnServiceJsonConsts.INTERFACE, interfaceJson);
		return root;
	}

	/**
	 * Set IP address and MAC address in interface JSON request body after
	 * conversion
	 * 
	 * @param openStackResourceBody
	 * @param interfaceJson
	 */
	private static void setIpAndMacAddress(JsonObject openStackResourceBody,
			JsonObject interfaceJson) {
		if (openStackResourceBody.has(VtnServiceOpenStackConsts.IP_ADDRESS)) {
			final String[] ipAddress = openStackResourceBody
					.get(VtnServiceOpenStackConsts.IP_ADDRESS).getAsString()
					.split(VtnServiceConsts.SLASH);
			interfaceJson.addProperty(VtnServiceJsonConsts.IPADDR,
					ipAddress[VtnServiceJsonConsts.VAL_0]);
			interfaceJson.addProperty(VtnServiceJsonConsts.PREFIX,
					ipAddress[VtnServiceJsonConsts.VAL_1]);
		}

		if (openStackResourceBody.has(VtnServiceOpenStackConsts.MAC_ADDRESS)) {
			String macAddress = openStackResourceBody.get(
					VtnServiceOpenStackConsts.MAC_ADDRESS).getAsString();
			macAddress = macAddress.replaceAll(VtnServiceConsts.COLON,
					VtnServiceConsts.EMPTY_STRING);

			final String macFirstPart = macAddress.substring(
					VtnServiceJsonConsts.VAL_0, VtnServiceJsonConsts.VAL_4);
			final String macSeconfPart = macAddress.substring(
					VtnServiceJsonConsts.VAL_4, VtnServiceJsonConsts.VAL_8);
			final String macThirdPart = macAddress.substring(
					VtnServiceJsonConsts.VAL_8, VtnServiceJsonConsts.VAL_12);

			interfaceJson.addProperty(VtnServiceJsonConsts.MACADDR,
					macFirstPart + VtnServiceConsts.DOT + macSeconfPart
							+ VtnServiceConsts.DOT + macThirdPart);
		}
	}
}
