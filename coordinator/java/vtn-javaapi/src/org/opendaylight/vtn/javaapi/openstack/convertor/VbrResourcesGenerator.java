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
 * Provides Request-Response Conversion methods for Network and Port Requests
 */
public class VbrResourcesGenerator {

	/**
	 * Generated Create vBridge request body from OpenStack request body
	 * 
	 * @param openStackResourceBody
	 *            - OpenStack request body for tenant
	 * @return - vBridge request body
	 */
	public static JsonObject getCreateVbrRequestBody(
			JsonObject openStackResourceBody) {
		final JsonObject root = new JsonObject();
		final JsonObject vbr = new JsonObject();

		vbr.addProperty(VtnServiceJsonConsts.VBRNAME, openStackResourceBody
				.get(VtnServiceOpenStackConsts.ID).getAsString());

		if (openStackResourceBody.has(VtnServiceOpenStackConsts.DESCRIPTION)
				&& !openStackResourceBody.get(
						VtnServiceOpenStackConsts.DESCRIPTION).isJsonNull()) {
			vbr.addProperty(
					VtnServiceJsonConsts.DESCRIPTION,
					openStackResourceBody.get(
							VtnServiceOpenStackConsts.DESCRIPTION)
							.getAsString());

		}

		vbr.addProperty(VtnServiceJsonConsts.CONTROLLERID,
				openStackResourceBody.get(VtnServiceJsonConsts.CONTROLLERID)
						.getAsString());

		vbr.addProperty(VtnServiceJsonConsts.DOMAINID,
				VtnServiceJsonConsts.DEFAULT_DOMAIN_ID);

		root.add(VtnServiceJsonConsts.VBRIDGE, vbr);
		return root;
	}

	/**
	 * Generated Update vBridge request body from OpenStack request body
	 * 
	 * @param openStackResourceBody
	 *            - OpenStack request body for tenant
	 * @return - vBridge request body
	 */
	public static JsonObject getUpdateVbrRequestBody(
			JsonObject openStackResourceBody) {
		final JsonObject root = new JsonObject();
		final JsonObject vbr = new JsonObject();

		if (!openStackResourceBody.has(VtnServiceOpenStackConsts.DESCRIPTION)
				|| openStackResourceBody.get(
						VtnServiceOpenStackConsts.DESCRIPTION).isJsonNull()
				|| openStackResourceBody
						.get(VtnServiceOpenStackConsts.DESCRIPTION)
						.getAsString().isEmpty()) {
			vbr.addProperty(VtnServiceJsonConsts.DESCRIPTION,
					VtnServiceConsts.EMPTY_STRING);
		} else {
			vbr.addProperty(
					VtnServiceJsonConsts.DESCRIPTION,
					openStackResourceBody.get(
							VtnServiceOpenStackConsts.DESCRIPTION)
							.getAsString());

		}

		root.add(VtnServiceJsonConsts.VBRIDGE, vbr);
		return root;
	}

	/**
	 * Generated Create vBridge interface request body from OpenStack request
	 * body
	 * 
	 * @param openStackResourceBody
	 *            - OpenStack request body for tenant
	 * @return - vBridge interface request body
	 */
	public static JsonObject getCreateVbrIfRequestBody(
			JsonObject openStackResourceBody) {
		final JsonObject root = new JsonObject();
		final JsonObject interfaceJson = new JsonObject();

		interfaceJson.addProperty(VtnServiceJsonConsts.IFNAME,
				openStackResourceBody.get(VtnServiceOpenStackConsts.ID)
						.getAsString());

		root.add(VtnServiceJsonConsts.INTERFACE, interfaceJson);
		return root;
	}
}
