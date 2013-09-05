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
import org.opendaylight.vtn.javaapi.resources.logical.FlowFilterResource;
import org.opendaylight.vtn.javaapi.resources.logical.FlowFiltersResource;
import org.opendaylight.vtn.javaapi.resources.logical.VBridgeFlowFilterResource;
import org.opendaylight.vtn.javaapi.resources.logical.VBridgeFlowFiltersResource;
import org.opendaylight.vtn.javaapi.resources.logical.VBridgeInterfaceFlowFilterResource;
import org.opendaylight.vtn.javaapi.resources.logical.VBridgeInterfaceFlowFiltersResource;
import org.opendaylight.vtn.javaapi.resources.logical.VRouterInterfaceFlowFilterResource;
import org.opendaylight.vtn.javaapi.resources.logical.VRouterInterfaceFlowFiltersResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class FlowFilterResourceValidator validates request Json object for Flow
 * Filter API.
 */
public class FlowFilterResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(FlowFilterResourceValidator.class.getName());
	private final AbstractResource resource;
	final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new flow filter resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public FlowFilterResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for FlowFilterResource
	 * 
	 * @return true, if successful
	 * 
	 */
	@Override
	public boolean validateUri() {
		LOG.trace("Start FlowFilterResourceValidator#validateUri()");
		boolean isValid = false;
		// For FlowFilterResource instance
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.VTNNAME);
		if (resource instanceof FlowFilterResource
				&& ((FlowFilterResource) resource).getVtnName() != null
				&& !((FlowFilterResource) resource).getVtnName().trim()
						.isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((FlowFilterResource) resource).getVtnName().trim(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.FFTYPE);
				if (((FlowFilterResource) resource).getFfType() != null
						&& !((FlowFilterResource) resource).getFfType().trim()
								.isEmpty()) {
					isValid = VtnServiceJsonConsts.IN
							.equalsIgnoreCase(((FlowFilterResource) resource)
									.getFfType().trim())
							|| VtnServiceJsonConsts.OUT
									.equalsIgnoreCase(((FlowFilterResource) resource)
											.getFfType().trim());
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		}
		// For FlowFiltersResource instance
		else if (resource instanceof FlowFiltersResource
				&& ((FlowFiltersResource) resource).getVtnName() != null
				&& !((FlowFiltersResource) resource).getVtnName().trim()
						.isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((FlowFiltersResource) resource).getVtnName().trim(),
					VtnServiceJsonConsts.LEN_31);
			setListOpFlag(true);
		}
		// For VBridgeFlowFiltersResource instance
		else if (resource instanceof VBridgeFlowFiltersResource
				&& ((VBridgeFlowFiltersResource) resource).getVtnName() != null
				&& !((VBridgeFlowFiltersResource) resource).getVtnName().trim()
						.isEmpty()) {
			isValid = validator
					.isValidMaxLengthAlphaNum(
							((VBridgeFlowFiltersResource) resource)
									.getVtnName().trim(),
							VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VBRNAME);
				if (((VBridgeFlowFiltersResource) resource).getVbrName() != null
						&& !((VBridgeFlowFiltersResource) resource)
								.getVbrName().trim().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VBridgeFlowFiltersResource) resource)
									.getVbrName().trim(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(true);
		}
		// For VBridgeFlowFilterResource instance
		else if (resource instanceof VBridgeFlowFilterResource
				&& ((VBridgeFlowFilterResource) resource).getVtnName() != null
				&& !((VBridgeFlowFilterResource) resource).getVtnName().trim()
						.isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VBridgeFlowFilterResource) resource).getVtnName().trim(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VBRNAME);
				if (((VBridgeFlowFilterResource) resource).getVbrName() != null
						&& !((VBridgeFlowFilterResource) resource).getVbrName()
								.trim().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VBridgeFlowFilterResource) resource).getVbrName()
									.trim(), VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.FFTYPE);
				if (((VBridgeFlowFilterResource) resource).getFfType() != null
						&& !((VBridgeFlowFilterResource) resource).getFfType()
								.trim().isEmpty()) {
					isValid = VtnServiceJsonConsts.IN
							.equalsIgnoreCase(((VBridgeFlowFilterResource) resource)
									.getFfType().trim())
							|| VtnServiceJsonConsts.OUT
									.equalsIgnoreCase(((VBridgeFlowFilterResource) resource)
											.getFfType().trim());
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		}
		// For VBridgeInterfaceFlowFiltersResource instance
		else if (resource instanceof VBridgeInterfaceFlowFiltersResource
				&& ((VBridgeInterfaceFlowFiltersResource) resource)
						.getVtnName() != null
				&& !((VBridgeInterfaceFlowFiltersResource) resource)
						.getVtnName().trim().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VBridgeInterfaceFlowFiltersResource) resource)
							.getVtnName().trim(), VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VBRNAME);
				if (((VBridgeInterfaceFlowFiltersResource) resource)
						.getVbrName() != null
						&& !((VBridgeInterfaceFlowFiltersResource) resource)
								.getVbrName().trim().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VBridgeInterfaceFlowFiltersResource) resource)
									.getVbrName().trim(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.IFNAME);
				if (((VBridgeInterfaceFlowFiltersResource) resource)
						.getIfName() != null
						&& !((VBridgeInterfaceFlowFiltersResource) resource)
								.getIfName().trim().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VBridgeInterfaceFlowFiltersResource) resource)
									.getIfName().trim(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(true);
		}
		// For VBridgeInterfaceFlowFilterResource instance
		else if (resource instanceof VBridgeInterfaceFlowFilterResource
				&& ((VBridgeInterfaceFlowFilterResource) resource).getVtnName() != null
				&& !((VBridgeInterfaceFlowFilterResource) resource)
						.getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VBridgeInterfaceFlowFilterResource) resource)
							.getVtnName(), VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VBRNAME);
				if (((VBridgeInterfaceFlowFilterResource) resource)
						.getVbrName() != null
						&& !((VBridgeInterfaceFlowFilterResource) resource)
								.getVbrName().trim().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VBridgeInterfaceFlowFilterResource) resource)
									.getVbrName().trim(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.IFNAME);
				if (((VBridgeInterfaceFlowFilterResource) resource).getIfName() != null
						&& !((VBridgeInterfaceFlowFilterResource) resource)
								.getIfName().trim().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VBridgeInterfaceFlowFilterResource) resource)
									.getIfName().trim(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.FFTYPE);
				if (((VBridgeInterfaceFlowFilterResource) resource).getFfType() != null
						&& !((VBridgeInterfaceFlowFilterResource) resource)
								.getFfType().trim().isEmpty()) {
					isValid = ((VBridgeInterfaceFlowFilterResource) resource)
							.getFfType().trim()
							.equalsIgnoreCase(VtnServiceJsonConsts.IN)
							|| ((VBridgeInterfaceFlowFilterResource) resource)
									.getFfType().trim()
									.equalsIgnoreCase(VtnServiceJsonConsts.OUT);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		}
		// For VRouterInterfaceFlowFiltersResource instance
		else if (resource instanceof VRouterInterfaceFlowFiltersResource
				&& ((VRouterInterfaceFlowFiltersResource) resource)
						.getVtnName() != null
				&& !((VRouterInterfaceFlowFiltersResource) resource)
						.getVtnName().trim().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VRouterInterfaceFlowFiltersResource) resource)
							.getVtnName().trim(), VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VRTNAME);
				if (((VRouterInterfaceFlowFiltersResource) resource)
						.getVrtName() != null
						&& !((VRouterInterfaceFlowFiltersResource) resource)
								.getVrtName().trim().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VRouterInterfaceFlowFiltersResource) resource)
									.getVrtName().trim(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.IFNAME);
				if (((VRouterInterfaceFlowFiltersResource) resource)
						.getIfName() != null
						&& !((VRouterInterfaceFlowFiltersResource) resource)
								.getIfName().trim().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VRouterInterfaceFlowFiltersResource) resource)
									.getIfName().trim(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(true);
		}
		// For VRouterInterfaceFlowFilterResource instance
		else if (resource instanceof VRouterInterfaceFlowFilterResource
				&& ((VRouterInterfaceFlowFilterResource) resource).getVtnName() != null
				&& !((VRouterInterfaceFlowFilterResource) resource)
						.getVtnName().trim().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VRouterInterfaceFlowFilterResource) resource)
							.getVtnName().trim(), VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VRTNAME);
				if (((VRouterInterfaceFlowFilterResource) resource)
						.getVrtName() != null
						&& !((VRouterInterfaceFlowFilterResource) resource)
								.getVrtName().trim().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VRouterInterfaceFlowFilterResource) resource)
									.getVrtName().trim(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.IFNAME);
				if (((VRouterInterfaceFlowFilterResource) resource).getIfName() != null
						&& !((VRouterInterfaceFlowFilterResource) resource)
								.getIfName().trim().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VRouterInterfaceFlowFilterResource) resource)
									.getIfName().trim(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.FFTYPE);
				if (((VRouterInterfaceFlowFilterResource) resource).getFfType() != null
						&& !((VRouterInterfaceFlowFilterResource) resource)
								.getFfType().trim().isEmpty()) {
					isValid = ((VRouterInterfaceFlowFilterResource) resource)
							.getFfType().trim()
							.equalsIgnoreCase(VtnServiceJsonConsts.IN)
							|| ((VRouterInterfaceFlowFilterResource) resource)
									.getFfType().trim()
									.equalsIgnoreCase(VtnServiceJsonConsts.OUT);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		}
		LOG.trace("Complete FlowFilterResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request json for get and post method of FlowFilter API
	 */
	@Override
	public void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start FlowFilterResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of FlowFilterResourceValidator");
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
		LOG.trace("Complete FlowFilterResourceValidator#validate()");
	}

	/**
	 * Validate post request json for FlowFilter API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validatePost(final JsonObject requestBody) {
		LOG.trace("Start FlowFilterResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.FLOWFILTER);
		if (requestBody.has(VtnServiceJsonConsts.FLOWFILTER)
				&& requestBody.get(VtnServiceJsonConsts.FLOWFILTER)
						.isJsonObject()) {
			final JsonObject flowFilter = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.FLOWFILTER);
			setInvalidParameter(VtnServiceJsonConsts.FFTYPE);
			if (flowFilter.has(VtnServiceJsonConsts.FFTYPE)
					&& flowFilter.getAsJsonPrimitive(
							VtnServiceJsonConsts.FFTYPE).getAsString() != null) {
				final String ffType = flowFilter
						.getAsJsonPrimitive(VtnServiceJsonConsts.FFTYPE)
						.getAsString().trim();
				isValid = ffType.equalsIgnoreCase(VtnServiceJsonConsts.IN)
						|| ffType.equalsIgnoreCase(VtnServiceJsonConsts.OUT);
			}
		}
		LOG.trace("Complete FlowFilterResourceValidator#validatePost()");
		return isValid;
	}

}
