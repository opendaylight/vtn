/*
 * Copyright (c) 2012-2015 NEC Corporation
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
import org.opendaylight.vtn.javaapi.resources.AcquireConfigModeResource;
import org.opendaylight.vtn.javaapi.resources.ReleaseConfigModeResource;

/**
 * The Class ConfigModeValidtaor validates request Json object for Acquire
 * Configuration Mode API.
 */
public class ConfigModeResourceValidator extends VtnServiceValidator {

	/** The Constant LOG. */
	private static final Logger LOG = Logger
			.getLogger(ConfigModeResourceValidator.class.getName());

	/** the instance of AbstractResource */
	private final AbstractResource resource;

	/** The validator. */
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new configuration mode resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public ConfigModeResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri.
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start ConfigModeResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.CONFIGID);
		if (resource instanceof ReleaseConfigModeResource
				&& ((ReleaseConfigModeResource) resource).getConfigId() != null
				&& !((ReleaseConfigModeResource) resource).getConfigId()
						.isEmpty()) {
			isValid = validator.isValidRange(
					((ReleaseConfigModeResource) resource).getConfigId(),
					VtnServiceJsonConsts.LONG_VAL_0,
					VtnServiceJsonConsts.LONG_VAL_4294967295);
		} else if (resource instanceof AcquireConfigModeResource) {
			isValid = true;
		}
		LOG.trace("Complete ConfigModeResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request json for Acquire Configuration Mode API
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start ConfigModeResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of ConfigModeResourceValidator");
		boolean isValid = validateUri();
		if (isValid && requestBody != null
				&& VtnServiceConsts.POST.equalsIgnoreCase(method)) {
			isValid = validatePost(requestBody);
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
		LOG.trace("Complete ConfigModeResourceValidator#validate()");
	}

	/**
	 * Validate post request Json for Acquire Configuration Mode API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validatePost(final JsonObject requestBody) {
		LOG.trace("Start ConfigModeResourceValidator#validatePost()");
		boolean isValid = true;
		boolean isVtnMode = false;
		setInvalidParameter(VtnServiceJsonConsts.OP);
		if (requestBody.has(VtnServiceJsonConsts.OP)
				&& requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.OP)
						.getAsString() != null) {
			isValid = requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.OP)
					.getAsString().equalsIgnoreCase(VtnServiceJsonConsts.FORCE);
		}

		if (isValid && requestBody.has(VtnServiceJsonConsts.TIMEOUT)) {
			String timeout = requestBody.getAsJsonPrimitive(
					VtnServiceJsonConsts.TIMEOUT).getAsString();
			if (timeout != null && !timeout.isEmpty()) {
				long min = Integer.MIN_VALUE;
				long max = Integer.MAX_VALUE;
				try {
					isValid = validator.isValidRange(timeout, min, max);
				} catch (Exception e) {
					isValid = false;
				}
			} else {
				isValid = false;
			}
			
			if (!isValid) {
				setInvalidParameter(VtnServiceJsonConsts.TIMEOUT);
			}
		}

		if (isValid && requestBody.has(VtnServiceJsonConsts.MODE)) {
			setInvalidParameter(VtnServiceJsonConsts.MODE);
			String mode = requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.MODE).getAsString();
			if (mode.equalsIgnoreCase(VtnServiceJsonConsts.VIRTUAL_MODE)
					|| mode.equalsIgnoreCase(VtnServiceJsonConsts.REAL_MODE)
					|| mode.equalsIgnoreCase(VtnServiceJsonConsts.VTN_MODE)
					|| mode.equalsIgnoreCase(VtnServiceJsonConsts.GLOBAL_MODE)) {
				isValid = true;
				if (mode.equalsIgnoreCase(VtnServiceJsonConsts.VTN_MODE)) {
					isVtnMode = true;
				}
			} else {
				isValid = false;
			}
		}

		if (isValid && isVtnMode) {
			setInvalidParameter(VtnServiceJsonConsts.VTNNAME);
			if (requestBody.has(VtnServiceJsonConsts.VTNNAME)) {
				String vtnName = requestBody.getAsJsonPrimitive(
						VtnServiceJsonConsts.VTNNAME).getAsString();
				if (vtnName != null && !vtnName.isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							vtnName, VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			} else {
				isValid = false;
			}
		}
		LOG.trace("Complete ConfigModeResourceValidator#validatePost()");
		return isValid;
	}
}
