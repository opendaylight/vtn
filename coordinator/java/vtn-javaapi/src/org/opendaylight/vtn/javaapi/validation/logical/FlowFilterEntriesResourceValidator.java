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
import org.opendaylight.vtn.javaapi.resources.logical.FlowFilterEntriesResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class FlowFilterEntriesResourceValidator validates request Json object
 * for post and get method of FlowFilterEntry API.
 */
public class FlowFilterEntriesResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(FlowFilterEntriesResourceValidator.class.getName());
	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new flow filter entries resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public FlowFilterEntriesResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for FlowFilterEntries API.
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start FlowFilterEntriesResourceValidator#validateUri()");
		boolean isValid = false;
		// For FlowFilterEntriesResource instance
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.VTNNAME);
		if (resource instanceof FlowFilterEntriesResource
				&& ((FlowFilterEntriesResource) resource).getVtnName() != null
				&& !((FlowFilterEntriesResource) resource).getVtnName()
						.isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((FlowFilterEntriesResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.FFTYPE);
				if (((FlowFilterEntriesResource) resource).getFfType() != null
						&& !((FlowFilterEntriesResource) resource).getFfType()
								.isEmpty()) {
					isValid = VtnServiceJsonConsts.IN
							.equalsIgnoreCase(((FlowFilterEntriesResource) resource)
									.getFfType())
							|| VtnServiceJsonConsts.OUT
									.equalsIgnoreCase(((FlowFilterEntriesResource) resource)
											.getFfType());
				} else {
					isValid = false;
				}
			}
			setListOpFlag(true);
		}

		LOG.trace("Complete FlowFilterEntriesResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request Json object for get and post method of FlowFilterEntry
	 * API.
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start FlowFilterEntriesResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of FlowFilterEntriesResourceValidator");
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
		LOG.trace("Complete FlowFilterEntriesResourceValidator#validate()");
	}

	/**
	 * Validate post request Json object for FlowFilterEntriesResource.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validatePost(final JsonObject requestBody) {
		LOG.trace("Start FlowFilterEntriesResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.FLOWFILTERENTRY);
		if (requestBody.has(VtnServiceJsonConsts.FLOWFILTERENTRY)
				&& requestBody.get(VtnServiceJsonConsts.FLOWFILTERENTRY)
						.isJsonObject()) {
			final JsonObject ffEntry = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.FLOWFILTERENTRY);
			// validation for mandatory key: seqnum
			setInvalidParameter(VtnServiceJsonConsts.SEQNUM);
			if (ffEntry.has(VtnServiceJsonConsts.SEQNUM)
					&& ffEntry.getAsJsonPrimitive(VtnServiceJsonConsts.SEQNUM)
							.getAsString() != null
					&& !ffEntry.getAsJsonPrimitive(VtnServiceJsonConsts.SEQNUM)
							.getAsString().isEmpty()) {
				isValid = validator.isValidRange(
						ffEntry.getAsJsonPrimitive(VtnServiceJsonConsts.SEQNUM)
								.getAsString(), VtnServiceJsonConsts.VAL_1,
						VtnServiceJsonConsts.VAL_65535);
			} else {
				isValid = false;
			}
			if (isValid) {
				isValid = validator.isValidFlowFilterEntry(requestBody)
						&& validator.isValidVtnFlowFilterEntry(requestBody);
				setInvalidParameter(validator.getInvalidParameter());
			}
		}
		LOG.trace("Complete FlowFilterEntriesResourceValidator#validatePost()");
		return isValid;
	}

}
