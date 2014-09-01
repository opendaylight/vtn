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

/**
 * The Class ConfigResourceValidator validates request Json object for Commit
 * Configuration and Save Configuration API.
 */
public class ConfigResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(ConfigResourceValidator.class.getName());

	/**
	 * Instantiates a new config resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public ConfigResourceValidator(final AbstractResource resource) {
	}

	/**
	 * Validate request json for put method of Commit Configuration and Save
	 * Configuration API.
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start ConfigResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of ConfigResourceValidator");
		boolean isValid = false;
		if (requestBody != null && VtnServiceConsts.PUT.equals(method)) {
			isValid = validatePut(requestBody);
		} else {
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
		LOG.trace("Complete ConfigResourceValidator#validate()");
	}

	/**
	 * Validate put request json for Commit Configuration and Save Configuration
	 * API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validatePut(final JsonObject requestBody) {
		LOG.trace("Start ConfigResourceValidator#validatePut()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.CONFIGURATION);
		if (requestBody.has(VtnServiceJsonConsts.CONFIGURATION)
				&& requestBody.get(VtnServiceJsonConsts.CONFIGURATION)
						.isJsonObject()) {
			final JsonObject configuration = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.CONFIGURATION);
			setInvalidParameter(VtnServiceJsonConsts.OPERATION);
			if (configuration.has(VtnServiceJsonConsts.OPERATION)
					&& configuration.getAsJsonPrimitive(
							VtnServiceJsonConsts.OPERATION).getAsString() != null) {
				final String operation = configuration.getAsJsonPrimitive(
						VtnServiceJsonConsts.OPERATION).getAsString();
				isValid = operation
						.equalsIgnoreCase(VtnServiceJsonConsts.COMMIT)
						|| operation
								.equalsIgnoreCase(VtnServiceJsonConsts.SAVE);
			}
		}
		LOG.trace("Complete ConfigResourceValidator#validatePut()");
		return isValid;
	}
}
