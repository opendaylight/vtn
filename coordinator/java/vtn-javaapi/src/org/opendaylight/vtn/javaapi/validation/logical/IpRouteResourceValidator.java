/*
 * Copyright (c) 2012-2014 NEC Corporation
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
import org.opendaylight.vtn.javaapi.resources.logical.IpRouteResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class IpRouteResourceValidator validates request Json object for IpRoute
 * API.
 */
public class IpRouteResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(IpRouteResourceValidator.class.getName());

	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new Ip Route resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public IpRouteResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for IpRoute API.
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start IpRouteResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.VTNNAME);
		if (resource instanceof IpRouteResource
				&& ((IpRouteResource) resource).getVtnName() != null
				&& !((IpRouteResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((IpRouteResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VROUTERNAME);
				if (((IpRouteResource) resource).getVrtName() != null
						&& !((IpRouteResource) resource).getVrtName().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((IpRouteResource) resource).getVrtName(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		}
		LOG.trace("Complete IpRouteResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request Json object for get method of IpRoute API.
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start IpRouteResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of IpRouteResourceValidator");
		boolean isValid = false;
		isValid = validateUri();
		if (isValid && requestBody != null
				&& VtnServiceConsts.GET.equals(method)) {
			isValid = ValidateGet(requestBody);
			updateOpParameterForList(requestBody);
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
		LOG.trace("Complete IpRouteResourceValidator#validate()");
	}

	/**
	 * Validate get request Json object for IpRoute API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean ValidateGet(final JsonObject requestBody) {
		LOG.trace("Start IpRouteResourceValidator#ValidateGet()");
		boolean isValid = true;
		// validation for key: op
		setInvalidParameter(VtnServiceJsonConsts.OP);
		isValid = validator.isValidOperationForCount(requestBody);
		LOG.trace("Complete IpRouteResourceValidator#ValidateGet()");
		return isValid;
	}
}
