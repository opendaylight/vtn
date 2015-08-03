/*
 * Copyright (c) 2012-2015 NEC Corporation
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
import org.opendaylight.vtn.javaapi.resources.logical.VtnStationsResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class VtnStationsResourceValidator validates request Json object for
 * VtnStations API.
 */
public class VtnStationsResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(VtnStationsResourceValidator.class.getName());

	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new vtn stations resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public VtnStationsResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri for VtnStations API.
	 * 
	 * @return true, if successful
	 */
	@Override
	public boolean validateUri() {
		LOG.trace("Start VtnStationsResourceValidator#validateUri()");
		boolean isValid = false;
		if (resource instanceof VtnStationsResource) {
			isValid = true;
			setListOpFlag(true);
		}
		LOG.trace("Complete VtnStationsResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request Json object for get, put and post method of VtnStations
	 * API.
	 */
	@Override
	public void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VtnStationsResourceValidator#validate()");
		boolean isValid = false;
		try {
			isValid = validateUri();
			LOG.info("Validating request for " + method
					+ " of VtnStationsResourceValidator");
			if (isValid && requestBody != null
					&& VtnServiceConsts.GET.equals(method)) {
				isValid = validateGet(requestBody);
				updateOpParameterForList(requestBody);
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
		LOG.trace("Complete VtnStationsResourceValidator#validate()");
	}

	/**
	 * Validate get request Json object for VtnStations API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validateGet(final JsonObject requestBody) {
		LOG.trace("Start VtnStationsResourceValidator#validateGet()");
		boolean isValid = false;
		// validation for key: controller_id(mandatory)
		setInvalidParameter(VtnServiceJsonConsts.CONTROLLERID);
		if (requestBody.has(VtnServiceJsonConsts.CONTROLLERID)
				&& requestBody.getAsJsonPrimitive(
						VtnServiceJsonConsts.CONTROLLERID).getAsString() != null) {
			isValid = validator.isValidMaxLengthAlphaNum(requestBody
					.getAsJsonPrimitive(VtnServiceJsonConsts.CONTROLLERID)
					.getAsString(), VtnServiceJsonConsts.LEN_31);
		}
		if (isValid) {
			// only vlan_id or no_vlan_id is allowed
			setInvalidParameter(VtnServiceJsonConsts.VLANID
					+ VtnServiceJsonConsts.SLASH
					+ VtnServiceJsonConsts.NO_VLAN_ID);
			if (requestBody.has(VtnServiceJsonConsts.VLANID)
					&& requestBody.has(VtnServiceJsonConsts.NO_VLAN_ID)) {
				isValid = false;
			} else if (requestBody.has(VtnServiceJsonConsts.VLANID)) {
				setInvalidParameter(VtnServiceJsonConsts.VLANID);
				if (requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.VLANID)
						.getAsString() != null
						&& !requestBody
								.getAsJsonPrimitive(VtnServiceJsonConsts.VLANID)
								.getAsString().isEmpty()) {
					// validation for key: vlan_id
					isValid = validator.isValidRange(requestBody
							.getAsJsonPrimitive(VtnServiceJsonConsts.VLANID)
							.getAsString(), VtnServiceJsonConsts.VAL_1,
							VtnServiceJsonConsts.VAL_4095);
				} else {
					isValid = false;
				}
			} else if (requestBody.has(VtnServiceJsonConsts.NO_VLAN_ID)) {
				// validation for key: no_vlan_id
				setInvalidParameter(VtnServiceJsonConsts.NO_VLAN_ID);
				if (requestBody.getAsJsonPrimitive(
						VtnServiceJsonConsts.NO_VLAN_ID).getAsString() != null
						&& !requestBody
								.getAsJsonPrimitive(
										VtnServiceJsonConsts.NO_VLAN_ID)
								.getAsString().isEmpty()) {
					final String no_vlan_id = requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.NO_VLAN_ID).getAsString();
					isValid = no_vlan_id
							.equalsIgnoreCase(VtnServiceJsonConsts.TRUE);
				} else {
					isValid = false;
				}
			}
		}
		// validation for key: op
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.OP);
			isValid = validator.isValidOperation(requestBody);
		}
		// validation for key: macaddr
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.MACADDR);
			if (requestBody.has(VtnServiceJsonConsts.MACADDR)
					&& requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.MACADDR).getAsString() != null) {
				isValid = validator.isValidMacAddress(requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.MACADDR)
						.getAsString());
			}
		}
		// validation for key: ipaddr
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.IPADDR);
			if (requestBody.has(VtnServiceJsonConsts.IPADDR)
					&& requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.IPADDR).getAsString() != null) {
				isValid = validator.isValidIpV4(requestBody.getAsJsonPrimitive(
						VtnServiceJsonConsts.IPADDR).getAsString());
			}
		}
		// validation for key: ipv6addr
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.IPV6ADDR);
			if (requestBody.has(VtnServiceJsonConsts.IPV6ADDR)
					&& requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.IPV6ADDR).getAsString() != null) {
				isValid = validator.isValidIpV6(requestBody.getAsJsonPrimitive(
						VtnServiceJsonConsts.IPV6ADDR).getAsString());
			}
		}
		// validation for key: switch_id
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.SWITCHID);
			if (requestBody.has(VtnServiceJsonConsts.SWITCHID)
					&& requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.SWITCHID).getAsString() != null) {
				isValid = validator.isValidMaxLength(requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.SWITCHID)
						.getAsString(), VtnServiceJsonConsts.LEN_255)
						|| requestBody
								.getAsJsonPrimitive(
										VtnServiceJsonConsts.SWITCHID)
								.getAsString().isEmpty();
			}
		}
		// validation for key: port_name
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.PORTNAME);
			if (requestBody.has(VtnServiceJsonConsts.PORTNAME)
					&& requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.PORTNAME).getAsString() != null) {
				isValid = validator.isValidMaxLength(requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.PORTNAME)
						.getAsString(), VtnServiceJsonConsts.LEN_31)
						|| requestBody
								.getAsJsonPrimitive(
										VtnServiceJsonConsts.PORTNAME)
								.getAsString().isEmpty();
			}
		}

		// validation for key: vtn_name
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.VTNNAME);
			if (requestBody.has(VtnServiceJsonConsts.VTNNAME)
					&& requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.VTNNAME).getAsString() != null) {
				isValid = validator.isValidMaxLengthAlphaNum(requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.VTNNAME)
						.getAsString(), VtnServiceJsonConsts.LEN_31)
						|| requestBody
								.getAsJsonPrimitive(
										VtnServiceJsonConsts.VTNNAME)
								.getAsString().isEmpty();
			}
		}
		// validation for key: vnode_name
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.VNODENAME);
			if (requestBody.has(VtnServiceJsonConsts.VNODENAME)
					&& requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.VNODENAME).getAsString() != null) {
				isValid = validator.isValidMaxLengthAlphaNum(requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.VNODENAME)
						.getAsString(), VtnServiceJsonConsts.LEN_31)
						|| requestBody
								.getAsJsonPrimitive(
										VtnServiceJsonConsts.VNODENAME)
								.getAsString().isEmpty();
			}
		}
		// validation for key: if_name
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.IFNAME);
			if (requestBody.has(VtnServiceJsonConsts.IFNAME)
					&& requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.IFNAME).getAsString() != null) {
				isValid = validator.isValidMaxLengthAlphaNum(requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.IFNAME)
						.getAsString(), VtnServiceJsonConsts.LEN_31)
						|| requestBody
								.getAsJsonPrimitive(VtnServiceJsonConsts.IFNAME)
								.getAsString().isEmpty();
			}
		}
		// validation for key: domain_id
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.DOMAINID);
			if (requestBody.has(VtnServiceJsonConsts.DOMAINID)
					&& requestBody.getAsJsonPrimitive(
							VtnServiceJsonConsts.DOMAINID).getAsString() != null
					&& !requestBody
							.getAsJsonPrimitive(VtnServiceJsonConsts.DOMAINID)
							.getAsString().isEmpty()) {
				isValid = validator.isValidDomainId(requestBody
						.getAsJsonPrimitive(VtnServiceJsonConsts.DOMAINID)
						.getAsString(), VtnServiceJsonConsts.LEN_31);
			}
		}
		if (requestBody.has(VtnServiceJsonConsts.INDEX)) {
			requestBody.remove(VtnServiceJsonConsts.INDEX);
		} else {
			LOG.debug("No need to remove");
		}
		LOG.trace("Complete VtnStationsResourceValidator#validateGet()");
		return isValid;
	}
}
