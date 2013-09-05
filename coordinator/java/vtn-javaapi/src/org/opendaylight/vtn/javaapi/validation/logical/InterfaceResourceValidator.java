/*
 * Copyright (c) 2012-2013 NEC Corporation
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
import org.opendaylight.vtn.javaapi.resources.logical.DhcpRelayInterfaceResource;
import org.opendaylight.vtn.javaapi.resources.logical.DhcpRelayInterfacesResource;
import org.opendaylight.vtn.javaapi.resources.logical.VBridgeInterfaceResource;
import org.opendaylight.vtn.javaapi.resources.logical.VBridgeInterfacesResource;
import org.opendaylight.vtn.javaapi.resources.logical.VBypassInterfaceResource;
import org.opendaylight.vtn.javaapi.resources.logical.VBypassInterfacesResource;
import org.opendaylight.vtn.javaapi.resources.logical.VTepInterfaceResource;
import org.opendaylight.vtn.javaapi.resources.logical.VTepInterfacesResource;
import org.opendaylight.vtn.javaapi.resources.logical.VTunnelInterfaceResource;
import org.opendaylight.vtn.javaapi.resources.logical.VTunnelInterfacesResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class InterfaceResourceValidator validates request Json object for
 * Interface API.
 */
public class InterfaceResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(InterfaceResourceValidator.class.getName());
	private final AbstractResource resource;
	final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new interface resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public InterfaceResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for
	 * VUnknownInterfaceResource,VUnknownInterfacesResource
	 * ,VTunnelInterfaceResource,
	 * VTunnelInterfacesResource,VBridgeInterfaceResource
	 * ,VBridgeInterfacesResource,DhcpRelayInterfaceResource,
	 * DhcpRelayInterfacesResource,VTepInterfaceResource and
	 * VTepInterfacesResource
	 * 
	 * @return true, if successful
	 */
	@Override
	public boolean validateUri() {
		LOG.trace("Start InterfaceResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.VTNNAME);
		if (resource instanceof VBypassInterfaceResource
				&& ((VBypassInterfaceResource) resource).getVtnName() != null
				&& !((VBypassInterfaceResource) resource).getVtnName().trim()
				.isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VBypassInterfaceResource) resource).getVtnName().trim(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VBYPASS_NAME);
				if (((VBypassInterfaceResource) resource).getVbypassName() != null
						&& !((VBypassInterfaceResource) resource).getVbypassName()
						.trim().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VBypassInterfaceResource) resource).getVbypassName()
							.trim(), VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.IFNAME);
				if (((VBypassInterfaceResource) resource).getIfName() != null
						&& !((VBypassInterfaceResource) resource).getIfName()
						.trim().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VBypassInterfaceResource) resource).getIfName()
							.trim(), VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		} else if (resource instanceof VBypassInterfacesResource
				&& ((VBypassInterfacesResource) resource).getVtnName() != null
				&& !((VBypassInterfacesResource) resource).getVtnName().trim()
				.isEmpty()) {
			isValid = validator
					.isValidMaxLengthAlphaNum(
							((VBypassInterfacesResource) resource)
							.getVtnName().trim(),
							VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VBYPASS_NAME);
				if (((VBypassInterfacesResource) resource).getVbypassName() != null
						&& !((VBypassInterfacesResource) resource)
						.getVbypassName().trim().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VBypassInterfacesResource) resource)
							.getVbypassName().trim(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(true);
		} else if (resource instanceof VTunnelInterfaceResource
				&& ((VTunnelInterfaceResource) resource).getVtnName() != null
				&& !((VTunnelInterfaceResource) resource).getVtnName().trim()
				.trim().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VTunnelInterfaceResource) resource).getVtnName().trim(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VTUNNELNAME);
				if (((VTunnelInterfaceResource) resource).getvTunnelName() != null
						&& !((VTunnelInterfaceResource) resource)
						.getvTunnelName().trim().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VTunnelInterfaceResource) resource)
							.getvTunnelName().trim(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.IFNAME);
				if (((VTunnelInterfaceResource) resource).getIfName() != null
						&& !((VTunnelInterfaceResource) resource).getIfName()
						.trim().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VTunnelInterfaceResource) resource).getIfName()
							.trim(), VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		} else if (resource instanceof VTunnelInterfacesResource
				&& ((VTunnelInterfacesResource) resource).getVtnName() != null
				&& !((VTunnelInterfacesResource) resource).getVtnName().trim()
				.isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VTunnelInterfacesResource) resource).getVtnName().trim(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VTUNNELNAME);
				if (((VTunnelInterfacesResource) resource).getvTunnelName() != null
						&& !((VTunnelInterfacesResource) resource)
						.getvTunnelName().trim().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VTunnelInterfacesResource) resource)
							.getvTunnelName().trim(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(true);
		} else if (resource instanceof VBridgeInterfaceResource
				&& ((VBridgeInterfaceResource) resource).getVtnName() != null
				&& !((VBridgeInterfaceResource) resource).getVtnName().trim()
				.isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VBridgeInterfaceResource) resource).getVtnName().trim(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VBRNAME);
				if (((VBridgeInterfaceResource) resource).getVbrName() != null
						&& !((VBridgeInterfaceResource) resource).getVbrName()
						.trim().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VBridgeInterfaceResource) resource).getVbrName()
							.trim(), VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.IFNAME);
				if (((VBridgeInterfaceResource) resource).getIfName() != null
						&& !((VBridgeInterfaceResource) resource).getIfName()
						.trim().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VBridgeInterfaceResource) resource).getIfName()
							.trim(), VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		} else if (resource instanceof VBridgeInterfacesResource
				&& ((VBridgeInterfacesResource) resource).getVtnName() != null
				&& !((VBridgeInterfacesResource) resource).getVtnName().trim()
				.isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VBridgeInterfacesResource) resource).getVtnName().trim(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VBRNAME);
				if (((VBridgeInterfacesResource) resource).getVbrName() != null
						&& !((VBridgeInterfacesResource) resource).getVbrName()
						.trim().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VBridgeInterfacesResource) resource).getVbrName()
							.trim(), VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(true);
		} else if (resource instanceof DhcpRelayInterfaceResource
				&& ((DhcpRelayInterfaceResource) resource).getVtnName() != null
				&& !((DhcpRelayInterfaceResource) resource).getVtnName().trim()
				.isEmpty()) {
			isValid = validator
					.isValidMaxLengthAlphaNum(
							((DhcpRelayInterfaceResource) resource)
							.getVtnName().trim(),
							VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VRTNAME);
				if (((DhcpRelayInterfaceResource) resource).getVrtName() != null
						&& !((DhcpRelayInterfaceResource) resource)
						.getVrtName().trim().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((DhcpRelayInterfaceResource) resource)
							.getVrtName().trim(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.IFNAME);
				if (((DhcpRelayInterfaceResource) resource).getIfName() != null
						&& !((DhcpRelayInterfaceResource) resource).getIfName()
						.trim().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((DhcpRelayInterfaceResource) resource).getIfName()
							.trim(), VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		} else if (resource instanceof DhcpRelayInterfacesResource
				&& ((DhcpRelayInterfacesResource) resource).getVtnName() != null
				&& !((DhcpRelayInterfacesResource) resource).getVtnName()
				.trim().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((DhcpRelayInterfacesResource) resource).getVtnName()
					.trim(), VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VRTNAME);
				if (((DhcpRelayInterfacesResource) resource).getVrtName() != null
						&& !((DhcpRelayInterfacesResource) resource)
						.getVrtName().trim().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((DhcpRelayInterfacesResource) resource)
							.getVrtName().trim(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(true);
		} else if (resource instanceof VTepInterfaceResource
				&& ((VTepInterfaceResource) resource).getVtnName() != null
				&& !((VTepInterfaceResource) resource).getVtnName().trim()
				.isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VTepInterfaceResource) resource).getVtnName().trim(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VTEPNAME);
				if (((VTepInterfaceResource) resource).getvTepName() != null
						&& !((VTepInterfaceResource) resource).getvTepName()
						.trim().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VTepInterfaceResource) resource).getvTepName()
							.trim(), VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.IFNAME);
				if (((VTepInterfaceResource) resource).getIfName() != null
						&& !((VTepInterfaceResource) resource).getIfName()
						.trim().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VTepInterfaceResource) resource).getIfName()
							.trim(), VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		} else if (resource instanceof VTepInterfacesResource
				&& ((VTepInterfacesResource) resource).getVtnName() != null
				&& !((VTepInterfacesResource) resource).getVtnName().trim()
				.isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VTepInterfacesResource) resource).getVtnName().trim(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VTEPNAME);
				if (((VTepInterfacesResource) resource).getvTepName() != null
						&& !((VTepInterfacesResource) resource).getvTepName()
						.trim().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VTepInterfacesResource) resource).getvTepName()
							.trim(), VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(true);
		}
		LOG.trace("Complete InterfaceResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * validate request Json for get, put and post method of Interface API
	 */
	@Override
	public void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start InterfaceResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of InterfaceResourceValidator");
		boolean isValid = false;
		try {
			isValid = validateUri();

			if (isValid && requestBody != null
					&& VtnServiceConsts.GET.equals(method)) {
				if (resource instanceof DhcpRelayInterfacesResource) {
					isValid = validateOpDhcpInterface(requestBody, isValid);
				}
				if (isValid) {
					isValid = validator.isValidGet(requestBody, isListOpFlag());
					setInvalidParameter(validator.getInvalidParameter());
				}
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
			LOG.error("Inside catch:NumberFormatException");
			if (method.equals(VtnServiceConsts.GET)) {
				setInvalidParameter(validator.getInvalidParameter());
			}
			isValid = false;
		} catch (final ClassCastException e) {
			if (method.equals(VtnServiceConsts.GET)) {
				setInvalidParameter(validator.getInvalidParameter());
			}
			LOG.error("Inside catch:ClassCastException");
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
		LOG.trace("Complete InterfaceResourceValidator#validate()");

	}

	/**
	 * Validates op key for DHCP Interface API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @param isValid
	 *            boolean flag
	 * @return true, if successful
	 */
	private boolean validateOpDhcpInterface(final JsonObject requestBody,
			boolean isValid) {
		LOG.trace("Start InterfaceResourceValidator#validateOpDhcpInterface()");
		setInvalidParameter(VtnServiceJsonConsts.OP);
		if (requestBody.has(VtnServiceJsonConsts.OP)
				&& requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.OP)
				.getAsString() != null
				&& !requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.OP)
				.getAsString().trim().isEmpty()) {
			isValid = requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.OP)
					.getAsString().trim()
					.equalsIgnoreCase(VtnServiceJsonConsts.COUNT);
		}
		LOG.trace("Complete InterfaceResourceValidator#validateOpDhcpInterface()");
		return isValid;
	}

	/**
	 * validate post request Json object for Interface API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 */
	private boolean validatePost(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start InterfaceResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.INTERFACE);
		if (requestBody.has(VtnServiceJsonConsts.INTERFACE)
				&& requestBody.get(VtnServiceJsonConsts.INTERFACE)
				.isJsonObject()) {
			final JsonObject commonInterface = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.INTERFACE);
			// validation for key: if_name(mandatory)
			setInvalidParameter(VtnServiceJsonConsts.IFNAME);
			if (commonInterface.has(VtnServiceJsonConsts.IFNAME)
					&& commonInterface.getAsJsonPrimitive(
							VtnServiceJsonConsts.IFNAME).getAsString() != null
							&& !commonInterface
							.getAsJsonPrimitive(VtnServiceJsonConsts.IFNAME)
							.getAsString().trim().isEmpty()) {
				isValid = validator.isValidMaxLengthAlphaNum(commonInterface
						.getAsJsonPrimitive(VtnServiceJsonConsts.IFNAME)
						.getAsString().trim(), VtnServiceJsonConsts.LEN_31);
			}
			if (isValid) {
				isValid = validatePut(requestBody);
			}

		}
		LOG.trace("Complete InterfaceResourceValidator#validatePost()");
		return isValid;
	}

	/**
	 * validate put request Json object for Interface API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 */
	private boolean validatePut(final JsonObject requestBody) {
		LOG.trace("Start InterfaceResourceValidator#validatePut()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.INTERFACE);
		if (requestBody.has(VtnServiceJsonConsts.INTERFACE)
				&& requestBody.get(VtnServiceJsonConsts.INTERFACE)
				.isJsonObject()) {
			isValid = true;
			final JsonObject commonInterface = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.INTERFACE);
			// validation for key: description(optional)
			setInvalidParameter(VtnServiceJsonConsts.DESCRIPTION);
			if (commonInterface.has(VtnServiceJsonConsts.DESCRIPTION)
					&& commonInterface.getAsJsonPrimitive(
							VtnServiceJsonConsts.DESCRIPTION).getAsString() != null
							&& !commonInterface
							.getAsJsonPrimitive(
									VtnServiceJsonConsts.DESCRIPTION)
									.getAsString().trim().isEmpty()) {
				isValid = validator.isValidMaxLength(commonInterface
						.getAsJsonPrimitive(VtnServiceJsonConsts.DESCRIPTION)
						.getAsString().trim(), VtnServiceJsonConsts.LEN_127);
			}
			// validation for key: adminstatus(optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.ADMINSTATUS);
				if (commonInterface.has(VtnServiceJsonConsts.ADMINSTATUS)
						&& commonInterface.getAsJsonPrimitive(
								VtnServiceJsonConsts.ADMINSTATUS).getAsString() != null) {
					final String adminStatus = commonInterface
							.getAsJsonPrimitive(
									VtnServiceJsonConsts.ADMINSTATUS)
									.getAsString().trim();
					isValid = adminStatus
							.equalsIgnoreCase(VtnServiceJsonConsts.ENABLE)
							|| adminStatus
							.equalsIgnoreCase(VtnServiceJsonConsts.DISABLE);
				}
			}
		}
		LOG.trace("Complete InterfaceResourceValidator#validatePut()");
		return isValid;
	}
}
