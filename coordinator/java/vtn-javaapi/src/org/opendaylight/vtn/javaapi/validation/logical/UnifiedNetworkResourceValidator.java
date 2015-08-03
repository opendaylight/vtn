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
import org.opendaylight.vtn.javaapi.resources.logical.UnifiedNetworkResource;
import org.opendaylight.vtn.javaapi.resources.logical.UnifiedNetworksResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class UnifiedNetworkResourceValidator validates request Json
 * object for UnifiedNetwork and UnifiedNetworks API.
 */
public class UnifiedNetworkResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(UnifiedNetworkResourceValidator.class.getName());

	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new unified network resource validator.
	 * 
	 * @param resource the instance of AbstractResource
	 * 
	 */
	public UnifiedNetworkResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for UnifiedNetwork API
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start UnifiedNetworkResourceValidator#validateUri()");
		
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.UNIFIED_NW_NAME);
		
		if (resource instanceof UnifiedNetworkResource
				&& ((UnifiedNetworkResource) resource).getUnifiedNetworkName() != null
				&& !((UnifiedNetworkResource) resource).getUnifiedNetworkName().isEmpty()) {
			
			isValid = validator.isValidMaxLengthAlphaNum(
					((UnifiedNetworkResource) resource).getUnifiedNetworkName(),
					VtnServiceJsonConsts.LEN_31);
			
			setListOpFlag(false);
		} else if (resource instanceof UnifiedNetworksResource) {
			
			isValid = true;
			setListOpFlag(true);
		}
		
		LOG.trace("Complete UnifiedNetworkResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request Json object for get and post method of UnifiedNetwork
	 * and UnifiedNetworks API
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start UnifiedNetworkResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of UnifiedNetworkResourceValidator");
		
		boolean isValid = false;
		
		try {
			
			isValid = validateUri();
			if (isValid && requestBody != null
					&& VtnServiceConsts.GET.equals(method)) {
				isValid = validator.isValidGet(requestBody, isListOpFlag());
				setInvalidParameter(validator.getInvalidParameter());
				updateOpParameterForList(requestBody);
			} else if (isValid && requestBody != null
					&& VtnServiceConsts.POST.equalsIgnoreCase(method)) {
				isValid = validatePost(requestBody);
			} else if (isValid && VtnServiceConsts.PUT.equals(method)) {
				setInvalidParameter(VtnServiceConsts.INCORRECT_METHOD_INVOCATION);
				isValid = false;
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
		LOG.trace("Complete UnifiedNetworkResourceValidator#validate()");
	}

	/**
	 * Validate post request Json object for UnifiedNetworks API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validatePost(final JsonObject requestBody) {
		LOG.trace("Start UnifiedNetworkResourceValidator#validatePost()");
		
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.UNIFIED_NW);		
		if (requestBody.has(VtnServiceJsonConsts.UNIFIED_NW)
				&& requestBody.get(VtnServiceJsonConsts.UNIFIED_NW).isJsonObject()) {
			
			final JsonObject unifiedNw = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.UNIFIED_NW);
			
			setInvalidParameter(VtnServiceJsonConsts.UNIFIED_NW_NAME);
			// validation for mandatory key: unified_network_name
			if (unifiedNw.has(VtnServiceJsonConsts.UNIFIED_NW_NAME)
					&& unifiedNw.get(VtnServiceJsonConsts.UNIFIED_NW_NAME).isJsonPrimitive()
					&& !unifiedNw.getAsJsonPrimitive(VtnServiceJsonConsts.UNIFIED_NW_NAME)
							.getAsString().isEmpty()) {
				
				isValid = validator.isValidMaxLengthAlphaNum(unifiedNw
						.getAsJsonPrimitive(VtnServiceJsonConsts.UNIFIED_NW_NAME)
						.getAsString(), VtnServiceJsonConsts.LEN_31);
			}
			
			if (isValid) {
				// validation for mandatory key: routing_type
				setInvalidParameter(VtnServiceJsonConsts.ROUTING_TYPE);
				if (unifiedNw.has(VtnServiceJsonConsts.ROUTING_TYPE)
						&& unifiedNw.get(VtnServiceJsonConsts.ROUTING_TYPE).isJsonPrimitive()
						&& !unifiedNw.getAsJsonPrimitive(VtnServiceJsonConsts.ROUTING_TYPE)
						.getAsString().isEmpty()) {

					isValid = VtnServiceJsonConsts.ONE.equals(
								unifiedNw.getAsJsonPrimitive(
								VtnServiceJsonConsts.ROUTING_TYPE).getAsString());
				} else {
					isValid = false;
				}
			}
		}
		
		LOG.trace("Complete UnifiedNetworkResourceValidator#validatePost()");
		return isValid;
	}
}
