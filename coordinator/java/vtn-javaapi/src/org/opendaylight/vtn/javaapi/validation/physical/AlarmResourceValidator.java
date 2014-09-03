/*
 * Copyright (c) 2012-2014 NEC Corporation
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
import org.opendaylight.vtn.javaapi.resources.physical.AlarmResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class AlarmResourceValidator validates Clear Alarms API.
 */

public class AlarmResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(AlarmResourceValidator.class.getName());

	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new alarm resource validator.
	 * 
	 * @param resource
	 *            the resource
	 */
	public AlarmResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri.
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start AlarmResourceValidator#validateUri()");
		boolean isValid = false;
		if (resource instanceof AlarmResource) {
			isValid = true;
			setListOpFlag(false);
		}
		LOG.trace("Complete AlarmResourceValidator#validateUri()");
		return isValid;
	}

	/*
	 * Validate request JSON for Clear Alarms API.
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start AlarmResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of AlarmResourceValidator");
		boolean isValid = false;
		try {
			isValid = validateUri();
			if (isValid && requestBody != null
					&& VtnServiceConsts.PUT.equals(method)) {
				isValid = validatePut(requestBody);
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
		LOG.trace("Complete AlarmResourceValidator#validate()");
	}

	/**
	 * Validate put request json for Clear Alarms API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * 
	 * @return true, if successful
	 */
	private boolean validatePut(final JsonObject requestBody) {
		LOG.trace("Start AlarmResourceValidator#validatePut()");
		boolean isValid = false;
		// validation for key: alarm_no(mandatory)
		setInvalidParameter(VtnServiceJsonConsts.ALARMNO);
		if (requestBody.has(VtnServiceJsonConsts.ALARMNO)
				&& requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.ALARMNO)
						.getAsString() != null
				&& !requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.ALARMNO)
						.getAsString().isEmpty()) {
			isValid = validator.isValidBigIntegerRangeString(new BigInteger(
					requestBody
							.getAsJsonPrimitive(VtnServiceJsonConsts.ALARMNO)
							.getAsString()), VtnServiceJsonConsts.BIG_VAL0,
					VtnServiceJsonConsts.BIG_VAL_9999999999999999999);
		} else {
			isValid = false;
		}
		LOG.trace("Complete AlarmResourceValidator#validatePut()");
		return isValid;
	}
}
