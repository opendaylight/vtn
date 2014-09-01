/*
 * Copyright (c) 2013-2014 NEC Corporation
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
import org.opendaylight.vtn.javaapi.resources.logical.VtnMappingResource;
import org.opendaylight.vtn.javaapi.resources.logical.VtnMappingsResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class VtnMappingResourceValidator validates request Json object for VTN
 * Mapping API.
 */
public class VtnMappingResourceValidator extends VtnServiceValidator {
	/**
	 * logger for debugging purpose.
	 */
	private static final Logger LOG = Logger
			.getLogger(VtnMappingResourceValidator.class.getName());

	/**
	 * Abstract resource reference.
	 */
	private final AbstractResource resource;
	/**
	 * Common validator object for common validation methods.
	 */
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new Vtn mapping resource validator.
	 * 
	 * @param mappingResource
	 *            , the instance of AbstractResource
	 */
	public VtnMappingResourceValidator(final AbstractResource mappingResource) {
		this.resource = mappingResource;
	}

	/**
	 * Validate uri parameters for Vtn Mapping API.
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start VtnMappingResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.VTNNAME);
		if (resource instanceof VtnMappingsResource
				&& ((VtnMappingsResource) resource).getVtnName() != null
				&& !((VtnMappingsResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VtnMappingsResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			setListOpFlag(true);
		} else if (resource instanceof VtnMappingResource
				&& ((VtnMappingResource) resource).getVtnName() != null
				&& !((VtnMappingResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VtnMappingResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.MAPPINGID);
				if (((VtnMappingResource) resource).getMappingId() != null
						&& !((VtnMappingResource) resource).getMappingId()
								.isEmpty()) {
					final String[] mappingId = ((VtnMappingResource) resource)
							.getMappingId().split(VtnServiceJsonConsts.HYPHEN);
					isValid = validator.isValidMappingId(mappingId);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		}
		LOG.trace("Completed VtnMappingResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request json for get, put and post method of VBridge API.
	 * 
	 * @param method
	 *            , for method type get,put post delete
	 * @param requestBody
	 *            , for request
	 * @throws VtnServiceException
	 *             , for vtn exceptions
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VtnMappingResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of VtnMappingResourceValidator");
		boolean isValid = false;
		try {
			isValid = validateUri();
			if (isValid && requestBody != null
					&& VtnServiceConsts.GET.equals(method)) {
				isValid = isValidGetForMappingIdIndex(requestBody,
						isListOpFlag());
				setInvalidParameter(validator.getInvalidParameter());
				updateOpParameterForList(requestBody);
			} else {
				setInvalidParameter(VtnServiceConsts.INCORRECT_METHOD_INVOCATION);
				isValid = false;
			}
		} catch (final NumberFormatException e) {
			LOG.error(e, "Inside catch:NumberFormatException");
			if (method.equals(VtnServiceConsts.GET)) {
				setInvalidParameter(validator.getInvalidParameter());
			}
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
		LOG.trace("Complete VtnMappingResourceValidator#validate()");
	}

	/**
	 * @param requestBody
	 *            , for request
	 * @param opFlag
	 *            , for getting option1 type
	 * @return true , if parameter are in correct format values
	 */
	public final boolean isValidGetForMappingIdIndex(
			final JsonObject requestBody, final boolean opFlag) {
		LOG.trace("Start CommonValidator#isValidGetForMappingIdIndex");
		boolean isValid = true;

		// validation for key: targetdb
		setInvalidParameter(VtnServiceJsonConsts.TARGETDB);
		isValid = validator.isValidRequestDBState(requestBody);

		/*
		 * Remove unwanted parameters from request body for Show APIs
		 */
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
				isValid = validator.isValidOperationForDetail(requestBody);
			}
			// validation for key: index where index is combination of
			// controller id and domain id
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.INDEX);
				if (requestBody.has(VtnServiceJsonConsts.INDEX)
						&& requestBody.getAsJsonPrimitive(
								VtnServiceJsonConsts.INDEX).getAsString() != null) {
					final String[] mappingId = requestBody
							.getAsJsonPrimitive(VtnServiceJsonConsts.INDEX)
							.getAsString().split(VtnServiceJsonConsts.HYPHEN);
					isValid = validator.isValidMappingId(mappingId);
				}
			}
			// validation for key: max_repitition
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.MAX);
				isValid = validator.isValidMaxRepetition(requestBody);
			}
			
			// validation for key: vnode_type
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.VNODETYPE);
				if (requestBody.has(VtnServiceJsonConsts.VNODETYPE)
						&& requestBody.getAsJsonPrimitive(
								VtnServiceJsonConsts.VNODETYPE).getAsString() != null) {
					final String vnodeType = requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.VNODETYPE).getAsString();
					isValid = validator.isValidVnodeType(vnodeType);
				}
			}

			// validation for key: vnode_name
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.VNODENAME);
				if (requestBody.has(VtnServiceJsonConsts.VNODENAME)
						&& requestBody.getAsJsonPrimitive(
								VtnServiceJsonConsts.VNODENAME).getAsString() != null) {
					final String vnodeName = requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.VNODENAME).getAsString();
					isValid = validator.isValidMaxLengthAlphaNum(vnodeName,
							VtnServiceJsonConsts.LEN_31);
				}
			}
			
			// validation for key: if_name
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.VIFNAME);
				if (requestBody.has(VtnServiceJsonConsts.VIFNAME)
						&& requestBody.getAsJsonPrimitive(
								VtnServiceJsonConsts.VIFNAME).getAsString() != null) {
					final String ifName = requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.VIFNAME).getAsString();
					isValid = validator.isValidMaxLengthAlphaNum(ifName,
							VtnServiceJsonConsts.LEN_31);
				}
			}

		}
		LOG.trace("Complete CommonValidator#isValidGetForMappingIdIndex");
		return isValid;
	}
}
