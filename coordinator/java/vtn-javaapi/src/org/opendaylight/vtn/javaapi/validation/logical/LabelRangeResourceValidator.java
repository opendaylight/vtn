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
import org.opendaylight.vtn.javaapi.resources.logical.LabelRangeResource;
import org.opendaylight.vtn.javaapi.resources.logical.LabelRangesResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class LabelRangeResourceValidator validates request Json object for Label
 * Range API.
 */
public class LabelRangeResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(LabelRangeResourceValidator.class.getName());
	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new Label Range resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public LabelRangeResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for Label Range API.
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start LabelRangeResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.UNIFIED_NETWORK_NAME);
		if (resource instanceof LabelRangesResource
				&& ((LabelRangesResource) resource).getUnifiedNetworkName() != null
				&& !((LabelRangesResource) resource).getUnifiedNetworkName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((LabelRangesResource) resource).getUnifiedNetworkName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.LABEL_NAME);
				if (((LabelRangesResource) resource).getLabelName() != null
						&& !((LabelRangesResource) resource).getLabelName()
								.isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
									((LabelRangesResource) resource)
											.getLabelName(),
									VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(true);
		} else if (resource instanceof LabelRangeResource
				&& ((LabelRangeResource) resource).getUnifiedNetworkName() != null
				&& !((LabelRangeResource) resource).getUnifiedNetworkName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((LabelRangeResource) resource).getUnifiedNetworkName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.LABEL_NAME);
				if (((LabelRangeResource) resource).getLabelName() != null
						&& !((LabelRangeResource) resource).getLabelName()
								.isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
									((LabelRangeResource) resource)
											.getLabelName(),
									VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.RANGE_ID);
				if (((LabelRangeResource) resource).getRangeId() != null
						&& !((LabelRangeResource) resource).getRangeId()
								.isEmpty()) {
					isValid = isValidRangeId(((LabelRangeResource) resource)
											.getRangeId());
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		}
		LOG.trace("Completed LabelRangeResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request json for get, put and post method of Label Range API
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start LabelRangeResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of LabelRangeResourceValidator");
		boolean isValid = false;
		try {
			isValid = validateUri();
			if (isValid) {
				if (requestBody != null && VtnServiceConsts.GET.equals(method)) {
					isValid = validGet(requestBody, isListOpFlag());
					setInvalidParameter(validator.getInvalidParameter());
					updateOpParameterForList(requestBody);
				} else if (requestBody != null
						&& VtnServiceConsts.POST.equals(method)) {
					isValid = validatePost(requestBody);
				} else if (requestBody != null
						&& VtnServiceConsts.PUT.equals(method)) {
					setInvalidParameter(VtnServiceConsts.INCORRECT_METHOD_INVOCATION);
					isValid = false;
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
		LOG.trace("Complete LabelRangeResourceValidator#validate()");
	}

	/**
	 * Validate post request Json object for Lanbel Range API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validatePost(final JsonObject requestBody) {
		LOG.trace("Start LabelRangeResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.RANGE);
		if (requestBody.has(VtnServiceJsonConsts.RANGE)
				&& requestBody.get(VtnServiceJsonConsts.RANGE).isJsonObject()) {
			final JsonObject range = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.RANGE);

			// validation for mandatory key: min
			setInvalidParameter(VtnServiceJsonConsts.RANGE_MIN);
			if (range.has(VtnServiceJsonConsts.RANGE_MIN)
					&& range.get(VtnServiceJsonConsts.RANGE_MIN).isJsonPrimitive()) {
				isValid = validator.isValidRange(range.getAsJsonPrimitive(
								VtnServiceJsonConsts.RANGE_MIN)
									.getAsString(),
								VtnServiceJsonConsts.VAL_1,
								VtnServiceJsonConsts.VAL_4000);
			} else {
				isValid = false;
			}

			// validation for mandatory key: max
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.RANGE_MAX);
				if (range.has(VtnServiceJsonConsts.RANGE_MAX)
						&& range.get(VtnServiceJsonConsts.RANGE_MAX).isJsonPrimitive()) {
					isValid = validator.isValidRange(range.getAsJsonPrimitive(
									VtnServiceJsonConsts.RANGE_MAX)
										.getAsString(),
									VtnServiceJsonConsts.VAL_1,
									VtnServiceJsonConsts.VAL_4000);
				} else {
					isValid = false;
				}
			}
		}
		LOG.trace("Complete LabelRangeResourceValidator#validatePost()");
		return isValid;
	}

	/**
	 * Checks if is valid get request.
	 * 
	 * @param requestBody
	 *            the request JSON object
	 * @return true, if is valid get
	 */
	private boolean validGet(final JsonObject requestBody,
			final boolean opFlag) {
		LOG.trace("Start LabelRangeResourceValidator#isValidGet");
		boolean isValid = true;
		// validation for key: tagetdb
		setInvalidParameter(VtnServiceJsonConsts.TARGETDB);
		isValid = validator.isValidRequestDB(requestBody);

		/*
		 * Remove unwanted parameters from request body for Show APIs
		 */
		if (!opFlag) {
			if (requestBody.has(VtnServiceJsonConsts.OP)) {
				requestBody.remove(VtnServiceJsonConsts.OP);
			} else {
				LOG.debug("No need to remove");
			}
			if (requestBody.has(VtnServiceJsonConsts.INDEX)) {
				requestBody.remove(VtnServiceJsonConsts.INDEX);
			} else {
				LOG.debug("No need to remove");
			}
			if (requestBody.has(VtnServiceJsonConsts.MAX)) {
				requestBody.remove(VtnServiceJsonConsts.MAX);
			} else {
				LOG.debug("No need to remove");
			}
		} else {
			// validation for key: op
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.OP);
				isValid = validator.isValidOperation(requestBody);
			}
			// validation for key: index
			if (isValid
					&& requestBody.has(VtnServiceJsonConsts.INDEX)
					&& requestBody.get(VtnServiceJsonConsts.INDEX).isJsonPrimitive()) {
				setInvalidParameter(VtnServiceJsonConsts.INDEX);
				isValid = isValidRangeId(
						requestBody.getAsJsonPrimitive(
								VtnServiceJsonConsts.INDEX).getAsString());
			}
			// validation for key: max_repitition
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.MAX);
				isValid = validator.isValidMaxRepetition(requestBody);
			}
		}
		LOG.trace("Complete LabelRangeResourceValidator#isValidGet");
		return isValid;
	}

	/**
	 * Checks if is valid range id.
	 * 
	 * @param input
	 *            the value to be validated
	 * @return true, if is valid range
	 */
	private boolean isValidRangeId(final String rangeId) {
		boolean isValid = false;
		if (!rangeId.isEmpty()) {
			String[] minAndMax = rangeId.split(VtnServiceConsts.UNDERSCORE);
			if (minAndMax.length == 2) {
				int min = Integer.parseInt(minAndMax[0]);
				int max = Integer.parseInt(minAndMax[1]);
				isValid = (min >= VtnServiceJsonConsts.VAL_1 &&
						min <= VtnServiceJsonConsts.VAL_4000);
				isValid = isValid ? (max >= VtnServiceJsonConsts.VAL_1 &&
						max <= VtnServiceJsonConsts.VAL_4000) : false;
			}
		}
		return isValid;
	}
}
