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
import org.opendaylight.vtn.javaapi.resources.physical.BoundariesResource;
import org.opendaylight.vtn.javaapi.resources.physical.BoundaryResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class BoundaryResourceValidator validates .
 */
public class BoundaryResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(BoundaryResourceValidator.class.getName());

	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new Boundary resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public BoundaryResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for Boundary API
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start BoundaryResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.BOUNDARYID);
		if (resource instanceof BoundaryResource
				&& ((BoundaryResource) resource).getboundaryId() != null
				&& !((BoundaryResource) resource).getboundaryId().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((BoundaryResource) resource).getboundaryId(),
					VtnServiceJsonConsts.LEN_31);
			setListOpFlag(false);
		} else if (resource instanceof BoundariesResource) {
			isValid = true;
			setListOpFlag(true);
		}
		LOG.trace("Complete BoundaryResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request json for put, post and get methods of Boundary API
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start BoundaryResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of VtnResourceValidator");
		boolean isValid = false;
		try {
			isValid = validateUri();
			if (isValid && requestBody != null
					&& VtnServiceConsts.GET.equals(method)) {
				isValid = isValidateGet(requestBody, isListOpFlag());
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
		LOG.trace("Complete BoundaryResourceValidator#validate()");
	}

	/**
	 * Validate request json for get method of Boundary API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 */
	private boolean isValidateGet(final JsonObject requestBody,
			final boolean opFlag) {
		LOG.trace("Start BoundaryResourceValidator#isValidateGet()");
		boolean isValid = true;
		if (isValid) {
			// validation for key: targetdb
			setInvalidParameter(VtnServiceJsonConsts.TARGETDB);
			isValid = validator.isValidRequestDB(requestBody);
		}
		if (!opFlag) {
			if (requestBody.has(VtnServiceJsonConsts.OP)) {
				requestBody.remove(VtnServiceJsonConsts.OP);
			} else {
				LOG.debug("No need to remove -op ");
			}
			if (requestBody.has(VtnServiceJsonConsts.INDEX)) {
				requestBody.remove(VtnServiceJsonConsts.INDEX);
			} else {
				LOG.debug("No need to remove - index");
			}
			if (requestBody.has(VtnServiceJsonConsts.MAX)) {
				requestBody.remove(VtnServiceJsonConsts.MAX);
			} else {
				LOG.debug("No need to remove-max count");
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
					isValid = validator.isValidMaxLengthAlphaNum(requestBody
							.getAsJsonPrimitive(VtnServiceJsonConsts.INDEX)
							.getAsString(), VtnServiceJsonConsts.LEN_31);
				}
			}
			// validation for key: max_repitition
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.MAX);
				isValid = validator.isValidMaxRepetition(requestBody);
			}
			// validation for key: controller1_id
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.CONTROLLER1ID);
				if (requestBody.has(VtnServiceJsonConsts.CONTROLLER1ID)
						&& requestBody.getAsJsonPrimitive(
								VtnServiceJsonConsts.CONTROLLER1ID)
								.getAsString() != null
						&& !requestBody
								.getAsJsonPrimitive(
										VtnServiceJsonConsts.CONTROLLER1ID)
								.getAsString().isEmpty()) {
					isValid = validator
							.isValidMaxLengthAlphaNum(
									requestBody.getAsJsonPrimitive(
											VtnServiceJsonConsts.CONTROLLER1ID)
											.getAsString(),
									VtnServiceJsonConsts.LEN_31);
				}
			}
			// validation for key: controller2_id
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.CONTROLLER2ID);
				if (requestBody.has(VtnServiceJsonConsts.CONTROLLER2ID)
						&& requestBody.getAsJsonPrimitive(
								VtnServiceJsonConsts.CONTROLLER2ID)
								.getAsString() != null
						&& !requestBody
								.getAsJsonPrimitive(
										VtnServiceJsonConsts.CONTROLLER2ID)
								.getAsString().isEmpty()) {
					isValid = validator
							.isValidMaxLengthAlphaNum(
									requestBody.getAsJsonPrimitive(
											VtnServiceJsonConsts.CONTROLLER2ID)
											.getAsString(),
									VtnServiceJsonConsts.LEN_31);
				}
			}
		}
		LOG.trace("Complete BoundaryResourceValidator#isValidateGet");
		return isValid;
	}

	/**
	 * Validate post request json for create Boundary API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 */
	private boolean validatePost(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start BoundaryResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.BOUNDARY);
		if (requestBody.has(VtnServiceJsonConsts.BOUNDARY)
				&& requestBody.get(VtnServiceJsonConsts.BOUNDARY)
						.isJsonObject()) {
			final JsonObject boundary = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.BOUNDARY);
			// validation for mandatory key: boundary_name(mandatory)
			setInvalidParameter(VtnServiceJsonConsts.BOUNDARYID);
			if (boundary.has(VtnServiceJsonConsts.BOUNDARYID)
					&& boundary.getAsJsonPrimitive(
							VtnServiceJsonConsts.BOUNDARYID).getAsString() != null) {
				isValid = validator.isValidMaxLengthAlphaNum(boundary
						.getAsJsonPrimitive(VtnServiceJsonConsts.BOUNDARYID)
						.getAsString(), VtnServiceJsonConsts.LEN_31);
			}
			if (isValid) {
				// validation for key: description
				setInvalidParameter(VtnServiceJsonConsts.DESCRIPTION);
				if (boundary.has(VtnServiceJsonConsts.DESCRIPTION)
						&& boundary.getAsJsonPrimitive(
								VtnServiceJsonConsts.DESCRIPTION).getAsString() != null
						&& !boundary
								.getAsJsonPrimitive(
										VtnServiceJsonConsts.DESCRIPTION)
								.getAsString().isEmpty()) {
					isValid = validator.isValidMaxLength(
							boundary.getAsJsonPrimitive(
									VtnServiceJsonConsts.DESCRIPTION)
									.getAsString(),
							VtnServiceJsonConsts.LEN_127);
				}
			}
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.LINK);
				if (isValid && boundary.has(VtnServiceJsonConsts.LINK)) {
					final JsonObject link = boundary
							.getAsJsonObject(VtnServiceJsonConsts.LINK);
					// validation for key: controller1id (mandatory)
					setInvalidParameter(VtnServiceJsonConsts.CONTROLLER1ID);
					if (link.has(VtnServiceJsonConsts.CONTROLLER1ID)
							&& link.getAsJsonPrimitive(
									VtnServiceJsonConsts.CONTROLLER1ID)
									.getAsString() != null) {
						isValid = validator.isValidMaxLengthAlphaNum(
								link.getAsJsonPrimitive(
										VtnServiceJsonConsts.CONTROLLER1ID)
										.getAsString(),
								VtnServiceJsonConsts.LEN_31);
					} else {
						isValid = false;
					}
					if (isValid) {
						// validation for key: domain1_id (mandatory)
						setInvalidParameter(VtnServiceJsonConsts.DOMAIN1_ID);
						if (link.has(VtnServiceJsonConsts.DOMAIN1_ID)
								&& link.getAsJsonPrimitive(
										VtnServiceJsonConsts.DOMAIN1_ID)
										.getAsString() != null) {
							isValid = validator.isValidDomainId(
									link.getAsJsonPrimitive(
											VtnServiceJsonConsts.DOMAIN1_ID)
											.getAsString(),
									VtnServiceJsonConsts.LEN_31);
						} else {
							isValid = false;
						}
					}
					if (isValid) {
						// validation for key: logical_port1_id (optional)
						setInvalidParameter(VtnServiceJsonConsts.LOGICAL_PORT1_ID);
						if (link.has(VtnServiceJsonConsts.LOGICAL_PORT1_ID)
								&& link.getAsJsonPrimitive(
										VtnServiceJsonConsts.LOGICAL_PORT1_ID)
										.getAsString() != null
								&& !link.getAsJsonPrimitive(
										VtnServiceJsonConsts.LOGICAL_PORT1_ID)
										.getAsString().isEmpty()) {
							isValid = validator
									.isValidMaxLength(
											link.getAsJsonPrimitive(
													VtnServiceJsonConsts.LOGICAL_PORT1_ID)
													.getAsString(),
											VtnServiceJsonConsts.LEN_319);
						}
					}
					if (isValid) {
						// validation for key: controller2_id (mandatory)
						setInvalidParameter(VtnServiceJsonConsts.CONTROLLER2ID);
						if (link.has(VtnServiceJsonConsts.CONTROLLER2ID)
								&& link.getAsJsonPrimitive(
										VtnServiceJsonConsts.CONTROLLER2ID)
										.getAsString() != null) {
							isValid = validator.isValidMaxLengthAlphaNum(
									link.getAsJsonPrimitive(
											VtnServiceJsonConsts.CONTROLLER2ID)
											.getAsString(),
									VtnServiceJsonConsts.LEN_31);
						} else {
							isValid = false;
						}
					}
					if (isValid) {
						// validation for key: domain2_id (mandatory)
						setInvalidParameter(VtnServiceJsonConsts.DOMAIN2_ID);
						if (link.has(VtnServiceJsonConsts.DOMAIN2_ID)
								&& link.getAsJsonPrimitive(
										VtnServiceJsonConsts.DOMAIN2_ID)
										.getAsString() != null) {
							isValid = validator.isValidDomainId(
									link.getAsJsonPrimitive(
											VtnServiceJsonConsts.DOMAIN2_ID)
											.getAsString(),
									VtnServiceJsonConsts.LEN_31);
						} else {
							isValid = false;
						}
					}
					if (isValid) {
						// validation for key: logical_port2_id (optional)
						setInvalidParameter(VtnServiceJsonConsts.LOGICAL_PORT2_ID);
						if (link.has(VtnServiceJsonConsts.LOGICAL_PORT2_ID)
								&& link.getAsJsonPrimitive(
										VtnServiceJsonConsts.LOGICAL_PORT2_ID)
										.getAsString() != null
								&& !link.getAsJsonPrimitive(
										VtnServiceJsonConsts.LOGICAL_PORT2_ID)
										.getAsString().isEmpty()) {
							isValid = validator
									.isValidMaxLength(
											link.getAsJsonPrimitive(
													VtnServiceJsonConsts.LOGICAL_PORT2_ID)
													.getAsString(),
											VtnServiceJsonConsts.LEN_319);
						}
					}
				} else {
					isValid = false;
				}
			}
		} else {
			isValid = false;
		}
		LOG.trace("Complete BoundaryResourceValidator#validatePost()");
		return isValid;
	}

	/**
	 * Validate put request json for update Boundary API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * 
	 * @return true, if successful
	 */
	private boolean validatePut(final JsonObject requestBody) {
		LOG.trace("Start BoundaryResourceValidator#validatePut()");
		boolean isValid = true;
		setInvalidParameter(VtnServiceJsonConsts.BOUNDARY);
		if (requestBody.has(VtnServiceJsonConsts.BOUNDARY)
				&& requestBody.get(VtnServiceJsonConsts.BOUNDARY)
						.isJsonObject()) {
			final JsonObject boundary = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.BOUNDARY);
			// validation for key: description(optional)
			setInvalidParameter(VtnServiceJsonConsts.DESCRIPTION);
			if (boundary.has(VtnServiceJsonConsts.DESCRIPTION)
					&& boundary.getAsJsonPrimitive(
							VtnServiceJsonConsts.DESCRIPTION).getAsString() != null
					&& !boundary
							.getAsJsonPrimitive(
									VtnServiceJsonConsts.DESCRIPTION)
							.getAsString().isEmpty()) {
				isValid = validator.isValidMaxLength(boundary
						.getAsJsonPrimitive(VtnServiceJsonConsts.DESCRIPTION)
						.getAsString(), VtnServiceJsonConsts.LEN_127);
			}
			if (isValid && boundary.has(VtnServiceJsonConsts.LINK)) {
				boundary.remove(VtnServiceJsonConsts.LINK);
			}
		} else {
			isValid = false;
		}
		LOG.trace("Complete BoundaryResourceValidator#validatePut()");
		return isValid;
	}
}
