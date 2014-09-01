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
import org.opendaylight.vtn.javaapi.resources.logical.ArpEntryResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class ArpEntryResourceValidator validates request Json object for Arp
 * Entry API.
 */
public class ArpEntryResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(ArpEntryResourceValidator.class.getName());

	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new arp entry resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public ArpEntryResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate URI parameters for ArpEntry API
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start ArpEntryResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.VTNNAME);
		if (resource instanceof ArpEntryResource
				&& ((ArpEntryResource) resource).getVtnName() != null
				&& !((ArpEntryResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((ArpEntryResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VRTNAME);
				if (((ArpEntryResource) resource).getVrtName() != null
						&& !((ArpEntryResource) resource).getVrtName()
								.isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((ArpEntryResource) resource).getVrtName(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		}
		LOG.trace("Complete ArpEntryResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request Json object for get method of ArpEntry API
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start ArpEntryResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of ArpEntryResourceValidator");
		boolean isValid = false;
		isValid = validateUri();
		if (isValid && requestBody != null
				&& VtnServiceConsts.GET.equals(method)) {
			isValid = validateGet(requestBody);
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
		LOG.trace("Complete ArpEntryResourceValidator#validate()");
	}

	/**
	 * Validate get request Json object for ArpEntry API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	private boolean validateGet(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start ArpEntryResourceValidator#validateGet()");
		boolean isValid = true;
		// validation for key: op(optional)
		setInvalidParameter(VtnServiceJsonConsts.OP);
		isValid = validator.isValidOperationForCount(requestBody);
		// validation for key: type(optional)
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.TYPE);
			if (requestBody.has(VtnServiceJsonConsts.TYPE)
					&& requestBody
							.getAsJsonPrimitive(VtnServiceJsonConsts.TYPE)
							.getAsString() != null) {
				final String type = requestBody.getAsJsonPrimitive(
						VtnServiceJsonConsts.TYPE).getAsString();
				isValid = type.equalsIgnoreCase(VtnServiceJsonConsts.STATIC)
						|| type.equalsIgnoreCase(VtnServiceJsonConsts.DYNAMIC);

			}
		}
		LOG.trace("Complete ArpEntryResourceValidator#validateGet()");
		return isValid;
	}
}
