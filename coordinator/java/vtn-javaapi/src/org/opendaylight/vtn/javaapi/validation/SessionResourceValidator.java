/*
 * Copyright (c) 2012-2014 NEC Corporation
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
import org.opendaylight.vtn.javaapi.resources.SessionResource;
import org.opendaylight.vtn.javaapi.resources.SessionsResource;

/**
 * The Class SessionResourceValidator validates Create Session API and List
 * Sessions API.
 */

public class SessionResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(SessionResourceValidator.class.getName());

	/** The instance of AbstractResource. */
	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new session resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public SessionResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri.
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start SessionResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.SESSIONID);
		if (resource instanceof SessionResource
				&& ((SessionResource) resource).getSessionId() != null
				&& !((SessionResource) resource).getSessionId().isEmpty()) {
			/*
			 * if (Long.parseLong(((SessionResource) resource).getSessionId() )
			 * >= 0) { isValid = true; }
			 */
			isValid = validator.isValidRange(
					((SessionResource) resource).getSessionId(),
					VtnServiceJsonConsts.LONG_VAL_1,
					VtnServiceJsonConsts.LONG_VAL_4294967295);
			setListOpFlag(false);
		} else if (resource instanceof SessionsResource) {
			isValid = true;
			setListOpFlag(true);
		}
		LOG.trace("Complete SessionResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request json for Create Session API and List Sessions API.
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start SessionResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of SessionResourceValidator");
		boolean isValid = false;
		isValid = validateUri();
		if (isValid && requestBody != null
				&& VtnServiceConsts.GET.equals(method)) {
			isValid = validateGet(requestBody);
			updateOpParameterForList(requestBody);
		} else if (isValid && requestBody != null
				&& VtnServiceConsts.POST.equals(method)) {
			isValid = validatePost(requestBody);
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
		LOG.trace("Complete SessionResourceValidator#validate()");
	}

	/**
	 * Validate get request Json for List Sessions API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validateGet(final JsonObject requestBody) {
		LOG.trace("Start SessionResourceValidator#validateGet()");
		boolean isValid = true;
		// validation for key: op
		setInvalidParameter(VtnServiceJsonConsts.OP);
		if (requestBody.has(VtnServiceJsonConsts.OP)
				&& requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.OP)
						.getAsString() != null
				&& !requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.OP)
						.getAsString().isEmpty()) {
			isValid = validator.isValidOperation(requestBody);
		}
		LOG.trace("Complete SessionResourceValidator#validateGet()");
		return isValid;
	}

	/**
	 * Validate post request Json for Create Session API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validatePost(final JsonObject requestBody) {
		LOG.trace("Start SessionResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.SESSION);
		if (requestBody.has(VtnServiceJsonConsts.SESSION)
				&& requestBody.get(VtnServiceJsonConsts.SESSION).isJsonObject()) {
			final JsonObject session = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.SESSION);
			// validation for mandatory key: password
			setInvalidParameter(VtnServiceJsonConsts.PASSWORD);
			if (session.has(VtnServiceJsonConsts.PASSWORD)
					&& session
							.getAsJsonPrimitive(VtnServiceJsonConsts.PASSWORD)
							.getAsString() != null
					&& !session
							.getAsJsonPrimitive(VtnServiceJsonConsts.PASSWORD)
							.getAsString().isEmpty()) {
				isValid = validator.isValidMaxLength(session
						.getAsJsonPrimitive(VtnServiceJsonConsts.PASSWORD)
						.getAsString(), VtnServiceJsonConsts.LEN_72);
			}
			// validation for mandatory key: ipaddr
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.IPADDR);
				if (session.has(VtnServiceJsonConsts.IPADDR)
						&& session.getAsJsonPrimitive(
								VtnServiceJsonConsts.IPADDR).getAsString() != null
						&& !session
								.getAsJsonPrimitive(VtnServiceJsonConsts.IPADDR)
								.getAsString().isEmpty()) {
					isValid = validator.isValidIpV4(session.getAsJsonPrimitive(
							VtnServiceJsonConsts.IPADDR).getAsString());
				} else {
					isValid = false;
				}
			}
			// validation for key: login_name
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.LOGIN_NAME);
				if (session.has(VtnServiceJsonConsts.LOGIN_NAME)
						&& session.getAsJsonPrimitive(
								VtnServiceJsonConsts.LOGIN_NAME).getAsString() != null
						&& !session
								.getAsJsonPrimitive(
										VtnServiceJsonConsts.LOGIN_NAME)
								.getAsString().isEmpty()) {
					isValid = validator
							.isValidMaxLength(
									session.getAsJsonPrimitive(
											VtnServiceJsonConsts.LOGIN_NAME)
											.getAsString(),
									VtnServiceJsonConsts.LEN_32);
				}
			}
			// validation for key: username
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.USERNAME);
				if (session.has(VtnServiceJsonConsts.USERNAME)
						&& session.getAsJsonPrimitive(
								VtnServiceJsonConsts.USERNAME).getAsString() != null
						&& !session
								.getAsJsonPrimitive(
										VtnServiceJsonConsts.USERNAME)
								.getAsString().isEmpty()) {
					isValid = validator.isValidMaxLength(session
							.getAsJsonPrimitive(VtnServiceJsonConsts.USERNAME)
							.getAsString(), VtnServiceJsonConsts.LEN_32);
				} else {
					requestBody.getAsJsonObject(VtnServiceJsonConsts.SESSION)
							.remove(VtnServiceJsonConsts.USERNAME);
					requestBody.getAsJsonObject(VtnServiceJsonConsts.SESSION)
							.addProperty(VtnServiceJsonConsts.USERNAME,
									VtnServiceJsonConsts.OPER);
				}
			}
			// validation for key: type
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.TYPE);
				if (session.has(VtnServiceJsonConsts.TYPE)
						&& session
								.getAsJsonPrimitive(VtnServiceJsonConsts.TYPE)
								.getAsString() != null
						&& !session
								.getAsJsonPrimitive(VtnServiceJsonConsts.TYPE)
								.getAsString().isEmpty()) {
					isValid = session
							.getAsJsonPrimitive(VtnServiceJsonConsts.TYPE)
							.getAsString()
							.equalsIgnoreCase(VtnServiceJsonConsts.WEBAPI)
							|| session
									.getAsJsonPrimitive(
											VtnServiceJsonConsts.TYPE)
									.getAsString()

									.equalsIgnoreCase(
											VtnServiceJsonConsts.WEBUI);
				} else {
					requestBody.getAsJsonObject(VtnServiceJsonConsts.SESSION)
							.remove(VtnServiceJsonConsts.TYPE);
					requestBody.getAsJsonObject(VtnServiceJsonConsts.SESSION)
							.addProperty(VtnServiceJsonConsts.TYPE,
									VtnServiceJsonConsts.WEBUI);
				}
			}
			// validation for key: info
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.INFO);
				if (session.has(VtnServiceJsonConsts.INFO)
						&& session
								.getAsJsonPrimitive(VtnServiceJsonConsts.INFO)
								.getAsString() != null
						&& !session
								.getAsJsonPrimitive(VtnServiceJsonConsts.INFO)
								.getAsString().isEmpty()) {
					isValid = validator.isValidMaxLength(session
							.getAsJsonPrimitive(VtnServiceJsonConsts.INFO)
							.getAsString(), VtnServiceJsonConsts.LEN_63);
				}
			}
		}
		LOG.trace("Complete SessionResourceValidator#validatePost()");
		return isValid;
	}
}
