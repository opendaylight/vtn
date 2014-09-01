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
import org.opendaylight.vtn.javaapi.resources.logical.VtnPolicingMapResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * Class provides implementation for validation of request/URI parameters
 * corresponding to GET/PUT/DELETE method of policing-map under VTN
 */
public class VtnPolicingMapResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(VtnPolicingMapResourceValidator.class.getName());

	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new VTN Policing Map validator.
	 * 
	 * @param resource
	 *            the reference of AbstractResource
	 */
	public VtnPolicingMapResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate URI parameters for VTN Policing Map validator
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start VtnPolicingMapValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.VTNNAME);
		if (resource instanceof VtnPolicingMapResource
				&& ((VtnPolicingMapResource) resource).getVtnName() != null
				&& !((VtnPolicingMapResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VtnPolicingMapResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			setListOpFlag(false);
		}
		LOG.trace("Completed VtnPolicingMapValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request JSON object for GET/PUT method of VTN PolcingMapAPI API.
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VtnPolicingMapResourceValidator#validate()");
		boolean isValid = false;
		try {
			isValid = validateUri();
			if (isValid && requestBody != null
					&& VtnServiceConsts.GET.equals(method)) {
				isValid = validateGet(requestBody);
				updateOpParameterForList(requestBody);
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
		LOG.debug("validation successful for + " + method
				+ " of VtnPolicingMapResource");
		LOG.trace("Complete VtnPolicingMapResourceValidator#validate()");
	}

	/**
	 * validate PUT request JSON object for VTN PolcingMap API.
	 * 
	 * @param requestBody
	 *            the request JSON object
	 * @return true, if successful
	 */
	private boolean validatePut(final JsonObject requestBody) {
		LOG.trace("Start VtnPolicingMapResourceValidator#validatePut()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.POLICINGMAP);
		if (requestBody.has(VtnServiceJsonConsts.POLICINGMAP)
				&& requestBody.get(VtnServiceJsonConsts.POLICINGMAP)
						.isJsonObject()) {
			final JsonObject policingmap = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.POLICINGMAP);
			setInvalidParameter(VtnServiceJsonConsts.PROFILE_NAME);
			// validation for mandatory key: profile_name
			if (policingmap.has(VtnServiceJsonConsts.PROFILE_NAME)
					&& policingmap.getAsJsonPrimitive(
							VtnServiceJsonConsts.PROFILE_NAME).getAsString() != null) {
				isValid = validator.isValidMaxLengthAlphaNum(policingmap
						.getAsJsonPrimitive(VtnServiceJsonConsts.PROFILE_NAME)
						.getAsString(), VtnServiceJsonConsts.LEN_32);
			}
		}
		LOG.trace("Complete VtnPolicingMapResourceValidator#validatePut()");
		return isValid;
	}

	/**
	 * validate GET request JSON object for VTN PolcingMap API.
	 * 
	 * @param requestBody
	 *            the request JSON object
	 * @return true, if successful
	 */
	private boolean validateGet(final JsonObject requestBody) {
		LOG.trace("Start VtnPolicingMapResourceValidator#validateGet()");
		boolean isValid = false;

		// validation for key: targetdb
		setInvalidParameter(VtnServiceJsonConsts.TARGETDB);
		isValid = validator.isValidRequestDB(requestBody);

		// validation for key: op
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.OP);
			// isValid = validator.isValidOperation(requestBody);
			if (requestBody.has(VtnServiceJsonConsts.OP)
					&& requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.OP)
							.getAsString() != null) {
				final String operation = requestBody.getAsJsonPrimitive(
						VtnServiceJsonConsts.OP).getAsString();
				isValid = operation
						.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL);
			} else {
				requestBody.addProperty(VtnServiceJsonConsts.OP,
						VtnServiceJsonConsts.NORMAL);
			}
		}

		if (isValid) {
			// validation for key: controller_id(optional)
			setInvalidParameter(VtnServiceJsonConsts.CONTROLLERID);
			if (requestBody.has(VtnServiceJsonConsts.CONTROLLERID)
					&& requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.CONTROLLERID).getAsString() != null) {
				isValid = validator.isValidMaxLengthAlphaNum(requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.CONTROLLERID)
						.getAsString(), VtnServiceJsonConsts.LEN_31);
			}
		}

		if (isValid) {
			// validation for key: domain_id(optional)
			setInvalidParameter(VtnServiceJsonConsts.DOMAINID);
			if (requestBody.has(VtnServiceJsonConsts.DOMAINID)
					&& requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.DOMAINID).getAsString() != null) {
				isValid = validator.isValidDomainId(requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.DOMAINID)
						.getAsString(), VtnServiceJsonConsts.LEN_31);
			}
		}

		/*
		 * controller_id and domain_id are mandatory for detail case
		 */
		if (requestBody.get(VtnServiceJsonConsts.OP).getAsString()
				.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)
				&& requestBody.get(VtnServiceJsonConsts.TARGETDB).getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
			setInvalidParameter(VtnServiceJsonConsts.CONTROLLERID
					+ VtnServiceConsts.HYPHEN + VtnServiceJsonConsts.DOMAINID);
			if (requestBody.has(VtnServiceJsonConsts.CONTROLLERID)
					&& requestBody.has(VtnServiceJsonConsts.DOMAINID)) {
				LOG.info("mandatory parameters are provided for detail case");
			} else {
				LOG.error("mandatory parameters are not provided for detail case");
				isValid = false;
			}
		}

		// index parameter should not be used in show APIs
		if (requestBody.has(VtnServiceJsonConsts.INDEX)) {
			requestBody.remove(VtnServiceJsonConsts.INDEX);
		}

		LOG.trace("Complete VtnPolicingMapResourceValidator#validateGet()");
		return isValid;
	}
}
