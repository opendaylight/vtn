/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.validation.logical;

import java.util.Iterator;

import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.resources.logical.VTepGroupResource;
import org.opendaylight.vtn.javaapi.resources.logical.VTepGroupsResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class VTepGroupResourceValidator validates request Json object for
 * VtepGroup API.
 */
public class VTepGroupResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(VTepGroupResourceValidator.class.getName());

	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new v tep group resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public VTepGroupResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for VTepGroup API.
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start VTepGroupResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.VTNNAME);
		if (resource instanceof VTepGroupsResource
				&& ((VTepGroupsResource) resource).getVtnName() != null
				&& !((VTepGroupsResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VTepGroupsResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			setListOpFlag(true);
		} else if (resource instanceof VTepGroupResource
				&& ((VTepGroupResource) resource).getVtnName() != null
				&& !((VTepGroupResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VTepGroupResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VTEPGROUPNAME);
				if (((VTepGroupResource) resource).getVtepgroupName() != null
						&& !((VTepGroupResource) resource).getVtepgroupName()
								.isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VTepGroupResource) resource).getVtepgroupName(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		}
		LOG.trace("Complete VTepGroupResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request Json object for get, put and post method of VTepGroup
	 * API.
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VTepGroupResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of VTepGroupResourceValidator");
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
		LOG.trace("Complete VTepGroupResourceValidator#validate()");
	}

	/**
	 * validate post request Json object for VTepGroupresource
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	private boolean validatePost(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VTepGroupResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.VTEPGROUP);
		if (requestBody.has(VtnServiceJsonConsts.VTEPGROUP)
				&& requestBody.get(VtnServiceJsonConsts.VTEPGROUP)
						.isJsonObject()) {
			final JsonObject vTep = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VTEPGROUP);
			// validation for key: vtepgroup_name(mandatory)
			setInvalidParameter(VtnServiceJsonConsts.VTEPGROUPNAME);
			if (vTep.has(VtnServiceJsonConsts.VTEPGROUPNAME)
					&& vTep.getAsJsonPrimitive(
							VtnServiceJsonConsts.VTEPGROUPNAME).getAsString() != null) {
				isValid = validator.isValidMaxLengthAlphaNum(vTep
						.getAsJsonPrimitive(VtnServiceJsonConsts.VTEPGROUPNAME)
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
			// validation for key: description (optional)
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
			if (isValid) {
				isValid = validatePut(requestBody);
			}
		}
		LOG.trace("Complete VTepGroupResourceValidator#validatePost()");
		return isValid;
	}

	/**
	 * validate put request Json object for VTepGroup API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	private boolean validatePut(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VTepGroupResourceValidator#validatePut()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.VTEPGROUP);
		if (requestBody.has(VtnServiceJsonConsts.VTEPGROUP)
				&& requestBody.get(VtnServiceJsonConsts.VTEPGROUP)
						.isJsonObject()) {
			isValid = true;
			final JsonObject vTep = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VTEPGROUP);
			setInvalidParameter(VtnServiceJsonConsts.MEMBERVTEPS);
			if (vTep.has(VtnServiceJsonConsts.MEMBERVTEPS)
					&& vTep.get(VtnServiceJsonConsts.MEMBERVTEPS).isJsonArray()) {
				final JsonArray memberVtep = vTep
						.getAsJsonArray(VtnServiceJsonConsts.MEMBERVTEPS);
				final Iterator<JsonElement> vtep = memberVtep.iterator();
				while (vtep.hasNext()) {
					if (isValid) {
						setInvalidParameter(VtnServiceJsonConsts.VTEPNAME);
						final JsonObject vtepName = (JsonObject) vtep.next();
						// validation for key: vtep_name(optinal)
						if (vtepName.has(VtnServiceJsonConsts.VTEPNAME)
								&& vtepName.getAsJsonPrimitive(
										VtnServiceJsonConsts.VTEPNAME)
										.getAsString() != null
								&& !vtepName
										.getAsJsonPrimitive(
												VtnServiceJsonConsts.VTEPNAME)
										.getAsString().isEmpty()) {
							isValid = validator.isValidMaxLengthAlphaNum(
									vtepName.getAsJsonPrimitive(
											VtnServiceJsonConsts.VTEPNAME)
											.getAsString(),
									VtnServiceJsonConsts.LEN_31);
						}
					}
				}
			}
		}
		LOG.trace("Complete VTepGroupResourceValidator#validatePut()");
		return isValid;
	}
}
