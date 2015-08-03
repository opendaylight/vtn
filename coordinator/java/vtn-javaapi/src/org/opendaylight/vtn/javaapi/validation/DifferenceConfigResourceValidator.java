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

public class DifferenceConfigResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(DifferenceConfigResourceValidator.class.getName());

	public DifferenceConfigResourceValidator(final AbstractResource resource) {

	}

	/**
	 * Validate request Json for UNC Difference Config API.
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start DifferenceConfigResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of DifferenceConfigResourceValidator");
		boolean isValid = false;
		if (requestBody != null
				&& VtnServiceConsts.GET.equals(method)) {
			isValid = validateGet(requestBody);
			setListOpFlag(false);
		} else {
			setInvalidParameter(VtnServiceConsts.INCORRECT_METHOD_INVOCATION);
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
		LOG.trace("Complete DifferenceConfigResourceValidator#validate()");
	}

	/**
	 * Validate get request Json for Difference Config API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validateGet(final JsonObject queryString) {
		LOG.trace("Start DifferenceConfigResourceValidator#validateGet()");
		boolean isValid = false;
		// validation for key: mode_type
		setInvalidParameter(VtnServiceJsonConsts.MODE_TYPE);
		if (queryString.has(VtnServiceJsonConsts.MODE_TYPE)
			&& queryString.get(VtnServiceJsonConsts.MODE_TYPE).isJsonPrimitive()) {
			String modeType = queryString.getAsJsonPrimitive(
					VtnServiceJsonConsts.MODE_TYPE).getAsString();
			if (VtnServiceJsonConsts.GLOBAL_MODE.equals(modeType) ||
				VtnServiceJsonConsts.REAL_MODE.equals(modeType) ||
				VtnServiceJsonConsts.VIRTUAL_MODE.equals(modeType) ||
				VtnServiceJsonConsts.VTN_MODE.equals(modeType)) {
				isValid = true;
			}
		}
		LOG.trace("Complete DifferenceConfigResourceValidator#validateGet()");
		return isValid;
	}
}
