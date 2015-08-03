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
import org.opendaylight.vtn.javaapi.resources.logical.SpineDomainFdbResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class SpineDomainFdbResourceValidator validates request Json object for
 * SpineDomainFdbResource API.
 */
public class SpineDomainFdbResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(SpineDomainFdbResourceValidator.class.getName());

	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new spine domain fdb resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public SpineDomainFdbResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate URI parameters for SpineDomainFdbResource API
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start SpineDomainFdbResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.UNIFIED_NETWORK_NAME);
		if (resource instanceof SpineDomainFdbResource
				&& ((SpineDomainFdbResource) resource).getUnifiedNetworkName() != null
				&& !((SpineDomainFdbResource) resource).getUnifiedNetworkName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((SpineDomainFdbResource) resource).getUnifiedNetworkName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.SPINE_DOMAIN_NAME);
				if (((SpineDomainFdbResource) resource).getSpineDomainName() != null
						&& !((SpineDomainFdbResource) resource).getSpineDomainName()
								.isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((SpineDomainFdbResource) resource).getSpineDomainName(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		}
		LOG.trace("Complete SpineDomainFdbResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request Json object for get method of SpineDomainFdbResource API
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start SpineDomainFdbResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of SpineDomainFdbResourceValidator");
		boolean isValid = false;
		isValid = validateUri();
		if (isValid && requestBody != null
				&& VtnServiceConsts.GET.equals(method)) {
			isValid = true;
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
		LOG.trace("Complete SpineDomainFdbResourceValidator#validate()");
	}
}
