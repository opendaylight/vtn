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
 * The Class AutoSaveResourceValidatorvalidates request Json object for
 * Enable/Disable Auto-save Status API and Show Auto-save Status API
 */
public class AutoSaveResourceValidator extends VtnServiceValidator {

	/** The Constant LOG. */
	private static final Logger LOG = Logger
			.getLogger(AutoSaveResourceValidator.class.getName());

	/** The the instance of AbstractResource. */
	private final AbstractResource resource;

	/**
	 * Instantiates a new auto save resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public AutoSaveResourceValidator(final AbstractResource resource) {
		this.resource = resource;
		LOG.info(this.resource.toString());
	}

	/**
	 * Validate request json for Enable/Disable Auto-save Status API and Show
	 * Auto-save Status API
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("AutoSaveResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of AutoSaveResourceValidator");
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
		LOG.trace("Complete AutoSaveResourceValidator#validate()");
	}

	/**
	 * Validate put request json for Enable/Disable Auto-save Status API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validatePut(final JsonObject requestBody) {
		LOG.trace("Start AutoSaveResourceValidator#validatePut()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.AUTOSAVE);
		if (requestBody.has(VtnServiceJsonConsts.AUTOSAVE)
				&& requestBody.get(VtnServiceJsonConsts.AUTOSAVE)
						.isJsonObject()) {
			final JsonObject autoSave = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.AUTOSAVE);
			setInvalidParameter(VtnServiceJsonConsts.AUTOSAVESTATUS);
			if (autoSave.has(VtnServiceJsonConsts.AUTOSAVESTATUS)
					&& autoSave.getAsJsonPrimitive(
							VtnServiceJsonConsts.AUTOSAVESTATUS).getAsString() != null) {
				// checking whether auto_save_status is enable or disable
				final String status = autoSave.getAsJsonPrimitive(
						VtnServiceJsonConsts.AUTOSAVESTATUS).getAsString();
				isValid = status.equalsIgnoreCase(VtnServiceJsonConsts.ENABLE)
						|| status
								.equalsIgnoreCase(VtnServiceJsonConsts.DISABLE);
			}
		}
		LOG.trace("Complete AutoSaveResourceValidator#validatePut()");
		return isValid;
	}
}
