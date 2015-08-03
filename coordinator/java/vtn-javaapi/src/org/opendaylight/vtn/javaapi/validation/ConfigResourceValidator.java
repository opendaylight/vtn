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
import org.opendaylight.vtn.javaapi.validation.CommonValidator;

/**
 * The Class ConfigResourceValidator validates request Json object for Commit
 * Configuration and Save Configuration API.
 */
public class ConfigResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(ConfigResourceValidator.class.getName());

	/** The validator. */
	private final CommonValidator validator = new CommonValidator();

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
		String operation = null;
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
				operation = configuration.getAsJsonPrimitive(
						VtnServiceJsonConsts.OPERATION).getAsString();
				isValid = operation
						.equalsIgnoreCase(VtnServiceJsonConsts.COMMIT)
						|| operation
								.equalsIgnoreCase(VtnServiceJsonConsts.SAVE);
			}

			//if commit, check the mandatory param:timeout
			if (isValid &&
					operation.equalsIgnoreCase(VtnServiceJsonConsts.COMMIT)) {
				setInvalidParameter(VtnServiceJsonConsts.TIMEOUT);
				if (configuration.has(VtnServiceJsonConsts.TIMEOUT)) {
					String timeout = configuration.getAsJsonPrimitive(
							VtnServiceJsonConsts.TIMEOUT).getAsString();
					if (timeout != null && !timeout.isEmpty()) {
						try {
							long min = Integer.MIN_VALUE;
							long max = Integer.MAX_VALUE;
							isValid = validator.isValidRange(timeout, min, max);
						} catch (NumberFormatException e) {
							LOG.warning("ConfigResourceValidator#validatePut():timeout is invalid."
									+ timeout);
						isValid = false;
					}
					} else {
						isValid = false;
					}
				} else {
					isValid = false;
				}
			}

			//if commit, check the mandatory param:cancel_audit
			if (isValid && 
					operation.equalsIgnoreCase(VtnServiceJsonConsts.COMMIT)) {
				setInvalidParameter(VtnServiceJsonConsts.CANCEL_AUDIT);
				if (configuration.has(VtnServiceJsonConsts.CANCEL_AUDIT)) {
					String cancelAudit = configuration.getAsJsonPrimitive(
							VtnServiceJsonConsts.CANCEL_AUDIT).getAsString();
					if (cancelAudit != null && !cancelAudit.isEmpty()) {
						try {
							if (Integer.parseInt(cancelAudit) == 0 
									|| Integer.parseInt(cancelAudit) == 1) {
								isValid = true;
							} else {
								isValid = false;
							}
						} catch (NumberFormatException e) {
							LOG.warning("ConfigResourceValidator#validatePut():cancelAudit is invalid."
										+ cancelAudit);
							isValid = false;
						}
					} else {
						isValid = false;
					}
				} else {
					isValid = false;
				}
			}
		}
		LOG.trace("Complete ConfigResourceValidator#validatePut()");
		return isValid;
	}
}
