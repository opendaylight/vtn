/*
 * Copyright (c) 2012-2015 NEC Corporation
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
import org.opendaylight.vtn.javaapi.resources.logical.VBridgeExpandingResource;
import org.opendaylight.vtn.javaapi.resources.logical.VBridgeResource;
import org.opendaylight.vtn.javaapi.resources.logical.VBridgesResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class VBridgeResourceValidator validates request Json object for Vbridge
 * API.
 */
public class VBridgeResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(VBridgeResourceValidator.class.getName());
	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new Vbridge resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public VBridgeResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for VBridge API.
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start VBridgeResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.VTNNAME);
		if (resource instanceof VBridgesResource
				&& ((VBridgesResource) resource).getVtnName() != null
				&& !((VBridgesResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VBridgesResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			setListOpFlag(true);
		} else if (resource instanceof VBridgeResource
				&& ((VBridgeResource) resource).getVtnName() != null
				&& !((VBridgeResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VBridgeResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VBRNAME);
				if (((VBridgeResource) resource).getVbrName() != null
						&& !((VBridgeResource) resource).getVbrName().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VBridgeResource) resource).getVbrName(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		} else if (resource instanceof VBridgeExpandingResource
				&& ((VBridgeExpandingResource) resource).getVtnName() != null
				&& !((VBridgeExpandingResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VBridgeExpandingResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VBRNAME);
				if (((VBridgeExpandingResource) resource).getVbrName() != null
						&& !((VBridgeExpandingResource) resource).getVbrName().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VBridgeExpandingResource) resource).getVbrName(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		}
		LOG.trace("Completed VBridgeResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request json for get, put and post method of VBridge API
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VBridgeResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of VBridgeResourceValidator");
		boolean isValid = false;
		try {
			isValid = validateUri();
			if (isValid && requestBody != null
					&& VtnServiceConsts.GET.equals(method)) {
				isValid = validator.isValidGet(requestBody, isListOpFlag());
				setInvalidParameter(validator.getInvalidParameter());
				updateOpParameterForList(requestBody);
			} else if (isValid && requestBody != null
					&& VtnServiceConsts.PUT.equals(method)) {
				isValid = validatePut(requestBody);
			} else if (isValid && requestBody != null
					&& VtnServiceConsts.POST.equals(method)) {
				isValid = validatePost(requestBody);
			} else if (isValid) {
				setInvalidParameter(VtnServiceConsts.INCORRECT_METHOD_INVOCATION);
				isValid = false;
			}
		} catch (final NumberFormatException e) {
			LOG.error(e, "Inside catch:NumberFormatException");
			if (method.equals(VtnServiceConsts.GET)) {
				setInvalidParameter(validator.getInvalidParameter());
			}
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
		LOG.trace("Complete VBridgeResourceValidator#validate()");
	}

	/**
	 * Validate put request Json object for VBridge API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */

	private boolean validatePut(final JsonObject requestBody) {
		LOG.trace("Start VBridgeResourceValidator#validatePut()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.VBRIDGE);
		if (requestBody.has(VtnServiceJsonConsts.VBRIDGE)
				&& requestBody.get(VtnServiceJsonConsts.VBRIDGE).isJsonObject()) {
			isValid = true;
			final JsonObject vBridge = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VBRIDGE);
			// validation for key: description
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.DESCRIPTION);
				if (vBridge.has(VtnServiceJsonConsts.DESCRIPTION)
						&& vBridge.getAsJsonPrimitive(
								VtnServiceJsonConsts.DESCRIPTION).getAsString() != null
						&& !vBridge
								.getAsJsonPrimitive(
										VtnServiceJsonConsts.DESCRIPTION)
								.getAsString().isEmpty()) {
					isValid = validator.isValidMaxLength(
							vBridge.getAsJsonPrimitive(
									VtnServiceJsonConsts.DESCRIPTION)
									.getAsString(),
							VtnServiceJsonConsts.LEN_127);
				}
			}
		}
		LOG.trace("Complete VBridgeResourceValidator#validatePut()");
		return isValid;
	}

	/**
	 * Validate post request Json object for VBridge API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validatePost(final JsonObject requestBody) {
		LOG.trace("Start VBridgeResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.VBRIDGE);
		if (requestBody.has(VtnServiceJsonConsts.VBRIDGE)
				&& requestBody.get(VtnServiceJsonConsts.VBRIDGE).isJsonObject()) {
			final JsonObject vbridge = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VBRIDGE);
			// validation for mandatory key: vbr_name
			setInvalidParameter(VtnServiceJsonConsts.VBRNAME);
			if (vbridge.has(VtnServiceJsonConsts.VBRNAME)
					&& vbridge.getAsJsonPrimitive(VtnServiceJsonConsts.VBRNAME)
							.getAsString() != null) {
				isValid = validator.isValidMaxLengthAlphaNum(vbridge
						.getAsJsonPrimitive(VtnServiceJsonConsts.VBRNAME)
						.getAsString(), VtnServiceJsonConsts.LEN_31);
			}
			// validation for mandatory key:controller_id
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.CONTROLLERID);
				if (vbridge.has(VtnServiceJsonConsts.CONTROLLERID)
						&& vbridge.getAsJsonPrimitive(
								VtnServiceJsonConsts.CONTROLLERID)
								.getAsString() != null) {
					isValid = validator
							.isValidMaxLengthAlphaNum(
									vbridge.getAsJsonPrimitive(
											VtnServiceJsonConsts.CONTROLLERID)
											.getAsString(),
									VtnServiceJsonConsts.LEN_31);
				}
			}
			// validation for key: description
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.DESCRIPTION);
				if (vbridge.has(VtnServiceJsonConsts.DESCRIPTION)
						&& vbridge.getAsJsonPrimitive(
								VtnServiceJsonConsts.DESCRIPTION).getAsString() != null
						&& !vbridge
								.getAsJsonPrimitive(
										VtnServiceJsonConsts.DESCRIPTION)
								.getAsString().isEmpty()) {
					isValid = validator.isValidMaxLength(
							vbridge.getAsJsonPrimitive(
									VtnServiceJsonConsts.DESCRIPTION)
									.getAsString(),
							VtnServiceJsonConsts.LEN_127);
				}
			}
			// validation for key: DomainID
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.DOMAINID);
				if (vbridge.has(VtnServiceJsonConsts.DOMAINID)
						&& vbridge.getAsJsonPrimitive(
								VtnServiceJsonConsts.DOMAINID).getAsString() != null) {
					isValid = validator.isValidDomainId(vbridge
							.getAsJsonPrimitive(VtnServiceJsonConsts.DOMAINID)
							.getAsString(), VtnServiceJsonConsts.LEN_31);
				}
			}

			setInvalidParameter(VtnServiceJsonConsts.CONTROLLERID
					+ VtnServiceJsonConsts.SLASH
					+ VtnServiceJsonConsts.DOMAINID);
			// spcefity controller_id
			if (vbridge.has(VtnServiceJsonConsts.CONTROLLERID)) {
				isValid = vbridge.has(VtnServiceJsonConsts.DOMAINID);
			// not spcefity controller_id
			} else {
				isValid = !vbridge.has(VtnServiceJsonConsts.DOMAINID);
			}

		}
		LOG.trace("Complete VBridgeResourceValidator#validatePost()");
		return isValid;
	}
}
