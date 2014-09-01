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
import org.opendaylight.vtn.javaapi.resources.logical.VTepResource;
import org.opendaylight.vtn.javaapi.resources.logical.VTepsResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class VTepResourceValidator validates request Json object for Vtep API.
 */
public class VTepResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(VTepResourceValidator.class.getName());

	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new v tep resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public VTepResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for VTep API.
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start VTepResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.VTNNAME);
		if (resource instanceof VTepsResource
				&& ((VTepsResource) resource).getVtnName() != null
				&& !((VTepsResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VTepsResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			setListOpFlag(true);
		} else if (resource instanceof VTepResource
				&& ((VTepResource) resource).getVtnName() != null
				&& !((VTepResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VTepResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VTEPNAME);
				if (((VTepResource) resource).getvTepName() != null
						&& !((VTepResource) resource).getvTepName().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VTepResource) resource).getvTepName(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		}
		LOG.trace("Complete VTepResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request Json object for get, put and post method of VTep API.
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VTepResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of VTepResourceValidator");
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
		LOG.trace("Complete VTepResourceValidator#validate()");
	}

	/**
	 * validate post request Json object for VTep API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	private boolean validatePost(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VTepResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.VTEP);
		if (requestBody.has(VtnServiceJsonConsts.VTEP)
				&& requestBody.get(VtnServiceJsonConsts.VTEP).isJsonObject()) {
			final JsonObject vTep = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VTEP);
			// validation for key: vtep_name(mandatory)
			setInvalidParameter(VtnServiceJsonConsts.VTEPNAME);
			if (vTep.has(VtnServiceJsonConsts.VTEPNAME)
					&& vTep.getAsJsonPrimitive(VtnServiceJsonConsts.VTEPNAME)
							.getAsString() != null) {
				isValid = validator.isValidMaxLengthAlphaNum(vTep
						.getAsJsonPrimitive(VtnServiceJsonConsts.VTEPNAME)
						.getAsString(), VtnServiceJsonConsts.LEN_31);
			}
			// validation for key: controller_id(mandatory)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.CONTROLLERID);
				if (vTep.has(VtnServiceJsonConsts.CONTROLLERID)
						&& vTep.getAsJsonPrimitive(
								VtnServiceJsonConsts.CONTROLLERID)
								.getAsString() != null) {
					isValid = validator
							.isValidMaxLengthAlphaNum(
									vTep.getAsJsonPrimitive(
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
				if (vTep.has(VtnServiceJsonConsts.DESCRIPTION)
						&& vTep.getAsJsonPrimitive(
								VtnServiceJsonConsts.DESCRIPTION).getAsString() != null
						&& !vTep.getAsJsonPrimitive(
								VtnServiceJsonConsts.DESCRIPTION).getAsString()
								.isEmpty()) {
					isValid = validator.isValidMaxLength(
							vTep.getAsJsonPrimitive(
									VtnServiceJsonConsts.DESCRIPTION)
									.getAsString(),
							VtnServiceJsonConsts.LEN_127);
				}
			}
			// validation for key: DomainID(mandatory)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.DOMAINID);
				if (vTep.has(VtnServiceJsonConsts.DOMAINID)
						&& vTep.getAsJsonPrimitive(
								VtnServiceJsonConsts.DOMAINID).getAsString() != null) {
					isValid = validator.isValidDomainId(vTep
							.getAsJsonPrimitive(VtnServiceJsonConsts.DOMAINID)
							.getAsString(), VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
		}
		LOG.trace("Complete VTepResourceValidator#validatePost()");
		return isValid;
	}

	/**
	 * validate put request Json object for VTep API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validatePut(final JsonObject requestBody) {
		LOG.trace("Start VTepResourceValidator#validatePut()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.VTEP);
		if (requestBody.has(VtnServiceJsonConsts.VTEP)
				&& requestBody.get(VtnServiceJsonConsts.VTEP).isJsonObject()) {
			isValid = true;
			final JsonObject vTep = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VTEP);
			// validation for key: controller_id
			setInvalidParameter(VtnServiceJsonConsts.CONTROLLERID);
			if (vTep.has(VtnServiceJsonConsts.CONTROLLERID)) {
				isValid = false;
			}
			// validation for key: description(optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.DESCRIPTION);
				if (isValid
						&& vTep.has(VtnServiceJsonConsts.DESCRIPTION)
						&& vTep.getAsJsonPrimitive(
								VtnServiceJsonConsts.DESCRIPTION).getAsString() != null
						&& !vTep.getAsJsonPrimitive(
								VtnServiceJsonConsts.DESCRIPTION).getAsString()
								.isEmpty()) {
					isValid = validator.isValidMaxLength(
							vTep.getAsJsonPrimitive(
									VtnServiceJsonConsts.DESCRIPTION)
									.getAsString(),
							VtnServiceJsonConsts.LEN_127);
				}
			}
			// validation for key: DomainID
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.DOMAINID);
				if (vTep.has(VtnServiceJsonConsts.DOMAINID)) {
					isValid = false;
				}
			}
		}
		LOG.trace("Complete VTepResourceValidator#validatePut()");
		return isValid;
	}
}
