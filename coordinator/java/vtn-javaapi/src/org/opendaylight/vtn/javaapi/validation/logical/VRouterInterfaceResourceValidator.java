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
import org.opendaylight.vtn.javaapi.resources.logical.VRouterInterfaceResource;
import org.opendaylight.vtn.javaapi.resources.logical.VRouterInterfacesResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class VRouterInterfaceResourceValidator validates request Json object for
 * VrouterInterface API.
 */
public class VRouterInterfaceResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(VRouterInterfaceResourceValidator.class.getName());

	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new vrouter interface resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public VRouterInterfaceResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for VrouterInterface API
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start VRouterInterfaceResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.VTNNAME);
		if (resource instanceof VRouterInterfacesResource
				&& ((VRouterInterfacesResource) resource).getVtnName() != null
				&& !((VRouterInterfacesResource) resource).getVtnName()
						.isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VRouterInterfacesResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VRTNAME);
				if (((VRouterInterfacesResource) resource).getVrtName() != null
						&& !((VRouterInterfacesResource) resource).getVrtName()
								.isEmpty()) {
					isValid = validator
							.isValidMaxLengthAlphaNum(
									((VRouterInterfacesResource) resource)
											.getVrtName(),
									VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(true);
		} else if (resource instanceof VRouterInterfaceResource
				&& ((VRouterInterfaceResource) resource).getVtnName() != null
				&& !((VRouterInterfaceResource) resource).getVtnName()
						.isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VRouterInterfaceResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VRTNAME);
				if (((VRouterInterfaceResource) resource).getVrtName() != null
						&& !((VRouterInterfaceResource) resource).getVrtName()
								.isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VRouterInterfaceResource) resource).getVrtName(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.IFNAME);
				if (((VRouterInterfaceResource) resource).getIfName() != null
						&& !((VRouterInterfaceResource) resource).getIfName()
								.isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VRouterInterfaceResource) resource).getIfName(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		}
		LOG.trace("Complete VRouterInterfaceResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request Json object for get, put and post method of
	 * VrouterInterface API
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VRouterInterfaceResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of VRouterInterfaceResourceValidator");
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
		LOG.trace("Complete VRouterInterfaceResourceValidator#validate()");
	}

	/**
	 * validates post request Json object for VrouterInterface API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	private boolean validatePost(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VRouterInterfaceResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.INTERFACE);
		if (requestBody.has(VtnServiceJsonConsts.INTERFACE)
				&& requestBody.get(VtnServiceJsonConsts.INTERFACE)
						.isJsonObject()) {
			final JsonObject vRouterInterface = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.INTERFACE);
			// validation for key: if_name(mandatory)
			setInvalidParameter(VtnServiceJsonConsts.IFNAME);
			if (vRouterInterface.has(VtnServiceJsonConsts.IFNAME)
					&& vRouterInterface.getAsJsonPrimitive(
							VtnServiceJsonConsts.IFNAME).getAsString() != null) {
				isValid = validator.isValidMaxLengthAlphaNum(vRouterInterface
						.getAsJsonPrimitive(VtnServiceJsonConsts.IFNAME)
						.getAsString(), VtnServiceJsonConsts.LEN_31);
			}
			if (isValid) {
				isValid = validatePut(requestBody);
			}

		}
		LOG.trace("Complete VRouterInterfaceResourceValidator#validatePost()");
		return isValid;
	}

	/**
	 * Validate put request Json object for VrouterInterface API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	private boolean validatePut(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VRouterInterfaceResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.INTERFACE);
		if (requestBody.has(VtnServiceJsonConsts.INTERFACE)
				&& requestBody.get(VtnServiceJsonConsts.INTERFACE)
						.isJsonObject()) {
			isValid = true;
			final JsonObject vRouterInterface = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.INTERFACE);
			// validation for key: description(optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.DESCRIPTION);
				if (vRouterInterface.has(VtnServiceJsonConsts.DESCRIPTION)
						&& vRouterInterface.getAsJsonPrimitive(
								VtnServiceJsonConsts.DESCRIPTION).getAsString() != null) {
					isValid = validator.isValidMaxLength(
							vRouterInterface.getAsJsonPrimitive(
									VtnServiceJsonConsts.DESCRIPTION)
									.getAsString(),
							VtnServiceJsonConsts.LEN_127)
							|| vRouterInterface
									.getAsJsonPrimitive(
											VtnServiceJsonConsts.DESCRIPTION)
									.getAsString().isEmpty();
				}
			}
			// validation for key: adminstatus(optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.ADMINSTATUS);
				if (vRouterInterface.has(VtnServiceJsonConsts.ADMINSTATUS)
						&& vRouterInterface.getAsJsonPrimitive(
								VtnServiceJsonConsts.ADMINSTATUS).getAsString() != null) {
					final String adminStatus = vRouterInterface
							.getAsJsonPrimitive(
									VtnServiceJsonConsts.ADMINSTATUS)
							.getAsString();
					isValid = adminStatus
							.equalsIgnoreCase(VtnServiceJsonConsts.ENABLE)
							|| adminStatus
									.equalsIgnoreCase(VtnServiceJsonConsts.DISABLE);
				}
			}
			// validation for key: ipaddr(optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.IPADDR);
				if (vRouterInterface.has(VtnServiceJsonConsts.IPADDR)
						&& vRouterInterface.getAsJsonPrimitive(
								VtnServiceJsonConsts.IPADDR).getAsString() != null) {
					isValid = validator.isValidIpV4(vRouterInterface
							.getAsJsonPrimitive(VtnServiceJsonConsts.IPADDR)
							.getAsString());
				}
			}
			// validation for key: netmask(optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.PREFIX);
				if (vRouterInterface.has(VtnServiceJsonConsts.PREFIX)) {
					isValid = validator.isValidRange(vRouterInterface
							.getAsJsonPrimitive(VtnServiceJsonConsts.PREFIX)
							.getAsString(), VtnServiceJsonConsts.VAL_1,
							VtnServiceJsonConsts.VAL_30);
				}
			}
			// validation for key: macaddr(optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.MACADDR);
				if (vRouterInterface.has(VtnServiceJsonConsts.MACADDR)
						&& vRouterInterface.getAsJsonPrimitive(
								VtnServiceJsonConsts.MACADDR).getAsString() != null) {
					isValid = validator.isValidMacAddress(vRouterInterface
							.getAsJsonPrimitive(VtnServiceJsonConsts.MACADDR)
							.getAsString());
				}
			}
		}
		LOG.trace("Complete VRouterInterfaceResourceValidator#validatePost()");
		return isValid;
	}
}
