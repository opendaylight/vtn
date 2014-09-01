/*
 * Copyright (c) 2014 NEC Corporation
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
import org.opendaylight.vtn.javaapi.resources.logical.VTerminalResource;
import org.opendaylight.vtn.javaapi.resources.logical.VTerminalsResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class VTerminalResourceValidator validates request Json object for
 * Vterminal API.
 */
public class VTerminalResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(VTerminalResourceValidator.class.getName());
	private final AbstractResource resource;
	final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new VTerminal resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public VTerminalResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for VTerminal API.
	 * 
	 * @return true, if successful
	 */
	@Override
	public boolean validateUri() {
		LOG.trace("Start VTerminalResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.VTNNAME);
		if (resource instanceof VTerminalsResource
				&& ((VTerminalsResource) resource).getVtnName() != null
				&& !((VTerminalsResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VTerminalsResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			setListOpFlag(true);
		} else if (resource instanceof VTerminalResource
				&& ((VTerminalResource) resource).getVtnName() != null
				&& !((VTerminalResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VTerminalResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VTERMINAL_NAME);
				if (((VTerminalResource) resource).getVterminalName() != null
						&& !((VTerminalResource) resource).getVterminalName()
								.isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VTerminalResource) resource).getVterminalName(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		}
		LOG.trace("Completed VTerminalResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request json for get, put and post method of VTerminal API
	 */
	@Override
	public void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VTerminalResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of VTerminalResourceValidator");
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
		LOG.trace("Complete VTerminalResourceValidator#validate()");
	}

	/**
	 * Validate put request Json object for VTerminal API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */

	private boolean validatePut(final JsonObject requestBody) {
		LOG.trace("Start VTerminalResourceValidator#validatePut()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.VTERMINAL);
		if (requestBody.has(VtnServiceJsonConsts.VTERMINAL)
				&& requestBody.get(VtnServiceJsonConsts.VTERMINAL)
						.isJsonObject()) {
			isValid = true;
			final JsonObject vTerminal = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VTERMINAL);
			if (vTerminal.has(VtnServiceJsonConsts.CONTROLLERID)
					|| vTerminal.has(VtnServiceJsonConsts.DOMAINID)) {
				isValid = false;
			} else {
				// validation for key: description
				setInvalidParameter(VtnServiceJsonConsts.DESCRIPTION);
				if (vTerminal.has(VtnServiceJsonConsts.DESCRIPTION)
						&& vTerminal.getAsJsonPrimitive(
								VtnServiceJsonConsts.DESCRIPTION).getAsString() != null
						&& !vTerminal
								.getAsJsonPrimitive(
										VtnServiceJsonConsts.DESCRIPTION)
								.getAsString().isEmpty()) {
					isValid = validator.isValidMaxLength(
							vTerminal.getAsJsonPrimitive(
									VtnServiceJsonConsts.DESCRIPTION)
									.getAsString(),
							VtnServiceJsonConsts.LEN_127);
				}
			}
		}
		LOG.trace("Complete vTerminalResourceValidator#validatePut()");
		return isValid;
	}

	/**
	 * Validate post request Json object for vTerminal API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validatePost(final JsonObject requestBody) {
		LOG.trace("Start vTerminalResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.VTERMINAL);
		if (requestBody.has(VtnServiceJsonConsts.VTERMINAL)
				&& requestBody.get(VtnServiceJsonConsts.VTERMINAL)
						.isJsonObject()) {
			final JsonObject vterminal = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VTERMINAL);
			// validation for mandatory key: VTERMINAL_NAME
			setInvalidParameter(VtnServiceJsonConsts.VTERMINAL_NAME);
			if (vterminal.has(VtnServiceJsonConsts.VTERMINAL_NAME)
					&& vterminal.getAsJsonPrimitive(
							VtnServiceJsonConsts.VTERMINAL_NAME).getAsString() != null) {
				isValid = validator.isValidMaxLengthAlphaNum(
						vterminal.getAsJsonPrimitive(
								VtnServiceJsonConsts.VTERMINAL_NAME)
								.getAsString(), VtnServiceJsonConsts.LEN_31);
			}
			// validation for mandatory key:controller_id
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.CONTROLLERID);
				if (vterminal.has(VtnServiceJsonConsts.CONTROLLERID)
						&& vterminal.getAsJsonPrimitive(
								VtnServiceJsonConsts.CONTROLLERID)
								.getAsString() != null) {
					isValid = validator
							.isValidMaxLengthAlphaNum(
									vterminal.getAsJsonPrimitive(
											VtnServiceJsonConsts.CONTROLLERID)
											.getAsString(),
									VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			// validation for key: DomainID
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.DOMAINID);
				if (vterminal.has(VtnServiceJsonConsts.DOMAINID)
						&& vterminal.getAsJsonPrimitive(
								VtnServiceJsonConsts.DOMAINID).getAsString() != null) {
					isValid = validator.isValidDomainId(vterminal
							.getAsJsonPrimitive(VtnServiceJsonConsts.DOMAINID)
							.getAsString(), VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			// validation for key: description
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.DESCRIPTION);
				if (vterminal.has(VtnServiceJsonConsts.DESCRIPTION)
						&& vterminal.getAsJsonPrimitive(
								VtnServiceJsonConsts.DESCRIPTION).getAsString() != null
						&& !vterminal
								.getAsJsonPrimitive(
										VtnServiceJsonConsts.DESCRIPTION)
								.getAsString().isEmpty()) {
					isValid = validator.isValidMaxLength(
							vterminal.getAsJsonPrimitive(
									VtnServiceJsonConsts.DESCRIPTION)
									.getAsString(),
							VtnServiceJsonConsts.LEN_127);
				}
			}

		}
		LOG.trace("Complete vTerminalResourceValidator#validatePost()");
		return isValid;
	}
}
