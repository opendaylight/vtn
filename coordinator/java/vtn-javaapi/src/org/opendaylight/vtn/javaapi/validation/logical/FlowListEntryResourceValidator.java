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
import org.opendaylight.vtn.javaapi.resources.logical.FlowListEntriesResource;
import org.opendaylight.vtn.javaapi.resources.logical.FlowListEntryResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class FlowListEntryResourceValidator validates request Json object for
 * FlowListEntry API.
 */
public class FlowListEntryResourceValidator extends VtnServiceValidator {

	private static final Logger LOG = Logger
			.getLogger(FlowListEntryResourceValidator.class.getName());
	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new flow list entry resource validator.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public FlowListEntryResourceValidator(final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for FlowListEntry API.
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start FlowListEntryResourceValidator#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.FLNAME);
		if (resource instanceof FlowListEntryResource
				&& ((FlowListEntryResource) resource).getFlName() != null
				&& !((FlowListEntryResource) resource).getFlName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((FlowListEntryResource) resource).getFlName(),
					VtnServiceJsonConsts.LEN_32);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.SEQNUM);
				if (((FlowListEntryResource) resource).getSeqnum() != null
						&& !((FlowListEntryResource) resource).getSeqnum()
								.isEmpty()) {
					isValid = validator.isValidRange(
							((FlowListEntryResource) resource).getSeqnum(),
							VtnServiceJsonConsts.VAL_1,
							VtnServiceJsonConsts.VAL_65535);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		} else if (resource instanceof FlowListEntriesResource
				&& ((FlowListEntriesResource) resource).getFlName() != null
				&& !((FlowListEntriesResource) resource).getFlName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((FlowListEntriesResource) resource).getFlName(),
					VtnServiceJsonConsts.LEN_32);
			setListOpFlag(true);
		}
		LOG.trace("Start FlowListEntryResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * Validate request Json object for get, put and post method of
	 * FlowListEntry API
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start FlowListEntryResourceValidator#validate()");
		LOG.info("Validating request for " + method
				+ " of FlowListEntryResourceValidator");
		boolean isValid = false;
		try {
			isValid = validateUri();
			if (isValid && requestBody != null
					&& VtnServiceConsts.GET.equals(method)) {
				isValid = validator.isValidGetForIntIndex(requestBody,
						isListOpFlag());
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
		LOG.trace("Complete FlowListEntryResourceValidator#validate()");
	}

	/**
	 * Validate put request Json object for FlowListEntry API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validatePut(final JsonObject requestBody) {
		LOG.trace("Start FlowListEntryResourceValidator#validatePut()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.FLOWLISTENTRY);
		if (requestBody.has(VtnServiceJsonConsts.FLOWLISTENTRY)
				&& requestBody.get(VtnServiceJsonConsts.FLOWLISTENTRY)
						.isJsonObject()) {
			isValid = true;
			final JsonObject flowListEntry = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.FLOWLISTENTRY);
			// validation for key: macdstaddr(optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.MACDSTADDR);
				if (flowListEntry.has(VtnServiceJsonConsts.MACDSTADDR)
						&& flowListEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.MACDSTADDR).getAsString() != null) {
					isValid = validator
							.isValidMacAddress(flowListEntry
									.getAsJsonPrimitive(
											VtnServiceJsonConsts.MACDSTADDR)
									.getAsString());
				}
			}
			// validation for key: macsrcaddr
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.MACSRCADDR);
				if (flowListEntry.has(VtnServiceJsonConsts.MACSRCADDR)
						&& flowListEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.MACSRCADDR).getAsString() != null) {
					isValid = validator
							.isValidMacAddress(flowListEntry
									.getAsJsonPrimitive(
											VtnServiceJsonConsts.MACSRCADDR)
									.getAsString());
				}
			}
			// validation for key: macethertype
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.MACETHERTYPE);
				if (flowListEntry.has(VtnServiceJsonConsts.MACETHERTYPE)
						&& flowListEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.MACETHERTYPE)
								.getAsString() != null) {
					isValid = validator.isValidEtherType(flowListEntry
							.getAsJsonPrimitive(
									VtnServiceJsonConsts.MACETHERTYPE)
							.getAsString());
				}
			}
			// validation for key: macvlanpriority
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.MACVLANPRIORITY);
				if (flowListEntry.has(VtnServiceJsonConsts.MACVLANPRIORITY)
						&& flowListEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.MACVLANPRIORITY)
								.getAsString() != null) {
					isValid = validator.isValidRange(
							flowListEntry.getAsJsonPrimitive(
									VtnServiceJsonConsts.MACVLANPRIORITY)
									.getAsString(), VtnServiceJsonConsts.VAL_0,
							VtnServiceJsonConsts.VAL_7);
				}
			}
			// validation for key: ipdstaddr
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.IPDSTADDR);
				if (flowListEntry.has(VtnServiceJsonConsts.IPDSTADDR)
						&& flowListEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.IPDSTADDR).getAsString() != null) {
					isValid = validator.isValidIpV4(flowListEntry
							.getAsJsonPrimitive(VtnServiceJsonConsts.IPDSTADDR)
							.getAsString());
				}
			}
			// validation for key: ipdstaddrprefix
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.IPDSTADDRPREFIX);
				if (flowListEntry.has(VtnServiceJsonConsts.IPDSTADDRPREFIX)
						&& flowListEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.IPDSTADDRPREFIX)
								.getAsString() != null) {
					isValid = validator.isValidRange(
							flowListEntry.getAsJsonPrimitive(
									VtnServiceJsonConsts.IPDSTADDRPREFIX)
									.getAsString(), VtnServiceJsonConsts.VAL_1,
							VtnServiceJsonConsts.VAL_32);
				}
			}
			// validation for key: ipsrcaddr
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.IPSRCADDR);
				if (flowListEntry.has(VtnServiceJsonConsts.IPSRCADDR)
						&& flowListEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.IPSRCADDR).getAsString() != null) {
					isValid = validator.isValidIpV4(flowListEntry
							.getAsJsonPrimitive(VtnServiceJsonConsts.IPSRCADDR)
							.getAsString());
				}
			}
			// validation for key: ipsrcaddrprefix
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.IPSRCADDRPREFIX);
				if (flowListEntry.has(VtnServiceJsonConsts.IPSRCADDRPREFIX)
						&& flowListEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.IPSRCADDRPREFIX)
								.getAsString() != null) {
					isValid = validator.isValidRange(
							flowListEntry.getAsJsonPrimitive(
									VtnServiceJsonConsts.IPSRCADDRPREFIX)
									.getAsString(), VtnServiceJsonConsts.VAL_1,
							VtnServiceJsonConsts.VAL_32);
				}
			}
			// validation for key: ipv6dstaddr"
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.IPV6DSTADDR);
				if (flowListEntry.has(VtnServiceJsonConsts.IPV6DSTADDR)
						&& flowListEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.IPV6DSTADDR).getAsString() != null) {
					isValid = validator.isValidIpV6(flowListEntry
							.getAsJsonPrimitive(
									VtnServiceJsonConsts.IPV6DSTADDR)
							.getAsString());
				}
			}
			// validation for key: ipv6dstaddrprefix
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.IPV6DSTADDRPREFIX);
				if (flowListEntry.has(VtnServiceJsonConsts.IPV6DSTADDRPREFIX)
						&& flowListEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.IPV6DSTADDRPREFIX)
								.getAsString() != null) {
					isValid = validator.isValidRange(
							flowListEntry.getAsJsonPrimitive(
									VtnServiceJsonConsts.IPV6DSTADDRPREFIX)
									.getAsString(), VtnServiceJsonConsts.VAL_1,
							VtnServiceJsonConsts.VAL_128);
				}
			}
			// validation for key: ipv6srcaddr
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.IPV6SRCADDR);
				if (flowListEntry.has(VtnServiceJsonConsts.IPV6SRCADDR)
						&& flowListEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.IPV6SRCADDR).getAsString() != null) {
					isValid = validator.isValidIpV6(flowListEntry
							.getAsJsonPrimitive(
									VtnServiceJsonConsts.IPV6SRCADDR)
							.getAsString());
				}
			}
			// validation for key: ipv6srcaddrprefix
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.IPV6SRCADDRPREFIX);
				if (flowListEntry.has(VtnServiceJsonConsts.IPV6SRCADDRPREFIX)
						&& flowListEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.IPV6SRCADDRPREFIX)
								.getAsString() != null) {
					isValid = validator.isValidRange(
							flowListEntry.getAsJsonPrimitive(
									VtnServiceJsonConsts.IPV6SRCADDRPREFIX)
									.getAsString(), VtnServiceJsonConsts.VAL_1,
							VtnServiceJsonConsts.VAL_128);
				}
			}
			// validation for key: ipproto
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.IPPROTO);
				if (flowListEntry.has(VtnServiceJsonConsts.IPPROTO)
						&& flowListEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.IPPROTO).getAsString() != null) {
					isValid = validator.isValidRange(flowListEntry
							.getAsJsonPrimitive(VtnServiceJsonConsts.IPPROTO)
							.getAsString(), VtnServiceJsonConsts.VAL_1,
							VtnServiceJsonConsts.VAL_255);
				}
			}
			// validation for key: ipdscp
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.IPDSCP);
				if (flowListEntry.has(VtnServiceJsonConsts.IPDSCP)
						&& flowListEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.IPDSCP).getAsString() != null) {
					isValid = validator.isValidRange(flowListEntry
							.getAsJsonPrimitive(VtnServiceJsonConsts.IPDSCP)
							.getAsString(), VtnServiceJsonConsts.VAL_0,
							VtnServiceJsonConsts.VAL_63);
				}
			}
			// validation for key: l4dstport
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.L4DSTPORT);
				if (flowListEntry.has(VtnServiceJsonConsts.L4DSTPORT)
						&& flowListEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.L4DSTPORT).getAsString() != null) {
					isValid = validator.isValidRange(flowListEntry
							.getAsJsonPrimitive(VtnServiceJsonConsts.L4DSTPORT)
							.getAsString(), VtnServiceJsonConsts.VAL_0,
							VtnServiceJsonConsts.VAL_65535);
				}
			}
			// validation for key: l4dstendport
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.L4DSTENDPORT);
				if (flowListEntry.has(VtnServiceJsonConsts.L4DSTENDPORT)
						&& flowListEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.L4DSTENDPORT)
								.getAsString() != null) {
					isValid = validator.isValidRange(
							flowListEntry.getAsJsonPrimitive(
									VtnServiceJsonConsts.L4DSTENDPORT)
									.getAsString(), VtnServiceJsonConsts.VAL_1,
							VtnServiceJsonConsts.VAL_65535);
				}
			}
			// validation for key: l4srcport
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.L4SRCPORT);
				if (flowListEntry.has(VtnServiceJsonConsts.L4SRCPORT)
						&& flowListEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.L4SRCPORT).getAsString() != null) {
					isValid = validator.isValidRange(flowListEntry
							.getAsJsonPrimitive(VtnServiceJsonConsts.L4SRCPORT)
							.getAsString(), VtnServiceJsonConsts.VAL_0,

					VtnServiceJsonConsts.VAL_65535);
				}
			}
			// validation for key: l4srcendport
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.L4SRCENDPORT);
				if (flowListEntry.has(VtnServiceJsonConsts.L4SRCENDPORT)
						&& flowListEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.L4SRCENDPORT)
								.getAsString() != null) {
					isValid = validator.isValidRange(
							flowListEntry.getAsJsonPrimitive(
									VtnServiceJsonConsts.L4SRCENDPORT)
									.getAsString(), VtnServiceJsonConsts.VAL_1,
							VtnServiceJsonConsts.VAL_65535);
				}
			}
			// validation for key: icmptypenum
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.ICMPTYPENUM);
				if (flowListEntry.has(VtnServiceJsonConsts.ICMPTYPENUM)
						&& flowListEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.ICMPTYPENUM).getAsString() != null) {
					isValid = validator.isValidRange(
							flowListEntry.getAsJsonPrimitive(
									VtnServiceJsonConsts.ICMPTYPENUM)
									.getAsString(), VtnServiceJsonConsts.VAL_0,
							VtnServiceJsonConsts.VAL_255);
				}
			}
			// validation for key: icmpcodenum
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.ICMPCODENUM);
				if (flowListEntry.has(VtnServiceJsonConsts.ICMPCODENUM)
						&& flowListEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.ICMPCODENUM).getAsString() != null) {
					isValid = validator.isValidRange(
							flowListEntry.getAsJsonPrimitive(
									VtnServiceJsonConsts.ICMPCODENUM)
									.getAsString(), VtnServiceJsonConsts.VAL_0,
							VtnServiceJsonConsts.VAL_255);
				}
			}
			// validation for key: ipv6icmptypenum
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.IPV6ICMPTYPENUM);
				if (flowListEntry.has(VtnServiceJsonConsts.IPV6ICMPTYPENUM)
						&& flowListEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.IPV6ICMPTYPENUM)
								.getAsString() != null) {
					isValid = validator.isValidRange(
							flowListEntry.getAsJsonPrimitive(
									VtnServiceJsonConsts.IPV6ICMPTYPENUM)
									.getAsString(), VtnServiceJsonConsts.VAL_0,
							VtnServiceJsonConsts.VAL_255);
				}
			}
			// validation for key: ipv6icmpcodenum
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.IPV6ICMPCODENUM);
				if (flowListEntry.has(VtnServiceJsonConsts.IPV6ICMPCODENUM)
						&& flowListEntry.getAsJsonPrimitive(
								VtnServiceJsonConsts.IPV6ICMPCODENUM)
								.getAsString() != null) {
					isValid = validator.isValidRange(
							flowListEntry.getAsJsonPrimitive(
									VtnServiceJsonConsts.IPV6ICMPCODENUM)
									.getAsString(), VtnServiceJsonConsts.VAL_0,
							VtnServiceJsonConsts.VAL_255);
				}
			}
		}
		LOG.trace("Complete FlowListEntryResourceValidator#validatePut()");
		return isValid;
	}

	/**
	 * Validate post request Json object for FlowListEntryresource
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	private boolean validatePost(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start FlowListEntryResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.FLOWLISTENTRY);
		if (requestBody.has(VtnServiceJsonConsts.FLOWLISTENTRY)
				&& requestBody.get(VtnServiceJsonConsts.FLOWLISTENTRY)
						.isJsonObject()) {
			final JsonObject flowListEntry = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.FLOWLISTENTRY);
			// validation for key: seqnum
			setInvalidParameter(VtnServiceJsonConsts.SEQNUM);
			if (flowListEntry.has(VtnServiceJsonConsts.SEQNUM)
					&& flowListEntry.getAsJsonPrimitive(
							VtnServiceJsonConsts.SEQNUM).getAsString() != null) {
				isValid = validator.isValidRange(flowListEntry
						.getAsJsonPrimitive(VtnServiceJsonConsts.SEQNUM)
						.getAsString(), VtnServiceJsonConsts.VAL_1,
						VtnServiceJsonConsts.VAL_65535);
			}
			if (isValid) {
				isValid = validatePut(requestBody);
			}

		}
		LOG.trace("Complete FlowListEntryResourceValidator#validatePost()");
		return isValid;
	}
}
