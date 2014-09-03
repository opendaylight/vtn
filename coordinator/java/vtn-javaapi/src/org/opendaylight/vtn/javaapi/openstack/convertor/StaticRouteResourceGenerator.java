/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi.openstack.convertor;

import org.apache.commons.net.util.SubnetUtils;

import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;

/**
 * Provides Request-Response Conversion methods for Route requests Requests
 */
public class StaticRouteResourceGenerator {

	/**
	 * Generated Create static-ip-route request body from OpenStack request body
	 * 
	 * @param openStackResourceBody
	 *            - OpenStack request body for tenant
	 * @return - static-ip-route request body
	 */
	public static JsonObject getCreateStaticRouteRequestBody(
			JsonObject openStackResourceBody) {
		final JsonObject root = new JsonObject();
		final JsonObject staticRoute = new JsonObject();

		final String[] destination = openStackResourceBody
				.get(VtnServiceOpenStackConsts.DESTNATION).getAsString()
				.split(VtnServiceConsts.SLASH);

		staticRoute.addProperty(VtnServiceJsonConsts.IPADDR,
				destination[VtnServiceJsonConsts.VAL_0]);

		staticRoute.addProperty(VtnServiceJsonConsts.PREFIX,
				destination[VtnServiceJsonConsts.VAL_1]);

		staticRoute.addProperty(VtnServiceJsonConsts.NEXTHOPADDR,
				openStackResourceBody.get(VtnServiceOpenStackConsts.NEXTHOP)
						.getAsString());

		root.add(VtnServiceJsonConsts.STATIC_IPROUTE, staticRoute);
		return root;
	}

	/**
	 * Generated List static-ip-route request body
	 * 
	 * @return - static-ip-route request body
	 */
	public static JsonObject getListRequestBody() {
		final JsonObject request = new JsonObject();
		request.addProperty(VtnServiceJsonConsts.TARGETDB,
				VtnServiceJsonConsts.STATE);
		return request;
	}

	/**
	 * Convert response for static-ip-route from UNC format to OpenStack format
	 * 
	 * @param responseBody
	 *            - UNC formatted response body
	 * @return - OpenStack formatted response body
	 */
	public static JsonObject convertListResponseBody(JsonObject responseBody) {
		final JsonObject openStackResponse = new JsonObject();
		final JsonArray routes = new JsonArray();
		final JsonArray staticRoutes = responseBody.get(
				VtnServiceJsonConsts.STATIC_IPROUTES).getAsJsonArray();
		for (final JsonElement staticRoute : staticRoutes) {

			final String[] staticIpRouteId = staticRoute.getAsJsonObject()
					.get(VtnServiceJsonConsts.STATICIPROUTEID).getAsString()
					.split(VtnServiceConsts.HYPHEN);

			final String destination = staticIpRouteId[0]
					+ VtnServiceConsts.SLASH + staticIpRouteId[2];
			final String nexthop = staticIpRouteId[1];

			String routeId = null;
			if (staticIpRouteId[2].equals(VtnServiceConsts.ZERO)) {
				routeId = staticIpRouteId[0] + VtnServiceConsts.HYPHEN
						+ staticIpRouteId[1] + VtnServiceConsts.HYPHEN
						+ VtnServiceConsts.DEFAULT_IP;
			} else {
				final SubnetUtils subnetUtils = new SubnetUtils(destination);
				routeId = staticIpRouteId[0] + VtnServiceConsts.HYPHEN
						+ staticIpRouteId[1] + VtnServiceConsts.HYPHEN
						+ subnetUtils.getInfo().getNetmask();
			}

			final JsonObject route = new JsonObject();
			route.addProperty(VtnServiceOpenStackConsts.ID, routeId);
			route.addProperty(VtnServiceOpenStackConsts.DESTNATION, destination);
			route.addProperty(VtnServiceOpenStackConsts.NEXTHOP, nexthop);
			routes.add(route);
		}
		openStackResponse.add(VtnServiceOpenStackConsts.ROUTES, routes);
		return openStackResponse;
	}
}
