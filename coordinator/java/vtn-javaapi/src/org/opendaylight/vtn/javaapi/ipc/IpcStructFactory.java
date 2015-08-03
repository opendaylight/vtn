/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi.ipc;

import java.util.ArrayList;
import java.util.List;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.ipc.IpcStruct;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceIpcConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.init.VtnServiceInitManager;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcDataUnitWrapper;
import org.opendaylight.vtn.javaapi.ipc.enums.UncIndexEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncPhysicalStructIndexEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncStructEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncStructIndexEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncUPLLEnums.ValLabelType;
import org.opendaylight.vtn.javaapi.ipc.enums.UncUPLLEnums.ValUnifiedNwRoutingType;

/**
 * A factory for creating IpcStruct objects.
 */
public class IpcStructFactory {

	private static final Logger LOG = Logger.getLogger(IpcStructFactory.class
			.getName());

	/**
	 * Gets the key vtn struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key vtn struct
	 */
	public final IpcStruct getKeyVtnStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		/*
		 * ipc_struct KeyVtn { UINT8 vtn_name[32]; };
		 */
		LOG.trace("Start getKeyVtnStruct");
		final IpcStruct keyVtnStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyVtn.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.VTN)
				&& ((JsonObject) requestBody.get(VtnServiceJsonConsts.VTN))
						.has(VtnServiceJsonConsts.VTNNAME)) {
			keyVtnStruct.set(VtnServiceJsonConsts.VTNNAME, IpcDataUnitWrapper
					.setIpcUint8ArrayValue(((JsonObject) requestBody
							.get(VtnServiceJsonConsts.VTN)).get(
							VtnServiceJsonConsts.VTNNAME).getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.ONE.ordinal()) {
			keyVtnStruct.set(VtnServiceJsonConsts.VTNNAME, IpcDataUnitWrapper
					.setIpcUint8ArrayValue(uriParameters.get(0)));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVtnStruct");
		}
		LOG.trace("Complete getKeyVtnStruct");
		return keyVtnStruct;
	}

	/**
	 * Gets the val_vtn struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the val_vtn struct
	 */
	public final IpcStruct getValVtnStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		LOG.trace("Start getValVtnStruct");
		final IpcStruct valVtnStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValVtn.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.VTN)
				&& requestBody.getAsJsonObject(VtnServiceJsonConsts.VTN).has(
						VtnServiceJsonConsts.DESCRIPTION)) {
			valVtnStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVtnIndex.UPLL_IDX_DESC_VTN
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
											.ordinal()));
			valVtnStruct.set(VtnServiceJsonConsts.DESCRIPTION,
					IpcDataUnitWrapper.setIpcUint8ArrayValue(
							requestBody
									.getAsJsonObject(VtnServiceJsonConsts.VTN)
									.get(VtnServiceJsonConsts.DESCRIPTION)
									.getAsString(), valVtnStruct,
							UncStructIndexEnum.ValVtnIndex.UPLL_IDX_DESC_VTN
									.ordinal()));
		} else {
			valVtnStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVtnIndex.UPLL_IDX_DESC_VTN
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
		}
		LOG.info("Value Structure: " + valVtnStruct.toString());
		LOG.trace("Complete getValVtnStruct");
		return valVtnStruct;
	}

	/**
	 * Gets the key vrt struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key vrt struct
	 */
	public final IpcStruct getKeyVrtStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		LOG.trace("Start getKeyVrtStruct");
		final IpcStruct keyVtnVrtStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyVrt.getValue());
		IpcStruct keyVtnStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.ONE.ordinal()) {
			keyVtnStruct = getKeyVtnStruct(requestBody,
					uriParameters.subList(0, 1));
		}
		keyVtnVrtStruct.set(VtnServiceIpcConsts.VTNKEY, keyVtnStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.VROUTER)
				&& ((JsonObject) requestBody.get(VtnServiceJsonConsts.VROUTER))
						.has(VtnServiceJsonConsts.VRTNAME)) {
			keyVtnVrtStruct
					.set(VtnServiceIpcConsts.VROUTERNAME,
							IpcDataUnitWrapper
									.setIpcUint8ArrayValue(((JsonObject) requestBody
											.get(VtnServiceJsonConsts.VROUTER))
											.get(VtnServiceJsonConsts.VRTNAME)
											.getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.TWO.ordinal()) {
			keyVtnVrtStruct.set(VtnServiceIpcConsts.VROUTERNAME,
					IpcDataUnitWrapper.setIpcUint8ArrayValue(uriParameters
							.get(1)));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVrtStruct");
		}
		LOG.info("Key Structure: " + keyVtnVrtStruct.toString());
		LOG.trace("Complete getKeyVrtStruct");
		return keyVtnVrtStruct;
	}

	/**
	 * Gets the key dhcp relay if struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key dhcp relay if struct
	 */
	public final IpcStruct getKeyDhcpRelayIfStruct(
			final JsonObject requestBody, final List<String> uriParameters) {

		// Lower level structure
		/*
		 * ipc_struct KeyDhcpRelayIf{ key_vrt vrt_key; UINT8 if_name[32]; };
		 */
		LOG.trace("Start getKeyDhcpRelayIfStruct");
		final IpcStruct keyDhcpRelayIfStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyDhcpRelayIf.getValue());
		IpcStruct keyVtnVrtStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.TWO.ordinal()) {
			keyVtnVrtStruct = getKeyVrtStruct(requestBody,
					uriParameters.subList(0, 2));
		}
		keyDhcpRelayIfStruct.set(VtnServiceIpcConsts.VRTKEY, keyVtnVrtStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.INTERFACE)
				&& ((JsonObject) requestBody
						.get(VtnServiceJsonConsts.INTERFACE))
						.has(VtnServiceJsonConsts.IFNAME)) {
			keyDhcpRelayIfStruct
					.set(VtnServiceIpcConsts.IFNAME, IpcDataUnitWrapper
							.setIpcUint8ArrayValue(((JsonObject) requestBody
									.get(VtnServiceJsonConsts.INTERFACE)).get(
									VtnServiceJsonConsts.IFNAME).getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.THREE.ordinal()) {
			keyDhcpRelayIfStruct.set(VtnServiceIpcConsts.IFNAME,
					IpcDataUnitWrapper.setIpcUint8ArrayValue(uriParameters
							.get(2)));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyDhcpRelayIfStruct");
		}
		LOG.info("Key Structure: " + keyDhcpRelayIfStruct.toString());
		LOG.trace("Complete getKeyDhcpRelayIfStruct");
		return keyDhcpRelayIfStruct;
	}

	/**
	 * Gets the key dhcp relay server struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key dhcp relay server struct
	 */
	public IpcStruct getKeyDhcpRelayServerStruct(final JsonObject requestBody,
			final List<String> uriParameters) {

		// Lower level structure
		/*
		 * ipc_struct KeyDhcpRelay{ key_vrt vrt_key; IPV4 server_addr; };
		 */
		LOG.trace("Start getKeyDhcpRelayServerStruct");
		final IpcStruct keyDhcpRelayServerStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyDhcpRelayServer.getValue());
		IpcStruct keyVtnVrtStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.TWO.ordinal()) {
			keyVtnVrtStruct = getKeyVrtStruct(requestBody,
					uriParameters.subList(0, 2));
		}
		keyDhcpRelayServerStruct.set(VtnServiceIpcConsts.VRTKEY,
				keyVtnVrtStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.SERVER)
				&& ((JsonObject) requestBody.get(VtnServiceJsonConsts.SERVER))
						.has(VtnServiceJsonConsts.IPADDR)) {
			keyDhcpRelayServerStruct
					.set(VtnServiceIpcConsts.SERVERADDR, IpcDataUnitWrapper
							.setIpcInet4AddressValue(((JsonObject) requestBody
									.get(VtnServiceJsonConsts.SERVER)).get(
									VtnServiceJsonConsts.IPADDR).getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.THREE.ordinal()) {
			keyDhcpRelayServerStruct.set(VtnServiceIpcConsts.SERVERADDR,
					IpcDataUnitWrapper.setIpcInet4AddressValue(uriParameters
							.get(2)));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyDhcpRelayServerStruct");
		}
		LOG.info("Key Structure: " + keyDhcpRelayServerStruct.toString());
		LOG.trace("Complete getKeyDhcpRelayServerStruct");

		return keyDhcpRelayServerStruct;
	}

	/**
	 * Gets the key flow list struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key flow list struct
	 */
	public final IpcStruct getKeyFlowListStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		// low level structure
		/*
		 * ipc_struct key_flowlist { UINT8 flowlist_name[32 + 1]; };
		 */
		LOG.trace("Start getKeyFlowListStruct");
		final IpcStruct keyFlowListStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyFlowList.getValue());

		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.FLOWLIST)
				&& ((JsonObject) requestBody.get(VtnServiceJsonConsts.FLOWLIST))
						.has(VtnServiceJsonConsts.FLNAME)) {
			keyFlowListStruct
					.set(VtnServiceJsonConsts.FLOWLISTNAME, IpcDataUnitWrapper
							.setIpcUint8ArrayValue(((JsonObject) requestBody
									.get(VtnServiceJsonConsts.FLOWLIST)).get(
									VtnServiceJsonConsts.FLNAME).getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.ONE.ordinal()) {
			keyFlowListStruct.set(VtnServiceJsonConsts.FLOWLISTNAME,
					IpcDataUnitWrapper.setIpcUint8ArrayValue(uriParameters
							.get(0)));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyFlowListStruct");
		}
		LOG.info("Key Structure: " + keyFlowListStruct.toString());
		LOG.trace("Complete getKeyFlowListStruct");
		return keyFlowListStruct;
	}

	/**
	 * Gets the val flow list struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the val flow list struct
	 */
	public final IpcStruct getValFlowListStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		LOG.trace("Start ValFlowListStruct");
		final IpcStruct valFlowListStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValFlowList.getValue());
		if (requestBody != null) {
			valFlowListStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValFlowlistIndex.UPLL_IDX_IP_TYPE_FL
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
											.ordinal()));
			if ((requestBody.has(VtnServiceJsonConsts.FLOWLIST)
					&& requestBody.getAsJsonObject(
							VtnServiceJsonConsts.FLOWLIST).has(
							VtnServiceJsonConsts.IPVERSION) && requestBody
					.getAsJsonObject(VtnServiceJsonConsts.FLOWLIST)
					.get(VtnServiceJsonConsts.IPVERSION).getAsString()
					.equalsIgnoreCase(VtnServiceJsonConsts.IPV6))) {
				valFlowListStruct
						.set(VtnServiceIpcConsts.IPTYPE,
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.FlowlistIpType.UPLL_FLOWLIST_TYPE_IPV6
												.ordinal()));
				LOG.debug("ip_version:"
						+ requestBody
								.getAsJsonObject(VtnServiceJsonConsts.FLOWLIST)
								.get(VtnServiceJsonConsts.IPVERSION)
								.getAsString());
			} else if (requestBody.has(VtnServiceJsonConsts.FLOWLIST)) {
				valFlowListStruct
						.set(VtnServiceIpcConsts.IPTYPE,
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.FlowlistIpType.UPLL_FLOWLIST_TYPE_IP
												.ordinal()));
			} else if (requestBody.has(VtnServiceJsonConsts.IPVERSION)
					&& requestBody.get(VtnServiceJsonConsts.IPVERSION)
							.getAsString()
							.equalsIgnoreCase(VtnServiceJsonConsts.IPV6)) {
				valFlowListStruct
						.set(VtnServiceIpcConsts.IPTYPE,
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.FlowlistIpType.UPLL_FLOWLIST_TYPE_IPV6
												.ordinal()));
			} else {
				valFlowListStruct
						.set(VtnServiceIpcConsts.IPTYPE,
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.FlowlistIpType.UPLL_FLOWLIST_TYPE_IP
												.ordinal()));
			}
		} else {
			LOG.warning("request body and uri parameters are not correct for valFlowListStruct");
		}

		LOG.info("Value Structure: " + valFlowListStruct.toString());
		LOG.trace("Complete ValFlowListStruct");
		return valFlowListStruct;
	}

	/**
	 * Gets the key flow list entry struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key flow list entry struct
	 */
	public final IpcStruct getKeyFlowListEntryStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		// low level structure
		/*
		 * ipc_struct key_flowlist_entry { key_flowlist flowlist_key; UINT16
		 * sequence_num; };
		 */
		LOG.trace("Start getKeyFlowListEntryStruct");
		final IpcStruct keyFlowListEntryStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyFlowListEntry.getValue());

		IpcStruct keyFlowListStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.ONE.ordinal()) {
			keyFlowListStruct = getKeyFlowListStruct(requestBody,
					uriParameters.subList(0, 1));
		}
		keyFlowListEntryStruct.set(VtnServiceJsonConsts.FLOWLISTKEY,
				keyFlowListStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.FLOWLISTENTRY)
				&& ((JsonObject) requestBody
						.get(VtnServiceJsonConsts.FLOWLISTENTRY))
						.has(VtnServiceJsonConsts.SEQNUM)) {
			keyFlowListEntryStruct.set(VtnServiceJsonConsts.SEQUENCENUM,
					IpcDataUnitWrapper
							.setIpcUint16Value(((JsonObject) requestBody
									.get(VtnServiceJsonConsts.FLOWLISTENTRY))
									.get(VtnServiceJsonConsts.SEQNUM)
									.getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.TWO.ordinal()) {
			keyFlowListEntryStruct.set(VtnServiceJsonConsts.SEQUENCENUM,
					IpcDataUnitWrapper.setIpcUint16Value(uriParameters.get(1)));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyFlowListEntryStruct");
		}
		LOG.info("Key Structure: " + keyFlowListEntryStruct.toString());
		LOG.trace("Complete getKeyFlowListEntryStruct");

		return keyFlowListEntryStruct;
	}

	/**
	 * Gets the val flow list entry struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @return the val flow list entry struct
	 */
	public final IpcStruct getValFlowListEntryStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		LOG.trace("Start getValFlowListEntryStruct");
		final IpcStruct valFlowListEntryStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValFlowListEntry.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.FLOWLISTENTRY)) {
			final JsonObject flowListEntry = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.FLOWLISTENTRY);
			if (flowListEntry.has(VtnServiceJsonConsts.MACDSTADDR)) {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_MAC_DST_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				IpcDataUnitWrapper
						.setMacAddress(
								valFlowListEntryStruct,
								VtnServiceIpcConsts.MACDST,
								flowListEntry.get(
										VtnServiceJsonConsts.MACDSTADDR)
										.getAsString(),
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_MAC_DST_FLE
										.ordinal());
			} else {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_MAC_DST_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (flowListEntry.has(VtnServiceJsonConsts.MACSRCADDR)) {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_MAC_SRC_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				IpcDataUnitWrapper
						.setMacAddress(
								valFlowListEntryStruct,
								VtnServiceIpcConsts.MACSRC,
								flowListEntry.get(
										VtnServiceJsonConsts.MACSRCADDR)
										.getAsString(),
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_MAC_SRC_FLE
										.ordinal());
			} else {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_MAC_SRC_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (flowListEntry.has(VtnServiceJsonConsts.MACETHERTYPE)) {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_MAC_ETH_TYPE_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.MAC_ETH_TYPE,
								IpcDataUnitWrapper
										.setIpcUint16HexaValue(
												flowListEntry
														.get(VtnServiceJsonConsts.MACETHERTYPE)
														.getAsString(),
												valFlowListEntryStruct,
												UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_MAC_ETH_TYPE_FLE
														.ordinal()));
				LOG.debug("macethertype:"
						+ flowListEntry.get(VtnServiceJsonConsts.MACETHERTYPE)
								.getAsString());
			} else {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_MAC_ETH_TYPE_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (flowListEntry.has(VtnServiceJsonConsts.IPDSTADDR)) {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_DST_IP_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.DST_IP,
								IpcDataUnitWrapper
										.setIpcInet4AddressValue(
												flowListEntry
														.get(VtnServiceJsonConsts.IPDSTADDR)
														.getAsString(),
												valFlowListEntryStruct,
												UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_DST_IP_FLE
														.ordinal()));
			} else {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_DST_IP_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (flowListEntry.has(VtnServiceJsonConsts.IPDSTADDRPREFIX)) {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_DST_IP_PREFIX_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.DST_IP_PREFIXLEN,
								IpcDataUnitWrapper
										.setIpcUint8Value(
												flowListEntry
														.get(VtnServiceJsonConsts.IPDSTADDRPREFIX)
														.getAsString(),
												valFlowListEntryStruct,
												UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_DST_IP_PREFIX_FLE
														.ordinal()));
			} else {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_DST_IP_PREFIX_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (flowListEntry.has(VtnServiceJsonConsts.IPSRCADDR)) {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_SRC_IP_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.SRC_IP,
								IpcDataUnitWrapper
										.setIpcInet4AddressValue(
												flowListEntry
														.get(VtnServiceJsonConsts.IPSRCADDR)
														.getAsString(),
												valFlowListEntryStruct,
												UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_SRC_IP_FLE
														.ordinal()));
			} else {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_SRC_IP_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (flowListEntry.has(VtnServiceJsonConsts.IPSRCADDRPREFIX)) {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_SRC_IP_PREFIX_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.SRC_IP_PREFIXLEN,
								IpcDataUnitWrapper
										.setIpcUint8Value(
												flowListEntry
														.get(VtnServiceJsonConsts.IPSRCADDRPREFIX)
														.getAsString(),
												valFlowListEntryStruct,
												UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_SRC_IP_PREFIX_FLE
														.ordinal()));
			} else {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_SRC_IP_PREFIX_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}

			if (flowListEntry.has(VtnServiceJsonConsts.MACVLANPRIORITY)) {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_VLAN_PRIORITY_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VLAN_PRIORITY,
								IpcDataUnitWrapper
										.setIpcUint8Value(
												flowListEntry
														.get(VtnServiceJsonConsts.MACVLANPRIORITY)
														.getAsString(),
												valFlowListEntryStruct,
												UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_VLAN_PRIORITY_FLE
														.ordinal()));
			} else {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_VLAN_PRIORITY_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (flowListEntry.has(VtnServiceJsonConsts.IPV6DSTADDR)) {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_DST_IP_V6_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.DST_IPV6,
								IpcDataUnitWrapper
										.setIpcInet6AddressValue(
												flowListEntry
														.get(VtnServiceJsonConsts.IPV6DSTADDR)
														.getAsString(),
												valFlowListEntryStruct,
												UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_DST_IP_V6_FLE
														.ordinal()));
			} else {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_DST_IP_V6_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (flowListEntry.has(VtnServiceJsonConsts.IPV6DSTADDRPREFIX)) {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_DST_IP_V6_PREFIX_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.DST_IPV6_PREFIXLEN,
								IpcDataUnitWrapper
										.setIpcUint8Value(
												flowListEntry
														.get(VtnServiceJsonConsts.IPV6DSTADDRPREFIX)
														.getAsString(),
												valFlowListEntryStruct,
												UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_DST_IP_V6_PREFIX_FLE
														.ordinal()));
			} else {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_DST_IP_V6_PREFIX_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (flowListEntry.has(VtnServiceJsonConsts.IPV6SRCADDR)) {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_SRC_IP_V6_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.SRC_IPV6,
								IpcDataUnitWrapper
										.setIpcInet6AddressValue(
												flowListEntry
														.get(VtnServiceJsonConsts.IPV6SRCADDR)
														.getAsString(),
												valFlowListEntryStruct,
												UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_SRC_IP_V6_FLE
														.ordinal()));
				LOG.debug("ipv6srcaddr:"
						+ flowListEntry.get(VtnServiceJsonConsts.IPV6SRCADDR)
								.getAsString());
			} else {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_SRC_IP_V6_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (flowListEntry.has(VtnServiceJsonConsts.IPV6SRCADDRPREFIX)) {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_SRC_IP_V6_PREFIX_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.SRC_IPV6_PREFIXLEN,
								IpcDataUnitWrapper
										.setIpcUint8Value(
												flowListEntry
														.get(VtnServiceJsonConsts.IPV6SRCADDRPREFIX)
														.getAsString(),
												valFlowListEntryStruct,
												UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_SRC_IP_V6_PREFIX_FLE
														.ordinal()));
			} else {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_SRC_IP_V6_PREFIX_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (flowListEntry.has(VtnServiceJsonConsts.IPPROTO)) {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_IP_PROTOCOL_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.IP_PROTO,
								IpcDataUnitWrapper
										.setIpcUint8Value(
												flowListEntry
														.get(VtnServiceJsonConsts.IPPROTO)
														.getAsString(),
												valFlowListEntryStruct,
												UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_IP_PROTOCOL_FLE
														.ordinal()));
			} else {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_IP_PROTOCOL_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (flowListEntry.has(VtnServiceJsonConsts.IPDSCP)) {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_IP_DSCP_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.IP_DSCP,
								IpcDataUnitWrapper
										.setIpcUint8Value(
												flowListEntry
														.get(VtnServiceJsonConsts.IPDSCP)
														.getAsString(),
												valFlowListEntryStruct,
												UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_IP_DSCP_FLE
														.ordinal()));
			} else {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_IP_DSCP_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (flowListEntry.has(VtnServiceJsonConsts.L4DSTPORT)) {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_L4_DST_PORT_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.L4_DST_PORT,
								IpcDataUnitWrapper
										.setIpcUint16Value(
												flowListEntry
														.get(VtnServiceJsonConsts.L4DSTPORT)
														.getAsString(),
												valFlowListEntryStruct,
												UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_L4_DST_PORT_FLE
														.ordinal()));
			} else {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_L4_DST_PORT_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (flowListEntry.has(VtnServiceJsonConsts.L4DSTENDPORT)) {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_L4_DST_PORT_ENDPT_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.L4_DST_PORT_ENDPT,
								IpcDataUnitWrapper
										.setIpcUint16Value(
												flowListEntry
														.get(VtnServiceJsonConsts.L4DSTENDPORT)
														.getAsString(),
												valFlowListEntryStruct,
												UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_L4_DST_PORT_ENDPT_FLE
														.ordinal()));
			} else {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_L4_DST_PORT_ENDPT_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (flowListEntry.has(VtnServiceJsonConsts.L4SRCPORT)) {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_L4_SRC_PORT_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.L4_SRC_PORT,
								IpcDataUnitWrapper
										.setIpcUint16Value(
												flowListEntry
														.get(VtnServiceJsonConsts.L4SRCPORT)
														.getAsString(),
												valFlowListEntryStruct,
												UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_L4_SRC_PORT_FLE
														.ordinal()));
			} else {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_L4_SRC_PORT_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (flowListEntry.has(VtnServiceJsonConsts.L4SRCENDPORT)) {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_L4_SRC_PORT_ENDPT_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.L4_SRC_PORT_ENDPT,
								IpcDataUnitWrapper
										.setIpcUint16Value(
												flowListEntry
														.get(VtnServiceJsonConsts.L4SRCENDPORT)
														.getAsString(),
												valFlowListEntryStruct,
												UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_L4_SRC_PORT_ENDPT_FLE
														.ordinal()));
			} else {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_L4_SRC_PORT_ENDPT_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (flowListEntry.has(VtnServiceJsonConsts.ICMPTYPENUM)) {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_ICMP_TYPE_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.ICMP_TYPE,
								IpcDataUnitWrapper
										.setIpcUint8Value(
												flowListEntry
														.get(VtnServiceJsonConsts.ICMPTYPENUM)
														.getAsString(),
												valFlowListEntryStruct,
												UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_ICMP_TYPE_FLE
														.ordinal()));
			} else {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_ICMP_TYPE_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (flowListEntry.has(VtnServiceJsonConsts.ICMPCODENUM)) {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_ICMP_CODE_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.ICMP_CODE,
								IpcDataUnitWrapper
										.setIpcUint8Value(
												flowListEntry
														.get(VtnServiceJsonConsts.ICMPCODENUM)
														.getAsString(),
												valFlowListEntryStruct,
												UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_ICMP_CODE_FLE
														.ordinal()));
			} else {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_ICMP_CODE_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (flowListEntry.has(VtnServiceJsonConsts.IPV6ICMPTYPENUM)) {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_ICMP_V6_TYPE_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.ICMPV6_TYPE,
								IpcDataUnitWrapper
										.setIpcUint8Value(
												flowListEntry
														.get(VtnServiceJsonConsts.IPV6ICMPTYPENUM)
														.getAsString(),
												valFlowListEntryStruct,
												UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_ICMP_V6_TYPE_FLE
														.ordinal()));
			} else {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_ICMP_V6_TYPE_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (flowListEntry.has(VtnServiceJsonConsts.IPV6ICMPCODENUM)) {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_ICMP_V6_CODE_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.ICMPV6_CODE,
								IpcDataUnitWrapper
										.setIpcUint8Value(
												flowListEntry
														.get(VtnServiceJsonConsts.IPV6ICMPCODENUM)
														.getAsString(),
												valFlowListEntryStruct,
												UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_ICMP_V6_CODE_FLE
														.ordinal()));
			} else {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_ICMP_V6_CODE_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
		} else {
			LOG.warning("request body and uri parameters are not correct for getValFlowListEntryStruct");
		}
		LOG.info("Value Structure: " + valFlowListEntryStruct.toString());
		LOG.trace("Complete getValFlowListEntryStruct");
		return valFlowListEntryStruct;
	}

	/**
	 * Gets the key VTunnel if struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the URI parameters
	 * @return the key v tunnel if struct
	 */
	public final IpcStruct getKeyVtunnelIfStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		LOG.trace("Start getKeyVtunnelIfStruct");
		final IpcStruct KeyVTunnelIfStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyVtunnelIf.getValue());
		IpcStruct keyVtunnelStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.TWO.ordinal()) {
			keyVtunnelStruct = getKeyVtunnelStruct(requestBody,
					uriParameters.subList(0, 2));
		}
		KeyVTunnelIfStruct.set(VtnServiceIpcConsts.VTUNNEL_KEY,
				keyVtunnelStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.INTERFACE)
				&& ((JsonObject) requestBody
						.get(VtnServiceJsonConsts.INTERFACE))
						.has(VtnServiceJsonConsts.IFNAME)) {
			KeyVTunnelIfStruct
					.set(VtnServiceJsonConsts.IFNAME, IpcDataUnitWrapper
							.setIpcUint8ArrayValue(((JsonObject) requestBody
									.get(VtnServiceJsonConsts.INTERFACE)).get(
									VtnServiceJsonConsts.IFNAME).getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.THREE.ordinal()) {
			KeyVTunnelIfStruct.set(VtnServiceJsonConsts.IFNAME,
					IpcDataUnitWrapper.setIpcUint8ArrayValue(uriParameters
							.get(2)));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVtunnelIfStruct");
		}
		LOG.info("Key Structure: " + KeyVTunnelIfStruct.toString());
		LOG.trace("Complete getKeyVtunnelIfStruct");

		return KeyVTunnelIfStruct;
	}

	/**
	 * Gets the val v tunnel if struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the val v tunnel if struct
	 */
	public final IpcStruct getValVtunnelIfStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		LOG.trace("Start getValVtunnelIfStruct");
		final IpcStruct ValVTunnelIfStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValVtunnelIf.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.INTERFACE)) {
			if (requestBody.getAsJsonObject(VtnServiceJsonConsts.INTERFACE)
					.has(VtnServiceJsonConsts.DESCRIPTION)) {
				ValVTunnelIfStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtunnelIfIndex.UPLL_IDX_DESC_VTNL_IF
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				ValVTunnelIfStruct.set(VtnServiceJsonConsts.DESCRIPTION,
						IpcDataUnitWrapper
								.setIpcUint8ArrayValue(requestBody
										.getAsJsonObject(
												VtnServiceJsonConsts.INTERFACE)
										.get(VtnServiceJsonConsts.DESCRIPTION)
										.getAsString()));
			} else {
				ValVTunnelIfStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtunnelIfIndex.UPLL_IDX_DESC_VTNL_IF
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (requestBody.getAsJsonObject(VtnServiceJsonConsts.INTERFACE)
					.has(VtnServiceJsonConsts.ADMINSTATUS)) {
				ValVTunnelIfStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtunnelIfIndex.UPLL_IDX_ADMIN_ST_VTNL_IF
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				if (requestBody.getAsJsonObject(VtnServiceJsonConsts.INTERFACE)
						.get(VtnServiceJsonConsts.ADMINSTATUS).getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.ENABLE)) {
					ValVTunnelIfStruct
							.set(VtnServiceJsonConsts.ADMIN_STATUS,
									IpcDataUnitWrapper
											.setIpcUint8Value(
													UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_ENABLE
															.getValue(),
													ValVTunnelIfStruct,
													UncStructIndexEnum.ValVtunnelIfIndex.UPLL_IDX_ADMIN_ST_VTNL_IF
															.ordinal()));
				} else if (requestBody
						.getAsJsonObject(VtnServiceJsonConsts.INTERFACE)
						.get(VtnServiceJsonConsts.ADMINSTATUS).getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.DISABLE)) {
					ValVTunnelIfStruct
							.set(VtnServiceJsonConsts.ADMIN_STATUS,
									IpcDataUnitWrapper
											.setIpcUint8Value(
													UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_DISABLE
															.getValue(),
													ValVTunnelIfStruct,
													UncStructIndexEnum.ValVtunnelIfIndex.UPLL_IDX_ADMIN_ST_VTNL_IF
															.ordinal()));
				} else {
					ValVTunnelIfStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVtunnelIfIndex.UPLL_IDX_ADMIN_ST_VTNL_IF
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
													.ordinal()));

				}
				LOG.debug("adminstatus:"
						+ requestBody
								.getAsJsonObject(VtnServiceJsonConsts.INTERFACE)
								.get(VtnServiceJsonConsts.ADMINSTATUS)
								.getAsString());
			} else {
				ValVTunnelIfStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtunnelIfIndex.UPLL_IDX_ADMIN_ST_VTNL_IF
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			ValVTunnelIfStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVtunnelIfIndex.UPLL_IDX_PORT_MAP_VTNL_IF
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
		} else if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.PORTMAP)) {
			IpcStruct valPortMapStruct = null;
			ValVTunnelIfStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVtunnelIfIndex.UPLL_IDX_ADMIN_ST_VTNL_IF
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			ValVTunnelIfStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVtunnelIfIndex.UPLL_IDX_DESC_VTNL_IF
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			ValVTunnelIfStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVtunnelIfIndex.UPLL_IDX_PORT_MAP_VTNL_IF
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
											.ordinal()));
			valPortMapStruct = getValPortMapStruct(requestBody, uriParameters);
			ValVTunnelIfStruct.set(VtnServiceIpcConsts.PORTMAP,
					valPortMapStruct);
		} else if (requestBody == null) {
			final IpcStruct valPortMapStruct = IpcDataUnitWrapper
					.setIpcStructValue(UncStructEnum.ValPortMap.getValue());
			ValVTunnelIfStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVtunnelIfIndex.UPLL_IDX_PORT_MAP_VTNL_IF
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
											.ordinal()));
			ValVTunnelIfStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVtunnelIfIndex.UPLL_IDX_DESC_VTNL_IF
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			ValVTunnelIfStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVtunnelIfIndex.UPLL_IDX_ADMIN_ST_VTNL_IF
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			valPortMapStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_LOGICAL_PORT_ID_PM
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
											.ordinal()));
			valPortMapStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_VLAN_ID_PM
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
											.ordinal()));
			valPortMapStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_TAGGED_PM
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
											.ordinal()));
			ValVTunnelIfStruct.set(VtnServiceIpcConsts.PORTMAP,
					valPortMapStruct);
		} else {
			LOG.warning("request body and uri parameters are not correct for getValVtunnelIfStruct");
		}
		LOG.info("Value Structure: " + ValVTunnelIfStruct.toString());
		LOG.trace("Complete getValVtunnelIfStruct");

		return ValVTunnelIfStruct;
	}

	/**
	 * Gets the key vlink struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key vlink struct
	 */
	public final IpcStruct getKeyVlinkStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		LOG.trace("Start getKeyVlinkStruct");
		final IpcStruct KeyVlinkStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyVlink.getValue());
		IpcStruct keyVtnStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.ONE.ordinal()) {
			keyVtnStruct = getKeyVtnStruct(requestBody,
					uriParameters.subList(0, 1));
		}
		KeyVlinkStruct.set(VtnServiceIpcConsts.VTNKEY, keyVtnStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.VLINK)
				&& ((JsonObject) requestBody.get(VtnServiceJsonConsts.VLINK))
						.has(VtnServiceJsonConsts.VLKNAME)) {
			KeyVlinkStruct
					.set(VtnServiceIpcConsts.VLINK_NAME,
							IpcDataUnitWrapper
									.setIpcUint8ArrayValue(((JsonObject) requestBody
											.get(VtnServiceJsonConsts.VLINK))
											.get(VtnServiceJsonConsts.VLKNAME)
											.getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.TWO.ordinal()) {
			KeyVlinkStruct.set(VtnServiceIpcConsts.VLINK_NAME,
					IpcDataUnitWrapper.setIpcUint8ArrayValue(uriParameters
							.get(1)));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVlinkStruct");
		}
		LOG.info("Key Structure: " + KeyVlinkStruct.toString());
		LOG.trace("Complete getKeyVlinkStruct");
		return KeyVlinkStruct;
	}

	/**
	 * Gets the val vlink struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the val vlink struct
	 */
	public final IpcStruct getValVlinkStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		LOG.trace("Start getValVlinkStruct");
		final IpcStruct ValVlinkStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValVlink.getValue());
		if (requestBody != null && requestBody.has(VtnServiceJsonConsts.VLINK)) {
			final JsonObject vLink = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VLINK);
			if (vLink.has(VtnServiceJsonConsts.ADMINSTATUS)) {
				ValVlinkStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_ADMIN_STATUS_VLNK
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				if (vLink.get(VtnServiceJsonConsts.ADMINSTATUS).getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.ENABLE)) {
					ValVlinkStruct
							.set(VtnServiceJsonConsts.ADMIN_STATUS,
									IpcDataUnitWrapper
											.setIpcUint8Value(
													UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_ENABLE
															.getValue(),
													ValVlinkStruct,
													UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_ADMIN_STATUS_VLNK
															.ordinal()));
				} else {
					ValVlinkStruct
							.set(VtnServiceJsonConsts.ADMIN_STATUS,
									IpcDataUnitWrapper
											.setIpcUint8Value(
													UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_DISABLE
															.getValue(),
													ValVlinkStruct,
													UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_ADMIN_STATUS_VLNK
															.ordinal()));
				}
				LOG.debug("adminstatus:"
						+ vLink.get(VtnServiceJsonConsts.ADMINSTATUS)
								.getAsString());
			} else {
				ValVlinkStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_ADMIN_STATUS_VLNK
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (vLink.has(VtnServiceJsonConsts.VNODE1NAME)) {
				ValVlinkStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_VNODE1_NAME_VLNK
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				ValVlinkStruct
						.set(VtnServiceJsonConsts.VNODE1NAME,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vLink.get(
														VtnServiceJsonConsts.VNODE1NAME)
														.getAsString(),
												ValVlinkStruct,
												UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_VNODE1_NAME_VLNK
														.ordinal()));
			} else {
				ValVlinkStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_VNODE1_NAME_VLNK
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (vLink.has(VtnServiceJsonConsts.IF1NAME)) {
				ValVlinkStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_VNODE1_IF_NAME_VLNK
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				ValVlinkStruct
						.set(VtnServiceIpcConsts.VNODE1IFNAME,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vLink.get(
														VtnServiceJsonConsts.IF1NAME)
														.getAsString(),
												ValVlinkStruct,
												UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_VNODE1_IF_NAME_VLNK
														.ordinal()));
			} else {
				ValVlinkStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_VNODE1_IF_NAME_VLNK
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (vLink.has(VtnServiceJsonConsts.VNODE2NAME)) {
				ValVlinkStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_VNODE2_NAME_VLNK
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				ValVlinkStruct
						.set(VtnServiceJsonConsts.VNODE2NAME,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vLink.get(
														VtnServiceJsonConsts.VNODE2NAME)
														.getAsString(),
												ValVlinkStruct,
												UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_VNODE2_NAME_VLNK
														.ordinal()));
			} else {
				ValVlinkStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_VNODE2_NAME_VLNK
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (vLink.has(VtnServiceJsonConsts.IF2NAME)) {
				ValVlinkStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_VNODE2_IF_NAME_VLNK
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				ValVlinkStruct
						.set(VtnServiceIpcConsts.VNODE2IFNAME,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vLink.get(
														VtnServiceJsonConsts.IF2NAME)
														.getAsString(),
												ValVlinkStruct,
												UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_VNODE2_IF_NAME_VLNK
														.ordinal()));
			} else {
				ValVlinkStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_VNODE2_IF_NAME_VLNK
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (vLink.has(VtnServiceJsonConsts.BOUNDARYMAP)) {
				final JsonObject boundaryMap = vLink
						.getAsJsonObject(VtnServiceJsonConsts.BOUNDARYMAP);
				if (boundaryMap.has(VtnServiceJsonConsts.BOUNDARYID)) {
					ValVlinkStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_BOUNDARY_NAME_VLNK
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
													.ordinal()));
					ValVlinkStruct
							.set(VtnServiceIpcConsts.BOUNDARY_NAME,
									IpcDataUnitWrapper
											.setIpcUint8ArrayValue(
													boundaryMap
															.get(VtnServiceJsonConsts.BOUNDARYID)
															.getAsString(),
													ValVlinkStruct,
													UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_BOUNDARY_NAME_VLNK
															.ordinal()));
				} else {
					ValVlinkStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_BOUNDARY_NAME_VLNK
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
													.ordinal()));
				}

				if (boundaryMap.has(VtnServiceJsonConsts.VLANID)) {
					// set label_type
					ValVlinkStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_LABEL_TYPE_VLNK
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
													.ordinal()));
					ValVlinkStruct
							.set(VtnServiceIpcConsts.LABEL_TYPE,
									IpcDataUnitWrapper
											.setIpcUint8Value(
													ValLabelType.UPLL_LABEL_TYPE_VLAN.getValue(),
													ValVlinkStruct,
													UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_LABEL_TYPE_VLNK
															.ordinal()));

					// set label
					LOG.debug("Valid VLAN ID Case");
					ValVlinkStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_LABEL_VLINK
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
													.ordinal()));
					ValVlinkStruct
							.set(VtnServiceIpcConsts.LABEL,
									IpcDataUnitWrapper
											.setIpcUint32Value(
													boundaryMap
															.get(VtnServiceJsonConsts.VLANID)
															.getAsString(),
													ValVlinkStruct,
													UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_LABEL_VLINK
															.ordinal()));
				} else if (boundaryMap.has(VtnServiceJsonConsts.NO_VLAN_ID)) {
					// set label_type
					ValVlinkStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_LABEL_TYPE_VLNK
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
													.ordinal()));
					ValVlinkStruct
							.set(VtnServiceIpcConsts.LABEL_TYPE,
									IpcDataUnitWrapper
											.setIpcUint8Value(
													ValLabelType.UPLL_LABEL_TYPE_VLAN.getValue(),
													ValVlinkStruct,
													UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_LABEL_TYPE_VLNK
															.ordinal()));

					LOG.debug("Valid NO VLAN ID Case");
					ValVlinkStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_LABEL_VLINK
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
													.ordinal()));
					ValVlinkStruct
							.set(VtnServiceIpcConsts.LABEL,
									IpcDataUnitWrapper
											.setIpcUint32Value(
													VtnServiceIpcConsts.NO_VLAN_ID,
													ValVlinkStruct,
													UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_LABEL_VLINK
															.ordinal()));
				} else {
					ValVlinkStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_LABEL_TYPE_VLNK
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
													.ordinal()));
					LOG.debug("InValid VLAN ID Case");
					ValVlinkStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_LABEL_VLINK
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
													.ordinal()));
				}
				LOG.debug("VLAN ID Valid Bit : "
						+ ValVlinkStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_LABEL_VLINK
												.ordinal()));
			}
			if (vLink.has(VtnServiceJsonConsts.DESCRIPTION)) {
				ValVlinkStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_DESCRIPTION_VLNK
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				ValVlinkStruct
						.set(VtnServiceIpcConsts.DESCRIPTION,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vLink.get(
														VtnServiceJsonConsts.DESCRIPTION)
														.getAsString(),
												ValVlinkStruct,
												UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_DESCRIPTION_VLNK
														.ordinal()));
			} else {
				ValVlinkStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_DESCRIPTION_VLNK
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
		} else {
			LOG.warning("request body and uri parameters are not correct for getValVlinkStruct");
		}
		LOG.info("Value Structure: " + ValVlinkStruct.toString());
		LOG.trace("Complete getValVlinkStruct");
		return ValVlinkStruct;
	}

	/**
	 * Gets the key vtep struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key vtep struct
	 */
	public final IpcStruct getKeyVtepStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		LOG.trace("Start getKeyVtepStruct");
		final IpcStruct KeyVtepStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyVtep.getValue());
		IpcStruct keyVtnStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.ONE.ordinal()) {
			keyVtnStruct = getKeyVtnStruct(requestBody,
					uriParameters.subList(0, 1));
		}
		KeyVtepStruct.set(VtnServiceIpcConsts.VTNKEY, keyVtnStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.VTEP)
				&& ((JsonObject) requestBody.get(VtnServiceJsonConsts.VTEP))
						.has(VtnServiceJsonConsts.VTEPNAME)) {
			KeyVtepStruct.set(VtnServiceJsonConsts.VTEPNAME, IpcDataUnitWrapper
					.setIpcUint8ArrayValue(((JsonObject) requestBody
							.get(VtnServiceJsonConsts.VTEP)).get(
							VtnServiceJsonConsts.VTEPNAME).getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.TWO.ordinal()) {
			KeyVtepStruct.set(VtnServiceJsonConsts.VTEPNAME, IpcDataUnitWrapper
					.setIpcUint8ArrayValue(uriParameters.get(1)));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVtepStruct");
		}
		LOG.info("Key Structure: " + KeyVtepStruct.toString());
		LOG.trace("Complete getKeyVtepStruct");
		return KeyVtepStruct;
	}

	/**
	 * Gets the val VTEP struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the Val VTEP struct
	 */
	public final IpcStruct getValVtepStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		/*
		 * ipc_struct val_vtep { UINT8 valid[23]; UINT8 cs_row_status; UINT8
		 * cs_attr[23]; UINT8 description[128]; UINT8 controller_id[32]; UINT8
		 * domain_id[32] };
		 */
		LOG.trace("Start getValVtepStruct");
		final IpcStruct ValVtepStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValVtep.getValue());
		if (requestBody != null && requestBody.has(VtnServiceJsonConsts.VTEP)) {
			final JsonObject vtep = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VTEP);
			if (vtep.has(VtnServiceJsonConsts.DESCRIPTION)) {
				ValVtepStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtepVndex.UPLL_IDX_DESC_VTEP
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				ValVtepStruct
						.set(VtnServiceJsonConsts.DESCRIPTION,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vtep.get(
														VtnServiceJsonConsts.DESCRIPTION)
														.getAsString(),
												ValVtepStruct,
												UncStructIndexEnum.ValVtepVndex.UPLL_IDX_DESC_VTEP
														.ordinal()));
			} else {
				ValVtepStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtepVndex.UPLL_IDX_DESC_VTEP
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (vtep.has(VtnServiceJsonConsts.CONTROLLERID)) {
				ValVtepStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtepVndex.UPLL_IDX_CONTROLLER_ID_VTEP
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				ValVtepStruct
						.set(VtnServiceJsonConsts.CONTROLLERID,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vtep.get(
														VtnServiceJsonConsts.CONTROLLERID)
														.getAsString(),
												ValVtepStruct,
												UncStructIndexEnum.ValVtepVndex.UPLL_IDX_CONTROLLER_ID_VTEP
														.ordinal()));
			} else {
				ValVtepStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtepVndex.UPLL_IDX_CONTROLLER_ID_VTEP
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (vtep.has(VtnServiceJsonConsts.DOMAINID)) {
				ValVtepStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtepVndex.UPLL_IDX_DOMAIN_ID_VTEP
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				ValVtepStruct
						.set(VtnServiceJsonConsts.DOMAINID,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vtep.get(
														VtnServiceJsonConsts.DOMAINID)
														.getAsString(),
												ValVtepStruct,
												UncStructIndexEnum.ValVtepVndex.UPLL_IDX_DOMAIN_ID_VTEP
														.ordinal()));
			} else {
				ValVtepStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtepVndex.UPLL_IDX_DOMAIN_ID_VTEP
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
		} else {
			LOG.warning("request body and uri parameters are not correct for getValVtepStruct");
		}
		LOG.info("Value Structure: " + ValVtepStruct.toString());
		LOG.trace("Complete getValVtepStruct");
		return ValVtepStruct;
	}

	/**
	 * Gets the key vtep if struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key vtep if struct
	 */
	public final IpcStruct getKeyVtepIfStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		LOG.trace("Start getKeyVtepIfStruct");
		final IpcStruct KeyVtepIfStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyVtepIf.getValue());
		IpcStruct keyVTepStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.TWO.ordinal()) {
			keyVTepStruct = getKeyVtepStruct(requestBody,
					uriParameters.subList(0, 2));
		}
		KeyVtepIfStruct.set(VtnServiceIpcConsts.VTEP_KEY, keyVTepStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.INTERFACE)
				&& ((JsonObject) requestBody
						.get(VtnServiceJsonConsts.INTERFACE))
						.has(VtnServiceJsonConsts.VIFNAME)) {
			KeyVtepIfStruct
					.set(VtnServiceJsonConsts.VIFNAME,
							IpcDataUnitWrapper
									.setIpcUint8ArrayValue(((JsonObject) requestBody
											.get(VtnServiceJsonConsts.INTERFACE))
											.get(VtnServiceJsonConsts.VIFNAME)
											.getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.THREE.ordinal()) {
			KeyVtepIfStruct.set(VtnServiceJsonConsts.VIFNAME,
					IpcDataUnitWrapper.setIpcUint8ArrayValue(uriParameters
							.get(2)));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVtepIfStruct");
		}
		LOG.info("Key Structure: " + KeyVtepIfStruct.toString());
		LOG.trace("Complete getKeyVtepIfStruct");
		return KeyVtepIfStruct;
	}

	/**
	 * Gets the val VTEP Interface struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the Val VTEP Interface struct
	 */
	public final IpcStruct getValVtepIfStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		LOG.trace("Start getValVtepIfStruct");
		final IpcStruct ValVtepIfStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValVtepIf.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.INTERFACE)) {
			if (requestBody.getAsJsonObject(VtnServiceJsonConsts.INTERFACE)
					.has(VtnServiceJsonConsts.DESCRIPTION)) {
				ValVtepIfStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtepIfIndex.UPLL_IDX_DESC_VTEPI
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				ValVtepIfStruct.set(VtnServiceJsonConsts.DESCRIPTION,
						IpcDataUnitWrapper
								.setIpcUint8ArrayValue(requestBody
										.getAsJsonObject(
												VtnServiceJsonConsts.INTERFACE)
										.get(VtnServiceJsonConsts.DESCRIPTION)
										.getAsString()));
			} else {
				ValVtepIfStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtepIfIndex.UPLL_IDX_DESC_VTEPI
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}

			if (requestBody.getAsJsonObject(VtnServiceJsonConsts.INTERFACE)
					.has(VtnServiceJsonConsts.ADMINSTATUS)) {
				ValVtepIfStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtepIfIndex.UPLL_IDX_ADMIN_ST_VTEPI
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				if (requestBody.getAsJsonObject(VtnServiceJsonConsts.INTERFACE)
						.get(VtnServiceJsonConsts.ADMINSTATUS).getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.ENABLE)) {
					ValVtepIfStruct
							.set(VtnServiceJsonConsts.ADMIN_STATUS,
									IpcDataUnitWrapper
											.setIpcUint8Value(
													UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_ENABLE
															.getValue(),
													ValVtepIfStruct,
													UncStructIndexEnum.ValVtepIfIndex.UPLL_IDX_ADMIN_ST_VTEPI
															.ordinal()));
				} else if (requestBody
						.getAsJsonObject(VtnServiceJsonConsts.INTERFACE)
						.get(VtnServiceJsonConsts.ADMINSTATUS).getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.DISABLE)) {
					ValVtepIfStruct
							.set(VtnServiceJsonConsts.ADMIN_STATUS,
									IpcDataUnitWrapper
											.setIpcUint8Value(
													UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_DISABLE
															.getValue(),
													ValVtepIfStruct,
													UncStructIndexEnum.ValVtepIfIndex.UPLL_IDX_ADMIN_ST_VTEPI
															.ordinal()));
				} else {
					ValVtepIfStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVtepIfIndex.UPLL_IDX_ADMIN_ST_VTEPI
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
													.ordinal()));
				}
				LOG.debug("adminstatus:"
						+ requestBody
								.getAsJsonObject(VtnServiceJsonConsts.INTERFACE)
								.get(VtnServiceJsonConsts.ADMINSTATUS)
								.getAsString());
			} else {
				ValVtepIfStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtepIfIndex.UPLL_IDX_ADMIN_ST_VTEPI
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			ValVtepIfStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVtepIfIndex.UPLL_IDX_PORT_MAP_VTEPI
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
		} else if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.PORTMAP)) {
			IpcStruct valPortMapStruct = null;
			ValVtepIfStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVtepIfIndex.UPLL_IDX_ADMIN_ST_VTEPI
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			ValVtepIfStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVtepIfIndex.UPLL_IDX_DESC_VTEPI
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			ValVtepIfStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVtepIfIndex.UPLL_IDX_PORT_MAP_VTEPI
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
											.ordinal()));
			valPortMapStruct = getValPortMapStruct(requestBody, uriParameters);
			ValVtepIfStruct.set(VtnServiceIpcConsts.PORTMAP, valPortMapStruct);
		} else if (requestBody == null) {
			final IpcStruct valPortMapStruct = IpcDataUnitWrapper
					.setIpcStructValue(UncStructEnum.ValPortMap.getValue());
			ValVtepIfStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVtepIfIndex.UPLL_IDX_PORT_MAP_VTEPI
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
											.ordinal()));
			ValVtepIfStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVtepIfIndex.UPLL_IDX_DESC_VTEPI
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			ValVtepIfStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVtepIfIndex.UPLL_IDX_ADMIN_ST_VTEPI
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			valPortMapStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_LOGICAL_PORT_ID_PM
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
											.ordinal()));
			valPortMapStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_VLAN_ID_PM
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
											.ordinal()));
			valPortMapStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_TAGGED_PM
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
											.ordinal()));
			ValVtepIfStruct.set(VtnServiceIpcConsts.PORTMAP, valPortMapStruct);
		} else {
			LOG.warning("request body and uri parameters are not correct for getValVtepIfStruct");
		}
		LOG.info("Value Structure: " + ValVtepIfStruct.toString());
		LOG.trace("Complete getValVtepIfStruct");

		return ValVtepIfStruct;
	}

	/**
	 * Gets the key vbr if flow filter entry struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key vbr if flow filter entry struct
	 */
	public final IpcStruct getKeyVbrIfFlowFilterEntryStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		LOG.trace("Start getKeyVbrIfFlowFilterEntryStruct");
		final IpcStruct KeyVbrIfFlowFilterEntryStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyVbrIfFlowFilterEntry
						.getValue());
		IpcStruct keyVbrIfFlowFilterStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.FOUR.ordinal()) {
			keyVbrIfFlowFilterStruct = getKeyVbrIfFlowFilterStruct(requestBody,
					uriParameters.subList(0, 4));
		}
		KeyVbrIfFlowFilterEntryStruct.set(VtnServiceIpcConsts.FLOWFILTERKEY,
				keyVbrIfFlowFilterStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.FLOWFILTERENTRY)
				&& ((JsonObject) requestBody
						.get(VtnServiceJsonConsts.FLOWFILTERENTRY))
						.has(VtnServiceJsonConsts.SEQNUM)) {
			KeyVbrIfFlowFilterEntryStruct.set(VtnServiceJsonConsts.SEQUENCENUM,
					IpcDataUnitWrapper
							.setIpcUint16Value(((JsonObject) requestBody
									.get(VtnServiceJsonConsts.FLOWFILTERENTRY))
									.get(VtnServiceJsonConsts.SEQNUM)
									.getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.FIVE.ordinal()) {
			KeyVbrIfFlowFilterEntryStruct.set(VtnServiceJsonConsts.SEQUENCENUM,
					IpcDataUnitWrapper.setIpcUint16Value(uriParameters.get(4)));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVbrIfFlowFilterEntryStruct");
		}
		LOG.info("Key Structure: " + KeyVbrIfFlowFilterEntryStruct.toString());
		LOG.trace("Complete getKeyVbrIfFlowFilterEntryStruct");
		return KeyVbrIfFlowFilterEntryStruct;
	}
	
	/**
	 * Gets the key unified network struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key unified network struct
	 */
	public final IpcStruct getKeyUnifiedNetworkStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		
		LOG.trace("Start getKeyUnifiedNetworkStruct");
		final IpcStruct keyUnifiedNwStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyUnifiedNetwork.getValue());
		
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.UNIFIED_NW)
				&& ((JsonObject) requestBody.get(VtnServiceJsonConsts.UNIFIED_NW))
						.has(VtnServiceJsonConsts.UNIFIED_NW_NAME)) {
			keyUnifiedNwStruct.set(VtnServiceIpcConsts.UNIFIED_NW_ID,
					IpcDataUnitWrapper
							.setIpcUint8ArrayValue(((JsonObject) requestBody
									.get(VtnServiceJsonConsts.UNIFIED_NW))
									.get(VtnServiceJsonConsts.UNIFIED_NW_NAME)
									.getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.ONE.ordinal()) {
			keyUnifiedNwStruct.set(VtnServiceIpcConsts.UNIFIED_NW_ID,
					IpcDataUnitWrapper.setIpcUint8ArrayValue(uriParameters.get(0)));
		} else {
			LOG.debug("request body and uri parameters are not correct for getKeyUnifiedNetworkStruct");
		}
		
		LOG.info("Key Structure: " + keyUnifiedNwStruct.toString());
		LOG.trace("Complete getKeyUnifiedNetworkStruct");
		return keyUnifiedNwStruct;
	}
	
	/**
	 * Gets the val unified network struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the val unified network struct
	 */
	public final IpcStruct getValUnifiedNetworkStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		LOG.trace("Start getValUnifiedNetworkStruct");
		final IpcStruct valUnifiedNwStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValUnifiedNetwork.getValue());
		
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.UNIFIED_NW)
				&& requestBody.getAsJsonObject(VtnServiceJsonConsts.UNIFIED_NW)
				   .has(VtnServiceJsonConsts.ROUTING_TYPE)) {
			
			valUnifiedNwStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValUnifiedNwIndex.UPLL_IDX_ROUTING_TYPE_UNW
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
											.ordinal()));
			
			valUnifiedNwStruct.set(VtnServiceIpcConsts.ROUTING_TYPE,
					IpcDataUnitWrapper.setIpcUint8Value(
							Integer.toString(ValUnifiedNwRoutingType.UPLL_ROUTING_TYPE_QINQ_TO_QINQ.ordinal()),
							valUnifiedNwStruct,
							UncStructIndexEnum.ValUnifiedNwIndex.UPLL_IDX_ROUTING_TYPE_UNW
									.ordinal()));
			
			valUnifiedNwStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValUnifiedNwIndex.UPLL_IDX_IS_DEFAULT_UNW
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
		} else {
			LOG.debug("request body and uri parameters are not correct for getValUnifiedNetworkStruct");
		}
		LOG.info("Value Structure: " + valUnifiedNwStruct.toString());
		LOG.trace("Complete getValUnifiedNetworkStruct");
		return valUnifiedNwStruct;
	}
	
	/**
	 * Gets the key vbr port map struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key vbr port map struct
	 */
	public final IpcStruct getKeyVbrPortmapStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		LOG.trace("Start getKeyVbrPortmapStruct");
		final IpcStruct keyVbrPortmapStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyVbrPortmap.getValue());
		IpcStruct keyVbrKeyStruct = null;
		
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.TWO.ordinal()) {
			keyVbrKeyStruct = getKeyVbrStruct(requestBody,
					uriParameters.subList(0, 2));
		}
		
		keyVbrPortmapStruct.set(VtnServiceIpcConsts.VBRKEY, keyVbrKeyStruct);
		
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.PORTMAP)
				&& ((JsonObject) requestBody.get(VtnServiceJsonConsts.PORTMAP))
						.has(VtnServiceJsonConsts.PORTMAP_NAME)) {
			
			keyVbrPortmapStruct.set(VtnServiceIpcConsts.PORTMAP_ID,
					IpcDataUnitWrapper
							.setIpcUint8ArrayValue(((JsonObject) requestBody
									.get(VtnServiceJsonConsts.PORTMAP))
									.get(VtnServiceJsonConsts.PORTMAP_NAME)
									.getAsString()));
			
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.THREE.ordinal()) {
			keyVbrPortmapStruct.set(VtnServiceIpcConsts.PORTMAP_ID,
					IpcDataUnitWrapper.setIpcUint8ArrayValue(uriParameters.get(2)));
		} else {
			LOG.debug("request body and uri parameters are not correct for getKeyVbrPortmapStruct");
		}
		
		LOG.info("Key Structure: " + keyVbrPortmapStruct.toString());
		LOG.trace("Complete getKeyVbrPortmapStruct");
		return keyVbrPortmapStruct;
	}
	
	/**
	 * Gets the val vbr port map struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the val vbr port map struct
	 */
	public final IpcStruct getValVbrPortmapStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		LOG.trace("Start getValVbrPortmapStruct");
		final IpcStruct valVbrPortmapStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValVbrPortmap.getValue());
		
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.PORTMAP)) {
			
			final JsonObject portmap = requestBody.getAsJsonObject(VtnServiceJsonConsts.PORTMAP);
			
			if (portmap.has(VtnServiceJsonConsts.CONTROLLERID)) {
				valVbrPortmapStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVbrPortmapIndex.UPLL_IDX_CONTROLLER_ID_VBRPM
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
											.ordinal()));
		
				valVbrPortmapStruct.set(VtnServiceIpcConsts.CONTROLLERID,
						IpcDataUnitWrapper.setIpcUint8ArrayValue(
								portmap.get(VtnServiceJsonConsts.CONTROLLERID)
										.getAsString(), valVbrPortmapStruct,
								UncStructIndexEnum.ValVbrPortmapIndex.UPLL_IDX_CONTROLLER_ID_VBRPM
										.ordinal()));
			} else {
				valVbrPortmapStruct
				.set(VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVbrPortmapIndex.UPLL_IDX_CONTROLLER_ID_VBRPM
								.ordinal(),
						IpcDataUnitWrapper
								.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
										.ordinal()));
			}
			
			if (portmap.has(VtnServiceJsonConsts.DOMAINID)) {
				valVbrPortmapStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVbrPortmapIndex.UPLL_IDX_DOMAIN_ID_VBRPM
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				
				valVbrPortmapStruct.set(VtnServiceIpcConsts.DOMAINID,
						IpcDataUnitWrapper.setIpcUint8ArrayValue(
								portmap.get(VtnServiceJsonConsts.DOMAINID)
										.getAsString(), valVbrPortmapStruct,
								UncStructIndexEnum.ValVbrPortmapIndex.UPLL_IDX_DOMAIN_ID_VBRPM
										.ordinal()));
			} else {
				valVbrPortmapStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVbrPortmapIndex.UPLL_IDX_DOMAIN_ID_VBRPM
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			}
			
			if (portmap.has(VtnServiceJsonConsts.LOGICAL_PORT_ID)) {
				valVbrPortmapStruct.set(VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVbrPortmapIndex.UPLL_IDX_LOGICAL_PORT_ID_VBRPM
								.ordinal(),
						IpcDataUnitWrapper
								.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
										.ordinal()));
				
				valVbrPortmapStruct.set(VtnServiceIpcConsts.LOGICAL_PORT_ID,
						IpcDataUnitWrapper.setIpcUint8ArrayValue(
								portmap.get(VtnServiceJsonConsts.LOGICAL_PORT_ID)
										.getAsString(), valVbrPortmapStruct,
								UncStructIndexEnum.ValVbrPortmapIndex.UPLL_IDX_LOGICAL_PORT_ID_VBRPM
										.ordinal()));
			} else {
				valVbrPortmapStruct.set(VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVbrPortmapIndex.UPLL_IDX_LOGICAL_PORT_ID_VBRPM
								.ordinal(),
						IpcDataUnitWrapper
								.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
										.ordinal()));
			}
			
			if (portmap.has(VtnServiceJsonConsts.LABEL_TYPE)) {
				
				if (VtnServiceJsonConsts.VLAN_ID.equals(portmap.get(
						VtnServiceJsonConsts.LABEL_TYPE).getAsString())) {
					valVbrPortmapStruct.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVbrPortmapIndex.UPLL_IDX_LABEL_TYPE_VBRPM
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
											.ordinal()));
					
					valVbrPortmapStruct.set(VtnServiceIpcConsts.LABEL_TYPE,
							IpcDataUnitWrapper.setIpcUint8Value(
									ValLabelType.UPLL_LABEL_TYPE_VLAN.getValue(), 
									valVbrPortmapStruct,
									UncStructIndexEnum.ValVbrPortmapIndex.UPLL_IDX_LABEL_TYPE_VBRPM
											.ordinal()));
				} else {
					valVbrPortmapStruct.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVbrPortmapIndex.UPLL_IDX_LABEL_TYPE_VBRPM
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
											.ordinal()));
				}
				
			} else {
				valVbrPortmapStruct.set(VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVbrPortmapIndex.UPLL_IDX_LABEL_TYPE_VBRPM
								.ordinal(),
						IpcDataUnitWrapper
								.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
										.ordinal()));
			}
			
			if (portmap.has(VtnServiceJsonConsts.LABEL)) {
				
				valVbrPortmapStruct.set(VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVbrPortmapIndex.UPLL_IDX_LABEL_VBRPM
								.ordinal(),
						IpcDataUnitWrapper
								.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
										.ordinal()));
				
				if (VtnServiceJsonConsts.ANY_VLAN_ID.equals(
						portmap.getAsJsonPrimitive(VtnServiceJsonConsts.LABEL)
						.getAsString())) {
					
					valVbrPortmapStruct.set(VtnServiceIpcConsts.LABEL,
							IpcDataUnitWrapper.setIpcUint32Value(
									Integer.toString(VtnServiceJsonConsts.VAL_FFFE)
									, valVbrPortmapStruct,
									UncStructIndexEnum.ValVbrPortmapIndex.UPLL_IDX_LABEL_VBRPM
											.ordinal()));
				} else if (VtnServiceJsonConsts.NO_VLAN_ID.equals(
						portmap.getAsJsonPrimitive(VtnServiceJsonConsts.LABEL)
						.getAsString())) {
					
					valVbrPortmapStruct.set(VtnServiceIpcConsts.LABEL,
							IpcDataUnitWrapper.setIpcUint32Value(
									Integer.toString(VtnServiceJsonConsts.VAL_FFFF),
									valVbrPortmapStruct,
									UncStructIndexEnum.ValVbrPortmapIndex.UPLL_IDX_LABEL_VBRPM
											.ordinal()));
				} else {
					valVbrPortmapStruct.set(VtnServiceIpcConsts.LABEL,
							IpcDataUnitWrapper.setIpcUint32Value(
									portmap.get(VtnServiceJsonConsts.LABEL)
											.getAsString(), valVbrPortmapStruct,
									UncStructIndexEnum.ValVbrPortmapIndex.UPLL_IDX_LABEL_VBRPM
											.ordinal()));
				}
				
			} else {
				valVbrPortmapStruct.set(VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVbrPortmapIndex.UPLL_IDX_LABEL_VBRPM
								.ordinal(),
						IpcDataUnitWrapper
								.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
										.ordinal()));
			}
			
		} else {
			LOG.debug("request body and uri parameters are not correct for getValVbrPortmapStruct");
		}
		LOG.info("Value Structure: " + valVbrPortmapStruct.toString());
		LOG.trace("Complete getValVbrPortmapStruct");
		return valVbrPortmapStruct;
	}

	/**
	 * Gets the key vrt if flow filter struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key vrt if flow filter struct
	 */
	public final IpcStruct getKeyVrtIfFlowFilterStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		LOG.trace("Start getKeyVrtIfFlowFilterStruct");
		final IpcStruct KeyVrtIfFlowFilterStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyVrtIfFlowFilter.getValue());
		IpcStruct KeyVrtIfStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.THREE.ordinal()) {
			KeyVrtIfStruct = getKeyVrtIfStruct(requestBody,
					uriParameters.subList(0, 3));
		}
		KeyVrtIfFlowFilterStruct.set(VtnServiceIpcConsts.KEYVBRIF,
				KeyVrtIfStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.FLOWFILTER)
				&& ((JsonObject) requestBody
						.get(VtnServiceJsonConsts.FLOWFILTER))
						.has(VtnServiceJsonConsts.FFTYPE)) {
			if (requestBody.getAsJsonObject(VtnServiceJsonConsts.FLOWFILTER)
					.get(VtnServiceJsonConsts.FFTYPE).getAsString()
					.equalsIgnoreCase(VtnServiceJsonConsts.IN)) {
				KeyVrtIfFlowFilterStruct
						.set(VtnServiceIpcConsts.DIRECTION,
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_IN
												.ordinal()));
			} else {
				KeyVrtIfFlowFilterStruct
						.set(VtnServiceIpcConsts.DIRECTION,
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_OUT
												.ordinal()));
			}
			LOG.debug("ff_type:"
					+ requestBody
							.getAsJsonObject(VtnServiceJsonConsts.FLOWFILTER)
							.get(VtnServiceJsonConsts.FFTYPE).getAsString());
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.FOUR.ordinal()) {
			if (uriParameters.get(3).equalsIgnoreCase(VtnServiceJsonConsts.IN)) {
				KeyVrtIfFlowFilterStruct
						.set(VtnServiceIpcConsts.DIRECTION,
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_IN
												.ordinal()));
			} else {
				KeyVrtIfFlowFilterStruct
						.set(VtnServiceIpcConsts.DIRECTION,
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_OUT
												.ordinal()));
			}
			LOG.debug("ff_type:" + uriParameters.get(3));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVrtIfFlowFilterStruct");
		}
		LOG.info("Key Structure: " + KeyVrtIfFlowFilterStruct.toString());
		LOG.trace("Complete getKeyVrtIfFlowFilterStruct");
		return KeyVrtIfFlowFilterStruct;
	}

	/**
	 * Gets the key vrt if flow filter entry struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key vrt if flow filter entry struct
	 */
	public final IpcStruct getKeyVrtIfFlowFilterEntryStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		LOG.trace("Start getKeyVrtIfFlowFilterEntryStruct");
		final IpcStruct KeyVrtIfFlowFilterEntryStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyVrtIfFlowFilterEntry
						.getValue());
		IpcStruct KeyVrtIfFlowFilterStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.FOUR.ordinal()) {
			KeyVrtIfFlowFilterStruct = getKeyVrtIfFlowFilterStruct(requestBody,
					uriParameters.subList(0, 4));
		}
		KeyVrtIfFlowFilterEntryStruct.set(VtnServiceIpcConsts.FLOWFILTERKEY,
				KeyVrtIfFlowFilterStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.FLOWFILTERENTRY)
				&& ((JsonObject) requestBody
						.get(VtnServiceJsonConsts.FLOWFILTERENTRY))
						.has(VtnServiceJsonConsts.SEQNUM)) {
			KeyVrtIfFlowFilterEntryStruct.set(VtnServiceJsonConsts.SEQUENCENUM,
					IpcDataUnitWrapper
							.setIpcUint16Value(((JsonObject) requestBody
									.get(VtnServiceJsonConsts.FLOWFILTERENTRY))
									.get(VtnServiceJsonConsts.SEQNUM)
									.getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.FIVE.ordinal()) {
			KeyVrtIfFlowFilterEntryStruct.set(VtnServiceJsonConsts.SEQUENCENUM,
					IpcDataUnitWrapper.setIpcUint16Value(uriParameters.get(4)));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVrtIfFlowFilterEntryStruct");
		}
		LOG.info("Key Structure: " + KeyVrtIfFlowFilterEntryStruct.toString());
		LOG.trace("Complete getKeyVrtIfFlowFilterEntryStruct");
		return KeyVrtIfFlowFilterEntryStruct;
	}

	/**
	 * Gets the key vbr IfFlow Filter
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key vbr IfFlow Filter
	 */

	public final IpcStruct getKeyVbrIfFlowFilterStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		LOG.trace("Start getKeyVbrIfFlowFilterStruct");
		final IpcStruct keyVbrIfFlowFilterStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyVbrIfFlowFilter.getValue());
		IpcStruct keyVbrIfStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.THREE.ordinal()) {
			keyVbrIfStruct = getKeyVbrIfStruct(requestBody,
					uriParameters.subList(0, 3));
		}
		keyVbrIfFlowFilterStruct.set(VtnServiceIpcConsts.KEYVBRIF,
				keyVbrIfStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.FLOWFILTER)
				&& ((JsonObject) requestBody
						.get(VtnServiceJsonConsts.FLOWFILTER))
						.has(VtnServiceJsonConsts.FFTYPE)) {
			if (requestBody.getAsJsonObject(VtnServiceJsonConsts.FLOWFILTER)
					.get(VtnServiceJsonConsts.FFTYPE).getAsString()
					.equalsIgnoreCase(VtnServiceJsonConsts.IN)) {
				keyVbrIfFlowFilterStruct
						.set(VtnServiceIpcConsts.DIRECTION,
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_IN
												.ordinal()));
			} else {
				keyVbrIfFlowFilterStruct
						.set(VtnServiceIpcConsts.DIRECTION,
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_OUT
												.ordinal()));
			}
			LOG.debug("ff_type:"
					+ requestBody
							.getAsJsonObject(VtnServiceJsonConsts.FLOWFILTER)
							.get(VtnServiceJsonConsts.FFTYPE).getAsString());
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.FOUR.ordinal()) {

			if (uriParameters.get(3).equalsIgnoreCase(VtnServiceJsonConsts.IN)) {
				keyVbrIfFlowFilterStruct
						.set(VtnServiceIpcConsts.DIRECTION,
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_IN
												.ordinal()));
			} else {

				keyVbrIfFlowFilterStruct
						.set(VtnServiceIpcConsts.DIRECTION,
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_OUT
												.ordinal()));
			}
			LOG.debug("ff_type:" + uriParameters.get(3));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVbrIfFlowFilterStruct");
		}
		LOG.info("Key Structure: " + keyVbrIfFlowFilterStruct.toString());
		LOG.trace("Complete getKeyVbrIfFlowFilterStruct");
		return keyVbrIfFlowFilterStruct;
	}

	/**
	 * Gets the key vbr struct for VBridge APIs
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key vbr struct
	 */
	public final IpcStruct getKeyVbrStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		// Lower level structure
		/*
		 * ipc_struct KeyVbr { KeyVtn VtnKey; UINT8 vbridge_name[32]; };
		 */
		LOG.trace("Start getKeyVbrStruct");
		final IpcStruct keyVbrStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyVbr.getValue());
		IpcStruct keyVtnStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.ONE.ordinal()) {
			keyVtnStruct = getKeyVtnStruct(requestBody,
					uriParameters.subList(0, 1));
		}
		keyVbrStruct.set(VtnServiceIpcConsts.VTNKEY, keyVtnStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.VBRIDGE)
				&& ((JsonObject) requestBody.get(VtnServiceJsonConsts.VBRIDGE))
						.has(VtnServiceJsonConsts.VBRIDGENAME)) {
			keyVbrStruct.set(VtnServiceJsonConsts.VBRIDGE_NAME,
					IpcDataUnitWrapper
							.setIpcUint8ArrayValue(((JsonObject) requestBody
									.get(VtnServiceJsonConsts.VBRIDGE)).get(
									VtnServiceJsonConsts.VBRIDGENAME)
									.getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.TWO.ordinal()) {
			keyVbrStruct.set(VtnServiceJsonConsts.VBRIDGE_NAME,
					IpcDataUnitWrapper.setIpcUint8ArrayValue(uriParameters
							.get(1)));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVbrStruct");
		}
		LOG.info("Key Structure: " + keyVbrStruct.toString());
		LOG.trace("Complete getKeyVbrStruct");

		return keyVbrStruct;
	}

	/**
	 * Gets the val vbr struct for VBridge APIs
	 * 
	 * @param requestBody
	 *            the request body
	 * @return the val vbr struct
	 */
	public final IpcStruct getValVbrStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		/*
		 * ipc_struct val_vbr { UINT8 valid[5]; UINT8 cs_row_status; UINT8
		 * cs_attr[5]; UINT8 controller_id[32]; UINT8 domain_id[32] UINT8
		 * vbr_description[128]; IPV4 host_addr; UINT8 host_addr_prefixlen; }
		 */
		LOG.trace("Start getValVbrStruct");
		final IpcStruct valVbrStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValVbr.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.VBRIDGE)) {
			final JsonObject vBridge = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VBRIDGE);
			if (vBridge.has(VtnServiceJsonConsts.CONTROLLERID)) {
				valVbrStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVbrIndex.UPLL_IDX_CONTROLLER_ID_VBR
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVbrStruct
						.set(VtnServiceJsonConsts.CONTROLLERID,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vBridge.get(
														VtnServiceJsonConsts.CONTROLLERID)
														.getAsString(),
												valVbrStruct,
												UncStructIndexEnum.ValVbrIndex.UPLL_IDX_CONTROLLER_ID_VBR
														.ordinal()));
			} else {
				valVbrStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVbrIndex.UPLL_IDX_CONTROLLER_ID_VBR
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (vBridge.has(VtnServiceJsonConsts.DOMAINID)) {
				valVbrStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVbrIndex.UPLL_IDX_DOMAIN_ID_VBR
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVbrStruct
						.set(VtnServiceJsonConsts.DOMAINID,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vBridge.get(
														VtnServiceJsonConsts.DOMAINID)
														.getAsString(),
												valVbrStruct,
												UncStructIndexEnum.ValVbrIndex.UPLL_IDX_DOMAIN_ID_VBR
														.ordinal()));
			} else {
				valVbrStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVbrIndex.UPLL_IDX_DOMAIN_ID_VBR
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (vBridge.has(VtnServiceJsonConsts.DESCRIPTION)) {
				valVbrStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVbrIndex.UPLL_IDX_DESC_VBR
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVbrStruct
						.set(VtnServiceJsonConsts.VBRDESCRIPTION,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vBridge.get(
														VtnServiceJsonConsts.DESCRIPTION)
														.getAsString(),
												valVbrStruct,
												UncStructIndexEnum.ValVbrIndex.UPLL_IDX_DESC_VBR
														.ordinal()));
			} else {
				valVbrStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVbrIndex.UPLL_IDX_DESC_VBR
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			valVbrStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVbrIndex.UPLL_IDX_HOST_ADDR_VBR
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			valVbrStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVbrIndex.UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
		} else if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.IPADDRESS)) {
			final JsonObject ipAddress = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.IPADDRESS);
			valVbrStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVbrIndex.UPLL_IDX_CONTROLLER_ID_VBR
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			valVbrStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVbrIndex.UPLL_IDX_DOMAIN_ID_VBR
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			valVbrStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVbrIndex.UPLL_IDX_DESC_VBR
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			if (ipAddress.has(VtnServiceJsonConsts.IPADDR)) {
				valVbrStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVbrIndex.UPLL_IDX_HOST_ADDR_VBR
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVbrStruct
						.set(VtnServiceIpcConsts.HOST_ADDR,
								IpcDataUnitWrapper
										.setIpcInet4AddressValue(
												ipAddress
														.get(VtnServiceJsonConsts.IPADDR)
														.getAsString(),
												valVbrStruct,
												UncStructIndexEnum.ValVbrIndex.UPLL_IDX_HOST_ADDR_VBR
														.ordinal()));
			} else {
				valVbrStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVbrIndex.UPLL_IDX_HOST_ADDR_VBR
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (ipAddress.has(VtnServiceJsonConsts.PREFIX)) {
				valVbrStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVbrIndex.UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVbrStruct
						.set(VtnServiceIpcConsts.HOST_ADDR_PREFIXLEN,
								IpcDataUnitWrapper
										.setIpcUint8Value(
												ipAddress
														.get(VtnServiceJsonConsts.PREFIX)
														.getAsString(),
												valVbrStruct,
												UncStructIndexEnum.ValVbrIndex.UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR
														.ordinal()));
			} else {
				valVbrStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVbrIndex.UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
		} else if (requestBody == null) {
			valVbrStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVbrIndex.UPLL_IDX_CONTROLLER_ID_VBR
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			valVbrStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVbrIndex.UPLL_IDX_DOMAIN_ID_VBR
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			valVbrStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVbrIndex.UPLL_IDX_DESC_VBR
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			valVbrStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVbrIndex.UPLL_IDX_HOST_ADDR_VBR
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
											.ordinal()));
			valVbrStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVbrIndex.UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
											.ordinal()));
		} else {
			LOG.warning("request body and uri parameters are not correct for getValVbrStruct");
		}
		LOG.info("Value Structure: " + valVbrStruct.toString());
		LOG.trace("Complete getValVbrStruct");
		return valVbrStruct;
	}

	/**
	 * Gets the key vlan map struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key vlan map struct
	 */
	public final IpcStruct getKeyVlanMapStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		/*
		 * 
		 * ipc_struct key_vlan_map { key_vbr vbr_key; UINT8
		 * logical_port_id[320]; UINT8 logical_port_id_valid; };
		 */
		LOG.trace("Start getKeyVlanMapStruct");
		final IpcStruct keyVlanMapStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyVlanMap.getValue());
		IpcStruct keyVbrStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.TWO.ordinal()) {
			keyVbrStruct = getKeyVbrStruct(requestBody,
					uriParameters.subList(0, 2));
		}
		keyVlanMapStruct.set(VtnServiceIpcConsts.VBRKEY, keyVbrStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.VLANMAP)
				&& uriParameters.size() < UncIndexEnum.THREE.ordinal()) {
			if (((JsonObject) requestBody.get(VtnServiceJsonConsts.VLANMAP))
					.has(VtnServiceJsonConsts.LOGICAL_PORT_ID)) {
				keyVlanMapStruct
						.set(VtnServiceJsonConsts.LOGICAL_PORT_ID,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(((JsonObject) requestBody
												.get(VtnServiceJsonConsts.VLANMAP))
												.get(VtnServiceJsonConsts.LOGICAL_PORT_ID)
												.getAsString()));
				keyVlanMapStruct
						.set(VtnServiceIpcConsts.LPID_VALID,
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.PfcStatus.PFC_TRUE
												.getValue()));
			} else {
				keyVlanMapStruct
						.set(VtnServiceIpcConsts.LPID_VALID,
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.PfcStatus.PFC_FALSE
												.getValue()));
			}
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.THREE.ordinal()) {
			final String VlanMapId[] = uriParameters.get(2).split(
					VtnServiceJsonConsts.VLANMAPIDSEPERATOR, 2);
			if (VlanMapId.length == UncIndexEnum.TWO.ordinal()) {
				keyVlanMapStruct
						.set(VtnServiceIpcConsts.LPID_VALID,
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.PfcStatus.PFC_TRUE
												.getValue()));
				keyVlanMapStruct.set(VtnServiceJsonConsts.LOGICAL_PORT_ID,
						VlanMapId[1]);
			} else if (VlanMapId.length == UncIndexEnum.ONE.ordinal()) {
				keyVlanMapStruct
						.set(VtnServiceIpcConsts.LPID_VALID,
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.PfcStatus.PFC_FALSE
												.getValue()));
			}
		}
		LOG.info("Key Structure: " + keyVlanMapStruct.toString());
		LOG.trace("Complete getKeyVlanMapStruct");

		return keyVlanMapStruct;
	}

	/**
	 * Gets the val vlan map struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @return the val vlan map struct
	 */
	public final IpcStruct getValVlanMapStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		final IpcStruct valVlanMapStruct = new IpcStruct(
				UncStructEnum.ValVlanMap.getValue());
		/*
		 * ipc_struct val_vlan_map { UINT8 valid[1]; UINT8 cs_row_status; UINT8
		 * cs_attr[1]; UINT16 vlan_id; };
		 */
		LOG.trace("Start getValVlanMapStruct");
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.VLANMAP)
				&& requestBody.getAsJsonObject(VtnServiceJsonConsts.VLANMAP)
						.has(VtnServiceJsonConsts.VLANID)) {
			valVlanMapStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVlanMapIndex.UPLL_IDX_VLAN_ID_VM
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
											.ordinal()));
			valVlanMapStruct
					.set(VtnServiceJsonConsts.VLANID,
							IpcDataUnitWrapper
									.setIpcUint16Value(
											requestBody
													.getAsJsonObject(
															VtnServiceJsonConsts.VLANMAP)
													.get(VtnServiceJsonConsts.VLANID)
													.getAsString(),
											valVlanMapStruct,
											UncStructIndexEnum.ValVlanMapIndex.UPLL_IDX_VLAN_ID_VM
													.ordinal()));
		} else if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.VLANMAP)
				&& requestBody.getAsJsonObject(VtnServiceJsonConsts.VLANMAP)
						.has(VtnServiceJsonConsts.NO_VLAN_ID)) {
			valVlanMapStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVlanMapIndex.UPLL_IDX_VLAN_ID_VM
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
											.ordinal()));
			valVlanMapStruct
					.set(VtnServiceJsonConsts.VLANID,
							IpcDataUnitWrapper
									.setIpcUint16HexaValue(
											VtnServiceIpcConsts.VLAN_ID_DEFAULT_VALUE,
											valVlanMapStruct,
											UncStructIndexEnum.ValVlanMapIndex.UPLL_IDX_VLAN_ID_VM
													.ordinal()));
		} else {
			valVlanMapStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVlanMapIndex.UPLL_IDX_VLAN_ID_VM
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
		}
		LOG.info("Value Structure: " + valVlanMapStruct.toString());
		LOG.trace("Complete getValVlanMapStruct");
		return valVlanMapStruct;
	}

	/**
	 * Gets the key vtn flow filter struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key vtn flow filter struct
	 */
	public final IpcStruct getKeyVtnFlowFilterStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		// Lower level structure
		/*
		 * ipc_struct KeyVtnFlowFilter { KeyVtn vtn_key; UINT8 input_direction;
		 * };
		 */
		LOG.trace("Start getKeyVtnFlowFilterStruct");
		final IpcStruct keyVtnFlowFilterStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyVtnFlowFilter.getValue());
		/* IpcStruct keyVtnStruct = new IpcStruct(UncStructEnum.KeyVtn.name()); */
		IpcStruct keyVtnStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.ONE.ordinal()) {
			keyVtnStruct = getKeyVtnStruct(requestBody,
					uriParameters.subList(0, 1));
		}
		keyVtnFlowFilterStruct.set(VtnServiceIpcConsts.VTNKEY, keyVtnStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.FLOWFILTER)
				&& ((JsonObject) requestBody
						.get(VtnServiceJsonConsts.FLOWFILTER))
						.has(VtnServiceJsonConsts.FFTYPE)) {
			if (requestBody.getAsJsonObject(VtnServiceJsonConsts.FLOWFILTER)
					.get(VtnServiceJsonConsts.FFTYPE).getAsString()
					.equalsIgnoreCase(VtnServiceJsonConsts.IN)) {
				keyVtnFlowFilterStruct
						.set(VtnServiceIpcConsts.INPUTDIRECTION,
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_IN
												.ordinal()));
			} else {
				keyVtnFlowFilterStruct
						.set(VtnServiceIpcConsts.INPUTDIRECTION,
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_OUT
												.ordinal()));

			}
			LOG.debug("ff_type"
					+ requestBody
							.getAsJsonObject(VtnServiceJsonConsts.FLOWFILTER)
							.get(VtnServiceJsonConsts.FFTYPE).getAsString());
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.TWO.ordinal()) {
			if (uriParameters.get(1).equalsIgnoreCase(VtnServiceJsonConsts.IN)) {
				keyVtnFlowFilterStruct
						.set(VtnServiceIpcConsts.INPUTDIRECTION,
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_IN
												.ordinal()));
			} else {
				keyVtnFlowFilterStruct
						.set(VtnServiceIpcConsts.INPUTDIRECTION,
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_OUT
												.ordinal()));
			}
			LOG.debug("ff_type" + uriParameters.get(1));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVtnFlowFilterStruct");
		}
		LOG.info("Key Structure: " + keyVtnFlowFilterStruct.toString());
		LOG.trace("Complete KeyVtnFlowFilterStruct");

		return keyVtnFlowFilterStruct;
	}

	/**
	 * Gets the key vtn flow filter entry struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key vtn flow filter entry struct
	 */
	public final IpcStruct getKeyVtnFlowFilterEntryStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		// Lower level structure
		/*
		 * ipc_struct KeyVtnFlowFilterEntry{ KeyVtnFlowFilter flowfilter_key;
		 * UINT16 sequence_num; };
		 */
		LOG.trace("Start getKeyVtnFlowFilterEntryStruct");
		final IpcStruct keyVtnFlowFilterEntryStruct = new IpcStruct(
				UncStructEnum.KeyVtnFlowFilterEntry.getValue());
		/*
		 * IpcStruct keyVtnFlowFilterStruct = new
		 * IpcStruct(UncStructEnum.KeyVtnFlowFilter.name());
		 */
		IpcStruct keyVtnFlowFilterStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.TWO.ordinal()) {
			keyVtnFlowFilterStruct = getKeyVtnFlowFilterStruct(requestBody,
					uriParameters.subList(0, 2));
		}
		keyVtnFlowFilterEntryStruct.set(VtnServiceIpcConsts.FLOWFILTERKEY,
				keyVtnFlowFilterStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.FLOWFILTERENTRY)
				&& ((JsonObject) requestBody
						.get(VtnServiceJsonConsts.FLOWFILTERENTRY))
						.has(VtnServiceJsonConsts.SEQNUM)) {
			keyVtnFlowFilterEntryStruct.set(VtnServiceJsonConsts.SEQUENCENUM,
					IpcDataUnitWrapper
							.setIpcUint16Value(((JsonObject) requestBody
									.get(VtnServiceJsonConsts.FLOWFILTERENTRY))
									.get(VtnServiceJsonConsts.SEQNUM)
									.getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.THREE.ordinal()) {
			keyVtnFlowFilterEntryStruct.set(VtnServiceJsonConsts.SEQUENCENUM,
					IpcDataUnitWrapper.setIpcUint16Value(uriParameters.get(2)));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVtnFlowFilterEntryStruct");
		}
		LOG.info("Key Structure: " + keyVtnFlowFilterEntryStruct.toString());
		LOG.trace("Complete getKeyVtnFlowFilterEntryStruct");
		return keyVtnFlowFilterEntryStruct;
	}

	/**
	 * Gets the key vunknown struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key vunknown struct
	 */
	public final IpcStruct getKeyVunknownStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		/*
		 * ipc_struct KeyVunknown { KeyVtn vtn_key; UINT8 vunknown_name[32]; };
		 * 
		 * };
		 */
		LOG.trace("Start getKeyVunknownStruct");
		final IpcStruct keyVunknownStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyVunknown.getValue());
		/* IpcStruct keyVtnStruct = new IpcStruct(UncStructEnum.KeyVtn.name()); */
		IpcStruct keyVtnStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.ONE.ordinal()) {
			keyVtnStruct = getKeyVtnStruct(requestBody,
					uriParameters.subList(0, 1));
		}
		keyVunknownStruct.set(VtnServiceIpcConsts.VTNKEY, keyVtnStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.VBYPASS)
				&& ((JsonObject) requestBody.get(VtnServiceJsonConsts.VBYPASS))
						.has(VtnServiceJsonConsts.VBYPASS_NAME)) {
			keyVunknownStruct.set(VtnServiceIpcConsts.VUNKNOWNNAME,
					IpcDataUnitWrapper
							.setIpcUint8ArrayValue(((JsonObject) requestBody
									.get(VtnServiceJsonConsts.VBYPASS)).get(
									VtnServiceJsonConsts.VBYPASS_NAME)
									.getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.TWO.ordinal()) {
			keyVunknownStruct.set(VtnServiceIpcConsts.VUNKNOWNNAME,
					IpcDataUnitWrapper.setIpcUint8ArrayValue(uriParameters
							.get(1)));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVunknownStruct");
		}
		LOG.info("Key Structure: " + keyVunknownStruct.toString());
		LOG.trace("Complete getKeyVunknownStruct");

		return keyVunknownStruct;
	}

	/**
	 * Gets the key vunk if struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key vunk if struct
	 */
	public final IpcStruct getKeyVunkIfStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		/*
		 * ipc_struct key_vunk_if { key_vunknown vunk_key; UINT8 if_name[32]; };
		 */
		LOG.trace("Start getKeyVunkIfStruct");
		// instead of vtn_key it should be KeyVunknown
		final IpcStruct keyVunkIfStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyVunkIf.getValue());
		IpcStruct keyVunknownStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.TWO.ordinal()) {
			keyVunknownStruct = getKeyVunknownStruct(requestBody,
					uriParameters.subList(0, 2));
		}
		// keyVunkIfStruct.set(VtnServiceIpcConsts.VUNKNOWNKEY,
		// keyVunknownStruct);
		keyVunkIfStruct.set(VtnServiceIpcConsts.VUNK_KEY, keyVunknownStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.INTERFACE)
				&& ((JsonObject) requestBody
						.get(VtnServiceJsonConsts.INTERFACE))
						.has(VtnServiceJsonConsts.IFNAME)) {
			keyVunkIfStruct.set(VtnServiceIpcConsts.IFNAME, IpcDataUnitWrapper
					.setIpcUint8ArrayValue(((JsonObject) requestBody
							.get(VtnServiceJsonConsts.INTERFACE)).get(
							VtnServiceJsonConsts.IFNAME).getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.THREE.ordinal()) {
			keyVunkIfStruct.set(VtnServiceIpcConsts.IFNAME, IpcDataUnitWrapper
					.setIpcUint8ArrayValue(uriParameters.get(2)));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVunkIfStruct");
		}
		LOG.info("Key Structure: " + keyVunkIfStruct.toString());
		LOG.trace("Complete getKeyVunkIfStruct");

		return keyVunkIfStruct;
	}

	/**
	 * Gets the key vrt if struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key vrt if struct
	 */
	public final IpcStruct getKeyVrtIfStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		// Lower level structure
		/*
		 * ipc_struct KeyVrtIf { key_vrt vrt_key; UINT8 if_name[32]; };
		 */
		LOG.trace("Start getKeyVrtIfStruct");
		final IpcStruct keyVrtIfStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyVrtIf.getValue());
		IpcStruct keyVtnVrtStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.TWO.ordinal()) {
			keyVtnVrtStruct = getKeyVrtStruct(requestBody,
					uriParameters.subList(0, 2));
		}
		keyVrtIfStruct.set(VtnServiceIpcConsts.VRTKEY, keyVtnVrtStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.INTERFACE)
				&& ((JsonObject) requestBody
						.get(VtnServiceJsonConsts.INTERFACE))
						.has(VtnServiceJsonConsts.IFNAME)) {
			keyVrtIfStruct.set(VtnServiceIpcConsts.IFNAME, IpcDataUnitWrapper
					.setIpcUint8ArrayValue(((JsonObject) requestBody
							.get(VtnServiceJsonConsts.INTERFACE)).get(
							VtnServiceJsonConsts.IFNAME).getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.THREE.ordinal()) {
			keyVrtIfStruct.set(VtnServiceIpcConsts.IFNAME, IpcDataUnitWrapper
					.setIpcUint8ArrayValue(uriParameters.get(2)));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVrtIfStruct");
		}
		LOG.info("Key Structure: " + keyVrtIfStruct.toString());
		LOG.trace("Complete getKeyVrtIfStruct");

		return keyVrtIfStruct;
	}

	/**
	 * Gets the key vtunnel struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key vtunnel struct
	 */
	public final IpcStruct getKeyVtunnelStruct(final JsonObject requestBody,
			final List<String> uriParameters) {

		// Lower level structure
		/*
		 * ipc_struct KeyVtunnel { KeyVtn vtn_key; UINT8 vtunnel_name[32]; };
		 */
		LOG.trace("Start getKeyVtunnelStruct");
		final IpcStruct keyVtunnelStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyVtunnel.getValue());
		IpcStruct keyVtnStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.ONE.ordinal()) {
			keyVtnStruct = getKeyVtnStruct(requestBody,
					uriParameters.subList(0, 1));
		}
		keyVtunnelStruct.set(VtnServiceIpcConsts.VTNKEY, keyVtnStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.VTUNNEL)
				&& ((JsonObject) requestBody.get(VtnServiceJsonConsts.VTUNNEL))
						.has(VtnServiceJsonConsts.VTUNNELNAME)) {
			keyVtunnelStruct.set(VtnServiceIpcConsts.VTUNNELNAME,
					IpcDataUnitWrapper
							.setIpcUint8ArrayValue(((JsonObject) requestBody
									.get(VtnServiceJsonConsts.VTUNNEL)).get(
									VtnServiceJsonConsts.VTUNNELNAME)
									.getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.TWO.ordinal()) {
			keyVtunnelStruct.set(VtnServiceIpcConsts.VTUNNELNAME,
					IpcDataUnitWrapper.setIpcUint8ArrayValue(uriParameters
							.get(1)));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVtunnelStruct");
		}
		LOG.info("Key Structure: " + keyVtunnelStruct.toString());
		LOG.trace("Complete getKeyVtunnelStruct");

		return keyVtunnelStruct;
	}

	/**
	 * Gets the key static ip route struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key static ip route struct
	 */
	public final IpcStruct getKeyStaticIpRouteStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		// Lower level structure
		/*
		 * ipc_struct key_static_ip_route { key_vrt vrt_key; IPV4 dst_addr;
		 * UINT8 dst_addr_prefixlen; UINT8 nwm_name[32]; };
		 */
		LOG.trace("Start getKeyStaticIpRouteStruct");
		final IpcStruct keyStaticIpRouteStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyStaticIpRoute.getValue());
		IpcStruct keyVtnVrtStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.TWO.ordinal()) {
			keyVtnVrtStruct = getKeyVrtStruct(requestBody,
					uriParameters.subList(0, 2));
		}
		keyStaticIpRouteStruct.set(VtnServiceIpcConsts.VRTKEY, keyVtnVrtStruct);
		if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.THREE.ordinal()) {
			final String[] staticIpRouteId = uriParameters.get(2).split(
					VtnServiceJsonConsts.HYPHEN);
			keyStaticIpRouteStruct.set(VtnServiceIpcConsts.DST_ADDR,
					IpcDataUnitWrapper
							.setIpcInet4AddressValue(staticIpRouteId[0]));
			keyStaticIpRouteStruct.set(VtnServiceIpcConsts.DST_ADDR_PREFIXLEN,
					IpcDataUnitWrapper.setIpcUint8Value(staticIpRouteId[2]));
			keyStaticIpRouteStruct.set(VtnServiceIpcConsts.NEXT_HOP_ADDR,
					IpcDataUnitWrapper
							.setIpcInet4AddressValue(staticIpRouteId[1]));
		} else if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.STATIC_IPROUTE)) {
			final JsonObject staticIpRoute = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.STATIC_IPROUTE);
			if (staticIpRoute.has(VtnServiceJsonConsts.IPADDR)
					&& staticIpRoute.has(VtnServiceJsonConsts.PREFIX)
					&& staticIpRoute.has(VtnServiceJsonConsts.NEXTHOPADDR)) {
				keyStaticIpRouteStruct.set(VtnServiceIpcConsts.DST_ADDR,
						IpcDataUnitWrapper
								.setIpcInet4AddressValue(staticIpRoute.get(
										VtnServiceJsonConsts.IPADDR)
										.getAsString()));
				keyStaticIpRouteStruct.set(
						VtnServiceIpcConsts.DST_ADDR_PREFIXLEN,
						IpcDataUnitWrapper.setIpcUint8Value(staticIpRoute.get(
								VtnServiceJsonConsts.PREFIX).getAsString()));
				keyStaticIpRouteStruct.set(VtnServiceIpcConsts.NEXT_HOP_ADDR,
						IpcDataUnitWrapper
								.setIpcInet4AddressValue(staticIpRoute.get(
										VtnServiceJsonConsts.NEXTHOPADDR)
										.getAsString()));

			} else if (staticIpRoute.has(VtnServiceJsonConsts.NEXTHOPADDR)) {
				keyStaticIpRouteStruct.set(VtnServiceIpcConsts.NEXT_HOP_ADDR,
						IpcDataUnitWrapper
								.setIpcInet4AddressValue(staticIpRoute.get(
										VtnServiceJsonConsts.NEXTHOPADDR)
										.getAsString()));
			}
		} else {
			LOG.warning("Request body and uri parameters are not correct for getKeyStaticIpRouteStruct");
		}
		LOG.info("Key Structure: " + keyStaticIpRouteStruct.toString());
		LOG.trace("Complete getKeyStaticIpRouteStruct");
		return keyStaticIpRouteStruct;
	}

	/**
	 * Gets the val vtn flow filter entry struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the val vtn flow filter entry struct
	 */
	public final IpcStruct getValVtnFlowFilterEntryStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		LOG.trace("Start getValVtnFlowFilterEntryStruct");
		final IpcStruct valVtnFlowFilterEntryStruct = new IpcStruct(
				UncStructEnum.ValVtnFlowFilterEntry.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.FLOWFILTERENTRY)) {
			final JsonObject flowFilterEntry = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.FLOWFILTERENTRY);
			if (flowFilterEntry.has(VtnServiceJsonConsts.FLNAME)) {
				valVtnFlowFilterEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnFlowfilterEntryIndex.UPLL_IDX_FLOWLIST_NAME_VFFE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVtnFlowFilterEntryStruct
						.set(VtnServiceJsonConsts.FLOWLISTNAME,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												flowFilterEntry
														.get(VtnServiceJsonConsts.FLNAME)
														.getAsString(),
												valVtnFlowFilterEntryStruct,
												UncStructIndexEnum.ValVtnFlowfilterEntryIndex.UPLL_IDX_FLOWLIST_NAME_VFFE
														.ordinal()));
			} else {
				valVtnFlowFilterEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnFlowfilterEntryIndex.UPLL_IDX_FLOWLIST_NAME_VFFE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (flowFilterEntry.has(VtnServiceJsonConsts.ACTIONTYPE)) {
				valVtnFlowFilterEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnFlowfilterEntryIndex.UPLL_IDX_ACTION_VFFE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				if (flowFilterEntry.get(VtnServiceJsonConsts.ACTIONTYPE)
						.getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.PASS)) {
					valVtnFlowFilterEntryStruct
							.set(VtnServiceJsonConsts.ACTION,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.FlowfilterAction.UPLL_FLOWFILTER_ACT_PASS
													.ordinal()));
				} else {
					valVtnFlowFilterEntryStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVtnFlowfilterEntryIndex.UPLL_IDX_ACTION_VFFE
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
													.ordinal()));
				}
			} else {
				valVtnFlowFilterEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnFlowfilterEntryIndex.UPLL_IDX_ACTION_VFFE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (flowFilterEntry.has(VtnServiceJsonConsts.NMGNAME)) {
				valVtnFlowFilterEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnFlowfilterEntryIndex.UPLL_IDX_NWN_NAME_VFFE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVtnFlowFilterEntryStruct
						.set(VtnServiceIpcConsts.NWMNAME,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												flowFilterEntry
														.get(VtnServiceJsonConsts.NMGNAME)
														.getAsString(),
												valVtnFlowFilterEntryStruct,
												UncStructIndexEnum.ValVtnFlowfilterEntryIndex.UPLL_IDX_NWN_NAME_VFFE
														.ordinal()));
			} else {
				valVtnFlowFilterEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnFlowfilterEntryIndex.UPLL_IDX_NWN_NAME_VFFE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (flowFilterEntry.has(VtnServiceJsonConsts.DSCP)) {
				valVtnFlowFilterEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnFlowfilterEntryIndex.UPLL_IDX_DSCP_VFFE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVtnFlowFilterEntryStruct
						.set(VtnServiceJsonConsts.DSCP,
								IpcDataUnitWrapper
										.setIpcUint8Value(
												flowFilterEntry
														.get(VtnServiceJsonConsts.DSCP)
														.getAsString(),
												valVtnFlowFilterEntryStruct,
												UncStructIndexEnum.ValVtnFlowfilterEntryIndex.UPLL_IDX_DSCP_VFFE
														.ordinal()));
			} else {
				valVtnFlowFilterEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnFlowfilterEntryIndex.UPLL_IDX_DSCP_VFFE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (flowFilterEntry.has(VtnServiceJsonConsts.PRIORITY)) {
				valVtnFlowFilterEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnFlowfilterEntryIndex.UPLL_IDX_PRIORITY_VFFE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVtnFlowFilterEntryStruct
						.set(VtnServiceJsonConsts.PRIORITY,
								IpcDataUnitWrapper
										.setIpcUint8Value(
												flowFilterEntry
														.get(VtnServiceJsonConsts.PRIORITY)
														.getAsString(),
												valVtnFlowFilterEntryStruct,
												UncStructIndexEnum.ValVtnFlowfilterEntryIndex.UPLL_IDX_PRIORITY_VFFE
														.ordinal()));
			} else {
				valVtnFlowFilterEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnFlowfilterEntryIndex.UPLL_IDX_PRIORITY_VFFE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
		} else {
			LOG.warning("request body and uri parameters are not correct for getValVtnFlowFilterEntryStruct");
		}
		LOG.trace("Complete getValVtnFlowFilterEntryStruct");
		return valVtnFlowFilterEntryStruct;
	}

	/**
	 * Gets the val vunknown struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the val vunknown struct
	 */
	public final IpcStruct getValVunknownStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		// Lower level structure
		/*
		 * ipc_struct val_vunknown { UINT8 valid[4]; UINT8 cs_row_status; UINT8
		 * cs_attr[4]; UINT8 description[128]; UINT8 type; UINT8
		 * controller_id[32]; UINT8 domain_id[32]; };
		 */
		LOG.trace("Start getValVunknownStruct");
		final IpcStruct valValVunknownStruct = new IpcStruct(
				UncStructEnum.ValVunknown.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.VBYPASS)) {
			final JsonObject vByPass = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VBYPASS);
			if (vByPass.has(VtnServiceJsonConsts.DESCRIPTION)) {
				valValVunknownStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVunknownIndex.UPLL_IDX_DESC_VUN
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valValVunknownStruct
						.set(VtnServiceJsonConsts.DESCRIPTION,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vByPass.get(
														VtnServiceJsonConsts.DESCRIPTION)
														.getAsString(),
												valValVunknownStruct,
												UncStructIndexEnum.ValVunknownIndex.UPLL_IDX_DESC_VUN
														.ordinal()));
			} else {
				valValVunknownStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVunknownIndex.UPLL_IDX_DESC_VUN
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (vByPass.has(VtnServiceJsonConsts.TYPE)) {
				valValVunknownStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVunknownIndex.UPLL_IDX_TYPE_VUN
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				if (vByPass.get(VtnServiceJsonConsts.TYPE).getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.BRIDGE)) {
					valValVunknownStruct
							.set(VtnServiceIpcConsts.TYPE,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.ValVunknowntype.VUNKNOWN_TYPE_BRIDGE
													.ordinal()));
				} else if (vByPass.get(VtnServiceJsonConsts.TYPE).getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.ROUTER)) {
					valValVunknownStruct
							.set(VtnServiceIpcConsts.TYPE,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.ValVunknowntype.VUNKNOWN_TYPE_ROUTER
													.ordinal()));
				}
				LOG.debug("type:"
						+ vByPass.get(VtnServiceJsonConsts.TYPE).getAsString());
			} else {
				valValVunknownStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVunknownIndex.UPLL_IDX_TYPE_VUN
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (vByPass.has(VtnServiceJsonConsts.DOMAINID)) {
				valValVunknownStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVunknownIndex.UPLL_IDX_DOMAIN_ID_VUN
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valValVunknownStruct
						.set(VtnServiceJsonConsts.DOMAINID,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vByPass.get(
														VtnServiceJsonConsts.DOMAINID)
														.getAsString(),
												valValVunknownStruct,
												UncStructIndexEnum.ValVunknownIndex.UPLL_IDX_DOMAIN_ID_VUN
														.ordinal()));
			} else {
				valValVunknownStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVunknownIndex.UPLL_IDX_DOMAIN_ID_VUN
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (vByPass.has(VtnServiceJsonConsts.CONTROLLERID)) {
				valValVunknownStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVunknownIndex.UPLL_IDX_CONTROLLER_ID_VUN
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valValVunknownStruct
						.set(VtnServiceJsonConsts.CONTROLLERID,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vByPass.get(
														VtnServiceJsonConsts.CONTROLLERID)
														.getAsString(),
												valValVunknownStruct,
												UncStructIndexEnum.ValVunknownIndex.UPLL_IDX_CONTROLLER_ID_VUN
														.ordinal()));
			} else {
				valValVunknownStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVunknownIndex.UPLL_IDX_CONTROLLER_ID_VUN
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
		} else {
			LOG.warning("request body and uri parameters are not correct for getValVunknownStruct");
		}
		LOG.info("Value Structure: " + valValVunknownStruct.toString());
		LOG.trace("Complete getValVunknownStruct");
		return valValVunknownStruct;
	}

	/**
	 * Gets the val vunk if struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the val vunk if struct
	 */
	public final IpcStruct getValVunkIfStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		// Lower level structure
		/*
		 * ipc_struct val_vunk_if { UINT8 valid[2]; UINT8 cs_row_status; UINT8
		 * cs_attr[1]; UINT8 description[128]; UINT8 admin_status; };
		 * 
		 * enum val_vunk_if_index { UPLL_IDX_DESC_VUNI = 0,
		 * UPLL_IDX_ADMIN_ST_VUNI };
		 */
		LOG.trace("Start getValVunkIfStruct");
		final IpcStruct valVunkIfStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValVunkIf.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.INTERFACE)) {
			final JsonObject vunkIf = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.INTERFACE);
			if (vunkIf.has(VtnServiceJsonConsts.DESCRIPTION)) {
				valVunkIfStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVunkIfIndex.UPLL_IDX_DESC_VUNI
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVunkIfStruct
						.set(VtnServiceJsonConsts.DESCRIPTION,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vunkIf.get(
														VtnServiceJsonConsts.DESCRIPTION)
														.getAsString(),
												valVunkIfStruct,
												UncStructIndexEnum.ValVunkIfIndex.UPLL_IDX_DESC_VUNI
														.ordinal()));
			} else {
				valVunkIfStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVunkIfIndex.UPLL_IDX_DESC_VUNI
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (vunkIf.has(VtnServiceJsonConsts.ADMINSTATUS)) {
				valVunkIfStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVunkIfIndex.UPLL_IDX_ADMIN_ST_VUNI
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				if (vunkIf.get(VtnServiceJsonConsts.ADMINSTATUS).getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.ENABLE)) {
					valVunkIfStruct
							.set(VtnServiceJsonConsts.ADMIN_STATUS,
									IpcDataUnitWrapper
											.setIpcUint8Value(
													UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_ENABLE
															.getValue(),
													valVunkIfStruct,
													UncStructIndexEnum.ValVunkIfIndex.UPLL_IDX_ADMIN_ST_VUNI
															.ordinal()));
				} else {
					valVunkIfStruct
							.set(VtnServiceJsonConsts.ADMIN_STATUS,
									IpcDataUnitWrapper
											.setIpcUint8Value(
													UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_DISABLE
															.getValue(),
													valVunkIfStruct,
													UncStructIndexEnum.ValVunkIfIndex.UPLL_IDX_ADMIN_ST_VUNI
															.ordinal()));
				}
				LOG.debug("adminstatus:"
						+ vunkIf.get(VtnServiceJsonConsts.ADMINSTATUS)
								.getAsString());
			} else {
				valVunkIfStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVunkIfIndex.UPLL_IDX_ADMIN_ST_VUNI
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
		} else {
			LOG.warning("request body and uri parameters are not correct for getValVunkIfStruct");
		}
		LOG.info("Value Structure: " + valVunkIfStruct.toString());
		LOG.trace("Complete getValVunkIfStruct");
		return valVunkIfStruct;
	}

	/**
	 * Gets the struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the struct
	 */
	public final IpcStruct getKeyVbrFlowFilterStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		// Lower level structure
		/*
		 * ipc_struct KeyVbrFlowFilter { keyVbr vbr_key; UINT8 direction; };
		 */
		LOG.trace("Start getKeyVbrFlowFilterStruct");
		final IpcStruct keyVbrFlowFilterStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyVbrFlowFilter.getValue());
		IpcStruct keyVbrStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.TWO.ordinal()) {
			keyVbrStruct = getKeyVbrStruct(requestBody,
					uriParameters.subList(0, 2));
		}
		keyVbrFlowFilterStruct.set(VtnServiceIpcConsts.VBRKEY, keyVbrStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.FLOWFILTER)
				&& ((JsonObject) requestBody
						.get(VtnServiceJsonConsts.FLOWFILTER))
						.has(VtnServiceJsonConsts.FFTYPE)) {
			if (requestBody.getAsJsonObject(VtnServiceJsonConsts.FLOWFILTER)
					.get(VtnServiceJsonConsts.FFTYPE).getAsString()
					.equalsIgnoreCase(VtnServiceJsonConsts.IN)) {
				keyVbrFlowFilterStruct
						.set(VtnServiceIpcConsts.DIRECTION,
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_IN
												.ordinal()));
			} else {
				keyVbrFlowFilterStruct
						.set(VtnServiceIpcConsts.DIRECTION,
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_OUT
												.ordinal()));
			}
			LOG.debug("ff_type:"
					+ requestBody
							.getAsJsonObject(VtnServiceJsonConsts.FLOWFILTER)
							.get(VtnServiceJsonConsts.FFTYPE).getAsString());
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.THREE.ordinal()) {
			if (uriParameters.get(2).equalsIgnoreCase(VtnServiceJsonConsts.IN)) {
				keyVbrFlowFilterStruct
						.set(VtnServiceIpcConsts.DIRECTION,
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_IN
												.ordinal()));
			} else {
				keyVbrFlowFilterStruct
						.set(VtnServiceIpcConsts.DIRECTION,
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_OUT
												.ordinal()));
			}
			LOG.debug("ff_type:" + uriParameters.get(2));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVbrFlowFilterStruct");
		}
		LOG.info("Key Structure: " + keyVbrFlowFilterStruct.toString());
		LOG.trace("Complete getKeyVbrFlowFilterStruct");

		return keyVbrFlowFilterStruct;
	}

	/**
	 * Gets the KeyVbrFlowFilterEntry struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the struct
	 */
	public final IpcStruct getKeyVbrFlowFilterEntryStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		/*
		 * ipc_struct key_vbr_flowfilter_entry { key_vbr_flowfilter
		 * flowfilter_key; UINT16 sequence_num; };
		 */
		LOG.trace("Start getKeyVbrFlowFilterEntryStruct");
		final IpcStruct keyVbrFlowFilterEntryStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyVbrFlowFilterEntry
						.getValue());
		IpcStruct keyVbrFlowFilterStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.THREE.ordinal()) {
			keyVbrFlowFilterStruct = getKeyVbrFlowFilterStruct(requestBody,
					uriParameters.subList(0, 3));
		}
		keyVbrFlowFilterEntryStruct.set(VtnServiceIpcConsts.VBRFLOWFILTER,
				keyVbrFlowFilterStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.FLOWFILTERENTRY)
				&& ((JsonObject) requestBody
						.get(VtnServiceJsonConsts.FLOWFILTERENTRY))
						.has(VtnServiceJsonConsts.SEQNUM)) {
			keyVbrFlowFilterEntryStruct.set(VtnServiceJsonConsts.SEQUENCENUM,
					IpcDataUnitWrapper
							.setIpcUint16Value(((JsonObject) requestBody
									.get(VtnServiceJsonConsts.FLOWFILTERENTRY))
									.get(VtnServiceJsonConsts.SEQNUM)
									.getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.FOUR.ordinal()) {
			keyVbrFlowFilterEntryStruct.set(VtnServiceJsonConsts.SEQUENCENUM,
					IpcDataUnitWrapper.setIpcUint16Value(uriParameters.get(3)));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVbrFlowFilterEntryStruct");
		}
		LOG.info("Key Structure: " + keyVbrFlowFilterEntryStruct.toString());
		LOG.trace("Complete getKeyVbrFlowFilterEntryStruct");
		return keyVbrFlowFilterEntryStruct;
	}

	/**
	 * Gets the val FlowfilterEntry struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the valflowfilterEntry struct
	 */
	public final IpcStruct getValFlowfilterEntryStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		LOG.trace("Start getValFlowfilterEntryStruct");
		final IpcStruct valFlowfilterEntryStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValFlowfilterEntry.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.FLOWFILTERENTRY)) {
			final JsonObject flowFilterEntry = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.FLOWFILTERENTRY);
			if (flowFilterEntry.has(VtnServiceJsonConsts.FLNAME)) {
				valFlowfilterEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_FLOWLIST_NAME_FFE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valFlowfilterEntryStruct
						.set(VtnServiceJsonConsts.FLOWLISTNAME,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												flowFilterEntry
														.get(VtnServiceJsonConsts.FLNAME)
														.getAsString(),
												valFlowfilterEntryStruct,
												UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_FLOWLIST_NAME_FFE
														.ordinal()));
			} else {
				valFlowfilterEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_FLOWLIST_NAME_FFE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (flowFilterEntry.has(VtnServiceJsonConsts.ACTIONTYPE)) {
				valFlowfilterEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_ACTION_FFE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				if (flowFilterEntry.get(VtnServiceJsonConsts.ACTIONTYPE)
						.getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.PASS)) {
					valFlowfilterEntryStruct
							.set(VtnServiceIpcConsts.ACTION,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.FlowfilterAction.UPLL_FLOWFILTER_ACT_PASS
													.ordinal()));
				} else if (flowFilterEntry.get(VtnServiceJsonConsts.ACTIONTYPE)
						.getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.DROP)) {
					valFlowfilterEntryStruct
							.set(VtnServiceIpcConsts.ACTION,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.FlowfilterAction.UPLL_FLOWFILTER_ACT_DROP
													.ordinal()));
				} else if (flowFilterEntry.get(VtnServiceJsonConsts.ACTIONTYPE)
						.getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.REDIRECT)) {
					valFlowfilterEntryStruct
							.set(VtnServiceIpcConsts.ACTION,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.FlowfilterAction.UPLL_FLOWFILTER_ACT_REDIRECT
													.ordinal()));
				} else {
					valFlowfilterEntryStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_ACTION_FFE
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
													.ordinal()));
				}
				LOG.debug("action_type:"
						+ flowFilterEntry.get(VtnServiceJsonConsts.ACTIONTYPE)
								.getAsString());
			} else {
				valFlowfilterEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_ACTION_FFE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (flowFilterEntry.has(VtnServiceJsonConsts.REDIRECTDST)) {
				final JsonObject redirectDst = flowFilterEntry
						.getAsJsonObject(VtnServiceJsonConsts.REDIRECTDST);
				if (redirectDst.has(VtnServiceJsonConsts.VNODENAME)) {
					valFlowfilterEntryStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_REDIRECT_NODE_FFE
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
													.ordinal()));
					valFlowfilterEntryStruct
							.set(VtnServiceIpcConsts.REDIRECTNODE,
									IpcDataUnitWrapper
											.setIpcUint8ArrayValue(
													redirectDst
															.get(VtnServiceJsonConsts.VNODENAME)
															.getAsString(),
													valFlowfilterEntryStruct,
													UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_REDIRECT_NODE_FFE
															.ordinal()));
				} else {
					valFlowfilterEntryStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_REDIRECT_NODE_FFE
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
													.ordinal()));
				}
				if (redirectDst.has(VtnServiceJsonConsts.IFNAME)) {
					valFlowfilterEntryStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_REDIRECT_PORT_FFE
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
													.ordinal()));
					valFlowfilterEntryStruct
							.set(VtnServiceIpcConsts.REDIRECTPORT,
									IpcDataUnitWrapper
											.setIpcUint8ArrayValue(
													redirectDst
															.get(VtnServiceJsonConsts.IFNAME)
															.getAsString(),
													valFlowfilterEntryStruct,
													UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_REDIRECT_PORT_FFE
															.ordinal()));
				} else {
					valFlowfilterEntryStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_REDIRECT_PORT_FFE
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
													.ordinal()));
				}
				if (redirectDst.has(VtnServiceJsonConsts.DIRECTION)) {
					valFlowfilterEntryStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_REDIRECT_DIRECTION_FFE
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
													.ordinal()));

					if (redirectDst.get(VtnServiceJsonConsts.DIRECTION)
							.getAsString()
							.equalsIgnoreCase(VtnServiceJsonConsts.IN)) {
						valFlowfilterEntryStruct
								.set(VtnServiceIpcConsts.REDIRECTDIRECTION,
										IpcDataUnitWrapper
												.setIpcUint8Value(
														UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_IN
																.getValue(),
														valFlowfilterEntryStruct,
														UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_REDIRECT_DIRECTION_FFE
																.ordinal()));
					} else if (redirectDst.get(VtnServiceJsonConsts.DIRECTION)
							.getAsString()
							.equalsIgnoreCase(VtnServiceJsonConsts.OUT)) {
						valFlowfilterEntryStruct
								.set(VtnServiceIpcConsts.REDIRECTDIRECTION,
										IpcDataUnitWrapper
												.setIpcUint8Value(
														UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_OUT
																.getValue(),
														valFlowfilterEntryStruct,
														UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_REDIRECT_DIRECTION_FFE
																.ordinal()));
					} else {
						valFlowfilterEntryStruct
								.set(VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_REDIRECT_DIRECTION_FFE
												.ordinal(),
										IpcDataUnitWrapper
												.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
														.ordinal()));
					}
					LOG.debug("Direction:"
							+ redirectDst.get(VtnServiceJsonConsts.DIRECTION)
									.getAsString());

				}
				if (redirectDst.has(VtnServiceJsonConsts.MACDSTADDR)) {
					valFlowfilterEntryStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_MODIFY_DST_MAC_FFE
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
													.ordinal()));
					// valFlowfilterEntryStruct.set(VtnServiceIpcConsts.MODIFYDSTMACADDR,IpcDataUnitWrapper.setIpcUint8ArrayValue(requestBody.getAsJsonObject(VtnServiceJsonConsts.FLOWFILTERENTRY).getAsJsonObject(VtnServiceJsonConsts.REDIRECTDST).get(VtnServiceJsonConsts.MACDSTADDR).getAsString()));
					IpcDataUnitWrapper
							.setMacAddress(
									valFlowfilterEntryStruct,
									VtnServiceIpcConsts.MODIFYDSTMACADDR,
									redirectDst.get(
											VtnServiceJsonConsts.MACDSTADDR)
											.getAsString(),
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_MODIFY_DST_MAC_FFE
											.ordinal());
				} else {
					valFlowfilterEntryStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_MODIFY_DST_MAC_FFE
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
													.ordinal()));
				}
				if (redirectDst.has(VtnServiceJsonConsts.MACSRCADDR)) {
					valFlowfilterEntryStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_MODIFY_SRC_MAC_FFE
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
													.ordinal()));
					IpcDataUnitWrapper
							.setMacAddress(
									valFlowfilterEntryStruct,
									VtnServiceIpcConsts.MODIFYSRCMACADDR,
									redirectDst.get(
											VtnServiceJsonConsts.MACSRCADDR)
											.getAsString(),
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_MODIFY_SRC_MAC_FFE
											.ordinal());
				} else {
					valFlowfilterEntryStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_MODIFY_SRC_MAC_FFE
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
													.ordinal()));
				}
			}
			if (flowFilterEntry.has(VtnServiceJsonConsts.NMGNAME)) {
				valFlowfilterEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_NWM_NAME_FFE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valFlowfilterEntryStruct
						.set(VtnServiceIpcConsts.NWMNAME,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												flowFilterEntry
														.get(VtnServiceJsonConsts.NMGNAME)
														.getAsString(),
												valFlowfilterEntryStruct,
												UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_NWM_NAME_FFE
														.ordinal()));
			} else {
				valFlowfilterEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_NWM_NAME_FFE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (flowFilterEntry.has(VtnServiceJsonConsts.DSCP)) {
				valFlowfilterEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_DSCP_FFE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valFlowfilterEntryStruct
						.set(VtnServiceIpcConsts.DSCP,
								IpcDataUnitWrapper
										.setIpcUint8Value(
												flowFilterEntry
														.get(VtnServiceJsonConsts.DSCP)
														.getAsString(),
												valFlowfilterEntryStruct,
												UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_DSCP_FFE
														.ordinal()));
			} else {
				valFlowfilterEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_DSCP_FFE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (flowFilterEntry.has(VtnServiceJsonConsts.PRIORITY)) {
				valFlowfilterEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_PRIORITY_FFE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valFlowfilterEntryStruct
						.set(VtnServiceIpcConsts.PRIORITY,
								IpcDataUnitWrapper
										.setIpcUint8Value(
												flowFilterEntry
														.get(VtnServiceJsonConsts.PRIORITY)
														.getAsString(),
												valFlowfilterEntryStruct,
												UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_PRIORITY_FFE
														.ordinal()));
			} else {
				valFlowfilterEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_PRIORITY_FFE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
		} else {
			LOG.warning("request body and uri parameters are not correct for getValFlowfilterEntryStruct");
		}
		LOG.info("Value Structure: " + valFlowfilterEntryStruct.toString());
		LOG.trace("Complete getValFlowfilterEntryStruct");
		return valFlowfilterEntryStruct;
	}

	/**
	 * Gets the KeyVbrIfStruct struct for VBridgeInterface APIs.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the struct
	 */
	public final IpcStruct getKeyVbrIfStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		// Lower level structure
		/*
		 * ipc_struct key_vbr_if { key_vbr vbr_key; UINT8 if_name[32]; };
		 */
		LOG.trace("Start getKeyVbrIfStruct");
		final IpcStruct keyVbrIfStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyVbrIf.getValue());
		IpcStruct keyVbrStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.TWO.ordinal()) {
			keyVbrStruct = getKeyVbrStruct(requestBody,
					uriParameters.subList(0, 2));
		}
		keyVbrIfStruct.set(VtnServiceIpcConsts.VBRKEY, keyVbrStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.INTERFACE)
				&& ((JsonObject) requestBody
						.get(VtnServiceJsonConsts.INTERFACE))
						.has(VtnServiceJsonConsts.IFNAME)) {
			keyVbrIfStruct.set(VtnServiceJsonConsts.IFNAME, IpcDataUnitWrapper
					.setIpcUint8ArrayValue(((JsonObject) requestBody
							.get(VtnServiceJsonConsts.INTERFACE)).get(
							VtnServiceJsonConsts.IFNAME).getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.THREE.ordinal()) {
			keyVbrIfStruct.set(VtnServiceJsonConsts.IFNAME, IpcDataUnitWrapper
					.setIpcUint8ArrayValue(uriParameters.get(2)));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVbrIfStruct");
		}
		LOG.info("Key Structure: " + keyVbrIfStruct.toString());
		LOG.trace("Complete getKeyVbrIfStruct");
		return keyVbrIfStruct;
	}

	/**
	 * Gets the val PortMap struct for VBridgeInterface APIs
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the valPortMapStruct
	 */
	public final IpcStruct getValPortMapStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		/*
		 * ipc_struct val_port_map { UINT8 valid[5]; UINT8 cs_attr[5]; UINT8
		 * switch_id[256]; UINT16 port_type; UINT8 port_name[32]; UINT16
		 * vlan_id; UINT8 tagged; }
		 */
		LOG.trace("Start getValPortMapStruct");
		final IpcStruct valPortMapStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValPortMap.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.PORTMAP)) {
			final JsonObject portMap = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.PORTMAP);
			if (portMap.has(VtnServiceJsonConsts.LOGICAL_PORT_ID)) {
				valPortMapStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_LOGICAL_PORT_ID_PM
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valPortMapStruct
						.set(VtnServiceJsonConsts.LOGICAL_PORT_ID,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												portMap.get(
														VtnServiceJsonConsts.LOGICAL_PORT_ID)
														.getAsString(),
												valPortMapStruct,
												UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_LOGICAL_PORT_ID_PM
														.ordinal()));
			} else {
				valPortMapStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_LOGICAL_PORT_ID_PM
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (portMap.has(VtnServiceJsonConsts.VLANID)) {
				valPortMapStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_VLAN_ID_PM
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valPortMapStruct
						.set(VtnServiceJsonConsts.VLANID,
								IpcDataUnitWrapper
										.setIpcUint16Value(
												portMap.get(
														VtnServiceJsonConsts.VLANID)
														.getAsString(),
												valPortMapStruct,
												UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_VLAN_ID_PM
														.ordinal()));
			} else {
				valPortMapStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_VLAN_ID_PM
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (portMap.has(VtnServiceJsonConsts.TAGGED)) {
				valPortMapStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_TAGGED_PM
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				if (portMap.get(VtnServiceJsonConsts.TAGGED).getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.TRUE)) {
					valPortMapStruct
							.set(VtnServiceJsonConsts.TAGGED,
									IpcDataUnitWrapper
											.setIpcUint8Value(
													UncStructIndexEnum.vlan_tagged.UPLL_VLAN_TAGGED
															.getValue(),
													valPortMapStruct,
													UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_TAGGED_PM
															.ordinal()));
				} else if (portMap.get(VtnServiceJsonConsts.TAGGED)
						.getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.FALSE)) {
					valPortMapStruct
							.set(VtnServiceJsonConsts.TAGGED,
									IpcDataUnitWrapper
											.setIpcUint8Value(
													UncStructIndexEnum.vlan_tagged.UPLL_VLAN_UNTAGGED
															.getValue(),
													valPortMapStruct,
													UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_TAGGED_PM
															.ordinal()));
				} else {
					valPortMapStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_TAGGED_PM
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
													.ordinal()));
				}
				LOG.debug("tagged:"
						+ portMap.get(VtnServiceJsonConsts.TAGGED)
								.getAsString());
			} else {
				valPortMapStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_TAGGED_PM
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
		} else {
			LOG.warning("request body and uri parameters are not correct for getValPortMapStruct");
		}
		LOG.info("Value Structure: " + valPortMapStruct.toString());
		LOG.trace("Complete getValPortMapStruct");
		return valPortMapStruct;
	}

	/**
	 * Gets the val vbr If struct for for VBridgeInterface APIs
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the valVbrIfStruct
	 */
	public final IpcStruct getValVbrIfStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		/*
		 * ipc_struct val_vbr_if { UINT8 valid[3]; UINT8 admin_status; UINT8
		 * description[128]; UINT8 cs_row_status; UINT8 cs_attr[2]; ipc_struct
		 * val_port_map { UINT8 valid[5]; UINT8 cs_attr[5]; UINT8
		 * switch_id[256]; UINT16 port_type; UINT8 port_name[32]; UINT16
		 * vlan_id; UINT8 tagged; }portmap;};
		 */
		LOG.trace("Start getValVbrIfStruct");
		final IpcStruct valVbrIfStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValVbrIf.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.INTERFACE)) {
			final JsonObject vbrIf = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.INTERFACE);
			if (vbrIf.has(VtnServiceJsonConsts.ADMINSTATUS)) {
				valVbrIfStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVbrIfIndex.UPLL_IDX_ADMIN_STATUS_VBRI
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				if (vbrIf.get(VtnServiceJsonConsts.ADMINSTATUS).getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.ENABLE)) {
					valVbrIfStruct
							.set(VtnServiceJsonConsts.ADMIN_STATUS,
									IpcDataUnitWrapper
											.setIpcUint8Value(
													UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_ENABLE
															.getValue(),
													valVbrIfStruct,
													UncStructIndexEnum.ValVbrIfIndex.UPLL_IDX_ADMIN_STATUS_VBRI
															.ordinal()));
				} else if (vbrIf.get(VtnServiceJsonConsts.ADMINSTATUS)
						.getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.DISABLE)) {
					valVbrIfStruct
							.set(VtnServiceJsonConsts.ADMIN_STATUS,
									IpcDataUnitWrapper
											.setIpcUint8Value(
													UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_DISABLE
															.getValue(),
													valVbrIfStruct,
													UncStructIndexEnum.ValVbrIfIndex.UPLL_IDX_ADMIN_STATUS_VBRI
															.ordinal()));
				} else {
					valVbrIfStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVbrIfIndex.UPLL_IDX_ADMIN_STATUS_VBRI
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
													.ordinal()));
				}
				LOG.debug("adminstatus"
						+ vbrIf.get(VtnServiceJsonConsts.ADMINSTATUS)
								.getAsString());
			} else {
				valVbrIfStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVbrIfIndex.UPLL_IDX_ADMIN_STATUS_VBRI
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (vbrIf.has(VtnServiceJsonConsts.DESCRIPTION)) {
				valVbrIfStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVbrIfIndex.UPLL_IDX_DESC_VBRI
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVbrIfStruct
						.set(VtnServiceJsonConsts.DESCRIPTION,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vbrIf.get(
														VtnServiceJsonConsts.DESCRIPTION)
														.getAsString(),
												valVbrIfStruct,
												UncStructIndexEnum.ValVbrIfIndex.UPLL_IDX_DESC_VBRI
														.ordinal()));
			} else {
				valVbrIfStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVbrIfIndex.UPLL_IDX_DESC_VBRI
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			valVbrIfStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVbrIfIndex.UPLL_IDX_PM_VBRI
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
		} else if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.PORTMAP)) {
			IpcStruct valPortMapStruct = null;
			valVbrIfStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVbrIfIndex.UPLL_IDX_ADMIN_STATUS_VBRI
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			valVbrIfStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVbrIfIndex.UPLL_IDX_DESC_VBRI
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			valVbrIfStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVbrIfIndex.UPLL_IDX_PM_VBRI
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
											.ordinal()));
			valPortMapStruct = getValPortMapStruct(requestBody, uriParameters);
			valVbrIfStruct.set(VtnServiceIpcConsts.PORTMAP, valPortMapStruct);
		} else if (requestBody == null) {
			final IpcStruct valPortMapStruct = IpcDataUnitWrapper
					.setIpcStructValue(UncStructEnum.ValPortMap.getValue());
			valVbrIfStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVbrIfIndex.UPLL_IDX_ADMIN_STATUS_VBRI
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			valVbrIfStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVbrIfIndex.UPLL_IDX_DESC_VBRI
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			valVbrIfStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVbrIfIndex.UPLL_IDX_PM_VBRI
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
											.ordinal()));
			valPortMapStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_LOGICAL_PORT_ID_PM
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
											.ordinal()));
			valPortMapStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_VLAN_ID_PM
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
											.ordinal()));
			valPortMapStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_TAGGED_PM
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
											.ordinal()));
			valVbrIfStruct.set(VtnServiceIpcConsts.PORTMAP, valPortMapStruct);
		} else {
			LOG.warning("request body and uri parameters are not correct for getValVbrIfStruct");
		}
		LOG.info("Value Structure: " + valVbrIfStruct.toString());
		LOG.trace("Complete getValVbrIfStruct");

		return valVbrIfStruct;
	}

	/**
	 * Gets the val vrt struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the val vrt struct
	 */
	public final IpcStruct getValVrtStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		// Lower level structure
		/*
		 * ipc_struct val_vrt { UINT8 valid[43]; UINT8 cs_row_status; UINT8
		 * cs_attr[34]; UINT8 controller_id[32]; UINT8 domain_id[32] UINT8
		 * vrt_description[128]; UINT8 dhcp_relay_admin_status; };
		 */
		LOG.trace("Start getValVrtStruct");
		final IpcStruct valVrtStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValVrt.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.VROUTER)) {
			final JsonObject vRouter = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VROUTER);
			if (vRouter.has(VtnServiceJsonConsts.CONTROLLERID)) {
				valVrtStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtIndex.UPLL_IDX_CONTROLLER_ID_VRT
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVrtStruct
						.set(VtnServiceJsonConsts.CONTROLLERID,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vRouter.get(
														VtnServiceJsonConsts.CONTROLLERID)
														.getAsString(),
												valVrtStruct,
												UncStructIndexEnum.ValVrtIndex.UPLL_IDX_CONTROLLER_ID_VRT
														.ordinal()));
			} else {
				valVrtStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtIndex.UPLL_IDX_CONTROLLER_ID_VRT
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (vRouter.has(VtnServiceJsonConsts.DOMAINID)) {
				valVrtStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtIndex.UPLL_IDX_DOMAIN_ID_VRT
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVrtStruct
						.set(VtnServiceJsonConsts.DOMAINID,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vRouter.get(
														VtnServiceJsonConsts.DOMAINID)
														.getAsString(),
												valVrtStruct,
												UncStructIndexEnum.ValVrtIndex.UPLL_IDX_DOMAIN_ID_VRT
														.ordinal()));
			} else {
				valVrtStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtIndex.UPLL_IDX_DOMAIN_ID_VRT
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (vRouter.has(VtnServiceJsonConsts.DESCRIPTION)) {
				valVrtStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtIndex.UPLL_IDX_DESC_VRT
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVrtStruct
						.set(VtnServiceIpcConsts.VRTDESCRIPTION,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vRouter.get(
														VtnServiceJsonConsts.DESCRIPTION)
														.getAsString(),
												valVrtStruct,
												UncStructIndexEnum.ValVrtIndex.UPLL_IDX_DESC_VRT
														.ordinal()));
			} else {
				valVrtStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtIndex.UPLL_IDX_DESC_VRT
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			valVrtStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVrtIndex.UPLL_IDX_DHCP_RELAY_ADMIN_STATUS_VRT
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
		} else if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.DHCPRELAY)) {
			valVrtStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVrtIndex.UPLL_IDX_CONTROLLER_ID_VRT
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			valVrtStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVrtIndex.UPLL_IDX_DESC_VRT
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			valVrtStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVrtIndex.UPLL_IDX_DOMAIN_ID_VRT
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			if (requestBody.getAsJsonObject(VtnServiceJsonConsts.DHCPRELAY)
					.has(VtnServiceJsonConsts.DHCPRELAYSTATUS)) {
				valVrtStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtIndex.UPLL_IDX_DHCP_RELAY_ADMIN_STATUS_VRT
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				if (requestBody.getAsJsonObject(VtnServiceJsonConsts.DHCPRELAY)
						.get(VtnServiceJsonConsts.DHCPRELAYSTATUS)
						.getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.ENABLE)) {
					valVrtStruct
							.set(VtnServiceIpcConsts.DHCPRELAYADMINSTATUS,
									IpcDataUnitWrapper
											.setIpcUint8Value(
													UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_ENABLE
															.getValue(),
													valVrtStruct,
													UncStructIndexEnum.ValVrtIndex.UPLL_IDX_DHCP_RELAY_ADMIN_STATUS_VRT
															.ordinal()));
				} else {
					valVrtStruct
							.set(VtnServiceIpcConsts.DHCPRELAYADMINSTATUS,
									IpcDataUnitWrapper
											.setIpcUint8Value(
													UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_DISABLE
															.getValue(),
													valVrtStruct,
													UncStructIndexEnum.ValVrtIndex.UPLL_IDX_DHCP_RELAY_ADMIN_STATUS_VRT
															.ordinal()));
				}
				LOG.debug("dhcp_relay_status"
						+ requestBody
								.getAsJsonObject(VtnServiceJsonConsts.DHCPRELAY)
								.get(VtnServiceJsonConsts.DHCPRELAYSTATUS)
								.getAsString());
			} else {
				valVrtStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtIndex.UPLL_IDX_DHCP_RELAY_ADMIN_STATUS_VRT
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
		} else {
			LOG.warning("request body and uri parameters are not correct for getValVrtStruct");
		}
		LOG.info("Value Structure: " + valVrtStruct.toString());
		LOG.trace("Complete getValVrtStruct");
		return valVrtStruct;
	}

	/**
	 * Gets the val vrt if struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the val vrt if struct
	 */
	public final IpcStruct getValVrtIfStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		// Lower level structure
		/*
		 * ipc_struct val_vrt_if { UINT8 valid[5]; UINT8 cs_row_status; UINT8
		 * cs_attr[5]; UINT8 description[128]; IPV4 ip_addr; UINT8 prefixlen;
		 * UINT8 macaddr[6]; UINT8 admin_status; };
		 */
		LOG.trace("Start getValVrtIfStruct");
		final IpcStruct valVrtIfStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValVrtIf.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.INTERFACE)) {
			final JsonObject vRouterIf = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.INTERFACE);
			if (vRouterIf.has(VtnServiceJsonConsts.DESCRIPTION)) {
				valVrtIfStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtIfIndex.UPLL_IDX_DESC_VI
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVrtIfStruct
						.set(VtnServiceJsonConsts.DESCRIPTION,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vRouterIf
														.get(VtnServiceJsonConsts.DESCRIPTION)
														.getAsString(),
												valVrtIfStruct,
												UncStructIndexEnum.ValVrtIfIndex.UPLL_IDX_DESC_VI
														.ordinal()));
			} else {
				valVrtIfStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtIfIndex.UPLL_IDX_DESC_VI
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (vRouterIf.has(VtnServiceJsonConsts.IPADDR)) {
				valVrtIfStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtIfIndex.UPLL_IDX_IP_ADDR_VI
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVrtIfStruct
						.set(VtnServiceIpcConsts.IP_ADDR,
								IpcDataUnitWrapper
										.setIpcInet4AddressValue(
												vRouterIf
														.get(VtnServiceJsonConsts.IPADDR)
														.getAsString(),
												valVrtIfStruct,
												UncStructIndexEnum.ValVrtIfIndex.UPLL_IDX_IP_ADDR_VI
														.ordinal()));
			} else {
				valVrtIfStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtIfIndex.UPLL_IDX_IP_ADDR_VI
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (vRouterIf.has(VtnServiceJsonConsts.PREFIX)) {
				valVrtIfStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtIfIndex.UPLL_IDX_PREFIXLEN_VI
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVrtIfStruct
						.set(VtnServiceIpcConsts.PREFIXLEN,
								IpcDataUnitWrapper
										.setIpcUint8Value(
												vRouterIf
														.get(VtnServiceJsonConsts.PREFIX)
														.getAsString(),
												valVrtIfStruct,
												UncStructIndexEnum.ValVrtIfIndex.UPLL_IDX_PREFIXLEN_VI
														.ordinal()));
			} else {
				valVrtIfStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtIfIndex.UPLL_IDX_PREFIXLEN_VI
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (vRouterIf.has(VtnServiceJsonConsts.MACADDR)) {
				valVrtIfStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtIfIndex.UPLL_IDX_MAC_ADDR_VI
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));

				IpcDataUnitWrapper.setMacAddress(valVrtIfStruct,
						VtnServiceIpcConsts.MACADDR,
						vRouterIf.get(VtnServiceJsonConsts.MACADDR)
								.getAsString(),
						UncStructIndexEnum.ValVrtIfIndex.UPLL_IDX_MAC_ADDR_VI
								.ordinal());
			} else {
				valVrtIfStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtIfIndex.UPLL_IDX_MAC_ADDR_VI
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (vRouterIf.has(VtnServiceJsonConsts.ADMINSTATUS)) {
				valVrtIfStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtIfIndex.UPLL_IDX_ADMIN_ST_VI
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				if (vRouterIf.get(VtnServiceJsonConsts.ADMINSTATUS)
						.getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.ENABLE)) {
					valVrtIfStruct
							.set(VtnServiceJsonConsts.ADMIN_STATUS,
									IpcDataUnitWrapper
											.setIpcUint8Value(
													UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_ENABLE
															.getValue(),
													valVrtIfStruct,
													UncStructIndexEnum.ValVrtIfIndex.UPLL_IDX_ADMIN_ST_VI
															.ordinal()));
				} else if (vRouterIf.get(VtnServiceJsonConsts.ADMINSTATUS)
						.getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.DISABLE)) {
					valVrtIfStruct
							.set(VtnServiceJsonConsts.ADMIN_STATUS,
									IpcDataUnitWrapper
											.setIpcUint8Value(
													UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_DISABLE
															.getValue(),
													valVrtIfStruct,
													UncStructIndexEnum.ValVrtIfIndex.UPLL_IDX_ADMIN_ST_VI
															.ordinal()));
				} else {
					valVrtIfStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVrtIfIndex.UPLL_IDX_ADMIN_ST_VI
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
													.ordinal()));
				}
				LOG.debug("adminstatus:"
						+ vRouterIf.get(VtnServiceJsonConsts.ADMINSTATUS)
								.getAsString());
			} else {
				valVrtIfStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtIfIndex.UPLL_IDX_ADMIN_ST_VI
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
		} else {
			LOG.warning("request body and uri parameters are not correct for getValVrtIfStruct");
		}
		LOG.info("Value Structure: " + valVrtIfStruct.toString());
		LOG.trace("Complete getValVrtIfStruct");
		return valVrtIfStruct;
	}

	/**
	 * Gets the val vtunnel struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the val vtunnel struct
	 */
	public final IpcStruct getValVtunnelStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		// Lower level structure
		/*
		 * ipc_struct val_vtunnel { UINT8 valid[6]; UINT8 cs_row_status; UINT8
		 * cs_attr[6]; UINT8 description[128]; UINT8 controller_id[32]; UINT8
		 * domain_id[32] UINT8 vtn_name[32]; UINT8 vtep_grp_name[32]; UINT32
		 * label; };
		 */
		LOG.trace("Start getValVtunnelStruct");
		final IpcStruct valVtunnelStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValVtunnel.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.VTUNNEL)) {
			final JsonObject vTunnel = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VTUNNEL);
			if (vTunnel.has(VtnServiceJsonConsts.DESCRIPTION)) {
				valVtunnelStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtunnelIndex.UPLL_IDX_DESC_VTNL
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVtunnelStruct
						.set(VtnServiceJsonConsts.DESCRIPTION,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vTunnel.get(
														VtnServiceJsonConsts.DESCRIPTION)
														.getAsString(),
												valVtunnelStruct,
												UncStructIndexEnum.ValVtunnelIndex.UPLL_IDX_DESC_VTNL
														.ordinal()));
			} else {
				valVtunnelStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtunnelIndex.UPLL_IDX_DESC_VTNL
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (vTunnel.has(VtnServiceJsonConsts.CONTROLLERID)) {
				valVtunnelStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtunnelIndex.UPLL_IDX_CONTROLLER_ID_VTNL
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVtunnelStruct
						.set(VtnServiceJsonConsts.CONTROLLERID,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vTunnel.get(
														VtnServiceJsonConsts.CONTROLLERID)
														.getAsString(),
												valVtunnelStruct,
												UncStructIndexEnum.ValVtunnelIndex.UPLL_IDX_CONTROLLER_ID_VTNL
														.ordinal()));
			} else {
				valVtunnelStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtunnelIndex.UPLL_IDX_CONTROLLER_ID_VTNL
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (vTunnel.has(VtnServiceJsonConsts.DOMAINID)) {
				valVtunnelStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtunnelIndex.UPLL_IDX_DOMAIN_ID_VTNL
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVtunnelStruct
						.set(VtnServiceJsonConsts.DOMAINID,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vTunnel.get(
														VtnServiceJsonConsts.DOMAINID)
														.getAsString(),
												valVtunnelStruct,
												UncStructIndexEnum.ValVtunnelIndex.UPLL_IDX_DOMAIN_ID_VTNL
														.ordinal()));
			} else {
				valVtunnelStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtunnelIndex.UPLL_IDX_DOMAIN_ID_VTNL
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (vTunnel.has(VtnServiceJsonConsts.VTNNAME)) {
				valVtunnelStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtunnelIndex.UPLL_IDX_VTN_NAME_VTNL
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVtunnelStruct
						.set(VtnServiceIpcConsts.VTNNAME,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vTunnel.get(
														VtnServiceJsonConsts.VTNNAME)
														.getAsString(),
												valVtunnelStruct,
												UncStructIndexEnum.ValVtunnelIndex.UPLL_IDX_VTN_NAME_VTNL
														.ordinal()));
			} else {
				valVtunnelStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtunnelIndex.UPLL_IDX_VTN_NAME_VTNL
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (vTunnel.has(VtnServiceJsonConsts.VTEPGROUPNAME)) {
				valVtunnelStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtunnelIndex.UPLL_IDX_VTEP_GRP_NAME_VTNL
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVtunnelStruct
						.set(VtnServiceIpcConsts.VTEPGRPNAME,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vTunnel.get(
														VtnServiceJsonConsts.VTEPGROUPNAME)
														.getAsString(),
												valVtunnelStruct,
												UncStructIndexEnum.ValVtunnelIndex.UPLL_IDX_VTEP_GRP_NAME_VTNL
														.ordinal()));
			} else {
				valVtunnelStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtunnelIndex.UPLL_IDX_VTEP_GRP_NAME_VTNL
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (vTunnel.has(VtnServiceJsonConsts.LABEL)) {
				valVtunnelStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtunnelIndex.UPLL_IDX_LABEL_VTNL
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));

				valVtunnelStruct
						.set(VtnServiceIpcConsts.LABEL,
								IpcDataUnitWrapper
										.setIpcUint32Value(
												vTunnel.get(
														VtnServiceJsonConsts.LABEL)
														.getAsString(),
												valVtunnelStruct,
												UncStructIndexEnum.ValVtunnelIndex.UPLL_IDX_LABEL_VTNL
														.ordinal()));
			} else {
				valVtunnelStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtunnelIndex.UPLL_IDX_LABEL_VTNL
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
		} else {
			LOG.warning("request body and uri parameters are not correct for getValVtunnelStruct");
		}
		LOG.info("Value Structure: " + valVtunnelStruct.toString());
		LOG.trace("Complete getValVtunnelStruct");
		return valVtunnelStruct;
	}

	/**
	 * Gets the key vtn flow filter controller struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key vtn flow filter entry struct
	 */
	public final IpcStruct getKeyVtnFlowfilterControllerStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		// Lower level structure
		/*
		 * ipc_struct KeyVtnFlowfilterController { KeyVtn vtn_key; UINT8
		 * controller_name[NN+1]; };
		 */
		LOG.trace("Start getKeyVtnFlowfilterControllerStruct");
		final IpcStruct keyVtnFlowfilterControllerStruct = new IpcStruct(
				UncStructEnum.KeyVtnFlowfilterController.getValue());
		IpcStruct keyVtnStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.ONE.ordinal()) {
			keyVtnStruct = getKeyVtnStruct(requestBody,
					uriParameters.subList(0, 1));
		}
		keyVtnFlowfilterControllerStruct.set(VtnServiceIpcConsts.VTNKEY,
				keyVtnStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.DOMAINID)) {
			keyVtnFlowfilterControllerStruct.set(VtnServiceJsonConsts.DOMAINID,
					IpcDataUnitWrapper.setIpcUint8ArrayValue(requestBody
							.getAsJsonPrimitive(VtnServiceJsonConsts.DOMAINID)
							.getAsString()));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVtnFlowfilterControllerStruct");
		}
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.CONTROLLERID)) {
			keyVtnFlowfilterControllerStruct.set(
					VtnServiceIpcConsts.CONTROLLERNAME, IpcDataUnitWrapper
							.setIpcUint8ArrayValue(requestBody
									.getAsJsonPrimitive(
											VtnServiceJsonConsts.CONTROLLERID)
									.getAsString()));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVtnFlowfilterControllerStruct");
		}
		LOG.info("Key Structure: "
				+ keyVtnFlowfilterControllerStruct.toString());
		LOG.trace("Complete getKeyVtnFlowfilterControllerStruct");
		return keyVtnFlowfilterControllerStruct;
	}

	/**
	 * Gets the val vtn flow filter entry struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the val vtn flow filter entry struct
	 */
	public final IpcStruct getValFlowFilterControllerStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		// Lower level structure
		/*
		 * ipc_struct ValFlowFilterController{ UINT8 valid[2]; UINT8 direction;
		 * UINT16 sequence_num; enum ValFlowFilterControllerpIndex {
		 * kIdxDirection = 0, kIdxSeqNum }; };
		 */
		LOG.trace("Start getValFlowFilterControllerStruct");
		final IpcStruct valFlowFilterContollerStruct = new IpcStruct(
				UncStructEnum.ValFlowFilterController.getValue());
		if (uriParameters != null) {
			if (uriParameters.size() >= UncIndexEnum.THREE.ordinal()) {
				valFlowFilterContollerStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowfilterControllerIndex.UPLL_IDX_DIRECTION_FFC
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				if (uriParameters.get(1).equalsIgnoreCase(
						VtnServiceJsonConsts.IN)) {
					valFlowFilterContollerStruct
							.set(VtnServiceIpcConsts.DIRECTION,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_IN
													.ordinal()));
				} else if (uriParameters.get(1).equalsIgnoreCase(
						VtnServiceJsonConsts.OUT)) {
					valFlowFilterContollerStruct
							.set(VtnServiceIpcConsts.DIRECTION,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_OUT
													.ordinal()));
				}
				LOG.debug("ff_type" + uriParameters.get(1));
			} else {
				valFlowFilterContollerStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowfilterControllerIndex.UPLL_IDX_DIRECTION_FFC
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (uriParameters.size() == UncIndexEnum.THREE.ordinal()) {
				valFlowFilterContollerStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowfilterControllerIndex.UPLL_IDX_SEQ_NUM_FFC
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valFlowFilterContollerStruct
						.set(VtnServiceIpcConsts.SEQUENCENUM,
								IpcDataUnitWrapper.setIpcUint16Value(
										uriParameters.get(2),
										valFlowFilterContollerStruct,
										UncStructIndexEnum.ValFlowfilterControllerIndex.UPLL_IDX_SEQ_NUM_FFC
												.ordinal()));
			} else {
				valFlowFilterContollerStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowfilterControllerIndex.UPLL_IDX_SEQ_NUM_FFC
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
		} else {
			LOG.warning("request body and uri parameters are not correct for getValFlowFilterControllerStruct");
		}
		LOG.info("Value Structure: " + valFlowFilterContollerStruct.toString());
		LOG.trace("Complete getValFlowFilterControllerStruct");

		return valFlowFilterContollerStruct;
	}

	/**
	 * Gets the Key vtep group struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the val vtep group struct.
	 */
	public final IpcStruct getKeyVtepGrpStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		/*
		 * ipc_struct key_vtep_grp { key_vtn vtn_key; UINT8 vtepgrp_name[32]; };
		 */
		LOG.trace("Start getKeyVtepGrpStruct");
		final IpcStruct keyVtepGrpStruct = new IpcStruct(
				UncStructEnum.KeyVtepGrp.getValue());
		IpcStruct keyVtnStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.ONE.ordinal()) {
			keyVtnStruct = getKeyVtnStruct(requestBody,
					uriParameters.subList(0, 1));
		}
		keyVtepGrpStruct.set(VtnServiceIpcConsts.VTNKEY, keyVtnStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.VTEPGROUP)
				&& ((JsonObject) requestBody
						.get(VtnServiceJsonConsts.VTEPGROUP))
						.has(VtnServiceJsonConsts.VTEPGROUPNAME)) {

			keyVtepGrpStruct.set(VtnServiceIpcConsts.VTEPGRP_NAME,
					IpcDataUnitWrapper
							.setIpcUint8ArrayValue(((JsonObject) requestBody
									.get(VtnServiceJsonConsts.VTEPGROUP)).get(
									VtnServiceJsonConsts.VTEPGROUPNAME)
									.getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.TWO.ordinal()) {
			keyVtepGrpStruct.set(VtnServiceIpcConsts.VTEPGRP_NAME,
					uriParameters.get(1));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVtepGrpStruct");
		}
		LOG.info("Key Structure: " + keyVtepGrpStruct.toString());
		LOG.trace("Complete getKeyVtepGrpStruct");
		return keyVtepGrpStruct;
	}

	/**
	 * Gets the Key vtep group struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the val vtep group struct.
	 */
	public final IpcStruct getKeyVtepGrpMemberStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		/*
		 * ipc_struct key_vtep_grp_member { key_vtep_grp vtepgrp_key; UINT8
		 * vtepmember_name[32]; };
		 */
		LOG.trace("Start getKeyVtepGrpMemberStruct");
		final IpcStruct keyVtepGrpMemberStruct = new IpcStruct(
				UncStructEnum.KeyVtepGrpMember.getValue());
		keyVtepGrpMemberStruct.set(VtnServiceIpcConsts.VTEPGRP_KEY,
				getKeyVtepGrpStruct(requestBody, uriParameters));
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.VTEPNAME)
				&& null != requestBody.get(VtnServiceJsonConsts.VTEPNAME)) {
			keyVtepGrpMemberStruct.set(
					VtnServiceIpcConsts.VTEPMEMBER_NAME,
					IpcDataUnitWrapper.setIpcUint8ArrayValue(requestBody.get(
							VtnServiceJsonConsts.VTEPNAME).getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.THREE.ordinal()) {
			keyVtepGrpMemberStruct.set(VtnServiceIpcConsts.VTEPMEMBER_NAME,
					IpcDataUnitWrapper.setIpcUint8ArrayValue(uriParameters
							.get(2)));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVtepGrpStruct");
		}
		LOG.info("Key Structure: " + keyVtepGrpMemberStruct.toString());
		LOG.trace("Complete getKeyVtepGrpMemberStruct");
		return keyVtepGrpMemberStruct;
	}

	/**
	 * Gets the val vtep grp struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the val vtep grp struct
	 */
	public final IpcStruct getValVtepGrpStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		/*
		 * ipc_struct val_vtep_grp { UINT8 valid[2]; UINT8 cs_row_status; UINT8
		 * cs_attr[2]; UINT8 controller_id[32]; UINT8 description[128]; };
		 */
		LOG.trace("Start getValVtepGrpStruct");
		final IpcStruct valVtepGrpStruct = new IpcStruct(
				UncStructEnum.ValVtepGrp.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.VTEPGROUP)) {
			final JsonObject vTepGroup = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VTEPGROUP);
			if (vTepGroup.has(VtnServiceJsonConsts.CONTROLLERID)) {
				valVtepGrpStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.val_vtep_grp_index.UPLL_IDX_CONTROLLER_ID_VTEPG
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVtepGrpStruct
						.set(VtnServiceIpcConsts.CONTROLLERID,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vTepGroup
														.get(VtnServiceJsonConsts.CONTROLLERID)
														.getAsString(),
												valVtepGrpStruct,
												UncStructIndexEnum.val_vtep_grp_index.UPLL_IDX_CONTROLLER_ID_VTEPG
														.ordinal()));
			} else {
				valVtepGrpStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.val_vtep_grp_index.UPLL_IDX_CONTROLLER_ID_VTEPG
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (vTepGroup.has(VtnServiceJsonConsts.DESCRIPTION)) {
				valVtepGrpStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.val_vtep_grp_index.UPLL_IDX_DESCRIPTION_VTEPG
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVtepGrpStruct
						.set(VtnServiceIpcConsts.DESCRIPTION,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vTepGroup
														.get(VtnServiceJsonConsts.DESCRIPTION)
														.getAsString(),
												valVtepGrpStruct,
												UncStructIndexEnum.val_vtep_grp_index.UPLL_IDX_DESCRIPTION_VTEPG
														.ordinal()));
			} else {
				valVtepGrpStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.val_vtep_grp_index.UPLL_IDX_DESCRIPTION_VTEPG
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
		}
		LOG.info("Value Structure: " + valVtepGrpStruct.toString());
		LOG.trace("Complete getValVtepGrpStruct");
		return valVtepGrpStruct;
	}

	/**
	 * Gets the val static ip route struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the val vtep grp struct
	 */
	public final IpcStruct getValStaticIpRouteStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		/*
		 * ipc_struct val_static_ip_route { UINT8 valid[2]; UINT8 cs_row_status;
		 * UINT8 cs_attr[2]; IPV4 next_hop_addr; UINT16 group_metric; };
		 */
		LOG.trace("Start getValStaticIpRouteStruct");
		final IpcStruct valStaticIpRouteStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValStaticIpRoute.getValue());
		if (requestBody != null) {
			if (requestBody.has(VtnServiceJsonConsts.STATIC_IPROUTE)
					&& requestBody.getAsJsonObject(
							VtnServiceJsonConsts.STATIC_IPROUTE).has(
							VtnServiceJsonConsts.GROUPMETRIC)) {
				valStaticIpRouteStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValStaticIpRouteIndex.UPLL_IDX_GROUP_METRIC_SIR
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valStaticIpRouteStruct
						.set(VtnServiceIpcConsts.GROUP_METRIC,
								IpcDataUnitWrapper
										.setIpcUint16Value(
												requestBody
														.getAsJsonObject(
																VtnServiceJsonConsts.STATIC_IPROUTE)
														.get(VtnServiceJsonConsts.GROUPMETRIC)
														.getAsString(),
												valStaticIpRouteStruct,
												UncStructIndexEnum.ValStaticIpRouteIndex.UPLL_IDX_GROUP_METRIC_SIR
														.ordinal()));
			} else {
				valStaticIpRouteStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValStaticIpRouteIndex.UPLL_IDX_GROUP_METRIC_SIR
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (requestBody.has(VtnServiceJsonConsts.STATIC_IPROUTE)
					&& requestBody.getAsJsonObject(
							VtnServiceJsonConsts.STATIC_IPROUTE).has(
							VtnServiceJsonConsts.NMGNAME)) {
				valStaticIpRouteStruct
						.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValStaticIpRouteIndex.UPLL_IDX_NWM_NAME_SIR
									.ordinal(),
							IpcDataUnitWrapper.setIpcUint8Value(
									UncStructIndexEnum.Valid.UNC_VF_VALID.ordinal()));
				valStaticIpRouteStruct
						.set(VtnServiceIpcConsts.NWM_NAME,
								IpcDataUnitWrapper.setIpcUint8ArrayValue(requestBody
										.getAsJsonObject(VtnServiceJsonConsts.STATIC_IPROUTE)
												.get(VtnServiceJsonConsts.NMGNAME).getAsString(),
										valStaticIpRouteStruct,
										UncStructIndexEnum.ValStaticIpRouteIndex
													.UPLL_IDX_NWM_NAME_SIR.ordinal()));
			} else {
				valStaticIpRouteStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValStaticIpRouteIndex.UPLL_IDX_NWM_NAME_SIR
								.ordinal(),
						IpcDataUnitWrapper
								.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
										.ordinal()));
			}
		} else {
			LOG.warning("Request body and uri parameters are not correct for getValStaticIpRouteStruct");
		}
		LOG.info("Value Structure: " + valStaticIpRouteStruct.toString());
		LOG.trace("Complete getValStaticIpRouteStruct");
		return valStaticIpRouteStruct;
	}

	/**
	 * Gets the key_vtnstation_controller struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return key_vtnstation_controller struct
	 */
	public final IpcStruct getKeyVtnstationControllerStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		/*
		 * ipc_struct key_vtnstation_controller { UINT8 controller_name[32]; };
		 */
		LOG.trace("Start getKeyVtnstationControllerStruct");
		final IpcStruct keyVtnstationController = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyVtnstationController
						.getValue());
		/*
		 * Check if controller_id is available in request body then set the same
		 * to Key Structure
		 */
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.CONTROLLERID)) {
			// in case op is count then, Key Structure's controller_id will not
			// be set
			if (requestBody.has(VtnServiceJsonConsts.OP)) {
				keyVtnstationController.set(VtnServiceIpcConsts.CONTROLLERNAME,
						IpcDataUnitWrapper.setIpcUint8ArrayValue(requestBody
								.get(VtnServiceJsonConsts.CONTROLLERID)
								.getAsString()));
			}
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVtnstationControllerStruct");
		}
		LOG.info("Key Structure: " + keyVtnstationController.toString());
		LOG.trace("Complete getKeyVtnstationControllerStruct");
		return keyVtnstationController;
	}

	/**
	 * Gets the val_vtnstation_controller_st struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return val_vtnstation_controller_st struct
	 */
	public final IpcStruct getValVtnstationControllerStStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		LOG.trace("Start getValVtnstationControllerStStruct");
		final IpcStruct valVtnstationControllerSt = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValVtnstationControllerSt
						.getValue());
		/*
		 * If optional parameters are available in request body then add the
		 * same to Value structure and set the valid bit
		 */
		if (requestBody != null) {
			// for station_id parameter
			valVtnstationControllerSt
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_STATION_ID_VSCS
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));

			// for created_time parameter
			valVtnstationControllerSt
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_CREATED_TIME_VSCS
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));

			// for mac_addr parameter
			if (requestBody.has(VtnServiceJsonConsts.MACADDR)) {
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_MAC_ADDR_VSCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				IpcDataUnitWrapper
						.setMacAddress(
								valVtnstationControllerSt,
								VtnServiceIpcConsts.MAC_ADDR,
								requestBody.get(VtnServiceJsonConsts.MACADDR)
										.getAsString(),
								UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_MAC_ADDR_VSCS
										.ordinal());
			} else {
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_MAC_ADDR_VSCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			// for ipv4_count parameter
			if (requestBody.has(VtnServiceJsonConsts.IPADDR)) {
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_IPV4_COUNT_VSCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.IPV4_COUNT,
								IpcDataUnitWrapper
										.setIpcUint32Value(
												VtnServiceJsonConsts.ONE,
												valVtnstationControllerSt,
												UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_IPV4_COUNT_VSCS
														.ordinal()));
			} else {
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_IPV4_COUNT_VSCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			// for ipv6_count parameter
			if (requestBody.has(VtnServiceJsonConsts.IPV6ADDR)) {
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_IPV6_COUNT_VSCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.IPV6_COUNT,
								IpcDataUnitWrapper
										.setIpcUint32Value(
												VtnServiceJsonConsts.ONE,
												valVtnstationControllerSt,
												UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_IPV6_COUNT_VSCS
														.ordinal()));
			} else {
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_IPV6_COUNT_VSCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			// for switch_id parameter
			if (requestBody.has(VtnServiceJsonConsts.SWITCHID)) {
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_DATAPATH_ID_VSCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.SWITCHID,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												requestBody
														.get(VtnServiceJsonConsts.SWITCHID)
														.getAsString(),
												valVtnstationControllerSt,
												UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_DATAPATH_ID_VSCS
														.ordinal()));
			} else {
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_DATAPATH_ID_VSCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			// for port_name parameter
			if (requestBody.has(VtnServiceJsonConsts.PORTNAME)) {
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_PORT_NAME_VSCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.PORTNAME,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												requestBody
														.get(VtnServiceJsonConsts.PORTNAME)
														.getAsString(),
												valVtnstationControllerSt,
												UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_PORT_NAME_VSCS
														.ordinal()));
			} else {
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_PORT_NAME_VSCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			// for vlan_id parameter
			if (requestBody.has(VtnServiceJsonConsts.VLANID)
					|| requestBody.has(VtnServiceJsonConsts.NO_VLAN_ID)) {
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VLAN_ID_VSCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				if (requestBody.has(VtnServiceJsonConsts.VLANID)) {
					valVtnstationControllerSt
							.set(VtnServiceIpcConsts.VLANID,
									IpcDataUnitWrapper
											.setIpcUint16Value(
													requestBody
															.get(VtnServiceJsonConsts.VLANID)
															.getAsString(),
													valVtnstationControllerSt,
													UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VLAN_ID_VSCS
															.ordinal()));
				} else {
					valVtnstationControllerSt
							.set(VtnServiceIpcConsts.VLANID,
									IpcDataUnitWrapper
											.setIpcUint16HexaValue(
													VtnServiceIpcConsts.VLAN_ID_DEFAULT_VALUE,
													valVtnstationControllerSt,
													UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VLAN_ID_VSCS
															.ordinal()));
				}
			} else {
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VLAN_ID_VSCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			// for map_type parameter
			valVtnstationControllerSt
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_MAP_TYPE_VSCS
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			// for map_status parameter
			valVtnstationControllerSt
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_MAP_STATUS_VSCS
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			// for vtn_name parameter
			if (requestBody.has(VtnServiceJsonConsts.VTNNAME)) {
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VTN_NAME_VSCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VTNNAME,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												requestBody
														.get(VtnServiceJsonConsts.VTNNAME)
														.getAsString(),
												valVtnstationControllerSt,
												UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VTN_NAME_VSCS
														.ordinal()));
			} else {
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VTN_NAME_VSCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			// for domain_id parameter
			if (requestBody.has(VtnServiceJsonConsts.DOMAINID)) {
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_DOMAIN_ID_VSCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.DOMAINID,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												requestBody
														.get(VtnServiceJsonConsts.DOMAINID)
														.getAsString(),
												valVtnstationControllerSt,
												UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_DOMAIN_ID_VSCS
														.ordinal()));
			} else {
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_DOMAIN_ID_VSCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			// for vnode_type parameter
			valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VNODE_TYPE_VSCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			// for vnode_name parameter
			if (requestBody.has(VtnServiceJsonConsts.VNODENAME)) {
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VNODE_NAME_VSCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VNODENAME,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												requestBody
														.get(VtnServiceJsonConsts.VNODENAME)
														.getAsString(),
												valVtnstationControllerSt,
												UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VNODE_NAME_VSCS
														.ordinal()));
			} else {
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VNODE_NAME_VSCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			// for if_name parameter
			if (requestBody.has(VtnServiceJsonConsts.IFNAME)) {
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VNODE_IF_NAME_VSCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VNODEIF_NAME,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												requestBody
														.get(VtnServiceJsonConsts.IFNAME)
														.getAsString(),
												valVtnstationControllerSt,
												UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VNODE_IF_NAME_VSCS
														.ordinal()));
			} else {
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VNODE_IF_NAME_VSCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
		} else {
			LOG.warning("request body and uri parameters are not correct for getValStaticIpRouteStruct");
		}
		LOG.info("Value Structure: " + valVtnstationControllerSt.toString());
		LOG.trace("Complete getValVtnstationControllerStStruct");
		return valVtnstationControllerSt;
	}

	/**
	 * Physical Ipc structures
	 */
	public final IpcStruct getKeyCtrDomainStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		// Key structure
		/*
		 * ipc_struct key_ctr_domain{ key_ctr ctr_key; UINT8 domain_name[32]; };
		 */
		LOG.trace("Start getKeyCtrDomainStruct");
		final IpcStruct keyCtrDomain = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyCtrDomain.getValue());
		IpcStruct keyCtrStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.ONE.ordinal()) {
			// call Controller Key
			keyCtrStruct = getKeyCtrStruct(requestBody,
					uriParameters.subList(0, 1));
		}
		keyCtrDomain.set(VtnServiceIpcConsts.CTR_KEY, keyCtrStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.DOMAIN)
				&& ((JsonObject) requestBody.get(VtnServiceJsonConsts.DOMAIN))
						.has(VtnServiceJsonConsts.DOMAINID)) {
			keyCtrDomain.set(VtnServiceIpcConsts.DOMAIN_NAME,
					IpcDataUnitWrapper
							.setIpcUint8ArrayValue(((JsonObject) requestBody
									.get(VtnServiceJsonConsts.DOMAIN)).get(
									VtnServiceJsonConsts.DOMAINID)
									.getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.TWO.ordinal()) {
			keyCtrDomain.set(VtnServiceIpcConsts.DOMAIN_NAME,
					IpcDataUnitWrapper.setIpcUint8ArrayValue(uriParameters
							.get(1)));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyCtrDomain");
		}
		LOG.info("Key Structure: " + keyCtrDomain.toString());
		LOG.trace("Complete getKeyCtrDomainStruct");
		return keyCtrDomain;
	}

	/**
	 * 
	 * @param requestBody
	 * @param uriParameters
	 * @return
	 */
	public final IpcStruct getKeySwitchStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		// Key structure
		/*
		 * ipc_struct key_switch{ key_ctr ctr_key; UINT8 switch_id[256]; };
		 */
		LOG.trace("Start getKeySwitchStruct");
		final IpcStruct keySwitch = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeySwitch.getValue());
		IpcStruct keyCtrStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.ONE.ordinal()) {
			// call Controller Key
			keyCtrStruct = getKeyCtrStruct(requestBody,
					uriParameters.subList(0, 1));
		}
		keySwitch.set(VtnServiceIpcConsts.CTR_KEY, keyCtrStruct);
		if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.TWO.ordinal()) {
			keySwitch.set(VtnServiceJsonConsts.SWITCHID, IpcDataUnitWrapper
					.setIpcUint8ArrayValue(uriParameters.get(1)));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeySwitch");
		}
		LOG.info("Key Structure: " + keySwitch.toString());
		LOG.trace("Complete getKeySwitchStruct");
		return keySwitch;
	}

	/**
	 * 
	 * @param requestBody
	 * @param uriParameters
	 * @return
	 */
	public final IpcStruct getKeyLinkStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		// Key structure
		/*
		 * ipc_struct key_link { key_ctr ctr_key; UINT8 switch_id1[256]; UINT8
		 * port_id1[32]; UINT8 switch_id2[256]; UINT8 port_id2[32]; };
		 */
		LOG.trace("Start getKeyLinkStruct");
		final IpcStruct keyLink = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyLink.getValue());
		IpcStruct keyLinkStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.ONE.ordinal()) {
			// call Controller Key
			keyLinkStruct = getKeyCtrStruct(requestBody,
					uriParameters.subList(0, 1));
			keyLink.set(VtnServiceIpcConsts.CTR_KEY, keyLinkStruct);
		}
		if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.TWO.ordinal()) {
			final String linkName[] = uriParameters.get(1).split(
					VtnServiceJsonConsts.LINKSAPERATOR);
			if (linkName.length == UncIndexEnum.FOUR.ordinal()) {
				keyLink.set(VtnServiceIpcConsts.SWITCH_ID1,
						IpcDataUnitWrapper.setIpcUint8ArrayValue(linkName[0]));
				keyLink.set(VtnServiceIpcConsts.PORT_ID1,
						IpcDataUnitWrapper.setIpcUint8ArrayValue(linkName[1]));
				keyLink.set(VtnServiceIpcConsts.SWITCH_ID2,
						IpcDataUnitWrapper.setIpcUint8ArrayValue(linkName[2]));
				keyLink.set(VtnServiceIpcConsts.PORT_ID2,
						IpcDataUnitWrapper.setIpcUint8ArrayValue(linkName[3]));
			} else {
				LOG.error("Value of linkname is incorrect");
			}
		} else if (requestBody != null
				&& (requestBody.has(VtnServiceJsonConsts.SWITCH1ID) || requestBody
						.has(VtnServiceJsonConsts.SWITCH2ID))) {
			if (requestBody.has(VtnServiceJsonConsts.SWITCH1ID)) {
				keyLink.set(VtnServiceIpcConsts.SWITCH_ID1, IpcDataUnitWrapper
						.setIpcUint8ArrayValue(requestBody.get(
								VtnServiceJsonConsts.SWITCH1ID).getAsString()));
			}

			if (requestBody.has(VtnServiceJsonConsts.SWITCH2ID)) {
				keyLink.set(VtnServiceIpcConsts.SWITCH_ID2, IpcDataUnitWrapper
						.setIpcUint8ArrayValue(requestBody.get(
								VtnServiceJsonConsts.SWITCH2ID).getAsString()));
			}
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyLink");
		}
		LOG.info("Key Structure: " + keyLinkStruct.toString());
		LOG.trace("Complete getKeyLinkStruct");
		return keyLink;
	}

	/**
	 * Gets the key Ctr struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key Ctr struct
	 */
	public final IpcStruct getKeyCtrStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		/*
		 * ipc_struct key_ctr{ UINT8 controller_name[32]; };
		 */
		LOG.trace("Start getKeyCtrStruct");
		final IpcStruct keyCtrStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyCtr.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.CONTROLLER)
				&& ((JsonObject) requestBody
						.get(VtnServiceJsonConsts.CONTROLLER))
						.has(VtnServiceJsonConsts.CONTROLLERID)) {
			keyCtrStruct.set(VtnServiceIpcConsts.CONTROLLERNAME,
					IpcDataUnitWrapper
							.setIpcUint8ArrayValue(((JsonObject) requestBody
									.get(VtnServiceJsonConsts.CONTROLLER)).get(
									VtnServiceJsonConsts.CONTROLLERID)
									.getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.ONE.ordinal()) {
			keyCtrStruct.set(VtnServiceJsonConsts.CONTROLLERNAME,
					IpcDataUnitWrapper.setIpcUint8ArrayValue(uriParameters
							.get(0)));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyCtrStruct");
		}
		LOG.info("Key Structure: " + keyCtrStruct.toString());
		LOG.trace("Complete getKeyCtrStruct");
		return keyCtrStruct;
	}

	/**
	 * Gets the Val Ctr struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the Val Ctr struct
	 */
	public final IpcStruct getValCtrStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		LOG.trace("Start getValCtrStruct");
		final IpcStruct valCtrStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValCtr.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.CONTROLLER)) {
			final JsonObject controller = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.CONTROLLER);
			if (controller.has(VtnServiceJsonConsts.TYPE)) {
				valCtrStruct
						.set(VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxType
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				if (controller.get(VtnServiceJsonConsts.TYPE).getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.BYPASS)) {
					valCtrStruct
							.set(VtnServiceJsonConsts.TYPE,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncPhysicalStructIndexEnum.UpplTypeIndex.UNC_CT_UNKNOWN
													.ordinal()));
				} else if (controller.get(VtnServiceJsonConsts.TYPE)
						.getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.PFC)) {
					valCtrStruct
							.set(VtnServiceJsonConsts.TYPE,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncPhysicalStructIndexEnum.UpplTypeIndex.UNC_CT_PFC
													.ordinal()));
				} else if (controller.get(VtnServiceJsonConsts.TYPE)
						.getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.VNP)) {
					valCtrStruct
							.set(VtnServiceJsonConsts.TYPE,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncPhysicalStructIndexEnum.UpplTypeIndex.UNC_CT_VNP
													.ordinal()));
				} else if (controller
						.get(VtnServiceJsonConsts.TYPE)
						.getAsString()
						.equalsIgnoreCase(
								VtnServiceInitManager
										.getConfigurationMap()
										.getCommonConfigValue(
												VtnServiceConsts.CONF_FILE_FIELD_POLC))) {
					valCtrStruct
							.set(VtnServiceJsonConsts.TYPE,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncPhysicalStructIndexEnum.UpplTypeIndex.UNC_CT_POLC
													.ordinal()));
				} else if (controller
						.get(VtnServiceJsonConsts.TYPE)
						.getAsString()
						.equalsIgnoreCase(
								VtnServiceInitManager
										.getConfigurationMap()
										.getCommonConfigValue(
												VtnServiceConsts.CONF_FILE_FIELD_HPVANC))) {
					valCtrStruct
					.set(VtnServiceJsonConsts.TYPE,
							IpcDataUnitWrapper
									.setIpcUint8Value(UncPhysicalStructIndexEnum.UpplTypeIndex.UNC_CT_HPVANC
											.ordinal()));
				}  else if (controller
						.get(VtnServiceJsonConsts.TYPE)
						.getAsString()
						.equalsIgnoreCase(
								VtnServiceInitManager
										.getConfigurationMap()
										.getCommonConfigValue(
												VtnServiceConsts.CONF_FILE_FIELD_ODC))) {
					valCtrStruct
					.set(VtnServiceJsonConsts.TYPE,
							IpcDataUnitWrapper
									.setIpcUint8Value(UncPhysicalStructIndexEnum.UpplTypeIndex.UNC_CT_ODC
											.ordinal()));
				}
				LOG.debug("type:"
						+ controller.get(VtnServiceJsonConsts.TYPE)
								.getAsString());
			} else {
				valCtrStruct
						.set(VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxType
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (controller.has(VtnServiceJsonConsts.VERSION)) {
				valCtrStruct
						.set(VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxVersion
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valCtrStruct
						.set(VtnServiceJsonConsts.VERSION,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												controller
														.get(VtnServiceJsonConsts.VERSION)
														.getAsString(),
												valCtrStruct,
												UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxVersion
														.ordinal()));
			} else {
				valCtrStruct
						.set(VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxVersion
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (controller.has(VtnServiceJsonConsts.DESCRIPTION)) {
				valCtrStruct
						.set(VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxDescription
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valCtrStruct
						.set(VtnServiceJsonConsts.DESCRIPTION,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												controller
														.get(VtnServiceJsonConsts.DESCRIPTION)
														.getAsString(),
												valCtrStruct,
												UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxDescription
														.ordinal()));
			} else {
				valCtrStruct
						.set(VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxDescription
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (controller.has(VtnServiceJsonConsts.IPADDR)) {
				valCtrStruct
						.set(VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxIpAddress
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valCtrStruct
						.set(VtnServiceIpcConsts.IP_ADDRESS,
								IpcDataUnitWrapper
										.setIpcInet4AddressValue(
												controller
														.get(VtnServiceJsonConsts.IPADDR)
														.getAsString(),
												valCtrStruct,
												UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxIpAddress
														.ordinal()));
			} else {
				valCtrStruct
						.set(VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxIpAddress
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (controller.has(VtnServiceJsonConsts.USERNAME)) {
				valCtrStruct
						.set(VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxUser
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valCtrStruct
						.set(VtnServiceIpcConsts.USER,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												controller
														.get(VtnServiceJsonConsts.USERNAME)
														.getAsString(),
												valCtrStruct,
												UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxUser
														.ordinal()));
			} else {
				valCtrStruct
						.set(VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxUser
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (controller.has(VtnServiceJsonConsts.PASSWORD)) {
				valCtrStruct
						.set(VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxPassword
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valCtrStruct
						.set(VtnServiceJsonConsts.PASSWORD,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												controller
														.get(VtnServiceJsonConsts.PASSWORD)
														.getAsString(),
												valCtrStruct,
												UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxPassword
														.ordinal()));
			} else {
				valCtrStruct
						.set(VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxPassword
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (controller.has(VtnServiceJsonConsts.AUDITSTATUS)) {
				valCtrStruct
						.set(VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxEnableAudit
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				if (controller.get(VtnServiceJsonConsts.AUDITSTATUS)
						.getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.DISABLE)) {
					valCtrStruct
							.set(VtnServiceIpcConsts.ENABLE_AUDIT,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncPhysicalStructIndexEnum.UpplControllerAuditStatus.UPPL_AUTO_AUDIT_DISABLED
													.ordinal()));
				} else if (controller.get(VtnServiceJsonConsts.AUDITSTATUS)
						.getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.ENABLE)) {
					valCtrStruct
							.set(VtnServiceIpcConsts.ENABLE_AUDIT,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncPhysicalStructIndexEnum.UpplControllerAuditStatus.UPPL_AUTO_AUDIT_ENABLED
													.ordinal()));
				}
				LOG.debug("auditstatus"
						+ controller.get(VtnServiceJsonConsts.AUDITSTATUS)
								.getAsString());
			} else {
				valCtrStruct
						.set(VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxEnableAudit
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (controller.has(VtnServiceJsonConsts.PORT)) {
				valCtrStruct
						.set(VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxPort
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valCtrStruct
						.set(VtnServiceIpcConsts.PORT,
								IpcDataUnitWrapper
										.setIpcUint16Value(
												controller
														.get(VtnServiceJsonConsts.PORT)
														.getAsString(),
												valCtrStruct,
												UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxPort
														.ordinal()));
			} else {
				valCtrStruct
						.set(VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxPort
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}

		} else {
			LOG.warning("request body and uri parameters are not correct for getValCtrStruct");
		}
		LOG.info("Value Structure: " + valCtrStruct.toString());
		LOG.trace("Complete getValCtrStruct");
		return valCtrStruct;
	}

	public final IpcStruct getValCtrDomainStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		/*
		 * ipc_struct val_ctr_domain { UINT8 type; UINT8 description[128]; UINT8
		 * valid[2]; UINT8 cs_row_status; UINT8 cs_attr[2]; };
		 */
		LOG.trace("Start getValCtrDomainStruct");
		final IpcStruct valCtrDomainStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValCtrDomain.getValue());
		if (requestBody != null && requestBody.has(VtnServiceJsonConsts.DOMAIN)) {
			final JsonObject domain = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.DOMAIN);
			if (domain.has(VtnServiceJsonConsts.TYPE)) {
				valCtrDomainStruct
						.set(VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UpplValDomainIndex.kIdxDomainType
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				if (domain.get(VtnServiceJsonConsts.TYPE).getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.DEFAULT)) {
					valCtrDomainStruct
							.set(VtnServiceJsonConsts.TYPE,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncPhysicalStructIndexEnum.UpplDomainType.UPPL_DOMAIN_TYPE_DEFAULT
													.ordinal()));
				} else if (domain.get(VtnServiceJsonConsts.TYPE).getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.NORMAL)) {
					valCtrDomainStruct
							.set(VtnServiceJsonConsts.TYPE,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncPhysicalStructIndexEnum.UpplDomainType.UPPL_DOMAIN_TYPE_NORMAL
													.ordinal()));
				}
				LOG.debug("type:"
						+ domain.get(VtnServiceJsonConsts.TYPE).getAsString());
			} else {
				valCtrDomainStruct
						.set(VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UpplValDomainIndex.kIdxDomainType
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (domain.has(VtnServiceJsonConsts.DESCRIPTION)) {
				valCtrDomainStruct
						.set(VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UpplValDomainIndex.kIdxDomainDescription
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valCtrDomainStruct
						.set(VtnServiceJsonConsts.DESCRIPTION,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												domain.get(
														VtnServiceJsonConsts.DESCRIPTION)
														.getAsString(),
												valCtrDomainStruct,
												UncPhysicalStructIndexEnum.UpplValDomainIndex.kIdxDomainDescription
														.ordinal()));
			} else {
				valCtrDomainStruct
						.set(VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UpplValDomainIndex.kIdxDomainDescription
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
		} else {
			LOG.warning("request body and uri parameters are not correct for getValCtrDomainStruct");
		}
		LOG.info("Value Structure: " + valCtrDomainStruct.toString());
		LOG.trace("Complete getValCtrDomainStruct");
		return valCtrDomainStruct;
	}

	public final IpcStruct getKeyBoundaryStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		/*
		 * ipc_struct key_boundary { UINT8 boundary_id[32]; };
		 */
		LOG.trace("Start getKeyBoundaryStruct");
		final IpcStruct keyBoundaryStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyBoundary.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.BOUNDARY)
				&& ((JsonObject) requestBody.get(VtnServiceJsonConsts.BOUNDARY))
						.has(VtnServiceJsonConsts.BOUNDARYID)) {
			keyBoundaryStruct.set(VtnServiceJsonConsts.BOUNDARYID,
					IpcDataUnitWrapper
							.setIpcUint8ArrayValue(((JsonObject) requestBody
									.get(VtnServiceJsonConsts.BOUNDARY)).get(
									VtnServiceJsonConsts.BOUNDARYID)
									.getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.ONE.ordinal()) {
			keyBoundaryStruct.set(VtnServiceJsonConsts.BOUNDARYID,
					IpcDataUnitWrapper.setIpcUint8ArrayValue(uriParameters
							.get(0)));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyBoundaryStruct");
		}
		LOG.info("Key Structure: " + keyBoundaryStruct.toString());
		LOG.trace("Complete getKeyBoundaryStruct");

		return keyBoundaryStruct;
	}

	public final IpcStruct getValBoundaryStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		/*
		 * ipc_struct val_boundary { UINT8 description[128]; UINT8
		 * controller_name1[32]; UINT8 domain_name1[32]; UINT8
		 * logical_port_id1[320]; UINT8 controller_name2[32]; UINT8
		 * domain_name2[32]; UINT8 logical_port_id2[320]; UINT8 valid[7]; UINT8
		 * cs_row_status; UINT8 cs_attr[7]; };
		 */
		LOG.trace("Start getValBoundaryStruct");
		final IpcStruct valBoundaryStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValBoundary.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.BOUNDARY)) {
			final JsonObject boundary = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.BOUNDARY);
			if (boundary.has(VtnServiceJsonConsts.DESCRIPTION)) {
				valBoundaryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryDescription
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valBoundaryStruct
						.set(VtnServiceJsonConsts.DESCRIPTION,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												boundary.get(
														VtnServiceJsonConsts.DESCRIPTION)
														.getAsString(),
												valBoundaryStruct,
												UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryDescription
														.ordinal()));
			} else {
				valBoundaryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryDescription
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (boundary.has(VtnServiceJsonConsts.LINK)) {
				final JsonObject link = boundary
						.getAsJsonObject(VtnServiceJsonConsts.LINK);
				if (link.has(VtnServiceJsonConsts.CONTROLLER1ID)) {
					valBoundaryStruct
							.set(VtnServiceIpcConsts.VALID,
									UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryControllerName1
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
													.ordinal()));
					valBoundaryStruct
							.set(VtnServiceIpcConsts.CONTROLLER_NAME1,
									IpcDataUnitWrapper
											.setIpcUint8ArrayValue(
													link.get(
															VtnServiceJsonConsts.CONTROLLER1ID)
															.getAsString(),
													valBoundaryStruct,
													UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryControllerName1
															.ordinal()));
				} else {
					valBoundaryStruct
							.set(VtnServiceIpcConsts.VALID,
									UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryControllerName1
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
													.ordinal()));
				}
				if (link.has(VtnServiceJsonConsts.DOMAIN1_ID)) {
					valBoundaryStruct
							.set(VtnServiceIpcConsts.VALID,
									UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryDomainName1
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
													.ordinal()));
					valBoundaryStruct
							.set(VtnServiceIpcConsts.DOMAIN_NAME1,
									IpcDataUnitWrapper
											.setIpcUint8ArrayValue(
													link.get(
															VtnServiceJsonConsts.DOMAIN1_ID)
															.getAsString(),
													valBoundaryStruct,
													UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryDomainName1
															.ordinal()));
				} else {
					valBoundaryStruct
							.set(VtnServiceIpcConsts.VALID,
									UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryDomainName1
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
													.ordinal()));
				}
				if (link.has(VtnServiceJsonConsts.LOGICAL_PORT1_ID)) {
					valBoundaryStruct
							.set(VtnServiceIpcConsts.VALID,
									UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryLogicalPortId1
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
													.ordinal()));
					valBoundaryStruct
							.set(VtnServiceIpcConsts.LOGICAL_PORT_ID1,
									IpcDataUnitWrapper
											.setIpcUint8ArrayValue(
													link.get(
															VtnServiceJsonConsts.LOGICAL_PORT1_ID)
															.getAsString(),
													valBoundaryStruct,
													UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryLogicalPortId1
															.ordinal()));
				} else {
					valBoundaryStruct
							.set(VtnServiceIpcConsts.VALID,
									UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryLogicalPortId1
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
													.ordinal()));
				}
				if (link.has(VtnServiceJsonConsts.CONTROLLER2ID)) {
					valBoundaryStruct
							.set(VtnServiceIpcConsts.VALID,
									UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryControllerName2
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
													.ordinal()));
					valBoundaryStruct
							.set(VtnServiceIpcConsts.CONTROLLER_NAME2,
									IpcDataUnitWrapper
											.setIpcUint8ArrayValue(
													link.get(
															VtnServiceJsonConsts.CONTROLLER2ID)
															.getAsString(),
													valBoundaryStruct,
													UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryControllerName2
															.ordinal()));
				} else {
					valBoundaryStruct
							.set(VtnServiceIpcConsts.VALID,
									UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryControllerName2
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
													.ordinal()));
				}
				if (link.has(VtnServiceJsonConsts.DOMAIN2_ID)) {
					valBoundaryStruct
							.set(VtnServiceIpcConsts.VALID,
									UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryDomainName2
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
													.ordinal()));
					valBoundaryStruct
							.set(VtnServiceIpcConsts.DOMAIN_NAME2,
									IpcDataUnitWrapper
											.setIpcUint8ArrayValue(
													link.get(
															VtnServiceJsonConsts.DOMAIN2_ID)
															.getAsString(),
													valBoundaryStruct,
													UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryDomainName2
															.ordinal()));
				} else {
					valBoundaryStruct
							.set(VtnServiceIpcConsts.VALID,
									UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryDomainName2
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
													.ordinal()));
				}
				if (link.has(VtnServiceJsonConsts.LOGICAL_PORT2_ID)) {
					valBoundaryStruct
							.set(VtnServiceIpcConsts.VALID,
									UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryLogicalPortId2
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
													.ordinal()));
					valBoundaryStruct
							.set(VtnServiceIpcConsts.LOGICAL_PORT_ID2,
									IpcDataUnitWrapper
											.setIpcUint8ArrayValue(
													link.get(
															VtnServiceJsonConsts.LOGICAL_PORT2_ID)
															.getAsString(),
													valBoundaryStruct,
													UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryLogicalPortId2
															.ordinal()));
				} else {
					valBoundaryStruct
							.set(VtnServiceIpcConsts.VALID,
									UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryLogicalPortId2
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
													.ordinal()));
				}
			} else {
				LOG.warning("request body and uri parameters are not correct for link object");
			}
		} else if (requestBody != null) {
			valBoundaryStruct
					.set(VtnServiceIpcConsts.VALID,
							UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryDescription
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			if (requestBody.has(VtnServiceJsonConsts.CONTROLLER1ID)) {
				valBoundaryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryControllerName1
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valBoundaryStruct
						.set(VtnServiceIpcConsts.CONTROLLER_NAME1,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												requestBody
														.get(VtnServiceJsonConsts.CONTROLLER1ID)
														.getAsString(),
												valBoundaryStruct,
												UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryControllerName1
														.ordinal()));
			} else {
				valBoundaryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryControllerName1
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			valBoundaryStruct
					.set(VtnServiceIpcConsts.VALID,
							UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryDomainName1
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			valBoundaryStruct
					.set(VtnServiceIpcConsts.VALID,
							UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryLogicalPortId1
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			if (requestBody.has(VtnServiceJsonConsts.CONTROLLER2ID)) {
				valBoundaryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryControllerName2
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valBoundaryStruct
						.set(VtnServiceIpcConsts.CONTROLLER_NAME2,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												requestBody
														.get(VtnServiceJsonConsts.CONTROLLER2ID)
														.getAsString(),
												valBoundaryStruct,
												UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryControllerName2
														.ordinal()));
			} else {
				valBoundaryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryControllerName2
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			valBoundaryStruct
					.set(VtnServiceIpcConsts.VALID,
							UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryDomainName2
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			valBoundaryStruct
					.set(VtnServiceIpcConsts.VALID,
							UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryLogicalPortId2
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
		} else {
			LOG.warning("request body and uri parameters are not correct for getValCtrStruct");
		}
		LOG.info("Value Structure: " + valBoundaryStruct.toString());
		LOG.trace("Complete getValBoundaryStruct");
		return valBoundaryStruct;
	}

	/**
	 * This will get the key structure for port API
	 * 
	 * @param requestBody
	 * @param uriParameters
	 * @return
	 */
	public final IpcStruct getKeyPortStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		/*
		 * ipc_struct key_port { key_switch sw_key; UINT8 port_id[32]; };
		 */
		LOG.trace("Start getKeyPortStruct");
		final IpcStruct keyPortStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyPort.getValue());
		IpcStruct keySwitchStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.TWO.ordinal()) {
			// call Controller Key
			keySwitchStruct = getKeySwitchStruct(requestBody,
					uriParameters.subList(0, 2));
		}
		keyPortStruct.set(VtnServiceIpcConsts.SW_KEY, keySwitchStruct);

		if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.THREE.ordinal()) {
			keyPortStruct.set(VtnServiceJsonConsts.PORT_ID, IpcDataUnitWrapper
					.setIpcUint8ArrayValue(uriParameters.get(2)));
		} else {
			LOG.warning("response body and uri parameters are not correct for getKeySwitch");
		}
		LOG.info("Key Structure: " + keyPortStruct.toString());
		LOG.trace("Complete getKeyPortStruct");
		return keyPortStruct;
	}

	public final IpcStruct getValVrtArpEntryStStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		/*
		 * ipc_struct val_vrt_arp_entry_st { UINT8 valid[4]; UINT8 macaddr[6];
		 * IPV4 ip_addr; UINT8 type; UINT8 if_name[32]; };
		 */
		LOG.trace("Start getValVrtArpEntryStStruct");
		final IpcStruct ValArpStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValVrtArpEntrySt.getValue());
		if (requestBody != null) {
			if (requestBody.has(VtnServiceJsonConsts.TYPE)) {
				ValArpStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtArpEntryStIndex.UPLL_IDX_TYPE_VAES
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				if (requestBody.get(VtnServiceJsonConsts.TYPE).getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.STATIC)) {
					ValArpStruct
							.set(VtnServiceJsonConsts.TYPE,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.ValMacEntry.UPLL_MAC_ENTRY_STATIC
													.ordinal()));
				} else if (requestBody.get(VtnServiceJsonConsts.TYPE)
						.getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.DYNAMIC)) {
					ValArpStruct
							.set(VtnServiceJsonConsts.TYPE,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.ValMacEntry.UPLL_MAC_ENTRY_DYNAMIC
													.ordinal()));
				}
				LOG.debug("type:"
						+ requestBody.get(VtnServiceJsonConsts.TYPE)
								.getAsString());
			} else {
				ValArpStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtArpEntryStIndex.UPLL_IDX_TYPE_VAES
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}

		} else {
			LOG.warning("request body and uri parameters are not correct for getValVrtArpEntryStStruct");
		}
		LOG.info("Value Structure: " + ValArpStruct.toString());
		LOG.trace("Complete getValVrtArpEntryStStruct");
		return ValArpStruct;
	}

	public final IpcStruct getKeyLogicalPortStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		/*
		 * ipc_struct key_logical_port { key_ctr_domain domain_key; UINT8 port
		 * _id[320]; };
		 */
		LOG.trace("Start getKeyLogicalPortStruct");
		final IpcStruct keyLogicalPortStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyLogicalPort.getValue());
		IpcStruct keyDomainStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.TWO.ordinal()) {
			// call Domain Key
			keyDomainStruct = getKeyCtrDomainStruct(requestBody,
					uriParameters.subList(0, 2));
		}
		keyLogicalPortStruct.set(VtnServiceIpcConsts.DOMAIN_KEY,
				keyDomainStruct);
		if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.THREE.ordinal()) {
			keyLogicalPortStruct.set(VtnServiceJsonConsts.PORT_ID,
					IpcDataUnitWrapper.setIpcUint8ArrayValue(uriParameters
							.get(2)));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyLogicalPort");
		}
		LOG.info("Key Structure: " + keyLogicalPortStruct.toString());
		LOG.trace("Complete getKeyLogicalPortStruct");
		return keyLogicalPortStruct;
	}

	public final IpcStruct getKeyLogicalMemberPortStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		/*
		 * ipc_struct key_logical_member_port { key_logical_port
		 * logical_port_key; UINT8 switch_id[256]; UINT8 physical_port_id[32];
		 * };
		 */
		LOG.trace("Start getKeyLogicalMemberPortStruct");
		final IpcStruct keyLogicalMemberPortStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyLogicalMemberPort
						.getValue());
		IpcStruct keyLogicalPortStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.THREE.ordinal()) {
			// call LogicalPort Key
			keyLogicalPortStruct = getKeyLogicalPortStruct(requestBody,
					uriParameters.subList(0, 3));
		}
		keyLogicalMemberPortStruct.set(VtnServiceIpcConsts.LOGICAL_PORT_KEY,
				keyLogicalPortStruct);
		if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.FIVE.ordinal()) {

			if (!uriParameters.get(3).equalsIgnoreCase(
					VtnServiceJsonConsts.SWITCHID_NOT_FOUND)) {
				keyLogicalMemberPortStruct.set(VtnServiceIpcConsts.SWITCHID,
						IpcDataUnitWrapper.setIpcUint8ArrayValue(uriParameters
								.get(3)));
			}
			if (!uriParameters.get(4).equalsIgnoreCase(
					VtnServiceJsonConsts.PORTID_NOT_FOUND)) {
				keyLogicalMemberPortStruct.set(
						VtnServiceIpcConsts.PHYSICAL_PORT_ID,
						IpcDataUnitWrapper.setIpcUint8ArrayValue(uriParameters
								.get(4)));
			}
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyLogicalMemberPort");
		}
		LOG.info("Key Structure: " + keyLogicalMemberPortStruct.toString());
		LOG.trace("Complete getKeyLogicalMemberPortStruct");
		return keyLogicalMemberPortStruct;
	}

	// Added New key Structure and Value Structure for U12 Requirement
	/**
	 * Gets the Key Vtn Controller struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the Key Vtn Controller struct
	 */
	public final IpcStruct getKeyVtnControllerStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		LOG.trace("Start getKeyVtnControllerStruct");

		final IpcStruct KeyVtnControllerStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyVtnController.getValue());
		IpcStruct keyVtnStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.ONE.ordinal()) {
			keyVtnStruct = getKeyVtnStruct(requestBody,
					uriParameters.subList(0, 1));
		}
		KeyVtnControllerStruct.set(VtnServiceIpcConsts.VTNKEY, keyVtnStruct);
		if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.TWO.ordinal()) {

			final String mappingId[] = uriParameters.get(1).split(
					VtnServiceJsonConsts.HYPHEN);
			KeyVtnControllerStruct
					.set(VtnServiceJsonConsts.CONTROLLERNAME,
							IpcDataUnitWrapper
									.setIpcUint8ArrayValue(mappingId[VtnServiceJsonConsts.VAL_0]));
			KeyVtnControllerStruct
					.set(VtnServiceJsonConsts.DOMAINID,
							IpcDataUnitWrapper
									.setIpcUint8ArrayValue(mappingId[VtnServiceJsonConsts.VAL_1]));

		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVtnControllerStruct");
		}
		LOG.info("Key Structure: " + KeyVtnControllerStruct.toString());
		LOG.trace("Complete getKeyVtnControllerStruct");
		return KeyVtnControllerStruct;
	}
	
	
	// Added New key Structure for U14 Requirement
	/**
	 * Gets the Value VTN Mapping Controller State Struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the Value VTN Mapping Controller State Struct.
	 */
	public final IpcStruct getValVtnMappingControllerStStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		LOG.trace("Start getValVtnMappingControllerStStruct");
		final IpcStruct valVtnMappingControllerStStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValVtnMappingControllerSt.getValue());

		if (requestBody != null) {
			if (requestBody.has(VtnServiceJsonConsts.VNODETYPE)) {
				String vnodeTypeString = requestBody.get(
						VtnServiceJsonConsts.VNODETYPE).getAsString();
				int vnodeTypeVal = -1;
				if (vnodeTypeString
						.equalsIgnoreCase(VtnServiceJsonConsts.VBRIDGE)) {
					vnodeTypeVal = UncStructIndexEnum.ValVnodeType.UPLL_VNODE_VBRIDGE
							.ordinal();
				} else if (vnodeTypeString
						.equalsIgnoreCase(VtnServiceJsonConsts.VROUTER)) {
					vnodeTypeVal = UncStructIndexEnum.ValVnodeType.UPLL_VNODE_VROUTER
							.ordinal();
				} else if (vnodeTypeString
						.equalsIgnoreCase(VtnServiceJsonConsts.VTEP)) {
					vnodeTypeVal = UncStructIndexEnum.ValVnodeType.UPLL_VNODE_VTEP
							.ordinal();
				} else if (vnodeTypeString
						.equalsIgnoreCase(VtnServiceJsonConsts.VTERMINAL)) {
					vnodeTypeVal = UncStructIndexEnum.ValVnodeType.UPLL_VNODE_VTERMINAL
							.ordinal();
				} else if (vnodeTypeString
						.equalsIgnoreCase(VtnServiceJsonConsts.VTUNNEL)) {
					vnodeTypeVal = UncStructIndexEnum.ValVnodeType.UPLL_VNODE_VTUNNEL
							.ordinal();
				} else if (vnodeTypeString
						.equalsIgnoreCase(VtnServiceJsonConsts.VBYPASS)) {
					vnodeTypeVal = UncStructIndexEnum.ValVnodeType.UPLL_VNODE_VUNKNOWN
							.ordinal();
				}
				
				if (-1 != vnodeTypeVal) {
					valVtnMappingControllerStStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.valVtnMappingControllerStIndex.UPLL_IDX_VNODE_TYPE_VMCS
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
													.ordinal()));
					
					valVtnMappingControllerStStruct
							.set(VtnServiceJsonConsts.VNODETYPE,
									IpcDataUnitWrapper.setIpcUint8Value(
											String.valueOf(vnodeTypeVal),
											valVtnMappingControllerStStruct,
											UncStructIndexEnum.valVtnMappingControllerStIndex.UPLL_IDX_VNODE_TYPE_VMCS
													.ordinal()));
				} else {
					valVtnMappingControllerStStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.valVtnMappingControllerStIndex.UPLL_IDX_VNODE_TYPE_VMCS
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
													.ordinal()));
				}
			} else {
				valVtnMappingControllerStStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.valVtnMappingControllerStIndex.UPLL_IDX_VNODE_TYPE_VMCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			
			if (requestBody.has(VtnServiceJsonConsts.VNODENAME)) {
				valVtnMappingControllerStStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.valVtnMappingControllerStIndex.UPLL_IDX_VNODE_NAME_VMCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVtnMappingControllerStStruct.set(VtnServiceJsonConsts.VNODENAME,
						IpcDataUnitWrapper.setIpcUint8ArrayValue(
								requestBody
										.get(VtnServiceJsonConsts.VNODENAME)
										.getAsString(), valVtnMappingControllerStStruct,
								UncStructIndexEnum.valVtnMappingControllerStIndex.UPLL_IDX_VNODE_NAME_VMCS
										.ordinal()));
			} else {
				valVtnMappingControllerStStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.valVtnMappingControllerStIndex.UPLL_IDX_VNODE_NAME_VMCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			
			if (requestBody.has(VtnServiceJsonConsts.VIFNAME)) {
				valVtnMappingControllerStStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.valVtnMappingControllerStIndex.UPLL_IDX_VNODE_IF_NAME_VMCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVtnMappingControllerStStruct.set(VtnServiceJsonConsts.VNODEIFNAME,
						IpcDataUnitWrapper.setIpcUint8ArrayValue(
								requestBody
										.get(VtnServiceJsonConsts.VIFNAME)
										.getAsString(), valVtnMappingControllerStStruct,
								UncStructIndexEnum.valVtnMappingControllerStIndex.UPLL_IDX_VNODE_IF_NAME_VMCS
										.ordinal()));
			} else {
				valVtnMappingControllerStStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.valVtnMappingControllerStIndex.UPLL_IDX_VNODE_IF_NAME_VMCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
		}

		LOG.info("Value Structure: " + valVtnMappingControllerStStruct.toString());
		LOG.trace("Complete getValVtnMappingControllerStStruct");
		return valVtnMappingControllerStStruct;
	}
	

	// VTN Data Flow key Structure
	public final IpcStruct getKeyVtnDataflowStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		LOG.trace("Start getKeyVtnDataflowStruct");
		final IpcStruct keyVtnDataFlowStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyVtnDataflow.getValue());
		IpcStruct keyVtnStruct = null;
		if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.ONE.ordinal()) {
			keyVtnStruct = getKeyVtnStruct(requestBody, uriParameters);
		}
		keyVtnDataFlowStruct.set(VtnServiceIpcConsts.VTNKEY, keyVtnStruct);
		if (requestBody != null) {
			keyVtnDataFlowStruct
					.set(VtnServiceIpcConsts.VNODEID, IpcDataUnitWrapper
							.setIpcUint8ArrayValue((requestBody
									.get(VtnServiceJsonConsts.VNODENAME)
									.getAsString())));
			if (requestBody.has(VtnServiceJsonConsts.VLANID)) {
				LOG.debug("Valid VLAN ID Case");
				keyVtnDataFlowStruct.set(
						VtnServiceIpcConsts.VLANID1,
						IpcDataUnitWrapper.setIpcUint16Value(requestBody.get(
								VtnServiceJsonConsts.VLANID).getAsString()));
			} else if (requestBody.has(VtnServiceJsonConsts.NO_VLAN_ID)) {
				LOG.debug("Valid NO VLAN ID Case");
				keyVtnDataFlowStruct
						.set(VtnServiceIpcConsts.VLANID1,
								IpcDataUnitWrapper
										.setIpcUint16HexaValue(VtnServiceIpcConsts.VLAN_ID_DEFAULT_VALUE));
			}
			IpcDataUnitWrapper.setMacAddress(keyVtnDataFlowStruct,
					VtnServiceIpcConsts.SRC_MAC_ADDRESS,
					requestBody.get(VtnServiceJsonConsts.SRCMACADDR)
							.getAsString(), 0);
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVtnDataflowStruct");
		}
		LOG.info("Key Structure: " + keyVtnDataFlowStruct.toString());
		LOG.trace("Complete getKeyVtnDataflowStruct");
		return keyVtnDataFlowStruct;
	}

	/**
	 * Gets the key DataFlow struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key DataFlow struct
	 */

	public final IpcStruct getKeyDataFlowStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		LOG.trace("Start getKeyDataFlowStruct");
		final IpcStruct keyDataFlowStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyDataFlow.getValue());
		if (requestBody != null) {
			keyDataFlowStruct.set(VtnServiceIpcConsts.CONTROLLERNAME,
					IpcDataUnitWrapper.setIpcUint8ArrayValue((requestBody
							.get(VtnServiceJsonConsts.CONTROLLERID)
							.getAsString())));
			keyDataFlowStruct
					.set(VtnServiceIpcConsts.SWITCHID, IpcDataUnitWrapper
							.setIpcUint8ArrayValue((requestBody
									.get(VtnServiceJsonConsts.SWITCHID)
									.getAsString())));
			keyDataFlowStruct
					.set(VtnServiceIpcConsts.PORT_ID, IpcDataUnitWrapper
							.setIpcUint8ArrayValue((requestBody
									.get(VtnServiceJsonConsts.PORTNAME)
									.getAsString())));

			if (requestBody.has(VtnServiceJsonConsts.VLANID)) {
				LOG.debug("Valid VLAN ID Case");
				keyDataFlowStruct.set(
						VtnServiceJsonConsts.VLANID,
						IpcDataUnitWrapper.setIpcUint16Value(requestBody.get(
								VtnServiceJsonConsts.VLANID).getAsString()));
			} else if (requestBody.has(VtnServiceJsonConsts.NO_VLAN_ID)) {
				LOG.debug("Valid NO VLAN ID Case");
				keyDataFlowStruct
						.set(VtnServiceJsonConsts.VLANID,
								IpcDataUnitWrapper
										.setIpcUint16HexaValue(VtnServiceIpcConsts.VLAN_ID_DEFAULT_VALUE));
			}
			IpcDataUnitWrapper.setMacAddress(keyDataFlowStruct,
					VtnServiceIpcConsts.SRC_MAC_ADDRESS,
					requestBody.get(VtnServiceJsonConsts.SRCMACADDR)
							.getAsString(), 0);
		} else {
			LOG.error("request body and uri parameters are not correct for getKeyDataFlowStruct");
		}
		LOG.info("Key Structure: " + keyDataFlowStruct.toString());
		LOG.trace("Complete getKeyDataFlowStruct");
		return keyDataFlowStruct;
	}

	// key_ctr_dataflow

	public final IpcStruct getKeyCtrDataFlowStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		IpcStruct keyCtrStruct = null;

		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.ONE.ordinal()) {
			// create key structure for Controller
			keyCtrStruct = getKeyCtrStruct(requestBody, uriParameters);
		}
		LOG.trace("Start getKeyCtrDataFlowStruct");
		final IpcStruct keyCtrDataFlowStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyCtrDataFlow.getValue());
		if (requestBody != null) {
			keyCtrDataFlowStruct.set(VtnServiceIpcConsts.CTR_KEY, keyCtrStruct);
			keyCtrDataFlowStruct.set(VtnServiceIpcConsts.FLOW_ID,
					IpcDataUnitWrapper.setIpcUint64Value((requestBody
							.get(VtnServiceJsonConsts.FLOW_ID).getAsString())));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyCtrDataFlowStruct");
		}
		LOG.info("Key Structure: " + keyCtrDataFlowStruct.toString());
		LOG.trace("Complete getKeyCtrDataFlowStruct");
		return keyCtrDataFlowStruct;
	}

	// Added New key Structure and Value Structure for U13 Requirement
	/**
	 * Gets the key Vterm struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key Vterm struct
	 */
	public IpcStruct getKeyVtermStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		LOG.trace("Start getKeyVtermStruct");
		final IpcStruct keyVtnVtermStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyVterm.getValue());
		IpcStruct keyVtnStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.ONE.ordinal()) {
			keyVtnStruct = getKeyVtnStruct(requestBody,
					uriParameters.subList(0, 1));
		}
		keyVtnVtermStruct.set(VtnServiceIpcConsts.VTNKEY, keyVtnStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.VTERMINAL)
				&& ((JsonObject) requestBody
						.get(VtnServiceJsonConsts.VTERMINAL))
						.has(VtnServiceJsonConsts.VTERMINAL_NAME)) {
			keyVtnVtermStruct.set(VtnServiceIpcConsts.VTERMINAL_NAME,
					IpcDataUnitWrapper
							.setIpcUint8ArrayValue(((JsonObject) requestBody
									.get(VtnServiceJsonConsts.VTERMINAL)).get(
									VtnServiceJsonConsts.VTERMINAL_NAME)
									.getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.TWO.ordinal()) {
			keyVtnVtermStruct.set(VtnServiceIpcConsts.VTERMINAL_NAME,
					IpcDataUnitWrapper.setIpcUint8ArrayValue(uriParameters
							.get(1)));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVtermStruct");
		}
		LOG.trace("Complete getKeyVtermStruct");
		return keyVtnVtermStruct;
	}

	/**
	 * Gets the val Vterminal struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the Val Vterminal struct
	 */
	public IpcStruct getValVtermStruct(final JsonObject requestBody,
			final List<String> uriParameters) {

		LOG.trace("Start getValVtermStruct");
		final IpcStruct ValVtermStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValVterm.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.VTERMINAL)) {
			JsonObject vterminal = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VTERMINAL);
			if (vterminal.has(VtnServiceJsonConsts.DESCRIPTION)) {
				ValVtermStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.valVtermIndex.UPLL_IDX_DESC_VTERM
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				ValVtermStruct
						.set(VtnServiceIpcConsts.VTERM_DESCRIPTION,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vterminal
														.get(VtnServiceJsonConsts.DESCRIPTION)
														.getAsString(),
												ValVtermStruct,
												UncStructIndexEnum.valVtermIndex.UPLL_IDX_DESC_VTERM
														.ordinal()));
			} else {
				ValVtermStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.valVtermIndex.UPLL_IDX_DESC_VTERM
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (vterminal.has(VtnServiceJsonConsts.CONTROLLERID)) {
				ValVtermStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.valVtermIndex.UPLL_IDX_CONTROLLER_ID_VTERM
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				ValVtermStruct
						.set(VtnServiceJsonConsts.CONTROLLERID,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vterminal
														.get(VtnServiceJsonConsts.CONTROLLERID)
														.getAsString(),
												ValVtermStruct,
												UncStructIndexEnum.valVtermIndex.UPLL_IDX_CONTROLLER_ID_VTERM
														.ordinal()));
			} else {
				ValVtermStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.valVtermIndex.UPLL_IDX_CONTROLLER_ID_VTERM
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (vterminal.has(VtnServiceJsonConsts.DOMAINID)) {
				ValVtermStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.valVtermIndex.UPLL_IDX_DOMAIN_ID_VTERM
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				ValVtermStruct
						.set(VtnServiceJsonConsts.DOMAINID,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vterminal
														.get(VtnServiceJsonConsts.DOMAINID)
														.getAsString(),
												ValVtermStruct,
												UncStructIndexEnum.valVtermIndex.UPLL_IDX_DOMAIN_ID_VTERM
														.ordinal()));
			} else {
				ValVtermStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.valVtermIndex.UPLL_IDX_DOMAIN_ID_VTERM
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
		} else {
			LOG.warning("request body and uri parameters are not correct for getValVtermStruct");
		}
		LOG.trace("Complete getValVtermStruct");
		return ValVtermStruct;
	}

	/**
	 * Gets the key Vterm Interface struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key Vterm Interface struct
	 */
	public IpcStruct getKeyVtermIfStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		LOG.trace("Start getKeyVtermIfStruct");
		final IpcStruct keyVtnVtermIfStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyVtermIf.getValue());
		IpcStruct keyVtnVtermStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.TWO.ordinal()) {
			keyVtnVtermStruct = getKeyVtermStruct(requestBody,
					uriParameters.subList(0, 2));
		}
		keyVtnVtermIfStruct
				.set(VtnServiceIpcConsts.VTERMKEY, keyVtnVtermStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.INTERFACE)
				&& ((JsonObject) requestBody
						.get(VtnServiceJsonConsts.INTERFACE))
						.has(VtnServiceJsonConsts.IFNAME)) {
			keyVtnVtermIfStruct
					.set(VtnServiceIpcConsts.IFNAME, IpcDataUnitWrapper
							.setIpcUint8ArrayValue(((JsonObject) requestBody
									.get(VtnServiceJsonConsts.INTERFACE)).get(
									VtnServiceJsonConsts.IFNAME).getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.THREE.ordinal()) {
			keyVtnVtermIfStruct.set(VtnServiceIpcConsts.IFNAME,
					IpcDataUnitWrapper.setIpcUint8ArrayValue(uriParameters
							.get(2)));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVtermIfStruct");
		}
		LOG.trace("Complete getKeyVtermIfStruct");
		return keyVtnVtermIfStruct;
	}

	/**
	 * Gets the val Vterminal If struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the Val Vterminal If struct
	 */
	public IpcStruct getValVtermIfStruct(final JsonObject requestBody,
			final List<String> uriParameters) {

		LOG.trace("Start getValVtermIfStruct");
		final IpcStruct ValVtermIfStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValVtermIf.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.INTERFACE)) {
			JsonObject vterminal = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.INTERFACE);
			if (vterminal.has(VtnServiceJsonConsts.DESCRIPTION)) {
				ValVtermIfStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.valVtermIfIndex.UPLL_IDX_DESC_VTERMI
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				ValVtermIfStruct
						.set(VtnServiceJsonConsts.DESCRIPTION,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												vterminal
														.get(VtnServiceJsonConsts.DESCRIPTION)
														.getAsString(),
												ValVtermIfStruct,
												UncStructIndexEnum.valVtermIfIndex.UPLL_IDX_DESC_VTERMI
														.ordinal()));
			} else {
				ValVtermIfStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.valVtermIfIndex.UPLL_IDX_DESC_VTERMI
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (requestBody.getAsJsonObject(VtnServiceJsonConsts.INTERFACE)
					.has(VtnServiceJsonConsts.ADMINSTATUS)) {
				ValVtermIfStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.valVtermIfIndex.UPLL_IDX_ADMIN_STATUS_VTERMI
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				if (requestBody.getAsJsonObject(VtnServiceJsonConsts.INTERFACE)
						.get(VtnServiceJsonConsts.ADMINSTATUS).getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.ENABLE)) {
					ValVtermIfStruct
							.set(VtnServiceJsonConsts.ADMIN_STATUS,
									IpcDataUnitWrapper
											.setIpcUint8Value(
													UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_ENABLE
															.getValue(),
													ValVtermIfStruct,
													UncStructIndexEnum.valVtermIfIndex.UPLL_IDX_ADMIN_STATUS_VTERMI
															.ordinal()));
				} else if (requestBody
						.getAsJsonObject(VtnServiceJsonConsts.INTERFACE)
						.get(VtnServiceJsonConsts.ADMINSTATUS).getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.DISABLE)) {
					ValVtermIfStruct
							.set(VtnServiceJsonConsts.ADMIN_STATUS,
									IpcDataUnitWrapper
											.setIpcUint8Value(
													UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_DISABLE
															.getValue(),
													ValVtermIfStruct,
													UncStructIndexEnum.valVtermIfIndex.UPLL_IDX_ADMIN_STATUS_VTERMI
															.ordinal()));
				} else {
					ValVtermIfStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.valVtermIfIndex.UPLL_IDX_ADMIN_STATUS_VTERMI
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
													.ordinal()));
				}
				LOG.debug("adminstatus:"
						+ requestBody
								.getAsJsonObject(VtnServiceJsonConsts.INTERFACE)
								.get(VtnServiceJsonConsts.ADMINSTATUS)
								.getAsString());

			} else {
				ValVtermIfStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.valVtermIfIndex.UPLL_IDX_ADMIN_STATUS_VTERMI
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}

		} else if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.PORTMAP)) {
			ValVtermIfStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.valVtermIfIndex.UPLL_IDX_ADMIN_STATUS_VTERMI
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			ValVtermIfStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.valVtermIfIndex.UPLL_IDX_DESC_VTERMI
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			ValVtermIfStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.valVtermIfIndex.UPLL_IDX_PM_VTERMI
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
											.ordinal()));
			final IpcStruct valPortMapStruct = getValPortMapStruct(requestBody,
					uriParameters);
			ValVtermIfStruct.set(VtnServiceIpcConsts.PORTMAP, valPortMapStruct);
		} else if (requestBody == null) {
			final IpcStruct valPortMapStruct = IpcDataUnitWrapper
					.setIpcStructValue(UncStructEnum.ValPortMap.getValue());
			ValVtermIfStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.valVtermIfIndex.UPLL_IDX_ADMIN_STATUS_VTERMI
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			ValVtermIfStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.valVtermIfIndex.UPLL_IDX_DESC_VTERMI
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
			ValVtermIfStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.valVtermIfIndex.UPLL_IDX_PM_VTERMI
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
											.ordinal()));
			valPortMapStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_LOGICAL_PORT_ID_PM
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
											.ordinal()));
			valPortMapStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_VLAN_ID_PM
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
											.ordinal()));
			valPortMapStruct
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_TAGGED_PM
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
											.ordinal()));
			ValVtermIfStruct.set(VtnServiceIpcConsts.PORTMAP, valPortMapStruct);
		} else {
			LOG.warning("request body and uri parameters are not correct for getValVtermIfStruct");
		}
		LOG.trace("Complete getValVtermIfStruct");
		return ValVtermIfStruct;
	}

	/**
	 * Gets the key Vterm Interface Flowfilter struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key Vterm Interface Flowfilter struct
	 */
	public IpcStruct getKeyVtermIfFlowfilterStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		LOG.trace("Start getKeyVtermIfFlowfilterStruct");
		final IpcStruct keyVtnVtermIfFlowFilterStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyVtermIfFlowfilter
						.getValue());
		IpcStruct keyVtnVtermIf = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.THREE.ordinal()) {
			keyVtnVtermIf = getKeyVtermIfStruct(requestBody,
					uriParameters.subList(0, 3));
		}
		keyVtnVtermIfFlowFilterStruct.set(VtnServiceIpcConsts.IFKEY,
				keyVtnVtermIf);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.FLOWFILTER)
				&& ((JsonObject) requestBody
						.get(VtnServiceJsonConsts.FLOWFILTER))
						.has(VtnServiceJsonConsts.FFTYPE)) {
			if (requestBody.getAsJsonObject(VtnServiceJsonConsts.FLOWFILTER)
					.get(VtnServiceJsonConsts.FFTYPE).getAsString()
					.equalsIgnoreCase(VtnServiceJsonConsts.IN)) {
				keyVtnVtermIfFlowFilterStruct
						.set(VtnServiceIpcConsts.DIRECTION,
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_IN
												.ordinal()));
			} else {
				keyVtnVtermIfFlowFilterStruct
						.set(VtnServiceIpcConsts.DIRECTION,
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_OUT
												.ordinal()));
			}
			LOG.debug("ff_type:"
					+ requestBody
							.getAsJsonObject(VtnServiceJsonConsts.FLOWFILTER)
							.get(VtnServiceJsonConsts.FFTYPE).getAsString());
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.FOUR.ordinal()) {
			if (uriParameters.get(3).equalsIgnoreCase(VtnServiceJsonConsts.IN)) {
				keyVtnVtermIfFlowFilterStruct
						.set(VtnServiceIpcConsts.DIRECTION,
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_IN
												.ordinal()));
			} else {

				keyVtnVtermIfFlowFilterStruct
						.set(VtnServiceIpcConsts.DIRECTION,
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_OUT
												.ordinal()));
			}
			LOG.debug("ff_type:" + uriParameters.get(3));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVtermIfFlowfilterStruct");
		}
		LOG.trace("Complete getKeyVtermIfFlowfilterStruct");
		return keyVtnVtermIfFlowFilterStruct;
	}

	/**
	 * Gets the key Vterm Interface Flowfilter Entry struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key Vterm Interface Flowfilter Entry struct
	 */
	public IpcStruct getKeyVtermIfFlowfilterEntryStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		LOG.trace("Start getKeyVtermIfFlowfilterEntryStruct");

		final IpcStruct keyVtnVtermIfFlowfilterEntryStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyVtermIfFlowfilterEntry
						.getValue());
		IpcStruct keyVtnVtermIfFlowFilterStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.FOUR.ordinal()) {
			keyVtnVtermIfFlowFilterStruct = getKeyVtermIfFlowfilterStruct(
					requestBody, uriParameters.subList(0, 4));
		}
		keyVtnVtermIfFlowfilterEntryStruct.set(
				VtnServiceIpcConsts.FLOWFILTERKEY,
				keyVtnVtermIfFlowFilterStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.FLOWFILTERENTRY)
				&& ((JsonObject) requestBody
						.get(VtnServiceJsonConsts.FLOWFILTERENTRY))
						.has(VtnServiceJsonConsts.SEQNUM)) {
			keyVtnVtermIfFlowfilterEntryStruct.set(
					VtnServiceJsonConsts.SEQUENCENUM, IpcDataUnitWrapper
							.setIpcUint16Value(((JsonObject) requestBody
									.get(VtnServiceJsonConsts.FLOWFILTERENTRY))
									.get(VtnServiceJsonConsts.SEQNUM)
									.getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.FIVE.ordinal()) {
			keyVtnVtermIfFlowfilterEntryStruct.set(
					VtnServiceJsonConsts.SEQUENCENUM,
					IpcDataUnitWrapper.setIpcUint16Value(uriParameters.get(4)));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVtermIfFlowfilterEntryStruct");
		}
		LOG.trace("Complete getKeyVtermIfFlowfilterEntryStruct");
		return keyVtnVtermIfFlowfilterEntryStruct;
	}

	/**
	 * Gets the key Policing Profile struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key Policing Profile struct
	 */
	public IpcStruct getKeyPolicingProfileStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		LOG.trace("Start getKeyPolicingprofileStruct");
		final IpcStruct keyPolicingStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyPolicingProfile.getValue());
		String profileName = null;
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.POLICINGPROFILE)) {
			final JsonObject policingProfileJsonObject = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.POLICINGPROFILE);
			if (policingProfileJsonObject
					.has(VtnServiceJsonConsts.PROFILE_NAME)) {
				LOG.debug("profile_name:"
						+ policingProfileJsonObject.get(
								VtnServiceJsonConsts.PROFILE_NAME)
								.getAsString());
				keyPolicingStruct
						.set(VtnServiceIpcConsts.POLICING_PROFILE_NAME,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue((policingProfileJsonObject
												.get(VtnServiceJsonConsts.PROFILE_NAME)
												.getAsString())));
			}
		} else if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.ONE.ordinal()) {
			profileName = uriParameters.get(0);
			LOG.debug("profile_name: " + profileName);
			keyPolicingStruct.set(VtnServiceIpcConsts.POLICING_PROFILE_NAME,
					IpcDataUnitWrapper.setIpcUint8ArrayValue(profileName));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyPolicingprofileStruct");
		}
		LOG.trace("Complete getKeyPolicingprofileStruct");
		return keyPolicingStruct;
	}

	/**
	 * Gets val_policingmap struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the valflowfilterEntry struct
	 */
	public final IpcStruct getValPolicingmapStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		LOG.trace("Start getValPolicingmapStruct");
		final IpcStruct valVbrPolicingmapStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValPolicingmap.getValue());
		if (requestBody.has(VtnServiceJsonConsts.POLICINGMAP)
				&& requestBody.get(VtnServiceJsonConsts.POLICINGMAP)
						.isJsonObject()) {
			final JsonObject policingmap = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.POLICINGMAP);
			if (policingmap.has(VtnServiceJsonConsts.PROFILE_NAME)) {
				String profileName = policingmap.getAsJsonPrimitive(
						VtnServiceJsonConsts.PROFILE_NAME).getAsString();
				valVbrPolicingmapStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValPolicingMapIndex.UPLL_IDX_POLICERNAME_PM
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVbrPolicingmapStruct
						.set(VtnServiceIpcConsts.POLICERNAME,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												profileName,
												valVbrPolicingmapStruct,
												UncStructIndexEnum.ValPolicingMapIndex.UPLL_IDX_POLICERNAME_PM
														.ordinal()));
			} else {
				valVbrPolicingmapStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValPolicingMapIndex.UPLL_IDX_POLICERNAME_PM
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
		} else {
			LOG.warning("request body is not correct for getValPolicingmapStruct");
		}
		LOG.trace("Complete getValPolicingmapStruct");
		return valVbrPolicingmapStruct;
	}

	/**
	 * Gets the key Policing Profile Entry struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key Policing Profile Entry struct
	 */
	public IpcStruct getKeyPolicingProfileEntryStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		LOG.trace("Start getKeyPolicingprofileEntryStruct");
		final IpcStruct keyPolicingProfileEntryStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyPolicingProfileEntry
						.getValue());
		IpcStruct keyPolicingprofileStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.ONE.ordinal()) {
			keyPolicingprofileStruct = getKeyPolicingProfileStruct(requestBody,
					uriParameters.subList(0, 1));
		}
		keyPolicingProfileEntryStruct.set(
				VtnServiceIpcConsts.POLICING_PROFILE_KEY,
				keyPolicingprofileStruct);
		String seqNum = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.TWO.ordinal()) {
			seqNum = uriParameters.get(1);
			keyPolicingProfileEntryStruct.set(VtnServiceIpcConsts.SEQUENCENUM,
					IpcDataUnitWrapper.setIpcUint8Value(seqNum));
		} else if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.POLICINGPROFILEENTRY)
				&& ((JsonObject) requestBody
						.get(VtnServiceJsonConsts.POLICINGPROFILEENTRY))
						.has(VtnServiceJsonConsts.SEQNUM)) {
			keyPolicingProfileEntryStruct
					.set(VtnServiceIpcConsts.SEQUENCENUM,
							IpcDataUnitWrapper.setIpcUint8Value(((JsonObject) requestBody
									.get(VtnServiceJsonConsts.POLICINGPROFILEENTRY))
									.get(VtnServiceJsonConsts.SEQNUM)
									.getAsString()));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVtermIfFlowfilterEntryStruct");
		}
		LOG.trace("Complete getKeyPolicingprofileEntryStruct");
		return keyPolicingProfileEntryStruct;
	}

	/**
	 * Gets the val PolicingProfileEntry If struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the Val Policing Profile Entry Structure
	 */
	public IpcStruct getValPolicingProfileEntryStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		LOG.trace("Start getValPolicingProfileEntryStruct");
		final IpcStruct valPolicingProfileEntryStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValPolicingProfileEntry
						.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.POLICINGPROFILEENTRY)) {
			JsonObject profilEntryJson = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.POLICINGPROFILEENTRY);
			// add fl_name Json parameter in IpcValueStructure
			if (profilEntryJson.has(VtnServiceJsonConsts.FLNAME)) {
				valPolicingProfileEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.valPolicingProfileEntryIndex.UPLL_IDX_FLOWLIST_PPE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valPolicingProfileEntryStruct
						.set(VtnServiceIpcConsts.FLOWLIST,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												profilEntryJson
														.get(VtnServiceJsonConsts.FLNAME)
														.getAsString(),
												valPolicingProfileEntryStruct,
												UncStructIndexEnum.valPolicingProfileEntryIndex.UPLL_IDX_FLOWLIST_PPE
														.ordinal()));
			} else {
				valPolicingProfileEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.valPolicingProfileEntryIndex.UPLL_IDX_FLOWLIST_PPE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}

			if (profilEntryJson.has(VtnServiceJsonConsts.TWORATETHREECOLOR)) {
				JsonObject twoRateThreeColorJson = profilEntryJson
						.getAsJsonObject(VtnServiceJsonConsts.TWORATETHREECOLOR);

				if (twoRateThreeColorJson.has(VtnServiceJsonConsts.METER)) {
					JsonObject meterJson = twoRateThreeColorJson
							.getAsJsonObject(VtnServiceJsonConsts.METER);
					// add rateunit Json parameter in IpcValueStructure
					if (meterJson.has(VtnServiceJsonConsts.UNIT)) {
						valPolicingProfileEntryStruct
								.set(VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.valPolicingProfileEntryIndex.UPLL_IDX_RATE_PPE
												.ordinal(),
										IpcDataUnitWrapper
												.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
														.ordinal()));
						String rateValue = null;
						if (meterJson.get(VtnServiceJsonConsts.UNIT)
								.getAsString()
								.equalsIgnoreCase(VtnServiceJsonConsts.KBPS)) {
							rateValue = String
									.valueOf(UncStructIndexEnum.ValPolicingProfileRateType.UPLL_POLICINGPROFILE_RATE_KBPS
											.ordinal());
						} else if (meterJson.get(VtnServiceJsonConsts.UNIT)
								.getAsString()
								.equalsIgnoreCase(VtnServiceJsonConsts.PPS)) {
							rateValue = String
									.valueOf(UncStructIndexEnum.ValPolicingProfileRateType.UPLL_POLICINGPROFILE_RATE_PPS
											.ordinal());
						}
						valPolicingProfileEntryStruct
								.set(VtnServiceIpcConsts.RATE,
										IpcDataUnitWrapper
												.setIpcUint8Value(
														rateValue,
														valPolicingProfileEntryStruct,
														UncStructIndexEnum.valPolicingProfileEntryIndex.UPLL_IDX_RATE_PPE
																.ordinal()));
					} else {
						valPolicingProfileEntryStruct
								.set(VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.valPolicingProfileEntryIndex.UPLL_IDX_RATE_PPE
												.ordinal(),
										IpcDataUnitWrapper
												.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
														.ordinal()));
					}
					// add cir Json parameter in IpcValueStructure
					if (meterJson.has(VtnServiceJsonConsts.CIR)) {
						valPolicingProfileEntryStruct
								.set(VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.valPolicingProfileEntryIndex.UPLL_IDX_CIR_PPE
												.ordinal(),
										IpcDataUnitWrapper
												.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
														.ordinal()));
						valPolicingProfileEntryStruct
								.set(VtnServiceIpcConsts.CIR,
										IpcDataUnitWrapper
												.setIpcUint32Value(
														meterJson
																.get(VtnServiceJsonConsts.CIR)
																.getAsString(),
														valPolicingProfileEntryStruct,
														UncStructIndexEnum.valPolicingProfileEntryIndex.UPLL_IDX_CIR_PPE
																.ordinal()));
					} else {
						valPolicingProfileEntryStruct
								.set(VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.valPolicingProfileEntryIndex.UPLL_IDX_CIR_PPE
												.ordinal(),
										IpcDataUnitWrapper
												.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
														.ordinal()));
					}
					// add cbs Json parameter in IpcValueStructure
					if (meterJson.has(VtnServiceJsonConsts.CBS)) {
						valPolicingProfileEntryStruct
								.set(VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.valPolicingProfileEntryIndex.UPLL_IDX_CBS_PPE
												.ordinal(),
										IpcDataUnitWrapper
												.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
														.ordinal()));
						valPolicingProfileEntryStruct
								.set(VtnServiceIpcConsts.CBS,
										IpcDataUnitWrapper
												.setIpcUint32Value(
														meterJson
																.get(VtnServiceJsonConsts.CBS)
																.getAsString(),
														valPolicingProfileEntryStruct,
														UncStructIndexEnum.valPolicingProfileEntryIndex.UPLL_IDX_CBS_PPE
																.ordinal()));
					} else {
						valPolicingProfileEntryStruct
								.set(VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.valPolicingProfileEntryIndex.UPLL_IDX_CBS_PPE
												.ordinal(),
										IpcDataUnitWrapper
												.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
														.ordinal()));
					}
					// add pir Json parameter in IpcValueStructure
					if (meterJson.has(VtnServiceJsonConsts.PIR)) {
						valPolicingProfileEntryStruct
								.set(VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.valPolicingProfileEntryIndex.UPLL_IDX_PIR_PPE
												.ordinal(),
										IpcDataUnitWrapper
												.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
														.ordinal()));
						valPolicingProfileEntryStruct
								.set(VtnServiceIpcConsts.PIR,
										IpcDataUnitWrapper
												.setIpcUint32Value(
														meterJson
																.get(VtnServiceJsonConsts.PIR)
																.getAsString(),
														valPolicingProfileEntryStruct,
														UncStructIndexEnum.valPolicingProfileEntryIndex.UPLL_IDX_PIR_PPE
																.ordinal()));
					} else {
						valPolicingProfileEntryStruct
								.set(VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.valPolicingProfileEntryIndex.UPLL_IDX_PIR_PPE
												.ordinal(),
										IpcDataUnitWrapper
												.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
														.ordinal()));
					}
					// add pbs Json parameter in IpcValueStructure
					if (meterJson.has(VtnServiceJsonConsts.PBS)) {
						valPolicingProfileEntryStruct
								.set(VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.valPolicingProfileEntryIndex.UPLL_IDX_PBS_PPE
												.ordinal(),
										IpcDataUnitWrapper
												.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
														.ordinal()));
						valPolicingProfileEntryStruct
								.set(VtnServiceIpcConsts.PBS,
										IpcDataUnitWrapper
												.setIpcUint32Value(
														meterJson
																.get(VtnServiceJsonConsts.PBS)
																.getAsString(),
														valPolicingProfileEntryStruct,
														UncStructIndexEnum.valPolicingProfileEntryIndex.UPLL_IDX_PBS_PPE
																.ordinal()));
					} else {
						valPolicingProfileEntryStruct
								.set(VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.valPolicingProfileEntryIndex.UPLL_IDX_PBS_PPE
												.ordinal(),
										IpcDataUnitWrapper
												.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
														.ordinal()));
					}
				}

				List<String> actionParamName = new ArrayList<String>();

				/*
				 * set values for green-action
				 */
				if (twoRateThreeColorJson.has(VtnServiceJsonConsts.GREENACTION)) {
					JsonObject greenActionJson = twoRateThreeColorJson
							.getAsJsonObject(VtnServiceJsonConsts.GREENACTION);
					actionParamName.add(VtnServiceIpcConsts.GREENACTION);
					actionParamName
							.add(VtnServiceIpcConsts.GREENACTIONPRIORITY);
					actionParamName.add(VtnServiceIpcConsts.GREENACTIONDSCP);
					actionParamName
							.add(VtnServiceIpcConsts.GREENACTIONDROPPRECEDENCE);
					setActionParams(
							valPolicingProfileEntryStruct,
							greenActionJson,
							actionParamName,
							UncStructIndexEnum.valPolicingProfileEntryIndex.UPLL_IDX_GREEN_ACTION_PPE
									.ordinal());
				}

				/*
				 * set values for yellow-action
				 */
				if (twoRateThreeColorJson
						.has(VtnServiceJsonConsts.YELLOWACTION)) {
					JsonObject greenActionJson = twoRateThreeColorJson
							.getAsJsonObject(VtnServiceJsonConsts.YELLOWACTION);
					actionParamName.clear();
					actionParamName.add(VtnServiceIpcConsts.YELLOWACTION);
					actionParamName
							.add(VtnServiceIpcConsts.YELLOWACTIONPRIORITY);
					actionParamName.add(VtnServiceIpcConsts.YELLOWACTIONDSCP);
					actionParamName
							.add(VtnServiceIpcConsts.YELLOWACTIONDROPPRECEDENCE);
					setActionParams(
							valPolicingProfileEntryStruct,
							greenActionJson,
							actionParamName,
							UncStructIndexEnum.valPolicingProfileEntryIndex.UPLL_IDX_YELLOW_ACTION_PPE
									.ordinal());
				}

				/*
				 * set values for red-action
				 */
				if (twoRateThreeColorJson.has(VtnServiceJsonConsts.REDACTION)) {
					JsonObject greenActionJson = twoRateThreeColorJson
							.getAsJsonObject(VtnServiceJsonConsts.REDACTION);
					actionParamName.clear();
					actionParamName.add(VtnServiceIpcConsts.REDACTION);
					actionParamName.add(VtnServiceIpcConsts.REDACTIONPRIORITY);
					actionParamName.add(VtnServiceIpcConsts.REDACTIONDSCP);
					actionParamName
							.add(VtnServiceIpcConsts.REDACTIONDROPPRECEDENCE);
					setActionParams(
							valPolicingProfileEntryStruct,
							greenActionJson,
							actionParamName,
							UncStructIndexEnum.valPolicingProfileEntryIndex.UPLL_IDX_RED_ACTION_PPE
									.ordinal());
				}
			}
		} else {
			LOG.warning("request body and uri parameters are not correct for getValVtermIfStruct");
		}
		LOG.trace("Complete getValPolicingProfileEntryStruct");
		return valPolicingProfileEntryStruct;
	}

	/**
	 * Set action type, priority, dscp and drop precedence parameters to value
	 * structure as per actionParamName list
	 * 
	 * @param valPolicingProfileEntryStruct
	 * @param actionJson
	 *            - JSON containing values
	 * @param actionParamName
	 *            - Parameter names of value structure
	 * @param startIndex
	 *            - index corresponding to parameter names
	 */
	private void setActionParams(IpcStruct valPolicingProfileEntryStruct,
			JsonObject actionJson, List<String> actionParamName, int startIndex) {
		LOG.trace("Start setActionParams()");
		LOG.debug("action json : " + actionJson);

		// set action type to value strucure
		if (actionJson.has(VtnServiceJsonConsts.TYPE)) {
			valPolicingProfileEntryStruct
					.set(VtnServiceIpcConsts.VALID,
							startIndex,
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
											.ordinal()));
			String actionvalue = null;
			if (actionJson.get(VtnServiceJsonConsts.TYPE).getAsString()
					.equalsIgnoreCase(VtnServiceJsonConsts.PASS)) {
				actionvalue = String
						.valueOf(UncStructIndexEnum.ValPolicingProfileAction.UPLL_POLICINGPROFILE_ACT_PASS
								.ordinal());
			} else if (actionJson.get(VtnServiceJsonConsts.TYPE).getAsString()
					.equalsIgnoreCase(VtnServiceJsonConsts.DROP)) {
				actionvalue = String
						.valueOf(UncStructIndexEnum.ValPolicingProfileAction.UPLL_POLICINGPROFILE_ACT_DROP
								.ordinal());
			} else if (actionJson.get(VtnServiceJsonConsts.TYPE).getAsString()
					.equalsIgnoreCase(VtnServiceJsonConsts.PENALTY)) {
				actionvalue = String
						.valueOf(UncStructIndexEnum.ValPolicingProfileAction.UPLL_POLICINGPROFILE_ACT_PENALTY
								.ordinal());
			}
			valPolicingProfileEntryStruct.set(actionParamName.get(0),
					IpcDataUnitWrapper.setIpcUint8Value(actionvalue,
							valPolicingProfileEntryStruct, startIndex++));

		} else {
			valPolicingProfileEntryStruct
					.set(VtnServiceIpcConsts.VALID,
							startIndex++,
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
		}

		// set priority to value strucure
		if (actionJson.has(VtnServiceJsonConsts.PRIORITY)) {
			valPolicingProfileEntryStruct
					.set(VtnServiceIpcConsts.VALID,
							startIndex,
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
											.ordinal()));
			valPolicingProfileEntryStruct.set(actionParamName.get(1),
					IpcDataUnitWrapper.setIpcUint8Value(
							actionJson.get(VtnServiceJsonConsts.PRIORITY)
									.getAsString(),
							valPolicingProfileEntryStruct, startIndex++));
		} else {
			valPolicingProfileEntryStruct
					.set(VtnServiceIpcConsts.VALID,
							startIndex++,
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
		}

		// set dscp to value strucure
		if (actionJson.has(VtnServiceJsonConsts.DSCP)) {
			valPolicingProfileEntryStruct
					.set(VtnServiceIpcConsts.VALID,
							startIndex,
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
											.ordinal()));
			valPolicingProfileEntryStruct.set(actionParamName.get(2),
					IpcDataUnitWrapper.setIpcUint8Value(
							actionJson.get(VtnServiceJsonConsts.DSCP)
									.getAsString(),
							valPolicingProfileEntryStruct, startIndex++));
		} else {
			valPolicingProfileEntryStruct
					.set(VtnServiceIpcConsts.VALID,
							startIndex++,
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));
		}

		// set drop precedence to value strucure
		if (actionJson.has(VtnServiceJsonConsts.DROPPRECEDENCE)) {
			valPolicingProfileEntryStruct
					.set(VtnServiceIpcConsts.VALID,
							startIndex,
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
											.ordinal()));
			valPolicingProfileEntryStruct.set(actionParamName.get(3),
					IpcDataUnitWrapper.setIpcUint8Value(
							actionJson.get(VtnServiceJsonConsts.DROPPRECEDENCE)
									.getAsString(),
							valPolicingProfileEntryStruct, startIndex++));

		} else {
			valPolicingProfileEntryStruct
					.set(VtnServiceIpcConsts.VALID,
							startIndex++,
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));

		}
		LOG.trace("Complete setActionParams()");
	}

	/**
	 * Gets the key Vtn PolicingMap Controller struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the Key Vtn PolicingMap Controller Structure
	 */
	public final IpcStruct getKeyVtnPolicingMapControllerStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		LOG.trace("Start getKeyVtnPolicingMapControllerStruct");
		final IpcStruct keyVtnPolcingMapControllerStruct = new IpcStruct(
				UncStructEnum.KeyVtnPolicingMapController.getValue());
		IpcStruct keyVtnStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.ONE.ordinal()) {
			keyVtnStruct = getKeyVtnStruct(requestBody,
					uriParameters.subList(0, 1));
		}
		keyVtnPolcingMapControllerStruct.set(VtnServiceIpcConsts.VTNKEY,
				keyVtnStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.DOMAINID)
				&& requestBody.has(VtnServiceJsonConsts.CONTROLLERID)) {
			keyVtnPolcingMapControllerStruct.set(VtnServiceIpcConsts.DOMAINID,
					IpcDataUnitWrapper.setIpcUint8ArrayValue(requestBody
							.getAsJsonPrimitive(VtnServiceJsonConsts.DOMAINID)
							.getAsString()));
			keyVtnPolcingMapControllerStruct.set(
					VtnServiceIpcConsts.CONTROLLERNAME, IpcDataUnitWrapper
							.setIpcUint8ArrayValue(requestBody
									.getAsJsonPrimitive(
											VtnServiceJsonConsts.CONTROLLERID)
									.getAsString()));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVtnPolcingMapControllerStruct");
		}
		LOG.trace("Complete getKeyVtnPolicingMapControllerStruct");
		return keyVtnPolcingMapControllerStruct;
	}

	/**
	 * Gets the key Controller Path Policy struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the Key Controller Path Policy Structure
	 */
	public final IpcStruct getKeyCtrPathPolicyStruct(
			final JsonObject requestBody, final List<String> uriParameters) {

		LOG.trace("Start getKeyCtrPathPolicyStruct");
		final IpcStruct keyCtrPathPolicyStruct = new IpcStruct(
				UncStructEnum.KeyCtrPathPolicy.getValue());
		IpcStruct keyCtrStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.ONE.ordinal()) {
			keyCtrStruct = getKeyCtrStruct(requestBody,
					uriParameters.subList(0, 1));
			keyCtrPathPolicyStruct.set(VtnServiceIpcConsts.CTR_KEY,
					keyCtrStruct);
			String policyId = null;
			if (uriParameters.size() == UncIndexEnum.TWO.ordinal()) {
				policyId = uriParameters.get(1);
				keyCtrPathPolicyStruct.set(VtnServiceIpcConsts.POLICYID,
						IpcDataUnitWrapper.setIpcUint16Value(policyId));
			}
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyCtrPathPolicyStruct");
		}
		LOG.trace("Complete getKeyCtrPathPolicyStruct");
		return keyCtrPathPolicyStruct;
	}

	/**
	 * Gets the key Controller Path Policy Link weight struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the Key Controller Path Policy Link weight Structure
	 */
	public final IpcStruct getKeyCtrPpolicyLinkWeightStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		LOG.trace("Start getKeyCtrPpolicyLinkWeightStruct");
		final IpcStruct keyCtrPathPolicyLinkWeightStruct = new IpcStruct(
				UncStructEnum.KeyCtrPpolicyLinkWeight.getValue());
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.TWO.ordinal()) {
			IpcStruct keyCtrPathPolicyKeyStruct = getKeyCtrPathPolicyStruct(
					requestBody, uriParameters.subList(0, 2));
			keyCtrPathPolicyLinkWeightStruct.set(
					VtnServiceIpcConsts.CTR_PATH_POLICY_KEY,
					keyCtrPathPolicyKeyStruct);
			if (uriParameters.size() == UncIndexEnum.FOUR.ordinal()) {
				keyCtrPathPolicyLinkWeightStruct.set(
						VtnServiceIpcConsts.SWITCHID, uriParameters.get(2));
				keyCtrPathPolicyLinkWeightStruct.set(
						VtnServiceIpcConsts.PORT_ID, uriParameters.get(3));
			}
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyCtrPpolicyLinkWeightStruct");
		}
		LOG.trace("Complete getKeyCtrPpolicyLinkWeightStruct");
		return keyCtrPathPolicyLinkWeightStruct;
	}

	/**
	 * Gets the key Controller Path Policy Disable Switch struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the Key Controller Path Policy Link weight Disable Switch
	 *         Structure
	 */
	public final IpcStruct getKeyCtrPpolicyDisableSwitchStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		LOG.trace("Start getKeyCtrPpolicyDisableSwitchStruct");
		final IpcStruct keyCtrPpolicyDisableSwitchStruct = new IpcStruct(
				UncStructEnum.KeyCtrPpolicyDisableSwitch.getValue());
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.TWO.ordinal()) {
			IpcStruct keyCtrPthPolicyKeyStruct = getKeyCtrPathPolicyStruct(
					requestBody, uriParameters.subList(0, 2));
			keyCtrPpolicyDisableSwitchStruct.set(
					VtnServiceIpcConsts.CTR_PATH_POLICY_KEY,
					keyCtrPthPolicyKeyStruct);
			if (uriParameters.size() == UncIndexEnum.FOUR.ordinal()) {
				keyCtrPpolicyDisableSwitchStruct.set(
						VtnServiceIpcConsts.SWITCHID, uriParameters.get(2));
				keyCtrPpolicyDisableSwitchStruct.set(
						VtnServiceIpcConsts.PORT_ID, uriParameters.get(3));
			}
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyCtrPpolicyDisableSwitchStruct");
		}
		LOG.trace("Complete getKeyCtrPpolicyDisableSwitchStruct");
		return keyCtrPpolicyDisableSwitchStruct;
	}

	/**
	 * Gets the key VTN Path Map Entry Struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the Key VTN Path Map Entry Structure
	 */
	public final IpcStruct getKeyVtnPathMapEntryStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		LOG.trace("Start getKeyVtnPathMapEntryStruct");
		final IpcStruct KeyVtnPathMapEntryStruct = new IpcStruct(
				UncStructEnum.KeyVtnPathMapEntry.getValue());
		IpcStruct keyVtnStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.ONE.ordinal()) {
			keyVtnStruct = getKeyVtnStruct(requestBody,
					uriParameters.subList(0, 1));
		}
		KeyVtnPathMapEntryStruct.set(VtnServiceIpcConsts.VTNKEY, keyVtnStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.PATHMAPENTRY)
				&& ((JsonObject) requestBody
						.get(VtnServiceJsonConsts.PATHMAPENTRY))
						.has(VtnServiceJsonConsts.SEQNUM)) {
			KeyVtnPathMapEntryStruct.set(VtnServiceIpcConsts.SEQUENCENUM,
					IpcDataUnitWrapper
							.setIpcUint16Value(((JsonObject) requestBody
									.get(VtnServiceJsonConsts.PATHMAPENTRY))
									.get(VtnServiceJsonConsts.SEQNUM)
									.getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.TWO.ordinal()) {
			KeyVtnPathMapEntryStruct.set(VtnServiceIpcConsts.SEQUENCENUM,
					IpcDataUnitWrapper.setIpcUint16Value(uriParameters.get(1)));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVtnPathMapEntryStruct");
		}
		LOG.trace("Complete getKeyVtnPolicingMapControllerStruct");
		return KeyVtnPathMapEntryStruct;

	}

	/**
	 * Gets the Value VTN Path Map Entry Struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the Value VTN Path Map Entry Structure
	 */
	public final IpcStruct getValVtnPathMapEntryStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		LOG.trace("Start getValVtnPathMapEntryStruct");
		final IpcStruct valVtnPathMapEntryStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValVtnPathMapEntry.getValue());
		if (requestBody.has(VtnServiceJsonConsts.PATHMAPENTRY)
				&& requestBody.get(VtnServiceJsonConsts.PATHMAPENTRY)
						.isJsonObject()) {
			final JsonObject pathMapEntryJsonObject = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.PATHMAPENTRY);
			if (pathMapEntryJsonObject.has(VtnServiceJsonConsts.FLNAME)) {
				String flName = pathMapEntryJsonObject.getAsJsonPrimitive(
						VtnServiceJsonConsts.FLNAME).getAsString();
				valVtnPathMapEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnPathMapEntryIndex.UPLL_IDX_FLOWLIST_NAME_VPME
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVtnPathMapEntryStruct
						.set(VtnServiceIpcConsts.FLOWLIST_NAME,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												flName,
												valVtnPathMapEntryStruct,
												UncStructIndexEnum.ValVtnPathMapEntryIndex.UPLL_IDX_FLOWLIST_NAME_VPME
														.ordinal()));
			} else {
				valVtnPathMapEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnPathMapEntryIndex.UPLL_IDX_FLOWLIST_NAME_VPME
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
		} else {
			LOG.warning("request body is not correct for getValVtnPathMapEntryStruct");
		}
		LOG.trace("Complete getValVtnPathMapEntryStruct");
		return valVtnPathMapEntryStruct;

	}

	/**
	 * Gets the key VTN Path Policy Entry Struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the Key VTN Path Policy Entry Structure
	 */
	public final IpcStruct getKeyVtnPathmapPpolicyEntryStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		LOG.trace("Start getKeyVtnPathmapPpolicyEntryStruct");
		final IpcStruct keyVtnpPolicyEntryStruct = new IpcStruct(
				UncStructEnum.KeyVtnPathmapPpolicyEntry.getValue());
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.TWO.ordinal()) {
			IpcStruct keyVtnPathMapEntryStruct = getKeyVtnPathMapEntryStruct(
					requestBody, uriParameters.subList(0, 2));
			keyVtnpPolicyEntryStruct.set(
					VtnServiceIpcConsts.VTN_PATHMAP_ENTRY_KEY,
					keyVtnPathMapEntryStruct);
		} else {
			LOG.error("mandatory parameters are missing for key-structure");
		}

		// set controller_id and domain_id from request body
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.PATHPOLICYENTRY)
				&& requestBody.get(VtnServiceJsonConsts.PATHPOLICYENTRY)
						.getAsJsonObject()
						.has(VtnServiceJsonConsts.CONTROLLERID)
				&& requestBody.get(VtnServiceJsonConsts.PATHPOLICYENTRY)
						.getAsJsonObject().has(VtnServiceJsonConsts.DOMAINID)) {
			keyVtnpPolicyEntryStruct.set(VtnServiceIpcConsts.CONTROLLERID,
					IpcDataUnitWrapper
							.setIpcUint8ArrayValue(((JsonObject) requestBody
									.get(VtnServiceJsonConsts.PATHPOLICYENTRY))
									.get(VtnServiceJsonConsts.CONTROLLERID)
									.getAsString()));
			keyVtnpPolicyEntryStruct.set(VtnServiceIpcConsts.DOMAINID,
					IpcDataUnitWrapper
							.setIpcUint8ArrayValue(((JsonObject) requestBody
									.get(VtnServiceJsonConsts.PATHPOLICYENTRY))
									.get(VtnServiceJsonConsts.DOMAINID)
									.getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.THREE.ordinal()) {
			String entityId[] = uriParameters.get(2).split(
					VtnServiceJsonConsts.HYPHEN);
			keyVtnpPolicyEntryStruct.set(VtnServiceIpcConsts.CONTROLLERID,
					IpcDataUnitWrapper.setIpcUint8ArrayValue(entityId[0]));
			keyVtnpPolicyEntryStruct.set(VtnServiceIpcConsts.DOMAINID,
					IpcDataUnitWrapper.setIpcUint8ArrayValue(entityId[1]));
		} else {
			LOG.warning("request body and uri parameters are not correct for getKeyVtnPathmapPpolicyEntryStruct");
		}
		LOG.trace("Complete getKeyVtnPathmapPpolicyEntryStruct");
		return keyVtnpPolicyEntryStruct;
	}

	/**
	 * Gets the Value VTN Path Policy Entry Struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the Value VTN Path Policy Entry Structure
	 */
	public final IpcStruct getValVtnPathmapPpolicyEntryStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		LOG.trace("Start getValVtnPathmapPpolicyEntryStruct");
		final IpcStruct valVtnpPolicyEntryStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValVtnPathmapPpolicyEntry
						.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.PATHPOLICYENTRY)
				&& requestBody.get(VtnServiceJsonConsts.PATHPOLICYENTRY)
						.isJsonObject()) {
			final JsonObject pPolicyEntryJsonObject = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.PATHPOLICYENTRY);
			if (pPolicyEntryJsonObject.has(VtnServiceJsonConsts.POLICYID)) {
				String policyId = pPolicyEntryJsonObject.getAsJsonPrimitive(
						VtnServiceJsonConsts.POLICYID).getAsString();
				valVtnpPolicyEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnPathmapPpolicyEntryIndex.UPLL_IDX_POLICY_ID_VPMPPE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVtnpPolicyEntryStruct
						.set(VtnServiceIpcConsts.POLICYID,
								IpcDataUnitWrapper
										.setIpcUint8Value(
												policyId,
												valVtnpPolicyEntryStruct,
												UncStructIndexEnum.ValVtnPathmapPpolicyEntryIndex.UPLL_IDX_POLICY_ID_VPMPPE
														.ordinal()));
			} else {
				valVtnpPolicyEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnPathmapPpolicyEntryIndex.UPLL_IDX_POLICY_ID_VPMPPE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			if (pPolicyEntryJsonObject.has(VtnServiceJsonConsts.AGEOUTTIME)) {
				String ageoutTime = pPolicyEntryJsonObject.getAsJsonPrimitive(
						VtnServiceJsonConsts.AGEOUTTIME).getAsString();
				valVtnpPolicyEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnPathmapPpolicyEntryIndex.UPLL_IDX_AGING_TIME_OUT_VPMPPE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVtnpPolicyEntryStruct
						.set(VtnServiceIpcConsts.AGINGOUTTIME,
								IpcDataUnitWrapper
										.setIpcUint16Value(
												ageoutTime,
												valVtnpPolicyEntryStruct,
												UncStructIndexEnum.ValVtnPathmapPpolicyEntryIndex.UPLL_IDX_AGING_TIME_OUT_VPMPPE
														.ordinal()));
			} else {
				valVtnpPolicyEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnPathmapPpolicyEntryIndex.UPLL_IDX_AGING_TIME_OUT_VPMPPE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
		} else {
			LOG.error("request body is not correct for getValVtnPathmapPpolicyEntryStruct");
		}
		LOG.trace("Complete getValVtnPathmapPpolicyEntryStruct");
		return valVtnpPolicyEntryStruct;
	}

	/**
	 * Gets the key VTN Unified Network struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key VTN Unified Network struct
	 */
	public final IpcStruct getKeyVtnUnifiedStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		// low level structure
		/*
		 * ipc_struct key_vtn_unified { key_vtn vtn_key; unified_nw_id[32]; };
		 */
		LOG.trace("Start getKeyVtnUnifiedStruct");
		final IpcStruct keyVTNUnifiedNetwoekStruct = new IpcStruct(
				UncStructEnum.KeyVtnUnified.getValue());
		IpcStruct keyVtnStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.ONE.ordinal()) {
			keyVtnStruct = getKeyVtnStruct(requestBody,
					uriParameters.subList(0, 1));
		}
		keyVTNUnifiedNetwoekStruct.set(VtnServiceIpcConsts.VTNKEY, keyVtnStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.UNIFIED_NW)
				&& ((JsonObject) requestBody
						.get(VtnServiceJsonConsts.UNIFIED_NW))
						.has(VtnServiceJsonConsts.UNIFIED_NETWORK_NAME)) {

			keyVTNUnifiedNetwoekStruct.set(VtnServiceIpcConsts.UNIFIED_NW_ID,
					IpcDataUnitWrapper
							.setIpcUint8ArrayValue(((JsonObject) requestBody
									.get(VtnServiceJsonConsts.UNIFIED_NW)).get(
									VtnServiceJsonConsts.UNIFIED_NETWORK_NAME)
									.getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.TWO.ordinal()) {
			keyVTNUnifiedNetwoekStruct.set(VtnServiceIpcConsts.UNIFIED_NW_ID,
					uriParameters.get(1));
		} else {
			LOG.debug("request body and uri parameters are not correct for getKeyVTNUnifiedNetworkStruct");
		}
		LOG.info("Key Structure: " + keyVTNUnifiedNetwoekStruct.toString());
		LOG.trace("Complete getKeyVtnUnifiedStruct");
		return keyVTNUnifiedNetwoekStruct;
	}

	/**
	 * Gets the val VTN Unified Network struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the val VTN Unified Network struct
	 */
	public final IpcStruct getValVtnUnifiedStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		/*
		 * ipc_struct val_vtn_unified { UINT8 valid[1]; UINT8 cs_row_status; UINT8
		 * cs_attr[1]; UINT8 spine_id[32]; };
		 */
		LOG.trace("Start getValVtnUnifiedStruct");
		final IpcStruct valVtnUnifiedNetworkStruct = new IpcStruct(
				UncStructEnum.ValVtnUnified.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.UNIFIED_NW)) {
			final JsonObject vtnUnifiedNetwork = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.UNIFIED_NW);
			if (vtnUnifiedNetwork.has(VtnServiceJsonConsts.SPINE_DOMAIN_NAME)) {
				valVtnUnifiedNetworkStruct.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnUnifiedIndex.UPLL_IDX_SPINE_ID_VUNW.ordinal(),
								IpcDataUnitWrapper.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID.ordinal()));
				valVtnUnifiedNetworkStruct.set(VtnServiceIpcConsts.SPINE_ID,
								IpcDataUnitWrapper.setIpcUint8ArrayValue(
												vtnUnifiedNetwork.get(VtnServiceJsonConsts.SPINE_DOMAIN_NAME).getAsString(),
														valVtnUnifiedNetworkStruct,
												UncStructIndexEnum.ValVtnUnifiedIndex.UPLL_IDX_SPINE_ID_VUNW.ordinal()));
			} else {
				valVtnUnifiedNetworkStruct.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnUnifiedIndex.UPLL_IDX_SPINE_ID_VUNW.ordinal(),
								IpcDataUnitWrapper.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()));
			}
		}
		LOG.info("Value Structure: " + valVtnUnifiedNetworkStruct.toString());
		LOG.trace("Complete getValVtnUnifiedStruct");
		return valVtnUnifiedNetworkStruct;
	}

	/**
	 * Gets the key Spine Domain struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key Spine Domain struct
	 */
	public final IpcStruct getKeyUnwSpineDomainStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		// low level structure
		/*
		 * ipc_struct key_unw_spine_domain { key_unified_nw unw_key; unw_spine_id[32];  };
		 */
		LOG.trace("Start getKeyUnwSpineDomainStruct");
		final IpcStruct KeySpineDomainStruct = new IpcStruct(
				UncStructEnum.KeyUnwSpineDomain.getValue());
		IpcStruct KeyUnifiedNwStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.ONE.ordinal()) {
			KeyUnifiedNwStruct = getKeyUnifiedNetworkStruct(requestBody,
					uriParameters.subList(0, 1));
		}
		KeySpineDomainStruct.set(VtnServiceIpcConsts.UNW_KEY, KeyUnifiedNwStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.SPINE_DOMAIN)
				&& ((JsonObject) requestBody
						.get(VtnServiceJsonConsts.SPINE_DOMAIN))
						.has(VtnServiceJsonConsts.SPINE_DOMAIN_NAME)) {

			KeySpineDomainStruct.set(VtnServiceIpcConsts.UNW_SPINE_ID,
					IpcDataUnitWrapper.setIpcUint8ArrayValue(((JsonObject) requestBody
									.get(VtnServiceJsonConsts.SPINE_DOMAIN)).get(
									VtnServiceJsonConsts.SPINE_DOMAIN_NAME).getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.TWO.ordinal()) {
			KeySpineDomainStruct.set(VtnServiceIpcConsts.UNW_SPINE_ID,
					uriParameters.get(1));
		} else {
			LOG.debug("request body and uri parameters are not correct for getKeySpineDomainStruct");
		}
		LOG.info("Key Structure: " + KeySpineDomainStruct.toString());
		LOG.trace("Complete getKeyUnwSpineDomainStruct");
		return KeySpineDomainStruct;
	}

	/**
	 * Gets the val Spine Domain struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the val Spine Domain struct
	 */
	public final IpcStruct getValUnwSpineDomainStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		/*
		 * ipc_struct val_unw_spine_domain { UINT8 valid[3]; UINT8 cs_row_status; UINT8
		 * cs_attr[3]; UINT8 spine_controller_id[32]; UINT8 spine_domain_id[32]; UINT8 unw_label_id[32]; };
		 */
		LOG.trace("Start getValUnwSpineDomainStruct");
		final IpcStruct valSpineDomainStruct = new IpcStruct(
				UncStructEnum.ValUnwSpineDomain.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.SPINE_DOMAIN)) {
			final JsonObject sPineDomain = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.SPINE_DOMAIN);
			if (sPineDomain.has(VtnServiceJsonConsts.CONTROLLERID)) {
				valSpineDomainStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValUnwSpineDomainIndex.UPLL_IDX_SPINE_CONTROLLER_ID_UNWS.ordinal(),
								IpcDataUnitWrapper.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID.ordinal()));
				valSpineDomainStruct.set(VtnServiceIpcConsts.SPINE_CONTROLLER_ID,
								IpcDataUnitWrapper.setIpcUint8ArrayValue(
												sPineDomain.get(VtnServiceJsonConsts.CONTROLLERID).getAsString(),
														valSpineDomainStruct,
												UncStructIndexEnum.ValUnwSpineDomainIndex.UPLL_IDX_SPINE_CONTROLLER_ID_UNWS.ordinal()));
			} else {
				valSpineDomainStruct.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValUnwSpineDomainIndex.UPLL_IDX_SPINE_CONTROLLER_ID_UNWS.ordinal(),
								IpcDataUnitWrapper.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()));
			}
			if (sPineDomain.has(VtnServiceJsonConsts.DOMAINID)) {
				valSpineDomainStruct.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValUnwSpineDomainIndex.UPLL_IDX_SPINE_DOMAIN_ID_UNWS.ordinal(),
								IpcDataUnitWrapper.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID.ordinal()));
				valSpineDomainStruct
						.set(VtnServiceIpcConsts.SPINE_DOMAIN_ID,
								IpcDataUnitWrapper.setIpcUint8ArrayValue(
												sPineDomain.get(VtnServiceJsonConsts.DOMAINID).getAsString(),
														valSpineDomainStruct,
												UncStructIndexEnum.ValUnwSpineDomainIndex.UPLL_IDX_SPINE_DOMAIN_ID_UNWS.ordinal()));
			} else {
				valSpineDomainStruct.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValUnwSpineDomainIndex.UPLL_IDX_SPINE_DOMAIN_ID_UNWS.ordinal(),
								IpcDataUnitWrapper.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()));
			}
			if (sPineDomain.has(VtnServiceJsonConsts.LABEL_NAME)) {
				valSpineDomainStruct.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValUnwSpineDomainIndex.UPLL_IDX_UNW_LABEL_ID_UNWS.ordinal(),
								IpcDataUnitWrapper.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID.ordinal()));
				valSpineDomainStruct
						.set(VtnServiceIpcConsts.UNW_LABEL_ID,
								IpcDataUnitWrapper.setIpcUint8ArrayValue(
												sPineDomain.get(VtnServiceJsonConsts.LABEL_NAME).getAsString(),
														valSpineDomainStruct,
												UncStructIndexEnum.ValUnwSpineDomainIndex.UPLL_IDX_UNW_LABEL_ID_UNWS.ordinal()));
			} else {
				valSpineDomainStruct.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValUnwSpineDomainIndex.UPLL_IDX_UNW_LABEL_ID_UNWS.ordinal(),
								IpcDataUnitWrapper.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()));
			}
		}
		LOG.info("Value Structure: " + valSpineDomainStruct.toString());
		LOG.trace("Complete getValUnwSpineDomainStruct");
		return valSpineDomainStruct;
	}

	/**
	 * Gets the key unw label struct for label APIs
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key unw label struct
	 */
	public final IpcStruct getKeyUnwLabelStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		LOG.trace("Start getKeyUnwLabelStruct");
		final IpcStruct keyUnwLabelStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyUnwLabel.getValue());
		IpcStruct keyUnifiedNetworkStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.ONE.ordinal()) {
			keyUnifiedNetworkStruct = getKeyUnifiedNetworkStruct(requestBody,
					uriParameters.subList(0, 1));
		}
		keyUnwLabelStruct.set(VtnServiceIpcConsts.UNIFIED_NW_KEY, keyUnifiedNetworkStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.LABEL)
				&& ((JsonObject) requestBody.get(VtnServiceJsonConsts.LABEL))
						.has(VtnServiceJsonConsts.LABEL_NAME)) {
			keyUnwLabelStruct.set(VtnServiceIpcConsts.UNW_LABEL_ID,
					IpcDataUnitWrapper
							.setIpcUint8ArrayValue(((JsonObject) requestBody
									.get(VtnServiceJsonConsts.LABEL)).get(
									VtnServiceJsonConsts.LABEL_NAME)
									.getAsString()));
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.TWO.ordinal()) {
			keyUnwLabelStruct.set(VtnServiceIpcConsts.UNW_LABEL_ID,
					IpcDataUnitWrapper.setIpcUint8ArrayValue(uriParameters
							.get(1)));
		} else {
			LOG.debug("request body and uri parameters are not correct for getKeyUnwLabelStruct");
		}
		LOG.info("Key Structure: " + keyUnwLabelStruct.toString());
		LOG.trace("Complete getKeyUnwLabelStruct");

		return keyUnwLabelStruct;
	}

	/**
	 * Gets the val unw label struct for label APIs
	 * 
	 * @param requestBody
	 *            the request body
	 * @return the val unw label struct
	 */
	public final IpcStruct getValUnwLabelStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		LOG.trace("Start getValUnwLabelStruct");
		final IpcStruct valUnwLabelStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValUnwLabel.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.LABEL)) {
			final JsonObject label = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.LABEL);
			if (label.has(VtnServiceJsonConsts.MAX_COUNT)) {
				valUnwLabelStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValUnwLabelIndex.UPLL_IDX_MAX_COUNT_UNWL
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valUnwLabelStruct.set(VtnServiceIpcConsts.MAX_COUNT,
					IpcDataUnitWrapper.setIpcUint32Value(
						label.getAsJsonPrimitive(VtnServiceJsonConsts.MAX_COUNT)
							.getAsString(),
						valUnwLabelStruct,
						UncStructIndexEnum.ValUnwLabelIndex.UPLL_IDX_MAX_COUNT_UNWL
								.ordinal()));
			} else {
				valUnwLabelStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValUnwLabelIndex.UPLL_IDX_MAX_COUNT_UNWL
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}

			if (label.has(VtnServiceJsonConsts.RISING_THRESHOLD)) {
				valUnwLabelStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValUnwLabelIndex.UPLL_IDX_RAISING_THRESHOLD_UNWL
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valUnwLabelStruct.set(VtnServiceIpcConsts.RAISING_THRESHOLD,
					IpcDataUnitWrapper.setIpcUint32Value(
						label.getAsJsonPrimitive(VtnServiceJsonConsts.RISING_THRESHOLD)
							.getAsString(),
							valUnwLabelStruct,
							UncStructIndexEnum.ValUnwLabelIndex.UPLL_IDX_RAISING_THRESHOLD_UNWL
									.ordinal()));

			} else {
				valUnwLabelStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValUnwLabelIndex.UPLL_IDX_RAISING_THRESHOLD_UNWL
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}

			if (label.has(VtnServiceJsonConsts.FALLING_THRESHOLD)) {
				valUnwLabelStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValUnwLabelIndex.UPLL_IDX_FALLING_THRESHOLD_UNWL
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valUnwLabelStruct.set(VtnServiceIpcConsts.FALLING_THRESHOLD,
					IpcDataUnitWrapper.setIpcUint32Value(
						label.getAsJsonPrimitive(VtnServiceJsonConsts.FALLING_THRESHOLD)
							.getAsString(),
							valUnwLabelStruct,
							UncStructIndexEnum.ValUnwLabelIndex.UPLL_IDX_FALLING_THRESHOLD_UNWL
									.ordinal()));

			} else {
				valUnwLabelStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValUnwLabelIndex.UPLL_IDX_FALLING_THRESHOLD_UNWL
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
		} else {
			LOG.debug("request body and uri parameters are not correct for getValUnwLabelStruct");
		}

		LOG.info("Value Structure: " + valUnwLabelStruct.toString());
		LOG.trace("Complete getValUnwLabelStruct");
		return valUnwLabelStruct;
	}

	/**
	 * Gets the key unw label range struct for label APIs
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key unw label range struct
	 */
	public final IpcStruct getKeyUnwLabelRangeStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		LOG.trace("Start getKeyUnwLabelRangeStruct");
		final IpcStruct keyUnwLabelRangeStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.KeyUnwLabelRange.getValue());
		IpcStruct keyUnwLabelStruct = null;
		if (uriParameters != null
				&& uriParameters.size() >= UncIndexEnum.TWO.ordinal()) {
			keyUnwLabelStruct = getKeyUnwLabelStruct(requestBody,
					uriParameters.subList(0, 2));
		}
		keyUnwLabelRangeStruct.set(VtnServiceIpcConsts.UNW_LABEL_KEY, keyUnwLabelStruct);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.RANGE)) {
			if (((JsonObject) requestBody.get(VtnServiceJsonConsts.RANGE))
					.has(VtnServiceJsonConsts.RANGE_MIN)) {
				keyUnwLabelRangeStruct.set(VtnServiceIpcConsts.RANGE_MIN,
						IpcDataUnitWrapper.setIpcUint32Value((((JsonObject) requestBody
										.get(VtnServiceJsonConsts.RANGE)).get(
										VtnServiceJsonConsts.RANGE_MIN)
										.getAsString())));
			}

			if (((JsonObject) requestBody.get(VtnServiceJsonConsts.RANGE))
					.has(VtnServiceJsonConsts.RANGE_MAX)) {
				keyUnwLabelRangeStruct.set(VtnServiceIpcConsts.RANGE_MAX,
						IpcDataUnitWrapper.setIpcUint32Value((((JsonObject) requestBody
										.get(VtnServiceJsonConsts.RANGE)).get(
										VtnServiceJsonConsts.RANGE_MAX)
										.getAsString())));
			}
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.THREE.ordinal()) {
			String[] minAndMax = uriParameters.get(2).split(VtnServiceConsts.UNDERSCORE);
				keyUnwLabelRangeStruct.set(VtnServiceIpcConsts.RANGE_MIN,
						IpcDataUnitWrapper.setIpcUint32Value(minAndMax[0]));
				keyUnwLabelRangeStruct.set(VtnServiceIpcConsts.RANGE_MAX,
						IpcDataUnitWrapper.setIpcUint32Value(minAndMax[1]));
		} else {
			LOG.debug("request body and uri parameters are not correct " +
					"for getKeyUnwLabelRangeStruct");
		}
		LOG.info("Key Structure: " + keyUnwLabelRangeStruct.toString());
		LOG.trace("Complete getKeyUnwLabelRangeStruct");

		return keyUnwLabelRangeStruct;
	}

	/**
	 * Gets the val unw label range struct for label APIs
	 * 
	 * @param requestBody
	 *            the request body
	 * @return the val unw label range struct
	 */
	public final IpcStruct getValUnwLabelRangeStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		LOG.trace("Start getValUnwLabelStruct");
		final IpcStruct valUnwLabelRangeStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValUnwLabelRange.getValue());
		LOG.info("Key Structure: " + valUnwLabelRangeStruct.toString());
		LOG.trace("Complete getKeyUnwLabelStruct");

		return valUnwLabelRangeStruct;
	}
}
