/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.validation;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.resources.UserResource;

/**
 * The Class UserResourceValidator validated get method of Set Password API.
 */
public class UserResourceValidator extends VtnServiceValidator {

	/** The Constant LOG. */
	private static final Logger LOG = Logger
			.getLogger(UserResourceValidator.class.getName());

	/** The instance of AbstractResource. */
	private final AbstractResource resource;

	/** The validator. */
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new user resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public UserResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri.
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start UserResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.USERNAME);
		if (resource instanceof UserResource
				&& ((UserResource) resource).getUserName() != null
				&& !((UserResource) resource).getUserName().isEmpty()) {
			isValid = ((UserResource) resource).getUserName().equalsIgnoreCase(
					VtnServiceJsonConsts.ADMIN)
					|| ((UserResource) resource).getUserName()
							.equalsIgnoreCase(VtnServiceJsonConsts.OPER);
		}
		LOG.trace("Complete UserResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request json for Set Password API
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start UserResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of UserResourceValidator");
		boolean isValid = false;
		isValid = validateUri();
		if (isValid && requestBody != null
				&& VtnServiceConsts.PUT.equalsIgnoreCase(method)) {
			isValid = validatePut(requestBody);
		} else if (isValid) {
			setInvalidParameter(VtnServiceConsts.INCORRECT_METHOD_INVOCATION);
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
		LOG.trace("Complete UserResourceValidator#validate()");
	}

	/**
	 * Validate put request Json for Set Password API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validatePut(final JsonObject requestBody) {
		LOG.trace("Start UserResourceValidator#validatePut()");
		boolean isValid = false;
		// validation for password
		setInvalidParameter(VtnServiceJsonConsts.PASSWORD);
		if (requestBody.has(VtnServiceJsonConsts.PASSWORD)
				&& requestBody.get(VtnServiceJsonConsts.PASSWORD) instanceof JsonObject
				&& requestBody.get(VtnServiceJsonConsts.PASSWORD)
						.getAsJsonObject()
						.getAsJsonPrimitive(VtnServiceJsonConsts.PASSWORD)
						.getAsString() != null) {
			final String password = requestBody
					.get(VtnServiceJsonConsts.PASSWORD).getAsJsonObject()
					.get(VtnServiceJsonConsts.PASSWORD).getAsString();
			requestBody.remove(VtnServiceJsonConsts.PASSWORD);
			requestBody.addProperty(VtnServiceJsonConsts.PASSWORD, password);
		}
		if (requestBody.has(VtnServiceJsonConsts.PASSWORD)
				&& requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.PASSWORD)
						.getAsString() != null
				&& !requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.PASSWORD)
						.getAsString().isEmpty()) {
			isValid = validator.isValidMaxLength(requestBody
					.getAsJsonPrimitive(VtnServiceJsonConsts.PASSWORD)
					.getAsString(), VtnServiceJsonConsts.LEN_72);
		} else {
			isValid = false;
		}
		LOG.trace("Complete UserResourceValidator#validatePut()");
		return isValid;
	}
}
