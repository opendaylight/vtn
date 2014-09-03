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
import org.opendaylight.vtn.javaapi.resources.logical.VLinkResource;
import org.opendaylight.vtn.javaapi.resources.logical.VLinksResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class VLinkResourceValidator validates request Json object for Vlink API.
 */
public class VLinkResourceValidator extends VtnServiceValidator {
	/**
	 * logger for debugging.
	 */
	private static final Logger LOG = Logger
			.getLogger(VLinkResourceValidator.class.getName());
	/**
	 * Abstract resource.
	 */
	private final AbstractResource resource;
	/**
	 * common validations.
	 */
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new vlink resource validator.
	 * 
	 * @param mappingResource
	 *            the instance of AbstractResource
	 */
	public VLinkResourceValidator(final AbstractResource mappingResource) {
		this.resource = mappingResource;
	}

	/**
	 * Validate uri parameters for Vlink API.
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start VLinkResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.VTNNAME);
		if (resource instanceof VLinkResource
				&& ((VLinkResource) resource).getVtnName() != null
				&& !((VLinkResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VLinkResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VLKNAME);
				if (((VLinkResource) resource).getVlkName() != null
						&& !((VLinkResource) resource).getVlkName().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VLinkResource) resource).getVlkName(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		} else if (resource instanceof VLinksResource
				&& ((VLinksResource) resource).getVtnName() != null
				&& !((VLinksResource) resource).getVtnName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((VLinksResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			setListOpFlag(true);
		}
		LOG.trace("Complete VLinkResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request Json object for get, put and post method of Vlink API.
	 */
	/**
	 * @param method
	 *            , to get type of method
	 * @param requestBody
	 *            , for request
	 * @throws VtnServiceException
	 *             , for vtn exception
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VLinkResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of FlowListResourceValidator");
		boolean isValid = false;
		try {
			isValid = validateUri();
			if (isValid && requestBody != null
					&& VtnServiceConsts.GET.equals(method)) {
				isValid = validateGet(requestBody);
				setInvalidParameter(validator.getInvalidParameter());
				updateOpParameterForList(requestBody);
			} else if (isValid && requestBody != null
					&& VtnServiceConsts.PUT.equals(method)) {
				isValid = validatePut(requestBody);
			} else if (isValid && requestBody != null
					&& VtnServiceConsts.POST.equals(method)) {
				isValid = validatePost(requestBody);
			} else if (isValid) {
				setInvalidParameter(VtnServiceConsts.INCORRECT_METHOD_INVOCATION);
				isValid = false;
			}
		} catch (final NumberFormatException e) {
			if (method.equals(VtnServiceConsts.GET)) {
				setInvalidParameter(validator.getInvalidParameter());
			}
			LOG.error(e, "Inside catch:NumberFormatException");
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
		LOG.trace("Complete VLinkResourceValidator#validate()");

	}

	/**
	 * Validate get request Json object for Vlink API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validateGet(final JsonObject requestBody) {
		boolean isValid = false;
		isValid = validator.isValidGet(requestBody, isListOpFlag());
		setInvalidParameter(validator.getInvalidParameter());
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.VNODE1NAME);
			if (requestBody.has(VtnServiceJsonConsts.VNODE1NAME)
					&& requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.VNODE1NAME).getAsString() != null) {
				isValid = validator.isValidMaxLengthAlphaNum(requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.VNODE1NAME)
						.getAsString(), VtnServiceJsonConsts.LEN_31)
						|| requestBody
								.getAsJsonPrimitive(
										VtnServiceJsonConsts.VNODE1NAME)
								.getAsString().isEmpty();
			}
		}
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.VNODE2NAME);
			if (requestBody.has(VtnServiceJsonConsts.VNODE2NAME)
					&& requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.VNODE2NAME).getAsString() != null) {
				isValid = validator.isValidMaxLengthAlphaNum(requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.VNODE2NAME)
						.getAsString(), VtnServiceJsonConsts.LEN_31)
						|| requestBody
								.getAsJsonPrimitive(
										VtnServiceJsonConsts.VNODE2NAME)
								.getAsString().isEmpty();
			}
		}
		return isValid;
	}

	/**
	 * validate post request Json object for Vlink API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	private boolean validatePost(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VLinkResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.VLINK);
		if (requestBody.has(VtnServiceJsonConsts.VLINK)
				&& requestBody.get(VtnServiceJsonConsts.VLINK).isJsonObject()) {
			final JsonObject vlinkResourceEntry = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VLINK);
			// validation for key: vlk_name(mandatory)
			setInvalidParameter(VtnServiceJsonConsts.VLKNAME);
			if (vlinkResourceEntry.has(VtnServiceJsonConsts.VLKNAME)
					&& vlinkResourceEntry.getAsJsonPrimitive(
							VtnServiceJsonConsts.VLKNAME).getAsString() != null
					&& !vlinkResourceEntry
							.getAsJsonPrimitive(VtnServiceJsonConsts.VLKNAME)
							.getAsString().isEmpty()) {
				isValid = validator.isValidMaxLengthAlphaNum(vlinkResourceEntry
						.getAsJsonPrimitive(VtnServiceJsonConsts.VLKNAME)
						.getAsString(), VtnServiceJsonConsts.LEN_31);
			}
			// validation for key: vnode1_name(mandatory)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.VNODE1NAME);
				if (vlinkResourceEntry.has(VtnServiceJsonConsts.VNODE1NAME)
						&& vlinkResourceEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.VNODE1NAME).getAsString() != null) {
					isValid = validator
							.isValidMaxLengthAlphaNum(
									vlinkResourceEntry.getAsJsonPrimitive(
											VtnServiceJsonConsts.VNODE1NAME)
											.getAsString(),
									VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			// validation for key: if1_name(mandatory)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.IF1NAME);
				if (vlinkResourceEntry.has(VtnServiceJsonConsts.IF1NAME)
						&& vlinkResourceEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.IF1NAME).getAsString() != null) {
					isValid = validator
							.isValidMaxLengthAlphaNum(
									vlinkResourceEntry.getAsJsonPrimitive(
											VtnServiceJsonConsts.IF1NAME)
											.getAsString(),
									VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			// validation for key: vnode2_name(mandatory)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.VNODE2NAME);
				if (vlinkResourceEntry.has(VtnServiceJsonConsts.VNODE2NAME)
						&& vlinkResourceEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.VNODE2NAME).getAsString() != null) {
					isValid = validator
							.isValidMaxLengthAlphaNum(
									vlinkResourceEntry.getAsJsonPrimitive(
											VtnServiceJsonConsts.VNODE2NAME)
											.getAsString(),
									VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			// validation for key: if2_name(mandatory)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.IF2NAME);
				if (vlinkResourceEntry.has(VtnServiceJsonConsts.IF2NAME)
						&& vlinkResourceEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.IF2NAME).getAsString() != null) {
					isValid = validator
							.isValidMaxLengthAlphaNum(
									vlinkResourceEntry.getAsJsonPrimitive(
											VtnServiceJsonConsts.IF2NAME)
											.getAsString(),
									VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
				if (isValid) {
					isValid = commonValidation(vlinkResourceEntry, true);
				}
			}
		} else {
			isValid = false;
		}
		LOG.trace("Complete VLinkResourceValidator#validatePost()");
		return isValid;
	}

	/**
	 * validate put request Json object for Vlink API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	private boolean validatePut(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VLinkResourceValidator#validatePut()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.VLINK);
		if (requestBody.has(VtnServiceJsonConsts.VLINK)
				&& requestBody.get(VtnServiceJsonConsts.VLINK).isJsonObject()) {
			final JsonObject vlinkResourceEntry = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VLINK);
			isValid = commonValidation(vlinkResourceEntry, false);
		} else {
			isValid = false;
		}
		LOG.trace("Complete VLinkResourceValidator#validatePut()");
		return isValid;
	}

	/**
	 * Common validation for description, admin_status and boundary for put and
	 * post APIs.
	 * 
	 * @param vlinkResourceEntry
	 *            the vlink resource entry
	 * @return true, if successful
	 */
	private boolean commonValidation(final JsonObject vlinkResourceEntry, boolean isPost) {
		LOG.trace("Start VLinkResourceValidator#commonValidation()");
		boolean isValid = true;
		boolean isBoundaryValid = false;
		// validation for key: description(optional)
		setInvalidParameter(VtnServiceJsonConsts.DESCRIPTION);
		if (vlinkResourceEntry.has(VtnServiceJsonConsts.DESCRIPTION)
				&& vlinkResourceEntry.getAsJsonPrimitive(
						VtnServiceJsonConsts.DESCRIPTION).getAsString() != null) {
			isValid = validator.isValidMaxLength(vlinkResourceEntry
					.getAsJsonPrimitive(VtnServiceJsonConsts.DESCRIPTION)
					.getAsString(), VtnServiceJsonConsts.LEN_127)
					|| vlinkResourceEntry
							.getAsJsonPrimitive(
									VtnServiceJsonConsts.DESCRIPTION)
							.getAsString().isEmpty();
		}
		// validation for key: adminstatus(optional)
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.ADMINSTATUS);
			if (vlinkResourceEntry.has(VtnServiceJsonConsts.ADMINSTATUS)
					&& vlinkResourceEntry.getAsJsonPrimitive(
							VtnServiceJsonConsts.ADMINSTATUS).getAsString() != null) {
				final String adminstatus = vlinkResourceEntry
						.getAsJsonPrimitive(VtnServiceJsonConsts.ADMINSTATUS)
						.getAsString();
				isValid = adminstatus
						.equalsIgnoreCase(VtnServiceJsonConsts.ENABLE)
						|| adminstatus
								.equalsIgnoreCase(VtnServiceJsonConsts.DISABLE);
			}
		}
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.BOUNDARYMAP);
			if (vlinkResourceEntry.has(VtnServiceJsonConsts.BOUNDARYMAP)
					&& vlinkResourceEntry.get(VtnServiceJsonConsts.BOUNDARYMAP)
							.isJsonObject()) {
				final JsonObject boundary = vlinkResourceEntry
						.getAsJsonObject(VtnServiceJsonConsts.BOUNDARYMAP);

				//check the boundary_id
				setInvalidParameter(VtnServiceJsonConsts.BOUNDARYID);
				if (boundary.has(VtnServiceJsonConsts.BOUNDARYID)
						&& boundary.getAsJsonPrimitive(
								VtnServiceJsonConsts.BOUNDARYID)
								.getAsString() != null) {
					if (!boundary.getAsJsonPrimitive(
									VtnServiceJsonConsts.BOUNDARYID)
							.getAsString().isEmpty()) {
						if (!validator.isValidMaxLengthAlphaNum(
								boundary.getAsJsonPrimitive(
								VtnServiceJsonConsts.BOUNDARYID)
								.getAsString(), VtnServiceJsonConsts.LEN_31)) {
							//if boundary_id is invalid in post or put, no need to check the vlan_id or no_vlan_id
							LOG.trace("Complete VLinkResourceValidator#commonValidation() 2 ");
							return false;
						} else {
							isBoundaryValid = true;	
						}
					} 
				}

				// either vlan_id or no_vlan_id should be there
				setInvalidParameter(VtnServiceJsonConsts.VLANID
						+ VtnServiceJsonConsts.SLASH
						+ VtnServiceJsonConsts.NO_VLAN_ID);
				if (boundary.has(VtnServiceJsonConsts.VLANID)
						&& boundary.has(VtnServiceJsonConsts.NO_VLAN_ID)) {
					isValid = false;
				} else if (boundary.has(VtnServiceJsonConsts.VLANID)) {
					setInvalidParameter(VtnServiceJsonConsts.VLANID);
					if (boundary
							.getAsJsonPrimitive(VtnServiceJsonConsts.VLANID)
							.getAsString() != null
							&& !boundary
									.getAsJsonPrimitive(
											VtnServiceJsonConsts.VLANID)
									.getAsString().trim().isEmpty()) {
						isValid = validator.isValidRange(
								boundary.getAsJsonPrimitive(
										VtnServiceJsonConsts.VLANID)
										.getAsString(),
								VtnServiceJsonConsts.VAL_1,
								VtnServiceJsonConsts.VAL_4095);
					} else {
						isValid = false;
					}
				} else if (boundary.has(VtnServiceJsonConsts.NO_VLAN_ID)) {
					setInvalidParameter(VtnServiceJsonConsts.NO_VLAN_ID);
					if (boundary.getAsJsonPrimitive(
							VtnServiceJsonConsts.NO_VLAN_ID).getAsString() != null
							&& !boundary
									.getAsJsonPrimitive(
											VtnServiceJsonConsts.NO_VLAN_ID)
									.getAsString().isEmpty()) {
						isValid = boundary
								.getAsJsonPrimitive(
										VtnServiceJsonConsts.NO_VLAN_ID)
								.getAsString()
								.equalsIgnoreCase(VtnServiceJsonConsts.TRUE);
					} else {
						isValid = false;
					}
				} else {
					if (isPost && isBoundaryValid) {
						isValid = false;
					}
				}
			}
		}
		LOG.trace("Complete VLinkResourceValidator#commonValidation()");
		return isValid;
	}
}
