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
 * The Class AbortCandidateConfigResourceValidator validates request Json object
 * for Abort Candidate Configuration API.
 */
public class AbortCandidateConfigResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(AbortCandidateConfigResourceValidator.class.getName());

	/** The instance of AbstractResource. */
	private final AbstractResource resource;

	/**
	 * Instantiates a new config resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public AbortCandidateConfigResourceValidator(final AbstractResource resource) {
		this.resource = resource;
		LOG.info(this.resource.toString());
	}

	/**
	 * Validate request Json for put method of Abort Candidate Configuration API
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start AbortCandidateConfigResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of AbortCandidateConfigResourceValidator");
		boolean isValid = false;
		if (requestBody != null && VtnServiceConsts.PUT.equals(method)) {
			isValid = validatePut(requestBody);
		} else {
			setInvalidParameter(VtnServiceConsts.INCORRECT_METHOD_INVOCATION);
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
		LOG.trace("Complete AbortCandidateConfigResourceValidator#validate()");
	}

	/**
	 * Validate put request json for Abort Candidate Configuration API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validatePut(final JsonObject requestBody) {
		LOG.trace("Start AbortCandidateConfigResourceValidator#validatePut()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.CANDIDATE);
		if (requestBody.has(VtnServiceJsonConsts.CANDIDATE)
				&& requestBody.get(VtnServiceJsonConsts.CANDIDATE)
						.isJsonObject()) {
			final JsonObject candidate = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.CANDIDATE);
			setInvalidParameter(VtnServiceJsonConsts.OPERATION);
			if (candidate.has(VtnServiceJsonConsts.OPERATION)
					&& candidate.getAsJsonPrimitive(
							VtnServiceJsonConsts.OPERATION).getAsString() != null) {
				final String operation = candidate.getAsJsonPrimitive(
						VtnServiceJsonConsts.OPERATION).getAsString();
				isValid = operation
						.equalsIgnoreCase(VtnServiceJsonConsts.ABORT);
			}
		}
		LOG.trace("Complete AbortCandidateConfigResourceValidator#validatePut()");
		return isValid;
	}
}
