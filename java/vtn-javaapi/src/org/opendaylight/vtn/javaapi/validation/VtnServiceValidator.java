/*
 * Copyright (c) 2012-2013 NEC Corporation
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
 * The Class VtnServiceValidator.
 */
public abstract class VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(VtnServiceValidator.class.getName());

	private String invalidParameter = null;
	private boolean listOpFlag;

	/**
	 * Sets the list op flag.
	 * 
	 * @param listOpFlag
	 *            the new list op flag
	 */
	protected void setListOpFlag(final boolean listOpFlag) {
		this.listOpFlag = listOpFlag;
	}

	/**
	 * Checks if is list op flag.
	 * 
	 * @return true, if is list op flag
	 */
	public boolean isListOpFlag() {
		return listOpFlag;
	}

	/**
	 * Update op parameter for list.
	 * 
	 * @param requestBody
	 *            the request body
	 */
	public void updateOpParameterForList(final JsonObject requestBody) {
		if (listOpFlag && !requestBody.has(VtnServiceJsonConsts.OP)) {
			requestBody.addProperty(VtnServiceJsonConsts.OP,
					VtnServiceJsonConsts.NORMAL);
		}
	}

	/**
	 * Gets the invalid parameter.
	 * 
	 * @return the invalid parameter
	 */
	public String getInvalidParameter() {
		return invalidParameter;
	}

	/**
	 * Sets the invalid parameter.
	 * 
	 * @param invalidParameter
	 *            the new invalid parameter
	 */
	public void setInvalidParameter(final String invalidParameter) {
		this.invalidParameter = invalidParameter;
	}

	/**
	 * Default implementation of Validate method.
	 * 
	 * @param method
	 *            the method
	 * @param requestBody
	 *            the request body
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	public void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Return from VtnServiceValidator#validate");
	}

	/**
	 * Default implementation of Validate method.
	 * 
	 * @param method
	 *            the method called
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 * 
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	public void validate(final String method, final AbstractResource resource)
			throws VtnServiceException {
		LOG.trace("Return from VtnServiceValidator#validate");
		boolean isValid = false;
		if (VtnServiceConsts.GET.equals(method)
				|| VtnServiceConsts.DELETE.equals(method)) {
			isValid = resource.getValidator().validateUri();
		}
		// Throws exception if validation fails
		if (!isValid) {
			LOG.error("Validation failed");
			throw new VtnServiceException(Thread.currentThread()
					.getStackTrace()[1].getMethodName(),
					UncJavaAPIErrorCode.VALIDATION_ERROR.getErrorCode(),
					UncJavaAPIErrorCode.VALIDATION_ERROR.getErrorMessage());
		}
		LOG.info("URI Validation successful");
	}

	/**
	 * Default implementation of Validate uri method.
	 * 
	 * @return true, if successful
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	public boolean validateUri() throws VtnServiceException {
		LOG.trace("Return from VtnServiceValidator#validate");
		return true;
	}
}
