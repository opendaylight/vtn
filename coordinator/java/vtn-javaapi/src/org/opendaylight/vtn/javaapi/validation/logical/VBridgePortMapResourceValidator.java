/*
 * Copyright (c) 2015 NEC Corporation
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
import org.opendaylight.vtn.javaapi.resources.logical.VBridgePortMapResource;
import org.opendaylight.vtn.javaapi.resources.logical.VBridgePortMapsResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class VBridgePortMapResourceValidator validates request Json
 * object for VBridgePortMap and VBridgePortMaps API.
 */
public class VBridgePortMapResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(VBridgePortMapResourceValidator.class.getName());

	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new vBridge portmap resource validator.
	 * 
	 * @param resource the instance of AbstractResource
	 * 
	 */
	public VBridgePortMapResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for VBridgePortMap and VBridgePortMaps API
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start VBridgePortMapResourceValidator#validateUri()");
		
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.VTNNAME);
		
		// For VBridgePortMapsResource instance
		if (resource instanceof VBridgePortMapsResource
				&& ((VBridgePortMapsResource) resource).getVtnName() != null
				&& !((VBridgePortMapsResource) resource).getVtnName().isEmpty()) {
			
			isValid = validator.isValidMaxLengthAlphaNum(
					((VBridgePortMapsResource) resource).getVtnName(),
					VtnServiceJsonConsts.LEN_31);
			
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VBRNAME);
				if (((VBridgePortMapsResource) resource).getVbrName() != null
						&& !((VBridgePortMapsResource) resource).getVbrName().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VBridgePortMapsResource) resource).getVbrName(), 
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			
			setListOpFlag(true);
		} 
		// For VBridgePortMapResource instance
		else if (resource instanceof VBridgePortMapResource
				&& ((VBridgePortMapResource) resource).getVtnName() != null
				&& !((VBridgePortMapResource) resource).getVtnName().isEmpty()) {
			
			
			isValid = validator.isValidMaxLengthAlphaNum(
					((VBridgePortMapResource) resource).getVtnName(), 
					VtnServiceJsonConsts.LEN_31);
			
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.VBRNAME);
				if (((VBridgePortMapResource) resource).getVbrName() != null
						&& !((VBridgePortMapResource) resource).getVbrName().isEmpty()) {
					isValid = validator.isValidMaxLengthAlphaNum(
							((VBridgePortMapResource) resource).getVbrName(),
							VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.PORTMAP_NAME);
				if (((VBridgePortMapResource) resource).getPortmapName() != null
						&& !((VBridgePortMapResource) resource).getPortmapName().isEmpty()) {
					isValid = validator
							.isValidMaxLengthAlphaNum(
									((VBridgePortMapResource) resource).getPortmapName(),
									VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			
			setListOpFlag(false);
		}
		
		LOG.trace("Complete VBridgePortMapResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request Json object for get, put and post method of VBridgePortMap and VBridgePortMaps API
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VBridgePortMapResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of VBridgePortMapResourceValidator");
		
		boolean isValid = false;
		
		try {
			isValid = validateUri();
			if (isValid && requestBody != null
					&& VtnServiceConsts.GET.equals(method)) {
				isValid = validator.isValidGet(requestBody, isListOpFlag());
				setInvalidParameter(validator.getInvalidParameter());
				updateOpParameterForList(requestBody);
			} else if (isValid && requestBody != null
					&& VtnServiceConsts.POST.equalsIgnoreCase(method)) {
				isValid = validatePost(requestBody);
			} else if (isValid && requestBody != null
					&& VtnServiceConsts.PUT.equals(method)) {
				isValid = validatePut(requestBody);
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
		LOG.trace("Complete VBridgePortMapResourceValidator#validate()");
	}

	/**
	 * Validate post request Json object for VBridgePortMaps API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validatePost(final JsonObject requestBody) {
		LOG.trace("Start VBridgePortMapResourceValidator#validatePost()");
		
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.PORTMAP);		
		if (requestBody.has(VtnServiceJsonConsts.PORTMAP)
				&& requestBody.get(VtnServiceJsonConsts.PORTMAP).isJsonObject()) {
			
			final JsonObject portMap = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.PORTMAP);
			
			setInvalidParameter(VtnServiceJsonConsts.PORTMAP_NAME);
			// validation for mandatory key: portmap_name
			if (portMap.has(VtnServiceJsonConsts.PORTMAP_NAME)
					&& portMap.get(VtnServiceJsonConsts.PORTMAP_NAME)
							.isJsonPrimitive()
					&& !portMap.getAsJsonPrimitive(
							VtnServiceJsonConsts.PORTMAP_NAME)
							.getAsString().isEmpty()) {
				isValid = validator.isValidMaxLengthAlphaNum(portMap
						.getAsJsonPrimitive(VtnServiceJsonConsts.PORTMAP_NAME)
						.getAsString(), VtnServiceJsonConsts.LEN_31);
			}
			
			// validation for mandatory key: controller_id
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.CONTROLLERID);
				if (portMap.has(VtnServiceJsonConsts.CONTROLLERID)
						&& portMap.get(VtnServiceJsonConsts.CONTROLLERID)
								.isJsonPrimitive()
						&& !portMap.getAsJsonPrimitive(
								VtnServiceJsonConsts.CONTROLLERID)
								.getAsString().isEmpty()) {
					isValid = validator
							.isValidMaxLengthAlphaNum(
									portMap.getAsJsonPrimitive(
											VtnServiceJsonConsts.CONTROLLERID)
											.getAsString(),
									VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			
			// validation for mandatory key: domain_id
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.DOMAINID);
				if (portMap.has(VtnServiceJsonConsts.DOMAINID)
						&& portMap.get(VtnServiceJsonConsts.DOMAINID)
								.isJsonPrimitive()
						&& !portMap.getAsJsonPrimitive(
								VtnServiceJsonConsts.DOMAINID)
								.getAsString().isEmpty()) {
					isValid = validator
							.isValidDomainId(
									portMap.getAsJsonPrimitive(
											VtnServiceJsonConsts.DOMAINID)
											.getAsString(),
									VtnServiceJsonConsts.LEN_31);
				} else {
					isValid = false;
				}
			}
			
			if (isValid) {
				isValid = paramIsSpecifiedTogether(portMap);
			}
		}
		
		LOG.trace("Complete VBridgePortMapResourceValidator#validatePost()");
		return isValid;
	}

	/**
	 * Validate put request Json object for VBridgePortMap API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validatePut(final JsonObject requestBody) {
		LOG.trace("Start VBridgePortMapResourceValidator#validatePut()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.PORTMAP);	
		if (requestBody.has(VtnServiceJsonConsts.PORTMAP)
				&& requestBody.get(VtnServiceJsonConsts.PORTMAP).isJsonObject()) {
			
			final JsonObject portMap = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.PORTMAP);
			
			isValid = paramIsSpecifiedTogether( portMap);
		}
		
		LOG.trace("Complete VBridgePortMapResourceValidator#validatePut()");
		return isValid;
	}
	
	/**
	 *  Is logical_port_id, label_type and label specified together and have same valid flag
	 * 
	 * @param boolean
	 *            the validate result
	 * @param JsonObject
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean paramIsSpecifiedTogether(JsonObject portMap) {
		LOG.trace("Start VBridgePortMapResourceValidator#paramIsSpecifiedTogether()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.LOGICAL_PORT_ID
				+ VtnServiceJsonConsts.SLASH
				+ VtnServiceJsonConsts.LABEL_TYPE
				+ VtnServiceJsonConsts.SLASH
				+ VtnServiceJsonConsts.LABEL);	
		if (!portMap.has(VtnServiceJsonConsts.LOGICAL_PORT_ID)
				&& !portMap.has(VtnServiceJsonConsts.LABEL_TYPE)
				&& !portMap.has(VtnServiceJsonConsts.LABEL)) {
			
			isValid = true;
		} else if (portMap.has(VtnServiceJsonConsts.LOGICAL_PORT_ID)
				&& portMap.get(VtnServiceJsonConsts.LOGICAL_PORT_ID).isJsonPrimitive()
				&& portMap.has(VtnServiceJsonConsts.LABEL_TYPE)
				&& portMap.get(VtnServiceJsonConsts.LABEL_TYPE).isJsonPrimitive()
				&& portMap.has(VtnServiceJsonConsts.LABEL)
				&& portMap.get(VtnServiceJsonConsts.LABEL).isJsonPrimitive()) {
			
			if (portMap.getAsJsonPrimitive(
							VtnServiceJsonConsts.LOGICAL_PORT_ID).getAsString().isEmpty()
					&& portMap.getAsJsonPrimitive(
							VtnServiceJsonConsts.LABEL_TYPE).getAsString().isEmpty()
					&& portMap.getAsJsonPrimitive(
							VtnServiceJsonConsts.LABEL).getAsString().isEmpty()) {
				isValid = true;
			} else if (!portMap.getAsJsonPrimitive(
							VtnServiceJsonConsts.LOGICAL_PORT_ID).getAsString().isEmpty()
					&& !portMap.getAsJsonPrimitive(
							VtnServiceJsonConsts.LABEL_TYPE).getAsString().isEmpty()
					&& !portMap.getAsJsonPrimitive(
							VtnServiceJsonConsts.LABEL).getAsString().isEmpty()) {
				isValid = validateParameter(portMap);
			}
		}
		
		LOG.trace("Complete VBridgePortMapResourceValidator#paramIsSpecifiedTogether()");
		return isValid;
	}
	
	/**
	 * logical_port_id, label_type and label should have same valid flag
	 * 
	 * @param JsonObject
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validateParameter(JsonObject portMap) {
		LOG.trace("Start VBridgePortMapResourceValidator#validateParameter()");
		boolean isValidPortId = false;
		boolean isValidVlanId = false;
		boolean isValidLabel = false;
		
		// validation for mandatory key: logical_port_id
		setInvalidParameter(VtnServiceJsonConsts.LOGICAL_PORT_ID);	
		isValidPortId = validator.isValidMaxLength(
						portMap.getAsJsonPrimitive(
								VtnServiceJsonConsts.LOGICAL_PORT_ID)
								.getAsString(),
						VtnServiceJsonConsts.LEN_319);
			
					
		// validation for mandatory key: label_type
		if (isValidPortId) {
			setInvalidParameter(VtnServiceJsonConsts.LABEL_TYPE);
			if (VtnServiceJsonConsts.VLAN_ID.equals(
						portMap.getAsJsonPrimitive(
								VtnServiceJsonConsts.LABEL_TYPE).getAsString())) {
				isValidVlanId = true;
			}
		}
		
		// validation for mandatory key: label
		if (isValidVlanId) {
			setInvalidParameter(VtnServiceJsonConsts.LABEL);
			if (VtnServiceJsonConsts.ANY_VLAN_ID.equals(
					portMap.getAsJsonPrimitive(VtnServiceJsonConsts.LABEL).getAsString())) {
				isValidLabel = true;
			} else if (VtnServiceJsonConsts.NO_VLAN_ID.equals(
					portMap.getAsJsonPrimitive(VtnServiceJsonConsts.LABEL).getAsString())) {
				isValidLabel = true;
			} else {
				isValidLabel = validator.isValidRange(
						portMap.getAsJsonPrimitive(VtnServiceJsonConsts.LABEL).getAsString(),
							VtnServiceJsonConsts.VAL_1,
							VtnServiceJsonConsts.VAL_4095);
			}
		}
		
		LOG.trace("Complete VBridgePortMapResourceValidator#validateParameter()");
		return isValidPortId && isValidVlanId && isValidLabel;
	}
}
