/*
 * Copyright (c) 2015 NEC Corporation
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
import org.opendaylight.vtn.javaapi.resources.logical.LabelResource;
import org.opendaylight.vtn.javaapi.resources.logical.LabelsResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class LabelResourceValidator validates request Json object for Label
 * API.
 */
public class LabelResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(LabelResourceValidator.class.getName());
	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new Label resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public LabelResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for Label API.
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start LabelResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.UNIFIED_NETWORK_NAME);
		if (resource instanceof LabelsResource
				&& ((LabelsResource) resource).getUnifiedNetworkName() != null
				&& !((LabelsResource) resource).getUnifiedNetworkName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((LabelsResource) resource).getUnifiedNetworkName(),
					VtnServiceJsonConsts.LEN_31);
			setListOpFlag(true);
		} else if (resource instanceof LabelResource
				&& ((LabelResource) resource).getUnifiedNetworkName() != null
				&& !((LabelResource) resource).getUnifiedNetworkName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((LabelResource) resource).getUnifiedNetworkName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.LABEL_NAME);
				if (((LabelResource) resource).getLabelName() != null
						&& !((LabelResource) resource).getLabelName().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((LabelResource) resource).getLabelName(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		}
		LOG.trace("Completed LabelResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request json for get, put and post method of Label API
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start LabelResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of LabelResourceValidator");
		boolean isValid = false;
		try {
			isValid = validateUri();
			if (isValid) {
				if (requestBody != null
						&& VtnServiceConsts.GET.equals(method)) {
					isValid = validator.isValidGet(requestBody, isListOpFlag());
					setInvalidParameter(validator.getInvalidParameter());
					updateOpParameterForList(requestBody);
				} else if (requestBody != null
						&& VtnServiceConsts.PUT.equals(method)) {
					isValid = validatePut(requestBody);
				} else if (requestBody != null
						&& VtnServiceConsts.POST.equals(method)) {
					isValid = validatePost(requestBody);
				}
			}
		} catch (final NumberFormatException e) {
			LOG.error(e, "Inside catch:NumberFormatException");
			if (method.equals(VtnServiceConsts.GET)) {
				setInvalidParameter(validator.getInvalidParameter());
			}
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
		LOG.trace("Complete LabelResourceValidator#validate()");
	}

	/**
	 * Validate put request Json object for Lable API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validatePut(final JsonObject requestBody) {
		LOG.trace("Start LabelResourceValidator#validatePut()");
		boolean isValid = true;
		setInvalidParameter(VtnServiceJsonConsts.LABEL);
		if (requestBody.has(VtnServiceJsonConsts.LABEL)
				&& requestBody.get(VtnServiceJsonConsts.LABEL).isJsonObject()) {
			final JsonObject lable = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.LABEL);

			// validation for key: max_count
			setInvalidParameter(VtnServiceJsonConsts.MAX_COUNT);
			if (lable.has(VtnServiceJsonConsts.MAX_COUNT)) {
				if (lable.get(VtnServiceJsonConsts.MAX_COUNT).isJsonPrimitive()) {
					isValid = validator.isValidRange(
							lable.getAsJsonPrimitive(
									VtnServiceJsonConsts.MAX_COUNT)
									.getAsString(), VtnServiceJsonConsts.VAL_1,
							VtnServiceJsonConsts.VAL_4000);
				} else {
					isValid = false;
				}
			}

			// validation for key: raising_threshold
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.RISING_THRESHOLD);
				if (lable.has(VtnServiceJsonConsts.RISING_THRESHOLD)) {
					if (lable.get(VtnServiceJsonConsts.RISING_THRESHOLD)
							.isJsonPrimitive()) {
						isValid = validator.isValidRange(
								lable.getAsJsonPrimitive(
										VtnServiceJsonConsts.RISING_THRESHOLD)
										.getAsString(),
								VtnServiceJsonConsts.VAL_1,
								VtnServiceJsonConsts.VAL_4000);
					} else {
						isValid = false;
					}
				}
			}

			// validation for key: falling_threshold
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.FALLING_THRESHOLD);
				if (lable.has(VtnServiceJsonConsts.FALLING_THRESHOLD)) {
					if (lable.get(VtnServiceJsonConsts.FALLING_THRESHOLD)
							.isJsonPrimitive()) {
						isValid = validator.isValidRange(
								lable.getAsJsonPrimitive(
										VtnServiceJsonConsts.FALLING_THRESHOLD)
										.getAsString(),
								VtnServiceJsonConsts.VAL_1,
								VtnServiceJsonConsts.VAL_4000);
					} else {
						isValid = false;
					}
				}
			}
		} else {
			isValid = false;
		}
		LOG.trace("Complete LabelResourceValidator#validatePut()");
		return isValid;
	}

	/**
	 * Validate post request Json object for Label API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validatePost(final JsonObject requestBody) {
		LOG.trace("Start LabelResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.LABEL);
		if (requestBody.has(VtnServiceJsonConsts.LABEL)
				&& requestBody.get(VtnServiceJsonConsts.LABEL).isJsonObject()) {
			final JsonObject lable = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.LABEL);
			// validation for mandatory key: label_name
			setInvalidParameter(VtnServiceJsonConsts.LABEL_NAME);
			if (lable.has(VtnServiceJsonConsts.LABEL_NAME)
					&& lable.get(VtnServiceJsonConsts.LABEL_NAME)
							.isJsonPrimitive()) {
				String labelName = lable.getAsJsonPrimitive(
						VtnServiceJsonConsts.LABEL_NAME).getAsString();
				if (!labelName.isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(labelName,
							VtnServiceJsonConsts.LEN_31);
				}
			}

			if (isValid) {
				isValid = validatePut(requestBody);
			}
		}
		LOG.trace("Complete LabelResourceValidator#validatePost()");
		return isValid;
	}
}
