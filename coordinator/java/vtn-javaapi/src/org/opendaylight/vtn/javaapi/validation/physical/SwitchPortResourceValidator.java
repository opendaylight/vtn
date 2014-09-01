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
import org.opendaylight.vtn.javaapi.resources.physical.SwitchPortsResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class SwitchPortResourceValidator validates request Json object for Port
 * API.
 */
public class SwitchPortResourceValidator extends VtnServiceValidator {
	/**
	 * logger object for debugging.
	 */
	private static final Logger LOG = Logger
			.getLogger(SwitchPortResourceValidator.class.getName());
	/**
	 * Abstract resource.
	 */
	private final AbstractResource resource;
	/**
	 * Common validator class for common validations.
	 */
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new switch port resource validator.
	 * 
	 * @param resource1
	 *            the instance of AbstractResource
	 */
	public SwitchPortResourceValidator(final AbstractResource resource1) {
		this.resource = resource1;
	}

	/**
	 * Validate uri parameters for Port API.
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start SwitchPortResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.CONTROLLERID);
		if (resource instanceof SwitchPortsResource
				&& ((SwitchPortsResource) resource).getControllerId() != null
				&& !((SwitchPortsResource) resource).getControllerId()
						.isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((SwitchPortsResource) resource).getControllerId(),
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
	public final void validate(final String method, final JsonObject requestBody)
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
		LOG.trace("Complete SwitchPortResourceValidator#validate()");
	}

	/**
	 * Validate get request json for Port API.
	 * 
	 * @param opFlag
	 *            ,opflag set
	 * @param requestBody
	 *            the request Json object
	 * @return true, if is valid get
	 */
	public final boolean validateGet(final JsonObject requestBody,
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
				isValid = validator.isValidOperationInfo(requestBody);
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
							.getAsString(), VtnServiceJsonConsts.LEN_31);
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
				 * ); }
				 */
			}
		}
		if (isValid) {
			// validation for key: Port Name(Optional)
			setInvalidParameter(VtnServiceJsonConsts.PORTNAME);
			if (requestBody.has(VtnServiceJsonConsts.PORTNAME)
					&& requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.PORTNAME).getAsString() != null
					&& !requestBody
							.getAsJsonPrimitive(VtnServiceJsonConsts.PORTNAME)
							.getAsString().isEmpty()) {
				isValid = validator.isValidMaxLength(requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.PORTNAME)
						.getAsString(), VtnServiceJsonConsts.LEN_31);
			}
		}

		if (isValid) {
			// validation for key: Port Id(Optional)
			setInvalidParameter(VtnServiceJsonConsts.PORT_ID);
			if (requestBody.has(VtnServiceJsonConsts.PORT_ID)
					&& requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.PORT_ID).getAsString() != null
					&& !requestBody
							.getAsJsonPrimitive(VtnServiceJsonConsts.PORT_ID)
							.getAsString().isEmpty()) {
				isValid = validator.isValidRange(requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.PORT_ID)
						.getAsString(), VtnServiceJsonConsts.LONG_VAL_0,
						VtnServiceJsonConsts.LONG_VAL_4294967295);
			}
		}
		LOG.trace("Complete SwitchPortResourceValidator#isValidGet");
		return isValid;
	}
}
