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
import org.opendaylight.vtn.javaapi.resources.logical.VBridgeInterfacePolicingMapResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class VBridgeInterfacePolicingMapResourceValidator validates request JSON
 * object for PolicingMap API.
 */
public class VBridgeInterfacePolicingMapResourceValidator extends
		VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(VBridgeInterfacePolicingMapResourceValidator.class
					.getName());

	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new port map resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public VBridgeInterfacePolicingMapResourceValidator(
			final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate URI parameters for PolcingMapAPI.
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start VBridgeInterfacePolicingMapResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.VTNNAME);
		if (resource instanceof VBridgeInterfacePolicingMapResource
				&& ((VBridgeInterfacePolicingMapResource) resource)
						.getVtnName() != null
				&& !((VBridgeInterfacePolicingMapResource) resource)
						.getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VBridgeInterfacePolicingMapResource) resource)
							.getVtnName(), VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VBRNAME);
				if (((VBridgeInterfacePolicingMapResource) resource)
						.getVbrName() != null
						&& !((VBridgeInterfacePolicingMapResource) resource)
								.getVbrName().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VBridgeInterfacePolicingMapResource) resource)
									.getVbrName(), VtnServiceJsonConsts.LEN_31);

					if (isValid) {
						setInvalidParameter(VtnServiceJsonConsts.URI
								+ VtnServiceJsonConsts.IFNAME);
						if (((VBridgeInterfacePolicingMapResource) resource)
								.getIfName() != null
								&& !((VBridgeInterfacePolicingMapResource) resource)
										.getIfName().isEmpty()) {
							isValid = validator
									.isValidMaxLengthAlphaNum(
											((VBridgeInterfacePolicingMapResource) resource)
													.getIfName(),
											VtnServiceJsonConsts.LEN_31);
						} else {
							isValid = false;
						}
					}

				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		}
		LOG.trace("Completed VBridgeInterfacePolicingMapResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request JSON object for get, put method of
	 * VBridgeInterfacePolicingMapResource API.
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VBridgeInterfacePolicingMapResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of VBridgeInterfacePolicingMapResourceValidator");
		boolean isValid = false;
		try {
			isValid = validateUri();
			if (isValid && requestBody != null
					&& VtnServiceConsts.GET.equals(method)) {
				isValid = validateGet(requestBody);
			} else if (isValid && requestBody != null
					&& VtnServiceConsts.PUT.equals(method)) {
				isValid = validatePut(requestBody);
			} else if (isValid) {
				setInvalidParameter(VtnServiceConsts.INCORRECT_METHOD_INVOCATION);
				isValid = false;
			}
		} catch (final ClassCastException e) {
			if (method.equals(VtnServiceConsts.GET)) {
				setInvalidParameter(validator.getInvalidParameter());
			}
			LOG.info("Inside catch:ClassCastException");
			isValid = false;
		}
		// Throws exception if validation fails
		if (!isValid) {
			LOG.info("Validation failed");
			throw new VtnServiceException(Thread.currentThread()
					.getStackTrace()[1].getMethodName(),
					UncJavaAPIErrorCode.VALIDATION_ERROR.getErrorCode(),
					UncJavaAPIErrorCode.VALIDATION_ERROR.getErrorMessage());
		}
		LOG.info("Validation successful");
		LOG.trace("Complete VBridgeInterfacePolicingMapResourceValidator#validate()");

	}

	/**
	 * Checks if is valid get request.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if is valid get
	 */
	public final boolean validateGet(final JsonObject requestBody) {
		LOG.trace("Start VBridgePolicingMapResourceValidator#validateGet()");
		boolean isValid = true;
		setInvalidParameter(VtnServiceJsonConsts.TARGETDB);
		isValid = validator.isValidRequestDB(requestBody);
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.OP);
			if (requestBody.has(VtnServiceJsonConsts.OP)
					&& requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.OP)
							.getAsString() != null
					&& !requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.OP)
							.getAsString().isEmpty()) {
				final String operation = requestBody.getAsJsonPrimitive(
						VtnServiceJsonConsts.OP).getAsString();
				isValid = operation
						.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL);
			} else {
				requestBody.remove(VtnServiceJsonConsts.OP);
				requestBody.addProperty(VtnServiceJsonConsts.OP,
						VtnServiceJsonConsts.NORMAL);
			}
		}
		LOG.trace("Complete VBridgePolicingMapResourceValidator#validateGet()");
		return isValid;
	}

	/**
	 * validate put request JSON object for VBridgeInterfacePolicingMapResource
	 * API.
	 * 
	 * @param requestBody
	 *            the request JSON object
	 * @return true, if successful
	 */
	private boolean validatePut(final JsonObject requestBody) {
		LOG.trace("Start VBridgeInterfacePolicingMapResourceValidator#validatePut()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.POLICINGMAP);
		if (requestBody.has(VtnServiceJsonConsts.POLICINGMAP)
				&& requestBody.get(VtnServiceJsonConsts.POLICINGMAP)
						.isJsonObject()) {
			final JsonObject policing = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.POLICINGMAP);

			// validation for mandatory key: prf_name
			if (policing.has(VtnServiceJsonConsts.PROFILE_NAME)
					&& policing.getAsJsonPrimitive(
							VtnServiceJsonConsts.PROFILE_NAME).getAsString() != null) {
				isValid = validator.isValidMaxLengthAlphaNum(policing
						.getAsJsonPrimitive(VtnServiceJsonConsts.PROFILE_NAME)
						.getAsString(), VtnServiceJsonConsts.LEN_32);
			}

		}
		LOG.trace("Complete VBridgeInterfacePolicingMapResourceValidator#validatePut()");
		return isValid;
	}
}
