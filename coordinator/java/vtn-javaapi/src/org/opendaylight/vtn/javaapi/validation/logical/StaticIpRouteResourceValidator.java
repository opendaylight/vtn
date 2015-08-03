/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.validation.logical;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.resources.logical.StaticIpRouteResource;
import org.opendaylight.vtn.javaapi.resources.logical.StaticIpRoutesResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class StaticIpRouteResourceValidator validates request Json object for
 * StaticIpRoute API.
 */
public class StaticIpRouteResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(StaticIpRouteResourceValidator.class.getName());
	private transient AbstractResource resource;
	private transient CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new static ip route resource validator.
	 * 
	 * @param resource
	 *            the resource
	 */
	public StaticIpRouteResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for static ip route API
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start StaticIpRouteResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.VTNNAME);
		if (resource instanceof StaticIpRoutesResource
				&& ((StaticIpRoutesResource) resource).getVtnName() != null
				&& !((StaticIpRoutesResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((StaticIpRoutesResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VRTNAME);
				if (((StaticIpRoutesResource) resource).getVrtName() != null
						&& !((StaticIpRoutesResource) resource).getVrtName()
								.isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((StaticIpRoutesResource) resource).getVrtName(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(true);
		} else if (resource instanceof StaticIpRouteResource
				&& ((StaticIpRouteResource) resource).getVtnName() != null
				&& !((StaticIpRouteResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((StaticIpRouteResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VRTNAME);
				if (((StaticIpRouteResource) resource).getVrtName() != null
						&& !((StaticIpRouteResource) resource).getVrtName()
								.isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((StaticIpRouteResource) resource).getVrtName(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.STATICIPROUTEID);
				if (((StaticIpRouteResource) resource).getStaticIpRouteId() != null
						&& !((StaticIpRouteResource) resource)
								.getStaticIpRouteId().isEmpty()) {
					final String[] staticIpRoute = ((StaticIpRouteResource) resource)
							.getStaticIpRouteId().split(
									VtnServiceJsonConsts.HYPHEN);
					isValid = isValidStaticIpRouteId(staticIpRoute);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		}
		LOG.trace("Complete StaticIpRouteResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * @param staticIpRoute
	 * @return
	 */
	private boolean isValidStaticIpRouteId(final String[] staticIpRoute) {
		LOG.trace("Start StaticIpRouteResourceValidator#isValidStaticIpRouteId()");
		boolean isValid = false;
		isValid = validator.isValidIpV4(staticIpRoute[0])
				&& validator.isValidIpV4(staticIpRoute[1])
				&& validator
						.isValidRange(staticIpRoute[2],
								VtnServiceJsonConsts.VAL_0,
								VtnServiceJsonConsts.VAL_32)
				&& (staticIpRoute.length > 3 ? validator
						.isValidMaxLengthAlphaNum(staticIpRoute[3],
								VtnServiceJsonConsts.LEN_31) : true);
		LOG.trace("Complete StaticIpRouteResourceValidator#isValidStaticIpRouteId()");
		return isValid;
	}

	/**
	 * Validate request Json object for get, put and post method of
	 * StaticIpRoute API.
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start StaticIpRouteResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of StaticIpRouteResourceValidator");
		boolean isValid = false;
		try {
			isValid = validateUri();
			if (isValid && requestBody != null
					&& VtnServiceConsts.GET.equals(method)) {
				isValid = validateGet(requestBody, isListOpFlag());
				updateOpParameterForList(requestBody);
			} else if (isValid && requestBody != null
					&& VtnServiceConsts.PUT.equals(method)) {
				isValid = validatePut(requestBody);
			} else if (isValid && requestBody != null
					&& VtnServiceConsts.POST.equals(method)) {
				isValid = validatePost(requestBody);
			} else if (isValid) {
				setInvalidParameter(VtnServiceConsts.INCORRECT_METHOD_INVOCATION);
				isValid = false;
			}
		} catch (final NumberFormatException e) {
			LOG.error(e, "Inside catch:NumberFormatException");
			isValid = false;
		}
		// Throws exception if validation fails
		if (!isValid) {
			throw new VtnServiceException(Thread.currentThread()
					.getStackTrace()[1].getMethodName(),
					UncJavaAPIErrorCode.VALIDATION_ERROR.getErrorCode(),
					UncJavaAPIErrorCode.VALIDATION_ERROR.getErrorMessage());
		}
	}

	/**
	 * Validate get request Json object for StaticIpRoute API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @param opFlag
	 *            the op flag
	 * @return true, if successful
	 */
	private boolean validateGet(final JsonObject requestBody,
			final boolean opFlag) {
		LOG.trace("Start StaticIpRouteResourceValidator#validateGet()");
		boolean isValid = true;
		setInvalidParameter(VtnServiceJsonConsts.TARGETDB);
		if (isValid) {
			isValid = validator.isValidRequestDB(requestBody);
		}
		if (!opFlag) {
			if (requestBody.has(VtnServiceJsonConsts.OP)) {
				requestBody.remove(VtnServiceJsonConsts.OP);
			} else {
				LOG.debug("No need to remove -op ");
			}
			if (requestBody.has(VtnServiceJsonConsts.INDEX)) {
				requestBody.remove(VtnServiceJsonConsts.INDEX);
			} else {
				LOG.debug("No need to remove - index");
			}
			if (requestBody.has(VtnServiceJsonConsts.MAX)) {
				requestBody.remove(VtnServiceJsonConsts.MAX);
			} else {
				LOG.debug("No need to remove-max count");
			}
		} else {
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.OP);
				isValid = validator.isValidOperation(requestBody);
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.INDEX);
				if (requestBody.has(VtnServiceJsonConsts.INDEX)
						&& requestBody.getAsJsonPrimitive(
								VtnServiceJsonConsts.INDEX).getAsString() != null
						&& !requestBody
								.getAsJsonPrimitive(VtnServiceJsonConsts.INDEX)
								.getAsString().isEmpty()) {
					final String index = requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.INDEX).getAsString();
					final String[] staticIpRoute = index
							.split(VtnServiceJsonConsts.HYPHEN);
					isValid = isValidStaticIpRouteId(staticIpRoute);
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.MAX);
				isValid = validator.isValidMaxRepetition(requestBody);
			}
		}
		LOG.trace("Complete StaticIpRouteResourceValidator#validateGet()");
		return isValid;

	}

	/**
	 * Validate post request Json object for StaticIpRoute API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	private boolean validatePost(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start StaticIpRouteResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.STATIC_IPROUTE);
		if (requestBody.has(VtnServiceJsonConsts.STATIC_IPROUTE)
				&& requestBody.get(VtnServiceJsonConsts.STATIC_IPROUTE)
						.isJsonObject()) {
			final JsonObject staticIp = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.STATIC_IPROUTE);
			// validation for key: ipaddr(mandatory)
			setInvalidParameter(VtnServiceJsonConsts.IPADDR);
			if (staticIp.has(VtnServiceJsonConsts.IPADDR)
					&& staticIp.getAsJsonPrimitive(VtnServiceJsonConsts.IPADDR)
							.getAsString() != null) {
				isValid = validator.isValidIpV4(staticIp.getAsJsonPrimitive(
						VtnServiceJsonConsts.IPADDR).getAsString());
			}

			// validation for key: prefix(mandatory)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.PREFIX);
				if (staticIp.has(VtnServiceJsonConsts.PREFIX)
						&& staticIp.getAsJsonPrimitive(
								VtnServiceJsonConsts.PREFIX).getAsString() != null) {
					isValid = validator.isValidRange(staticIp
							.getAsJsonPrimitive(VtnServiceJsonConsts.PREFIX)
							.getAsString(), VtnServiceJsonConsts.VAL_0,
							VtnServiceJsonConsts.VAL_32);
				} else {
					isValid = false;
				}
			}
			// validation for key: nexthopaddress (mandatory)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.NEXTHOPADDR);
				if (staticIp.has(VtnServiceJsonConsts.NEXTHOPADDR)
						&& staticIp.getAsJsonPrimitive(
								VtnServiceJsonConsts.NEXTHOPADDR).getAsString() != null) {
					isValid = validator.isValidIpV4(staticIp.getAsJsonPrimitive(
							VtnServiceJsonConsts.NEXTHOPADDR).getAsString());
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				isValid = validatePut(requestBody);
			}

		}
		LOG.trace("Complete StaticIpRouteResourceValidator#validatePost()");
		return isValid;
	}

	/**
	 * Validate put request Json object for StaticIpRoute API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	private boolean validatePut(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start StaticIpRouteResourceValidator#validatePut()");
		boolean isValid = false;

		setInvalidParameter(VtnServiceJsonConsts.STATIC_IPROUTE);

		if (requestBody.has(VtnServiceJsonConsts.STATIC_IPROUTE)
				&& requestBody.get(VtnServiceJsonConsts.STATIC_IPROUTE)
						.isJsonObject()) {
			final JsonObject staticIp = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.STATIC_IPROUTE);

			isValid = true;
			//validation for key: nmg_name(option)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.NMGNAME);
				if (staticIp.has(VtnServiceJsonConsts.NMGNAME)
						&& staticIp.getAsJsonPrimitive(
								VtnServiceJsonConsts.NMGNAME).getAsString() != null) {
					isValid = validator.isValidMaxLengthAlphaNum(staticIp
									.getAsJsonPrimitive(VtnServiceJsonConsts.NMGNAME)
									.getAsString(), VtnServiceJsonConsts.LEN_31);
				}
			}
			//validation for key: groupmetric(option)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.GROUPMETRIC);
				if (staticIp.has(VtnServiceJsonConsts.GROUPMETRIC)
						&& staticIp.getAsJsonPrimitive(
								VtnServiceJsonConsts.GROUPMETRIC).getAsString() != null) {
					isValid = validator.isValidRange(staticIp.getAsJsonPrimitive(
									VtnServiceJsonConsts.GROUPMETRIC).getAsString(),
									VtnServiceJsonConsts.VAL_1,
									VtnServiceJsonConsts.VAL_65535);
				}
			}
		}
		LOG.trace("Complete StaticIpRouteResourceValidator#validatePut()");
		return isValid;
	}
}
