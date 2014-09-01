/*
 * Copyright (c) 2013-2014 NEC Corporation
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
 * The Class ClearStartupConfigurationResourceValidator validates request Json
 * object for Clear Startup Configuration API.
 */
public class ClearStartupConfigurationResourceValidator extends
		VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(ClearStartupConfigurationResourceValidator.class
					.getName());

	private final CommonValidator validator = new CommonValidator();

	/** The instance of AbstractResource. */
	private final AbstractResource resource;

	/**
	 * Instantiates a new ClearStartup Configuration Resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public ClearStartupConfigurationResourceValidator(
			final AbstractResource resource) {
		this.resource = resource;
		LOG.info(this.resource.toString());
	}

	/**
	 * Validate request Json for put method of Clear Startup Configuration API.
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start ClearStartupConfigurationResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of ClearStartupConfigurationResourceValidator");
		boolean isValid = false;
		try {
			if (requestBody != null && VtnServiceConsts.PUT.equals(method)) {
				isValid = validatePut(requestBody);
			} else {
				setInvalidParameter(VtnServiceConsts.INCORRECT_METHOD_INVOCATION);
				isValid = false;
			}
		} catch (final NumberFormatException e) {
			LOG.error(e, "Inside catch:NumberFormatException");
			if (method.equals(VtnServiceConsts.GET)) {
				setInvalidParameter(validator.getInvalidParameter());
			}
			isValid = false;
		}
		if (!isValid) {
			LOG.error("Validation failed");
			throw new VtnServiceException(Thread.currentThread()
					.getStackTrace()[1].getMethodName(),
					UncJavaAPIErrorCode.VALIDATION_ERROR.getErrorCode(),
					UncJavaAPIErrorCode.VALIDATION_ERROR.getErrorMessage());
		}
		LOG.info("Validation successful");
		LOG.trace("Complete ClearStartupConfigurationResourceValidator#validate()");
	}

	/**
	 * Validate put request json for Clear Startup Configuration API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validatePut(final JsonObject requestBody) {
		LOG.trace("Start ClearStartupConfigurationResourceValidator#validatePut()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.STARTUP);
		if (requestBody.has(VtnServiceJsonConsts.STARTUP)
				&& requestBody.get(VtnServiceJsonConsts.STARTUP).isJsonObject()) {
			final JsonObject startup = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.STARTUP);
			setInvalidParameter(VtnServiceJsonConsts.OPERATION);
			if (startup.has(VtnServiceJsonConsts.OPERATION)
					&& startup.getAsJsonPrimitive(
							VtnServiceJsonConsts.OPERATION).getAsString() != null
					&& !startup
							.getAsJsonPrimitive(VtnServiceJsonConsts.OPERATION)
							.getAsString().isEmpty()) {
				isValid = startup
						.getAsJsonPrimitive(VtnServiceJsonConsts.OPERATION)
						.getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.CLEAR);
			}
		}
		LOG.trace("Complete ClearStartupConfigurationResourceValidator#validatePut()");
		return isValid;
	}
}
