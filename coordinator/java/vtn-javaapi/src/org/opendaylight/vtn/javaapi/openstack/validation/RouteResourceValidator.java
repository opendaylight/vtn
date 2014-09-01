/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.openstack.validation;

import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpAddressUtil;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.resources.openstack.RouteResource;
import org.opendaylight.vtn.javaapi.resources.openstack.RoutesResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * Route Validation Resource class. Contains methods to validate URI, parameters
 * of POST request body
 * 
 */
public class RouteResourceValidator extends VtnServiceValidator {

	/* Logger instance */
	private static final Logger LOG = Logger
			.getLogger(RouteResourceValidator.class.getName());

	/* AbstractResource reference pointing to actual Resource class */
	private final AbstractResource resource;

	/* instance for common validation operation */
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Constructor that provide reference of actual Resource class to instance
	 * variable resource
	 * 
	 * @param resource
	 *            - Resource class reference
	 */
	public RouteResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Calls the respective validation method according to type of method
	 * 
	 * @see org.opendaylight.vtn.javaapi.validation.VtnServiceValidator#validate
	 *      (java.lang.String, com.google.gson.JsonObject)
	 */
	@Override
	public void validate(String method, JsonObject requestBody)
			throws VtnServiceException {
		LOG.info("Start RouteResourceValidator#validate()");
		boolean isValid = false;
		try {
			if (requestBody != null) {
				isValid = validateUri();
				if (isValid && VtnServiceConsts.POST.equalsIgnoreCase(method)) {
					isValid = validatePost(requestBody);
				} else if (isValid
						&& VtnServiceConsts.GET.equalsIgnoreCase(method)) {
					isValid = true;
				} else if (isValid) {
					setInvalidParameter(UncCommonEnum.UncResultCode.UNC_METHOD_NOT_ALLOWED
							.getMessage());
					isValid = false;
				}
			} else {
				setInvalidParameter(UncCommonEnum.UncResultCode.UNC_INVALID_FORMAT
						.getMessage());
			}
		} catch (final NumberFormatException e) {
			LOG.error(e, "Invalid value : " + e.getMessage());
			isValid = false;
		} catch (final ClassCastException e) {
			LOG.error(e, "Invalid type : " + e.getMessage());
			isValid = false;
		}

		/*
		 * throw exception in case of validation fail
		 */
		if (!isValid) {
			LOG.error("Validation failure");
			throw new VtnServiceException(Thread.currentThread()
					.getStackTrace()[1].getMethodName(),
					UncJavaAPIErrorCode.VALIDATION_ERROR.getErrorCode(),
					UncJavaAPIErrorCode.VALIDATION_ERROR.getErrorMessage());
		}
		LOG.info("Complete RouteResourceValidator#validate()");
	}

	/**
	 * Validate resource specific URI parameters
	 * 
	 * @see org.opendaylight.vtn.javaapi.validation.VtnServiceValidator#validateUri
	 *      ()
	 */
	@Override
	public boolean validateUri() throws VtnServiceException {
		LOG.info("Start RouteResourceValidator#validateUri()");
		boolean isValid = true;
		if (resource instanceof RouteResource) {
			// validation of tenant_id
			final String tenantId = ((RouteResource) resource).getTenantId();
			if (tenantId == null
					|| tenantId.isEmpty()
					|| !validator.isValidMaxLengthAlphaNum(tenantId,
							VtnServiceJsonConsts.LEN_31)) {
				isValid = false;
				setInvalidParameter(VtnServiceOpenStackConsts.TENANT_ID
						+ VtnServiceConsts.COLON + tenantId);
			}

			// validation of router_id
			if (isValid) {
				final String routerId = ((RouteResource) resource)
						.getRouterId();
				if (routerId == null
						|| routerId.isEmpty()
						|| !validator.isValidMaxLengthAlphaNum(routerId,
								VtnServiceJsonConsts.LEN_31)) {
					isValid = false;
					setInvalidParameter(VtnServiceOpenStackConsts.ROUTER_ID
							+ VtnServiceConsts.COLON + routerId);
				}
			}

			// validation of route_id
			if (isValid) {
				final String routeId = ((RouteResource) resource).getRouteId();
				setInvalidParameter(VtnServiceOpenStackConsts.ID);
				if (routeId == null || routeId.isEmpty()
						|| !isValidRouteId(routeId)) {
					isValid = false;
					setInvalidParameter(VtnServiceOpenStackConsts.ROUTE_ID
							+ VtnServiceConsts.COLON + routeId);
				}
			}
		} else if (resource instanceof RoutesResource) {
			// validation of tenant_id
			final String tenantId = ((RoutesResource) resource).getTenantId();
			if (tenantId == null
					|| tenantId.isEmpty()
					|| !validator.isValidMaxLengthAlphaNum(tenantId,
							VtnServiceJsonConsts.LEN_31)) {
				isValid = false;
				setInvalidParameter(VtnServiceOpenStackConsts.TENANT_ID
						+ VtnServiceConsts.COLON + tenantId);
			}

			// validation of router_id
			if (isValid) {
				final String routerId = ((RoutesResource) resource)
						.getRouterId();
				if (routerId == null
						|| routerId.isEmpty()
						|| !validator.isValidMaxLengthAlphaNum(routerId,
								VtnServiceJsonConsts.LEN_31)) {
					isValid = false;
					setInvalidParameter(VtnServiceOpenStackConsts.ROUTER_ID
							+ VtnServiceConsts.COLON + routerId);
				}
			}
		}
		LOG.info("Complete RouteResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validates the parameters of POST request body
	 * 
	 * @param requestBody
	 *            - JSON request body corresponding to POST operation
	 * @return - validation status as true or false
	 */
	private boolean validatePost(JsonObject requestBody) {
		LOG.trace("Start RouteResourceValidator#validatePost()");
		boolean isValid = true;

		// validation of mandatory parameters
		if (requestBody == null
				|| !requestBody.has(VtnServiceOpenStackConsts.DESTNATION)
				|| !requestBody.has(VtnServiceOpenStackConsts.NEXTHOP)) {
			isValid = false;
			setInvalidParameter(UncCommonEnum.UncResultCode.UNC_INVALID_FORMAT
					.getMessage());
		} else {
			final JsonElement destination = requestBody
					.get(VtnServiceOpenStackConsts.DESTNATION);

			// validation of destination
			if (destination.isJsonNull() || destination.getAsString().isEmpty()
					|| !isValidDestination(destination.getAsString())) {
				isValid = false;
				setInvalidParameter(VtnServiceOpenStackConsts.DESTNATION
						+ VtnServiceConsts.COLON
						+ (destination.isJsonNull() ? destination : destination
								.getAsString()));
			}

			final JsonElement nexthop = requestBody
					.get(VtnServiceOpenStackConsts.NEXTHOP);

			// validation of nexthop
			if (isValid) {
				if (nexthop.isJsonNull()
						|| nexthop.getAsString().isEmpty()
						|| IpAddressUtil.textToNumericFormatV4(nexthop
								.getAsString()) == null) {
					isValid = false;
					setInvalidParameter(VtnServiceOpenStackConsts.NEXTHOP
							+ VtnServiceConsts.COLON
							+ (nexthop.isJsonNull() ? nexthop : nexthop
									.getAsString()));
				}
			}

			/*
			 * Check special case for IP address in POST operation
			 */
			if (isValid) {
				final String nexthopIp = requestBody.get(
						VtnServiceOpenStackConsts.NEXTHOP).getAsString();
				if (VtnServiceOpenStackConsts.DEFAULT_IP.equals(nexthopIp)) {
					isValid = false;
					setInvalidParameter(VtnServiceOpenStackConsts.NEXTHOP
							+ VtnServiceConsts.COLON + nexthopIp);
				}
			}
		}
		LOG.trace("Complete RouteResourceValidator#validatePost()");
		return isValid;
	}

	/**
	 * Check the validity of destination IP address
	 * 
	 * @param cidrDestination
	 *            - CIDR formatted IP address
	 * @return - status as true or false
	 */
	private boolean isValidDestination(String cidrDestination) {
		boolean isValid = false;
		setInvalidParameter(VtnServiceOpenStackConsts.DESTNATION);
		final String[] destination = cidrDestination
				.split(VtnServiceConsts.SLASH);
		if (destination != null && destination.length == 2) {
			try {
				isValid = IpAddressUtil.textToNumericFormatV4(destination[0]) != null
						&& validator.isValidRange(destination[1], 0, 32);
			} catch (Exception e) {
				isValid = false;
			}
		}
		return isValid;
	}

	/**
	 * Check the validity of OpenStack format route_id
	 * 
	 * @param routeId
	 *            - OpenStack formatted route_id
	 * @return - result as true or false
	 */
	private boolean isValidRouteId(String routeId) {
		boolean isValid = false;
		final String[] routeIdparts = routeId.split(VtnServiceConsts.HYPHEN);
		if (routeIdparts != null && routeIdparts.length == 3) {
			isValid = IpAddressUtil.textToNumericFormatV4(routeIdparts[0]) != null
					&& IpAddressUtil.textToNumericFormatV4(routeIdparts[1]) != null
					&& IpAddressUtil.textToNumericFormatV4(routeIdparts[2]) != null;
		}
		return isValid;
	}
}
