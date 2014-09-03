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
import org.opendaylight.vtn.javaapi.resources.logical.VBridgeFlowFilterEntriesResource;
import org.opendaylight.vtn.javaapi.resources.logical.VBridgeInterfaceFlowFilterEntriesResource;
import org.opendaylight.vtn.javaapi.resources.logical.VRouterInterfaceFlowFilterEntriesResource;
import org.opendaylight.vtn.javaapi.resources.logical.VTerminalInterfaceFlowFilterEntriesResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class VBridgeFlowFilterEntriesResourceValidator validates request Json
 * object for VbridgeFlowFilterEntry, VbridgeInetrfaceFlowFilterEntry and
 * VrouterInterfaceFlowFilterEntry API.
 */
public class VBridgeFlowFilterEntriesResourceValidator extends
		VtnServiceValidator {
	private static final Logger LOG = Logger
			.getLogger(VBridgeFlowFilterEntriesResourceValidator.class
					.getName());
	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new v bridge flow filter entries resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public VBridgeFlowFilterEntriesResourceValidator(
			final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for VBridgeFlowFilterEntriesResource,
	 * VBridgeInterfaceFlowFilterEntriesResource,
	 * VRouterInterfaceFlowFilterEntriesResource
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start VBridgeFlowFilterEntriesResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.VTNNAME);
		// For VBridgeFlowFilterEntriesResource instance
		if (resource instanceof VBridgeFlowFilterEntriesResource
				&& ((VBridgeFlowFilterEntriesResource) resource).getVtnName() != null
				&& !((VBridgeFlowFilterEntriesResource) resource).getVtnName()
						.isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VBridgeFlowFilterEntriesResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VBRNAME);
				if (((VBridgeFlowFilterEntriesResource) resource).getVbrName() != null
						&& !((VBridgeFlowFilterEntriesResource) resource)
								.getVbrName().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VBridgeFlowFilterEntriesResource) resource)
									.getVbrName(), VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.FFTYPE);
				if (((VBridgeFlowFilterEntriesResource) resource).getFfType() != null
						&& !((VBridgeFlowFilterEntriesResource) resource)
								.getFfType().isEmpty()) {
					isValid = VtnServiceJsonConsts.IN
							.equalsIgnoreCase(((VBridgeFlowFilterEntriesResource) resource)
									.getFfType())
							|| VtnServiceJsonConsts.OUT
									.equalsIgnoreCase(((VBridgeFlowFilterEntriesResource) resource)
											.getFfType());
				} else {
					isValid = false;
				}
			}
			setListOpFlag(true);
		} else if (resource instanceof VBridgeInterfaceFlowFilterEntriesResource
				&& ((VBridgeInterfaceFlowFilterEntriesResource) resource)
						.getVtnName() != null
				&& !((VBridgeInterfaceFlowFilterEntriesResource) resource)
						.getVtnName().isEmpty()) {
			// For VBridgeInterfaceFlowFilterEntriesResource instance
			isValid = validator.isValidMaxLengthAlphaNum(
					((VBridgeInterfaceFlowFilterEntriesResource) resource)
							.getVtnName(), VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VBRNAME);
				if (((VBridgeInterfaceFlowFilterEntriesResource) resource)
						.getVbrName() != null
						&& !((VBridgeInterfaceFlowFilterEntriesResource) resource)
								.getVbrName().isEmpty()) {
					isValid = validator
							.isValidMaxLengthAlphaNum(
									((VBridgeInterfaceFlowFilterEntriesResource) resource)
											.getVbrName(),
									VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.IFNAME);
				if (((VBridgeInterfaceFlowFilterEntriesResource) resource)
						.getIfName() != null
						&& !((VBridgeInterfaceFlowFilterEntriesResource) resource)
								.getIfName().isEmpty()) {
					isValid = validator
							.isValidMaxLengthAlphaNum(
									((VBridgeInterfaceFlowFilterEntriesResource) resource)
											.getIfName(),
									VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.FFTYPE);
				if (((VBridgeInterfaceFlowFilterEntriesResource) resource)
						.getFfType() != null
						&& !((VBridgeInterfaceFlowFilterEntriesResource) resource)
								.getFfType().isEmpty()) {
					isValid = VtnServiceJsonConsts.IN
							.equalsIgnoreCase(((VBridgeInterfaceFlowFilterEntriesResource) resource)
									.getFfType())
							|| VtnServiceJsonConsts.OUT
									.equalsIgnoreCase(((VBridgeInterfaceFlowFilterEntriesResource) resource)
											.getFfType());
				} else {
					isValid = false;
				}
			}
			setListOpFlag(true);
		} else if (resource instanceof VRouterInterfaceFlowFilterEntriesResource
				&& ((VRouterInterfaceFlowFilterEntriesResource) resource)
						.getVtnName() != null
				&& !((VRouterInterfaceFlowFilterEntriesResource) resource)
						.getVtnName().isEmpty()) {
			// For VRouterInterfaceFlowFilterEntriesResource instance
			isValid = validator.isValidMaxLengthAlphaNum(
					((VRouterInterfaceFlowFilterEntriesResource) resource)
							.getVtnName(), VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VRTNAME);
				if (((VRouterInterfaceFlowFilterEntriesResource) resource)
						.getVrtName() != null
						&& !((VRouterInterfaceFlowFilterEntriesResource) resource)
								.getVrtName().isEmpty()) {
					isValid = validator
							.isValidMaxLengthAlphaNum(
									((VRouterInterfaceFlowFilterEntriesResource) resource)
											.getVrtName(),
									VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.IFNAME);
				if (((VRouterInterfaceFlowFilterEntriesResource) resource)
						.getIfName() != null
						&& !((VRouterInterfaceFlowFilterEntriesResource) resource)
								.getIfName().isEmpty()) {
					isValid = validator
							.isValidMaxLengthAlphaNum(
									((VRouterInterfaceFlowFilterEntriesResource) resource)
											.getIfName(),
									VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.FFTYPE);
				if (((VRouterInterfaceFlowFilterEntriesResource) resource)
						.getFfType() != null
						&& !((VRouterInterfaceFlowFilterEntriesResource) resource)
								.getFfType().isEmpty()) {
					isValid = VtnServiceJsonConsts.IN
							.equalsIgnoreCase(((VRouterInterfaceFlowFilterEntriesResource) resource)
									.getFfType())
							|| VtnServiceJsonConsts.OUT
									.equalsIgnoreCase(((VRouterInterfaceFlowFilterEntriesResource) resource)
											.getFfType());
				} else {
					isValid = false;
				}
			}
			setListOpFlag(true);
		}
		// For VTerminalInterfaceFlowFilterEntriesResource instance
		else if (resource instanceof VTerminalInterfaceFlowFilterEntriesResource
				&& ((VTerminalInterfaceFlowFilterEntriesResource) resource)
						.getVtnName() != null
				&& !((VTerminalInterfaceFlowFilterEntriesResource) resource)
						.getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VTerminalInterfaceFlowFilterEntriesResource) resource)
							.getVtnName(), VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VTERMINALNAME);
				if (((VTerminalInterfaceFlowFilterEntriesResource) resource)
						.getVterminalName() != null
						&& !((VTerminalInterfaceFlowFilterEntriesResource) resource)
								.getVterminalName().isEmpty()) {
					isValid = validator
							.isValidMaxLengthAlphaNum(
									((VTerminalInterfaceFlowFilterEntriesResource) resource)
											.getVterminalName(),
									VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.IFNAME);
				if (((VTerminalInterfaceFlowFilterEntriesResource) resource)
						.getIfName() != null
						&& !((VTerminalInterfaceFlowFilterEntriesResource) resource)
								.getIfName().isEmpty()) {
					isValid = validator
							.isValidMaxLengthAlphaNum(
									((VTerminalInterfaceFlowFilterEntriesResource) resource)
											.getIfName(),
									VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.FFTYPE);
				if (((VTerminalInterfaceFlowFilterEntriesResource) resource)
						.getFfType() != null
						&& !((VTerminalInterfaceFlowFilterEntriesResource) resource)
								.getFfType().isEmpty()) {
					isValid = VtnServiceJsonConsts.IN
							.equalsIgnoreCase(((VTerminalInterfaceFlowFilterEntriesResource) resource)
									.getFfType())
							|| VtnServiceJsonConsts.OUT
									.equalsIgnoreCase(((VTerminalInterfaceFlowFilterEntriesResource) resource)
											.getFfType());
				} else {
					isValid = false;
				}
			}
			setListOpFlag(true);
		}
		LOG.trace("Complete VBridgeFlowFilterEntriesResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request Json object for get and post method of
	 * VbridgeFlowFilterEntry, VbridgeInetrfaceFlowFilterEntry and
	 * VrouterInterfaceFlowFilterEntry API.
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VBridgeFlowFilterEntriesResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of VBridgeFlowFilterEntriesResourceValidator");
		boolean isValid = false;
		try {
			isValid = validateUri();
			if (isValid && requestBody != null
					&& VtnServiceConsts.GET.equals(method)) {
				isValid = validator.isValidGetForIntIndex(requestBody,
						isListOpFlag());
				setInvalidParameter(validator.getInvalidParameter());
				updateOpParameterForList(requestBody);
			} else if (isValid && requestBody != null
					&& VtnServiceConsts.POST.equals(method)) {
				isValid = validatePost(requestBody);
				if (validator.getInvalidParameter() != null) {
					setInvalidParameter(validator.getInvalidParameter());
				}
			} else if (isValid) {
				setInvalidParameter(VtnServiceConsts.INCORRECT_METHOD_INVOCATION);
				isValid = false;
			}
		} catch (final NumberFormatException e) {
			if (validator.getInvalidParameter() != null) {
				setInvalidParameter(validator.getInvalidParameter());
			}
			LOG.error(e, "Inside catch:NumberFormatException");
			isValid = false;
		} catch (final ClassCastException e) {
			if (validator.getInvalidParameter() != null) {
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
		LOG.trace("Complete VBridgeFlowFilterEntriesResourceValidator#validate()");
	}

	/**
	 * validate post request Json object for VbridgeFlowFilterEntry,
	 * VbridgeInetrfaceFlowFilterEntry and VrouterInterfaceFlowFilterEntry API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validatePost(final JsonObject requestBody) {
		LOG.trace("Start VBridgeFlowFilterEntriesResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.FLOWFILTERENTRY);
		if (requestBody.has(VtnServiceJsonConsts.FLOWFILTERENTRY)
				&& requestBody.get(VtnServiceJsonConsts.FLOWFILTERENTRY)
						.isJsonObject()) {
			final JsonObject ffEntry = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.FLOWFILTERENTRY);
			// validation for key: seqnum(mandatory)
			setInvalidParameter(VtnServiceJsonConsts.SEQNUM);
			if (ffEntry.has(VtnServiceJsonConsts.SEQNUM)
					&& ffEntry.getAsJsonPrimitive(VtnServiceJsonConsts.SEQNUM)
							.getAsString() != null) {
				isValid = validator.isValidRange(
						ffEntry.getAsJsonPrimitive(VtnServiceJsonConsts.SEQNUM)
								.getAsString(), VtnServiceJsonConsts.VAL_1,
						VtnServiceJsonConsts.VAL_65535);
			}
			if (isValid) {
				isValid = validator.isValidFlowFilterEntry(requestBody);
			}
			if (isValid) {
				// validation for redirectdst
				isValid = validator.isValidRedirectDst(isValid, ffEntry);
			}
		}
		LOG.trace("Complete VBridgeFlowFilterEntriesResourceValidator#validatePost()");
		return isValid;
	}
}
