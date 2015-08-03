/*
 * Copyright (c) 2012-2015 NEC Corporation
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
import org.opendaylight.vtn.javaapi.resources.physical.ControllerResource;
import org.opendaylight.vtn.javaapi.resources.physical.ControllersResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

public class ControllerResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(ControllerResourceValidator.class.getName());

	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new controller resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public ControllerResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for Controller API
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start ControllerResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.CONTROLLERID);
		if (resource instanceof ControllerResource
				&& ((ControllerResource) resource).getControllerId() != null
				&& !((ControllerResource) resource).getControllerId().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((ControllerResource) resource).getControllerId(),
					VtnServiceJsonConsts.LEN_31);
			setListOpFlag(false);
		} else if (resource instanceof ControllersResource) {
			isValid = true;
			setListOpFlag(true);
		}
		LOG.trace("Complete ControllerResourceValidator#validateUri()");
		return isValid;
	}

	/*
	 * Validate request json for put, post and get method of controller API
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start ControlleResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of ControllerResourceValidator");
		boolean isValid = false;
		try {
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
		LOG.trace("Complete ControllerResourceValidator#validate()");
	}

	/**
	 * Validate request json for get method of controller API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * 
	 * @return true, if is valid get
	 */
	public final boolean validateGet(final JsonObject requestBody,
			final boolean opFlag) {
		LOG.trace("Start ControllerResourceValidator#isValidGet");
		boolean isValid = true;
		// validation for key: targetdb
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.TARGETDB);
			isValid = validator.isValidRequestDB(requestBody);
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
			// updated validation for max_repetition_count
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.MAX);
				isValid = validator.isValidMaxRepetition(requestBody);
				return isValid;
			}
		}
		LOG.trace("Complete ControllerResourceValidator#isValidGet");
		return isValid;
	}

	/**
	 * Validate request json for post method of controller API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * 
	 * @return true, if successful
	 */
	private boolean validatePost(final JsonObject requestBody) {
		LOG.trace("Start ControllerResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.CONTROLLER);
		if (requestBody.has(VtnServiceJsonConsts.CONTROLLER)
				&& requestBody.get(VtnServiceJsonConsts.CONTROLLER)
						.isJsonObject()) {
			final JsonObject controller = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.CONTROLLER);
			// validation for mandatory key: controllerId(mandatory)
			setInvalidParameter(VtnServiceJsonConsts.CONTROLLERID);
			if (controller.has(VtnServiceJsonConsts.CONTROLLERID)
					&& controller.getAsJsonPrimitive(
							VtnServiceJsonConsts.CONTROLLERID).getAsString() != null) {
				isValid = validator.isValidMaxLengthAlphaNum(controller
						.getAsJsonPrimitive(VtnServiceJsonConsts.CONTROLLERID)
						.getAsString(), VtnServiceJsonConsts.LEN_31);
			}
			if (isValid) {
				// validation for key: type(mandatory)
				setInvalidParameter(VtnServiceJsonConsts.TYPE);
				if (controller.has(VtnServiceJsonConsts.TYPE)
						&& controller.getAsJsonPrimitive(
								VtnServiceJsonConsts.TYPE).getAsString() != null) {
					isValid = validator.isValidType(controller
							.getAsJsonPrimitive(VtnServiceJsonConsts.TYPE)
							.getAsString());
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				// validation for key: version(mandatory)
				setInvalidParameter(VtnServiceJsonConsts.VERSION);
				if (controller.has(VtnServiceJsonConsts.VERSION)
						&& controller.getAsJsonPrimitive(
								VtnServiceJsonConsts.VERSION).getAsString() != null) {
					isValid = validator.isValidVersion(controller
							.getAsJsonPrimitive(VtnServiceJsonConsts.VERSION)
							.getAsString(), VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			if (isValid) {
				isValid = validateField(isValid, controller);
			}
		}
		LOG.trace("Complete ControllerResourceValidator#validatePost()");
		return isValid;
	}

	/**
	 * Validate common request for post and put method of controller API
	 * 
	 * @param isValid
	 *            boolean variable
	 * @param controller
	 *            the request Json object
	 * 
	 * @return true, if successful
	 */
	private boolean validateField(boolean isValid, final JsonObject controller) {
		// validation for key: description
		setInvalidParameter(VtnServiceJsonConsts.DESCRIPTION);
		if (controller.has(VtnServiceJsonConsts.DESCRIPTION)
				&& controller.getAsJsonPrimitive(
						VtnServiceJsonConsts.DESCRIPTION).getAsString() != null
				&& !controller
						.getAsJsonPrimitive(VtnServiceJsonConsts.DESCRIPTION)
						.getAsString().isEmpty()) {
			isValid = validator.isValidMaxLength(
					controller.getAsJsonPrimitive(
							VtnServiceJsonConsts.DESCRIPTION).getAsString(),
					VtnServiceJsonConsts.LEN_127);
		}
		if (isValid) {
			// validation for key: ipaddr
			setInvalidParameter(VtnServiceJsonConsts.IPADDR);
			if (controller.has(VtnServiceJsonConsts.IPADDR)
					&& controller.getAsJsonPrimitive(
							VtnServiceJsonConsts.IPADDR).getAsString() != null) {
				isValid = validator.isValidIpV4(controller.getAsJsonPrimitive(
						VtnServiceJsonConsts.IPADDR).getAsString());
			}
		}
		if (isValid) {
			// validation for key: audit_status
			setInvalidParameter(VtnServiceJsonConsts.AUDITSTATUS);
			if (controller.has(VtnServiceJsonConsts.AUDITSTATUS)
					&& controller.getAsJsonPrimitive(
							VtnServiceJsonConsts.AUDITSTATUS).getAsString() != null) {
				isValid = validator.isValidAuditStatus(controller
						.getAsJsonPrimitive(VtnServiceJsonConsts.AUDITSTATUS)
						.getAsString());
			}
		}
		if (isValid) {
			// validation for key: username
			setInvalidParameter(VtnServiceJsonConsts.USERNAME);
			if (controller.has(VtnServiceJsonConsts.USERNAME)
					&& controller.getAsJsonPrimitive(
							VtnServiceJsonConsts.USERNAME).getAsString() != null) {
				isValid = validator.isValidMaxLength(controller
						.getAsJsonPrimitive(VtnServiceJsonConsts.USERNAME)
						.getAsString(), VtnServiceJsonConsts.LEN_31);
			}
		}
		if (isValid) {
			// validation for key: password
			setInvalidParameter(VtnServiceJsonConsts.PASSWORD);
			if (controller.has(VtnServiceJsonConsts.PASSWORD)
					&& controller.getAsJsonPrimitive(
							VtnServiceJsonConsts.PASSWORD).getAsString() != null) {
				isValid = validator.isValidMaxLength(controller
						.getAsJsonPrimitive(VtnServiceJsonConsts.PASSWORD)
						.getAsString(), VtnServiceJsonConsts.LEN_256);
			}
		}
		if (isValid) {
			// validation for key: port
			setInvalidParameter(VtnServiceJsonConsts.PORT);
			if (controller.has(VtnServiceJsonConsts.PORT)
					&& controller.getAsJsonPrimitive(
							VtnServiceJsonConsts.PORT).getAsString() != null) {
				isValid = validator.isValidRange(controller
						.getAsJsonPrimitive(VtnServiceJsonConsts.PORT)
						.getAsString(), VtnServiceJsonConsts.VAL_0,
						VtnServiceJsonConsts.VAL_65535);
			}
		}
		return isValid;
	}

	/**
	 * Validate request json for put method of controller API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * 
	 * @return true, if successful
	 */
	private boolean validatePut(final JsonObject requestBody) {
		LOG.trace("Start ControllerResourceValidator#validatePut()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.CONTROLLER);
		if (requestBody.has(VtnServiceJsonConsts.CONTROLLER)
				&& requestBody.get(VtnServiceJsonConsts.CONTROLLER)
						.isJsonObject()) {
			isValid = true;
			final JsonObject controller = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.CONTROLLER);
			/*
			 * // validation for key: type
			 * setInvalidParameter(VtnServiceJsonConsts.TYPE); if
			 * (controller.has(VtnServiceJsonConsts.TYPE) &&
			 * controller.getAsJsonPrimitive(
			 * VtnServiceJsonConsts.TYPE).getAsString() != null) { isValid =
			 * validator.isValidType(controller
			 * .getAsJsonPrimitive(VtnServiceJsonConsts.TYPE) .getAsString()); }
			 */

			// Remove type if present in request JSON
			if (controller.has(VtnServiceJsonConsts.TYPE)) {
				requestBody.getAsJsonObject(VtnServiceJsonConsts.CONTROLLER)
						.remove(VtnServiceJsonConsts.TYPE);
			}
			// validation for key: version
			setInvalidParameter(VtnServiceJsonConsts.VERSION);
			if (controller.has(VtnServiceJsonConsts.VERSION)
					&& controller.getAsJsonPrimitive(
							VtnServiceJsonConsts.VERSION).getAsString() != null
					&& !controller
							.getAsJsonPrimitive(VtnServiceJsonConsts.VERSION)
							.getAsString().isEmpty()) {
				isValid = validator.isValidVersion(controller
						.getAsJsonPrimitive(VtnServiceJsonConsts.VERSION)
						.getAsString(), VtnServiceJsonConsts.LEN_31);
			}
			if (isValid) {
				isValid = validateField(isValid, controller);
			}
		}
		LOG.trace("Complete ControllerResourceValidator#validatePut()");
		return isValid;
	}
}
