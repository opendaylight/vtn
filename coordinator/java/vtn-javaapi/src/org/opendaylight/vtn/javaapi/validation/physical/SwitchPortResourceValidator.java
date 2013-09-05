/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.validation.physical;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.resources.physical.SwitchPortResource;
import org.opendaylight.vtn.javaapi.resources.physical.SwitchPortsResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class SwitchPortResourceValidator validates request Json object for Port
 * API.
 */
public class SwitchPortResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(SwitchPortResourceValidator.class.getName());

	private final AbstractResource resource;
	final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new switch port resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public SwitchPortResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for Port API
	 * 
	 * @return true, if successful
	 */
	@Override
	public boolean validateUri() {
		LOG.trace("Start SwitchPortResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.CONTROLLERID);
		if (resource instanceof SwitchPortResource
				&& ((SwitchPortResource) resource).getControllerId() != null
				&& !((SwitchPortResource) resource).getControllerId().trim()
						.isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((SwitchPortResource) resource).getControllerId().trim(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.SWITCHID);
				if (((SwitchPortResource) resource).getSwitchId() != null
						&& !((SwitchPortResource) resource).getSwitchId()
								.trim().isEmpty()) {
					isValid = validator.isValidMaxLength(
							((SwitchPortResource) resource).getSwitchId()
									.trim(), VtnServiceJsonConsts.LEN_255);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.PORTNAME);
				if (((SwitchPortResource) resource).getPortName() != null
						&& !((SwitchPortResource) resource).getPortName()
								.trim().isEmpty()) {
					isValid = validator.isValidMaxLength(
							((SwitchPortResource) resource).getPortName()
									.trim(), VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		} else if (resource instanceof SwitchPortsResource
				&& ((SwitchPortsResource) resource).getControllerId() != null
				&& !((SwitchPortsResource) resource).getControllerId().trim()
						.isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((SwitchPortsResource) resource).getControllerId().trim(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.SWITCHID);
				if (((SwitchPortsResource) resource).getSwitchId() != null) {
					isValid = validator.isValidMaxLength(
							((SwitchPortsResource) resource).getSwitchId(),
							VtnServiceJsonConsts.LEN_255);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(true);
		}
		LOG.trace("Complete SwitchPortResourceValidator#validateUri()");
		return isValid;
	}

	/*
	 * Validate get request json for Port API
	 */
	@Override
	public void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start SwitchPortResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of SwitchPortResourceValidator");
		boolean isValid = false;
		try {
			isValid = validateUri();
			if (isValid && requestBody != null
					&& VtnServiceConsts.GET.equals(method)) {
				isValid = validateGet(requestBody, isListOpFlag());
				updateOpParameterForList(requestBody);
			} else if (isValid) {
				setInvalidParameter(VtnServiceConsts.INCORRECT_METHOD_INVOCATION);
				isValid = false;
			}
		} catch (final NumberFormatException e) {
			LOG.error("Inside catch:NumberFormatException");
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
		LOG.trace("Complete SwitchPortResourceValidator#validate()");
	}

	/**
	 * Validate get request json for Port API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if is valid get
	 */
	public boolean validateGet(final JsonObject requestBody,
			final boolean opFlag) {
		LOG.trace("Start SwitchPortResourceValidator#ValidGet");
		boolean isValid = true;
		// validation for key: targetdb
		setInvalidParameter(VtnServiceJsonConsts.TARGETDB);
		if (requestBody.has(VtnServiceJsonConsts.TARGETDB)
				&& requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.TARGETDB)
						.getAsString() != null
				&& !requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.TARGETDB)
						.getAsString().trim().isEmpty()) {
			isValid = VtnServiceJsonConsts.STATE.equalsIgnoreCase(requestBody
					.getAsJsonPrimitive(VtnServiceJsonConsts.TARGETDB)
					.getAsString().trim());
		} else {
			requestBody.remove(VtnServiceJsonConsts.TARGETDB);
			requestBody.addProperty(VtnServiceJsonConsts.TARGETDB,
					VtnServiceJsonConsts.STATE);
		}
		if (!opFlag) {
			if (requestBody.has(VtnServiceJsonConsts.OP)) {
				requestBody.remove(VtnServiceJsonConsts.OP);
			} else {
				LOG.debug("No need to remove");
			}
			if (requestBody.has(VtnServiceJsonConsts.INDEX)) {
				requestBody.remove(VtnServiceJsonConsts.INDEX);
			} else {
				LOG.debug("No need to remove");
			}
			if (requestBody.has(VtnServiceJsonConsts.MAX)) {
				requestBody.remove(VtnServiceJsonConsts.MAX);
			} else {
				LOG.debug("No need to remove");
			}
		} else {
			// validation for key: op
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.OP);
				isValid = validator.isValidOperation(requestBody);
			}
			// validation for key: index
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.INDEX);
				if (requestBody.has(VtnServiceJsonConsts.INDEX)
						&& requestBody.getAsJsonPrimitive(
								VtnServiceJsonConsts.INDEX).getAsString() != null
						&& !requestBody
								.getAsJsonPrimitive(VtnServiceJsonConsts.INDEX)
								.getAsString().isEmpty()) {
					isValid = validator.isValidMaxLength(requestBody
							.getAsJsonPrimitive(VtnServiceJsonConsts.INDEX)
							.getAsString().trim(), VtnServiceJsonConsts.LEN_31);
				}
			}
			// validation for key: max_repitition
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.MAX);
				isValid = validator.isValidMaxRepetition(requestBody);
				/*
				 * if(requestBody.has(VtnServiceJsonConsts.MAX) &&
				 * requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.MAX)
				 * .getAsString() != null) { isValid =
				 * validator.isValidMaxLengthNumber(requestBody
				 * .getAsJsonPrimitive(VtnServiceJsonConsts.MAX).getAsString()
				 * .trim()); }
				 */
			}
		}
		LOG.trace("Complete SwitchResourceValidator#isValidGet");
		return isValid;
	}
}
