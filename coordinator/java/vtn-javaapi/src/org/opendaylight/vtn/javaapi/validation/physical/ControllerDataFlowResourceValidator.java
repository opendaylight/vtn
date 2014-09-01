/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi.validation.physical;

import java.math.BigInteger;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.resources.physical.ControllerDataFlowResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class DataFlowResourceValidator validates .
 */
public class ControllerDataFlowResourceValidator extends VtnServiceValidator {
	/**
	 * . Logger is used for debugging purpose
	 */
	private static final Logger LOG = Logger
			.getLogger(BoundaryResourceValidator.class.getName());
	/**
	 * . AbsractResouce class reference
	 */
	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new Controller Data Flow resource Validator. the instance
	 * of AbstractResource.
	 * 
	 * @param mappingResource
	 *            ,AbstarctResouce reference assigned to derived class
	 */
	public ControllerDataFlowResourceValidator(
			final AbstractResource mappingResource) {
		this.resource = mappingResource;
	}

	/**
	 * Validate uri for ControllerDataFlowResource API.
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start ControllerDataFlowResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.CONTROLLERID);
		if (resource instanceof ControllerDataFlowResource
				&& ((ControllerDataFlowResource) resource).getControllerId() != null
				&& !((ControllerDataFlowResource) resource).getControllerId()
						.isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((ControllerDataFlowResource) resource).getControllerId(),
					VtnServiceJsonConsts.LEN_31);
			setListOpFlag(true);
		}

		LOG.trace("Complete ControllerDataFlowResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request json for Controller Data Flow APIs.
	 * 
	 * @param method
	 *            , contains info about get,post ,delete.
	 * @param requestBody
	 *            , contains request parameter.
	 * @throws VtnServiceException
	 *             , vtn exception is thrown.
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start ControllerDataFlowResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of ControllerDataFlowResourceValidator");
		boolean isValid = false;
		try {
			isValid = validateUri();
			if (isValid && requestBody != null
					&& VtnServiceConsts.GET.equals(method)) {
				isValid = isValidateGet(requestBody, isListOpFlag());
				updateOpParameterForList(requestBody);
			} else if (isValid) {
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
		LOG.trace("Complete ControllerDataFlowResourceValidator#validate()");
	}

	/**
	 * Validate request json for get method of Controller Data Flow API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 * @param opFlag
	 *            , operation type
	 */
	private boolean isValidateGet(final JsonObject requestBody,
			final boolean opFlag) {
		LOG.trace("Start ControllerDataFlowResourceValidator#isValidateGet()");
		boolean isValid = true;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.CONTROLLERID);
		if (resource instanceof ControllerDataFlowResource
				&& ((ControllerDataFlowResource) resource).getControllerId() != null
				&& !((ControllerDataFlowResource) resource).getControllerId()
						.isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((ControllerDataFlowResource) resource).getControllerId(),
					VtnServiceJsonConsts.LEN_31);
			setListOpFlag(true);
		}
		setInvalidParameter(VtnServiceJsonConsts.CONTROLLERID);
		if (requestBody.has(VtnServiceJsonConsts.CONTROLLERID)
				&& requestBody.getAsJsonPrimitive(
						VtnServiceJsonConsts.CONTROLLERID).getAsString() != null) {
			isValid = validator.isValidMaxLengthAlphaNum(requestBody
					.getAsJsonPrimitive(VtnServiceJsonConsts.CONTROLLERID)
					.getAsString(), VtnServiceJsonConsts.LEN_31);
		}

		setInvalidParameter(VtnServiceJsonConsts.FLOW_ID);
		if (requestBody.has(VtnServiceJsonConsts.FLOW_ID)
				&& requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.FLOW_ID)
						.getAsString() != null) {
			isValid = validator.isValidBigIntegerRangeString(new BigInteger(
					requestBody
							.getAsJsonPrimitive(VtnServiceJsonConsts.FLOW_ID)
							.getAsString()), VtnServiceJsonConsts.BIG_VAL0,
					VtnServiceJsonConsts.BIG_VAL_18446744073709551615);
		} else {
			isValid = false;
		}

		LOG.trace("Complete ControllerDataFlowResourceValidator#isValidateGet");
		return isValid;
	}
}
