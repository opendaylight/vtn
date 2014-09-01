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
 * The Class ReadLockResourceValidator validates put method of Acquire Read Lock
 * API.
 */
public class ReadLockResourceValidator extends VtnServiceValidator {

	/** The Constant LOG. */
	private static final Logger LOG = Logger
			.getLogger(ReadLockResourceValidator.class.getName());

	/** The validator. */
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new read lock resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public ReadLockResourceValidator(final AbstractResource resource) {

	}

	/**
	 * Validate request json for Read Lock API
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("ReadLockResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of ReadLockResourceValidator");
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
		LOG.trace("Complete ReadLockResourceValidator#validate()");
	}

	/**
	 * Validate put request Json for Acquire Read Lock API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validatePut(final JsonObject requestBody) {
		LOG.trace("Start ReadLockResourceValidator#validatePut()");
		boolean isValid = false;
		// validation for timeout as an integer
		setInvalidParameter(VtnServiceJsonConsts.READLOCK);
		if (requestBody.has(VtnServiceJsonConsts.READLOCK)
				&& requestBody.get(VtnServiceJsonConsts.READLOCK)
						.isJsonObject()) {
			isValid = true;
			final JsonObject readLock = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.READLOCK);
			setInvalidParameter(VtnServiceJsonConsts.TIMEOUT);
			if (readLock.has(VtnServiceJsonConsts.TIMEOUT)
					&& readLock
							.getAsJsonPrimitive(VtnServiceJsonConsts.TIMEOUT)
							.getAsString() != null) {
				isValid = validator.isValidRange(
						readLock.getAsJsonPrimitive(
								VtnServiceJsonConsts.TIMEOUT).getAsString(),
						VtnServiceJsonConsts.LONG_VAL_0,
						VtnServiceJsonConsts.LONG_VAL_4294967295);
			}
		}
		LOG.trace("Complete ReadLockResourceValidator#validatePut()");
		return isValid;
	}
}
