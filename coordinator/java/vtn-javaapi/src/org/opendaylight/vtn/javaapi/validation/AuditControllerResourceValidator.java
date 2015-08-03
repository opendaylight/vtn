/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi.validation;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.resources.AuditControllerResource;

public class AuditControllerResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(AuditControllerResourceValidator.class.getName());

	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new Audit controller resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public AuditControllerResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate URI parameters for Audit Controller API
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start AuditControllerResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.CONTROLLERID);
		if (resource instanceof AuditControllerResource
				&& ((AuditControllerResource) resource).getControllerId() != null
				&& !((AuditControllerResource) resource).getControllerId()
						.isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((AuditControllerResource) resource).getControllerId(),
					VtnServiceJsonConsts.LEN_31);
		}
		LOG.trace("Complete AuditControllerResourceValidator#validateUri()");
		return isValid;
	}

	/*
	 * Validate request json for put method of Audit controller API
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start AuditControllerResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of AuditControllerResourceValidator");
		boolean isValid = false;
		try {
			isValid = validateUri();
			if (isValid && requestBody != null
					&& VtnServiceConsts.PUT.equals(method)) {
				isValid = validatePut(requestBody);
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
		LOG.trace("Complete AuditControllerResourceValidator#validate()");
	}

	/**
	 * Validate request json for put method of Audit controller API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * 
	 * @return true, if successful
	 */
	private boolean validatePut(final JsonObject requestBody) {
		LOG.trace("Start AuditControllerResourceValidator#validatePut()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.AUDIT);
		if (requestBody.has(VtnServiceJsonConsts.AUDIT)
				&& requestBody.get(VtnServiceJsonConsts.AUDIT).isJsonObject()) {
			isValid = true;
			final JsonObject audit = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.AUDIT);
			setInvalidParameter(VtnServiceJsonConsts.FORCE);
			if (audit.has(VtnServiceJsonConsts.FORCE)
					&& audit.getAsJsonPrimitive(VtnServiceJsonConsts.FORCE)
							.getAsString() != null) {
				String force = audit.getAsJsonPrimitive(
						VtnServiceJsonConsts.FORCE).getAsString();
				isValid = force.equalsIgnoreCase(VtnServiceJsonConsts.TRUE)
						|| force.equalsIgnoreCase(VtnServiceJsonConsts.FALSE);
			}

			setInvalidParameter(VtnServiceJsonConsts.REAL_NETWORK_AUDIT);
			if (audit.has(VtnServiceJsonConsts.REAL_NETWORK_AUDIT)
					&& audit.getAsJsonPrimitive(VtnServiceJsonConsts.REAL_NETWORK_AUDIT)
							.getAsString() != null) {
				String auditType = audit.getAsJsonPrimitive(
						VtnServiceJsonConsts.REAL_NETWORK_AUDIT).getAsString();
				isValid = auditType.equalsIgnoreCase(VtnServiceJsonConsts.TRUE)
						|| auditType.equalsIgnoreCase(VtnServiceJsonConsts.FALSE);
			}
		}
		LOG.trace("Complete AuditControllerResourceValidator#validatePut()");
		return isValid;
	}
}
