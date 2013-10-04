/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.ipc;

import java.util.List;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.ipc.IpcStruct;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceIpcConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcDataUnitWrapper;
import org.opendaylight.vtn.javaapi.ipc.enums.UncIndexEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncPhysicalStructIndexEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncStructEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncStructIndexEnum;

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
	public IpcStruct getKeyVtnStruct(final JsonObject requestBody,
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
		LOG.info("Key Structure: " + keyVtnStruct.toString());
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
	public IpcStruct getValVtnStruct(final JsonObject requestBody,
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
	public IpcStruct getKeyVrtStruct(final JsonObject requestBody,
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
	public IpcStruct getKeyDhcpRelayIfStruct(final JsonObject requestBody,
			final List<String> uriParameters) {

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
	public IpcStruct getKeyFlowListStruct(final JsonObject requestBody,
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
	public IpcStruct getValFlowListStruct(final JsonObject requestBody,
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
	public IpcStruct getKeyFlowListEntryStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
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
	public IpcStruct getValFlowListEntryStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		LOG.trace("Start getValFlowListEntryStruct");
		final IpcStruct valFlowListEntryStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValFlowListEntry.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.FLOWLISTENTRY)) {
			JsonObject flowListEntry = requestBody.getAsJsonObject(VtnServiceJsonConsts.FLOWLISTENTRY);
			if (flowListEntry.has(VtnServiceJsonConsts.MACDSTADDR)) {
				valFlowListEntryStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_MAC_DST_FLE
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				IpcDataUnitWrapper.setMacAddress(
						valFlowListEntryStruct,
						VtnServiceIpcConsts.MACDST,flowListEntry.get(VtnServiceJsonConsts.MACDSTADDR).getAsString(),
						UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_MAC_DST_FLE.ordinal());
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
				IpcDataUnitWrapper.setMacAddress(
						valFlowListEntryStruct,
						VtnServiceIpcConsts.MACSRC,
						flowListEntry.get(VtnServiceJsonConsts.MACSRCADDR).getAsString(),
						UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_MAC_SRC_FLE.ordinal());
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
				valFlowListEntryStruct.set(
						VtnServiceIpcConsts.MAC_ETH_TYPE,
						IpcDataUnitWrapper.setIpcUint16HexaValue(flowListEntry.get(VtnServiceJsonConsts.MACETHERTYPE).getAsString(),
								valFlowListEntryStruct, UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_MAC_ETH_TYPE_FLE.ordinal()));
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
				valFlowListEntryStruct.set(
						VtnServiceIpcConsts.DST_IP,
						IpcDataUnitWrapper.setIpcInet4AddressValue(flowListEntry.get(VtnServiceJsonConsts.IPDSTADDR).getAsString(),
								valFlowListEntryStruct, UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_DST_IP_FLE.ordinal()));
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
				valFlowListEntryStruct.set(
						VtnServiceIpcConsts.DST_IP_PREFIXLEN,
						IpcDataUnitWrapper.setIpcUint8Value(flowListEntry.get(VtnServiceJsonConsts.IPDSTADDRPREFIX).getAsString(), 
								valFlowListEntryStruct, UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_DST_IP_PREFIX_FLE.ordinal()));
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
				valFlowListEntryStruct.set(
						VtnServiceIpcConsts.SRC_IP,
						IpcDataUnitWrapper.setIpcInet4AddressValue(flowListEntry.get(VtnServiceJsonConsts.IPSRCADDR).getAsString(),
								valFlowListEntryStruct, UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_SRC_IP_FLE.ordinal()));
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
				valFlowListEntryStruct.set(
						VtnServiceIpcConsts.SRC_IP_PREFIXLEN,
						IpcDataUnitWrapper.setIpcUint8Value(flowListEntry.get(VtnServiceJsonConsts.IPSRCADDRPREFIX).getAsString(), 
								valFlowListEntryStruct, UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_SRC_IP_PREFIX_FLE.ordinal()));
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
				valFlowListEntryStruct.set(
						VtnServiceIpcConsts.VLAN_PRIORITY,
						IpcDataUnitWrapper.setIpcUint8Value(flowListEntry.get(VtnServiceJsonConsts.MACVLANPRIORITY).getAsString(), 
								valFlowListEntryStruct, UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_VLAN_PRIORITY_FLE.ordinal()));
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
				valFlowListEntryStruct.set(
						VtnServiceIpcConsts.DST_IPV6,
						IpcDataUnitWrapper.setIpcInet6AddressValue(flowListEntry.get(VtnServiceJsonConsts.IPV6DSTADDR).getAsString(),
								valFlowListEntryStruct, UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_DST_IP_V6_FLE.ordinal()));
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
				valFlowListEntryStruct.set(
						VtnServiceIpcConsts.DST_IPV6_PREFIXLEN,
						IpcDataUnitWrapper.setIpcUint8Value(flowListEntry.get(VtnServiceJsonConsts.IPV6DSTADDRPREFIX).getAsString(), 
								valFlowListEntryStruct, UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_DST_IP_V6_PREFIX_FLE.ordinal()));
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
				valFlowListEntryStruct.set(
						VtnServiceIpcConsts.SRC_IPV6,
						IpcDataUnitWrapper.setIpcInet6AddressValue(flowListEntry.get(VtnServiceJsonConsts.IPV6SRCADDR).getAsString(),
								valFlowListEntryStruct, UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_SRC_IP_V6_FLE.ordinal()));
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
				valFlowListEntryStruct.set(
						VtnServiceIpcConsts.SRC_IPV6_PREFIXLEN,
						IpcDataUnitWrapper.setIpcUint8Value(flowListEntry.get(VtnServiceJsonConsts.IPV6SRCADDRPREFIX).getAsString(), 
								valFlowListEntryStruct, UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_SRC_IP_V6_PREFIX_FLE.ordinal()));
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
				valFlowListEntryStruct.set(
						VtnServiceIpcConsts.IP_PROTO,
						IpcDataUnitWrapper.setIpcUint8Value(flowListEntry.get(VtnServiceJsonConsts.IPPROTO).getAsString(), 
								valFlowListEntryStruct, UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_IP_PROTOCOL_FLE.ordinal()));
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
										.setIpcUint8Value(flowListEntry.get(VtnServiceJsonConsts.IPDSCP).getAsString(), 
												valFlowListEntryStruct, UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_IP_DSCP_FLE.ordinal()));
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
				valFlowListEntryStruct.set(
						VtnServiceIpcConsts.L4_DST_PORT,
						IpcDataUnitWrapper.setIpcUint16Value(flowListEntry.get(VtnServiceJsonConsts.L4DSTPORT).getAsString(),
								valFlowListEntryStruct, UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_L4_DST_PORT_FLE.ordinal()));
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
				valFlowListEntryStruct.set(
						VtnServiceIpcConsts.L4_DST_PORT_ENDPT,
						IpcDataUnitWrapper.setIpcUint16Value(flowListEntry.get(VtnServiceJsonConsts.L4DSTENDPORT).getAsString(),
								valFlowListEntryStruct, UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_L4_DST_PORT_ENDPT_FLE.ordinal()));
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
				valFlowListEntryStruct.set(
						VtnServiceIpcConsts.L4_SRC_PORT,
						IpcDataUnitWrapper.setIpcUint16Value(flowListEntry.get(VtnServiceJsonConsts.L4SRCPORT).getAsString(),
								valFlowListEntryStruct, UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_L4_SRC_PORT_FLE.ordinal()));
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
				valFlowListEntryStruct.set(
						VtnServiceIpcConsts.L4_SRC_PORT_ENDPT,
						IpcDataUnitWrapper.setIpcUint16Value(flowListEntry.get(VtnServiceJsonConsts.L4SRCENDPORT).getAsString(),
								valFlowListEntryStruct, UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_L4_SRC_PORT_ENDPT_FLE.ordinal()));
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
				valFlowListEntryStruct.set(
						VtnServiceIpcConsts.ICMP_TYPE,
						IpcDataUnitWrapper.setIpcUint8Value(flowListEntry.get(VtnServiceJsonConsts.ICMPTYPENUM).getAsString(), 
								valFlowListEntryStruct, UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_ICMP_TYPE_FLE.ordinal()));
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
				valFlowListEntryStruct.set(
						VtnServiceIpcConsts.ICMP_CODE,
						IpcDataUnitWrapper.setIpcUint8Value(flowListEntry.get(VtnServiceJsonConsts.ICMPCODENUM).getAsString(), 
								valFlowListEntryStruct, UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_ICMP_CODE_FLE.ordinal()));
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
				valFlowListEntryStruct.set(
						VtnServiceIpcConsts.ICMPV6_TYPE,
						IpcDataUnitWrapper.setIpcUint8Value(flowListEntry.get(VtnServiceJsonConsts.IPV6ICMPTYPENUM).getAsString(), 
								valFlowListEntryStruct, UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_ICMP_V6_TYPE_FLE.ordinal()));
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
				valFlowListEntryStruct.set(
						VtnServiceIpcConsts.ICMPV6_CODE,
						IpcDataUnitWrapper.setIpcUint8Value(flowListEntry.get(VtnServiceJsonConsts.IPV6ICMPCODENUM).getAsString(), 
								valFlowListEntryStruct, UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_ICMP_V6_CODE_FLE.ordinal()));
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
	public IpcStruct getKeyVtunnelIfStruct(final JsonObject requestBody,
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
	public IpcStruct getValVtunnelIfStruct(final JsonObject requestBody,
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
											.setIpcUint8Value(UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_ENABLE.getValue(),
													ValVTunnelIfStruct, UncStructIndexEnum.ValVtunnelIfIndex.UPLL_IDX_ADMIN_ST_VTNL_IF.ordinal()));
				} else {
					ValVTunnelIfStruct
							.set(VtnServiceJsonConsts.ADMIN_STATUS,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_DISABLE.getValue(),
													ValVTunnelIfStruct, UncStructIndexEnum.ValVtunnelIfIndex.UPLL_IDX_ADMIN_ST_VTNL_IF.ordinal()));
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
	public IpcStruct getKeyVlinkStruct(final JsonObject requestBody,
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
	public IpcStruct getValVlinkStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		LOG.trace("Start getValVlinkStruct");
		final IpcStruct ValVlinkStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValVlink.getValue());
		if (requestBody != null && requestBody.has(VtnServiceJsonConsts.VLINK)) {
			JsonObject vLink = requestBody.getAsJsonObject(VtnServiceJsonConsts.VLINK);
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
											.setIpcUint8Value(UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_ENABLE.getValue(),
													ValVlinkStruct,UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_ADMIN_STATUS_VLNK.ordinal()));
				} else {
					ValVlinkStruct
							.set(VtnServiceJsonConsts.ADMIN_STATUS,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_DISABLE.getValue(),
													ValVlinkStruct,UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_ADMIN_STATUS_VLNK.ordinal()));
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
												vLink.get(VtnServiceJsonConsts.VNODE1NAME)
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
												vLink.get(VtnServiceJsonConsts.IF1NAME)
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
												vLink.get(VtnServiceJsonConsts.VNODE2NAME)
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
												vLink.get(VtnServiceJsonConsts.IF2NAME)
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
			if (vLink.has(VtnServiceJsonConsts.BOUNDARYMAP)){
					JsonObject boundaryMap = vLink.getAsJsonObject(VtnServiceJsonConsts.BOUNDARYMAP);
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
					LOG.debug("Valid VLAN ID Case");
					ValVlinkStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_VLAN_ID_VLNK
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
													.ordinal()));
					ValVlinkStruct.set(VtnServiceJsonConsts.VLANID,
							IpcDataUnitWrapper.setIpcUint16Value(boundaryMap.get(VtnServiceJsonConsts.VLANID).getAsString(),
									ValVlinkStruct, UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_VLAN_ID_VLNK.ordinal()));
				} else if (boundaryMap.has(VtnServiceJsonConsts.NO_VLAN_ID)) {
					LOG.debug("Valid NO VLAN ID Case");
					ValVlinkStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_VLAN_ID_VLNK
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
													.ordinal()));
					ValVlinkStruct
							.set(VtnServiceJsonConsts.VLANID,
									IpcDataUnitWrapper
											.setIpcUint16HexaValue(VtnServiceIpcConsts.VLAN_ID_DEFAULT_VALUE,
													ValVlinkStruct, UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_VLAN_ID_VLNK.ordinal()));
				} else {
					LOG.debug("InValid VLAN ID Case");
					ValVlinkStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_VLAN_ID_VLNK
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
													.ordinal()));
				}
				LOG.debug("VLAN ID Valid Bit : "
						+ ValVlinkStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_VLAN_ID_VLNK
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
	public IpcStruct getKeyVtepStruct(final JsonObject requestBody,
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
	public IpcStruct getValVtepStruct(final JsonObject requestBody,
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
			JsonObject vtep = requestBody.getAsJsonObject(VtnServiceJsonConsts.VTEP);
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
												vtep.get(VtnServiceJsonConsts.DESCRIPTION)
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
												vtep.get(VtnServiceJsonConsts.CONTROLLERID)
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
												vtep.get(VtnServiceJsonConsts.DOMAINID)
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
	public IpcStruct getKeyVtepIfStruct(final JsonObject requestBody,
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
	public IpcStruct getValVtepIfStruct(final JsonObject requestBody,
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
											.setIpcUint8Value(UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_ENABLE.getValue(),
													ValVtepIfStruct,UncStructIndexEnum.ValVtepIfIndex.UPLL_IDX_ADMIN_ST_VTEPI.ordinal()));
				} else {
					ValVtepIfStruct
							.set(VtnServiceJsonConsts.ADMIN_STATUS,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_DISABLE.getValue(),
													ValVtepIfStruct,UncStructIndexEnum.ValVtepIfIndex.UPLL_IDX_ADMIN_ST_VTEPI.ordinal()));
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
	public IpcStruct getKeyVbrIfFlowFilterEntryStruct(
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
	 * Gets the key vrt if flow filter struct.
	 * 
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the key vrt if flow filter struct
	 */
	public IpcStruct getKeyVrtIfFlowFilterStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
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
	public IpcStruct getKeyVrtIfFlowFilterEntryStruct(
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

	public IpcStruct getKeyVbrIfFlowFilterStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
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
	public IpcStruct getKeyVbrStruct(final JsonObject requestBody,
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
	public IpcStruct getValVbrStruct(final JsonObject requestBody,
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
			JsonObject vBridge = requestBody.getAsJsonObject(VtnServiceJsonConsts.VBRIDGE);
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
												vBridge.get(VtnServiceJsonConsts.CONTROLLERID)
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
												vBridge.get(VtnServiceJsonConsts.DOMAINID)
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
												vBridge.get(VtnServiceJsonConsts.DESCRIPTION)
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
			JsonObject ipAddress = requestBody.getAsJsonObject(VtnServiceJsonConsts.IPADDRESS);
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
				valVbrStruct.set(VtnServiceIpcConsts.HOST_ADDR,
						IpcDataUnitWrapper
								.setIpcInet4AddressValue(ipAddress.get(VtnServiceJsonConsts.IPADDR).getAsString(),
										valVbrStruct, UncStructIndexEnum.ValVbrIndex.UPLL_IDX_HOST_ADDR_VBR.ordinal()));
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
				valVbrStruct.set(VtnServiceIpcConsts.HOST_ADDR_PREFIXLEN,
						IpcDataUnitWrapper
								.setIpcUint8Value(ipAddress.get(VtnServiceJsonConsts.PREFIX).getAsString(),
										valVbrStruct, UncStructIndexEnum.ValVbrIndex.UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR.ordinal()));
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
	public IpcStruct getKeyVlanMapStruct(final JsonObject requestBody,
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
			String VlanMapId[] = uriParameters.get(2).split(
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
	public IpcStruct getValVlanMapStruct(final JsonObject requestBody,
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
			valVlanMapStruct.set(
					VtnServiceJsonConsts.VLANID,
					IpcDataUnitWrapper.setIpcUint16Value(requestBody
							.getAsJsonObject(VtnServiceJsonConsts.VLANMAP)
							.get(VtnServiceJsonConsts.VLANID).getAsString(),
							valVlanMapStruct, UncStructIndexEnum.ValVlanMapIndex.UPLL_IDX_VLAN_ID_VM.ordinal()));
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
									.setIpcUint16HexaValue(VtnServiceIpcConsts.VLAN_ID_DEFAULT_VALUE,
											valVlanMapStruct, UncStructIndexEnum.ValVlanMapIndex.UPLL_IDX_VLAN_ID_VM.ordinal()));
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
	public IpcStruct getKeyVtnFlowFilterStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
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
	public IpcStruct getKeyVtnFlowFilterEntryStruct(
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
	public IpcStruct getKeyVunknownStruct(final JsonObject requestBody,
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
			keyVunknownStruct
					.set(VtnServiceIpcConsts.VUNKNOWNNAME,
							IpcDataUnitWrapper
									.setIpcUint8ArrayValue(((JsonObject) requestBody
											.get(VtnServiceJsonConsts.VBYPASS))
											.get(VtnServiceJsonConsts.VBYPASS_NAME)
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
	public IpcStruct getKeyVunkIfStruct(final JsonObject requestBody,
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
	public IpcStruct getKeyVrtIfStruct(final JsonObject requestBody,
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
	public IpcStruct getKeyVtunnelStruct(final JsonObject requestBody,
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
	public IpcStruct getKeyStaticIpRouteStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
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
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.STATIC_IPROUTE)) {
			JsonObject staticIpRoute = requestBody
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
				
			} else if (staticIpRoute.has(VtnServiceJsonConsts.NEXTHOPADDR)){
				keyStaticIpRouteStruct.set(VtnServiceIpcConsts.NEXT_HOP_ADDR,
						IpcDataUnitWrapper
								.setIpcInet4AddressValue(staticIpRoute.get(
										VtnServiceJsonConsts.NEXTHOPADDR)
										.getAsString()));
			}
		} else if (uriParameters != null
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
	public IpcStruct getValVtnFlowFilterEntryStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		// Lower level structure
		/*
		 * ipc_struct ValVtnFlowFilterEntry{ UINT8 valid[5]; UINT8
		 * cs_row_status; UINT8 cs_attr[5]; UINT8 flowlist_name[32+1]; UINT8
		 * action; UINT8 nwm_name[31+1]; UINT8 dscp; UINT8 priority;
		 * 
		 * enum ValVtnFlowFilterEntryIndex { kIdxFlowListName=0, kIdxAction,
		 * kIdxnwn_name, kIdxdscp, kIdxPriority };
		 * 
		 * };
		 */
		LOG.trace("Start getValVtnFlowFilterEntryStruct");
		final IpcStruct valVtnFlowFilterEntryStruct = new IpcStruct(
				UncStructEnum.ValVtnFlowFilterEntry.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.FLOWFILTERENTRY)) {
			JsonObject flowFilterEntry = requestBody.getAsJsonObject(
					VtnServiceJsonConsts.FLOWFILTERENTRY);
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
												flowFilterEntry.get(VtnServiceJsonConsts.FLNAME)
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
				if (flowFilterEntry.get(VtnServiceJsonConsts.ACTIONTYPE).getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.PASS)) {
					valVtnFlowFilterEntryStruct
							.set(VtnServiceJsonConsts.ACTION,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.FlowfilterAction.UPLL_FLOWFILTER_ACT_PASS
													.ordinal()));
				} else if (flowFilterEntry.get(VtnServiceJsonConsts.ACTIONTYPE).getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.DROP)) {
					valVtnFlowFilterEntryStruct
							.set(VtnServiceJsonConsts.ACTION,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.FlowfilterAction.UPLL_FLOWFILTER_ACT_DROP
													.ordinal()));
				} else if (flowFilterEntry.get(VtnServiceJsonConsts.ACTIONTYPE).getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.REDIRECT)) {
					valVtnFlowFilterEntryStruct
							.set(VtnServiceJsonConsts.ACTION,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.FlowfilterAction.UPLL_FLOWFILTER_ACT_REDIRECT
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
												flowFilterEntry.get(VtnServiceJsonConsts.NMGNAME)
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
				valVtnFlowFilterEntryStruct.set(
						VtnServiceJsonConsts.DSCP,
						IpcDataUnitWrapper.setIpcUint8Value(flowFilterEntry.get(VtnServiceJsonConsts.DSCP).getAsString(),
								valVtnFlowFilterEntryStruct,UncStructIndexEnum.ValVtnFlowfilterEntryIndex.UPLL_IDX_DSCP_VFFE.ordinal()));
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
				valVtnFlowFilterEntryStruct.set(
						VtnServiceJsonConsts.PRIORITY,
						IpcDataUnitWrapper.setIpcUint8Value(flowFilterEntry.get(VtnServiceJsonConsts.PRIORITY).getAsString(),
								valVtnFlowFilterEntryStruct,UncStructIndexEnum.ValVtnFlowfilterEntryIndex.UPLL_IDX_PRIORITY_VFFE.ordinal()));
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
		LOG.info("Value Structure: " + valVtnFlowFilterEntryStruct.toString());
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
	public IpcStruct getValVunknownStruct(final JsonObject requestBody,
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
			JsonObject vByPass = requestBody.getAsJsonObject(VtnServiceJsonConsts.VBYPASS);
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
												vByPass.get(VtnServiceJsonConsts.DESCRIPTION)
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
												vByPass.get(VtnServiceJsonConsts.DOMAINID)
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
												vByPass.get(VtnServiceJsonConsts.CONTROLLERID)
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
	public IpcStruct getValVunkIfStruct(final JsonObject requestBody,
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
			JsonObject vunkIf = requestBody.getAsJsonObject(VtnServiceJsonConsts.INTERFACE);
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
												vunkIf.get(VtnServiceJsonConsts.DESCRIPTION)
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
											.setIpcUint8Value(UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_ENABLE.getValue(),
													valVunkIfStruct, UncStructIndexEnum.ValVunkIfIndex.UPLL_IDX_ADMIN_ST_VUNI.ordinal()));
				} else {
					valVunkIfStruct
							.set(VtnServiceJsonConsts.ADMIN_STATUS,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_DISABLE.getValue(),
													valVunkIfStruct, UncStructIndexEnum.ValVunkIfIndex.UPLL_IDX_ADMIN_ST_VUNI.ordinal()));
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
	public IpcStruct getKeyVbrFlowFilterStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
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
	public IpcStruct getKeyVbrFlowFilterEntryStruct(
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
	public IpcStruct getValFlowfilterEntryStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		/*
		 * ipc_struct val_flowfilter_entry { UINT8 valid[9]; UINT8
		 * cs_row_status; UINT8 cs_attr[9]; UINT8 flowlist_name[32+1]; UINT8
		 * action; UINT8 redirect_node[31+1]; UINT8 redirect_port[31+1]; UINT8
		 * modify_dstmac[6]; UINT8 modify_srcmac[6]; UINT8 nwm_name[31+1]; UINT8
		 * dscp; UINT8 priority; };
		 */
		LOG.trace("Start getValFlowfilterEntryStruct");
		final IpcStruct valFlowfilterEntryStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValFlowfilterEntry.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.FLOWFILTERENTRY)) {
			JsonObject flowFilterEntry = requestBody.getAsJsonObject(
					VtnServiceJsonConsts.FLOWFILTERENTRY);
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
												flowFilterEntry.get(VtnServiceJsonConsts.FLNAME)
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
				if (flowFilterEntry.get(VtnServiceJsonConsts.ACTIONTYPE).getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.PASS)) {
					valFlowfilterEntryStruct
							.set(VtnServiceIpcConsts.ACTION,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.FlowfilterAction.UPLL_FLOWFILTER_ACT_PASS
													.ordinal()));
				} else if (flowFilterEntry.get(VtnServiceJsonConsts.ACTIONTYPE).getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.DROP)) {
					valFlowfilterEntryStruct
							.set(VtnServiceIpcConsts.ACTION,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.FlowfilterAction.UPLL_FLOWFILTER_ACT_DROP
													.ordinal()));
				} else {
					valFlowfilterEntryStruct
							.set(VtnServiceIpcConsts.ACTION,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.FlowfilterAction.UPLL_FLOWFILTER_ACT_REDIRECT
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
				JsonObject redirectDst = flowFilterEntry
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
				if (redirectDst.has(VtnServiceJsonConsts.MACDSTADDR)) {
					valFlowfilterEntryStruct
							.set(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_MODIFY_DST_MAC_FFE
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
													.ordinal()));
					// valFlowfilterEntryStruct.set(VtnServiceIpcConsts.MODIFYDSTMACADDR,IpcDataUnitWrapper.setIpcUint8ArrayValue(requestBody.getAsJsonObject(VtnServiceJsonConsts.FLOWFILTERENTRY).getAsJsonObject(VtnServiceJsonConsts.REDIRECTDST).get(VtnServiceJsonConsts.MACDSTADDR).getAsString()));
					IpcDataUnitWrapper.setMacAddress(valFlowfilterEntryStruct,
							VtnServiceIpcConsts.MODIFYDSTMACADDR, redirectDst
									.get(VtnServiceJsonConsts.MACDSTADDR).getAsString(),
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_MODIFY_DST_MAC_FFE.ordinal());
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
					IpcDataUnitWrapper.setMacAddress(valFlowfilterEntryStruct,
							VtnServiceIpcConsts.MODIFYSRCMACADDR, redirectDst
									.get(VtnServiceJsonConsts.MACSRCADDR).getAsString(),
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_MODIFY_SRC_MAC_FFE.ordinal());
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
												flowFilterEntry.get(VtnServiceJsonConsts.NMGNAME)
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
				valFlowfilterEntryStruct.set(
						VtnServiceIpcConsts.DSCP,
						IpcDataUnitWrapper.setIpcUint8Value(flowFilterEntry.get(VtnServiceJsonConsts.DSCP).getAsString(),
								valFlowfilterEntryStruct, UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_DSCP_FFE.ordinal()));
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
				valFlowfilterEntryStruct.set(
						VtnServiceIpcConsts.PRIORITY,
						IpcDataUnitWrapper.setIpcUint8Value(flowFilterEntry.get(VtnServiceJsonConsts.PRIORITY).getAsString(),
								valFlowfilterEntryStruct, UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_PRIORITY_FFE.ordinal()));
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
	public IpcStruct getKeyVbrIfStruct(final JsonObject requestBody,
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
	public IpcStruct getValPortMapStruct(final JsonObject requestBody,
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
			JsonObject portMap = requestBody.getAsJsonObject(VtnServiceJsonConsts.PORTMAP);
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
												portMap.get(VtnServiceJsonConsts.LOGICAL_PORT_ID)
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
				valPortMapStruct.set(VtnServiceJsonConsts.VLANID,
						IpcDataUnitWrapper
								.setIpcUint16Value(portMap.get(VtnServiceJsonConsts.VLANID).getAsString(),
										valPortMapStruct, UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_VLAN_ID_PM.ordinal()));
			} 
			else {
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
											.setIpcUint8Value(UncStructIndexEnum.vlan_tagged.UPLL_VLAN_TAGGED.getValue(),
													valPortMapStruct, UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_TAGGED_PM.ordinal()));
				} else if (portMap.get(VtnServiceJsonConsts.TAGGED).getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.FALSE)){
					valPortMapStruct
							.set(VtnServiceJsonConsts.TAGGED,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.vlan_tagged.UPLL_VLAN_UNTAGGED.getValue(),
													valPortMapStruct, UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_TAGGED_PM.ordinal()));
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
						+ portMap.get(VtnServiceJsonConsts.TAGGED).getAsString());
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
	public IpcStruct getValVbrIfStruct(final JsonObject requestBody,
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
			JsonObject vbrIf = requestBody.getAsJsonObject(VtnServiceJsonConsts.INTERFACE);
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
											.setIpcUint8Value(UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_ENABLE.getValue(), 
													valVbrIfStruct, UncStructIndexEnum.ValVbrIfIndex.UPLL_IDX_ADMIN_STATUS_VBRI.ordinal()));
				} else {
					valVbrIfStruct
							.set(VtnServiceJsonConsts.ADMIN_STATUS,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_DISABLE.getValue(), 
													valVbrIfStruct, UncStructIndexEnum.ValVbrIfIndex.UPLL_IDX_ADMIN_STATUS_VBRI.ordinal()));
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
												vbrIf.get(VtnServiceJsonConsts.DESCRIPTION)
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
	public IpcStruct getValVrtStruct(final JsonObject requestBody,
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
			JsonObject vRouter = requestBody.getAsJsonObject(VtnServiceJsonConsts.VROUTER);
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
												vRouter.get(VtnServiceJsonConsts.CONTROLLERID)
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
												vRouter.get(VtnServiceJsonConsts.DOMAINID)
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
												vRouter.get(VtnServiceJsonConsts.DESCRIPTION)
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
											.setIpcUint8Value(UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_ENABLE.getValue(),
													valVrtStruct, UncStructIndexEnum.ValVrtIndex.UPLL_IDX_DHCP_RELAY_ADMIN_STATUS_VRT.ordinal()));
				} else {
					valVrtStruct
							.set(VtnServiceIpcConsts.DHCPRELAYADMINSTATUS,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_DISABLE.getValue(),
													valVrtStruct, UncStructIndexEnum.ValVrtIndex.UPLL_IDX_DHCP_RELAY_ADMIN_STATUS_VRT.ordinal()));
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
	public IpcStruct getValVrtIfStruct(final JsonObject requestBody,
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
			JsonObject vRouterIf = requestBody.getAsJsonObject(VtnServiceJsonConsts.INTERFACE);
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
												vRouterIf.get(VtnServiceJsonConsts.DESCRIPTION)
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
				valVrtIfStruct.set(VtnServiceIpcConsts.IP_ADDR,
						IpcDataUnitWrapper
								.setIpcInet4AddressValue(vRouterIf.get(VtnServiceJsonConsts.IPADDR).getAsString(),
										valVrtIfStruct, UncStructIndexEnum.ValVrtIfIndex.UPLL_IDX_IP_ADDR_VI.ordinal()));
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
				valVrtIfStruct.set(VtnServiceIpcConsts.PREFIXLEN,
						IpcDataUnitWrapper
								.setIpcUint8Value(vRouterIf.get(VtnServiceJsonConsts.PREFIX).getAsString(),
										valVrtIfStruct, UncStructIndexEnum.ValVrtIfIndex.UPLL_IDX_PREFIXLEN_VI.ordinal()));
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
			
				IpcDataUnitWrapper
						.setMacAddress(
								valVrtIfStruct,
								VtnServiceIpcConsts.MACADDR,
								vRouterIf.get(VtnServiceJsonConsts.MACADDR).getAsString(),
								UncStructIndexEnum.ValVrtIfIndex.UPLL_IDX_MAC_ADDR_VI.ordinal());
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
				if (vRouterIf.get(VtnServiceJsonConsts.ADMINSTATUS).getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.DISABLE)) {
					valVrtIfStruct
							.set(VtnServiceJsonConsts.ADMIN_STATUS,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_DISABLE.getValue(),
													valVrtIfStruct, UncStructIndexEnum.ValVrtIfIndex.UPLL_IDX_ADMIN_ST_VI.ordinal()));
				} else {
					valVrtIfStruct
							.set(VtnServiceJsonConsts.ADMIN_STATUS,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_ENABLE.getValue(),
													valVrtIfStruct, UncStructIndexEnum.ValVrtIfIndex.UPLL_IDX_ADMIN_ST_VI.ordinal()));
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
	public IpcStruct getValVtunnelStruct(final JsonObject requestBody,
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
			JsonObject vTunnel = requestBody.getAsJsonObject(VtnServiceJsonConsts.VTUNNEL);
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
												vTunnel.get(VtnServiceJsonConsts.DESCRIPTION)
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
												vTunnel.get(VtnServiceJsonConsts.CONTROLLERID)
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
												vTunnel.get(VtnServiceJsonConsts.DOMAINID)
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
												vTunnel.get(VtnServiceJsonConsts.VTNNAME)
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
												vTunnel.get(VtnServiceJsonConsts.VTEPGROUPNAME)
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
						.set(VtnServiceIpcConsts.LABEL, IpcDataUnitWrapper
								.setIpcUint32Value(vTunnel.get(VtnServiceJsonConsts.LABEL).getAsString(),
										valVtunnelStruct, UncStructIndexEnum.ValVtunnelIndex.UPLL_IDX_LABEL_VTNL.ordinal()));
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
	public IpcStruct getKeyVtnFlowfilterControllerStruct(
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
	public IpcStruct getValFlowFilterControllerStruct(
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
				valFlowFilterContollerStruct.set(
						VtnServiceIpcConsts.SEQUENCENUM, IpcDataUnitWrapper
								.setIpcUint16Value(uriParameters.get(2),
								valFlowFilterContollerStruct, UncStructIndexEnum.ValFlowfilterControllerIndex.UPLL_IDX_SEQ_NUM_FFC.ordinal()));
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
	public IpcStruct getKeyVtepGrpStruct(final JsonObject requestBody,
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
	public IpcStruct getKeyVtepGrpMemberStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
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
	public IpcStruct getValVtepGrpStruct(final JsonObject requestBody,
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
			JsonObject vTepGroup = requestBody
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
	public IpcStruct getValStaticIpRouteStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
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
				valStaticIpRouteStruct.set(
						VtnServiceIpcConsts.GROUP_METRIC,
						IpcDataUnitWrapper.setIpcUint16Value(requestBody
								.getAsJsonObject(VtnServiceJsonConsts.STATIC_IPROUTE)
								.get(VtnServiceJsonConsts.GROUPMETRIC).getAsString(),
								valStaticIpRouteStruct, UncStructIndexEnum.ValStaticIpRouteIndex.UPLL_IDX_GROUP_METRIC_SIR.ordinal()));
			} else {
				valStaticIpRouteStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValStaticIpRouteIndex.UPLL_IDX_GROUP_METRIC_SIR
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
	public IpcStruct getKeyVtnstationControllerStruct(
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
	public IpcStruct getValVtnstationControllerStStruct(
			final JsonObject requestBody, final List<String> uriParameters) {
		/*
		 * ipc_struct val_vtnstation_controller_st { UINT8 valid[15]; UINT64
		 * station_id; UINT64 created_time; UINT8 mac_addr[6]; UINT32
		 * ipv4_count; UINT32 ipv6_count; UINT8 switch_id[256]; UINT8
		 * port_name[32]; UINT16 vlan_id; UINT8 map_type; UINT8 map_status;
		 * UINT8 vtn_name[32]; UINT8 domain_id[32]; UINT8 vbr_name[32]; UINT8
		 * vbrif_name[32]; UINT8 vbrif_status; };
		 */
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
				IpcDataUnitWrapper.setMacAddress(valVtnstationControllerSt,
						VtnServiceIpcConsts.MAC_ADDR,
						requestBody.get(VtnServiceJsonConsts.MACADDR).getAsString(),
						UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_MAC_ADDR_VSCS.ordinal());
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
				valVtnstationControllerSt.set(VtnServiceIpcConsts.IPV4_COUNT,
						IpcDataUnitWrapper
								.setIpcUint32Value(VtnServiceJsonConsts.ONE,
										valVtnstationControllerSt, UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_IPV4_COUNT_VSCS.ordinal()));
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
				valVtnstationControllerSt.set(VtnServiceIpcConsts.IPV6_COUNT,
						IpcDataUnitWrapper
								.setIpcUint32Value(VtnServiceJsonConsts.ONE, 
										valVtnstationControllerSt, UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_IPV6_COUNT_VSCS.ordinal()));
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
			if (requestBody.has(VtnServiceJsonConsts.VLANID) || requestBody.has(VtnServiceJsonConsts.NO_VLAN_ID)) {
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VLAN_ID_VSCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				if (requestBody.has(VtnServiceJsonConsts.VLANID)) {
					valVtnstationControllerSt.set(
							VtnServiceIpcConsts.VLANID,
							IpcDataUnitWrapper.setIpcUint16Value(requestBody.get(
									VtnServiceJsonConsts.VLANID).getAsString(),
									valVtnstationControllerSt, UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VLAN_ID_VSCS.ordinal()));
				} else {
					valVtnstationControllerSt
							.set(VtnServiceIpcConsts.VLANID,
									IpcDataUnitWrapper
											.setIpcUint16HexaValue(VtnServiceIpcConsts.VLAN_ID_DEFAULT_VALUE,
													valVtnstationControllerSt, 
													UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VLAN_ID_VSCS.ordinal()));
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
			// for vbr_name parameter
			if (requestBody.has(VtnServiceJsonConsts.VBRNAME)) {
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VBR_NAME_VSCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VBRNAME,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												requestBody
														.get(VtnServiceJsonConsts.VBRNAME)
														.getAsString(),
												valVtnstationControllerSt,
												UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VBR_NAME_VSCS
														.ordinal()));
			} else {
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VBR_NAME_VSCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			// for if_name parameter
			if (requestBody.has(VtnServiceJsonConsts.IFNAME)) {
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VBR_IF_NAME_VSCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VBRIFNAME,
								IpcDataUnitWrapper
										.setIpcUint8ArrayValue(
												requestBody
														.get(VtnServiceJsonConsts.IFNAME)
														.getAsString(),
												valVtnstationControllerSt,
												UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VBR_IF_NAME_VSCS
														.ordinal()));
			} else {
				valVtnstationControllerSt
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VBR_IF_NAME_VSCS
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
												.ordinal()));
			}
			// for vbrif_status parameter
			valVtnstationControllerSt
					.set(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VBR_IF_STATUS_VSCS
									.ordinal(),
							IpcDataUnitWrapper
									.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_INVALID
											.ordinal()));

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
	public IpcStruct getKeyCtrDomainStruct(final JsonObject requestBody,
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
	public IpcStruct getKeySwitchStruct(final JsonObject requestBody,
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
	public IpcStruct getKeyLinkStruct(final JsonObject requestBody,
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
		if (requestBody != null && requestBody.has(VtnServiceJsonConsts.INDEX)) {
			String linkName[] = requestBody.get(VtnServiceJsonConsts.INDEX)
					.getAsString().split(VtnServiceJsonConsts.LINKSAPERATOR);
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
		} else if (uriParameters != null
				&& uriParameters.size() == UncIndexEnum.TWO.ordinal()) {
			String linkName[] = uriParameters.get(1).split(
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
	public IpcStruct getKeyCtrStruct(final JsonObject requestBody,
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
	public IpcStruct getValCtrStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		LOG.trace("Start getValCtrStruct");
		final IpcStruct valCtrStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValCtr.getValue());
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.CONTROLLER)) {
			JsonObject controller = requestBody.getAsJsonObject(VtnServiceJsonConsts.CONTROLLER);
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
				} else if (controller.get(VtnServiceJsonConsts.TYPE).getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.PFC)) {
					valCtrStruct
							.set(VtnServiceJsonConsts.TYPE,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncPhysicalStructIndexEnum.UpplTypeIndex.UNC_CT_PFC
													.ordinal()));
				} else if (controller.get(VtnServiceJsonConsts.TYPE).getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.VNP)) {
					valCtrStruct
							.set(VtnServiceJsonConsts.TYPE,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncPhysicalStructIndexEnum.UpplTypeIndex.UNC_CT_VNP
													.ordinal()));
				} else if (controller.get(VtnServiceJsonConsts.TYPE).getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.ODC)) {
					valCtrStruct
							.set(VtnServiceJsonConsts.TYPE,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncPhysicalStructIndexEnum.UpplTypeIndex.UNC_CT_ODC
													.ordinal()));
				}
				LOG.debug("type:"
						+ controller.get(VtnServiceJsonConsts.TYPE).getAsString());
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
												controller.get(VtnServiceJsonConsts.VERSION)
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
												controller.get(VtnServiceJsonConsts.DESCRIPTION)
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
										.setIpcInet4AddressValue(controller.get(VtnServiceJsonConsts.IPADDR).getAsString(),
												valCtrStruct, UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxIpAddress.ordinal()));
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
												controller.get(VtnServiceJsonConsts.USERNAME)
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
												controller.get(VtnServiceJsonConsts.PASSWORD)
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
				if (controller.get(VtnServiceJsonConsts.AUDITSTATUS).getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.DISABLE)) {
					valCtrStruct
							.set(VtnServiceIpcConsts.ENABLE_AUDIT,
									IpcDataUnitWrapper
											.setIpcUint8Value(UncPhysicalStructIndexEnum.UpplControllerAuditStatus.UPPL_AUTO_AUDIT_DISABLED
													.ordinal()));
				} else if (controller.get(VtnServiceJsonConsts.AUDITSTATUS).getAsString()
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
		} else {
			LOG.warning("request body and uri parameters are not correct for getValCtrStruct");
		}
		LOG.info("Value Structure: " + valCtrStruct.toString());
		LOG.trace("Complete getValCtrStruct");
		return valCtrStruct;
	}

	public IpcStruct getValCtrDomainStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
		/*
		 * ipc_struct val_ctr_domain { UINT8 type; UINT8 description[128]; UINT8
		 * valid[2]; UINT8 cs_row_status; UINT8 cs_attr[2]; };
		 */
		LOG.trace("Start getValCtrDomainStruct");
		final IpcStruct valCtrDomainStruct = IpcDataUnitWrapper
				.setIpcStructValue(UncStructEnum.ValCtrDomain.getValue());
		if (requestBody != null && requestBody.has(VtnServiceJsonConsts.DOMAIN)) {
			JsonObject domain = requestBody.getAsJsonObject(VtnServiceJsonConsts.DOMAIN);
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
												domain.get(VtnServiceJsonConsts.DESCRIPTION)
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

	public IpcStruct getKeyBoundaryStruct(final JsonObject requestBody,
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

	public IpcStruct getValBoundaryStruct(final JsonObject requestBody,
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
			JsonObject boundary = requestBody.getAsJsonObject(VtnServiceJsonConsts.BOUNDARY);
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
												boundary.get(VtnServiceJsonConsts.DESCRIPTION)
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
				JsonObject link = boundary.getAsJsonObject(VtnServiceJsonConsts.LINK);
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
													link.get(VtnServiceJsonConsts.CONTROLLER1ID)
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
													link.get(VtnServiceJsonConsts.DOMAIN1_ID)
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
													link.get(VtnServiceJsonConsts.LOGICAL_PORT1_ID)
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
													link.get(VtnServiceJsonConsts.CONTROLLER2ID)
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
													link.get(VtnServiceJsonConsts.DOMAIN2_ID)
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
													link.get(VtnServiceJsonConsts.LOGICAL_PORT2_ID)
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
		}
		else {
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
	public IpcStruct getKeyPortStruct(final JsonObject requestBody,
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

	public IpcStruct getValVrtArpEntryStStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
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

	public IpcStruct getKeyLogicalPortStruct(final JsonObject requestBody,
			final List<String> uriParameters) {
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

	public IpcStruct getKeyLogicalMemberPortStruct(
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
}
