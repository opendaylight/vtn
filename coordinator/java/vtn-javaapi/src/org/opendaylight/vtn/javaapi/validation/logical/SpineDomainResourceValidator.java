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
import org.opendaylight.vtn.javaapi.resources.logical.SpineDomainResource;
import org.opendaylight.vtn.javaapi.resources.logical.SpineDomainsResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class SpineDomainResourceValidator validates request Json object for
 * Spine domain API.
 */
public class SpineDomainResourceValidator extends VtnServiceValidator {

	/**
	 * logger for debugging.
	 */
	private static final Logger LOG = Logger
			.getLogger(SpineDomainResourceValidator.class.getName());
	/**
	 * Abstract resource.
	 */
	private final AbstractResource resource;

	/**
	 * common validations.
	 */
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new Spine domain resource validator.
	 * 
	 * @param mappingResource
	 *            the instance of AbstractResource
	 */
	public SpineDomainResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for Spine domain API.
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start SpineDomainResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.UNIFIED_NETWORK_NAME);
		if (resource instanceof SpineDomainResource
				&& ((SpineDomainResource) resource).getUnifiedNetworkName() != null
				&& !((SpineDomainResource) resource).getUnifiedNetworkName()
						.isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((SpineDomainResource) resource).getUnifiedNetworkName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.SPINE_DOMAIN_NAME);
				if (((SpineDomainResource) resource).getSpineDomainName() != null
						&& !((SpineDomainResource) resource)
								.getSpineDomainName().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((SpineDomainResource) resource)
									.getSpineDomainName(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		} else if (resource instanceof SpineDomainsResource
				&& ((SpineDomainsResource) resource).getUnifiedNetworkName() != null
				&& !((SpineDomainsResource) resource).getUnifiedNetworkName()
						.isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((SpineDomainsResource) resource).getUnifiedNetworkName(),
					VtnServiceJsonConsts.LEN_31);
			setListOpFlag(true);
		}
		LOG.trace("Complete SpineDomainResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request Json object for get, put and post method of Spine domain
	 * API.
	 */
	/**
	 * @param method
	 *            , to get type of method
	 * @param requestBody
	 *            , for request
	 * @throws VtnServiceException
	 *             , for vtn exception
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start SpineDomainResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of SpineDomainResourceValidator");
		boolean isValid = false;
		try {
			isValid = validateUri();
			if (isValid && requestBody != null
					&& VtnServiceConsts.GET.equals(method)) {
				isValid = validateGet(requestBody);
				setInvalidParameter(validator.getInvalidParameter());
				updateOpParameterForList(requestBody);
			} else if (isValid && requestBody != null
					&& VtnServiceConsts.PUT.equals(method)) {
				isValid = validatePut(requestBody);
			} else if (isValid && requestBody != null
					&& VtnServiceConsts.POST.equals(method)) {
				isValid = validatePost(requestBody);
			}
		} catch (final NumberFormatException e) {
			if (method.equals(VtnServiceConsts.GET)) {
				setInvalidParameter(validator.getInvalidParameter());
			}
			LOG.error(e, "Inside catch:NumberFormatException");
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
		LOG.trace("Complete SpineDomainResourceValidator#validate()");

	}

	/**
	 * Validate get request Json object for Spine domain API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validateGet(final JsonObject requestBody) {
		boolean isValid = false;
		isValid = validator.isValidGet(requestBody, isListOpFlag());
		setInvalidParameter(validator.getInvalidParameter());
		return isValid;
	}

	/**
	 * validate post request Json object for Spine domain API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	private boolean validatePost(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start SpineDomainResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.SPINE_DOMAIN);
		if (requestBody.has(VtnServiceJsonConsts.SPINE_DOMAIN)
				&& requestBody.get(VtnServiceJsonConsts.SPINE_DOMAIN)
						.isJsonObject()) {
			final JsonObject spineDomainResourceEntry = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.SPINE_DOMAIN);
			// validation for key: spine_domain_name(mandatory)
			setInvalidParameter(VtnServiceJsonConsts.SPINE_DOMAIN_NAME);
			if (spineDomainResourceEntry.has(VtnServiceJsonConsts.SPINE_DOMAIN_NAME)
					&& spineDomainResourceEntry.get(
							VtnServiceJsonConsts.SPINE_DOMAIN_NAME)
							.isJsonPrimitive()
					&& !spineDomainResourceEntry.getAsJsonPrimitive(
							VtnServiceJsonConsts.SPINE_DOMAIN_NAME)
							.getAsString().isEmpty()) {
				isValid = validator.isValidMaxLengthAlphaNum(
						spineDomainResourceEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.SPINE_DOMAIN_NAME)
								.getAsString(), VtnServiceJsonConsts.LEN_31);
			}
			// validation for key: controller_id(mandatory)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.CONTROLLERID);
				if (spineDomainResourceEntry.has(VtnServiceJsonConsts.CONTROLLERID)
						&& spineDomainResourceEntry.get(VtnServiceJsonConsts.CONTROLLERID)
								.isJsonPrimitive()
						&& !spineDomainResourceEntry
								.getAsJsonPrimitive(VtnServiceJsonConsts.CONTROLLERID)
								.getAsString().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
									spineDomainResourceEntry.getAsJsonPrimitive(
											VtnServiceJsonConsts.CONTROLLERID).getAsString(),
									VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			// validation for key: domain_id(mandatory)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.DOMAINID);
				if (spineDomainResourceEntry.has(VtnServiceJsonConsts.DOMAINID)
						&& spineDomainResourceEntry.get(VtnServiceJsonConsts.DOMAINID).isJsonPrimitive()
						&& !spineDomainResourceEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.DOMAINID).getAsString().isEmpty()) {
					isValid = validator.isValidDomainId(spineDomainResourceEntry
									.getAsJsonPrimitive(VtnServiceJsonConsts.DOMAINID)
											.getAsString(),
									VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			// validation for key: label_name(option)
			if (isValid) {
				if (spineDomainResourceEntry.has(VtnServiceJsonConsts.LABEL_NAME)) {
					setInvalidParameter(VtnServiceJsonConsts.LABEL_NAME);
					if (spineDomainResourceEntry.get(VtnServiceJsonConsts.LABEL_NAME).isJsonPrimitive()) {
						isValid = validator.isValidMaxLengthAlphaNum(spineDomainResourceEntry
										.getAsJsonPrimitive(VtnServiceJsonConsts.LABEL_NAME).getAsString(),
									VtnServiceJsonConsts.LEN_31);
					} else {
						isValid = false;
					}
				}
			}
		} else {
			isValid = false;
		}
		LOG.trace("Complete SpineDomainResourceValidator#validatePost()");
		return isValid;
	}

	/**
	 * validate put request Json object for Spine domain API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	private boolean validatePut(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start SpineDomainResourceValidator#validatePut()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.SPINE_DOMAIN);
		if (requestBody.has(VtnServiceJsonConsts.SPINE_DOMAIN)
				&& requestBody.get(VtnServiceJsonConsts.SPINE_DOMAIN)
						.isJsonObject()) {
			final JsonObject spineDomainResourceEntry = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.SPINE_DOMAIN);
			isValid = true;
			if (spineDomainResourceEntry.has(VtnServiceJsonConsts.LABEL_NAME)) {
				// validation for key: spine_domain_name(option)
				setInvalidParameter(VtnServiceJsonConsts.LABEL_NAME);
				if (spineDomainResourceEntry.get(
						VtnServiceJsonConsts.LABEL_NAME).isJsonPrimitive()) {
					isValid = validator.isValidMaxLengthAlphaNum(spineDomainResourceEntry
										.getAsJsonPrimitive(VtnServiceJsonConsts.LABEL_NAME)
											.getAsString(),
									VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
		}
		LOG.trace("Complete SpineDomainResourceValidator#validatePut()");
		return isValid;
	}
}
