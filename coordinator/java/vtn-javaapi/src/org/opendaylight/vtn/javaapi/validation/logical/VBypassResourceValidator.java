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
import org.opendaylight.vtn.javaapi.resources.logical.VBypassResource;
import org.opendaylight.vtn.javaapi.resources.logical.VBypassesResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class VBypassResourceValidator validates request Json object for VBypass
 * API.
 */
public class VBypassResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(VBypassResourceValidator.class.getName());
	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new VBypass resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public VBypassResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for VBypassResource
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start VBypassResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.VTNNAME);
		if (resource instanceof VBypassResource
				&& ((VBypassResource) resource).getVtnName() != null
				&& !((VBypassResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VBypassResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VBYPASS_NAME);
				if (((VBypassResource) resource).getVbypassName() != null
						&& !((VBypassResource) resource).getVbypassName()
								.isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VBypassResource) resource).getVbypassName(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		} else if (resource instanceof VBypassesResource
				&& ((VBypassesResource) resource).getVtnName() != null
				&& !((VBypassesResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VBypassesResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			setListOpFlag(true);
		}
		LOG.trace("Complete VBypassResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request Json object for get, put and post method of VBypass API
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VBypassResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of VBypassResourceValidator");
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
			} else if (isValid && requestBody != null
					&& VtnServiceConsts.POST.equals(method)) {
				isValid = validatePost(requestBody);
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
		LOG.trace("Complete VBypassResourceValidator#validate()");
	}

	/**
	 * Validate post request Json object for VBypassResource.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	private boolean validatePost(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VBypassResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.VBYPASS);
		if (requestBody.has(VtnServiceJsonConsts.VBYPASS)
				&& requestBody.get(VtnServiceJsonConsts.VBYPASS).isJsonObject()) {
			final JsonObject vBypass = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VBYPASS);
			// validation for key: VBYPASS_NAME(mandatory)
			setInvalidParameter(VtnServiceJsonConsts.VBYPASS_NAME);
			if (vBypass.has(VtnServiceJsonConsts.VBYPASS_NAME)
					&& vBypass.getAsJsonPrimitive(
							VtnServiceJsonConsts.VBYPASS_NAME).getAsString() != null) {
				isValid = validator.isValidMaxLengthAlphaNum(vBypass
						.getAsJsonPrimitive(VtnServiceJsonConsts.VBYPASS_NAME)
						.getAsString(), VtnServiceJsonConsts.LEN_31);
			}
			if (isValid) {
				isValid = commonValidations(isValid, vBypass);
			}
			// validation for key: DOMAINID(mandatory)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.DOMAINID);
				if (vBypass.has(VtnServiceJsonConsts.DOMAINID)
						&& vBypass.getAsJsonPrimitive(
								VtnServiceJsonConsts.DOMAINID).getAsString() != null) {
					isValid = validator.isValidDomainId(vBypass
							.getAsJsonPrimitive(VtnServiceJsonConsts.DOMAINID)
							.getAsString(), VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
		}
		LOG.trace("Complete VBypassResourceValidator#validatePost()");
		return isValid;
	}

	/**
	 * @param isValid
	 * @param vBypass
	 * @return
	 */
	private boolean commonValidations(boolean isValid, final JsonObject vBypass) {
		LOG.trace("Start VBypassResourceValidator#commonValidations()");
		// validation for key: description
		setInvalidParameter(VtnServiceJsonConsts.DESCRIPTION);
		if (vBypass.has(VtnServiceJsonConsts.DESCRIPTION)
				&& vBypass.getAsJsonPrimitive(VtnServiceJsonConsts.DESCRIPTION)
						.getAsString() != null
				&& !vBypass
						.getAsJsonPrimitive(VtnServiceJsonConsts.DESCRIPTION)
						.getAsString().isEmpty()) {
			isValid = validator
					.isValidMaxLength(
							vBypass.getAsJsonPrimitive(
									VtnServiceJsonConsts.DESCRIPTION)
									.getAsString(),
							VtnServiceJsonConsts.LEN_127);
		}
		// validation for key: type
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.TYPE);
			if (vBypass.has(VtnServiceJsonConsts.TYPE)
					&& vBypass.getAsJsonPrimitive(VtnServiceJsonConsts.TYPE)
							.getAsString() != null) {
				isValid = validType(vBypass.getAsJsonPrimitive(
						VtnServiceJsonConsts.TYPE).getAsString());
			}
		}
		LOG.trace("Complete VBypassResourceValidator#commonValidations()");
		return isValid;
	}

	/**
	 * Validate put request Json object for VBypassResource
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	private boolean validatePut(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VBypassResourceValidator#validatePut()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.VBYPASS);
		if (requestBody.has(VtnServiceJsonConsts.VBYPASS)
				&& requestBody.get(VtnServiceJsonConsts.VBYPASS).isJsonObject()) {
			isValid = true;
			final JsonObject vBypass = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VBYPASS);
			isValid = commonValidations(isValid, vBypass);
		}
		LOG.trace("Complete VBypassResourceValidator#validatePut()");
		return isValid;
	}

	/**
	 * check if type is of valid type or not
	 * 
	 * @param type
	 *            value of type field
	 * @return
	 */
	private boolean validType(final String type) {
		if (VtnServiceConsts.EMPTY_STRING.equals(type)) {
			return true;
		}
		return VtnServiceJsonConsts.ROUTER.equalsIgnoreCase(type)
				|| VtnServiceJsonConsts.BRIDGE.equalsIgnoreCase(type);
	}
}
