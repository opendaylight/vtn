/*
 * Copyright (c) 2012-2014 NEC Corporation
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
import org.opendaylight.vtn.javaapi.resources.logical.HostAddressResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class HostAddressResourceValidator validates request Json object for
 * HostAddress API.
 */
public class HostAddressResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(HostAddressResourceValidator.class.getName());

	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new host address resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public HostAddressResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for HostAddress API.
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start HostAddressResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.VTNNAME);
		if (resource instanceof HostAddressResource
				&& ((HostAddressResource) resource).getVtnName() != null
				&& !((HostAddressResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((HostAddressResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VBRNAME);
				if (((HostAddressResource) resource).getVbrName() != null
						&& !((HostAddressResource) resource).getVbrName()
								.isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((HostAddressResource) resource).getVbrName(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		}
		LOG.trace("Complete HostAddressResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request Json object for get, put and post method of HostAddress
	 * API.
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start HostAddressResourceValidator#validate()");
		boolean isValid = false;
		try {
			isValid = validateUri();
			LOG.info("Validating request for " + method
					+ " of HostAddressResourceValidator");
			if (isValid && requestBody != null
					&& VtnServiceConsts.GET.equals(method)) {
				isValid = validator.isValidGet(requestBody, isListOpFlag());
				setInvalidParameter(validator.getInvalidParameter());
				updateOpParameterForList(requestBody);
			} else if (isValid && requestBody != null
					&& VtnServiceConsts.PUT.equals(method)) {
				isValid = validatePut(requestBody);
			} else if (isValid) {
				setInvalidParameter(VtnServiceConsts.INCORRECT_METHOD_INVOCATION);
				isValid = false;
			}
		} catch (final NumberFormatException e) {
			if (method.equals(VtnServiceConsts.GET)) {
				setInvalidParameter(validator.getInvalidParameter());
			}
			LOG.error(e, "Inside catch:NumberFormatException");
			isValid = false;
		} catch (final ClassCastException e) {
			if (method.equals(VtnServiceConsts.GET)) {
				setInvalidParameter(validator.getInvalidParameter());
			}
			LOG.error(e, "Inside catch:ClassCastException");
			isValid = false;
		}
		// Throws exception if validation fails
		if (!isValid) {
			LOG.error("Validation failed");
			throw new VtnServiceException(Thread.currentThread()
					.getStackTrace()[1].getMethodName(),
					UncJavaAPIErrorCode.VALIDATION_ERROR.getErrorCode(),
					UncJavaAPIErrorCode.VALIDATION_ERROR.getErrorMessage());
		}
		LOG.info("Validation successful");
		LOG.trace("Complete HostAddressResourceValidator#validate()");
	}

	/**
	 * Validate put request Json object for HostAddressResource
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validatePut(final JsonObject requestBody) {
		LOG.trace("Start HostAddressResourceValidator#validatePut()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.IPADDRESS);
		if (requestBody.has(VtnServiceJsonConsts.IPADDRESS)
				&& requestBody.get(VtnServiceJsonConsts.IPADDRESS)
						.isJsonObject()) {
			final JsonObject ipaddr = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.IPADDRESS);
			// validation for mandatory keys: ipaddr
			setInvalidParameter(VtnServiceJsonConsts.IPADDR);
			if (ipaddr.has(VtnServiceJsonConsts.IPADDR)
					&& ipaddr.getAsJsonPrimitive(VtnServiceJsonConsts.IPADDR)
							.getAsString() != null) {
				isValid = validator.isValidIpV4(ipaddr.getAsJsonPrimitive(
						VtnServiceJsonConsts.IPADDR).getAsString());
			}
			if (isValid) {
				// validation for mandatory keys: netmask
				setInvalidParameter(VtnServiceJsonConsts.PREFIX);
				if (ipaddr.has(VtnServiceJsonConsts.PREFIX)
						&& ipaddr.getAsJsonPrimitive(
								VtnServiceJsonConsts.PREFIX).getAsString() != null) {
					isValid = validator.isValidRange(
							ipaddr.getAsJsonPrimitive(
									VtnServiceJsonConsts.PREFIX).getAsString(),
							VtnServiceJsonConsts.VAL_1,
							VtnServiceJsonConsts.VAL_30);
				} else {
					isValid = false;
				}
			}
		}
		LOG.trace("Complete HostAddressResourceValidator#validatePut()");
		return isValid;
	}
}
