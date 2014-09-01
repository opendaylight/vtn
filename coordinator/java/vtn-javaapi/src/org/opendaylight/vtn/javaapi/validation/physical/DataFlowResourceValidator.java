/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi.validation.physical;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.resources.physical.DataFlowResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class DataFlowResourceValidator validates request Json object for
 * DataFlowResource API.
 */
public class DataFlowResourceValidator extends VtnServiceValidator {
	/**
	 * Logger to debug.
	 */
	private static final Logger LOG = Logger
			.getLogger(DataFlowResourceValidator.class.getName());
	/**
	 * Abstract class resource reference.
	 */
	private final AbstractResource resource;
	/**
	 * Common validator class object to invoke common validation methods.
	 */
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new Data Flow resource validator. the instance of
	 * AbstractResource
	 */
	/**
	 * @param mappingResource
	 *            ,resource class object
	 */
	public DataFlowResourceValidator(final AbstractResource mappingResource) {
		this.resource = mappingResource;
	}

	/**
	 * Validate uri for DataFlowResource API.
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start DataFlowResourceValidator#validateUri()");
		boolean isValid = false;
		if (resource instanceof DataFlowResource) {
			isValid = true;
			setListOpFlag(true);
		}
		LOG.trace("Complete DataFlowResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request JSON for get methods of Data Flow API.
	 */
	/**
	 * @param method
	 *            , contains info about get,post ,delete.
	 * @param requestBody
	 *            , contains request parameters.
	 * @throws VtnServiceException
	 *             , vtn exception is thrown.
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start DataFlowResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of DataFlowResourceValidator");
		boolean isValid = false;
		try {
			isValid = validateUri();
			if (isValid && requestBody != null
					&& VtnServiceConsts.GET.equals(method)) {
				isValid = isValidateGet(requestBody, isListOpFlag());
				updateOpParameterForList(requestBody);
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
		LOG.trace("Complete DataFlowResourceValidator#validate()");
	}

	/**
	 * Validate request json for get method of Data Flow API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful.
	 * @param opFlag
	 *            ,set for operation type
	 */
	private boolean isValidateGet(final JsonObject requestBody,
			final boolean opFlag) {
		LOG.trace("Start DataFlowResourceValidator#isValidateGet()");
		boolean isValid = false;
		// validation for key:controller_id(mandatory)
		setInvalidParameter(VtnServiceJsonConsts.CONTROLLERID);
		if (requestBody.has(VtnServiceJsonConsts.CONTROLLERID)
				&& requestBody.getAsJsonPrimitive(
						VtnServiceJsonConsts.CONTROLLERID).getAsString() != null
				&& !requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.CONTROLLERID)
						.getAsString().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(requestBody
					.getAsJsonPrimitive(VtnServiceJsonConsts.CONTROLLERID)
					.getAsString(), VtnServiceJsonConsts.LEN_31);
		}
		// validation for key:switch_id(mandatory)
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.SWITCHID);
			if (requestBody.has(VtnServiceJsonConsts.SWITCHID)
					&& requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.SWITCHID).getAsString() != null
					&& !requestBody
							.getAsJsonPrimitive(VtnServiceJsonConsts.SWITCHID)
							.getAsString().isEmpty()) {
				isValid = validator.isValidMaxLength(requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.SWITCHID)
						.getAsString(), VtnServiceJsonConsts.LEN_255);
			} else {
				isValid = false;
			}
		}
		// validation for key:port_name(mandatory)
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.PORTNAME);
			if (requestBody.has(VtnServiceJsonConsts.PORTNAME)
					&& requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.PORTNAME).getAsString() != null
					&& !requestBody
							.getAsJsonPrimitive(VtnServiceJsonConsts.PORTNAME)
							.getAsString().isEmpty()) {
				isValid = validator.isValidMaxLength(requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.PORTNAME)
						.getAsString(), VtnServiceJsonConsts.LEN_31);
			} else {
				isValid = false;
			}
		}

		// validation for key: vlan_id or no_vlan_id(mandatory one of them)
		if (isValid) {
			// only vlan_id or no_vlan_id is allowed
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
		LOG.trace("Complete DataFlowResourceValidator#isValidateGet");
		return isValid;
	}
}
