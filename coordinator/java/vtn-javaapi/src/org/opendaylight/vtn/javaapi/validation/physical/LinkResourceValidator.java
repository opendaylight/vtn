/*
 * Copyright (c) 2012-2014 NEC Corporation
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
import org.opendaylight.vtn.javaapi.resources.physical.LinksResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class LinkResourceValidator validates get request Json object for Link
 * API.
 */
public class LinkResourceValidator extends VtnServiceValidator {
	/**
	 * Logger for debugging purpose.
	 */
	private static final Logger LOG = Logger
			.getLogger(LinkResourceValidator.class.getName());
	/**
	 * resource , the instance of AbstractResource.
	 */
	private final AbstractResource resource;
	/**
	 * validator object for common validations.
	 */
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new link resource validator.
	 * 
	 * @param resource
	 *            , the instance of AbstractResource
	 */
	public LinkResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for Link API .
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start LinkResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.CONTROLLERID);
		if (resource instanceof LinksResource
				&& ((LinksResource) resource).getControllerId() != null
				&& !((LinksResource) resource).getControllerId().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((LinksResource) resource).getControllerId(),
					VtnServiceJsonConsts.LEN_31);
			setListOpFlag(true);
		}
		LOG.trace("Complete LinkResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate link name .
	 * 
	 * @param link
	 *            value to be validated
	 * 
	 * @return true, if successful
	 */
	private boolean linkNameValidator(final String link) {
		boolean isValid = false;
		final String[] uri = link.split(VtnServiceJsonConsts.LINKSAPERATOR);
		if (uri.length == VtnServiceJsonConsts.LEN_4) {
			isValid = validator.isValidMaxLength(
					uri[VtnServiceJsonConsts.VAL_0],
					VtnServiceJsonConsts.LEN_255);
			if (isValid) {
				isValid = validator.isValidMaxLength(
						uri[VtnServiceJsonConsts.VAL_1],
						VtnServiceJsonConsts.LEN_31);
			}
			if (isValid) {
				isValid = validator.isValidMaxLength(
						uri[VtnServiceJsonConsts.VAL_2],
						VtnServiceJsonConsts.LEN_255);
			}
			if (isValid) {
				isValid = validator.isValidMaxLength(
						uri[VtnServiceJsonConsts.VAL_3],
						VtnServiceJsonConsts.LEN_31);
			}
		}
		return isValid;
	}

	/**
	 * Validate request JSON for get methods of Data Flow API.
	 */
	/**
	 * @param method
	 *            , contains info about get,post ,delete.
	 * @param requestBody
	 *            , contains request param.
	 * @throws VtnServiceException
	 *             , vtnexcpetion is thrown.
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start LinkResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of LinkResourceValidator");
		boolean isValid = false;
		try {
			isValid = validateUri();
			if (isValid && requestBody != null
					&& VtnServiceConsts.GET.equals(method)) {
				isValid = validateGet(requestBody, isListOpFlag());
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
		LOG.trace("Complete LinkResourceValidator#validate()");
	}

	/**
	 * Validate get request json for Link API .
	 * 
	 * @param requestBody
	 *            , the request Json object .
	 * @param opFlag
	 *            , to resolve operation type .
	 * @return true, if is valid get
	 */
	public final boolean validateGet(final JsonObject requestBody,
			final boolean opFlag) {
		LOG.trace("Start LinkResourceValidator#ValidGet");
		boolean isValid = true;
		// validation for key: targetdb
		setInvalidParameter(VtnServiceJsonConsts.TARGETDB);
		if (requestBody.has(VtnServiceJsonConsts.TARGETDB)
				&& requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.TARGETDB)
						.getAsString() != null
				&& !requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.TARGETDB)
						.getAsString().isEmpty()) {
			isValid = VtnServiceJsonConsts.STATE.equalsIgnoreCase(requestBody
					.getAsJsonPrimitive(VtnServiceJsonConsts.TARGETDB)
					.getAsString());
		} else {
			requestBody.remove(VtnServiceJsonConsts.TARGETDB);
			requestBody.addProperty(VtnServiceJsonConsts.TARGETDB,
					VtnServiceJsonConsts.STATE);
		}
		if (!opFlag) {
			if (requestBody.has(VtnServiceJsonConsts.OP)) {
				requestBody.remove(VtnServiceJsonConsts.OP);
			} else {
				LOG.debug("No need to remove");
			}
			if (requestBody.has(VtnServiceJsonConsts.INDEX)) {
				requestBody.remove(VtnServiceJsonConsts.INDEX);
			} else {
				LOG.debug("No need to remove");
			}
			if (requestBody.has(VtnServiceJsonConsts.MAX)) {
				requestBody.remove(VtnServiceJsonConsts.MAX);
			} else {
				LOG.debug("No need to remove");
			}
		} else {
			// validation for key: op
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.OP);
				isValid = validator.isValidOperation(requestBody);
			}
			// validation for key: index
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.INDEX);
				if (requestBody.has(VtnServiceJsonConsts.INDEX)
						&& requestBody.getAsJsonPrimitive(
								VtnServiceJsonConsts.INDEX).getAsString() != null) {
					isValid = linkNameValidator(requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.INDEX).getAsString());
				}
			}
			// validation for key: max_repitition
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.MAX);
				isValid = validator.isValidMaxRepetition(requestBody);
				/*
				 * if(requestBody.has(VtnServiceJsonConsts.MAX) &&
				 * requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.MAX)
				 * .getAsString() != null) { isValid =
				 * validator.isValidMaxLengthNumber(requestBody
				 * .getAsJsonPrimitive(VtnServiceJsonConsts.MAX).getAsString()
				 * ); }
				 */
			}
		}
		// validation for key: switch_id
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.SWITCH1ID);
			if (requestBody.has(VtnServiceJsonConsts.SWITCH1ID)
					&& requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.SWITCH1ID).getAsString() != null
					&& !requestBody
							.getAsJsonPrimitive(VtnServiceJsonConsts.SWITCH1ID)
							.getAsString().isEmpty()) {
				isValid = validator.isValidMaxLength(requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.SWITCH1ID)
						.getAsString(), VtnServiceJsonConsts.LEN_255);
			}
		}

		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.SWITCH2ID);
			if (requestBody.has(VtnServiceJsonConsts.SWITCH2ID)
					&& requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.SWITCH2ID).getAsString() != null
					&& !requestBody
							.getAsJsonPrimitive(VtnServiceJsonConsts.SWITCH2ID)
							.getAsString().isEmpty()) {
				isValid = validator.isValidMaxLength(requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.SWITCH2ID)
						.getAsString(), VtnServiceJsonConsts.LEN_255);
			}
		}
		// validation for key: Link Name
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.LINKNAME);
			if (requestBody.has(VtnServiceJsonConsts.LINKNAME)
					&& requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.LINKNAME).getAsString() != null) {
				isValid = linkNameValidator(requestBody.getAsJsonPrimitive(
						VtnServiceJsonConsts.LINKNAME).getAsString());
			}
		}

		LOG.trace("Complete LinkResourceValidator#isValidGet");
		return isValid;
	}

}
