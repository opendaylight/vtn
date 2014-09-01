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
import org.opendaylight.vtn.javaapi.resources.logical.VlanMapResource;
import org.opendaylight.vtn.javaapi.resources.logical.VlanMapsResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class VLanMapResourceValidator validates request Json Object for VlanMap
 * API.
 */
public class VLanMapResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(VLanMapResourceValidator.class.getName());

	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new vlan map resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public VLanMapResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for VLanMap API.
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start VLanMapResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.VTNNAME);
		if (resource instanceof VlanMapsResource
				&& ((VlanMapsResource) resource).getVtnName() != null
				&& !((VlanMapsResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VlanMapsResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VBRNAME);
				if (((VlanMapsResource) resource).getVbrName() != null
						&& !((VlanMapsResource) resource).getVbrName()
								.isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VlanMapsResource) resource).getVbrName(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(true);
		} else if (resource instanceof VlanMapResource
				&& ((VlanMapResource) resource).getVtnName() != null
				&& !((VlanMapResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VlanMapResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VBRNAME);
				if (((VlanMapResource) resource).getVbrName() != null
						&& !((VlanMapResource) resource).getVbrName().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VlanMapResource) resource).getVbrName(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VLANMAPID);
				if (((VlanMapResource) resource).getVlanMapId() != null
						&& !((VlanMapResource) resource).getVlanMapId()
								.isEmpty()) {
					final String vlamMapId = ((VlanMapResource) resource)
							.getVlanMapId();
					isValid = isValidVLanMapId(vlamMapId);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		}
		LOG.trace("Completed VLanMapResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * @param vlamMapId
	 * @return
	 */
	private boolean isValidVLanMapId(final String vlamMapId) {
		LOG.trace("Start VLanMapResourceValidator#isValidVLanMapId()");
		boolean isValid;
		if (vlamMapId.contains(VtnServiceJsonConsts.LPID
				+ VtnServiceJsonConsts.VLANMAPIDSEPERATOR)) {
			isValid = validator.isValidMaxLength(vlamMapId.substring(5),
					VtnServiceJsonConsts.LEN_319);
		} else if (vlamMapId.contains(VtnServiceJsonConsts.NOLPID)) {
			isValid = true;
		} else {
			isValid = false;
		}
		LOG.trace("Completed VLanMapResourceValidator#isValidVLanMapId()");
		return isValid;
	}

	/**
	 * Validate request Json object for get, put and post method of VLanMap API.
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VLanMapResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of VLanMapResourceValidator");
		boolean isValid = false;
		try {
			isValid = validateUri();
			if (isValid && requestBody != null
					&& VtnServiceConsts.GET.equals(method)) {
				isValid = validateGet(requestBody, isListOpFlag());
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
		LOG.trace("Complete VLanMapResourceValidator#validate()");
	}

	/**
	 * Validate get request Json object for VLanMap API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validateGet(final JsonObject requestBody,
			final boolean opFlag) {

		LOG.trace("Start VLanMapValiadtor#isValidGet");
		boolean isValid = true;
		// validation for key: targetdb
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.TARGETDB);
			isValid = validator.isValidRequestDB(requestBody);
		}
		if (!opFlag) {
			if (requestBody.has(VtnServiceJsonConsts.OP)) {
				requestBody.remove(VtnServiceJsonConsts.OP);
			} else {
				LOG.debug("No need to remove -op ");
			}
			if (requestBody.has(VtnServiceJsonConsts.INDEX)) {
				requestBody.remove(VtnServiceJsonConsts.INDEX);
			} else {
				LOG.debug("No need to remove - index");
			}
			if (requestBody.has(VtnServiceJsonConsts.MAX)) {
				requestBody.remove(VtnServiceJsonConsts.MAX);
			} else {
				LOG.debug("No need to remove-max count");
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
								VtnServiceJsonConsts.INDEX).getAsString() != null) {

					final String vlamMapId = requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.INDEX).getAsString();
					isValid = isValidVLanMapId(vlamMapId);
				}
			}
			// validation for key: max_repitition
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.MAX);
				isValid = validator.isValidMaxRepetition(requestBody);
			}
		}
		LOG.trace("Complete VLanMapValidator#isValidGet");
		return isValid;
	}

	/**
	 * Validate post request Json object for VLanMap API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	private boolean validatePost(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VLanMapValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.VLANMAP);
		if (requestBody.has(VtnServiceJsonConsts.VLANMAP)
				&& requestBody.get(VtnServiceJsonConsts.VLANMAP).isJsonObject()) {
			isValid = true;
			final JsonObject vLanMap = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VLANMAP);
			// validation for key: logical_port_id(optional)
			setInvalidParameter(VtnServiceJsonConsts.LOGICAL_PORT_ID);
			if (vLanMap.has(VtnServiceJsonConsts.LOGICAL_PORT_ID)
					&& vLanMap.getAsJsonPrimitive(
							VtnServiceJsonConsts.LOGICAL_PORT_ID).getAsString() != null
					&& !vLanMap
							.getAsJsonPrimitive(
									VtnServiceJsonConsts.LOGICAL_PORT_ID)
							.getAsString().isEmpty()) {
				isValid = validator.isValidMaxLength(
						vLanMap.getAsJsonPrimitive(
								VtnServiceJsonConsts.LOGICAL_PORT_ID)
								.getAsString(), VtnServiceJsonConsts.LEN_319);
			}
			if (isValid) {
				isValid = validatePut(requestBody);
			}

		}
		LOG.trace("Complete VLanMapValidator#validatePost()");
		return isValid;
	}

	/**
	 * Validate put request Json object for VLanMap API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	private boolean validatePut(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VLanMapResourceValidator#validatePut()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.VLANMAP);
		if (requestBody.has(VtnServiceJsonConsts.VLANMAP)
				&& requestBody.get(VtnServiceJsonConsts.VLANMAP).isJsonObject()) {
			final JsonObject vLanMap = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VLANMAP);
			// only vlan_id or no_vlan_id is allowed
			setInvalidParameter(VtnServiceJsonConsts.VLANID
					+ VtnServiceJsonConsts.SLASH
					+ VtnServiceJsonConsts.NO_VLAN_ID);
			if (vLanMap.has(VtnServiceJsonConsts.VLANID)
					&& vLanMap.has(VtnServiceJsonConsts.NO_VLAN_ID)) {
				isValid = false;
			} else if (vLanMap.has(VtnServiceJsonConsts.VLANID)) {
				setInvalidParameter(VtnServiceJsonConsts.VLANID);
				if (vLanMap.getAsJsonPrimitive(VtnServiceJsonConsts.VLANID)
						.getAsString() != null
						&& !vLanMap
								.getAsJsonPrimitive(VtnServiceJsonConsts.VLANID)
								.getAsString().isEmpty()) {
					// validation for key: vlan_id
					isValid = validator.isValidRange(vLanMap
							.getAsJsonPrimitive(VtnServiceJsonConsts.VLANID)
							.getAsString(), VtnServiceJsonConsts.VAL_1,
							VtnServiceJsonConsts.VAL_4095);
				} else {
					isValid = false;
				}
			} else if (vLanMap.has(VtnServiceJsonConsts.NO_VLAN_ID)) {
				setInvalidParameter(VtnServiceJsonConsts.NO_VLAN_ID);
				if (vLanMap.getAsJsonPrimitive(VtnServiceJsonConsts.NO_VLAN_ID)
						.getAsString() != null
						&& !vLanMap
								.getAsJsonPrimitive(
										VtnServiceJsonConsts.NO_VLAN_ID)
								.getAsString().isEmpty()) {
					// validation for key: no_vlan_id
					final String no_vlan_id = vLanMap.getAsJsonPrimitive(
							VtnServiceJsonConsts.NO_VLAN_ID).getAsString();
					isValid = no_vlan_id
							.equalsIgnoreCase(VtnServiceJsonConsts.TRUE);
				} else {
					isValid = false;
				}
			} else {
				isValid = false;
			}

		} else {
			isValid = false;
		}
		LOG.trace("Complete VLanMapResourceValidator#validatePut()");
		return isValid;
	}
}
