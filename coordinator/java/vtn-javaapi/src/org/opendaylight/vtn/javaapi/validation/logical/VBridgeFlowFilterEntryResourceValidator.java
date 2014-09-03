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
import org.opendaylight.vtn.javaapi.resources.logical.VBridgeFlowFilterEntryResource;
import org.opendaylight.vtn.javaapi.resources.logical.VBridgeInterfaceFlowFilterEntryResource;
import org.opendaylight.vtn.javaapi.resources.logical.VRouterInterfaceFlowFilterEntryResource;
import org.opendaylight.vtn.javaapi.resources.logical.VTerminalInterfaceFlowFilterEntryResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class VBridgeFlowFilterEntryResourceValidator validates request Json
 * object for VbridgeFlowFilterEntry, VbridgeInetrfaceFlowFilterEntry and
 * VrouterInterfaceFlowFilterEntry API.
 */
public class VBridgeFlowFilterEntryResourceValidator extends
		VtnServiceValidator {
	private static final Logger LOG = Logger
			.getLogger(VBridgeFlowFilterEntryResourceValidator.class.getName());
	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new vbridge flow filter entry resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public VBridgeFlowFilterEntryResourceValidator(
			final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for
	 * VBridgeFlowFilterEntryResource,VBridgeInterfaceFlowFilterEntryResource,
	 * VRouterInterfaceFlowFilterEntryResource
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start VBridgeFlowFilterEntryResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.VTNNAME);
		// For VBridgeFlowFilterEntryResourceValidator instance
		if (resource instanceof VBridgeFlowFilterEntryResource
				&& ((VBridgeFlowFilterEntryResource) resource).getVtnName() != null
				&& !((VBridgeFlowFilterEntryResource) resource).getVtnName()
						.isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VBridgeFlowFilterEntryResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VBRNAME);
				if (((VBridgeFlowFilterEntryResource) resource).getVbrName() != null
						&& !((VBridgeFlowFilterEntryResource) resource)
								.getVbrName().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VBridgeFlowFilterEntryResource) resource)
									.getVbrName(), VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.FFTYPE);
				if (((VBridgeFlowFilterEntryResource) resource).getFfType() != null
						&& !((VBridgeFlowFilterEntryResource) resource)
								.getFfType().isEmpty()) {
					isValid = ((VBridgeFlowFilterEntryResource) resource)
							.getFfType().equalsIgnoreCase(
									VtnServiceJsonConsts.IN)
							|| ((VBridgeFlowFilterEntryResource) resource)
									.getFfType().equalsIgnoreCase(
											VtnServiceJsonConsts.OUT);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.SEQNUM);
				if (((VBridgeFlowFilterEntryResource) resource).getSeqnum() != null
						&& !((VBridgeFlowFilterEntryResource) resource)
								.getSeqnum().isEmpty()) {
					isValid = validator.isValidRange(
							((VBridgeFlowFilterEntryResource) resource)
									.getSeqnum(), VtnServiceJsonConsts.VAL_1,
							VtnServiceJsonConsts.VAL_65535);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		} else if (resource instanceof VBridgeInterfaceFlowFilterEntryResource
				&& ((VBridgeInterfaceFlowFilterEntryResource) resource)
						.getVtnName() != null
				&& !((VBridgeInterfaceFlowFilterEntryResource) resource)
						.getVtnName().isEmpty()) {
			// For VBridgeInterfaceFlowFilterEntryResource instance
			isValid = validator.isValidMaxLengthAlphaNum(
					((VBridgeInterfaceFlowFilterEntryResource) resource)
							.getVtnName(), VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VBRNAME);
				if (((VBridgeInterfaceFlowFilterEntryResource) resource)
						.getVbrName() != null
						&& !((VBridgeInterfaceFlowFilterEntryResource) resource)
								.getVbrName().isEmpty()) {
					isValid = validator
							.isValidMaxLengthAlphaNum(
									((VBridgeInterfaceFlowFilterEntryResource) resource)
											.getVbrName(),
									VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.IFNAME);
				if (((VBridgeInterfaceFlowFilterEntryResource) resource)
						.getIfName() != null
						&& !((VBridgeInterfaceFlowFilterEntryResource) resource)
								.getIfName().isEmpty()) {
					isValid = validator
							.isValidMaxLengthAlphaNum(
									((VBridgeInterfaceFlowFilterEntryResource) resource)
											.getIfName(),
									VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.FFTYPE);
				if (((VBridgeInterfaceFlowFilterEntryResource) resource)
						.getFfType() != null
						&& !((VBridgeInterfaceFlowFilterEntryResource) resource)
								.getFfType().isEmpty()) {
					isValid = ((VBridgeInterfaceFlowFilterEntryResource) resource)
							.getFfType().equalsIgnoreCase(
									VtnServiceJsonConsts.IN)
							|| ((VBridgeInterfaceFlowFilterEntryResource) resource)
									.getFfType().equalsIgnoreCase(
											VtnServiceJsonConsts.OUT);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.SEQNUM);
				if (((VBridgeInterfaceFlowFilterEntryResource) resource)
						.getSeqnum() != null
						&& !((VBridgeInterfaceFlowFilterEntryResource) resource)
								.getSeqnum().isEmpty()) {
					isValid = validator
							.isValidRange(
									((VBridgeInterfaceFlowFilterEntryResource) resource)
											.getSeqnum(),
									VtnServiceJsonConsts.VAL_1,
									VtnServiceJsonConsts.VAL_65535);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		} else if (resource instanceof VRouterInterfaceFlowFilterEntryResource
				&& ((VRouterInterfaceFlowFilterEntryResource) resource)
						.getVtnName() != null
				&& !((VRouterInterfaceFlowFilterEntryResource) resource)
						.getVtnName().isEmpty()) {
			// For VRouterInterfaceFlowFilterEntryResource instance
			isValid = validator.isValidMaxLengthAlphaNum(
					((VRouterInterfaceFlowFilterEntryResource) resource)
							.getVtnName(), VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VRTNAME);
				if (((VRouterInterfaceFlowFilterEntryResource) resource)
						.getVrtName() != null
						&& !((VRouterInterfaceFlowFilterEntryResource) resource)
								.getVrtName().isEmpty()) {
					isValid = validator
							.isValidMaxLengthAlphaNum(
									((VRouterInterfaceFlowFilterEntryResource) resource)
											.getVrtName(),
									VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.IFNAME);
				if (((VRouterInterfaceFlowFilterEntryResource) resource)
						.getIfName() != null
						&& !((VRouterInterfaceFlowFilterEntryResource) resource)
								.getIfName().isEmpty()) {
					isValid = validator
							.isValidMaxLengthAlphaNum(
									((VRouterInterfaceFlowFilterEntryResource) resource)
											.getIfName(),
									VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.FFTYPE);
				if (((VRouterInterfaceFlowFilterEntryResource) resource)
						.getFfType() != null
						&& !((VRouterInterfaceFlowFilterEntryResource) resource)
								.getFfType().isEmpty()) {
					isValid = ((VRouterInterfaceFlowFilterEntryResource) resource)
							.getFfType().equalsIgnoreCase(
									VtnServiceJsonConsts.IN)
							|| ((VRouterInterfaceFlowFilterEntryResource) resource)
									.getFfType().equalsIgnoreCase(
											VtnServiceJsonConsts.OUT);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.SEQNUM);
				if (((VRouterInterfaceFlowFilterEntryResource) resource)
						.getSeqnum() != null
						&& !((VRouterInterfaceFlowFilterEntryResource) resource)
								.getSeqnum().isEmpty()) {
					isValid = validator
							.isValidRange(
									((VRouterInterfaceFlowFilterEntryResource) resource)
											.getSeqnum(),
									VtnServiceJsonConsts.VAL_1,
									VtnServiceJsonConsts.VAL_65535);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		}
		// For VTerminalInterfaceFlowFilterEntryResource instance
		else if (resource instanceof VTerminalInterfaceFlowFilterEntryResource
				&& ((VTerminalInterfaceFlowFilterEntryResource) resource)
						.getVtnName() != null
				&& !((VTerminalInterfaceFlowFilterEntryResource) resource)
						.getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VTerminalInterfaceFlowFilterEntryResource) resource)
							.getVtnName(), VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VTERMINALNAME);
				if (((VTerminalInterfaceFlowFilterEntryResource) resource)
						.getVterminalName() != null
						&& !((VTerminalInterfaceFlowFilterEntryResource) resource)
								.getVterminalName().isEmpty()) {
					isValid = validator
							.isValidMaxLengthAlphaNum(
									((VTerminalInterfaceFlowFilterEntryResource) resource)
											.getVterminalName(),
									VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.IFNAME);
				if (((VTerminalInterfaceFlowFilterEntryResource) resource)
						.getIfName() != null
						&& !((VTerminalInterfaceFlowFilterEntryResource) resource)
								.getIfName().isEmpty()) {
					isValid = validator
							.isValidMaxLengthAlphaNum(
									((VTerminalInterfaceFlowFilterEntryResource) resource)
											.getIfName(),
									VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.FFTYPE);
				if (((VTerminalInterfaceFlowFilterEntryResource) resource)
						.getFfType() != null
						&& !((VTerminalInterfaceFlowFilterEntryResource) resource)
								.getFfType().isEmpty()) {
					isValid = ((VTerminalInterfaceFlowFilterEntryResource) resource)
							.getFfType().equalsIgnoreCase(
									VtnServiceJsonConsts.IN)
							|| ((VTerminalInterfaceFlowFilterEntryResource) resource)
									.getFfType().equalsIgnoreCase(
											VtnServiceJsonConsts.OUT);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.SEQNUM);
				if (((VTerminalInterfaceFlowFilterEntryResource) resource)
						.getSeqnum() != null
						&& !((VTerminalInterfaceFlowFilterEntryResource) resource)
								.getSeqnum().isEmpty()) {
					isValid = validator
							.isValidRange(
									((VTerminalInterfaceFlowFilterEntryResource) resource)
											.getSeqnum(),
									VtnServiceJsonConsts.VAL_1,
									VtnServiceJsonConsts.VAL_65535);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		}
		LOG.trace("Complete VBridgeFlowFilterEntryResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * validate request Json object for get and put mthods of
	 * VbridgeFlowFilterEntry, VbridgeInetrfaceFlowFilterEntry and
	 * VrouterInterfaceFlowFilterEntry API.
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VBridgeFlowFilterEntryResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of VBridgeFlowFilterEntryResourceValidator");
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
				if (validator.getInvalidParameter() != null) {
					setInvalidParameter(validator.getInvalidParameter());
				}
			} else if (isValid) {
				setInvalidParameter(VtnServiceConsts.INCORRECT_METHOD_INVOCATION);
				isValid = false;
			}
		} catch (final NumberFormatException e) {
			if (method.equals(VtnServiceConsts.PUT)) {
				setInvalidParameter(validator.getInvalidParameter());
			}
			LOG.error(e, "Inside catch:NumberFormatException");
			isValid = false;
		} catch (final ClassCastException e) {
			if (method.equals(VtnServiceConsts.PUT)) {
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
		LOG.trace("Complete VBridgeFlowFilterEntryResourceValidator#validate()");
	}

	/**
	 * validate get request Json object for VbridgeFlowFilterEntry,
	 * VbridgeInetrfaceFlowFilterEntry and VrouterInterfaceFlowFilterEntry API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validateGet(final JsonObject requestBody) {
		LOG.trace("Start VBridgeFlowFilterEntryResourceValidator#validateGet()");
		boolean isValid = false;
		// // validation for key: controller_id(mandatory)
		// setInvalidParameter(VtnServiceJsonConsts.CONTROLLERID);
		// if (requestBody.has(VtnServiceJsonConsts.CONTROLLERID)
		// && requestBody.getAsJsonPrimitive(
		// VtnServiceJsonConsts.CONTROLLERID).getAsString() != null) {
		// isValid = validator.isValidMaxLengthAlphaNum(requestBody
		// .getAsJsonPrimitive(VtnServiceJsonConsts.CONTROLLERID)
		// .getAsString(), VtnServiceJsonConsts.LEN_31);
		// }

		// validation for key: targetdb(optional)
		// if (isValid) {
		setInvalidParameter(VtnServiceJsonConsts.TARGETDB);
		isValid = validator.isValidRequestDB(requestBody);
		// }

		// validation for key: op(optinal)
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.OP);
			if (requestBody.has(VtnServiceJsonConsts.OP)
					&& requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.OP)
							.getAsString() != null
					&& !requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.OP)
							.getAsString().isEmpty()) {
				isValid = requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.OP)
						.getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL);
			} else {
				requestBody.remove(VtnServiceJsonConsts.OP);
				requestBody.addProperty(VtnServiceJsonConsts.OP,
						VtnServiceJsonConsts.NORMAL);
			}
		}

		LOG.trace("Complete VBridgeFlowFilterEntryResourceValidator#validateGet()");
		return isValid;
	}

	/**
	 * /* validate put request Json object for VbridgeFlowFilterEntry,
	 * VbridgeInetrfaceFlowFilterEntry and VrouterInterfaceFlowFilterEntry API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validatePut(final JsonObject requestBody) {
		LOG.trace("Start VBridgeFlowFilterEntryResourceValidator#validatePut()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.FLOWFILTERENTRY);
		if (requestBody.has(VtnServiceJsonConsts.FLOWFILTERENTRY)
				&& requestBody.get(VtnServiceJsonConsts.FLOWFILTERENTRY)
						.isJsonObject()) {
			isValid = true;
			final JsonObject ffEntry = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.FLOWFILTERENTRY);
			isValid = validator.isValidFlowFilterEntry(requestBody);
			if (isValid) {
				// validation for redirectdst
				isValid = validator.isValidRedirectDst(isValid, ffEntry);
			}
		}
		LOG.trace("Complete VBridgeFlowFilterEntryResourceValidator#validatePut()");
		return isValid;
	}
}
