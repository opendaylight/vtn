/*
 * Copyright (c) 2012-2013 NEC Corporation
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
import org.opendaylight.vtn.javaapi.resources.logical.VUnknownResource;
import org.opendaylight.vtn.javaapi.resources.logical.VUnknownsResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class VUnknownResourceValidator validates request Json object for
 * Vunknown API.
 */
public class VUnknownResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(VUnknownResourceValidator.class.getName());
	private final AbstractResource resource;
	final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new v unknown resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public VUnknownResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for VUnknownRsource
	 * 
	 * @return true, if successful
	 */
	@Override
	public boolean validateUri() {
		LOG.trace("Start VUnknownResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.VTNNAME);
		if (resource instanceof VUnknownResource
				&& ((VUnknownResource) resource).getVtnName() != null
				&& !((VUnknownResource) resource).getVtnName().trim().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VUnknownResource) resource).getVtnName().trim(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VUKNAME);
				if (((VUnknownResource) resource).getVukName() != null
						&& !((VUnknownResource) resource).getVukName().trim()
								.isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VUnknownResource) resource).getVukName().trim(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		} else if (resource instanceof VUnknownsResource
				&& ((VUnknownsResource) resource).getVtnName() != null
				&& !((VUnknownsResource) resource).getVtnName().trim()
						.isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VUnknownsResource) resource).getVtnName().trim(),
					VtnServiceJsonConsts.LEN_31);
			setListOpFlag(true);
		}
		LOG.trace("Complete VUnknownResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request Json object for get, put and post method of VUnknown API
	 */
	@Override
	public void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VUnknownResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of VUnknownResourceValidator");
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
			} else {
				isValid = false;
			}
		} catch (final NumberFormatException e) {
			if (method.equals(VtnServiceConsts.GET)) {
				setInvalidParameter(validator.getInvalidParameter());
			}
			LOG.error("Inside catch:NumberFormatException");
			isValid = false;
		} catch (final ClassCastException e) {
			if (method.equals(VtnServiceConsts.GET)) {
				setInvalidParameter(validator.getInvalidParameter());
			}
			LOG.error("Inside catch:ClassCastException");
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
		LOG.trace("Complete VUnknownResourceValidator#validate()");
	}

	/**
	 * Validate post request Json object for VUnknownRsource.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	private boolean validatePost(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VUnknownResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.VUNKNOWN);
		if (requestBody.has(VtnServiceJsonConsts.VUNKNOWN)
				&& requestBody.get(VtnServiceJsonConsts.VUNKNOWN)
						.isJsonObject()) {
			final JsonObject vUnknown = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VUNKNOWN);
			// validation for key: vuk_name(mandatory)
			setInvalidParameter(VtnServiceJsonConsts.VUKNAME);
			if (vUnknown.has(VtnServiceJsonConsts.VUKNAME)
					&& vUnknown
							.getAsJsonPrimitive(VtnServiceJsonConsts.VUKNAME)
							.getAsString() != null) {
				isValid = validator.isValidMaxLengthAlphaNum(vUnknown
						.getAsJsonPrimitive(VtnServiceJsonConsts.VUKNAME)
						.getAsString().trim(), VtnServiceJsonConsts.LEN_31);
			}
			if (isValid) {
				isValid = commonValidations(isValid, vUnknown);
			}
			// validation for key: DOMAINID(mandatory)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.DOMAINID);
				if (vUnknown.has(VtnServiceJsonConsts.DOMAINID)
						&& vUnknown.getAsJsonPrimitive(
								VtnServiceJsonConsts.DOMAINID).getAsString() != null) {
					isValid = validator.isValidDomainId(vUnknown
							.getAsJsonPrimitive(VtnServiceJsonConsts.DOMAINID)
							.getAsString().trim(), VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
		}
		LOG.trace("Complete VUnknownResourceValidator#validatePost()");
		return isValid;
	}

	/**
	 * @param isValid
	 * @param vUnknown
	 * @return
	 */
	private boolean commonValidations(boolean isValid, final JsonObject vUnknown) {
		LOG.trace("Start VUnknownResourceValidator#commonValidations()");
		// validation for key: description
		setInvalidParameter(VtnServiceJsonConsts.DESCRIPTION);
		if (vUnknown.has(VtnServiceJsonConsts.DESCRIPTION)
				&& vUnknown.getAsJsonPrimitive(
						VtnServiceJsonConsts.DESCRIPTION).getAsString() != null
						&& !vUnknown
						.getAsJsonPrimitive(
								VtnServiceJsonConsts.DESCRIPTION)
								.getAsString().trim().isEmpty()) {
			isValid = validator.isValidMaxLength(
					vUnknown.getAsJsonPrimitive(
							VtnServiceJsonConsts.DESCRIPTION)
							.getAsString().trim(),
							VtnServiceJsonConsts.LEN_127);
		}
		// validation for key: type
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.TYPE);
			if (vUnknown.has(VtnServiceJsonConsts.TYPE)
					&& vUnknown.getAsJsonPrimitive(
							VtnServiceJsonConsts.TYPE).getAsString() != null) {
				isValid = validType(vUnknown
						.getAsJsonPrimitive(VtnServiceJsonConsts.TYPE)
						.getAsString().trim());
			}
		}
		// validation for key: controller_id
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.CONTROLLERID);
			if (vUnknown.has(VtnServiceJsonConsts.CONTROLLERID)
					&& vUnknown.getAsJsonPrimitive(
							VtnServiceJsonConsts.CONTROLLERID)
							.getAsString() != null
							&& !vUnknown
							.getAsJsonPrimitive(
									VtnServiceJsonConsts.CONTROLLERID)
									.getAsString().trim().isEmpty()) {
				isValid = validator.isValidMaxLengthAlphaNum(
						vUnknown.getAsJsonPrimitive(
								VtnServiceJsonConsts.CONTROLLERID)
								.getAsString().trim(),
								VtnServiceJsonConsts.LEN_31);
			}
		}
		LOG.trace("Complete VUnknownResourceValidator#commonValidations()");
		return isValid;
	}

	/**
	 * Validate put request Json object for VUnknownRsource
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	private boolean validatePut(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VUnknownResourceValidator#validatePut()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.VUNKNOWN);
		if (requestBody.has(VtnServiceJsonConsts.VUNKNOWN)
				&& requestBody.get(VtnServiceJsonConsts.VUNKNOWN)
						.isJsonObject()) {
			isValid = true;
			final JsonObject vUnknown = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VUNKNOWN);
			isValid = commonValidations(isValid, vUnknown);
			// validation for key: DOMAINID(optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.DOMAINID);
				if (vUnknown.has(VtnServiceJsonConsts.DOMAINID)
						&& vUnknown.getAsJsonPrimitive(
								VtnServiceJsonConsts.DOMAINID).getAsString() != null
						&& !vUnknown
								.getAsJsonPrimitive(
										VtnServiceJsonConsts.DOMAINID)
								.getAsString().trim().isEmpty()) {
					isValid = validator.isValidDomainId(vUnknown
							.getAsJsonPrimitive(VtnServiceJsonConsts.DOMAINID)
							.getAsString().trim(), VtnServiceJsonConsts.LEN_31);
				}
			}
		}
		LOG.trace("Complete VUnknownResourceValidator#validatePut()");
		return isValid;
	}

	/**
	 * check if type is of valid type or not
	 * 
	 * @param type
	 *            value of type field
	 * @return
	 */
	private boolean validType(final String type) {
		return VtnServiceJsonConsts.ROUTER.equalsIgnoreCase(type)
				|| VtnServiceJsonConsts.BRIDGE.equalsIgnoreCase(type);
	}
}
