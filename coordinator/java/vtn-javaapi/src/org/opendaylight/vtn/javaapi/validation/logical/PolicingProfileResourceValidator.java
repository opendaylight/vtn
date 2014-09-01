/*
 * Copyright (c) 2014 NEC Corporation
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
import org.opendaylight.vtn.javaapi.resources.logical.PolicingProfileResource;
import org.opendaylight.vtn.javaapi.resources.logical.PolicingProfilesResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

public class PolicingProfileResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(PolicingProfileResourceValidator.class.getName());

	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new policing profile resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public PolicingProfileResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate URI parameters for PolicingProfile API
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start PolicingProfileResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.PROFILE_NAME);
		if (resource instanceof PolicingProfileResource
				&& ((PolicingProfileResource) resource).getProfileName() != null
				&& !((PolicingProfileResource) resource).getProfileName()
						.isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((PolicingProfileResource) resource).getProfileName(),
					VtnServiceJsonConsts.LEN_32);
			setListOpFlag(false);
		} else if (resource instanceof PolicingProfilesResource) {
			isValid = true;
			setListOpFlag(true);
		}
		LOG.trace("Complete PolicingProfileResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request JSON object for get, put and post method of Policing
	 * Profile API
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start PolicingProfileResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of PolicingProfileResourceValidator");
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
		LOG.trace("Complete PolicingProfileResourceValidator#validate()");
	}

	/**
	 * Validate post request JSON object for Policing Profile API
	 * 
	 * @param requestBody
	 *            the request JSON object
	 * @return true, if successful
	 */
	private boolean validatePost(final JsonObject requestBody) {
		LOG.trace("Start PolicingProfileResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.POLICINGPROFILE);
		if (requestBody.has(VtnServiceJsonConsts.POLICINGPROFILE)
				&& requestBody.get(VtnServiceJsonConsts.POLICINGPROFILE)
						.isJsonObject()) {
			final JsonObject policingProfileJsonObject = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.POLICINGPROFILE);
			setInvalidParameter(VtnServiceJsonConsts.PROFILE_NAME);

			// validation for mandatory key: prf_name
			if (policingProfileJsonObject
					.has(VtnServiceJsonConsts.PROFILE_NAME)
					&& policingProfileJsonObject.getAsJsonPrimitive(
							VtnServiceJsonConsts.PROFILE_NAME).getAsString() != null) {
				isValid = validator.isValidMaxLengthAlphaNum(
						policingProfileJsonObject.getAsJsonPrimitive(
								VtnServiceJsonConsts.PROFILE_NAME)
								.getAsString(), VtnServiceJsonConsts.LEN_32);
			}

		}
		LOG.trace("Complete PolicingProfileResourceValidator#validatePost()");
		return isValid;
	}

}
