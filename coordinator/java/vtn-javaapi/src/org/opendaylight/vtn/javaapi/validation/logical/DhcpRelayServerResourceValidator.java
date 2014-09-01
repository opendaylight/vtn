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
import com.google.gson.JsonParseException;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.resources.logical.DhcpRelayServerResource;
import org.opendaylight.vtn.javaapi.resources.logical.DhcpRelayServersResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class DhcpRelayServerResourceValidator validates request Json object for
 * DHCP Relay Server API.
 */
public class DhcpRelayServerResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(DhcpRelayServerResourceValidator.class.getName());

	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new DHCP relay server resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public DhcpRelayServerResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameteres for DhcpRelayServer API
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start DhcpRelayServerResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.VTNNAME);
		if (resource instanceof DhcpRelayServersResource
				&& ((DhcpRelayServersResource) resource).getVtnName() != null
				&& !((DhcpRelayServersResource) resource).getVtnName()
						.isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((DhcpRelayServersResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VRTNAME);
				if (((DhcpRelayServersResource) resource).getVrtName() != null
						&& !((DhcpRelayServersResource) resource).getVrtName()
								.isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((DhcpRelayServersResource) resource).getVrtName(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(true);
		} else if (resource instanceof DhcpRelayServerResource
				&& ((DhcpRelayServerResource) resource).getVtnName() != null
				&& !((DhcpRelayServerResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((DhcpRelayServerResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VRTNAME);
				if (((DhcpRelayServerResource) resource).getVrtName() != null
						&& !((DhcpRelayServerResource) resource).getVrtName()
								.isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((DhcpRelayServerResource) resource).getVrtName(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.IPADDR);
				if (((DhcpRelayServerResource) resource).getIpaddr() != null
						&& !((DhcpRelayServerResource) resource).getIpaddr()
								.isEmpty()) {
					isValid = validator
							.isValidIpV4(((DhcpRelayServerResource) resource)
									.getIpaddr());
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		}
		LOG.trace("Complete DhcpRelayServerResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request Json object for get, put and post method of
	 * DhcpRelayServer API
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start DhcpRelayServerResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of DhcpRelayServerResourceValidator");
		boolean isValid = false;
		isValid = validateUri();
		if (isValid && requestBody != null
				&& VtnServiceConsts.GET.equals(method)) {
			isValid = validateGet(requestBody, isListOpFlag());
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
		LOG.trace("Complete DhcpRelayServerResourceValidator#validate()");

	}

	/**
	 * Validate post request Json object for DhcpRelayServer API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validatePost(final JsonObject requestBody) {
		LOG.trace("Start DhcpRelayServerResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.SERVER);
		if (requestBody.has(VtnServiceJsonConsts.SERVER)
				&& requestBody.get(VtnServiceJsonConsts.SERVER).isJsonObject()) {
			final JsonObject dcpRelayServer = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.SERVER);
			// validation for key: ipaddr(mandatory)
			setInvalidParameter(VtnServiceJsonConsts.IPADDR);
			if (dcpRelayServer.has(VtnServiceJsonConsts.IPADDR)
					&& dcpRelayServer.getAsJsonPrimitive(
							VtnServiceJsonConsts.IPADDR).getAsString() != null) {
				isValid = validator.isValidIpV4(dcpRelayServer
						.getAsJsonPrimitive(VtnServiceJsonConsts.IPADDR)
						.getAsString());
			}
		}
		LOG.trace("Complete DhcpRelayServerResourceValidator#validatePost()");
		return isValid;

	}

	/**
	 * Validate get request json for DhcpRelayServer API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws JsonParseException
	 *             the json parse exception
	 */
	private boolean validateGet(final JsonObject requestBody,
			final boolean opFlag) throws JsonParseException {
		LOG.trace("Start DhcpRelayServerResourceValidator#validateGet()");
		boolean isValid = true;
		// validation for key: targetdb(optional)
		setInvalidParameter(VtnServiceJsonConsts.TARGETDB);
		if (isValid) {
			isValid = validator.isValidRequestDB(requestBody);
		}
		if (!opFlag) {
			if (requestBody.has(VtnServiceJsonConsts.OP)) {
				requestBody.remove(VtnServiceJsonConsts.OP);
			} else {
				LOG.debug("No need to remove");
			}
			if (requestBody.has(VtnServiceJsonConsts.INDEX)) {
				requestBody.remove(VtnServiceJsonConsts.INDEX);
			} else {
				LOG.debug("No need to remove");
			}
			if (requestBody.has(VtnServiceJsonConsts.MAX)) {
				requestBody.remove(VtnServiceJsonConsts.MAX);
			} else {
				LOG.debug("No need to remove");
			}
		} else {
			// validation for key: op(optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.OP);
				isValid = validator.isValidOperationForCount(requestBody);
			}
			// validation for key: index(optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.INDEX);
				if (requestBody.has(VtnServiceJsonConsts.INDEX)
						&& requestBody.getAsJsonPrimitive(
								VtnServiceJsonConsts.INDEX).getAsString() != null) {
					isValid = validator.isValidIpV4(requestBody
							.getAsJsonPrimitive(VtnServiceJsonConsts.INDEX)
							.getAsString());
				}
			}
			// validation for key: max(optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.MAX);
				isValid = validator.isValidMaxRepetition(requestBody);
			}
		}
		LOG.trace("Complete DhcpRelayServerResourceValidator#validateGet()");
		return isValid;
	}
}
