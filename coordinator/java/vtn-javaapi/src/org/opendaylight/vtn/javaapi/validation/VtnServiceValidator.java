/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.validation;

import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;

/**
 * The Class VtnServiceValidator.
 */
public abstract class VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(VtnServiceValidator.class.getName());

	private String invalidParameter = null;
	private boolean listOpFlag;

	/**
	 * Sets the list op flag.
	 * 
	 * @param listOpFlag
	 *            the new list op flag
	 */
	protected final void setListOpFlag(final boolean listOpFlag) {
		this.listOpFlag = listOpFlag;
	}

	/**
	 * Checks if is list op flag.
	 * 
	 * @return true, if is list op flag
	 */
	public final boolean isListOpFlag() {
		return listOpFlag;
	}

	/**
	 * Update op parameter for list.
	 * 
	 * @param requestBody
	 *            the request body
	 */
	public final void updateOpParameterForList(final JsonObject requestBody) {
		if (listOpFlag && !requestBody.has(VtnServiceJsonConsts.OP)) {
			requestBody.addProperty(VtnServiceJsonConsts.OP,
					VtnServiceJsonConsts.NORMAL);
		}
	}

	/**
	 * Gets the invalid parameter.
	 * 
	 * @return the invalid parameter
	 */
	public final String getInvalidParameter() {
		int colonIndex = invalidParameter.indexOf(VtnServiceConsts.COLON);
		if (colonIndex > -1
				&& invalidParameter.substring(colonIndex + 1).length() > VtnServiceOpenStackConsts.MAX_MSG_LEN) {
			LOG.debug("message length is more than 1024, required to truncate");
			String firstPart = invalidParameter.substring(0, colonIndex + 1);
			String secondPart = invalidParameter.substring(colonIndex + 1)
					.substring(0, VtnServiceOpenStackConsts.MAX_MSG_LEN);
			invalidParameter = firstPart + secondPart;
		}
		return invalidParameter;
	}

	/**
	 * Sets the invalid parameter.
	 * 
	 * @param invalidParameter
	 *            the new invalid parameter
	 */
	public final void setInvalidParameter(final String invalidParameter) {
		this.invalidParameter = invalidParameter;
	}

	/**
	 * Default implementation of Validate method.
	 * 
	 * @param method
	 *            the method
	 * @param requestBody
	 *            the request body
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	public void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Return from VtnServiceValidator#validate");
	}

	/**
	 * Default implementation of Validate method.
	 * 
	 * @param method
	 *            the method called
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 * 
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	public void validate(final String method, final AbstractResource resource)
			throws VtnServiceException {
		LOG.trace("Return from VtnServiceValidator#validate");
		boolean isValid = false;
		if (VtnServiceConsts.GET.equals(method)
				|| VtnServiceConsts.DELETE.equals(method)) {
			isValid = resource.getValidator().validateUri();
		}
		// Throws exception if validation fails
		if (!isValid) {
			LOG.error("Validation failed");
			throw new VtnServiceException(Thread.currentThread()
					.getStackTrace()[1].getMethodName(),
					UncJavaAPIErrorCode.VALIDATION_ERROR.getErrorCode(),
					UncJavaAPIErrorCode.VALIDATION_ERROR.getErrorMessage());
		}
		LOG.info("URI Validation successful");
	}

	/**
	 * Default implementation of Validate URI method.
	 * 
	 * @return true, if successful
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	public boolean validateUri() throws VtnServiceException {
		LOG.trace("Return from VtnServiceValidator#validate");
		return true;
	}

	/**
	 * Validates the parameters of POST request body
	 * 
	 * @param validator
	 *            - instance for CommonValidator
	 * @param requestBody
	 *            - JSON object contains "id" and "description" parameters
	 * @return - validation status as true or false
	 */
	public boolean validatePost(final CommonValidator validator,
			final JsonObject requestBody) {
		LOG.trace("Start VtnServiceValidator#validatePost()");
		boolean isValid = true;
		// validation of id
		if (requestBody != null
				&& requestBody.has(VtnServiceOpenStackConsts.ID)) {
			final JsonElement id = requestBody
					.get(VtnServiceOpenStackConsts.ID);
			if (id.isJsonNull()
					|| id.getAsString().isEmpty()
					|| !validator.isValidMaxLengthAlphaNum(id.getAsString(),
							VtnServiceJsonConsts.LEN_31)) {
				isValid = false;
				setInvalidParameter(VtnServiceOpenStackConsts.ID
						+ VtnServiceConsts.COLON
						+ (id.isJsonNull() ? id : id.getAsString()));
			}
		}

		if (isValid) {
			isValid = validatePut(validator, requestBody);
		}

		LOG.trace("Complete VtnServiceValidator#validatePost()");
		return isValid;
	}

	/**
	 * Validates the parameters of PUT request body
	 * 
	 * @param validator
	 *            - instance for CommonValidator
	 * @param requestBody
	 *            - JSON object "description" parameter
	 * @return - validation status as true or false
	 */
	public boolean validatePut(final CommonValidator validator,
			final JsonObject requestBody) {
		LOG.trace("Start VtnServiceValidator#validatePut()");
		boolean isValid = true;
		// validation of description
		if (requestBody != null
				&& requestBody.has(VtnServiceOpenStackConsts.DESCRIPTION)) {
			final JsonElement description = requestBody
					.get(VtnServiceOpenStackConsts.DESCRIPTION);
			if (!description.isJsonNull()) {
				if (hasInvaidDescChars(description.getAsString())
						|| !validator.isValidMaxLength(
								description.getAsString(),
								VtnServiceJsonConsts.LEN_127)) {
					isValid = false;
					setInvalidParameter(VtnServiceOpenStackConsts.DESCRIPTION
							+ VtnServiceConsts.COLON
							+ description.getAsString());
				}
			}
		}
		LOG.trace("Complete VtnServiceValidator#validatePut()");
		return isValid;
	}

	/**
	 * Check if description string contains invalid characters or not
	 * 
	 * @param description
	 *            - string that required to validate
	 * @return - result as true or false
	 */
	private boolean hasInvaidDescChars(String description) {
		return ((description.contains(VtnServiceConsts.QUESTION_MARK)) || (description
				.contains(VtnServiceConsts.QUOTE_CHAR)));
	}
}
