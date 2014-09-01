/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.openstack.validation;

import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.resources.openstack.FilterResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * Filter Validation Resource class. Contains methods to validate URI,
 * parameters of POST or PUT request body
 * 
 */
public class FilterResourceValidator extends VtnServiceValidator {

	/* Logger instance */
	private static final Logger LOG = Logger
			.getLogger(FilterResourceValidator.class.getName());

	/* AbstractResource reference pointing to actual Resource class */
	private final AbstractResource resource;

	/* Instance for common validation operation */
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Constructor that provide reference of actual resource class to instance
	 * variable resource
	 * 
	 * @param resource
	 *            - Resource class reference
	 */
	public FilterResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/*
	 * Calls the respective validation method according to type of method
	 * 
	 * @see
	 * org.opendaylight.vtn.javaapi.validation.VtnServiceValidator#validate
	 * (java.lang.String, com.google.gson.JsonObject)
	 */
	@Override
	public void validate(String method, JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start FilterResourceValidator#validate()");

		boolean isValid = false;
		try {
			if (null != requestBody) {
				isValid = validateUri();
				if (isValid) {
					if (method.equalsIgnoreCase(VtnServiceConsts.POST)) {
						isValid = validatePost(requestBody);
					} else if (method.equalsIgnoreCase(VtnServiceConsts.PUT)) {
						isValid = validatePut(requestBody);
					} else if (method.equalsIgnoreCase(VtnServiceConsts.GET)) {
						isValid = true;
					} else {
						isValid = false;
						setInvalidParameter(UncCommonEnum.UncResultCode.UNC_METHOD_NOT_ALLOWED
								.getMessage());
					}
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

		LOG.trace("Complete FilterResourceValidator#validate()");
	}

	/**
	 * Validate resource specific URI parameters
	 * 
	 * @see org.opendaylight.vtn.javaapi.validation.VtnServiceValidator#validateUri
	 *      ()
	 */
	@Override
	public boolean validateUri() throws VtnServiceException {
		LOG.trace("Start FilterResourceValidator#validateUri()");

		boolean isValid = true;
		if (resource instanceof FilterResource) {
			String filterId = ((FilterResource) resource).getFilterId();
			if (null == filterId || filterId.isEmpty()
					|| !isValidFilterId(filterId)) {
				setInvalidParameter(VtnServiceOpenStackConsts.FILTER_ID
						+ VtnServiceConsts.COLON + filterId);
				isValid = false;
			}
		}

		LOG.trace("Complete FilterResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validates the parameters of POST request body.
	 * 
	 * @param requestBody
	 *            - JSON request body corresponding to POST operation
	 * @return
	 */
	private boolean validatePost(final JsonObject requestBody) {
		LOG.trace("Start FilterResourceValidator#validatePost()");

		boolean isValid = false;
		if (null == requestBody) {
			setInvalidParameter(UncCommonEnum.UncResultCode.UNC_INVALID_FORMAT
					.getMessage());
		} else if (!requestBody.has(VtnServiceOpenStackConsts.ACTION)) {
			setInvalidParameter("no action");
		} else if (!requestBody.has(VtnServiceOpenStackConsts.PRIORITY)) {
			setInvalidParameter("no priority");
		} else {
			/* Check id. */
			String idValue = "";
			if (requestBody.has(VtnServiceOpenStackConsts.ID)) {
				JsonElement id = requestBody.get(VtnServiceOpenStackConsts.ID);
				if (id.isJsonNull() || id.getAsString().isEmpty()
						|| !isValidFilterId(id.getAsString())) {
					setInvalidParameter(VtnServiceOpenStackConsts.ID
							+ VtnServiceConsts.COLON
							+ (id.isJsonNull() ? id : id.getAsString()));
					return isValid;
				} else {
					idValue = id.getAsString();
				}
			}

			/* Check action. */
			JsonElement action = requestBody
					.get(VtnServiceOpenStackConsts.ACTION);
			if (action.isJsonNull()
					|| action.getAsString().isEmpty()
					|| (!action.getAsString().equals(VtnServiceJsonConsts.PASS) && !action
							.getAsString().equals(VtnServiceJsonConsts.DROP))) {
				setInvalidParameter(VtnServiceOpenStackConsts.ACTION
						+ VtnServiceConsts.COLON
						+ (action.isJsonNull() ? action : action.getAsString()));
				return isValid;
			}
			if (!idValue.isEmpty()
					&& idValue.charAt(4) != action.getAsString().charAt(0)) {
				/* "id:" + idValue + "," + "action:" + action.getAsString() */
				setInvalidParameter(VtnServiceOpenStackConsts.ID
						+ VtnServiceConsts.COLON + idValue
						+ VtnServiceConsts.COMMA
						+ VtnServiceOpenStackConsts.ACTION
						+ VtnServiceConsts.COLON + action.getAsString());
				return isValid;
			}

			/* Check priority. */
			JsonElement priority = requestBody
					.get(VtnServiceOpenStackConsts.PRIORITY);
			if (priority.isJsonNull() || priority.getAsString().isEmpty()) {
				isValid = false;
			} else {
				try {
					isValid = validator.isValidRange(priority.getAsString(),
							VtnServiceJsonConsts.VAL_1,
							VtnServiceJsonConsts.VAL_32766);
				} catch (Exception e) {
					isValid = false;
				}
			}
			if (!isValid) {
				setInvalidParameter(VtnServiceOpenStackConsts.PRIORITY
						+ VtnServiceConsts.COLON
						+ (priority.isJsonNull() ? priority : priority
								.getAsString()));
			} else if (!idValue.isEmpty()) {
				int val1 = Integer.parseInt(idValue.substring(5, 9), 16);
				int val2 = Integer.parseInt(priority.getAsString());
				if (val1 != val2) {
					/*
					 * "id:" + idValue + "," + "priority:" +
					 * priority.getAsString()
					 */
					setInvalidParameter(VtnServiceOpenStackConsts.ID
							+ VtnServiceConsts.COLON + idValue
							+ VtnServiceConsts.COMMA
							+ VtnServiceOpenStackConsts.PRIORITY
							+ VtnServiceConsts.COLON + priority.getAsString());
					return false;
				}
			}
		}

		if (isValid) {
			isValid = validatePut(requestBody);
		}
		LOG.trace("Complete FilterResourceValidator#validatePost()");
		return isValid;
	}

	/**
	 * Validates the parameters of PUT request body.
	 * 
	 * @param requestBody
	 *            - JSON request body corresponding to PUT operation
	 * @return - validation status as true or false
	 */
	private boolean validatePut(final JsonObject requestBody) {
		LOG.trace("Start FilterResourceValidator#validatePut()");
		boolean isValid = false;

		/* Check src_mac. */
		if (requestBody.has(VtnServiceOpenStackConsts.SRC_MAC)) {
			JsonElement srcMac = requestBody
					.get(VtnServiceOpenStackConsts.SRC_MAC);
			if (srcMac.isJsonNull()
					|| (!srcMac.getAsString().isEmpty()
					&& !srcMac.getAsString().matches(
							VtnServiceOpenStackConsts.OS_MAC_ADD_REGEX))) {
				setInvalidParameter(VtnServiceOpenStackConsts.SRC_MAC
						+ VtnServiceConsts.COLON
						+ (srcMac.isJsonNull() ? srcMac : srcMac.getAsString()));
				return isValid;
			}
		}

		/* Check dst_mac. */
		if (requestBody.has(VtnServiceOpenStackConsts.DST_MAC)) {
			JsonElement dstMac = requestBody
					.get(VtnServiceOpenStackConsts.DST_MAC);
			if (dstMac.isJsonNull()
					|| (!dstMac.getAsString().isEmpty()
					&& !dstMac.getAsString().matches(
							VtnServiceOpenStackConsts.OS_MAC_ADD_REGEX))) {
				setInvalidParameter(VtnServiceOpenStackConsts.DST_MAC
						+ VtnServiceConsts.COLON
						+ (dstMac.isJsonNull() ? dstMac : dstMac.getAsString()));
				return isValid;
			}
		}

		/* Check eth_type. */
		if (requestBody.has(VtnServiceOpenStackConsts.ETH_TYPE)) {
			JsonElement ethType = requestBody
					.get(VtnServiceOpenStackConsts.ETH_TYPE);
			if (ethType.isJsonNull()) {
				isValid = false;
			} else if (ethType.getAsString().isEmpty()) {
				isValid = true;
			} else {
				try {
					isValid = ethType.getAsString().matches(
							VtnServiceConsts.ETH_TYPE_REGEX);
				} catch (Exception e) {
					isValid = false;
				}
			}

			if (!isValid) {
				setInvalidParameter(VtnServiceOpenStackConsts.ETH_TYPE
						+ VtnServiceConsts.COLON
						+ (ethType.isJsonNull() ? ethType : ethType
								.getAsString()));
				return isValid;
			}
		}

		isValid = false;
		/* Check src_cidr. */
		if (requestBody.has(VtnServiceOpenStackConsts.SRC_CIDR)) {
			JsonElement srcCidr = requestBody
					.get(VtnServiceOpenStackConsts.SRC_CIDR);
			if (srcCidr.isJsonNull() || (!srcCidr.getAsString().isEmpty()
					&& !isValidIp(srcCidr.getAsString()))) {
				setInvalidParameter(VtnServiceOpenStackConsts.SRC_CIDR
						+ VtnServiceConsts.COLON
						+ (srcCidr.isJsonNull() ? srcCidr : srcCidr
								.getAsString()));
				return isValid;
			}
		}

		/* Check dst_cidr. */
		if (requestBody.has(VtnServiceOpenStackConsts.DST_CIDR)) {
			JsonElement dstCidr = requestBody
					.get(VtnServiceOpenStackConsts.DST_CIDR);
			if (dstCidr.isJsonNull() || (!dstCidr.getAsString().isEmpty()
					&& !isValidIp(dstCidr.getAsString()))) {
				setInvalidParameter(VtnServiceOpenStackConsts.DST_CIDR
						+ VtnServiceConsts.COLON
						+ (dstCidr.isJsonNull() ? dstCidr : dstCidr
								.getAsString()));
				return isValid;
			}
		}

		/* Check protocol. */
		int protocolVal = 0;
		if (requestBody.has(VtnServiceOpenStackConsts.PROTOCOL)) {
			JsonElement protocol = requestBody
					.get(VtnServiceOpenStackConsts.PROTOCOL);
			if (protocol.isJsonNull()) {
				isValid = false;
			} else if (protocol.getAsString().isEmpty()) {
				isValid = true;
			} else {
				try {
					isValid = validator.isValidRange(protocol.getAsString(),
							VtnServiceJsonConsts.VAL_1,
							VtnServiceJsonConsts.VAL_255);
				} catch (Exception e) {
					isValid = false;
				}
			}
			if (!isValid) {
				setInvalidParameter(VtnServiceOpenStackConsts.PROTOCOL
						+ VtnServiceConsts.COLON
						+ (protocol.isJsonNull() ? protocol : protocol
								.getAsString()));
				return isValid;
			} else if (!protocol.getAsString().isEmpty()) {
				protocolVal = Integer.parseInt(protocol.getAsString());
			}
		}

		isValid = false;
		/* Check src_port. */
		if (requestBody.has(VtnServiceOpenStackConsts.SRC_PORT)) {
			JsonElement srcPort = requestBody
					.get(VtnServiceOpenStackConsts.SRC_PORT);
			if (srcPort.isJsonNull() || (!srcPort.getAsString().isEmpty()
					&& !isValidPort(protocolVal, srcPort.getAsString()))) {
				setInvalidParameter(VtnServiceOpenStackConsts.SRC_PORT
						+ VtnServiceConsts.COLON
						+ (srcPort.isJsonNull() ? srcPort : srcPort
								.getAsString()));
				return isValid;
			}
		}

		/* Check dst_port. */
		if (requestBody.has(VtnServiceOpenStackConsts.DST_PORT)) {
			JsonElement dstPort = requestBody
					.get(VtnServiceOpenStackConsts.DST_PORT);
			if (dstPort.isJsonNull() || (!dstPort.getAsString().isEmpty()
					&& !isValidPort(protocolVal, dstPort.getAsString()))) {
				setInvalidParameter(VtnServiceOpenStackConsts.DST_PORT
						+ VtnServiceConsts.COLON
						+ (dstPort.isJsonNull() ? dstPort : dstPort
								.getAsString()));
				return isValid;
			}
		}

		isValid = true;
		/* Check apply_ports. */
		if (requestBody.has(VtnServiceOpenStackConsts.APPLY_PORTS)) {
			JsonElement applyPorts = requestBody
					.get(VtnServiceOpenStackConsts.APPLY_PORTS);
			isValid = isValidApplyPort(applyPorts);
		}

		LOG.trace("Complete FilterResourceValidator#validatePut()");
		return isValid;
	}

	/**
	 * Check the validity of filter id notation.
	 * 
	 * @param filterId
	 *            - filter id format string, the format is "os_fXHHHH_", detail:
	 *            X: 'p' or 'd', HHHH: Hexadecimal number
	 * @return - result as true or false
	 */
	private boolean isValidFilterId(String filterId) {
		boolean isValid = false;
		String subString;

		if (filterId.length() < 11
				|| !validator.isValidMaxLengthAlphaNum(filterId,
						VtnServiceJsonConsts.LEN_32)) {
			return isValid;
		}

		/* Check "os_f". */
		subString = filterId.substring(0, 4);
		if (!subString.equals(VtnServiceOpenStackConsts.FL_PREFIX)) {
			return isValid;
		}

		/* Check "X". */
		if (filterId.charAt(4) != 'p' && filterId.charAt(4) != 'd') {
			return isValid;
		}

		/* Check "HHHH". */
		subString = filterId.substring(5, 9);
		try {
			int value = Integer.parseInt(subString, 16);
			if (value >= VtnServiceJsonConsts.VAL_1
					&& value <= VtnServiceJsonConsts.VAL_32766) {
				isValid = true;
			}
		} catch (Exception e) {
			isValid = false;
		}
		if (!isValid) {
			return isValid;
		}

		/* Check "_". */
		if (filterId.charAt(9) != '_') {
			return false;
		}

		return isValid;
	}

	/**
	 * Check the validity of IP address notation.
	 * 
	 * @param ip
	 *            - ip format string
	 * @return - result as true or false
	 */
	private boolean isValidIp(String ip) {
		boolean isValid = false;
		String[] splitString = ip.split(VtnServiceConsts.SLASH);

		if (splitString != null && splitString.length == 2) {
			/* Check ip. */
			if (!splitString[0].matches(VtnServiceConsts.IPV4_ADD_REGEX)) {
				return isValid;
			}

			/* Check prefix. */
			try {
				isValid = validator
						.isValidRange(splitString[1],
								VtnServiceJsonConsts.VAL_1,
								VtnServiceJsonConsts.VAL_32);
			} catch (Exception e) {
				return isValid;
			}
		}

		return isValid;
	}

	/**
	 * Check the validity of port notation.
	 * 
	 * @param protocol
	 *            - protocol value
	 * @param port
	 *            - port value
	 * @return - result as true or false
	 */
	private boolean isValidPort(int protocol, String port) {
		boolean isValid = false;
		int min = VtnServiceJsonConsts.VAL_0;
		int max;

		/* When protocol is ICMP. */
		if (1 == protocol) {
			max = VtnServiceJsonConsts.VAL_255;
		} else { /* Other case. */
			max = VtnServiceJsonConsts.VAL_65535;
		}

		try {
			isValid = validator.isValidRange(port, min, max);
		} catch (Exception e) {
			return isValid;
		}

		return isValid;
	}

	/**
	 * Check the validity of applyPort.
	 * 
	 * @param protocol
	 *            - protocol value
	 * @param port
	 *            - port value
	 * @return - result as true or false
	 */
	private boolean isValidApplyPort(JsonElement applyPort) {
		boolean isValid = false;

		if (applyPort.isJsonNull() || !applyPort.isJsonArray()) {
			setInvalidParameter(UncCommonEnum.UncResultCode.UNC_INVALID_FORMAT
					.getMessage());
			return isValid;
		}

		JsonArray portArray = applyPort.getAsJsonArray();

		/* "apply_ports":[""] case, Invalid Format */
		/* "apply_ports":[{""}] case, Invalid Format */
		/* "apply_ports":[{}] case, Invalid Format */
		if (portArray.toString().equals("[\"\"]")
				|| portArray.toString().equals("[{\"\"}]")
				|| portArray.toString().equals("[{}]")) {
			setInvalidParameter(UncCommonEnum.UncResultCode.UNC_INVALID_FORMAT
					.getMessage());
			return isValid;
		}

		/* When "apply_ports":[] case. */
		if (portArray.size() == 0) {
			return true;
		}

		for (int i = 0; i < portArray.size(); i++) {
			JsonObject info = portArray.get(i).getAsJsonObject();

			if (!(info.has(VtnServiceOpenStackConsts.TENANT) && ((info
					.has(VtnServiceOpenStackConsts.NETWORK) && info
					.has(VtnServiceOpenStackConsts.PORT)) || (info
					.has(VtnServiceOpenStackConsts.ROUTER) && info
					.has(VtnServiceOpenStackConsts.INTERFACE))))) {
				setInvalidParameter(createPortErrorMsg(info));
				return isValid;
			}

			/* Check tenant id. */
			JsonElement tenantId = info.get(VtnServiceOpenStackConsts.TENANT);
			if (tenantId.isJsonNull()
					|| tenantId.getAsString().isEmpty()
					|| !validator
							.isValidMaxLengthAlphaNum(tenantId.getAsString(),
									VtnServiceJsonConsts.LEN_31)) {
				setInvalidParameter(createPortErrorMsg(info));
				return isValid;
			}

			/* Check network id and port id. */
			if (info.has(VtnServiceOpenStackConsts.NETWORK)) {
				JsonElement netId = info.get(VtnServiceOpenStackConsts.NETWORK);
				if (netId.isJsonNull()
						|| netId.getAsString().isEmpty()
						|| !validator.isValidMaxLengthAlphaNum(
								netId.getAsString(),
								VtnServiceJsonConsts.LEN_31)) {
					setInvalidParameter(createPortErrorMsg(info));
					return isValid;
				}

				JsonElement portId = info.get(VtnServiceOpenStackConsts.PORT);
				if (portId.isJsonNull()
						|| portId.getAsString().isEmpty()
						|| !validator.isValidMaxLengthAlphaNum(
								portId.getAsString(),
								VtnServiceJsonConsts.LEN_24)) {
					setInvalidParameter(createPortErrorMsg(info));
					return isValid;
				}
			}

			/* Check router id and interface id. */
			if (info.has(VtnServiceOpenStackConsts.ROUTER)) {
				JsonElement routerId = info
						.get(VtnServiceOpenStackConsts.ROUTER);
				if (routerId.isJsonNull()
						|| routerId.getAsString().isEmpty()
						|| !validator.isValidMaxLengthAlphaNum(
								routerId.getAsString(),
								VtnServiceJsonConsts.LEN_31)) {
					setInvalidParameter(createPortErrorMsg(info));
					return isValid;
				}

				JsonElement ifId = info
						.get(VtnServiceOpenStackConsts.INTERFACE);
				if (ifId.isJsonNull()
						|| ifId.getAsString().isEmpty()
						|| !validator
								.isValidMaxLengthAlphaNum(ifId.getAsString(),
										VtnServiceJsonConsts.LEN_24)) {
					setInvalidParameter(createPortErrorMsg(info));
					return isValid;
				}
			}
		}

		return true;
	}

	/**
	 * When port information is invalid, create error massage.
	 * 
	 * @param info
	 *            - Port information reference
	 * @return - result as true or false
	 */
	private String createPortErrorMsg(final JsonObject info) {
		StringBuffer msg = new StringBuffer();

		/* Format1: apply_ports:{tenant:os_vtn_1,network:os_vbr_#2,port:1} */
		/* Format2: apply_ports:{tenant:os_vtn_1,router:os_vrt_#,interface:2} */

		/* Append "apply_ports:{tenant:" */
		msg.append(VtnServiceOpenStackConsts.APPLY_PORTS);
		msg.append(VtnServiceConsts.COLON);
		msg.append(VtnServiceConsts.OPEN_CURLY_BRACES);
		msg.append(VtnServiceOpenStackConsts.TENANT);
		msg.append(VtnServiceConsts.COLON);

		if (info.has(VtnServiceOpenStackConsts.TENANT)) {
			JsonElement tenantId = info.get(VtnServiceOpenStackConsts.TENANT);
			if (!(tenantId.isJsonNull() || tenantId.getAsString().isEmpty())) {
				/* Append "os_vtn_1" */
				msg.append(tenantId.getAsString());
			}
		}

		/* Append "," */
		msg.append(VtnServiceConsts.COMMA);

		if (info.has(VtnServiceOpenStackConsts.ROUTER)
				|| info.has(VtnServiceOpenStackConsts.INTERFACE)) {
			/* Append "router:" */
			msg.append(VtnServiceOpenStackConsts.ROUTER);
			msg.append(VtnServiceConsts.COLON);

			if (info.has(VtnServiceOpenStackConsts.ROUTER)) {
				JsonElement routerId = info
						.get(VtnServiceOpenStackConsts.ROUTER);
				if (!(routerId.isJsonNull() || routerId.getAsString().isEmpty())) {
					/* Append "os_vrt_#" */
					msg.append(routerId.getAsString());
				}
			}

			/* Append ",interface:" */
			msg.append(VtnServiceConsts.COMMA);
			msg.append(VtnServiceOpenStackConsts.INTERFACE);
			msg.append(VtnServiceConsts.COLON);

			if (info.has(VtnServiceOpenStackConsts.INTERFACE)) {
				JsonElement ifId = info
						.get(VtnServiceOpenStackConsts.INTERFACE);
				if (!(ifId.isJsonNull() || ifId.getAsString().isEmpty())) {
					/* Append "2" */
					msg.append(ifId.getAsString());
				}
			}
		} else {
			/* Append "network:" */
			msg.append(VtnServiceOpenStackConsts.NETWORK);
			msg.append(VtnServiceConsts.COLON);

			if (info.has(VtnServiceOpenStackConsts.NETWORK)) {
				JsonElement netId = info.get(VtnServiceOpenStackConsts.NETWORK);
				if (!(netId.isJsonNull() || netId.getAsString().isEmpty())) {
					/* Append "os_vbr_#2" */
					msg.append(netId.getAsString());
				}
			}

			/* Append ",port:" */
			msg.append(VtnServiceConsts.COMMA);
			msg.append(VtnServiceOpenStackConsts.PORT);
			msg.append(VtnServiceConsts.COLON);

			if (info.has(VtnServiceOpenStackConsts.PORT)) {
				JsonElement portId = info.get(VtnServiceOpenStackConsts.PORT);
				if (!(portId.isJsonNull() || portId.getAsString().isEmpty())) {
					/* Append "1" */
					msg.append(portId.getAsString());
				}
			}
		}

		/* Append "}" */
		msg.append(VtnServiceConsts.CLOSE_CURLY_BRACES);

		return msg.toString();
	}
}
