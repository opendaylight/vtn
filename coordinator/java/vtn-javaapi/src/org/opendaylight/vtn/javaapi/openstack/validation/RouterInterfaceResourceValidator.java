/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.openstack.validation;

import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpAddressUtil;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.resources.openstack.RouterInterfaceResource;
import org.opendaylight.vtn.javaapi.resources.openstack.RouterInterfacesResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * Router Interface Validation Resource class. Contains methods to validate URI,
 * parameters of POST and PUT request body
 * 
 */
public class RouterInterfaceResourceValidator extends VtnServiceValidator {

	/* Logger instance */
	private static final Logger LOG = Logger
			.getLogger(RouterInterfaceResourceValidator.class.getName());

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
	public RouterInterfaceResourceValidator(final AbstractResource resource) {
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
		LOG.trace("Start RouterInterfaceResourceValidator#validate()");

		boolean isValid = false;
		try {
			if (requestBody != null) {
				isValid = validateUri();
				if (isValid && VtnServiceConsts.POST.equalsIgnoreCase(method)) {
					isValid = validatePost(requestBody);
				} else if (isValid && VtnServiceConsts.PUT.equals(method)) {
					isValid = validatePut(requestBody);
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

		LOG.trace("Complete RouterInterfaceResourceValidator#validate()");
	}

	/**
	 * Validates the parameters of PUT request body
	 * 
	 * @param requestBody
	 *            - JSON request body corresponding to PUT operation
	 * @return - validation status as true or false
	 */
	private boolean validatePut(JsonObject requestBody) {
		boolean isValid = true;
		// validation of ip_address
		if (requestBody.has(VtnServiceOpenStackConsts.IP_ADDRESS)) {
			final JsonElement ipAddress = requestBody
					.get(VtnServiceOpenStackConsts.IP_ADDRESS);

			if (ipAddress.isJsonNull() || ipAddress.getAsString().isEmpty()
					|| !isValidIP(ipAddress.getAsString())) {
				isValid = false;
				setInvalidParameter(VtnServiceOpenStackConsts.IP_ADDRESS
						+ VtnServiceConsts.COLON
						+ (ipAddress.isJsonNull() ? ipAddress : ipAddress
								.getAsString()));
			}
		}

		// validation of mac_address
		if (isValid && requestBody.has(VtnServiceOpenStackConsts.MAC_ADDRESS)) {
			final JsonElement macAddress = requestBody
					.get(VtnServiceOpenStackConsts.MAC_ADDRESS);

			if (macAddress.isJsonNull() || macAddress.getAsString().isEmpty()
					|| !isValidMac(macAddress.getAsString())) {
				isValid = false;
				setInvalidParameter(VtnServiceOpenStackConsts.MAC_ADDRESS
						+ VtnServiceConsts.COLON
						+ (macAddress.isJsonNull() ? macAddress : macAddress
								.getAsString()));
			}
		}
		return isValid;
	}

	/**
	 * Validates the parameters of POST request body
	 * 
	 * @param requestBody
	 *            - JSON request body corresponding to POST operation
	 * @return - validation status as true or false
	 */
	private boolean validatePost(JsonObject requestBody) {

		boolean isValid = true;

		// validation of mandatory parameter
		if (requestBody == null
				|| !requestBody.has(VtnServiceOpenStackConsts.NET_ID)) {
			isValid = false;
			setInvalidParameter(UncCommonEnum.UncResultCode.UNC_INVALID_FORMAT
					.getMessage());
		} else {
			final JsonElement netId = requestBody
					.get(VtnServiceOpenStackConsts.NET_ID);

			// validation of net_id
			if (netId.isJsonNull()
					|| netId.getAsString().isEmpty()
					|| !validator.isValidMaxLengthAlphaNum(netId.getAsString(),
							VtnServiceJsonConsts.LEN_31)) {
				isValid = false;
				setInvalidParameter(VtnServiceOpenStackConsts.NET_ID
						+ VtnServiceConsts.COLON
						+ (netId.isJsonNull() ? netId : netId.getAsString()));
			}

			if (isValid) {
				isValid = validatePut(requestBody);
			}
			/*
			 * Check special case for IP address in POST operation
			 */
			if (isValid
					&& requestBody.has(VtnServiceOpenStackConsts.IP_ADDRESS)) {
				final String ipAddress = requestBody.get(
						VtnServiceOpenStackConsts.IP_ADDRESS).getAsString();
				if (VtnServiceOpenStackConsts.DEFAULT_IP.equals(ipAddress
						.split(VtnServiceConsts.SLASH)[0])) {
					isValid = false;
					setInvalidParameter(VtnServiceOpenStackConsts.IP_ADDRESS
							+ VtnServiceConsts.COLON + ipAddress);
				}
			}

			/*
			 * Check special case for MAC address in POST operation
			 */
			if (isValid
					&& requestBody.has(VtnServiceOpenStackConsts.MAC_ADDRESS)) {
				final String macAddress = requestBody.get(
						VtnServiceOpenStackConsts.MAC_ADDRESS).getAsString();
				if (VtnServiceOpenStackConsts.DEFAULT_MAC.equals(macAddress)) {
					isValid = false;
					setInvalidParameter(VtnServiceOpenStackConsts.MAC_ADDRESS
							+ VtnServiceConsts.COLON + macAddress);
				}
			}
		}
		return isValid;
	}

	/**
	 * Validate resource specific URI parameters
	 * 
	 * @see org.opendaylight.vtn.javaapi.validation.VtnServiceValidator#validateUri
	 *      ()
	 */
	@Override
	public boolean validateUri() throws VtnServiceException {
		LOG.info("Start RouterInterfaceResourceValidator#validateUri()");
		boolean isValid = true;
		if (resource instanceof RouterInterfaceResource) {
			// validation of tenant_id
			final String tenantId = ((RouterInterfaceResource) resource)
					.getTenantId();
			if (tenantId == null
					|| tenantId.isEmpty()
					|| !validator.isValidMaxLengthAlphaNum(tenantId,
							VtnServiceJsonConsts.LEN_31)) {
				isValid = false;
				setInvalidParameter(VtnServiceOpenStackConsts.TENANT_ID
						+ VtnServiceConsts.COLON + tenantId);
			}

			// validation of router_id
			if (isValid) {
				final String routerId = ((RouterInterfaceResource) resource)
						.getRouterId();
				if (routerId == null
						|| routerId.isEmpty()
						|| !validator.isValidMaxLengthAlphaNum(routerId,
								VtnServiceJsonConsts.LEN_31)) {
					isValid = false;
					setInvalidParameter(VtnServiceOpenStackConsts.ROUTER_ID
							+ VtnServiceConsts.COLON + routerId);
				}
			}

			// validation of if_id
			if (isValid) {
				final String ifId = ((RouterInterfaceResource) resource)
						.getInterfaceId();
				if (ifId == null || ifId.isEmpty()) {
					isValid = false;
				} else {
					try {
						isValid = validator.isValidRange(ifId, 1, 393216);
					} catch (Exception e) {
						isValid = false;
					}
				}
				// set message as per above checks
				if (!isValid) {
					setInvalidParameter(VtnServiceOpenStackConsts.IF_ID
							+ VtnServiceConsts.COLON + ifId);
				}
			}
		} else if (resource instanceof RouterInterfacesResource) {
			// validation of tenant_id
			final String tenantId = ((RouterInterfacesResource) resource)
					.getTenantId();
			if (tenantId == null
					|| tenantId.isEmpty()
					|| !validator.isValidMaxLengthAlphaNum(tenantId,
							VtnServiceJsonConsts.LEN_31)) {
				isValid = false;
				setInvalidParameter(VtnServiceOpenStackConsts.TENANT_ID
						+ VtnServiceConsts.COLON + tenantId);
			}

			// validation of router_id
			if (isValid) {
				final String routerId = ((RouterInterfacesResource) resource)
						.getRouterId();
				if (routerId == null
						|| routerId.isEmpty()
						|| !validator.isValidMaxLengthAlphaNum(routerId,
								VtnServiceJsonConsts.LEN_31)) {
					isValid = false;
					setInvalidParameter(VtnServiceOpenStackConsts.ROUTER_ID
							+ VtnServiceConsts.COLON + routerId);
				}
			}
		}
		LOG.info("Complete RouterInterfaceResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Check the validity of IP address in CIDR notation
	 * 
	 * @param cidrDestination
	 *            - CIDR format IP address string
	 * @return - result as true or false
	 */
	private boolean isValidIP(String cidrDestination) {
		boolean isValid = false;
		if (VtnServiceOpenStackConsts.DEFAULT_CIDR_IP.equals(cidrDestination)) {
			isValid = true;
		} else {
			setInvalidParameter(VtnServiceOpenStackConsts.IP_ADDRESS);
			final String[] destination = cidrDestination
					.split(VtnServiceConsts.SLASH);
			if (destination != null && destination.length == 2) {
				try {
					isValid = IpAddressUtil
							.textToNumericFormatV4(destination[0]) != null
							&& validator.isValidRange(destination[1], 1, 30);
				} catch (Exception e) {
					isValid = false;
				}
			}
		}
		return isValid;
	}

	/**
	 * Check the validity of OpenStack format MAC address
	 * 
	 * @param macAddress
	 *            - OpenStack format MAC address
	 * @return - result as true or false
	 */
	private boolean isValidMac(String macAddress) {
		return macAddress.matches(VtnServiceOpenStackConsts.OS_MAC_ADD_REGEX);
	}
}
