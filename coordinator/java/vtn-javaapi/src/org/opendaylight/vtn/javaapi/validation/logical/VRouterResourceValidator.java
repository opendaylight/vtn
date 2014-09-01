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
import org.opendaylight.vtn.javaapi.resources.logical.VRouterResource;
import org.opendaylight.vtn.javaapi.resources.logical.VRoutersResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class VRouterResourceValidator validates request Json object for Vrouter
 * API.
 */
public class VRouterResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(VRouterResourceValidator.class.getName());
	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new vrouter resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public VRouterResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for Vrouter API
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start VRouterResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.VTNNAME);
		if (resource instanceof VRoutersResource
				&& ((VRoutersResource) resource).getVtnName() != null
				&& !((VRoutersResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VRoutersResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			setListOpFlag(true);
		} else if (resource instanceof VRouterResource
				&& ((VRouterResource) resource).getVtnName() != null
				&& !((VRouterResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VRouterResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VRTNAME);
				if (((VRouterResource) resource).getVrtName() != null
						&& !((VRouterResource) resource).getVrtName().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VRouterResource) resource).getVrtName(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		}
		LOG.trace("Complete VRouterResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request Json object for get, put and post method of VRouter API.
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VRouterResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of VRouterResourceValidator");
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
		LOG.trace("Complete VRouterResourceValidator#validate()");
	}

	/**
	 * Validate post request Json object of Vrouter API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	private boolean validatePost(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VRouterResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.VROUTER);
		if (requestBody.has(VtnServiceJsonConsts.VROUTER)
				&& requestBody.get(VtnServiceJsonConsts.VROUTER).isJsonObject()) {
			final JsonObject vRouter = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VROUTER);
			// validation for key: vrt_name(mandatory)
			setInvalidParameter(VtnServiceJsonConsts.VRTNAME);
			if (vRouter.has(VtnServiceJsonConsts.VRTNAME)
					&& vRouter.getAsJsonPrimitive(VtnServiceJsonConsts.VRTNAME)
							.getAsString() != null) {
				isValid = validator.isValidMaxLengthAlphaNum(vRouter
						.getAsJsonPrimitive(VtnServiceJsonConsts.VRTNAME)
						.getAsString(), VtnServiceJsonConsts.LEN_31);
			}
			// validation for key: controller_id(mandatory)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.CONTROLLERID);
				if (vRouter.has(VtnServiceJsonConsts.CONTROLLERID)
						&& vRouter.getAsJsonPrimitive(
								VtnServiceJsonConsts.CONTROLLERID)
								.getAsString() != null) {
					isValid = validator
							.isValidMaxLengthAlphaNum(
									vRouter.getAsJsonPrimitive(
											VtnServiceJsonConsts.CONTROLLERID)
											.getAsString(),
									VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			// validation for key: description(optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.DESCRIPTION);
				if (vRouter.has(VtnServiceJsonConsts.DESCRIPTION)
						&& vRouter.getAsJsonPrimitive(
								VtnServiceJsonConsts.DESCRIPTION).getAsString() != null) {
					isValid = validator.isValidMaxLength(
							vRouter.getAsJsonPrimitive(
									VtnServiceJsonConsts.DESCRIPTION)
									.getAsString(),
							VtnServiceJsonConsts.LEN_127)
							|| vRouter
									.getAsJsonPrimitive(
											VtnServiceJsonConsts.DESCRIPTION)
									.getAsString().isEmpty();
				}
			}

			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.DOMAINID);
				if (vRouter.has(VtnServiceJsonConsts.DOMAINID)
						&& vRouter.getAsJsonPrimitive(
								VtnServiceJsonConsts.DOMAINID).getAsString() != null) {
					isValid = validator.isValidDomainId(vRouter
							.getAsJsonPrimitive(VtnServiceJsonConsts.DOMAINID)
							.getAsString(), VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
		}
		LOG.trace("Complete VRouterResourceValidator#validatePost()");
		return isValid;
	}

	/**
	 * Validate put request Json object for Vrouter API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	private boolean validatePut(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VRouterResourceValidator#validatePut()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.VROUTER);
		if (requestBody.has(VtnServiceJsonConsts.VROUTER)
				&& requestBody.get(VtnServiceJsonConsts.VROUTER).isJsonObject()) {
			isValid = true;
			final JsonObject vRouter = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VROUTER);
			// validation for key: description(optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.DESCRIPTION);
				if (vRouter.has(VtnServiceJsonConsts.DESCRIPTION)
						&& vRouter.getAsJsonPrimitive(
								VtnServiceJsonConsts.DESCRIPTION).getAsString() != null) {
					isValid = validator.isValidMaxLength(
							vRouter.getAsJsonPrimitive(
									VtnServiceJsonConsts.DESCRIPTION)
									.getAsString(),
							VtnServiceJsonConsts.LEN_127)
							|| vRouter
									.getAsJsonPrimitive(
											VtnServiceJsonConsts.DESCRIPTION)
									.getAsString().isEmpty();
				}
			}
		}
		LOG.trace("Complete VRouterResourceValidator#validatePut()");
		return isValid;
	}
}
