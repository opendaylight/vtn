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
import org.opendaylight.vtn.javaapi.resources.logical.FlowListResource;
import org.opendaylight.vtn.javaapi.resources.logical.FlowListsResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class FlowListResourceValidator validates request Json object for
 * FlowList API.
 */
public class FlowListResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(FlowListResourceValidator.class.getName());
	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new flow list resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public FlowListResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for FlowList API
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start FlowListResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.FLNAME);
		if (resource instanceof FlowListResource
				&& ((FlowListResource) resource).getFlName() != null
				&& !((FlowListResource) resource).getFlName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((FlowListResource) resource).getFlName(),
					VtnServiceJsonConsts.LEN_32);
			setListOpFlag(false);
		} else if (resource instanceof FlowListsResource) {
			isValid = true;
			setListOpFlag(true);
		}
		LOG.trace("Start FlowListResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request Json object for get and post method of FlowList API.
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start FlowListResourceValidator#validate()");

		LOG.info("Validating request for " + method
				+ " of FlowListResourceValidator");
		boolean isValid = false;
		try {
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
		} catch (final NumberFormatException e) {
			LOG.error(e, "Inside catch:NumberFormatException");
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
		LOG.trace("Complete FlowListResourceValidator#validate()");
	}

	/**
	 * Validate post request Json object for FlowList API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validatePost(final JsonObject requestBody) {
		LOG.trace("Start FlowListResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.FLOWLIST);
		if (requestBody.has(VtnServiceJsonConsts.FLOWLIST)
				&& requestBody.get(VtnServiceJsonConsts.FLOWLIST)
						.isJsonObject()) {
			final JsonObject flowList = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.FLOWLIST);
			// validation for madatory key: fl_name
			setInvalidParameter(VtnServiceJsonConsts.FLNAME);
			if (flowList.has(VtnServiceJsonConsts.FLNAME)
					&& flowList.getAsJsonPrimitive(VtnServiceJsonConsts.FLNAME)
							.getAsString() != null
					&& !flowList
							.getAsJsonPrimitive(VtnServiceJsonConsts.FLNAME)
							.getAsString().isEmpty()) {
				isValid = validator.isValidMaxLengthAlphaNum(flowList
						.getAsJsonPrimitive(VtnServiceJsonConsts.FLNAME)
						.getAsString(), VtnServiceJsonConsts.LEN_32);
			}
			// validation for key: ip_version(optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.IPVERSION);
				if (flowList.has(VtnServiceJsonConsts.IPVERSION)
						&& flowList.getAsJsonPrimitive(
								VtnServiceJsonConsts.IPVERSION).getAsString() != null
						&& !flowList
								.getAsJsonPrimitive(
										VtnServiceJsonConsts.IPVERSION)
								.getAsString().isEmpty()) {
					final String ipVersion = flowList.getAsJsonPrimitive(
							VtnServiceJsonConsts.IPVERSION).getAsString();
					isValid = ipVersion
							.equalsIgnoreCase(VtnServiceJsonConsts.IP)
							|| ipVersion
									.equalsIgnoreCase(VtnServiceJsonConsts.IPV6);
				} else {
					flowList.remove(VtnServiceJsonConsts.IPVERSION);
					flowList.addProperty(VtnServiceJsonConsts.IPVERSION,
							VtnServiceJsonConsts.IP);
					isValid = true;
				}
			}
		}
		LOG.trace("Complete FlowListResourceValidator#validatePost()");
		return isValid;
	}

	/**
	 * Validate get request Json object for FlowList API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @param opFlag
	 *            the op flag
	 * @return true, if successful
	 */

	private boolean validateGet(final JsonObject requestBody,
			final boolean opFlag) {
		LOG.trace("Start FlowListResourceValidator#validateGet()");
		boolean isValid = true;
		// validation for key: targetdb
		setInvalidParameter(VtnServiceJsonConsts.TARGETDB);
		isValid = validator.isValidRequestDB(requestBody);
		// validation for key: ip_version(optional)
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.IPVERSION);
			if (requestBody.has(VtnServiceJsonConsts.IPVERSION)
					&& requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.IPVERSION).getAsString() != null
					&& !requestBody
							.getAsJsonPrimitive(VtnServiceJsonConsts.IPVERSION)
							.getAsString().isEmpty()) {
				final String ipVersion = requestBody.getAsJsonPrimitive(
						VtnServiceJsonConsts.IPVERSION).getAsString();
				isValid = ipVersion.equalsIgnoreCase(VtnServiceJsonConsts.IP)
						|| ipVersion
								.equalsIgnoreCase(VtnServiceJsonConsts.IPV6);
			} else {
				requestBody.remove(VtnServiceJsonConsts.IPVERSION);
				requestBody.addProperty(VtnServiceJsonConsts.IPVERSION,
						VtnServiceJsonConsts.IP);
				isValid = true;
			}
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
			// validation for key: op
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.OP);
				isValid = validator.isValidOperation(requestBody);
			}
			// validation for key: index
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.INDEX);
				if (requestBody.has(VtnServiceJsonConsts.INDEX)
						&& requestBody.getAsJsonPrimitive(
								VtnServiceJsonConsts.INDEX).getAsString() != null
						&& !requestBody
								.getAsJsonPrimitive(VtnServiceJsonConsts.INDEX)
								.getAsString().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(requestBody
							.getAsJsonPrimitive(VtnServiceJsonConsts.INDEX)
							.getAsString(), VtnServiceJsonConsts.LEN_32);
				}
			}
			// validation for key: max_repitition
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.MAX);
				isValid = validator.isValidMaxRepetition(requestBody);
			}
		}
		LOG.trace("Complete FlowListResourceValidator#validateGet()");
		return isValid;
	}
}
