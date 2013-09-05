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
import org.opendaylight.vtn.javaapi.resources.physical.SwitchResource;
import org.opendaylight.vtn.javaapi.resources.physical.SwitchesResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class SwitchResourceValidator validates request Json object for Switch
 * API.
 */
public class SwitchResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(SwitchResourceValidator.class.getName());

	private final AbstractResource resource;
	final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new switch resource validator.
	 * 
	 * @param switchResource
	 *            the instance of AbstractResource
	 */
	public SwitchResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for Switch API
	 * 
	 * @return true, if successful
	 */
	@Override
	public boolean validateUri() {
		LOG.trace("Start SwitchResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.CONTROLLERID);
		if (resource instanceof SwitchResource
				&& ((SwitchResource) resource).getControllerId() != null
				&& !((SwitchResource) resource).getControllerId().trim()
						.isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((SwitchResource) resource).getControllerId().trim(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.SWITCHID);
				if (((SwitchResource) resource).getSwitchId() != null
						&& !((SwitchResource) resource).getSwitchId().trim()
								.isEmpty()) {
					isValid = validator.isValidMaxLength(
							((SwitchResource) resource).getSwitchId().trim(),
							VtnServiceJsonConsts.LEN_255);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		} else if (resource instanceof SwitchesResource
				&& ((SwitchesResource) resource).getControllerId() != null
				&& !((SwitchesResource) resource).getControllerId().trim()
						.isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((SwitchesResource) resource).getControllerId().trim(),
					VtnServiceJsonConsts.LEN_31);
			setListOpFlag(true);
		}
		LOG.trace("Complete SwitchResourceValidator#validateUri()");
		return isValid;
	}

	/*
	 * Validate get request json for Switch API
	 */
	@Override
	public void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start SwitchResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of SwitchResourceValidator");
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
		LOG.trace("Complete SwitchResourceValidator#validate()");
	}

	/**
	 * Validate get request json for Switch API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if is valid get
	 */
	public boolean validateGet(final JsonObject requestBody,
			final boolean opFlag) {
		LOG.trace("Start SwitchResourceValidator#ValidateGet");
		boolean isValid = true;
		// validation for key: targetdb
		// updated check for targetdb
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
			if (isValid) {
				// validation for key: op
				setInvalidParameter(VtnServiceJsonConsts.OP);
				isValid = validator.isValidOperation(requestBody);
			}
			if (isValid) {
				// validation for key: index
				setInvalidParameter(VtnServiceJsonConsts.INDEX);
				if (requestBody.has(VtnServiceJsonConsts.INDEX)
						&& requestBody.getAsJsonPrimitive(
								VtnServiceJsonConsts.INDEX).getAsString() != null
						&& !requestBody
								.getAsJsonPrimitive(VtnServiceJsonConsts.INDEX)
								.getAsString().trim().isEmpty()) {
					isValid = validator
							.isValidMaxLength(
									requestBody
											.getAsJsonPrimitive(
													VtnServiceJsonConsts.INDEX)
											.getAsString().trim(),
									VtnServiceJsonConsts.LEN_255);
				}
			}
			if (isValid) {
				// validation for key: max_repitition
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
		LOG.trace("Complete SwitchResourceValidator#ValidateGet");
		return isValid;
	}
}