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
import org.opendaylight.vtn.javaapi.resources.logical.VtnResource;
import org.opendaylight.vtn.javaapi.resources.logical.VtnsResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class VtnResourceValidator validates request Json object for Vtn API.
 */
public class VtnResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(VtnResourceValidator.class.getName());

	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new vtn resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public VtnResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for Vtn API
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start VtnResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.VTNNAME);
		if (resource instanceof VtnResource
				&& ((VtnResource) resource).getVtnName() != null
				&& !((VtnResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VtnResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			setListOpFlag(false);
		} else if (resource instanceof VtnsResource) {
			isValid = true;
			setListOpFlag(true);
		}
		LOG.trace("Complete VtnResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request Json object for get, put and post method of Vtn API
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VtnResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of VtnResourceValidator");
		boolean isValid = false;
		try {
			isValid = validateUri();
			if (isValid && requestBody != null
					&& VtnServiceConsts.GET.equals(method)) {
				isValid = validator.isValidGet(requestBody, isListOpFlag());
				setInvalidParameter(validator.getInvalidParameter());
				updateOpParameterForList(requestBody);
			} else if (isValid && requestBody != null
					&& VtnServiceConsts.POST.equalsIgnoreCase(method)) {
				isValid = validatePost(requestBody);
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
		LOG.trace("Complete VtnResourceValidator#validate()");
	}

	/**
	 * Validate post request Json object for Vtn API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validatePost(final JsonObject requestBody) {
		LOG.trace("Start VtnResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.VTN);
		if (requestBody.has(VtnServiceJsonConsts.VTN)
				&& requestBody.get(VtnServiceJsonConsts.VTN).isJsonObject()) {
			final JsonObject vtn = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VTN);
			setInvalidParameter(VtnServiceJsonConsts.VTNNAME);
			// validation for mandatory key: vtn_name
			if (vtn.has(VtnServiceJsonConsts.VTNNAME)
					&& vtn.getAsJsonPrimitive(VtnServiceJsonConsts.VTNNAME)
							.getAsString() != null) {
				isValid = validator.isValidMaxLengthAlphaNum(vtn
						.getAsJsonPrimitive(VtnServiceJsonConsts.VTNNAME)
						.getAsString(), VtnServiceJsonConsts.LEN_31);
			}
			if (isValid) {
				isValid = validatePut(requestBody);
			}
		}
		LOG.trace("Complete VtnResourceValidator#validatePost()");
		return isValid;
	}

	/**
	 * Validate put request Json object for Vtn API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validatePut(final JsonObject requestBody) {
		LOG.trace("Start VtnResourceValidator#validatePut()");
		boolean isValid = true;
		setInvalidParameter(VtnServiceJsonConsts.VTN);
		if (requestBody.has(VtnServiceJsonConsts.VTN)
				&& requestBody.get(VtnServiceJsonConsts.VTN).isJsonObject()) {
			// validation for key: description
			setInvalidParameter(VtnServiceJsonConsts.DESCRIPTION);
			if (requestBody.getAsJsonObject(VtnServiceJsonConsts.VTN).has(
					VtnServiceJsonConsts.DESCRIPTION)
					&& requestBody
							.getAsJsonObject(VtnServiceJsonConsts.VTN)
							.getAsJsonPrimitive(
									VtnServiceJsonConsts.DESCRIPTION)
							.getAsString() != null
					&& !requestBody
							.getAsJsonObject(VtnServiceJsonConsts.VTN)
							.getAsJsonPrimitive(
									VtnServiceJsonConsts.DESCRIPTION)
							.getAsString().isEmpty()) {
				isValid = validator.isValidMaxLength(requestBody
						.getAsJsonObject(VtnServiceJsonConsts.VTN)
						.getAsJsonPrimitive(VtnServiceJsonConsts.DESCRIPTION)
						.getAsString(), VtnServiceJsonConsts.LEN_127);
			}
		} else {
			isValid = false;
		}
		LOG.trace("Complete VtnResourceValidator#validatePut()");
		return isValid;
	}
}
