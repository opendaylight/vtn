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
 * Provides Request-Response Conversion methods for Vlan-map and Port-map
 * requests
 */
public class MapResourceGenerator {

	/**
	 * Generated Create vlan-map request body from OpenStack request body
	 * 
	 * @param openStackResourceBody
	 *            - OpenStack request body for tenant
	 * @return - vlan-map request body
	 */
	public static JsonObject getCreateVlanMapRequestBody(
			JsonObject openStackResourceBody) {
		final JsonObject root = new JsonObject();
		final JsonObject vlanmap = new JsonObject();

		String datapathId = openStackResourceBody.get(
				VtnServiceOpenStackConsts.DATAPATH_ID).getAsString();

		if (!datapathId.isEmpty()) {
			datapathId = datapathId.substring(VtnServiceJsonConsts.VAL_2,
					datapathId.length());
			final StringBuilder sb = new StringBuilder();
			for (int toPrepend = VtnServiceJsonConsts.VAL_16
					- datapathId.length(); toPrepend > VtnServiceJsonConsts.VAL_0; toPrepend--) {
				sb.append('0');
			}
			datapathId = sb.append(datapathId).toString();
			final String logicalPortIdPartFirst = datapathId.substring(
					VtnServiceJsonConsts.VAL_0, VtnServiceJsonConsts.VAL_4);
			final String logicalPortIdPartSecond = datapathId.substring(
					VtnServiceJsonConsts.VAL_4, VtnServiceJsonConsts.VAL_8);
			final String logicalPortIdPartThird = datapathId.substring(
					VtnServiceJsonConsts.VAL_8, VtnServiceJsonConsts.VAL_12);
			final String logicalPortIdPartFour = datapathId.substring(
					VtnServiceJsonConsts.VAL_12, VtnServiceJsonConsts.VAL_16);

			vlanmap.addProperty(VtnServiceJsonConsts.LOGICAL_PORT_ID,
					VtnServiceOpenStackConsts.SW + VtnServiceConsts.HYPHEN
							+ logicalPortIdPartFirst + VtnServiceConsts.HYPHEN
							+ logicalPortIdPartSecond + VtnServiceConsts.HYPHEN
							+ logicalPortIdPartThird + VtnServiceConsts.HYPHEN
							+ logicalPortIdPartFour);
		}

		final int vlanId = openStackResourceBody.get(
				VtnServiceOpenStackConsts.VID).getAsInt();
		if (vlanId >= VtnServiceJsonConsts.VAL_1
				&& vlanId <= VtnServiceJsonConsts.VAL_4095) {
			vlanmap.addProperty(VtnServiceJsonConsts.VLANID, vlanId);
		} else {
			vlanmap.addProperty(VtnServiceJsonConsts.NO_VLAN_ID,
					VtnServiceJsonConsts.TRUE);
		}

		root.add(VtnServiceJsonConsts.VLANMAP, vlanmap);
		return root;
	}

	/**
	 * Generated Create port-map request body from OpenStack request body
	 * 
	 * @param openStackResourceBody
	 *            - OpenStack request body for tenant
	 * @return - port-map request body
	 */
	public static JsonObject getCreatePortMapRequestBody(
			JsonObject openStackResourceBody) {
		final JsonObject root = new JsonObject();
		final JsonObject portmap = new JsonObject();

		String datapathId = openStackResourceBody.get(
				VtnServiceOpenStackConsts.DATAPATH_ID).getAsString();

		datapathId = datapathId.substring(VtnServiceJsonConsts.VAL_2,
				datapathId.length());
		final StringBuilder sb = new StringBuilder();
		for (int toPrepend = VtnServiceJsonConsts.VAL_16 - datapathId.length(); 
				toPrepend > VtnServiceJsonConsts.VAL_0; toPrepend--) {
			sb.append('0');
		}
		datapathId = sb.append(datapathId).toString();
		final String logicalPortIdPartFirst = datapathId.substring(
				VtnServiceJsonConsts.VAL_0, VtnServiceJsonConsts.VAL_4);
		final String logicalPortIdPartSecond = datapathId.substring(
				VtnServiceJsonConsts.VAL_4, VtnServiceJsonConsts.VAL_8);
		final String logicalPortIdPartThird = datapathId.substring(
				VtnServiceJsonConsts.VAL_8, VtnServiceJsonConsts.VAL_12);
		final String logicalPortIdPartFour = datapathId.substring(
				VtnServiceJsonConsts.VAL_12, VtnServiceJsonConsts.VAL_16);

		portmap.addProperty(
				VtnServiceJsonConsts.LOGICAL_PORT_ID,
				VtnServiceOpenStackConsts.PP
						+ VtnServiceConsts.HYPHEN
						+ logicalPortIdPartFirst
						+ VtnServiceConsts.HYPHEN
						+ logicalPortIdPartSecond
						+ VtnServiceConsts.HYPHEN
						+ logicalPortIdPartThird
						+ VtnServiceConsts.HYPHEN
						+ logicalPortIdPartFour
						+ VtnServiceConsts.HYPHEN
						+ openStackResourceBody.get(
								VtnServiceJsonConsts.PORTNAME).getAsString());

		final int vlanId = openStackResourceBody.get(
				VtnServiceOpenStackConsts.VID).getAsInt();
		if (vlanId >= VtnServiceJsonConsts.VAL_1
				&& vlanId <= VtnServiceJsonConsts.VAL_4095) {
			portmap.addProperty(VtnServiceJsonConsts.VLANID, vlanId);
			portmap.addProperty(VtnServiceJsonConsts.TAGGED,
					VtnServiceJsonConsts.FALSE);
		}

		root.add(VtnServiceJsonConsts.PORTMAP, portmap);
		return root;
	}

	/**
	 * Generated Get vlan-map request body
	 * 
	 * @return - vlan-map request body
	 */
	public static JsonObject getVLanMapCountRequestBody() {
		final JsonObject request = new JsonObject();
		request.addProperty(VtnServiceJsonConsts.TARGETDB,
				VtnServiceJsonConsts.RUNNING);
		request.addProperty(VtnServiceJsonConsts.OP, VtnServiceJsonConsts.COUNT);
		return request;
	}

	/**
	 * Generated Get port request body from OpenStack request body
	 * 
	 * @param openStackResourceBody
	 *            - OpenStack request body for tenant
	 * @return - port request body
	 */
	public static JsonObject getPortNameRequestBody(
			JsonObject openStackResourceBody) {
		final JsonObject request = new JsonObject();
		request.addProperty(VtnServiceJsonConsts.TARGETDB,
				VtnServiceJsonConsts.STATE);
		request.addProperty(VtnServiceJsonConsts.OP,
				VtnServiceJsonConsts.DETAIL);
		request.addProperty(VtnServiceJsonConsts.PORT_ID, openStackResourceBody
				.get(VtnServiceOpenStackConsts.PORT).getAsString());
		return request;
	}

}
