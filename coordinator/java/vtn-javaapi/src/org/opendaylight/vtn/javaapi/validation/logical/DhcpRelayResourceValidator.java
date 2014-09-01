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
import org.opendaylight.vtn.javaapi.resources.logical.DhcpRelayResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class DhcpRelayResourceValidator validates request Json object for DHCP
 * Relay API.
 */
public class DhcpRelayResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(DhcpRelayResourceValidator.class.getName());

	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new DHCP relay resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public DhcpRelayResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate URI parameters for DhcpRelay API
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start DhcpRelayResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.VTNNAME);
		if (resource instanceof DhcpRelayResource
				&& ((DhcpRelayResource) resource).getVtnName() != null
				&& !((DhcpRelayResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((DhcpRelayResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VRTNAME);
				if (((DhcpRelayResource) resource).getVrtName() != null
						&& !((DhcpRelayResource) resource).getVrtName()
								.isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((DhcpRelayResource) resource).getVrtName(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		}
		LOG.trace("Complete DhcpRelayResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request Json object for get, put method of DhcpRelay API
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start DhcpRelayResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of DhcpRelayResourceValidator");
		boolean isValid = false;
		try {
			isValid = validateUri();
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
		LOG.trace("Complete DhcpRelayResourceValidator#validate()");

	}

	/**
	 * Validate put request Json object for DhcpRelay API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	private boolean validatePut(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start DhcpRelayResourceValidator#validatePut()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.DHCPRELAY);
		if (requestBody.has(VtnServiceJsonConsts.DHCPRELAY)
				&& requestBody.get(VtnServiceJsonConsts.DHCPRELAY)
						.isJsonObject()) {
			final JsonObject dhcpRelay = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.DHCPRELAY);
			// validation for key: relay_status(mandatory)
			setInvalidParameter(VtnServiceJsonConsts.DHCPRELAYSTATUS);
			if (dhcpRelay.has(VtnServiceJsonConsts.DHCPRELAYSTATUS)
					&& dhcpRelay.getAsJsonPrimitive(
							VtnServiceJsonConsts.DHCPRELAYSTATUS).getAsString() != null) {
				final String relayStatus = dhcpRelay.getAsJsonPrimitive(
						VtnServiceJsonConsts.DHCPRELAYSTATUS).getAsString();
				isValid = relayStatus
						.equalsIgnoreCase(VtnServiceJsonConsts.ENABLE)
						|| relayStatus
								.equalsIgnoreCase(VtnServiceJsonConsts.DISABLE);
			}
		}
		LOG.trace("Complete DhcpRelayResourceValidator#validatePut()");
		return isValid;
	}
}
