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
import org.opendaylight.vtn.javaapi.resources.logical.VTNUnifiedNetworkResource;
import org.opendaylight.vtn.javaapi.resources.logical.VTNUnifiedNetworksResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class VTNUnifiedNetworkResourceValidator validates request Json object for VTN Unified Network API.
 */
public class VTNUnifiedNetworkResourceValidator extends VtnServiceValidator {

	/**
	 * logger for debugging.
	 */
	private static final Logger LOG = Logger
			.getLogger(VTNUnifiedNetworkResourceValidator.class.getName());
	/**
	 * Abstract resource.
	 */
	private final AbstractResource resource;

	/**
	 * common validations.
	 */
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new VTN Unified Network resource validator.
	 * 
	 * @param mappingResource
	 *            the instance of AbstractResource
	 */
	public VTNUnifiedNetworkResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for VTN Unified Network API.
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start VTNUnifiedNetworkResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.VTNNAME);
		if (resource instanceof VTNUnifiedNetworkResource
				&& ((VTNUnifiedNetworkResource) resource).getVtnName() != null
				&& !((VTNUnifiedNetworkResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VTNUnifiedNetworkResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.UNIFIED_NETWORK_NAME);
				if (((VTNUnifiedNetworkResource) resource).getUnifiedNetworkName() != null
						&& !((VTNUnifiedNetworkResource) resource).getUnifiedNetworkName().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VTNUnifiedNetworkResource) resource).getUnifiedNetworkName(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		} else if (resource instanceof VTNUnifiedNetworksResource
				&& ((VTNUnifiedNetworksResource) resource).getVtnName() != null
				&& !((VTNUnifiedNetworksResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VTNUnifiedNetworksResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			setListOpFlag(true);
		}
		LOG.trace("Complete VTNUnifiedNetworkResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request Json object for get, put and post method of VTN Unified Network API.
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
		LOG.trace("Start VTNUnifiedNetworkResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of VTNUnifiedNetworkResourceValidator");
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
		LOG.trace("Complete VTNUnifiedNetworkResourceValidator#validate()");

	}

	/**
	 * Validate get request Json object for VTN Unified Network API.
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
	 * validate post request Json object for VTN Unified Network API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	private boolean validatePost(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VTNUnifiedNetworkResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.UNIFIED_NW);
		if (requestBody.has(VtnServiceJsonConsts.UNIFIED_NW)
				&& requestBody.get(VtnServiceJsonConsts.UNIFIED_NW).isJsonObject()) {
			final JsonObject vtnUnifiedNetworkResourceEntry = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.UNIFIED_NW);
			// validation for key: unified_network_name(mandatory)
			setInvalidParameter(VtnServiceJsonConsts.UNIFIED_NETWORK_NAME);
			if (vtnUnifiedNetworkResourceEntry.has(VtnServiceJsonConsts.UNIFIED_NETWORK_NAME)
					&& vtnUnifiedNetworkResourceEntry.get(VtnServiceJsonConsts.UNIFIED_NETWORK_NAME).isJsonPrimitive()
					&& !vtnUnifiedNetworkResourceEntry
							.getAsJsonPrimitive(VtnServiceJsonConsts.UNIFIED_NETWORK_NAME)
							.getAsString().isEmpty()) {
				isValid = validator.isValidMaxLengthAlphaNum(vtnUnifiedNetworkResourceEntry
						.getAsJsonPrimitive(VtnServiceJsonConsts.UNIFIED_NETWORK_NAME)
						.getAsString(), VtnServiceJsonConsts.LEN_31);
			}
			if (isValid && vtnUnifiedNetworkResourceEntry.has(VtnServiceJsonConsts.SPINE_DOMAIN_NAME)) {
				// validation for key: spine_domain_name(option)
				setInvalidParameter(VtnServiceJsonConsts.SPINE_DOMAIN_NAME);
				if (vtnUnifiedNetworkResourceEntry.get(VtnServiceJsonConsts.SPINE_DOMAIN_NAME).isJsonPrimitive()) {
					isValid = validator.isValidMaxLengthAlphaNum(vtnUnifiedNetworkResourceEntry
									.getAsJsonPrimitive(VtnServiceJsonConsts.SPINE_DOMAIN_NAME)
									.getAsString(),
									VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
		}
		LOG.trace("Complete VTNUnifiedNetworkResourceValidator#validatePost()");
		return isValid;
	}

	/**
	 * validate put request Json object for VTN Unified Network API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	private boolean validatePut(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VTNUnifiedNetworkResourceValidator#validatePut()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.UNIFIED_NW);
		if (requestBody.has(VtnServiceJsonConsts.UNIFIED_NW)
				&& requestBody.get(VtnServiceJsonConsts.UNIFIED_NW).isJsonObject()) {
			final JsonObject vtnUnifiedNetworkResourceEntry = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.UNIFIED_NW);
			// validation for key: spine_domain_name(option)
			isValid = true;
			if (vtnUnifiedNetworkResourceEntry.has(VtnServiceJsonConsts.SPINE_DOMAIN_NAME)) {
				setInvalidParameter(VtnServiceJsonConsts.SPINE_DOMAIN_NAME);
			    if (vtnUnifiedNetworkResourceEntry.get(VtnServiceJsonConsts.SPINE_DOMAIN_NAME).isJsonPrimitive()) {
			    	isValid = validator.isValidMaxLengthAlphaNum(vtnUnifiedNetworkResourceEntry
						.getAsJsonPrimitive(VtnServiceJsonConsts.SPINE_DOMAIN_NAME)
						.getAsString(), VtnServiceJsonConsts.LEN_31);
			    } else {
			    	isValid = false;
				}
			}
		}
		LOG.trace("Complete VTNUnifiedNetworkResourceValidator#validatePut()");
		return isValid;
	}
}
