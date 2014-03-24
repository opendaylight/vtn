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
 * Provides Request-Response Conversion methods for Tenant Requests
 */
public class VtnResourcesGenerator {

	/**
	 * Generated Create VTN request body from OpenStack request body
	 * 
	 * @param openStackResourceBody
	 *            - OpenStack request body for tenant
	 * @return - VTN request body
	 */
	public static JsonObject getCreateVtnRequestBody(
			JsonObject openStackResourceBody) {
		final JsonObject root = new JsonObject();
		final JsonObject vtn = new JsonObject();

		vtn.addProperty(VtnServiceJsonConsts.VTNNAME, openStackResourceBody
				.get(VtnServiceOpenStackConsts.ID).getAsString());

		if (openStackResourceBody.has(VtnServiceOpenStackConsts.DESCRIPTION)
				&& !openStackResourceBody.get(
						VtnServiceOpenStackConsts.DESCRIPTION).isJsonNull()) {
			vtn.addProperty(
					VtnServiceJsonConsts.DESCRIPTION,
					openStackResourceBody.get(
							VtnServiceOpenStackConsts.DESCRIPTION)
							.getAsString());

		}

		root.add(VtnServiceJsonConsts.VTN, vtn);
		return root;
	}

	/**
	 * Generated Update VTN request body from OpenStack request body
	 * 
	 * @param openStackResourceBody
	 *            - OpenStack request body for tenant
	 * @return - VTN request body
	 */
	public static JsonObject getUpdateVtnRequestBody(
			JsonObject openStackResourceBody) {
		final JsonObject root = new JsonObject();
		final JsonObject vtn = new JsonObject();

		if (!openStackResourceBody.has(VtnServiceOpenStackConsts.DESCRIPTION)
				|| openStackResourceBody.get(
						VtnServiceOpenStackConsts.DESCRIPTION).isJsonNull()
				|| openStackResourceBody
						.get(VtnServiceOpenStackConsts.DESCRIPTION)
						.getAsString().isEmpty()) {
			vtn.addProperty(VtnServiceJsonConsts.DESCRIPTION,
					VtnServiceConsts.EMPTY_STRING);
		} else {
			vtn.addProperty(
					VtnServiceJsonConsts.DESCRIPTION,
					openStackResourceBody.get(
							VtnServiceOpenStackConsts.DESCRIPTION)
							.getAsString());

		}

		root.add(VtnServiceJsonConsts.VTN, vtn);
		return root;
	}
}
