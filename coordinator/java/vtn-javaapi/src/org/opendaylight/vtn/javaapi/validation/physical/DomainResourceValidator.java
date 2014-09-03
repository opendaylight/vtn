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
import org.opendaylight.vtn.javaapi.resources.physical.DomainResource;
import org.opendaylight.vtn.javaapi.resources.physical.DomainsResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class DomainResourceValidator validates put, post and get methods of
 * Domain API.
 */
public class DomainResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(DomainResourceValidator.class.getName());

	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new Domain resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public DomainResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for Domain API
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start DomainResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.CONTROLLERID);
		if (resource instanceof DomainResource
				&& ((DomainResource) resource).getcontrollerId() != null
				&& !((DomainResource) resource).getcontrollerId().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((DomainResource) resource).getcontrollerId(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.DOMAINID);
				if (((DomainResource) resource).getdomainId() != null
						&& !((DomainResource) resource).getdomainId().isEmpty()) {
					isValid = validator.isValidDomainId(
							((DomainResource) resource).getdomainId(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		} else if (resource instanceof DomainsResource
				&& ((DomainsResource) resource).getControllerId() != null
				&& !((DomainsResource) resource).getControllerId().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((DomainsResource) resource).getControllerId(),
					VtnServiceJsonConsts.LEN_31);
			setListOpFlag(true);
		}
		LOG.trace("Complete DomainResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request json for Domain API
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start DomainResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of DomainResourceValidator");
		boolean isValid = false;
		isValid = validateUri();
		if (isValid && requestBody != null
				&& VtnServiceConsts.GET.equals(method)) {
			isValid = validateGet(requestBody, isListOpFlag());
			updateOpParameterForList(requestBody);
		} else if (isValid && requestBody != null
				&& VtnServiceConsts.POST.equalsIgnoreCase(method)) {
			isValid = validatePost(requestBody);
		} else if (isValid && requestBody != null
				&& VtnServiceConsts.PUT.equals(method)) {
			isValid = validatePut(requestBody);
		} else if (isValid) {
			setInvalidParameter(VtnServiceConsts.INCORRECT_METHOD_INVOCATION);
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
		LOG.trace("Complete DomainResourceValidator#validate()");
	}

	/**
	 * Validate post request json for Domain API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 */
	private boolean validatePost(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start DomainResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.DOMAIN);
		if (requestBody.has(VtnServiceJsonConsts.DOMAIN)
				&& requestBody.get(VtnServiceJsonConsts.DOMAIN).isJsonObject()) {
			final JsonObject domain = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.DOMAIN);
			// validation for mandatory key: domain_id
			setInvalidParameter(VtnServiceJsonConsts.DOMAINID);
			if (domain.has(VtnServiceJsonConsts.DOMAINID)
					&& domain.getAsJsonPrimitive(VtnServiceJsonConsts.DOMAINID)
							.getAsString() != null) {
				isValid = validator
						.isValidDomainId(
								domain.getAsJsonPrimitive(
										VtnServiceJsonConsts.DOMAINID)
										.getAsString(),
								VtnServiceJsonConsts.LEN_31);
			}
			// validation for mandatory key: type
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.TYPE);
				if (domain.has(VtnServiceJsonConsts.TYPE)
						&& domain.getAsJsonPrimitive(VtnServiceJsonConsts.TYPE)
								.getAsString() != null) {
					final String type = domain.getAsJsonPrimitive(
							VtnServiceJsonConsts.TYPE).getAsString();
					isValid = type
							.equalsIgnoreCase(VtnServiceJsonConsts.NORMAL);
				} else {
					isValid = false;
				}
			}
			// validation for key: description
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.DESCRIPTION);
				isValid = commonValidation(isValid, domain);
			}

		}
		LOG.trace("Complete DomainResourceValidator#validatePost()");
		return isValid;
	}

	/**
	 * 
	 * @param isValid
	 * @param domain
	 * @return
	 */
	private boolean commonValidation(boolean isValid, final JsonObject domain) {
		if (domain.has(VtnServiceJsonConsts.DESCRIPTION)
				&& domain.getAsJsonPrimitive(VtnServiceJsonConsts.DESCRIPTION)
						.getAsString() != null
				&& !domain.getAsJsonPrimitive(VtnServiceJsonConsts.DESCRIPTION)
						.getAsString().isEmpty()) {
			isValid = validator.isValidMaxLength(
					domain.getAsJsonPrimitive(VtnServiceJsonConsts.DESCRIPTION)
							.getAsString(), VtnServiceJsonConsts.LEN_127);
		}
		return isValid;
	}

	/**
	 * Validate put request json for DomainResource
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validatePut(final JsonObject requestBody) {
		LOG.trace("Start DomainResourceValidator#validatePut()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.DOMAIN);
		if (requestBody.has(VtnServiceJsonConsts.DOMAIN)
				&& requestBody.get(VtnServiceJsonConsts.DOMAIN).isJsonObject()) {
			isValid = true;
			final JsonObject domain = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.DOMAIN);
			// validation for key: description
			setInvalidParameter(VtnServiceJsonConsts.DESCRIPTION);
			isValid = commonValidation(isValid, domain);
			// validation for key: type as on 23-04-2013
			/*
			 * if (isValid) { setInvalidParameter(VtnServiceJsonConsts.TYPE); if
			 * (domain.has(VtnServiceJsonConsts.TYPE) &&
			 * domain.getAsJsonPrimitive(VtnServiceJsonConsts.TYPE)
			 * .getAsString() != null) { final String type = domain
			 * .getAsJsonPrimitive(VtnServiceJsonConsts.TYPE) .getAsString();
			 * isValid = type .equalsIgnoreCase(VtnServiceJsonConsts.NORMAL) ||
			 * type.equalsIgnoreCase(VtnServiceJsonConsts.DEFAULT); } }
			 */

			if (domain.has(VtnServiceJsonConsts.TYPE)) {
				requestBody.getAsJsonObject(VtnServiceJsonConsts.DOMAIN)
						.remove(VtnServiceJsonConsts.TYPE);
			}
		}
		LOG.trace("Complete DomainResourceValidator#validatePut()");
		return isValid;
	}

	/**
	 * Validate get request json for Domain API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 */
	private boolean validateGet(final JsonObject requestBody,
			final boolean opFlag) throws VtnServiceException {
		LOG.trace("Start DomainResourceValidator#validateGet()");
		boolean isValid = true;
		// validation for key: targetdb
		setInvalidParameter(VtnServiceJsonConsts.TARGETDB);
		isValid = validator.isValidRequestDB(requestBody);
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
								VtnServiceJsonConsts.INDEX).getAsString() != null
						&& !requestBody
								.getAsJsonPrimitive(VtnServiceJsonConsts.INDEX)
								.getAsString().isEmpty()) {
					isValid = validator.isValidDomainId(requestBody
							.getAsJsonPrimitive(VtnServiceJsonConsts.INDEX)
							.getAsString(), VtnServiceJsonConsts.LEN_31);
				}
			}
			// validation for key: max_repitition
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.MAX);
				isValid = validator.isValidMaxRepetition(requestBody);
			}
		}
		LOG.trace("Complete DomainResourceValidator#validateGet()");
		return isValid;
	}
}
