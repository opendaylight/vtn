/*
 * Copyright (c) 2012-2014 NEC Corporation
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
	/**
	 * Logger for debugging purpose.
	 */
	private static final Logger LOG = Logger
			.getLogger(SwitchResourceValidator.class.getName());
	/**
	 * resource , the instance of AbstractResource.
	 */
	private final AbstractResource resource;
	/**
	 * validator object for common validations.
	 */
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new switch resource validator.
	 * 
	 * @param resource
	 *            , the instance of AbstractResource.
	 */
	public SwitchResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for Switch API.
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start SwitchResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.CONTROLLERID);
		if (resource instanceof SwitchResource
				&& ((SwitchResource) resource).getControllerId() != null
				&& !((SwitchResource) resource).getControllerId().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((SwitchResource) resource).getControllerId(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.SWITCHID);
				if (((SwitchResource) resource).getSwitchId() != null
						&& !((SwitchResource) resource).getSwitchId().isEmpty()) {
					isValid = validator.isValidMaxLength(
							((SwitchResource) resource).getSwitchId(),
							VtnServiceJsonConsts.LEN_255);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		} else if (resource instanceof SwitchesResource
				&& ((SwitchesResource) resource).getControllerId() != null
				&& !((SwitchesResource) resource).getControllerId().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((SwitchesResource) resource).getControllerId(),
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
	public final void validate(final String method, final JsonObject requestBody)
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
			LOG.error(e, "Inside catch:NumberFormatException");
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
	 * Validate get request json for Switch API.
	 * 
	 * @param requestBody
	 *            , the request Json object .
	 * @return true, if is valid get .
	 * @param opFlag
	 *            ,to reolve type of operations .
	 */
	public final boolean validateGet(final JsonObject requestBody,
			final boolean opFlag) {
		LOG.trace("Start SwitchResourceValidator#ValidateGet");
		boolean isValid = true;
		// validation for key: targetdb
		setInvalidParameter(VtnServiceJsonConsts.TARGETDB);
		if (requestBody.has(VtnServiceJsonConsts.TARGETDB)
				&& requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.TARGETDB)
						.getAsString() != null
				&& !requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.TARGETDB)
						.getAsString().isEmpty()) {
			isValid = VtnServiceJsonConsts.STATE.equalsIgnoreCase(requestBody
					.getAsJsonPrimitive(VtnServiceJsonConsts.TARGETDB)
					.getAsString());
		} else {
			requestBody.remove(VtnServiceJsonConsts.TARGETDB);
			requestBody.addProperty(VtnServiceJsonConsts.TARGETDB,
					VtnServiceJsonConsts.STATE);
		}
		if (!opFlag) {
			// validation for key: op
			setInvalidParameter(VtnServiceJsonConsts.OP);
			if (requestBody.has(VtnServiceJsonConsts.OP)
					&& requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.OP)
							.getAsString() != null
					&& !requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.OP)
							.getAsString().isEmpty()) {
				final String operation = requestBody.getAsJsonPrimitive(
						VtnServiceJsonConsts.OP).getAsString();
				isValid = operation
						.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL);
			} else {
				requestBody.remove(VtnServiceJsonConsts.OP);
				requestBody.addProperty(VtnServiceJsonConsts.OP,
						VtnServiceJsonConsts.NORMAL);
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
				if (requestBody.has(VtnServiceJsonConsts.OP)
						&& requestBody.getAsJsonPrimitive(
								VtnServiceJsonConsts.OP).getAsString() != null
						&& !requestBody
								.getAsJsonPrimitive(VtnServiceJsonConsts.OP)
								.getAsString().isEmpty()) {
					final String operation = requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.OP).getAsString();
					isValid = operation
							.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)
							|| operation
									.equalsIgnoreCase(VtnServiceJsonConsts.COUNT);
				} else {
					requestBody.remove(VtnServiceJsonConsts.OP);
					requestBody.addProperty(VtnServiceJsonConsts.OP,
							VtnServiceJsonConsts.NORMAL);
				}
			}
			if (isValid) {
				// validation for key: index
				setInvalidParameter(VtnServiceJsonConsts.INDEX);
				if (requestBody.has(VtnServiceJsonConsts.INDEX)
						&& requestBody.getAsJsonPrimitive(
								VtnServiceJsonConsts.INDEX).getAsString() != null
						&& !requestBody
								.getAsJsonPrimitive(VtnServiceJsonConsts.INDEX)
								.getAsString().isEmpty()) {
					isValid = validator.isValidMaxLength(requestBody
							.getAsJsonPrimitive(VtnServiceJsonConsts.INDEX)
							.getAsString(), VtnServiceJsonConsts.LEN_255);
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
				 * ); }
				 */
			}
		}
		LOG.trace("Complete SwitchResourceValidator#ValidateGet");
		return isValid;
	}
}
