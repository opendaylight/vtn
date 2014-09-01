/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.openstack.validation;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.resources.openstack.NetworkResource;
import org.opendaylight.vtn.javaapi.resources.openstack.NetworksResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * Network Validation Resource class. Contains methods to validate URI,
 * parameters of POST and PUT request body
 * 
 */
public class NetworkResourceValidator extends VtnServiceValidator {

	/* Logger instance */
	private static final Logger LOG = Logger
			.getLogger(NetworkResourceValidator.class.getName());

	/* AbstractResource reference pointing to actual Resource class */
	private final AbstractResource resource;

	/* instance for common validation operation */
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Constructor that provide reference of actual Resource class to instance
	 * variable resource
	 * 
	 * @param resource
	 *            - Resource class reference
	 */
	public NetworkResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Calls the respective validation method according to type of method
	 * 
	 * @see org.opendaylight.vtn.javaapi.validation.VtnServiceValidator#validate
	 *      (java.lang.String, com.google.gson.JsonObject)
	 */
	@Override
	public void validate(String method, JsonObject requestBody)
			throws VtnServiceException {
		LOG.info("Start NetworkResourceValidator#validate()");
		boolean isValid = false;
		try {
			if (requestBody != null) {
				isValid = validateUri();
				if (isValid && VtnServiceConsts.POST.equalsIgnoreCase(method)) {
					isValid = validatePost(validator, requestBody);
				} else if (isValid && VtnServiceConsts.PUT.equals(method)) {
					isValid = validatePut(validator, requestBody);
				} else if (isValid) {
					setInvalidParameter(UncCommonEnum.UncResultCode.UNC_METHOD_NOT_ALLOWED
							.getMessage());
					isValid = false;
				}
			} else {
				setInvalidParameter(UncCommonEnum.UncResultCode.UNC_INVALID_FORMAT
						.getMessage());
			}
		} catch (final NumberFormatException e) {
			LOG.error(e, "Invalid value : " + e.getMessage());
			isValid = false;
		} catch (final ClassCastException e) {
			LOG.error(e, "Invalid type : " + e.getMessage());
			isValid = false;
		}

		/*
		 * throw exception in case of validation fail
		 */
		if (!isValid) {
			LOG.error("Validation failure");
			throw new VtnServiceException(Thread.currentThread()
					.getStackTrace()[1].getMethodName(),
					UncJavaAPIErrorCode.VALIDATION_ERROR.getErrorCode(),
					UncJavaAPIErrorCode.VALIDATION_ERROR.getErrorMessage());
		}
		LOG.info("Complete NetworkResourceValidator#validate()");
	}

	/**
	 * Validate resource specific URI parameters
	 * 
	 * @see org.opendaylight.vtn.javaapi.validation.VtnServiceValidator#validateUri
	 *      ()
	 */
	@Override
	public boolean validateUri() throws VtnServiceException {
		LOG.trace("Start NetworkResourceValidator#validateUri()");
		boolean isValid = true;
		if (resource instanceof NetworkResource) {
			// validation of tenant_id
			final String tenantId = ((NetworkResource) resource).getTenantId();
			if (tenantId == null
					|| tenantId.isEmpty()
					|| !validator.isValidMaxLengthAlphaNum(tenantId,
							VtnServiceJsonConsts.LEN_31)) {
				isValid = false;
				setInvalidParameter(VtnServiceOpenStackConsts.TENANT_ID
						+ VtnServiceConsts.COLON + tenantId);
			}

			// validation of net_id
			if (isValid) {
				final String netId = ((NetworkResource) resource).getNetId();
				if (netId == null
						|| netId.isEmpty()
						|| !validator.isValidMaxLengthAlphaNum(netId,
								VtnServiceJsonConsts.LEN_31)) {
					isValid = false;
					setInvalidParameter(VtnServiceOpenStackConsts.NET_ID
							+ VtnServiceConsts.COLON + netId);
				}
			}
		} else if (resource instanceof NetworksResource) {
			// validation of tenant_id
			final String tenantId = ((NetworksResource) resource).getTenantId();
			if (tenantId == null
					|| tenantId.isEmpty()
					|| !validator.isValidMaxLengthAlphaNum(tenantId,
							VtnServiceJsonConsts.LEN_31)) {
				isValid = false;
				setInvalidParameter(VtnServiceOpenStackConsts.TENANT_ID
						+ VtnServiceConsts.COLON + tenantId);
			}
		}
		LOG.trace("Complete NetworkResourceValidator#validateUri()");
		return isValid;
	}
}
