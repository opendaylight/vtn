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
import org.opendaylight.vtn.javaapi.resources.logical.VTunnelResource;
import org.opendaylight.vtn.javaapi.resources.logical.VTunnelsResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class VTunnelResourceValidator validates request Json object for Vtunnel
 * API.
 */
public class VTunnelResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(VTunnelResourceValidator.class.getName());

	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new v tunnel resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public VTunnelResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for VTunnel API.
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start VTunnelResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.VTNNAME);
		if (resource instanceof VTunnelsResource
				&& ((VTunnelsResource) resource).getVtnName() != null
				&& !((VTunnelsResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VTunnelsResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			setListOpFlag(true);
		} else if (resource instanceof VTunnelResource
				&& ((VTunnelResource) resource).getVtnName() != null
				&& !((VTunnelResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VTunnelResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VTUNNELNAME);
				if (((VTunnelResource) resource).getvTunnelName() != null
						&& !((VTunnelResource) resource).getvTunnelName()
								.isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VTunnelResource) resource).getvTunnelName(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		}
		LOG.trace("Complete VTunnelResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request Json object for get, put and post method of VTunnel API.
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VTunnelResource#validate()");
		LOG.info("Validating request for " + method
				+ " of VTunnelResourceValidator");
		boolean isValid = false;
		try {
			isValid = validateUri();
			if (isValid && requestBody != null
					&& VtnServiceConsts.GET.equals(method)) {
				isValid = validator.isValidGet(requestBody, isListOpFlag());
				setInvalidParameter(validator.getInvalidParameter());
				updateOpParameterForList(requestBody);
			} else if (isValid && requestBody != null
					&& VtnServiceConsts.POST.equals(method)) {
				isValid = validatePost(requestBody);
			} else if (isValid && requestBody != null
					&& VtnServiceConsts.PUT.equals(method)) {
				isValid = validatePut(requestBody);
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
		LOG.trace("Complete VTunnelResourceValidator#validate()");
	}

	/**
	 * Validate post request Json object for VTunnel API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	private boolean validatePost(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VTunnelResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.VTUNNEL);
		if (requestBody.has(VtnServiceJsonConsts.VTUNNEL)
				&& requestBody.get(VtnServiceJsonConsts.VTUNNEL).isJsonObject()) {
			final JsonObject vTunnel = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VTUNNEL);
			// validation for key: vtunnel_name(mandatory)
			setInvalidParameter(VtnServiceJsonConsts.VTUNNELNAME);
			if (vTunnel.has(VtnServiceJsonConsts.VTUNNELNAME)
					&& vTunnel.getAsJsonPrimitive(
							VtnServiceJsonConsts.VTUNNELNAME).getAsString() != null) {
				isValid = validator.isValidMaxLengthAlphaNum(vTunnel
						.getAsJsonPrimitive(VtnServiceJsonConsts.VTUNNELNAME)
						.getAsString(), VtnServiceJsonConsts.LEN_31);
			}
			// validation for key: controller_id(mandatory)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.CONTROLLERID);
				if (vTunnel.has(VtnServiceJsonConsts.CONTROLLERID)
						&& vTunnel.getAsJsonPrimitive(
								VtnServiceJsonConsts.CONTROLLERID)
								.getAsString() != null) {
					isValid = validator
							.isValidMaxLengthAlphaNum(
									vTunnel.getAsJsonPrimitive(
											VtnServiceJsonConsts.CONTROLLERID)
											.getAsString(),
									VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}

			// validation for key: label(mandatory)
			setInvalidParameter(VtnServiceJsonConsts.LABEL);
			if (isValid
					&& vTunnel.has(VtnServiceJsonConsts.LABEL)
					&& vTunnel.getAsJsonPrimitive(VtnServiceJsonConsts.LABEL)
							.getAsString() != null) {
				isValid = validator.isValidRange(
						vTunnel.getAsJsonPrimitive(VtnServiceJsonConsts.LABEL)
								.getAsString(),
						VtnServiceJsonConsts.LONG_VAL_0,
						VtnServiceJsonConsts.LONG_VAL_4294967295);
			} else {
				isValid = false;
			}

			// validation for key: DomainId(mandatory)
			setInvalidParameter(VtnServiceJsonConsts.DOMAINID);
			if (isValid
					&& vTunnel.has(VtnServiceJsonConsts.DOMAINID)
					&& vTunnel
							.getAsJsonPrimitive(VtnServiceJsonConsts.DOMAINID)
							.getAsString() != null) {
				isValid = validator.isValidDomainId(
						vTunnel.getAsJsonPrimitive(
								VtnServiceJsonConsts.DOMAINID).getAsString(),
						VtnServiceJsonConsts.LEN_31);
			} else {
				isValid = false;
			}

			if (isValid) {
				// validation for key: description(optional),
				// vtn_name(optional), vtepgroup_name(optional)
				isValid = commonValidations(isValid, vTunnel);
			}

		}
		LOG.trace("complete VTunnelResourceValidator#validatePost()");
		return isValid;
	}

	/**
	 * @param isValid
	 * @param vTunnel
	 * @return
	 */
	private boolean commonValidations(boolean isValid, final JsonObject vTunnel) {
		LOG.trace("Start VTunnelResourceValidator#commonValidations()");
		// validation for key: description(optional)
		setInvalidParameter(VtnServiceJsonConsts.DESCRIPTION);
		if (vTunnel.has(VtnServiceJsonConsts.DESCRIPTION)
				&& vTunnel.getAsJsonPrimitive(VtnServiceJsonConsts.DESCRIPTION)
						.getAsString() != null) {
			isValid = validator
					.isValidMaxLength(
							vTunnel.getAsJsonPrimitive(
									VtnServiceJsonConsts.DESCRIPTION)
									.getAsString(),
							VtnServiceJsonConsts.LEN_127)
					|| vTunnel
							.getAsJsonPrimitive(
									VtnServiceJsonConsts.DESCRIPTION)
							.getAsString().isEmpty();
		}
		// validation for key: vtn_name(optional)
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.VTNNAME);
			if (vTunnel.has(VtnServiceJsonConsts.VTNNAME)
					&& vTunnel.getAsJsonPrimitive(VtnServiceJsonConsts.VTNNAME)
							.getAsString() != null) {
				isValid = validator.isValidMaxLengthAlphaNum(vTunnel
						.getAsJsonPrimitive(VtnServiceJsonConsts.VTNNAME)
						.getAsString(), VtnServiceJsonConsts.LEN_31)
						|| vTunnel
								.getAsJsonPrimitive(
										VtnServiceJsonConsts.VTNNAME)
								.getAsString().isEmpty();
			}
		}
		// validation for key: vtepgroup_name(optional)
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.VTEPGROUPNAME);
			if (vTunnel.has(VtnServiceJsonConsts.VTEPGROUPNAME)
					&& vTunnel.getAsJsonPrimitive(
							VtnServiceJsonConsts.VTEPGROUPNAME).getAsString() != null) {
				isValid = validator.isValidMaxLengthAlphaNum(vTunnel
						.getAsJsonPrimitive(VtnServiceJsonConsts.VTEPGROUPNAME)
						.getAsString(), VtnServiceJsonConsts.LEN_31)
						|| vTunnel
								.getAsJsonPrimitive(
										VtnServiceJsonConsts.VTEPGROUPNAME)
								.getAsString().isEmpty();
			}
		}
		LOG.trace("complete VTunnelResourceValidator#commonValidations()");
		return isValid;
	}

	/**
	 * Validate put request Json object for VTunnelResource
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	private boolean validatePut(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VTunnelResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.VTUNNEL);
		if (requestBody.has(VtnServiceJsonConsts.VTUNNEL)
				&& requestBody.get(VtnServiceJsonConsts.VTUNNEL).isJsonObject()) {
			isValid = true;
			final JsonObject vTunnel = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VTUNNEL);
			// validation for key: description(optional), vtn_name(optional),
			// vtepgroup_name(optional), label(optional)
			isValid = commonValidations(isValid, vTunnel);
			// validation for key: controller_id
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.CONTROLLERID);
				if (vTunnel.has(VtnServiceJsonConsts.CONTROLLERID)) {
					isValid = false;
				}
			}
			// validation for key: DomainId
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.DOMAINID);
				if (vTunnel.has(VtnServiceJsonConsts.DOMAINID)) {
					isValid = false;
				}
			}
			// validation for key: label
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.LABEL);
				if (vTunnel.has(VtnServiceJsonConsts.LABEL)) {
					isValid = false;
				}
			}
		}
		LOG.trace("Complete VTunnelResourceValidator#validatePost()");
		return isValid;
	}
}
