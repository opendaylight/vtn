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
import org.opendaylight.vtn.javaapi.resources.logical.FlowFilterEntryResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class FlowFilterEntryResourceValidator validates request Json object for
 * put and get method of FlowFilterEntry API.
 */
public class FlowFilterEntryResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(FlowFilterEntryResourceValidator.class.getName());
	private final AbstractResource resource;
	final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new flow filter entry resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public FlowFilterEntryResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for FlowFilterEntry API.
	 * 
	 * @return true, if successful
	 */
	@Override
	public boolean validateUri() {
		LOG.trace("Start FlowFilterEntryResourceValidator#validateUri()");
		boolean isValid = false;
		// For FlowFilterEntriesResource instance
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.VTNNAME);
		if (resource instanceof FlowFilterEntryResource
				&& ((FlowFilterEntryResource) resource).getVtnName() != null
				&& !((FlowFilterEntryResource) resource).getVtnName().trim()
						.isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((FlowFilterEntryResource) resource).getVtnName().trim(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.FFTYPE);
				if (((FlowFilterEntryResource) resource).getFfType() != null
						&& !((FlowFilterEntryResource) resource).getFfType()
								.trim().isEmpty()) {
					isValid = ((FlowFilterEntryResource) resource).getFfType()
							.trim().equalsIgnoreCase(VtnServiceJsonConsts.IN)
							|| ((FlowFilterEntryResource) resource).getFfType()
									.trim()
									.equalsIgnoreCase(VtnServiceJsonConsts.OUT);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.SEQNUM);
				if (((FlowFilterEntryResource) resource).getSeqnum() != null
						&& !((FlowFilterEntryResource) resource).getSeqnum()
								.trim().isEmpty()) {
					isValid = validator.isValidRange(((FlowFilterEntryResource) resource)
									.getSeqnum().trim(),
							VtnServiceJsonConsts.VAL_1,
							VtnServiceJsonConsts.VAL_65535);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		}
		LOG.trace("Complete FlowFilterEntryResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request Json object for get, put method of FlowFilterEntry API
	 */
	@Override
	public void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start FlowFilterEntryResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of FlowFilterEntryResourceValidator");
		boolean isValid = false;
		try {
			isValid = validateUri();
			if (isValid && requestBody != null
					&& VtnServiceConsts.GET.equals(method)) {
				isValid = validateGet(requestBody);
				updateOpParameterForList(requestBody);
			} else if (isValid && requestBody != null
					&& VtnServiceConsts.PUT.equals(method)) {
				isValid = validator.isValidFlowFilterEntry(requestBody);
				setInvalidParameter(validator.getInvalidParameter());
			} else if (isValid) {
				setInvalidParameter(VtnServiceConsts.INCORRECT_METHOD_INVOCATION);
				isValid = false;
			}
		} catch (final NumberFormatException e) {
			if (method.equals(VtnServiceConsts.PUT)) {
				setInvalidParameter(validator.getInvalidParameter());
			}
			LOG.error("Inside catch:NumberFormatException");
			isValid = false;
		} catch (final ClassCastException e) {
			if (method.equals(VtnServiceConsts.PUT)) {
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
		LOG.trace("Complete FlowFilterEntryResourceValidator#validate()");
	}

	/**
	 * Validate get request Json object for FlowFilterEntry API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validateGet(final JsonObject requestBody) {
		LOG.trace("Start FlowFilterEntryResourceValidator#validateGet()");
		boolean isValid = false;
		
		// validation for key: targetdb
		setInvalidParameter(VtnServiceJsonConsts.TARGETDB);
		isValid = validator.isValidRequestDB(requestBody);

		if (isValid
				&& requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.TARGETDB)
						.getAsString().trim()
						.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
			// validation for key: controller_id(mandatory)
			setInvalidParameter(VtnServiceJsonConsts.CONTROLLERID);
			if (requestBody.has(VtnServiceJsonConsts.CONTROLLERID)
					&& requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.CONTROLLERID).getAsString() != null) {
				isValid = validator.isValidMaxLengthAlphaNum(requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.CONTROLLERID)
						.getAsString().trim(), VtnServiceJsonConsts.LEN_31);
			} else {
				isValid = false;
			}
			
			if (isValid) {
				// validation for key: domain_id(mandatory)
				setInvalidParameter(VtnServiceJsonConsts.DOMAINID);
				if (requestBody.has(VtnServiceJsonConsts.DOMAINID)
						&& requestBody.getAsJsonPrimitive(
								VtnServiceJsonConsts.DOMAINID).getAsString() != null) {
					isValid = validator.isValidDomainId(requestBody
							.getAsJsonPrimitive(VtnServiceJsonConsts.DOMAINID)
							.getAsString().trim(), VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}			
		}
						
		// validation for key: op
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.OP);
			// isValid = validator.isValidOperation(requestBody);
			if (requestBody.has(VtnServiceJsonConsts.OP)
					&& requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.OP)
							.getAsString() != null
					&& !requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.OP)
							.getAsString().trim().isEmpty()) {
				final String operation = requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.OP)
						.getAsString().trim();
				isValid = operation
						.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL);
			} else {
				requestBody.remove(VtnServiceJsonConsts.OP);
				requestBody.addProperty(VtnServiceJsonConsts.OP,
						VtnServiceJsonConsts.NORMAL);
			}
		}
		if (requestBody.has(VtnServiceJsonConsts.INDEX)) {
			requestBody.remove(VtnServiceJsonConsts.INDEX);
		} else {
			LOG.debug("No need to remove");
		}
		LOG.trace("Complete FlowFilterEntryResourceValidator#validateGet()");
		return isValid;
	}
}
