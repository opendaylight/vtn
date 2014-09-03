/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.openstack.validation;

import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * Destination Controller Validation Resource class. Contains methods to
 * validate URI, parameters of PUT request body
 * 
 */
public class DestinationControllerResourceValidator extends VtnServiceValidator {

	/* Logger instance */
	private static final Logger LOG = Logger
			.getLogger(DestinationControllerResourceValidator.class.getName());

	/* instance for common validation operation */
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Constructor that provide reference of actual Resource class to instance
	 * variable resource
	 * 
	 * @param resource
	 *            - Resource class reference
	 */
	public DestinationControllerResourceValidator(
			final AbstractResource resource) {
		LOG.debug("No use of resource instance : " + resource);
	}

	/**
	 * Calls the respective validation method according to type of method
	 * 
	 * @see org.opendaylight.vtn.javaapi.validation.VtnServiceValidator#validate
	 *      (java.lang.String, com.google.gson.JsonObject)
	 */
	@Override
	public void validate(String method, JsonObject requestBody)
			throws VtnServiceException {
		LOG.info("Start DestinationControllerResourceValidator#validate()");
		boolean isValid = true;
		try {
			if (requestBody != null) {
				if (VtnServiceConsts.PUT.equals(method)) {
					isValid = validatePut(requestBody);
				} else if (VtnServiceConsts.GET.equals(method)) {
					isValid = true;
				} else {
					setInvalidParameter(UncCommonEnum.UncResultCode.UNC_METHOD_NOT_ALLOWED
							.getMessage());
					isValid = false;
				}
			} else {
				isValid = false;
				setInvalidParameter(UncCommonEnum.UncResultCode.UNC_INVALID_FORMAT
						.getMessage());
			}
		} catch (final NumberFormatException e) {
			LOG.error(e, "Invalid value : " + e.getMessage());
			isValid = false;
		} catch (final ClassCastException e) {
			LOG.error(e, "Invalid type : " + e.getMessage());
			isValid = false;
		}

		/*
		 * throw exception in case of validation fail
		 */
		if (!isValid) {
			LOG.error("Validation failure");
			throw new VtnServiceException(Thread.currentThread()
					.getStackTrace()[1].getMethodName(),
					UncJavaAPIErrorCode.VALIDATION_ERROR.getErrorCode(),
					UncJavaAPIErrorCode.VALIDATION_ERROR.getErrorMessage());
		}
		LOG.info("Complete DestinationControllerResourceValidator#validate()");
	}

	/**
	 * Validates the parameters of PUT request body
	 * 
	 * @param requestBody
	 *            - JSON request body corresponding to PUT operation
	 * @return - validation status as true or false
	 */
	private boolean validatePut(JsonObject requestBody) {
		LOG.trace("Start DestinationControllerResourceValidator#validatePut()");
		boolean isValid = true;
		// validation of id
		if (requestBody.has(VtnServiceOpenStackConsts.ID)) {
			final JsonElement id = requestBody
					.get(VtnServiceOpenStackConsts.ID);
			if (id.isJsonNull()
					|| id.getAsString().isEmpty()
					|| !validator.isValidMaxLengthAlphaNum(id.getAsString(),
							VtnServiceJsonConsts.LEN_31)) {
				isValid = false;
				setInvalidParameter(VtnServiceOpenStackConsts.ID
						+ VtnServiceConsts.COLON
						+ (id.isJsonNull() ? id : id.getAsString()));
			}
		} else {
			isValid = false;
			setInvalidParameter(UncCommonEnum.UncResultCode.UNC_INVALID_FORMAT
					.getMessage());
		}
		LOG.trace("Complete DestinationControllerResourceValidator#validatePut()");
		return isValid;
	}
}
