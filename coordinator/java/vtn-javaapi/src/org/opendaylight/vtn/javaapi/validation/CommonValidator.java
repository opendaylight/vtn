/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.validation;

import java.math.BigInteger;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.init.VtnServiceConfiguration;
import org.opendaylight.vtn.javaapi.init.VtnServiceInitManager;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpAddressUtil;

/**
 * The Class CommonValidator validates request Json object.
 */
public class CommonValidator {

	private static final Logger LOG = Logger.getLogger(CommonValidator.class
			.getName());

	private String invalidParameter = null;

	public String getInvalidParameter() {
		return invalidParameter;
	}

	public void setInvalidParameter(final String invalidParameter) {
		this.invalidParameter = invalidParameter;
	}

	/**
	 * Checks if is valid get request.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if is valid get
	 */
	public boolean isValidGet(final JsonObject requestBody, final boolean opFlag) {
		LOG.trace("Start CommonValidator#isValidGet");
		boolean isValid = true;

		// validation for key: targetdb
		invalidParameter = VtnServiceJsonConsts.TARGETDB;
		isValid = isValidRequestDB(requestBody);

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
				invalidParameter = VtnServiceJsonConsts.OP;
				isValid = isValidOperation(requestBody);
			}
			// validation for key: index
			if (isValid) {
				invalidParameter = VtnServiceJsonConsts.INDEX;
				if (requestBody.has(VtnServiceJsonConsts.INDEX)
						&& requestBody.getAsJsonPrimitive(
								VtnServiceJsonConsts.INDEX).getAsString() != null
								&& !requestBody
								.getAsJsonPrimitive(VtnServiceJsonConsts.INDEX)
								.getAsString().isEmpty()) {
					isValid = isValidMaxLengthAlphaNum(requestBody
							.getAsJsonPrimitive(VtnServiceJsonConsts.INDEX)
							.getAsString().trim(), VtnServiceJsonConsts.LEN_31);
				}
			}
			// validation for key: max_repitition
			if (isValid) {
				invalidParameter = VtnServiceJsonConsts.MAX;
				isValid = isValidMaxRepetition(requestBody);
			}
		}
		LOG.trace("Complete CommonValidator#isValidGet");
		return isValid;
	}

	/**
	 * Checks if is valid get for integer index.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if is valid get for int index
	 */
	public boolean isValidGetForIntIndex(final JsonObject requestBody,
			final boolean opFlag) {
		LOG.trace("Start CommonValidator#isValidGetForIntIndex");
		boolean isValid = true;
		// validation for key: tagetdb
		invalidParameter = VtnServiceJsonConsts.TARGETDB;
		isValid = isValidRequestDB(requestBody);

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
				invalidParameter = VtnServiceJsonConsts.OP;
				isValid = isValidOperation(requestBody);
			}
			// validation for key: index
			if (isValid
					&& requestBody.has(VtnServiceJsonConsts.INDEX)
					&& requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.INDEX).getAsString() != null) {
				invalidParameter = VtnServiceJsonConsts.INDEX;
				isValid = isValidRange(
						requestBody
								.getAsJsonPrimitive(VtnServiceJsonConsts.INDEX)
								.getAsString().trim(),
								VtnServiceJsonConsts.VAL_1,
								VtnServiceJsonConsts.VAL_65535);

			}
			// validation for key: max_repitition
			if (isValid) {
				invalidParameter = VtnServiceJsonConsts.MAX;
				isValid = isValidMaxRepetition(requestBody);
			}
		}

		LOG.trace("Complete CommonValidator#isValidGetForIntIndex");
		return isValid;
	}

	/**
	 * Checks if is valid tagetdb db.
	 * 
	 * @param targetdb
	 *            the value of targetdb in the request Json object
	 * @return true, if is valid request db
	 */
	public boolean isValidRequestDB(final JsonObject requestBody) {
		LOG.trace("Start CommonValidator#isValidRequestDB");
		boolean isValid = true;
		if (requestBody.has(VtnServiceJsonConsts.TARGETDB)
				&& requestBody
				.getAsJsonPrimitive(VtnServiceJsonConsts.TARGETDB)
				.getAsString() != null
				&& !requestBody
				.getAsJsonPrimitive(VtnServiceJsonConsts.TARGETDB)
				.getAsString().trim().isEmpty()) {
			final String targetdb = requestBody
					.getAsJsonPrimitive(VtnServiceJsonConsts.TARGETDB)
					.getAsString().trim();
			isValid = targetdb.equalsIgnoreCase(VtnServiceJsonConsts.CANDIDATE)
					|| targetdb.equalsIgnoreCase(VtnServiceJsonConsts.RUNNING)
					|| targetdb.equalsIgnoreCase(VtnServiceJsonConsts.STATE)
					|| targetdb.equalsIgnoreCase(VtnServiceJsonConsts.STARTUP);
		} else {
			requestBody.remove(VtnServiceJsonConsts.TARGETDB);
			requestBody.addProperty(VtnServiceJsonConsts.TARGETDB,
					VtnServiceJsonConsts.STATE);
		}
		LOG.trace("Complete CommonValidator#isValidRequestDB");
		return isValid;
	}

	/**
	 * Checks if is operation is count or detail.
	 * 
	 * @param operation
	 *            the value of operation in the request Json object
	 * @return true, if is valid operation
	 */
	public boolean isValidOperation(final JsonObject requestBody) {
		LOG.trace("Start CommonValidator#isValidOperation");
		boolean isValid = true;
		if (requestBody.has(VtnServiceJsonConsts.OP)
				&& requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.OP)
				.getAsString() != null
				&& !requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.OP)
				.getAsString().trim().isEmpty()) {
			final String operation = requestBody
					.getAsJsonPrimitive(VtnServiceJsonConsts.OP).getAsString()
					.trim();
			isValid = operation.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)
					|| operation.equalsIgnoreCase(VtnServiceJsonConsts.COUNT);
		} else {
			requestBody.remove(VtnServiceJsonConsts.OP);
			requestBody.addProperty(VtnServiceJsonConsts.OP,
					VtnServiceJsonConsts.NORMAL);
		}
		LOG.trace("Complete CommonValidator#isValidOperation");
		return isValid;
	}

	/**
	 * Checks if is operation is count.
	 * 
	 * @param operation
	 *            the value of operation in the request Json object
	 * @return true, if is valid operation
	 */
	public boolean isValidOperationForCount(final JsonObject requestBody) {
		LOG.trace("Start CommonValidator#isValidOperationForCount");
		boolean isValid = true;
		if (requestBody.has(VtnServiceJsonConsts.OP)
				&& requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.OP)
				.getAsString() != null
				&& !requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.OP)
				.getAsString().trim().isEmpty()) {
			isValid = requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.OP)
					.getAsString().trim()
					.equalsIgnoreCase(VtnServiceJsonConsts.COUNT);
		} else {
			requestBody.remove(VtnServiceJsonConsts.OP);
			requestBody.addProperty(VtnServiceJsonConsts.OP,
					VtnServiceJsonConsts.NORMAL);
		}
		LOG.trace("Complete CommonValidator#isValidOperationForCount");
		return isValid;
	}

	/**
	 * Checks if is valid max repetition count.
	 * 
	 * @param operation
	 *            the value of max repetition count in the request Json object
	 * @return true, if is valid operation
	 */
	public boolean isValidMaxRepetition(final JsonObject requestBody) {
		LOG.trace("Start CommonValidator#isValidMaxRepetition");
		boolean isValid = true;
		if (requestBody.has(VtnServiceJsonConsts.MAX)
				&& requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.MAX)
				.getAsString() != null
				&& !requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.MAX)
				.getAsString().trim().isEmpty()) {
			final String max = requestBody
					.getAsJsonPrimitive(VtnServiceJsonConsts.MAX).getAsString()
					.trim();
			isValid = isValidRange(max, VtnServiceJsonConsts.LONG_VAL_1,
					VtnServiceJsonConsts.LONG_VAL_4294967295);
		} else {
			VtnServiceConfiguration configuration = VtnServiceInitManager
					.getConfigurationMap();
			requestBody.remove(VtnServiceJsonConsts.MAX);
			requestBody.addProperty(VtnServiceJsonConsts.MAX, configuration
					.getConfigValue(VtnServiceConsts.MAX_REP_DEFAULT));
		}
		LOG.trace("Complete CommonValidator#isValidMaxRepetition");
		return isValid;
	}

	/**
	 * Checks if is valid range.
	 * 
	 * @param input
	 *            the value to be validated
	 * @param min
	 *            the minimum value possible
	 * @param max
	 *            the maximum value possible
	 * @return true, if is valid range
	 */
	public boolean isValidRange(final String input, final int min, final int max) {
		LOG.trace("Inside CommonValidator#isValidRange (int)");
		if (!(input.equals(VtnServiceConsts.EMPTY_STRING))) {
			int inputResult = Integer.parseInt(input);
			return inputResult >= min && inputResult <= max;
		} else {
			return true;
		}
	}

	/**
	 * checks if range is Valid for alarm no
	 * 
	 * @param input
	 *            the input
	 * 
	 * @return true, if is valid alarm range
	 */
	public boolean isValidAlarmRange(final BigInteger input,
			final BigInteger min, final BigInteger max) {
		LOG.trace("Inside CommonValidator#isValidRange (BigInteger)");
		if (input.compareTo(min) == -1 || input.compareTo(max) == 1) {
			return false;
		}
		return true;
	}

	/**
	 * checks if range is Valid for long type
	 * 
	 * @param input
	 *            the value to be validated
	 * 
	 * @param min
	 *            the minimum value possible
	 * 
	 * @param max
	 *            the maximum value possible
	 * 
	 * @return true,if it is valid range
	 */

	public boolean isValidRange(final String input, final Long min, final Long max) {
		LOG.trace("Inside CommonValidator#isValidRange (Long)");
		if (!(input.equals(VtnServiceConsts.EMPTY_STRING))) {
			long inputResult = Long.parseLong(input);
			return inputResult >= min && inputResult <= max;
		} else {
			return true;
		}
	}

	/**
	 * Checks if is valid mac address.
	 * 
	 * @param input
	 *            the value to be validated
	 * 
	 * @return true, if is valid mac address
	 */
	public boolean isValidMacAddress(final String input) {
		LOG.trace("Inside CommonValidator#isValidMacAddress");
		if(VtnServiceConsts.EMPTY_STRING.equals(input)){
			return true;
		}
		return input.matches(VtnServiceConsts.MAC_ADD_REGEX);
	}

	/**
	 * Checks if is valid ether type.
	 * 
	 * @param input
	 *            the value to be validated
	 * @return true, if is valid ether type
	 */
	public boolean isValidEtherType(final String input) {
		LOG.trace("Inside CommonValidator#isValidEtherType");
		if (VtnServiceConsts.EMPTY_STRING.equals(input)) {
			return true;
		} else if (input.contains("x") || input.contains("X")) {
			return input.matches(VtnServiceConsts.ETH_TYPE_REGEX);
		} else {
			return isValidRange(input, VtnServiceJsonConsts.VAL_0,
					VtnServiceJsonConsts.VAL_65535);
		}
	}

	/**
	 * Checks if is valid IP v4 address.
	 * 
	 * @param input
	 *            the value to be validated
	 * @return true, if is valid IP v4 address
	 */
	public boolean isValidIpV4(final String input) {
		LOG.trace("Inside CommonValidator#isValidIpV4");
		if (VtnServiceConsts.EMPTY_STRING.equals(input)) {
			return true;
		} else if (null != IpAddressUtil.textToNumericFormatV4(input)) {
			return true;
		} else {
			return false;
		}
	}

	/**
	 * Checks if is valid IP v6 address.
	 * 
	 * @param input
	 *            the value to be validated
	 * @return true, if is valid IP v6 address
	 */
	public boolean isValidIpV6(final String input) {
		LOG.trace("Inside CommonValidator#isValidIpV6");
		if (VtnServiceConsts.EMPTY_STRING.equals(input)) {
			return true;
		} else if (null != IpAddressUtil.textToNumericFormatV6(input)) {
			return true;
		} else {
			return false;
		}
	}

	/**
	 * Checks if is valid max length.
	 * 
	 * @param input
	 *            the value to be validated
	 * @param length
	 *            the maximum length
	 * @return true, if is valid max length
	 */
	public boolean isValidMaxLength(final String input, final int length) {
		LOG.trace("Inside CommonValidator#isValidMaxLength");
		return input.length() <= length;
	}

	/**
	 * @param input
	 *            the value to be validated
	 * 
	 * @param length
	 *            the maximum length possible
	 * 
	 * @return true, if is valid alpha numeric value
	 */
	public boolean isValidMaxLengthAlphaNum(final String input, final int length) {
		LOG.trace("Inside CommonValidator#isValidMaxLengthAlphaNum");
		if (VtnServiceConsts.EMPTY_STRING.equals(input)) {
			return true;
		}
		return input.matches(VtnServiceConsts.ALPHANUM_REGEX)
				&& input.length() <= length;
	}

	/**
	 * Type validation recommended values : bypass,pfc and vnp
	 * 
	 * @param input
	 *            the value to be validated
	 * @return true, if is valid type
	 */
	public boolean isValidType(final String input) {
		LOG.trace("Start CommonValidator#isValidType");
		if (VtnServiceConsts.EMPTY_STRING.equals(input)) {
			return true;
		}
		Boolean valid = false;
		if (VtnServiceJsonConsts.BYPASS.equalsIgnoreCase(input)
				|| VtnServiceJsonConsts.PFC.equalsIgnoreCase(input)
				|| VtnServiceJsonConsts.VNP.equalsIgnoreCase(input)) {
			valid = true;
		}
		LOG.trace("Complete CommonValidator#isValidType");
		return valid;
	}

	/**
	 * Check validation for Audit Status Parameter
	 * @param input
	 * @return
	 */
	public boolean isValidAuditStatus(final String input) {
		LOG.trace("Inside CommonValidator#isValidAuditStatus");
		if (VtnServiceConsts.EMPTY_STRING.equals(input)) {
			return true;
		}
		return VtnServiceJsonConsts.ENABLE.equalsIgnoreCase(input)
				|| VtnServiceJsonConsts.DISABLE.equalsIgnoreCase(input);
	}

	/**
	 * Validator for Switch Port and Link Function allows "-" and "/" in the
	 * value
	 * 
	 * @param linkName
	 *            the value to be validated
	 * @param length
	 *            the maximum length possible
	 * @return true, if is valid physical Id
	 */
	public boolean isValidPhysicalId(final String linkName, final int length) {
		LOG.trace("Inside CommonValidator#isValidPhysicalId");
		if (VtnServiceConsts.EMPTY_STRING.equals(linkName)) {
			return true;
		}
		return linkName.matches(VtnServiceConsts.SWITCH_REGEX)
				&& linkName.length() <= length;
	}

	/**
	 * Validator for version, allows alphanumeric characters and "." in the
	 * value
	 * 
	 * @param input
	 *            the value to be validated
	 * @param length
	 *            the maximum length possible
	 * @return true, if is valid value
	 */
	public boolean isValidVersion(final String input, final int length) {
		LOG.trace("Inside CommonValidator#isValidVersion");
		if (VtnServiceConsts.EMPTY_STRING.equals(input)) {
			return true;
		}
		return input.matches(VtnServiceConsts.VERSION_REGEX)
				&& input.length() <= length;
	}

	/**
	 * Validator for domain_id, allows alphanumeric characters and an underscore
	 * except for the leading underscores. for specifying default domain, string
	 * "(DEFAULT)" need to be passed.
	 * 
	 * @param input
	 *            the value to be validated
	 * @param length
	 *            the maximum length possible
	 * @return true, if is valid value
	 */
	public boolean isValidDomainId(final String input, final int length) {
		LOG.trace("Inside CommonValidator#isValidDomainId");
		if (VtnServiceConsts.EMPTY_STRING.equals(input)) {
			return true;
		}
		return (input.matches(VtnServiceConsts.ALPHANUM_REGEX) || input
				.equals(VtnServiceJsonConsts.DEFAULT_DOMAIN_ID))
				&& input.length() <= length;
	}

	/**
	 * Validate put request Json object for FlowFilterEntry APIs
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	public boolean isValidFlowFilterEntry(final JsonObject requestBody) {
		LOG.trace("Start CommonValidator#isValidFlowFilterEntry()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.FLOWFILTERENTRY);
		if (requestBody.has(VtnServiceJsonConsts.FLOWFILTERENTRY)
				&& requestBody.get(VtnServiceJsonConsts.FLOWFILTERENTRY)
				.isJsonObject()) {
			isValid = true;
			final JsonObject ffEntry = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.FLOWFILTERENTRY);
			// validation for key: fl_name
			setInvalidParameter(VtnServiceJsonConsts.FLNAME);
			if (ffEntry.has(VtnServiceJsonConsts.FLNAME)
					&& ffEntry.getAsJsonPrimitive(
							VtnServiceJsonConsts.FLNAME).getAsString() != null
							&& !ffEntry.get(VtnServiceJsonConsts.FLNAME)
							.getAsString().trim().isEmpty()) {
				isValid = isValidMaxLengthAlphaNum(ffEntry
						.getAsJsonPrimitive(VtnServiceJsonConsts.FLNAME)
						.getAsString().trim(), VtnServiceJsonConsts.LEN_32);
			}
			// validation for key: action_type
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.ACTIONTYPE);
				if (ffEntry.has(VtnServiceJsonConsts.ACTIONTYPE)
						&& ffEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.ACTIONTYPE).getAsString() != null) {
					final String actionType = ffEntry
							.getAsJsonPrimitive(VtnServiceJsonConsts.ACTIONTYPE)
							.getAsString().trim();
					isValid = actionType
							.equalsIgnoreCase(VtnServiceJsonConsts.PASS)
							|| actionType
							.equalsIgnoreCase(VtnServiceJsonConsts.DROP)
							|| actionType
							.equalsIgnoreCase(VtnServiceJsonConsts.REDIRECT);
				}
			}
			// validation for key: nmg_name
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.NMGNAME);
				if (ffEntry.has(VtnServiceJsonConsts.NMGNAME)
						&& ffEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.NMGNAME).getAsString() != null
								&& !ffEntry.get(VtnServiceJsonConsts.NMGNAME)
								.getAsString().trim().isEmpty()) {
					isValid = isValidMaxLengthAlphaNum(ffEntry
							.getAsJsonPrimitive(VtnServiceJsonConsts.NMGNAME)
							.getAsString().trim(), VtnServiceJsonConsts.LEN_31);
				}
			}
			// validation for key: priority
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.PRIORITY);
				if (ffEntry.has(VtnServiceJsonConsts.PRIORITY)
						&& ffEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.PRIORITY).getAsString() != null) {
					isValid = isValidRange(
							ffEntry
									.getAsJsonPrimitive(
											VtnServiceJsonConsts.PRIORITY)
											.getAsString().trim(),
											VtnServiceJsonConsts.VAL_0,
											VtnServiceJsonConsts.VAL_7);
				}
			}
			// validation for key: dscp
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.DSCP);
				if (ffEntry.has(VtnServiceJsonConsts.DSCP)
						&& ffEntry
						.getAsJsonPrimitive(VtnServiceJsonConsts.DSCP)
						.getAsString() != null) {
					isValid = isValidRange(
							ffEntry
									.getAsJsonPrimitive(
											VtnServiceJsonConsts.DSCP)
											.getAsString().trim(),
											VtnServiceJsonConsts.VAL_0,
											VtnServiceJsonConsts.VAL_63);
				}
			}
		}
		LOG.trace("Complete CommonValidator#isValidFlowFilterEntry()");
		return isValid;
	}


	/** Validate redirectdst Json Object in request Json object for FlowFilterEntry APIs
	 * 
	 * @param ffEntry
	 *            the request Json object to be validated
	 * @return true, if successful
	 */
	public boolean isValidRedirectDst(boolean isValid, final JsonObject ffEntry) {
		LOG.trace("Start CommonValidator#isValidRedirectDst()"); 
		setInvalidParameter(VtnServiceJsonConsts.REDIRECTDST);
		if (ffEntry.has(VtnServiceJsonConsts.REDIRECTDST)) {
			final JsonObject dest = ffEntry
					.getAsJsonObject(VtnServiceJsonConsts.REDIRECTDST);
			// validation for key: vnode_name (optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.VNODENAME);
				if (dest.has(VtnServiceJsonConsts.VNODENAME)
						&& dest.getAsJsonPrimitive(
								VtnServiceJsonConsts.VNODENAME)
								.getAsString() != null) {
					isValid = isValidMaxLengthAlphaNum(
							dest.getAsJsonPrimitive(
									VtnServiceJsonConsts.VNODENAME)
									.getAsString().trim(),
									VtnServiceJsonConsts.LEN_31)
									|| dest.getAsJsonPrimitive(
											VtnServiceJsonConsts.VNODENAME)
											.getAsString().trim().isEmpty();
				}
			}
			// validation for key: if_name (optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.IFNAME);
				if (dest.has(VtnServiceJsonConsts.IFNAME)
						&& dest.getAsJsonPrimitive(
								VtnServiceJsonConsts.IFNAME)
								.getAsString() != null) {
					isValid = isValidMaxLengthAlphaNum(
							dest.getAsJsonPrimitive(
									VtnServiceJsonConsts.IFNAME)
									.getAsString().trim(),
									VtnServiceJsonConsts.LEN_31)
									|| dest.getAsJsonPrimitive(
											VtnServiceJsonConsts.IFNAME)
											.getAsString().trim().isEmpty();
				}
			}
			// validation for key: macdstaddr (optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.MACDSTADDR);
				if (dest.has(VtnServiceJsonConsts.MACDSTADDR)
						&& dest.getAsJsonPrimitive(
								VtnServiceJsonConsts.MACDSTADDR)
								.getAsString() != null) {
					isValid = isValidMacAddress(dest
							.getAsJsonPrimitive(
									VtnServiceJsonConsts.MACDSTADDR)
									.getAsString().trim());
				}
			}
			// validation for key: macsrcaddr (optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.MACSRCADDR);
				if (isValid
						&& dest.has(VtnServiceJsonConsts.MACSRCADDR)
						&& dest.getAsJsonPrimitive(
								VtnServiceJsonConsts.MACSRCADDR)
								.getAsString() != null) {
					isValid = isValidMacAddress(dest
							.getAsJsonPrimitive(
									VtnServiceJsonConsts.MACSRCADDR)
									.getAsString().trim());
				}
			}
		}
		LOG.trace("Complete CommonValidator#isValidRedirectDst()");
		return isValid;
	}
}
