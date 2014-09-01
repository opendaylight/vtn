/*
 * Copyright (c) 2013-2014 NEC Corporation
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
import org.opendaylight.vtn.javaapi.resources.logical.VtnDataFlowResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class VtnDataFlowResourceValidator validates request Json and URI object
 * for VTN Data Flow .
 */
public class VtnDataFlowResourceValidator extends VtnServiceValidator {
	/**
	 * logger for debugging purpose.
	 */
	private static final Logger LOG = Logger
			.getLogger(VtnDataFlowResourceValidator.class.getName());

	/**
	 * Abstract resouce reference.
	 */
	private final AbstractResource resource;
	/**
	 * Common validator object for common validation methods.
	 */
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new VTN Data Flow resource validator.
	 * 
	 * @param resource1
	 *            the instance of AbstractResource.
	 */
	public VtnDataFlowResourceValidator(final AbstractResource resource1) {
		this.resource = resource1;
	}

	/**
	 * Validate URI parameters for VTN Data Flow API.
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start VtnDataFlowResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.VTNNAME);
		if (resource instanceof VtnDataFlowResource
				&& ((VtnDataFlowResource) resource).getVtnName() != null
				&& !((VtnDataFlowResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VtnDataFlowResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
		}
		LOG.trace("Complete VtnDataFlowResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request JSON for get methods of VTN Data Flow API.
	 */
	/**
	 * @param method
	 *            , for put get post delete.
	 * @param requestBody
	 *            , for requested params
	 * @throws VtnServiceException
	 *             ,in case of exception regarding vtn
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VtnDataFlowResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of VtnDataFlowResourceValidator");
		boolean isValid = false;
		try {
			isValid = validateUri();
			if (isValid && requestBody != null
					&& VtnServiceConsts.GET.equals(method)) {
				isValid = isValidateGet(requestBody);
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
		LOG.trace("Complete VtnDataFlowResourceValidator#validate()");
	}

	/**
	 * Validate request JSON for get method of VTN Data Flow API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 */
	private boolean isValidateGet(final JsonObject requestBody) {
		LOG.trace("Start VtnDataFlowResourceValidator#isValidateGet()");
		boolean isValid = true;
		// validation for key:vnode_name(mandatory)
		setInvalidParameter(VtnServiceJsonConsts.VNODENAME);

		if (requestBody.has(VtnServiceJsonConsts.VNODENAME)
				&& requestBody.getAsJsonPrimitive(
						VtnServiceJsonConsts.VNODENAME).getAsString() != null
				&& !requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.VNODENAME)
						.getAsString().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(requestBody
					.getAsJsonPrimitive(VtnServiceJsonConsts.VNODENAME)
					.getAsString(), VtnServiceJsonConsts.LEN_31);
		} else {
			isValid = false;
		}
		// validation for key: srcmacaddr(mandatory)
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.SRCMACADDR);
			if (requestBody.has(VtnServiceJsonConsts.SRCMACADDR)
					&& requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.SRCMACADDR).getAsString() != null
					&& !requestBody
							.getAsJsonPrimitive(VtnServiceJsonConsts.SRCMACADDR)
							.getAsString().isEmpty()) {
				isValid = validator.isValidMacAddress(requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.SRCMACADDR)
						.getAsString());
			} else {
				isValid = false;
			}
		}

		// validation for key: vlan_id or no_vlan_id(One of them is Mandatory)
		if (isValid) {
			// Only vlan_id or no_vlan_id is allowed in request body
			setInvalidParameter(VtnServiceJsonConsts.VLANID
					+ VtnServiceJsonConsts.SLASH
					+ VtnServiceJsonConsts.NO_VLAN_ID);
			if (requestBody.has(VtnServiceJsonConsts.VLANID)
					&& requestBody.has(VtnServiceJsonConsts.NO_VLAN_ID)) {
				isValid = false;
			} else if (requestBody.has(VtnServiceJsonConsts.VLANID)) {
				setInvalidParameter(VtnServiceJsonConsts.VLANID);
				if (requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.VLANID)
						.getAsString() != null
						&& !requestBody
								.getAsJsonPrimitive(VtnServiceJsonConsts.VLANID)
								.getAsString().isEmpty()) {
					isValid = validator.isValidRange(requestBody
							.getAsJsonPrimitive(VtnServiceJsonConsts.VLANID)
							.getAsString(), VtnServiceJsonConsts.VAL_1,
							VtnServiceJsonConsts.VAL_4095);
				} else {
					isValid = false;
				}
			} else if (requestBody.has(VtnServiceJsonConsts.NO_VLAN_ID)) {
				setInvalidParameter(VtnServiceJsonConsts.NO_VLAN_ID);
				if (requestBody.getAsJsonPrimitive(
						VtnServiceJsonConsts.NO_VLAN_ID).getAsString() != null
						&& !requestBody
								.getAsJsonPrimitive(
										VtnServiceJsonConsts.NO_VLAN_ID)
								.getAsString().isEmpty()) {
					isValid = requestBody
							.getAsJsonPrimitive(VtnServiceJsonConsts.NO_VLAN_ID)
							.getAsString()
							.equalsIgnoreCase(VtnServiceJsonConsts.TRUE);
				} else {
					isValid = false;

				}
			} else {
				isValid = false;
			}
		}

		LOG.trace("Complete VtnDataFlowResourceValidator#isValidateGet");
		return isValid;
	}
}
