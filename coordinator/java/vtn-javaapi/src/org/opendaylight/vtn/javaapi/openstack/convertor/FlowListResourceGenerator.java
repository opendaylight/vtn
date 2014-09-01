/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.openstack.convertor;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;

/**
 * Provides Request-Response Conversion methods for flow list requests
 */
public class FlowListResourceGenerator {

	/**
	 * Generated Get flow list request body from OpenStack request body
	 * 
	 * @param openStackResourceBody
	 *            - OpenStack request body for tenant
	 * @return - port request body
	 */
	public static JsonObject getFlowListEntryRequestBody() {
		final JsonObject request = new JsonObject();
		request.addProperty(VtnServiceJsonConsts.TARGETDB,
				VtnServiceJsonConsts.STATE);
		return request;
	}

	/**
	 * Convert response for flow list from UNC format to OpenStack format
	 * 
	 * @param responseBody
	 *            - UNC formatted response body
	 * @return - OpenStack formatted response body
	 */
	public static JsonObject setFlowListResponseBody(
			final JsonObject openStackResponse, JsonObject responseBody) {

		JsonObject flowlistentry = responseBody.getAsJsonObject()
				.get(VtnServiceJsonConsts.FLOWLISTENTRY).getAsJsonObject();

		if (null != flowlistentry.get(VtnServiceJsonConsts.MACSRCADDR)) {
			String mac = flowlistentry.get(VtnServiceJsonConsts.MACSRCADDR)
					.getAsString();
			openStackResponse.addProperty(VtnServiceOpenStackConsts.SRC_MAC,
					mac3To6(mac));
		}
		if (null != flowlistentry.get(VtnServiceJsonConsts.MACDSTADDR)) {
			String mac = flowlistentry.get(VtnServiceJsonConsts.MACDSTADDR)
					.getAsString();
			openStackResponse.addProperty(VtnServiceOpenStackConsts.DST_MAC,
					mac3To6(mac));
		}
		if (null != flowlistentry.get(VtnServiceJsonConsts.MACETHERTYPE)) {
			openStackResponse.addProperty(VtnServiceOpenStackConsts.ETH_TYPE,
					flowlistentry.get(VtnServiceJsonConsts.MACETHERTYPE)
							.getAsString());
		}
		if (null != flowlistentry.get(VtnServiceJsonConsts.IPSRCADDR)
				&& null != flowlistentry
						.get(VtnServiceJsonConsts.IPSRCADDRPREFIX)) {
			openStackResponse.addProperty(
					VtnServiceOpenStackConsts.SRC_CIDR,
					flowlistentry.get(VtnServiceJsonConsts.IPSRCADDR)
							.getAsString()
							+ VtnServiceJsonConsts.SLASH
							+ flowlistentry.get(
									VtnServiceJsonConsts.IPSRCADDRPREFIX)
									.getAsString());
		}
		if (null != flowlistentry.get(VtnServiceJsonConsts.IPDSTADDR)
				&& null != flowlistentry
						.get(VtnServiceJsonConsts.IPDSTADDRPREFIX)) {
			openStackResponse.addProperty(
					VtnServiceOpenStackConsts.DST_CIDR,
					flowlistentry.get(VtnServiceJsonConsts.IPDSTADDR)
							.getAsString()
							+ VtnServiceJsonConsts.SLASH
							+ flowlistentry.get(
									VtnServiceJsonConsts.IPDSTADDRPREFIX)
									.getAsString());
		}
		if (null != flowlistentry.get(VtnServiceJsonConsts.IPPROTO)) {
			String ipProto = flowlistentry.get(VtnServiceJsonConsts.IPPROTO)
					.getAsString();
			openStackResponse.addProperty(VtnServiceOpenStackConsts.PROTOCOL,
					ipProto);
			/*
			 * ICMP protocol
			 */
			if ("1".equals(ipProto)) {
				if (null != flowlistentry.get(VtnServiceJsonConsts.ICMPTYPENUM)) {
					openStackResponse.addProperty(
							VtnServiceOpenStackConsts.SRC_PORT, flowlistentry
									.get(VtnServiceJsonConsts.ICMPTYPENUM)
									.getAsString());
				}
				if (null != flowlistentry.get(VtnServiceJsonConsts.ICMPCODENUM)) {
					openStackResponse.addProperty(
							VtnServiceOpenStackConsts.DST_PORT, flowlistentry
									.get(VtnServiceJsonConsts.ICMPCODENUM)
									.getAsString());
				}
			} else {
				if (null != flowlistentry.get(VtnServiceJsonConsts.L4SRCPORT)) {
					openStackResponse.addProperty(
							VtnServiceOpenStackConsts.SRC_PORT, flowlistentry
									.get(VtnServiceJsonConsts.L4SRCPORT)
									.getAsString());
				}
				if (null != flowlistentry.get(VtnServiceJsonConsts.L4DSTPORT)) {
					openStackResponse.addProperty(
							VtnServiceOpenStackConsts.DST_PORT, flowlistentry
									.get(VtnServiceJsonConsts.L4DSTPORT)
									.getAsString());
				}
			}
		} else {
			if (null != flowlistentry.get(VtnServiceJsonConsts.L4SRCPORT)) {
				openStackResponse.addProperty(
						VtnServiceOpenStackConsts.SRC_PORT,
						flowlistentry.get(VtnServiceJsonConsts.L4SRCPORT)
								.getAsString());
			}
			if (null != flowlistentry.get(VtnServiceJsonConsts.L4DSTPORT)) {
				openStackResponse.addProperty(
						VtnServiceOpenStackConsts.DST_PORT,
						flowlistentry.get(VtnServiceJsonConsts.L4DSTPORT)
								.getAsString());
			}
		}

		return openStackResponse;
	}

	/**
	 * Generated delete flow list request body from protocol
	 * 
	 * @param protocol
	 *            - protocol from UPLL, 1 is ICMP, otherwise not ICMP
	 * @return - flow list entry request body
	 */
	public static JsonObject getDelFlowListEntryRequestBody(String protocol) {
		JsonObject root = new JsonObject();
		JsonObject flowListEntry = new JsonObject();

		if (protocol.equals("1")) {
			flowListEntry.addProperty(VtnServiceJsonConsts.ICMPTYPENUM,
					VtnServiceJsonConsts.EMPTY);
			flowListEntry.addProperty(VtnServiceJsonConsts.ICMPCODENUM,
					VtnServiceJsonConsts.EMPTY);
		} else {
			flowListEntry.addProperty(VtnServiceJsonConsts.L4SRCPORT,
					VtnServiceJsonConsts.EMPTY);
			flowListEntry.addProperty(VtnServiceJsonConsts.L4DSTPORT,
					VtnServiceJsonConsts.EMPTY);
		}

		root.add(VtnServiceJsonConsts.FLOWLISTENTRY, flowListEntry);

		return root;
	}

	/**
	 * Generated post or put flow list request body from OpenStack request body
	 * 
	 * @param openStackResourceBody
	 *            - OpenStack request body for tenant
	 * @param type
	 *            - type of request body
	 * @param protocol
	 *            - protocol from UPLL, 1 is ICMP, otherwise not ICMP
	 * @return - flow list entry request body
	 */
	public static JsonObject getFlowListEntryRequestBody(
			JsonObject openStackRsourceBody, String type, String protocol) {
		JsonObject root = new JsonObject();
		JsonObject flowListEntry = new JsonObject();

		if (type.equalsIgnoreCase(VtnServiceConsts.POST)) {
			flowListEntry.addProperty(VtnServiceJsonConsts.SEQNUM, "1");
		}

		if (openStackRsourceBody.has(VtnServiceOpenStackConsts.DST_MAC)) {
			flowListEntry.addProperty(
					VtnServiceJsonConsts.MACDSTADDR,
					mac6To3(openStackRsourceBody.get(
							VtnServiceOpenStackConsts.DST_MAC).getAsString()));
		}

		if (openStackRsourceBody.has(VtnServiceOpenStackConsts.SRC_MAC)) {
			flowListEntry.addProperty(
					VtnServiceJsonConsts.MACSRCADDR,
					mac6To3(openStackRsourceBody.get(
							VtnServiceOpenStackConsts.SRC_MAC).getAsString()));
		}

		if (openStackRsourceBody.has(VtnServiceOpenStackConsts.ETH_TYPE)) {
			flowListEntry.addProperty(VtnServiceJsonConsts.MACETHERTYPE,
					openStackRsourceBody
							.get(VtnServiceOpenStackConsts.ETH_TYPE)
							.getAsString());
		}

		if (openStackRsourceBody.has(VtnServiceOpenStackConsts.DST_CIDR)) {
			if (openStackRsourceBody.get(VtnServiceOpenStackConsts.DST_CIDR)
					.getAsString().equals(VtnServiceJsonConsts.EMPTY)) {
				flowListEntry.addProperty(VtnServiceJsonConsts.IPDSTADDR,
						VtnServiceJsonConsts.EMPTY);
				flowListEntry.addProperty(VtnServiceJsonConsts.IPDSTADDRPREFIX,
						VtnServiceJsonConsts.EMPTY);
			} else {
				String[] ip = openStackRsourceBody
						.get(VtnServiceOpenStackConsts.DST_CIDR).getAsString()
						.split(VtnServiceJsonConsts.SLASH);
				flowListEntry
						.addProperty(VtnServiceJsonConsts.IPDSTADDR, ip[0]);
				flowListEntry.addProperty(VtnServiceJsonConsts.IPDSTADDRPREFIX,
						ip[1]);
			}
		}

		if (openStackRsourceBody.has(VtnServiceOpenStackConsts.SRC_CIDR)) {
			if (openStackRsourceBody.get(VtnServiceOpenStackConsts.SRC_CIDR)
					.getAsString().equals(VtnServiceJsonConsts.EMPTY)) {
				flowListEntry.addProperty(VtnServiceJsonConsts.IPSRCADDR,
						VtnServiceJsonConsts.EMPTY);
				flowListEntry.addProperty(VtnServiceJsonConsts.IPSRCADDRPREFIX,
						VtnServiceJsonConsts.EMPTY);
			} else {
				String[] ip = openStackRsourceBody
						.get(VtnServiceOpenStackConsts.SRC_CIDR).getAsString()
						.split(VtnServiceJsonConsts.SLASH);
				flowListEntry
						.addProperty(VtnServiceJsonConsts.IPSRCADDR, ip[0]);
				flowListEntry.addProperty(VtnServiceJsonConsts.IPSRCADDRPREFIX,
						ip[1]);
			}
		}

		int protocolVal = 0;
		if (openStackRsourceBody.has(VtnServiceOpenStackConsts.PROTOCOL)) {
			flowListEntry.addProperty(VtnServiceJsonConsts.IPPROTO,
					openStackRsourceBody
							.get(VtnServiceOpenStackConsts.PROTOCOL)
							.getAsString());
			if (!openStackRsourceBody.get(VtnServiceOpenStackConsts.PROTOCOL)
					.getAsString().equals(VtnServiceConsts.EMPTY_STRING)) {
				protocolVal = Integer.parseInt(openStackRsourceBody.get(
						VtnServiceOpenStackConsts.PROTOCOL).getAsString());
			}
		}

		if (type.equalsIgnoreCase(VtnServiceConsts.PUT)) {
			if (!protocol.equals("-1")) {
				protocolVal = Integer.parseInt(protocol);
			}
		}
		if (openStackRsourceBody.has(VtnServiceOpenStackConsts.SRC_PORT)) {
			if (1 == protocolVal) {
				flowListEntry.addProperty(
						VtnServiceJsonConsts.ICMPTYPENUM,
						openStackRsourceBody.get(
								VtnServiceOpenStackConsts.SRC_PORT)
								.getAsString());
			} else {
				flowListEntry.addProperty(
						VtnServiceJsonConsts.L4SRCPORT,
						openStackRsourceBody.get(
								VtnServiceOpenStackConsts.SRC_PORT)
								.getAsString());
			}
		}

		if (openStackRsourceBody.has(VtnServiceOpenStackConsts.DST_PORT)) {
			if (1 == protocolVal) {
				flowListEntry.addProperty(
						VtnServiceJsonConsts.ICMPCODENUM,
						openStackRsourceBody.get(
								VtnServiceOpenStackConsts.DST_PORT)
								.getAsString());
			} else {
				flowListEntry.addProperty(
						VtnServiceJsonConsts.L4DSTPORT,
						openStackRsourceBody.get(
								VtnServiceOpenStackConsts.DST_PORT)
								.getAsString());
			}
		}

		root.add(VtnServiceJsonConsts.FLOWLISTENTRY, flowListEntry);

		return root;
	}

	/**
	 * Convert MAC address from HHHH.HHHH.HHHH to HH:HH:HH:HH:HH:HH
	 */
	public static String mac3To6(String mac) {
		StringBuilder sb = new StringBuilder();
		if (null != mac && mac.matches(VtnServiceConsts.MAC_ADD_REGEX)) {
			sb.append(mac.substring(0, 2));
			sb.append(VtnServiceConsts.COLON);
			sb.append(mac.substring(2, 4));
			sb.append(VtnServiceConsts.COLON);
			sb.append(mac.substring(5, 7));
			sb.append(VtnServiceConsts.COLON);
			sb.append(mac.substring(7, 9));
			sb.append(VtnServiceConsts.COLON);
			sb.append(mac.substring(10, 12));
			sb.append(VtnServiceConsts.COLON);
			sb.append(mac.substring(12));
		}

		return sb.toString();
	}

	/**
	 * Convert mac format.(Example: 11:22:33:44:55:66 => 1122.3344.5566)
	 * 
	 * @param mac
	 *            - HH:HH:HH:HH:HH:HH format mac address
	 * @return - HHHH.HHHH.HHHH format mac address
	 */
	public static String mac6To3(String mac) {
		StringBuilder sb = new StringBuilder();
		if (null != mac && !mac.isEmpty()
				&& mac.matches(VtnServiceOpenStackConsts.OS_MAC_ADD_REGEX)) {
			mac = mac.replace(VtnServiceConsts.COLON,
					VtnServiceConsts.EMPTY_STRING);
			sb.append(mac.substring(0, 4));
			sb.append(VtnServiceConsts.DOT);
			sb.append(mac.substring(4, 8));
			sb.append(VtnServiceConsts.DOT);
			sb.append(mac.substring(8));
		}

		return sb.toString();
	}
}
