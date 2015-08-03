/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi.ipc.conversion;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;

import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonPrimitive;
import org.opendaylight.vtn.core.ipc.IpcDataUnit;
import org.opendaylight.vtn.core.ipc.IpcStruct;
import org.opendaylight.vtn.core.ipc.IpcUint32;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.core.util.UnsignedInteger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceIpcConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.init.VtnServiceInitManager;
import org.opendaylight.vtn.javaapi.ipc.enums.PomStatsIndex;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncPhysicalStructIndexEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncStructIndexEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncUPLLEnums.ValLabelType;
import org.opendaylight.vtn.javaapi.ipc.enums.UncUPLLEnums.ValUnifiedNwRoutingType;

public class IpcLogicalResponseFactory {
	private static final Logger LOG = Logger
			.getLogger(IpcLogicalResponseFactory.class.getName());

	/**
	 * Used for Vtn response
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return JsonObject
	 */
	public JsonObject getVtnResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody, final String getType) {
		LOG.trace("Start getVtnResponse");
		final JsonObject root = new JsonObject();
		JsonArray vtnsArray = null;
		LOG.debug("getType: " + getType);
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}

		/*
		 * data type will be required to resolve the response structures
		 */
		String dataType = VtnServiceJsonConsts.STATE;
		if (requestBody.has(VtnServiceJsonConsts.TARGETDB)) {
			dataType = requestBody.get(VtnServiceJsonConsts.TARGETDB)
					.getAsString();
		}
		String rootJsonName;
		/*
		 * get type (show or list) will be required to resolve root json name
		 * here it will be vtn for show and vtns for list
		 */
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.VTN;

		} else {
			rootJsonName = VtnServiceJsonConsts.VTNS;
			// json array will be required for list type of cases
			vtnsArray = new JsonArray();

		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject vtn = null;
		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			vtn = new JsonObject();
			vtn.addProperty(
					VtnServiceJsonConsts.COUNT,
					IpcDataUnitWrapper
							.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, vtn);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {
				vtn = new JsonObject();
				byte validBit;
				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;
				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyVtnStruct = (IpcStruct) responsePacket[index++];
				vtn.addProperty(VtnServiceJsonConsts.VTNNAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								keyVtnStruct, VtnServiceIpcConsts.VTNNAME));
				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */
				if (getType.equals(VtnServiceJsonConsts.SHOW)
						|| opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
					LOG.debug("Case : Show or detail");
					/*
					 * add valid informations from value structure
					 */
					final IpcStruct valVtnStruct = (IpcStruct) responsePacket[index++];
					validBit = valVtnStruct.getByte(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVtnIndex.UPLL_IDX_DESC_VTN
									.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vtn,
								VtnServiceJsonConsts.DESCRIPTION,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVtnStruct,
										VtnServiceIpcConsts.DESCRIPTION));
					}
					/*
					 * If data type is set as "state", then value structure will
					 * also contain the state information
					 */
					if (dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
						LOG.debug("targetdb : State");
						final IpcStruct valVtnStStruct = (IpcStruct) responsePacket[index++];
						/*
						 * If response is required in detail format then use the
						 * State value structure
						 */
						validBit = valVtnStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.ValVtnStIndex.UPLL_IDX_OPER_STATUS_VS
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVtnStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_UP
											.getValue())) {
								setValueToJsonObject(validBit, vtn,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.UP);

							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVtnStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_DOWN
											.getValue())) {
								setValueToJsonObject(validBit, vtn,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.DOWN);

							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVtnStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_UNKNOWN
											.getValue())) {
								setValueToJsonObject(validBit, vtn,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.UNKNOWN);

							} else {
								LOG.debug("Operstatus invalid");
							}
							LOG.debug("Operstatus :"
									+ IpcDataUnitWrapper
											.getIpcStructUint8Value(
													valVtnStStruct,
													VtnServiceIpcConsts.OPERSTATUS));
						}
						validBit = valVtnStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.ValVtnStIndex.UPLL_IDX_ALARM_STATUS_VS
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							if (IpcDataUnitWrapper
									.getIpcStructUint8Value(
											valVtnStStruct,
											VtnServiceIpcConsts.VTN_ALARM_STATUS)
									.equals(UncStructIndexEnum.ValAlarmStatus.UPLL_ALARM_CLEAR
											.getValue())) {
								setValueToJsonObject(validBit, vtn,
										VtnServiceJsonConsts.ALARMSTATUS,
										VtnServiceJsonConsts.CLEAR);

							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(
											valVtnStStruct,
											VtnServiceIpcConsts.VTN_ALARM_STATUS)
									.equals(UncStructIndexEnum.ValAlarmStatus.UPLL_ALARM_RAISE
											.getValue())) {
								setValueToJsonObject(validBit, vtn,
										VtnServiceJsonConsts.ALARMSTATUS,
										VtnServiceJsonConsts.RAISE);

							} else {
								LOG.debug("Alarmstatus : invalid");
							}
							LOG.debug("Alarmstatus :"
									+ IpcDataUnitWrapper
											.getIpcStructUint8Value(
													valVtnStStruct,
													VtnServiceIpcConsts.VTN_ALARM_STATUS));
						}
						validBit = valVtnStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.ValVtnStIndex.UPLL_IDX_CREATEION_TIME_VS
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							setValueToJsonObject(validBit, vtn,
									VtnServiceJsonConsts.CREATEDTIME,
									IpcDataUnitWrapper.getIpcStructUint64Value(
											valVtnStStruct,
											VtnServiceIpcConsts.CREATEDTIME));
						}
						validBit = valVtnStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.ValVtnStIndex.UPLL_IDX_LAST_UPDATE_TIME_VS
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							setValueToJsonObject(validBit, vtn,
									VtnServiceJsonConsts.LASTCOMMITTEDTIME,
									IpcDataUnitWrapper.getIpcStructUint64Value(
											valVtnStStruct,
											VtnServiceIpcConsts.LASTUPDATETIME));
						}
					}
				} else {
					LOG.debug("Operation : normal Skip value struture");
					index++;
					if (dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
						LOG.debug("Operation : normal and target db :state Skip St value struture ");
						index++;
					}
				}
				// add current json object to array, if it has been initialized
				// earlier
				if (null != vtnsArray) {
					vtnsArray.add(vtn);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != vtnsArray) {
				root.add(rootJsonName, vtnsArray);
			} else {
				root.add(rootJsonName, vtn);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVtnResponse");

		return root;
	}

	/**
	 * Used for Show FlowFilter response
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return JsonObject
	 */
	public JsonObject getFlowFilterResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody, final String getType) {
		LOG.trace("Start getFlowFilterResponse");
		final JsonObject root = new JsonObject();
		final String rootJsonName = VtnServiceJsonConsts.FLOWFILTER;
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject flowFilter = null;
		for (int index = 0; index < responsePacket.length; index++) {
			flowFilter = new JsonObject();
			// There is no use of key type
			LOG.debug("Skip key type: no use");
			index++;
			/*
			 * add mandatory informations from key structure
			 */
			final IpcStruct keyVtnFlowFilter = (IpcStruct) responsePacket[index++];
			if (IpcDataUnitWrapper
					.getIpcStructUint8Value(keyVtnFlowFilter,
							VtnServiceIpcConsts.INPUTDIRECTION)
					.equals(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_IN
							.getValue())) {
				flowFilter.addProperty(VtnServiceJsonConsts.FFTYPE,
						VtnServiceJsonConsts.IN);
				LOG.debug("FF Type :"
						+ IpcDataUnitWrapper.getIpcStructUint8Value(
								keyVtnFlowFilter,
								VtnServiceIpcConsts.INPUTDIRECTION));
			} else if (IpcDataUnitWrapper
					.getIpcStructUint8Value(keyVtnFlowFilter,
							VtnServiceIpcConsts.INPUTDIRECTION)
					.equals(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_OUT
							.getValue())) {
				flowFilter.addProperty(VtnServiceJsonConsts.FFTYPE,
						VtnServiceJsonConsts.OUT);
				LOG.debug("FF Type :"
						+ IpcDataUnitWrapper.getIpcStructUint8Value(
								keyVtnFlowFilter,
								VtnServiceIpcConsts.INPUTDIRECTION));
			} else {
				LOG.debug("Invalid value for FFTYPE parameter");
			}
		}
		/*
		 * finally add single object to root json object and return the same.
		 */
		root.add(rootJsonName, flowFilter);
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getFlowFilterResponse");
		return root;
	}

	/**
	 * Used for Show HostAddress response
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return JsonObject
	 */
	public JsonObject getHostAddressResourceResponse(
			final IpcDataUnit[] responsePacket, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start getHostAddressResourceResponse");
		final JsonObject root = new JsonObject();

		final String rootJsonName = VtnServiceJsonConsts.IPADDRESS;
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject ipAddress = null;
		for (int index = 0; index < responsePacket.length; index++) {

			ipAddress = new JsonObject();
			byte validBit;

			// There is no use of key type
			LOG.debug("Skip key type: no use");
			index++;

			/*
			 * There is no use of key structure
			 */
			LOG.debug("Skip key Structure: no use");
			index++;
			/*
			 * add valid informations from value structure
			 */
			final IpcStruct valVbrStruct = (IpcStruct) responsePacket[index++];
			/*
			 * add valid ipaddress from value structure
			 */
			validBit = valVbrStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValVbrIndex.UPLL_IDX_HOST_ADDR_VBR
							.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				if (validBit == (byte) UncStructIndexEnum.Valid.UNC_VF_VALID
						.ordinal()) {
					setValueToJsonObject(validBit, ipAddress,
							VtnServiceJsonConsts.IPADDR,
							IpcDataUnitWrapper
									.getIpcStructIpv4Value(valVbrStruct,
											VtnServiceIpcConsts.HOST_ADDR));
				}
			}
			/*
			 * add valid netmask from value structure
			 */
			validBit = valVbrStruct
					.getByte(
							VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVbrIndex.UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR
									.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				if (validBit == (byte) UncStructIndexEnum.Valid.UNC_VF_VALID
						.ordinal()) {
					setValueToJsonObject(validBit, ipAddress,
							VtnServiceJsonConsts.PREFIX,
							IpcDataUnitWrapper.getIpcStructUint8Value(
									valVbrStruct,
									VtnServiceIpcConsts.HOST_ADDR_PREFIXLEN));
				}
			}

		}

		/*
		 * finally add single object to root json object and return the same.
		 */
		root.add(rootJsonName, ipAddress);
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getHostAddressResourceResponse");

		return root;
	}

	/**
	 * Used for Show L2Domain response
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return JsonObject
	 */
	public JsonObject getL2DomainResourceResponse(
			final IpcDataUnit[] responsePacket, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start getL2DomainResourceResponse");
		final JsonObject root = new JsonObject();

		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}

		final String rootJsonName = VtnServiceJsonConsts.L2DOMAINS;
		LOG.debug("Json Name :" + rootJsonName);
		final JsonArray l2DomainArray = new JsonArray();
		JsonObject l2Domain = null;

		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			l2Domain = new JsonObject();
			String count = VtnServiceConsts.ZERO;
			if (responsePacket.length >= 1) {
				count = IpcDataUnitWrapper
						.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]);
			}
			l2Domain.addProperty(VtnServiceJsonConsts.COUNT, count);
			root.add(rootJsonName, l2Domain);
		} else {
			// There is no use of key type, key structure and count of l2domains
			for (int index = 3; index < responsePacket.length;) {
				l2Domain = new JsonObject();
				byte validBit;

				/*
				 * add valid informations from value structure
				 */
				final IpcStruct valVbrL2DomainSt = (IpcStruct) responsePacket[index++];

				/*
				 * add valid l2Domain_id from value structure
				 */
				validBit = valVbrL2DomainSt
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.valVbrL2DomainStIndex.UPLL_IDX_L2_DOMAIN_ID_VL2DS
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					setValueToJsonObject(validBit, l2Domain,
							VtnServiceJsonConsts.L2DOMAINID,
							IpcDataUnitWrapper.getIpcStructUint64Value(
									valVbrL2DomainSt,
									VtnServiceIpcConsts.L2DOMAINID));
				}

				/*
				 * get valid ofs_count from value structure
				 */
				int memberCount;
				validBit = valVbrL2DomainSt
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.valVbrL2DomainStIndex.UPLL_IDX_OFS_COUNT_VL2DS
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					memberCount = Integer.parseInt(IpcDataUnitWrapper
							.getIpcStructUint32Value(valVbrL2DomainSt,
									VtnServiceIpcConsts.OFSCOUNT));
					LOG.debug("Count of L2 domain member :" + memberCount);
					final JsonArray l2DomainMemberArray = new JsonArray();
					for (int memIndex = 0; memIndex < memberCount; memIndex++) {

						final JsonObject l2DomainMember = new JsonObject();
						final IpcStruct valVbrL2DomainMemberSt = (IpcStruct) responsePacket[index++];

						validBit = valVbrL2DomainMemberSt
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.valVbrL2DomainMemberStIndex.UPLL_IDX_SWITCH_ID_VL2DMS
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							setValueToJsonObject(
									validBit,
									l2DomainMember,
									VtnServiceJsonConsts.SWITCHID,
									IpcDataUnitWrapper
											.getIpcStructUint8ArrayValue(
													valVbrL2DomainMemberSt,
													VtnServiceIpcConsts.SWITCHID));
						}
						validBit = valVbrL2DomainMemberSt
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.valVbrL2DomainMemberStIndex.UPLL_IDX_VLAN_ID_VL2DMS
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							setValueToJsonObject(validBit, l2DomainMember,
									VtnServiceJsonConsts.VLANID,
									IpcDataUnitWrapper.getIpcStructUint16Value(
											valVbrL2DomainMemberSt,
											VtnServiceJsonConsts.VLANID));
							LOG.debug("Vlan Id :"
									+ IpcDataUnitWrapper
											.getIpcStructUint16Value(
													valVbrL2DomainMemberSt,
													VtnServiceJsonConsts.VLANID));
						}

						l2DomainMemberArray.add(l2DomainMember);
					}
					l2Domain.add(VtnServiceJsonConsts.L2DOMAINMEMBERS,
							l2DomainMemberArray);
				}
				// add current json object to array, if it has been initialized
				// earlier
				if (null != l2DomainArray) {
					l2DomainArray.add(l2Domain);
				}
			}
			/*
			 * finally add array to root json object and return the same.
			 */
			root.add(rootJsonName, l2DomainArray);
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getL2DomainResourceResponse");
		return root;
	}

	/**
	 * Used for Show MacEntry response
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return JsonObject
	 */
	public JsonObject getMacEntryResourceResponse(
			final IpcDataUnit[] responsePacket, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start getMacEntryResourceResponse");
		final JsonObject root = new JsonObject();

		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}

		final String rootJsonName = VtnServiceJsonConsts.MACENTRIES;
		LOG.debug("Json Name :" + rootJsonName);
		final JsonArray macEntriesArray = new JsonArray();
		JsonObject macEntries = null;

		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			macEntries = new JsonObject();
			String count = VtnServiceConsts.ZERO;
			if (responsePacket.length >= 1) {
				count = IpcDataUnitWrapper
						.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]);
			}
			macEntries.addProperty(VtnServiceJsonConsts.COUNT, count);
			root.add(rootJsonName, macEntries);
		} else {
			// There is no use of key type and key structure
			// start reading from index 2
			LOG.debug("Start from index 2, because of no use of key type and key structure");
			for (int index = 2; index < responsePacket.length; index++) {
				macEntries = new JsonObject();
				byte validBit;

				/*
				 * add valid informations from value structure
				 */
				final IpcStruct valVbrMacEntrySt = (IpcStruct) responsePacket[index];
				/*
				 * add valid macaddr from value structure
				 */
				validBit = valVbrMacEntrySt
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.valVbrMacEntryStIndex.UPLL_IDX_MAC_ADDR_VMES
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					setValueToJsonObject(validBit, macEntries,
							VtnServiceJsonConsts.MACADDR,
							// IpcDataUnitWrapper.getIpcStructUint8ArrayValue(valVbrMacEntrySt,
							// VtnServiceIpcConsts.MACADDR));
							IpcDataUnitWrapper.getMacAddress(valVbrMacEntrySt,
									VtnServiceIpcConsts.MACADDR));
				}
				/*
				 * add valid type from value structure
				 */
				validBit = valVbrMacEntrySt
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.valVbrMacEntryStIndex.UPLL_IDX_TYPE_VMES
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					if (IpcDataUnitWrapper
							.getIpcStructUint8Value(valVbrMacEntrySt,
									VtnServiceIpcConsts.TYPE)
							.equals(UncStructIndexEnum.ValMacEntry.UPLL_MAC_ENTRY_STATIC
									.getValue())) {
						setValueToJsonObject(validBit, macEntries,
								VtnServiceJsonConsts.TYPE,
								VtnServiceJsonConsts.STATIC);
					} else if (IpcDataUnitWrapper
							.getIpcStructUint8Value(valVbrMacEntrySt,
									VtnServiceIpcConsts.TYPE)
							.equals(UncStructIndexEnum.ValMacEntry.UPLL_MAC_ENTRY_DYNAMIC
									.getValue())) {
						setValueToJsonObject(validBit, macEntries,
								VtnServiceJsonConsts.TYPE,
								VtnServiceJsonConsts.DYNAMIC);
					} else {
						LOG.debug("Type : invalid");
					}
					LOG.debug("Type :"
							+ IpcDataUnitWrapper.getIpcStructUint8Value(
									valVbrMacEntrySt, VtnServiceIpcConsts.TYPE));
				}
				/*
				 * add valid IF_name from value structure
				 */
				validBit = valVbrMacEntrySt
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.valVbrMacEntryStIndex.UPLL_IDX_IF_NAME_VMES
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					setValueToJsonObject(validBit, macEntries,
							VtnServiceJsonConsts.IFNAME,
							IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
									valVbrMacEntrySt,
									VtnServiceIpcConsts.IFNAME));
				}
				/*
				 * add valid if_kind from value structure
				 */
				validBit = valVbrMacEntrySt
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.valVbrMacEntryStIndex.UPLL_IDX_IF_KIND_VMES
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					if (IpcDataUnitWrapper
							.getIpcStructUint8Value(valVbrMacEntrySt,
									VtnServiceIpcConsts.IFKIND)
							.equals(UncStructIndexEnum.ValMacEntryIfKind.UPLL_MAC_ENTRY_BLANK
									.getValue())) {
						setValueToJsonObject(validBit, macEntries,
								VtnServiceJsonConsts.IFKIND,
								VtnServiceConsts.EMPTY_STRING);
					} else if (IpcDataUnitWrapper
							.getIpcStructUint8Value(valVbrMacEntrySt,
									VtnServiceIpcConsts.IFKIND)
							.equals(UncStructIndexEnum.ValMacEntryIfKind.UPLL_MAC_ENTRY_TRUNK
									.getValue())) {
						setValueToJsonObject(validBit, macEntries,
								VtnServiceJsonConsts.IFKIND,
								VtnServiceJsonConsts.TRUNK);
					} else {
						LOG.debug("If Kind invalid");
					}
					LOG.debug("If Kind :"
							+ IpcDataUnitWrapper.getIpcStructUint8Value(
									valVbrMacEntrySt,
									VtnServiceIpcConsts.IFKIND));
				}

				// add current json object to array, if it has been initialized
				// earlier
				macEntriesArray.add(macEntries);
			}
			/*
			 * finally add array to root json object and return the same.
			 */
			root.add(rootJsonName, macEntriesArray);
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getMacEntryResourceResponse");

		return root;
	}

	/**
	 * Used for List & Show VBridgeFlowFilterEntry response
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return JsonObject
	 */
	public JsonObject getVBridgeFlowFilterEntryResponse(
			final IpcDataUnit[] responsePacket, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start getVBridgeFlowFilterEntryResponse");
		final JsonObject root = new JsonObject();
		JsonArray flowFilterEntryArray = null;

		/*
		 * operation type will be required to resolve the response type
		 */
		LOG.debug("getType: " + getType);
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}
		/*
		 * data type will be required to resolve the response structures
		 */
		String dataType = VtnServiceJsonConsts.STATE;
		if (requestBody.has(VtnServiceJsonConsts.TARGETDB)) {
			dataType = requestBody.get(VtnServiceJsonConsts.TARGETDB)
					.getAsString();
		}
		String rootJsonName;
		/*
		 * get type (show or list) will be required to resolve root json name
		 * here it will be vtn for show and vtns for list
		 */
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.FLOWFILTERENTRY;
		} else {
			rootJsonName = VtnServiceJsonConsts.FLOWFILTERENTRIES;
			// json array will be required for list type of cases
			flowFilterEntryArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject flowFilterEntry = null;

		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			flowFilterEntry = new JsonObject();
			flowFilterEntry
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, flowFilterEntry);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {

				flowFilterEntry = new JsonObject();
				byte validBit;

				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;

				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyVbrFlowFilterEntryStruct = (IpcStruct) responsePacket[index++];
				flowFilterEntry.addProperty(VtnServiceJsonConsts.SEQNUM,
						IpcDataUnitWrapper.getIpcStructUint16Value(
								keyVbrFlowFilterEntryStruct,
								VtnServiceIpcConsts.SEQUENCENUM));

				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */

				if (getType.equals(VtnServiceJsonConsts.LIST)
						&& opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)
						|| getType.equals(VtnServiceJsonConsts.SHOW)) {
					LOG.debug("Case : Show or List with detail");
					/*
					 * add valid informations from value structure
					 */
					final IpcStruct valFlowFilterEntryStruct = (IpcStruct) responsePacket[index++];

					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_FLOWLIST_NAME_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, flowFilterEntry,
								VtnServiceJsonConsts.FLNAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valFlowFilterEntryStruct,
										VtnServiceJsonConsts.FLOWLISTNAME));
					}
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_ACTION_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						if (IpcDataUnitWrapper
								.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.ACTION)
								.equalsIgnoreCase(
										UncStructIndexEnum.FlowfilterAction.UPLL_FLOWFILTER_ACT_PASS
												.getValue())) {
							setValueToJsonObject(validBit, flowFilterEntry,
									VtnServiceJsonConsts.ACTIONTYPE,
									VtnServiceJsonConsts.PASS);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.ACTION)
								.equalsIgnoreCase(
										UncStructIndexEnum.FlowfilterAction.UPLL_FLOWFILTER_ACT_DROP
												.getValue())) {
							setValueToJsonObject(validBit, flowFilterEntry,
									VtnServiceJsonConsts.ACTIONTYPE,
									VtnServiceJsonConsts.DROP);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.ACTION)
								.equalsIgnoreCase(
										UncStructIndexEnum.FlowfilterAction.UPLL_FLOWFILTER_ACT_REDIRECT
												.getValue())) {
							setValueToJsonObject(validBit, flowFilterEntry,
									VtnServiceJsonConsts.ACTIONTYPE,
									VtnServiceJsonConsts.REDIRECT);
						} else {
							LOG.debug("Action type: Invalid");
						}
						LOG.debug("Action type :"
								+ IpcDataUnitWrapper.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.ACTION));
					}
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_NWM_NAME_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, flowFilterEntry,
								VtnServiceJsonConsts.NMGNAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.NWMNAME));
					}
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_PRIORITY_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, flowFilterEntry,
								VtnServiceJsonConsts.PRIORITY,
								IpcDataUnitWrapper.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceJsonConsts.PRIORITY));
					}
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_DSCP_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, flowFilterEntry,
								VtnServiceJsonConsts.DSCP,
								IpcDataUnitWrapper.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceJsonConsts.DSCP));
					}
					final JsonObject redirectDst = new JsonObject();
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_REDIRECT_NODE_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, redirectDst,
								VtnServiceJsonConsts.VNODENAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.REDIRECTNODE));
					}
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_REDIRECT_PORT_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, redirectDst,
								VtnServiceJsonConsts.IFNAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.REDIRECTPORT));
					}
					// Direction
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_REDIRECT_DIRECTION_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						if (IpcDataUnitWrapper
								.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.REDIRECTDIRECTION)
								.equals(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_IN
										.getValue())) {
							setValueToJsonObject(validBit, redirectDst,
									VtnServiceJsonConsts.DIRECTION,
									VtnServiceJsonConsts.IN);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.REDIRECTDIRECTION)
								.equals(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_OUT
										.getValue())) {
							setValueToJsonObject(validBit, redirectDst,
									VtnServiceJsonConsts.DIRECTION,
									VtnServiceJsonConsts.OUT);
						} else {
							LOG.debug("Direction : Invalid");
						}
						LOG.debug("Direction :"
								+ IpcDataUnitWrapper.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.REDIRECTDIRECTION));
					}
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_MODIFY_DST_MAC_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, redirectDst,
								VtnServiceJsonConsts.MACDSTADDR,
								IpcDataUnitWrapper.getMacAddress(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.MODIFYDSTMACADDR));
					}
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_MODIFY_SRC_MAC_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, redirectDst,
								VtnServiceJsonConsts.MACSRCADDR,
								IpcDataUnitWrapper.getMacAddress(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.MODIFYSRCMACADDR));
					}
					flowFilterEntry.add(VtnServiceJsonConsts.REDIRECTDST,
							redirectDst);
				}
				if (getType.equals(VtnServiceJsonConsts.SHOW)
						&& dataType
								.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
					LOG.debug("Case : Show and targetdb : State ");

					if (opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
						final IpcStruct valFlowFilterEntryStStruct = (IpcStruct) responsePacket[index++];
						LOG.debug("op : detail");
						validBit = valFlowFilterEntryStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.ValFlowfilterEntryStIndex.UPLL_IDX_NWM_STATUS_FFES
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							setValueToJsonObject(validBit, flowFilterEntry,
									VtnServiceJsonConsts.NMG_STATUS,
									IpcDataUnitWrapper.getIpcStructUint8Value(
											valFlowFilterEntryStStruct,
											VtnServiceIpcConsts.NWM_STATUS));
						}
						final PomStatsIndex pomStatsIndexSet = new PomStatsIndex();
						pomStatsIndexSet
								.setSoftware(UncStructIndexEnum.ValFlowfilterEntryStIndex.UPLL_IDX_SOFTWARE_FFES
										.ordinal());
						pomStatsIndexSet
								.setExistingFlow(UncStructIndexEnum.ValFlowfilterEntryStIndex.UPLL_IDX_EXIST_FFES
										.ordinal());
						pomStatsIndexSet
								.setExpiredFlow(UncStructIndexEnum.ValFlowfilterEntryStIndex.UPLL_IDX_EXPIRE_FFES
										.ordinal());
						pomStatsIndexSet
								.setTotal(UncStructIndexEnum.ValFlowfilterEntryStIndex.UPLL_IDX_TOTAL_FFES
										.ordinal());
						LOG.debug("call getPomStats : for statics information");
						getPomStats(flowFilterEntry,
								valFlowFilterEntryStStruct, pomStatsIndexSet);

						final String flowListName = VtnServiceJsonConsts.FLOWLIST;
						final JsonObject flowListJson = new JsonObject();
						final String flowListEntriesName = VtnServiceJsonConsts.FLOWLISTENTRIES;
						final JsonArray flowListEntriesJsonArray = new JsonArray();
						LOG.debug("call getPomStatsFlowList : for statics information of flowList");
						index = getPomStatsFlowList(responsePacket, index,
								flowListEntriesJsonArray);
						flowListJson.add(flowListEntriesName,
								flowListEntriesJsonArray);
						flowFilterEntry.add(flowListName, flowListJson);
					} else {
						LOG.debug("Show ,Operation : normal and target db :state Skip flowList value strutures ");
						// increasing index to eliminate flow list entry
						// structures in case of show and op : normal
						index = responsePacket.length - 1;
					}
				}
				if (getType.equals(VtnServiceJsonConsts.LIST)
						&& opType.equalsIgnoreCase(VtnServiceJsonConsts.NORMAL)) {
					LOG.debug("List ,Operation : normal Skip value strutures ");
					index++;
				}
				// add current json object to array, if it has been initialized
				// earlier
				if (null != flowFilterEntryArray) {
					flowFilterEntryArray.add(flowFilterEntry);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != flowFilterEntryArray) {
				root.add(rootJsonName, flowFilterEntryArray);
			} else {
				root.add(rootJsonName, flowFilterEntry);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVBridgeFlowFilterEntryResponse");
		return root;
	}
	
	/**
	 * Used for List and Show VBridgePortMap response
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return JsonObject
	 */
	public JsonObject getVBridgePortMapResponse(
			final IpcDataUnit[] responsePacket, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start getVBridgePortMapResponse");
		final JsonObject root = new JsonObject();
		JsonArray portmapArray = null;

		/*
		 * operation type will be required to resolve the response type
		 */
		LOG.debug("getType: " + getType);
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}
		
		/*
		 * get type (show or list) will be required to resolve root json name
		 * here it will be portmap for show and portmaps for list
		 */
		String rootJsonName;
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.PORTMAP;
		} else {
			rootJsonName = VtnServiceJsonConsts.PORTMAPS;
			// json array will be required for list type of cases
			portmapArray = new JsonArray();
		}
		
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject portmap = null;

		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			portmap = new JsonObject();
			portmap.addProperty(
					VtnServiceJsonConsts.COUNT,
					IpcDataUnitWrapper
							.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, portmap);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {

				portmap = new JsonObject();
				byte validBit;
				boolean isShowLabel = false;

				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;

				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyVbrPortmapStruct = (IpcStruct) responsePacket[index++];
				portmap.addProperty(VtnServiceJsonConsts.PORTMAP_NAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								keyVbrPortmapStruct,VtnServiceIpcConsts.PORTMAP_ID));

				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */

				if (getType.equals(VtnServiceJsonConsts.LIST)
						&& opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)
						|| getType.equals(VtnServiceJsonConsts.SHOW)) {
					LOG.debug("Case : Show or List with detail");
					/*
					 * add valid informations from value structure
					 */
					final IpcStruct valVbrPortMapStruct = (IpcStruct) responsePacket[index++];

					validBit = valVbrPortMapStruct.getByte(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVbrPortmapIndex.UPLL_IDX_CONTROLLER_ID_VBRPM
											.ordinal());
					
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, portmap,
								VtnServiceJsonConsts.CONTROLLERID,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVbrPortMapStruct,
										VtnServiceIpcConsts.CONTROLLERID));
					}
					
					validBit = valVbrPortMapStruct.getByte(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVbrPortmapIndex.UPLL_IDX_DOMAIN_ID_VBRPM
											.ordinal());
					
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
							setValueToJsonObject(validBit, portmap,
									VtnServiceJsonConsts.DOMAINID,
									IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
											valVbrPortMapStruct,
											VtnServiceIpcConsts.DOMAINID));
					}
					
					validBit = valVbrPortMapStruct.getByte(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVbrPortmapIndex.UPLL_IDX_LOGICAL_PORT_ID_VBRPM
											.ordinal());
					
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, portmap,
								VtnServiceJsonConsts.LOGICAL_PORT_ID,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVbrPortMapStruct,
										VtnServiceIpcConsts.LOGICAL_PORT_ID));
					}
					
					validBit = valVbrPortMapStruct.getByte(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVbrPortmapIndex.UPLL_IDX_LABEL_TYPE_VBRPM
											.ordinal());
							
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()
							&& ValLabelType.UPLL_LABEL_TYPE_VLAN.getValue().equals(
									IpcDataUnitWrapper.getIpcStructUint8Value(
											valVbrPortMapStruct, VtnServiceIpcConsts.LABEL_TYPE))) {
						
							setValueToJsonObject(validBit, portmap,
									VtnServiceJsonConsts.LABEL_TYPE,
									VtnServiceJsonConsts.VLAN_ID);
							
							isShowLabel = true;
					}
					
					if (isShowLabel) {
						validBit = valVbrPortMapStruct.getByte(VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.ValVbrPortmapIndex.UPLL_IDX_LABEL_VBRPM
												.ordinal());
						
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							
							String strLabel = IpcDataUnitWrapper.getIpcStructUint32Value(
									valVbrPortMapStruct,VtnServiceIpcConsts.LABEL);
							
							if (Integer.toString(VtnServiceJsonConsts.VAL_FFFE).equals(strLabel)) {
								setValueToJsonObject(validBit, portmap,
										VtnServiceJsonConsts.LABEL,
										VtnServiceJsonConsts.ANY_VLAN_ID);
							} else if (Integer.toString(VtnServiceJsonConsts.VAL_FFFF).equals(strLabel)) {
								setValueToJsonObject(validBit, portmap,
										VtnServiceJsonConsts.LABEL,
										VtnServiceJsonConsts.NO_VLAN_ID);
							} else {
								setValueToJsonObject(validBit, portmap,
										VtnServiceJsonConsts.LABEL,
										strLabel);
							}
						}
					}
				}
				
				if (getType.equals(VtnServiceJsonConsts.LIST)
						&& opType.equalsIgnoreCase(VtnServiceJsonConsts.NORMAL)) {
					LOG.debug("List ,Operation : normal Skip value strutures ");
					index++;
				}
				
				// add current json object to array, if it has been initialized
				// earlier
				if (null != portmapArray) {
					portmapArray.add(portmap);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != portmapArray) {
				root.add(rootJsonName, portmapArray);
			} else {
				root.add(rootJsonName, portmap);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVBridgePortMapResponse");
		return root;
	}
	
	/**
	 * Used for List and Show UnifiedNetwork response
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return JsonObject
	 */
	public JsonObject getUnifiedNetworkResponse(
			final IpcDataUnit[] responsePacket, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start getUnifiedNetworkResponse");
		final JsonObject root = new JsonObject();
		JsonArray unifiedNwArray = null;

		/*
		 * operation type will be required to resolve the response type
		 */
		LOG.debug("getType: " + getType);
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}
		
		/*
		 * get type (show or list) will be required to resolve root json name
		 * here it will be portmap for show and portmaps for list
		 */
		String rootJsonName;
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.UNIFIED_NW;
		} else {
			rootJsonName = VtnServiceJsonConsts.UNIFIED_NWS;
			// json array will be required for list type of cases
			unifiedNwArray = new JsonArray();
		}
		
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject unifiedNw = null;

		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			unifiedNw = new JsonObject();
			unifiedNw.addProperty(
					VtnServiceJsonConsts.COUNT,
					IpcDataUnitWrapper
							.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, unifiedNw);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {

				unifiedNw = new JsonObject();
				byte validBit;

				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;

				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyUnifiedNetworkStruct = (IpcStruct) responsePacket[index++];
				unifiedNw.addProperty(VtnServiceJsonConsts.UNIFIED_NW_NAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								keyUnifiedNetworkStruct,VtnServiceIpcConsts.UNIFIED_NW_ID));

				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */

				if (getType.equals(VtnServiceJsonConsts.LIST)
						&& opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)
						|| getType.equals(VtnServiceJsonConsts.SHOW)) {
					LOG.debug("Case : Show or List with detail");
					/*
					 * add valid informations from value structure
					 */
					final IpcStruct valUnifiedNetworkStruct = (IpcStruct) responsePacket[index++];

					validBit = valUnifiedNetworkStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValUnifiedNwIndex.UPLL_IDX_ROUTING_TYPE_UNW
											.ordinal());
					
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
						
						String routingType = IpcDataUnitWrapper.getIpcStructUint8Value(
								valUnifiedNetworkStruct, VtnServiceIpcConsts.ROUTING_TYPE);
						
						if (Integer.toString(ValUnifiedNwRoutingType.UPLL_ROUTING_TYPE_QINQ_TO_QINQ.ordinal())
								.equals(routingType)) {
							setValueToJsonObject(validBit, unifiedNw,
									VtnServiceJsonConsts.ROUTING_TYPE,
									routingType);
						}
					}
				}
				
				if (getType.equals(VtnServiceJsonConsts.LIST)
						&& opType.equalsIgnoreCase(VtnServiceJsonConsts.NORMAL)) {
					LOG.debug("List ,Operation : normal Skip value strutures ");
					index++;
				}
				
				// add current json object to array, if it has been initialized
				// earlier
				if (null != unifiedNwArray) {
					unifiedNwArray.add(unifiedNw);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != unifiedNwArray) {
				root.add(rootJsonName, unifiedNwArray);
			} else {
				root.add(rootJsonName, unifiedNw);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getUnifiedNetworkResponse");
		return root;
	}

	/**
	 * Gets the VRouter response.
	 * 
	 * @param responsePacket
	 *            the response packet
	 * @param requestBody
	 *            the request body
	 * @param getType
	 *            the get type
	 * @return the v router response
	 */
	public JsonObject getVRouterResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody, final String getType) {
		LOG.trace("Start getVRouterResponse");
		final JsonObject root = new JsonObject();
		JsonArray vRoutersArray = null;
		/*
		 * operation type will be required to resolve the response type
		 */
		LOG.debug("getType: " + getType);
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}
		/*
		 * data type will be required to resolve the response structures
		 */
		String dataType = VtnServiceJsonConsts.STATE;
		if (requestBody.has(VtnServiceJsonConsts.TARGETDB)) {
			dataType = requestBody.get(VtnServiceJsonConsts.TARGETDB)
					.getAsString();
		}
		String rootJsonName;
		/*
		 * get type (show or list) will be required to resolve root json name
		 * here it will be vRouter for show and vRouters for list
		 */
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.VROUTER;
		} else {
			rootJsonName = VtnServiceJsonConsts.VROUTERS;
			// json array will be required for list type of cases
			vRoutersArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject vRouter = null;
		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			vRouter = new JsonObject();
			vRouter.addProperty(
					VtnServiceJsonConsts.COUNT,
					IpcDataUnitWrapper
							.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, vRouter);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {

				vRouter = new JsonObject();
				byte validBit;
				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;
				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyVrtStruct = (IpcStruct) responsePacket[index++];
				vRouter.addProperty(VtnServiceJsonConsts.VRTNAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								keyVrtStruct, VtnServiceIpcConsts.VROUTERNAME));
				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */
				if (getType.equals(VtnServiceJsonConsts.SHOW)
						|| opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
					LOG.debug("Case : Show or detail");
					/*
					 * add valid informations from value structure
					 */
					final IpcStruct valVrtStruct = (IpcStruct) responsePacket[index++];
					validBit = valVrtStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVrtIndex.UPLL_IDX_CONTROLLER_ID_VRT
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vRouter,
								VtnServiceJsonConsts.CONTROLLERID,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVrtStruct,
										VtnServiceJsonConsts.CONTROLLERID));
					}
					validBit = valVrtStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVrtIndex.UPLL_IDX_DOMAIN_ID_VRT
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vRouter,
								VtnServiceJsonConsts.DOMAINID,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVrtStruct,
										VtnServiceJsonConsts.DOMAINID));
					}
					validBit = valVrtStruct.getByte(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVrtIndex.UPLL_IDX_DESC_VRT
									.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vRouter,
								VtnServiceJsonConsts.DESCRIPTION,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVrtStruct,
										VtnServiceIpcConsts.VRTDESCRIPTION));
					}
					/*
					 * If data type is set as "state", then value structure will
					 * also contain the state information
					 */
					if (dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
						LOG.debug("targetdb : State");
						final IpcStruct valVrtStStruct = (IpcStruct) responsePacket[index++];
						validBit = valVrtStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.ValVrtStIndex.UPLL_IDX_OPER_STATUS_VRTS
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVrtStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_UP
											.getValue())) {
								setValueToJsonObject(validBit, vRouter,
										VtnServiceJsonConsts.STATUS,
										VtnServiceJsonConsts.UP);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVrtStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_DOWN
											.getValue())) {
								setValueToJsonObject(validBit, vRouter,
										VtnServiceJsonConsts.STATUS,
										VtnServiceJsonConsts.DOWN);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVrtStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_UNKNOWN
											.getValue())) {
								setValueToJsonObject(validBit, vRouter,
										VtnServiceJsonConsts.STATUS,
										VtnServiceJsonConsts.UNKNOWN);
							} else {
								LOG.debug("Operstatus invalid");
							}
							LOG.debug("Operstatus :"
									+ IpcDataUnitWrapper
											.getIpcStructUint8Value(
													valVrtStStruct,
													VtnServiceIpcConsts.OPERSTATUS));
						}
					}
				} else {
					LOG.debug("Operation : normal Skip value struture");
					index++;
					if (dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
						LOG.debug("Operation : normal and target db :state Skip St value struture ");
						index++;
					}
				}
				// add current json object to array, if it has been initialized
				// earlier
				if (null != vRoutersArray) {
					vRoutersArray.add(vRouter);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != vRoutersArray) {
				root.add(rootJsonName, vRoutersArray);
			} else {
				root.add(rootJsonName, vRouter);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVRouterResponse");
		return root;
	}

	/**
	 * Used for List & Show FLowList response
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return JsonObject
	 */
	public JsonObject getFlowListResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody, final String getType) {
		LOG.trace("Start getFlowListResponse");
		final JsonObject root = new JsonObject();
		JsonArray flowListsArray = null;

		/*
		 * operation type will be required to resolve the response type
		 */
		LOG.debug("getType: " + getType);
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}

		String rootJsonName;
		/*
		 * get type (show or list) will be required to resolve root json name
		 * here it will be flowList for show and flowLists for list
		 */
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.FLOWLIST;
		} else {
			rootJsonName = VtnServiceJsonConsts.FLOWLISTS;
			// json array will be required for list type of cases
			flowListsArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject flowList = null;

		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			flowList = new JsonObject();
			flowList.addProperty(
					VtnServiceJsonConsts.COUNT,
					IpcDataUnitWrapper
							.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, flowList);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {

				flowList = new JsonObject();
				byte validBit;

				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;

				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyFlowListStruct = (IpcStruct) responsePacket[index++];
				flowList.addProperty(VtnServiceJsonConsts.FLNAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								keyFlowListStruct,
								VtnServiceJsonConsts.FLOWLISTNAME));

				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */
				if (getType.equals(VtnServiceJsonConsts.SHOW)
						|| opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
					LOG.debug("Case : Show or detail");
					/*
					 * add valid informations from value structure
					 */
					final IpcStruct valFlowListStruct = (IpcStruct) responsePacket[index++];

					validBit = valFlowListStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowlistIndex.UPLL_IDX_IP_TYPE_FL
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valFlowListStruct,
										VtnServiceIpcConsts.IPTYPE)
								.equalsIgnoreCase(
										UncStructIndexEnum.FlowlistIpType.UPLL_FLOWLIST_TYPE_IPV6
												.getValue())) {
							setValueToJsonObject(validBit, flowList,
									VtnServiceJsonConsts.IPVERSION,
									VtnServiceJsonConsts.IPV6);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valFlowListStruct,
										VtnServiceIpcConsts.IPTYPE)
								.equalsIgnoreCase(
										UncStructIndexEnum.FlowlistIpType.UPLL_FLOWLIST_TYPE_IP
												.getValue())) {
							setValueToJsonObject(validBit, flowList,
									VtnServiceJsonConsts.IPVERSION,
									VtnServiceJsonConsts.IP);
						} else {
							LOG.debug("Ip version: Invalid");
						}
						LOG.debug("Ip version :"
								+ IpcDataUnitWrapper.getIpcStructUint8Value(
										valFlowListStruct,
										VtnServiceIpcConsts.IPTYPE));
					}

				} else {
					LOG.debug("Operation : normal Skip value struture");
					index++;
				}
				// add current json object to array, if it has been initialized
				// earlier
				if (null != flowListsArray) {
					flowListsArray.add(flowList);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != flowListsArray) {
				root.add(rootJsonName, flowListsArray);
			} else {
				root.add(rootJsonName, flowList);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getFlowListResponse");
		return root;
	}

	public JsonObject getVBypassResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody, final String getType) {
		LOG.trace("Start getVBypassResponse");
		final JsonObject root = new JsonObject();
		JsonArray vBypassArray = null;
		LOG.debug("getType: " + getType);
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}

		String rootJsonName;
		/*
		 * get type (show or list) will be required to resolve root json name
		 * here it will be VBYPASS for show and VBYPASS for list
		 */
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.VBYPASS;
		} else {
			rootJsonName = VtnServiceJsonConsts.VBYPASSES;
			// json array will be required for list type of cases
			vBypassArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject vBypassList = null;

		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			vBypassList = new JsonObject();
			vBypassList
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, vBypassList);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {

				vBypassList = new JsonObject();
				byte validBit;

				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;

				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyFlowListStruct = (IpcStruct) responsePacket[index++];
				vBypassList.addProperty(VtnServiceJsonConsts.VBYPASS_NAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								keyFlowListStruct,
								VtnServiceIpcConsts.VUNKNOWNNAME));

				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */
				if (getType.equals(VtnServiceJsonConsts.SHOW)
						|| opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
					LOG.debug("Case : Show or detail");
					/*
					 * add valid informations from value structure
					 */
					final IpcStruct valVUnknownStruct = (IpcStruct) responsePacket[index++];

					validBit = valVUnknownStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVunknownIndex.UPLL_IDX_DESC_VUN
											.ordinal());

					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vBypassList,
								VtnServiceJsonConsts.DESCRIPTION,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVUnknownStruct,
										VtnServiceJsonConsts.DESCRIPTION));
					}
					validBit = valVUnknownStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVunknownIndex.UPLL_IDX_TYPE_VUN
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valVUnknownStruct,
										VtnServiceIpcConsts.TYPE)
								.equalsIgnoreCase(
										UncStructIndexEnum.ValVunknowntype.VUNKNOWN_TYPE_BRIDGE
												.getValue())) {
							setValueToJsonObject(validBit, vBypassList,
									VtnServiceJsonConsts.TYPE,
									VtnServiceJsonConsts.BRIDGE);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valVUnknownStruct,
										VtnServiceIpcConsts.TYPE)
								.equalsIgnoreCase(
										UncStructIndexEnum.ValVunknowntype.VUNKNOWN_TYPE_ROUTER
												.getValue())) {
							setValueToJsonObject(validBit, vBypassList,
									VtnServiceJsonConsts.TYPE,
									VtnServiceJsonConsts.ROUTER);
						} else {
							LOG.debug("Type: Invalid");
						}
						LOG.debug("Type :"
								+ IpcDataUnitWrapper.getIpcStructUint8Value(
										valVUnknownStruct,
										VtnServiceIpcConsts.TYPE));
					}
					validBit = valVUnknownStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVunknownIndex.UPLL_IDX_CONTROLLER_ID_VUN
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vBypassList,
								VtnServiceJsonConsts.CONTROLLERID,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVUnknownStruct,
										VtnServiceJsonConsts.CONTROLLERID));
					}
					validBit = valVUnknownStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVunknownIndex.UPLL_IDX_DOMAIN_ID_VUN
											.ordinal());

					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vBypassList,
								VtnServiceJsonConsts.DOMAINID,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVUnknownStruct,
										VtnServiceJsonConsts.DOMAINID));
					}

				} else {
					LOG.debug("Operation : normal Skip value struture");
					index++;
				}
				// add current json object to array, if it has been initialized
				// earlier
				if (null != vBypassArray) {
					vBypassArray.add(vBypassList);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != vBypassArray) {
				root.add(rootJsonName, vBypassArray);
			} else {
				root.add(rootJsonName, vBypassList);

			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVBypassResponse");
		return root;
	}

	/**
	 * Used for List & Show FlowFilterEntry response
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return JsonObject
	 */
	public JsonObject getVtnFlowFilterEntryResponse(
			final IpcDataUnit[] responsePacket, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start getVtnFlowFilterEntryResponse");
		final JsonObject root = new JsonObject();
		JsonArray flowFilterEntryArray = null;
		/*
		 * operation type will be required to resolve the response type
		 */
		LOG.debug("getType: " + getType);
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}
		/*
		 * data type will be required to resolve the response structures
		 */
		String dataType = VtnServiceJsonConsts.STATE;
		if (requestBody.has(VtnServiceJsonConsts.TARGETDB)) {
			dataType = requestBody.get(VtnServiceJsonConsts.TARGETDB)
					.getAsString();
		}
		String rootJsonName;
		/*
		 * get type (show or list) will be required to resolve root json name
		 * here it will be vtn for show and vtns for list
		 */
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.FLOWFILTERENTRY;
		} else {
			rootJsonName = VtnServiceJsonConsts.FLOWFILTERENTRIES;
			// json array will be required for list type of cases
			flowFilterEntryArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject flowFilterEntry = null;
		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			flowFilterEntry = new JsonObject();
			flowFilterEntry
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, flowFilterEntry);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {

				flowFilterEntry = new JsonObject();
				byte validBit;
				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;
				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyFlowFilterEntryStruct = (IpcStruct) responsePacket[index++];
				if (getType.equals(VtnServiceJsonConsts.SHOW)
						&& dataType
								.equalsIgnoreCase(VtnServiceJsonConsts.STATE)
						&& requestBody.has(VtnServiceJsonConsts.CONTROLLERID)
						&& requestBody.has(VtnServiceJsonConsts.DOMAINID)) {
					LOG.debug("Show with controller and domain information in state db");
					final IpcStruct valVtnFlowFilterControllerStruct = (IpcStruct) responsePacket[index++];
					// from controller - seqnum
					validBit = valVtnFlowFilterControllerStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterControllerIndex.UPLL_IDX_SEQ_NUM_FFC
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						flowFilterEntry.addProperty(
								VtnServiceJsonConsts.SEQNUM,
								IpcDataUnitWrapper.getIpcStructUint16Value(
										valVtnFlowFilterControllerStruct,
										VtnServiceIpcConsts.SEQUENCENUM));
					}

					final IpcStruct valVtnFlowFilterControllerStStruct = (IpcStruct) responsePacket[index++];
					final IpcStruct valFlowFilterEntryStruct = (IpcStruct) responsePacket[index++];

					getValVtnFlowFilterEntry(flowFilterEntry,
							valFlowFilterEntryStruct);
					if (opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
						LOG.debug("op specified as detail");
						validBit = valVtnFlowFilterControllerStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.ValVtnFlowfilterControllerStIndex.UPLL_IDX_NWM_STATUS_VFFCS
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							setValueToJsonObject(validBit, flowFilterEntry,
									VtnServiceJsonConsts.NMG_STATUS,
									IpcDataUnitWrapper.getIpcStructUint8Value(
											valVtnFlowFilterControllerStStruct,
											VtnServiceIpcConsts.NWM_STATUS));
						}
						final PomStatsIndex pomStatsIndexSet = new PomStatsIndex();
						pomStatsIndexSet
								.setSoftware(UncStructIndexEnum.ValVtnFlowfilterControllerStIndex.UPLL_IDX_SOFTWARE_VFFCS
										.ordinal());
						pomStatsIndexSet
								.setExistingFlow(UncStructIndexEnum.ValVtnFlowfilterControllerStIndex.UPLL_IDX_EXIST_VFFCS
										.ordinal());
						pomStatsIndexSet
								.setExpiredFlow(UncStructIndexEnum.ValVtnFlowfilterControllerStIndex.UPLL_IDX_EXPIRE_VFFCS
										.ordinal());
						pomStatsIndexSet
								.setTotal(UncStructIndexEnum.ValVtnFlowfilterControllerStIndex.UPLL_IDX_TOTAL_VFFCS
										.ordinal());
						LOG.debug("call getPomStats : for statics information");
						getPomStats(flowFilterEntry,
								valVtnFlowFilterControllerStStruct,
								pomStatsIndexSet);
						final String flowListName = VtnServiceJsonConsts.FLOWLIST;
						final JsonObject flowListJson = new JsonObject();
						final String flowListEntriesName = VtnServiceJsonConsts.FLOWLISTENTRIES;
						final JsonArray flowListEntriesJsonArray = new JsonArray();
						LOG.debug("call getPomStatsFLowList : for statics information of flowList");
						index = getPomStatsFlowList(responsePacket, index,
								flowListEntriesJsonArray);
						flowListJson.add(flowListEntriesName,
								flowListEntriesJsonArray);
						flowFilterEntry.add(flowListName, flowListJson);
					} else {
						LOG.debug("op not specified as detail");
						// increasing index to eliminate flow list entry
						// structures in case of show and op : normal
						index = responsePacket.length - 1;
					}
				} else {
					LOG.debug("Case except : Show with controller and domain information in state db");
					flowFilterEntry.addProperty(VtnServiceJsonConsts.SEQNUM,
							IpcDataUnitWrapper.getIpcStructUint16Value(
									keyFlowFilterEntryStruct,
									VtnServiceIpcConsts.SEQUENCENUM));
					if (getType.equals(VtnServiceJsonConsts.LIST)
							&& opType
									.equalsIgnoreCase(VtnServiceJsonConsts.NORMAL)) {
						LOG.debug("list with no detail information");
						index++;
					} else {
						LOG.debug("List with detail option or show case");
						/*
						 * add valid informations from value structure
						 */
						final IpcStruct valFlowFilterEntryStruct = (IpcStruct) responsePacket[index++];
						LOG.debug("call getValVtnFlowFilterEntry to get data from value structure ValVtnFlowFilterEntry");
						getValVtnFlowFilterEntry(flowFilterEntry,
								valFlowFilterEntryStruct);
					}
				}
				// add current json object to array, if it has been initialized
				// earlier
				if (null != flowFilterEntryArray) {
					flowFilterEntryArray.add(flowFilterEntry);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != flowFilterEntryArray) {
				root.add(rootJsonName, flowFilterEntryArray);
			} else {
				root.add(rootJsonName, flowFilterEntry);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVtnFlowFilterEntryResponse");

		return root;
	}

	/**
	 * used to give statistics Information in list for FlowListEntries
	 * 
	 * @param targetJson
	 * @param responseStruct
	 * @param pomStatsIndexSet
	 */
	public int getPomStatsFlowList(final IpcDataUnit[] responsePacket,
			final int indexValue, final JsonArray flowListResponseJsonArray) {
		LOG.trace("Start getPomStatsFLowList");
		byte validBit;
		int index = indexValue;
		LOG.debug("staring index: " + index);
		for (; index < responsePacket.length; index++) {

			final JsonObject flowListEntriesJson = new JsonObject();
			final IpcStruct valflowlistentrySt = (IpcStruct) responsePacket[index];
			validBit = valflowlistentrySt
					.getByte(
							VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValFlowlistEntryStIndex.UPLL_IDX_SEQ_NUM_FLES
									.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, flowListEntriesJson,
						VtnServiceJsonConsts.SEQNUM,
						IpcDataUnitWrapper.getIpcStructUint16Value(
								valflowlistentrySt,
								VtnServiceJsonConsts.SEQUENCENUM));
			}
			final PomStatsIndex pomStatsIndexSet = new PomStatsIndex();
			pomStatsIndexSet
					.setSoftware(UncStructIndexEnum.ValFlowlistEntryStIndex.UPLL_IDX_SOFTWARE_FLES
							.ordinal());
			pomStatsIndexSet
					.setExistingFlow(UncStructIndexEnum.ValFlowlistEntryStIndex.UPLL_IDX_EXIST_FLES
							.ordinal());
			pomStatsIndexSet
					.setExpiredFlow(UncStructIndexEnum.ValFlowlistEntryStIndex.UPLL_IDX_EXPIRE_FLES
							.ordinal());
			pomStatsIndexSet
					.setTotal(UncStructIndexEnum.ValFlowlistEntryStIndex.UPLL_IDX_TOTAL_FLES
							.ordinal());
			LOG.debug("call getPomStats : for statics information");
			getPomStats(flowListEntriesJson, valflowlistentrySt,
					pomStatsIndexSet);
			flowListResponseJsonArray.add(flowListEntriesJson);

		}
		LOG.debug("end index: " + index);
		LOG.trace("Complete getPomStatsFLowList");
		return index;
	}

	/**
	 * used to give statistics Information
	 * 
	 * @param targetJson
	 * @param responseStruct
	 * @param pomStatsIndexSet
	 */
	public void getPomStats(final JsonObject targetJson,
			final IpcStruct responseStruct, final PomStatsIndex pomStatsIndexSet) {
		LOG.trace("Start getPomStats");
		byte validBit;
		// pom stats starts
		final String statisticsFF = VtnServiceJsonConsts.STATISTICS;
		final JsonObject statisticsFFJson = new JsonObject();
		final String softwareFF = VtnServiceJsonConsts.SOFTWARE;
		final JsonObject softwareFFJson = new JsonObject();
		final String existFF1 = VtnServiceIpcConsts.EXISTINGFLOW;
		final JsonObject existFFJson = new JsonObject();
		final String expireFF = VtnServiceIpcConsts.EXPIREDFLOW;
		final JsonObject expireFFJson = new JsonObject();
		final String totalFF = VtnServiceJsonConsts.TOTAL;
		final JsonObject totalFFJson = new JsonObject();
		if (pomStatsIndexSet != null) {

			if (pomStatsIndexSet.getSoftware() != null) {
				validBit = responseStruct.getByte(VtnServiceIpcConsts.VALID,
						pomStatsIndexSet.getSoftware());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					validBit = IpcDataUnitWrapper
							.getInnerIpcStruct(responseStruct,
									VtnServiceJsonConsts.SOFTWARE)
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValPomStatsIndex.UPLL_IDX_STATS_PACKETS
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, softwareFFJson,
								VtnServiceJsonConsts.PACKETS,
								IpcDataUnitWrapper.getIpcStructUint64Value(
										IpcDataUnitWrapper.getInnerIpcStruct(
												responseStruct,
												VtnServiceJsonConsts.SOFTWARE),
										VtnServiceJsonConsts.PACKETS));
					}
					validBit = IpcDataUnitWrapper
							.getInnerIpcStruct(responseStruct,
									VtnServiceJsonConsts.SOFTWARE)
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValPomStatsIndex.UPLL_IDX_STATS_BYTES
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, softwareFFJson,
								VtnServiceJsonConsts.OCTETS,
								IpcDataUnitWrapper.getIpcStructUint64Value(
										IpcDataUnitWrapper.getInnerIpcStruct(
												responseStruct,
												VtnServiceJsonConsts.SOFTWARE),
										VtnServiceIpcConsts.BYTES));
					}
				}
			}
			statisticsFFJson.add(softwareFF, softwareFFJson);
			if (pomStatsIndexSet.getExistingFlow() != null) {
				validBit = responseStruct.getByte(VtnServiceIpcConsts.VALID,
						pomStatsIndexSet.getExistingFlow());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					validBit = IpcDataUnitWrapper
							.getInnerIpcStruct(responseStruct,
									VtnServiceIpcConsts.EXIST)
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValPomStatsIndex.UPLL_IDX_STATS_PACKETS
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, existFFJson,
								VtnServiceJsonConsts.PACKETS,
								IpcDataUnitWrapper.getIpcStructUint64Value(
										IpcDataUnitWrapper.getInnerIpcStruct(
												responseStruct,
												VtnServiceIpcConsts.EXIST),
										VtnServiceJsonConsts.PACKETS));
					}
					validBit = IpcDataUnitWrapper
							.getInnerIpcStruct(responseStruct,
									VtnServiceIpcConsts.EXIST)
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValPomStatsIndex.UPLL_IDX_STATS_BYTES
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, existFFJson,
								VtnServiceJsonConsts.OCTETS,
								IpcDataUnitWrapper.getIpcStructUint64Value(
										IpcDataUnitWrapper.getInnerIpcStruct(
												responseStruct,
												VtnServiceIpcConsts.EXIST),
										VtnServiceIpcConsts.BYTES));
					}
				}
			}
			statisticsFFJson.add(existFF1, existFFJson);
			if (pomStatsIndexSet.getExpiredFlow() != null) {
				validBit = responseStruct.getByte(VtnServiceIpcConsts.VALID,
						pomStatsIndexSet.getExpiredFlow());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					validBit = IpcDataUnitWrapper
							.getInnerIpcStruct(responseStruct,
									VtnServiceIpcConsts.EXPIRE)
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValPomStatsIndex.UPLL_IDX_STATS_PACKETS
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, expireFFJson,
								VtnServiceJsonConsts.PACKETS,
								IpcDataUnitWrapper.getIpcStructUint64Value(
										IpcDataUnitWrapper.getInnerIpcStruct(
												responseStruct,
												VtnServiceIpcConsts.EXPIRE),
										VtnServiceJsonConsts.PACKETS));
					}
					validBit = IpcDataUnitWrapper
							.getInnerIpcStruct(responseStruct,
									VtnServiceIpcConsts.EXPIRE)
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValPomStatsIndex.UPLL_IDX_STATS_BYTES
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, expireFFJson,
								VtnServiceJsonConsts.OCTETS,
								IpcDataUnitWrapper.getIpcStructUint64Value(
										IpcDataUnitWrapper.getInnerIpcStruct(
												responseStruct,
												VtnServiceIpcConsts.EXPIRE),
										VtnServiceIpcConsts.BYTES));
					}
				}
			}
			statisticsFFJson.add(expireFF, expireFFJson);
			if (pomStatsIndexSet.getTotal() != null) {
				validBit = responseStruct.getByte(VtnServiceIpcConsts.VALID,
						pomStatsIndexSet.getTotal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					validBit = IpcDataUnitWrapper
							.getInnerIpcStruct(responseStruct,
									VtnServiceJsonConsts.TOTAL)
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValPomStatsIndex.UPLL_IDX_STATS_PACKETS
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, totalFFJson,
								VtnServiceJsonConsts.PACKETS,
								IpcDataUnitWrapper.getIpcStructUint64Value(
										IpcDataUnitWrapper.getInnerIpcStruct(
												responseStruct,
												VtnServiceJsonConsts.TOTAL),
										VtnServiceJsonConsts.PACKETS));
					}
					validBit = IpcDataUnitWrapper
							.getInnerIpcStruct(responseStruct,
									VtnServiceJsonConsts.TOTAL)
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValPomStatsIndex.UPLL_IDX_STATS_BYTES
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, totalFFJson,
								VtnServiceJsonConsts.OCTETS,
								IpcDataUnitWrapper.getIpcStructUint64Value(
										IpcDataUnitWrapper.getInnerIpcStruct(
												responseStruct,
												VtnServiceJsonConsts.TOTAL),
										VtnServiceIpcConsts.BYTES));
					}
				}
			}
			statisticsFFJson.add(totalFF, totalFFJson);
		} else {
			LOG.debug("pomStatsIndexSet is null");
			statisticsFFJson.add(softwareFF, softwareFFJson);
			statisticsFFJson.add(existFF1, existFFJson);
			statisticsFFJson.add(expireFF, expireFFJson);
			statisticsFFJson.add(totalFF, totalFFJson);
		}

		targetJson.add(statisticsFF, statisticsFFJson);
		LOG.debug("statics Json: " + targetJson.toString());
		LOG.trace("complete getPomStats");
	}

	public JsonObject getDhcpRelayResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody, final String getType) {
		LOG.trace("Start getDhcpRelayResponse");
		final JsonObject root = new JsonObject();

		final String rootJsonName = VtnServiceJsonConsts.DHCPRELAY;
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject dhcpRelay = null;

		for (int index = 0; index < responsePacket.length; index++) {

			dhcpRelay = new JsonObject();
			byte validBit;

			// There is no use of key type
			LOG.debug("Skip key type: no use");
			index++;

			/*
			 * There is no use of key structure
			 */
			LOG.debug("Skip key Struture: no use");
			index++;

			/*
			 * add valid informations from value structure
			 */
			final IpcStruct valVrtStruct = (IpcStruct) responsePacket[index++];

			validBit = valVrtStruct
					.getByte(
							VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVrtIndex.UPLL_IDX_DHCP_RELAY_ADMIN_STATUS_VRT
									.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				if (IpcDataUnitWrapper.getIpcStructUint8Value(valVrtStruct,
						VtnServiceIpcConsts.DHCPRELAYADMINSTATUS).equals(
						UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_ENABLE
								.getValue())) {
					setValueToJsonObject(validBit, dhcpRelay,
							VtnServiceJsonConsts.DHCPRELAYSTATUS,
							VtnServiceJsonConsts.ENABLE);
				} else if (IpcDataUnitWrapper
						.getIpcStructUint8Value(valVrtStruct,
								VtnServiceIpcConsts.DHCPRELAYADMINSTATUS)
						.equals(UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_DISABLE
								.getValue())) {
					setValueToJsonObject(validBit, dhcpRelay,
							VtnServiceJsonConsts.DHCPRELAYSTATUS,
							VtnServiceJsonConsts.DISABLE);
				} else {
					LOG.debug("DhcpRelaystatus : Invalid value");
				}
				LOG.debug("DhcpRelaystatus :"
						+ IpcDataUnitWrapper.getIpcStructUint8Value(
								valVrtStruct,
								VtnServiceIpcConsts.DHCPRELAYADMINSTATUS));
			}
		}

		/*
		 * finally add either array or single object to root json object and
		 * return the same.
		 */
		root.add(rootJsonName, dhcpRelay);
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getDhcpRelayResponse");

		return root;
	}

	public JsonObject getDhcpRelayInterfaceResponse(
			final IpcDataUnit[] responsePacket, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start getDhcpRelayInterfaceResponse");
		final JsonObject root = new JsonObject();
		JsonArray dhcpRelayInterfacesArray = null;
		LOG.debug("getType: " + getType);
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}

		/*
		 * data type will be required to resolve the response structures
		 */
		String dataType = VtnServiceJsonConsts.STATE;
		if (requestBody.has(VtnServiceJsonConsts.TARGETDB)) {
			dataType = requestBody.get(VtnServiceJsonConsts.TARGETDB)
					.getAsString();
		}
		/*
		 * data type will be required to resolve the response structures
		 */
		String rootJsonName;
		/*
		 * get type (show or list) will be required to resolve root json name
		 * here it will be vRouter for show and vRouters for list
		 */
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.INTERFACE;
		} else {
			rootJsonName = VtnServiceJsonConsts.INTERFACES;
			// json array will be required for list type of cases
			dhcpRelayInterfacesArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject dhcpRelayIf = null;

		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			dhcpRelayIf = new JsonObject();
			dhcpRelayIf
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, dhcpRelayIf);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {
				dhcpRelayIf = new JsonObject();
				byte validBit;
				// There is no use of key type
				index++;
				LOG.debug("Skip key type: no use");
				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyDhcpRelayIfStruct = (IpcStruct) responsePacket[index++];
				dhcpRelayIf.addProperty(VtnServiceJsonConsts.IFNAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								keyDhcpRelayIfStruct,
								VtnServiceIpcConsts.IFNAME));
				/*
				 * add valid informations from value structure
				 */
				if (dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
					final IpcStruct valDhcpRelayIfStStruct = (IpcStruct) responsePacket[index++];

					validBit = valDhcpRelayIfStStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValDhcpRelayIfStIndex.UPLL_IDX_DHCP_RELAY_STATUS_DRIS
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						LOG.debug("targetdb : State");
						if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valDhcpRelayIfStStruct,
										VtnServiceIpcConsts.DHCPRELAY_STATUS)
								.equals(UncStructIndexEnum.ValDhcpRelayIfStatus.UPLL_DR_IF_ACTIVE
										.getValue())) {
							setValueToJsonObject(validBit, dhcpRelayIf,
									VtnServiceJsonConsts.STATUS,
									VtnServiceJsonConsts.ACTIVE);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valDhcpRelayIfStStruct,
										VtnServiceIpcConsts.DHCPRELAY_STATUS)
								.equals(UncStructIndexEnum.ValDhcpRelayIfStatus.UPLL_DR_IF_ERROR
										.getValue())) {
							setValueToJsonObject(validBit, dhcpRelayIf,
									VtnServiceJsonConsts.STATUS,
									VtnServiceJsonConsts.ERROR);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valDhcpRelayIfStStruct,
										VtnServiceIpcConsts.DHCPRELAY_STATUS)
								.equals(UncStructIndexEnum.ValDhcpRelayIfStatus.UPLL_DR_IF_INACTIVE
										.getValue())) {
							setValueToJsonObject(validBit, dhcpRelayIf,
									VtnServiceJsonConsts.STATUS,
									VtnServiceJsonConsts.INACTIVE);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valDhcpRelayIfStStruct,
										VtnServiceIpcConsts.DHCPRELAY_STATUS)
								.equals(UncStructIndexEnum.ValDhcpRelayIfStatus.UPLL_DR_IF_STARTING
										.getValue())) {
							setValueToJsonObject(validBit, dhcpRelayIf,
									VtnServiceJsonConsts.STATUS,
									VtnServiceJsonConsts.STARTING);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valDhcpRelayIfStStruct,
										VtnServiceIpcConsts.DHCPRELAY_STATUS)
								.equals(UncStructIndexEnum.ValDhcpRelayIfStatus.UPLL_DR_IF_WAITING
										.getValue())) {
							setValueToJsonObject(validBit, dhcpRelayIf,
									VtnServiceJsonConsts.STATUS,
									VtnServiceJsonConsts.WAITING);
						} else {
							LOG.debug("status : Invalid value");
						}
						LOG.debug("status :"
								+ IpcDataUnitWrapper.getIpcStructUint8Value(
										valDhcpRelayIfStStruct,
										VtnServiceIpcConsts.DHCPRELAY_STATUS));
					}
				} else {
					LOG.debug("targetdb : not state Skip value struture");
					index++;

				}
				// add current json object to array, if it has been initialized
				// earlier
				if (null != dhcpRelayInterfacesArray) {
					dhcpRelayInterfacesArray.add(dhcpRelayIf);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != dhcpRelayInterfacesArray) {
				root.add(rootJsonName, dhcpRelayInterfacesArray);
			} else {
				root.add(rootJsonName, dhcpRelayIf);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getDhcpRelayInterfaceResponse");
		return root;
	}

	/**
	 * Set Value to Json according to valid bit
	 * 
	 * @param validBit
	 * @param json
	 * @param key
	 * @param value
	 */
	private void setValueToJsonObject(final byte validBit,
			final JsonObject json, final String key, final String value) {
		if (validBit == (byte) UncStructIndexEnum.Valid.UNC_VF_VALID.ordinal()
				|| validBit == (byte) UncStructIndexEnum.Valid.UNC_VF_VALID_NO_VALUE
						.ordinal()) {
			json.addProperty(key, value);
		} else {
			throw new IllegalArgumentException(
					UncJavaAPIErrorCode.IPC_SERVER_ERROR.getErrorCode()
							+ VtnServiceConsts.HYPHEN
							+ UncJavaAPIErrorCode.IPC_SERVER_ERROR
									.getErrorMessage());
		}
	}

	/**
	 * Function to get response of Flow List Entry
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return
	 */
	public JsonObject getFlowListEntryResponse(
			final IpcDataUnit[] responsePacket, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start getFlowListEntryResponse");
		final JsonObject root = new JsonObject();
		JsonArray flowListEntrysArray = null;
		LOG.debug("getType: " + getType);
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}
		/*
		 * data type will be required to resolve the response structures
		 */
		String rootJsonName;
		/*
		 * get type (show or list) will be required to resolve root json name
		 * here it will be flowlistentry for show and flowlistentries for list
		 */
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.FLOWLISTENTRY;
		} else {
			rootJsonName = VtnServiceJsonConsts.FLOWLISTENTRIES;
			// json array will be required for list type of cases
			flowListEntrysArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject flowListEntry = null;

		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			flowListEntry = new JsonObject();
			flowListEntry
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, flowListEntry);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {

				flowListEntry = new JsonObject();
				byte validBit;
				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;
				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyFlowListEntryStruct = (IpcStruct) responsePacket[index++];
				flowListEntry.addProperty(VtnServiceJsonConsts.SEQNUM,
						IpcDataUnitWrapper.getIpcStructUint16Value(
								keyFlowListEntryStruct,
								VtnServiceIpcConsts.SEQUENCENUM));
				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */
				if (getType.equals(VtnServiceJsonConsts.SHOW)
						|| opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
					LOG.debug("Case : Show or detail");
					/*
					 * add valid informations from value structure
					 */
					final IpcStruct valFlowListEntryStruct = (IpcStruct) responsePacket[index++];

					validBit = valFlowListEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_MAC_DST_FLE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, flowListEntry,
								VtnServiceJsonConsts.MACDSTADDR,
								// IpcDataUnitWrapper.getIpcStructUint8ArrayValue(valFlowListEntryStruct,
								// VtnServiceIpcConsts.MACDST));
								IpcDataUnitWrapper.getMacAddress(
										valFlowListEntryStruct,
										VtnServiceIpcConsts.MACDST));
					}

					validBit = valFlowListEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_MAC_SRC_FLE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, flowListEntry,
								VtnServiceJsonConsts.MACSRCADDR,
								// IpcDataUnitWrapper.getIpcStructUint8ArrayValue(valFlowListEntryStruct,
								// VtnServiceIpcConsts.MACSRC));
								IpcDataUnitWrapper.getMacAddress(
										valFlowListEntryStruct,
										VtnServiceIpcConsts.MACSRC));
					}

					validBit = valFlowListEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_MAC_ETH_TYPE_FLE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, flowListEntry,
								VtnServiceJsonConsts.MACETHERTYPE,
								IpcDataUnitWrapper.getIpcStructUint16HexaValue(
										valFlowListEntryStruct,
										VtnServiceIpcConsts.MAC_ETH_TYPE));
					}

					validBit = valFlowListEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_VLAN_PRIORITY_FLE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, flowListEntry,
								VtnServiceJsonConsts.MACVLANPRIORITY,
								IpcDataUnitWrapper.getIpcStructUint8Value(
										valFlowListEntryStruct,
										VtnServiceIpcConsts.VLAN_PRIORITY));
					}

					validBit = valFlowListEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_DST_IP_FLE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, flowListEntry,
								VtnServiceJsonConsts.IPDSTADDR,
								IpcDataUnitWrapper.getIpcStructIpv4Value(
										valFlowListEntryStruct,
										VtnServiceIpcConsts.DST_IP));
					}

					validBit = valFlowListEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_DST_IP_PREFIX_FLE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, flowListEntry,
								VtnServiceJsonConsts.IPDSTADDRPREFIX,
								IpcDataUnitWrapper.getIpcStructUint8Value(
										valFlowListEntryStruct,
										VtnServiceIpcConsts.DST_IP_PREFIXLEN));
					}

					validBit = valFlowListEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_SRC_IP_FLE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, flowListEntry,
								VtnServiceJsonConsts.IPSRCADDR,
								IpcDataUnitWrapper.getIpcStructIpv4Value(
										valFlowListEntryStruct,
										VtnServiceIpcConsts.SRC_IP));
					}

					validBit = valFlowListEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_SRC_IP_PREFIX_FLE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, flowListEntry,
								VtnServiceJsonConsts.IPSRCADDRPREFIX,
								IpcDataUnitWrapper.getIpcStructUint8Value(
										valFlowListEntryStruct,
										VtnServiceIpcConsts.SRC_IP_PREFIXLEN));
					}

					validBit = valFlowListEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_DST_IP_V6_FLE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, flowListEntry,
								VtnServiceJsonConsts.IPV6DSTADDR,
								IpcDataUnitWrapper.getIpcStructIpv6Value(
										valFlowListEntryStruct,
										VtnServiceIpcConsts.DST_IPV6));
					}

					validBit = valFlowListEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_DST_IP_V6_PREFIX_FLE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, flowListEntry,
								VtnServiceJsonConsts.IPV6DSTADDRPREFIX,
								IpcDataUnitWrapper.getIpcStructUint8Value(
										valFlowListEntryStruct,
										VtnServiceIpcConsts.DST_IPV6_PREFIXLEN));
					}

					validBit = valFlowListEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_SRC_IP_V6_FLE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, flowListEntry,
								VtnServiceJsonConsts.IPV6SRCADDR,
								IpcDataUnitWrapper.getIpcStructIpv6Value(
										valFlowListEntryStruct,
										VtnServiceIpcConsts.SRC_IPV6));
					}

					validBit = valFlowListEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_SRC_IP_V6_PREFIX_FLE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, flowListEntry,
								VtnServiceJsonConsts.IPV6SRCADDRPREFIX,
								IpcDataUnitWrapper.getIpcStructUint8Value(
										valFlowListEntryStruct,
										VtnServiceIpcConsts.SRC_IPV6_PREFIXLEN));
					}

					validBit = valFlowListEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_IP_PROTOCOL_FLE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, flowListEntry,
								VtnServiceJsonConsts.IPPROTO,
								IpcDataUnitWrapper.getIpcStructUint8Value(
										valFlowListEntryStruct,
										VtnServiceIpcConsts.IP_PROTO));
					}

					validBit = valFlowListEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_IP_DSCP_FLE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, flowListEntry,
								VtnServiceJsonConsts.IPDSCP,
								IpcDataUnitWrapper.getIpcStructUint8Value(
										valFlowListEntryStruct,
										VtnServiceIpcConsts.IP_DSCP));
					}

					validBit = valFlowListEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_L4_DST_PORT_FLE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, flowListEntry,
								VtnServiceJsonConsts.L4DSTPORT,
								IpcDataUnitWrapper.getIpcStructUint16Value(
										valFlowListEntryStruct,
										VtnServiceIpcConsts.L4_DST_PORT));
					}

					validBit = valFlowListEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_L4_DST_PORT_ENDPT_FLE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, flowListEntry,
								VtnServiceJsonConsts.L4DSTENDPORT,
								IpcDataUnitWrapper.getIpcStructUint16Value(
										valFlowListEntryStruct,
										VtnServiceIpcConsts.L4_DST_PORT_ENDPT));
					}

					validBit = valFlowListEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_L4_SRC_PORT_FLE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, flowListEntry,
								VtnServiceJsonConsts.L4SRCPORT,
								IpcDataUnitWrapper.getIpcStructUint16Value(
										valFlowListEntryStruct,
										VtnServiceIpcConsts.L4_SRC_PORT));
					}

					validBit = valFlowListEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_L4_SRC_PORT_ENDPT_FLE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, flowListEntry,
								VtnServiceJsonConsts.L4SRCENDPORT,
								IpcDataUnitWrapper.getIpcStructUint16Value(
										valFlowListEntryStruct,
										VtnServiceIpcConsts.L4_SRC_PORT_ENDPT));
					}

					validBit = valFlowListEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_ICMP_TYPE_FLE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, flowListEntry,
								VtnServiceJsonConsts.ICMPTYPENUM,
								IpcDataUnitWrapper.getIpcStructUint8Value(
										valFlowListEntryStruct,
										VtnServiceIpcConsts.ICMP_TYPE));
					}

					validBit = valFlowListEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_ICMP_CODE_FLE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, flowListEntry,
								VtnServiceJsonConsts.ICMPCODENUM,
								IpcDataUnitWrapper.getIpcStructUint8Value(
										valFlowListEntryStruct,
										VtnServiceIpcConsts.ICMP_CODE));
					}

					validBit = valFlowListEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_ICMP_V6_TYPE_FLE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, flowListEntry,
								VtnServiceJsonConsts.IPV6ICMPTYPENUM,
								IpcDataUnitWrapper.getIpcStructUint8Value(
										valFlowListEntryStruct,
										VtnServiceIpcConsts.ICMPV6_TYPE));
					}

					validBit = valFlowListEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowlistEntryIndex.UPLL_IDX_ICMP_V6_CODE_FLE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, flowListEntry,
								VtnServiceJsonConsts.IPV6ICMPCODENUM,
								IpcDataUnitWrapper.getIpcStructUint8Value(
										valFlowListEntryStruct,
										VtnServiceIpcConsts.ICMPV6_CODE));
					}

				} else {
					LOG.debug("Operation : normal Skip value struture");
					index++;
				}
				// add current json object to array, if it has been initialized
				// earlier
				if (null != flowListEntrysArray) {
					flowListEntrysArray.add(flowListEntry);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != flowListEntrysArray) {
				root.add(rootJsonName, flowListEntrysArray);
			} else {
				root.add(rootJsonName, flowListEntry);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getFlowListEntryResponse");

		return root;
	}

	/**
	 * Function for ARP Entry Show
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return
	 */
	public JsonObject getARPEntryResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody, final String getType) {
		LOG.trace("Start getARPEntryResponse");
		final JsonObject root = new JsonObject();
		final JsonArray aRPEntriesArray = new JsonArray();
		LOG.debug("getType: " + getType);
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}

		final String rootJsonName = VtnServiceJsonConsts.ARPENTRIES;
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject aRPEntries = null;

		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			aRPEntries = new JsonObject();
			String count = VtnServiceConsts.ZERO;
			if (responsePacket.length >= 1) {
				count = IpcDataUnitWrapper
						.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]);
			}
			aRPEntries.addProperty(VtnServiceJsonConsts.COUNT, count);
			root.add(rootJsonName, aRPEntries);
		} else {
			int index = 0;
			/*
			 * There is no use of key Type
			 */
			LOG.debug("Skip key type: no use");
			index++;

			/*
			 * There is no use of key structure
			 */
			LOG.debug("Skip key structure: no use");
			index++;

			// get count of value structure
			Long count = 0L;
			if (responsePacket.length != 0) {
				count = Long
						.parseLong(IpcDataUnitWrapper
								.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			}
			LOG.debug("Count of value structure: " + count);
			/*
			 * moving from count to value structure parameter
			 */
			index++;

			for (int indexValueStructure = 0; indexValueStructure < count; indexValueStructure++) {

				aRPEntries = new JsonObject();
				byte validBit;
				/*
				 * add valid informations from value structure
				 */
				final IpcStruct valARPEntriesStruct = (IpcStruct) responsePacket[index++];

				/*
				 * this part is always required in Show, but not required in
				 * Only Show case id there so no need of additional if
				 */
				validBit = valARPEntriesStruct
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtArpEntryStIndex.UPLL_IDX_IP_ADDR_VAES
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					setValueToJsonObject(validBit, aRPEntries,
							VtnServiceJsonConsts.IPADDR,
							IpcDataUnitWrapper.getIpcStructIpv4Value(
									valARPEntriesStruct,
									VtnServiceIpcConsts.IP_ADDR));
				}

				validBit = valARPEntriesStruct
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtArpEntryStIndex.UPLL_IDX_MAC_ADDR_VAES
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					setValueToJsonObject(validBit, aRPEntries,
							VtnServiceJsonConsts.MACADDR,
							IpcDataUnitWrapper.getMacAddress(
									valARPEntriesStruct,
									VtnServiceIpcConsts.MACADDR));
				}

				validBit = valARPEntriesStruct
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtArpEntryStIndex.UPLL_IDX_TYPE_VAES
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {

					if (IpcDataUnitWrapper
							.getIpcStructUint8Value(valARPEntriesStruct,
									VtnServiceIpcConsts.TYPE)
							.equals(UncStructIndexEnum.ValMacEntry.UPLL_MAC_ENTRY_STATIC
									.getValue())) {
						setValueToJsonObject(validBit, aRPEntries,
								VtnServiceJsonConsts.TYPE,
								VtnServiceJsonConsts.STATIC);
					} else if (IpcDataUnitWrapper
							.getIpcStructUint8Value(valARPEntriesStruct,
									VtnServiceIpcConsts.TYPE)
							.equals(UncStructIndexEnum.ValMacEntry.UPLL_MAC_ENTRY_DYNAMIC
									.getValue())) {
						setValueToJsonObject(validBit, aRPEntries,
								VtnServiceJsonConsts.TYPE,
								VtnServiceJsonConsts.DYNAMIC);
					} else {
						LOG.debug("Type: Invalid value");
					}
					LOG.debug("Type:"
							+ IpcDataUnitWrapper.getIpcStructUint8Value(
									valARPEntriesStruct,
									VtnServiceIpcConsts.TYPE));
				}

				validBit = valARPEntriesStruct
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtArpEntryStIndex.UPLL_IDX_IF_NAME_VAES
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					setValueToJsonObject(validBit, aRPEntries,
							VtnServiceJsonConsts.IFNAME,
							IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
									valARPEntriesStruct,
									VtnServiceIpcConsts.IFNAME));
				}

				// add current json object to array, if it has been initialized
				// earlier
				aRPEntriesArray.add(aRPEntries);
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			root.add(rootJsonName, aRPEntriesArray);
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getARPEntryResponse");

		return root;
	}

	/**
	 * Function to create DHCP Relay Server Response (Show / List) There is no
	 * key structure
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return
	 */
	public JsonObject getDHCPRelayServerResponse(
			final IpcDataUnit[] responsePacket, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start getDHCPRelayServerResponse");
		final JsonObject root = new JsonObject();
		JsonArray relayServerArray = null;
		LOG.debug("getType: " + getType);
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}
		String rootJsonName;
		/*
		 * get type (show or list) will be required to resolve root json name
		 * here it will be vtn for show and vtns for list
		 */
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.SERVER;
		} else {
			rootJsonName = VtnServiceJsonConsts.SERVERS;
			// json array will be required for list type of cases
			relayServerArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject dHCPRelayServer = null;

		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			dHCPRelayServer = new JsonObject();
			dHCPRelayServer
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, dHCPRelayServer);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {

				dHCPRelayServer = new JsonObject();

				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;
				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyDhcpRelayServerStruct = (IpcStruct) responsePacket[index++];
				dHCPRelayServer.addProperty(VtnServiceJsonConsts.IPADDR,
						IpcDataUnitWrapper.getIpcStructIpv4Value(
								keyDhcpRelayServerStruct,
								VtnServiceIpcConsts.SERVERADDR));
				// There is no use of value structure
				LOG.debug("Skip Value strcuture: no use");
				index++;
				// add current json object to array, if it has been initialized
				// earlier
				if (null != relayServerArray) {
					relayServerArray.add(dHCPRelayServer);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != relayServerArray) {
				root.add(rootJsonName, relayServerArray);
			} else {
				root.add(rootJsonName, dHCPRelayServer);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getDHCPRelayServerResponse");

		return root;
	}

	/**
	 * Function to get vRouter Interface Flow Filter Show Response
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return
	 */
	public JsonObject getVRouterInterfaceFlowFilterResponse(
			final IpcDataUnit[] responsePacket, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start getVRouterInterfaceFlowFilterResponse");
		final JsonObject root = new JsonObject();

		final String rootJsonName = VtnServiceJsonConsts.FLOWFILTER;
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject vrtInterfaceFF = null;

		for (int index = 0; index < responsePacket.length; index++) {
			vrtInterfaceFF = new JsonObject();
			// There is no use of key type
			LOG.debug("Skip key type: no use");
			index++;
			/*
			 * add mandatory informations from key structure
			 */
			final IpcStruct keyFlowFilterStruct = (IpcStruct) responsePacket[index++];

			if (IpcDataUnitWrapper
					.getIpcStructUint8Value(keyFlowFilterStruct,
							VtnServiceIpcConsts.DIRECTION)
					.equals(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_IN
							.getValue())) {
				vrtInterfaceFF.addProperty(VtnServiceJsonConsts.FFTYPE,
						VtnServiceJsonConsts.IN);
				LOG.debug("FF Type :"
						+ IpcDataUnitWrapper.getIpcStructUint8Value(
								keyFlowFilterStruct,
								VtnServiceIpcConsts.DIRECTION));

			} else if (IpcDataUnitWrapper
					.getIpcStructUint8Value(keyFlowFilterStruct,
							VtnServiceIpcConsts.DIRECTION)
					.equals(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_OUT
							.getValue())) {
				vrtInterfaceFF.addProperty(VtnServiceJsonConsts.FFTYPE,
						VtnServiceJsonConsts.OUT);
				LOG.debug("FF Type :"
						+ IpcDataUnitWrapper.getIpcStructUint8Value(
								keyFlowFilterStruct,
								VtnServiceIpcConsts.DIRECTION));
			} else {
				LOG.debug("Invalid value for FFTYPE parameter");
			}
		}
		/*
		 * finally add either array or single object to root json object and
		 * return the same.
		 */
		root.add(rootJsonName, vrtInterfaceFF);
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVRouterInterfaceFlowFilterResponse");
		return root;
	}

	/**
	 * Function to get vRouter Interface Flow Filter Entry Response
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return
	 */
	public JsonObject getVRouterInterfaceFlowFilterEntryResponse(
			final IpcDataUnit[] responsePacket, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start getVRouterInterfaceFlowFilterEntryResponse");
		final JsonObject root = new JsonObject();
		JsonArray vRouterflowFilterEntryArray = null;
		LOG.debug("getType: " + getType);
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}
		/*
		 * data type will be required to resolve the response structures
		 */
		String dataType = VtnServiceJsonConsts.STATE;
		if (requestBody.has(VtnServiceJsonConsts.TARGETDB)) {
			dataType = requestBody.get(VtnServiceJsonConsts.TARGETDB)
					.getAsString();
		}
		String rootJsonName;
		/*
		 * get type (show or list) will be required to resolve root json name
		 * here it will be vtn for show and vtns for list
		 */
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.FLOWFILTERENTRY;
		} else {
			rootJsonName = VtnServiceJsonConsts.FLOWFILTERENTRIES;
			// json array will be required for list type of cases
			vRouterflowFilterEntryArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject vRouterflowFilterEntry = null;

		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			vRouterflowFilterEntry = new JsonObject();
			vRouterflowFilterEntry
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, vRouterflowFilterEntry);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {

				vRouterflowFilterEntry = new JsonObject();
				byte validBit;

				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;

				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyFlowFilterEntryStruct = (IpcStruct) responsePacket[index++];
				vRouterflowFilterEntry.addProperty(VtnServiceJsonConsts.SEQNUM,
						IpcDataUnitWrapper.getIpcStructUint16Value(
								keyFlowFilterEntryStruct,
								VtnServiceIpcConsts.SEQUENCENUM));

				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */
				if (getType.equals(VtnServiceJsonConsts.LIST)
						&& opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)
						|| getType.equals(VtnServiceJsonConsts.SHOW)) {
					LOG.debug("Case : Show or List with detail");
					/*
					 * add valid informations from value structure
					 */
					final IpcStruct valFlowFilterEntryStruct = (IpcStruct) responsePacket[index++];

					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_FLOWLIST_NAME_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vRouterflowFilterEntry,
								VtnServiceJsonConsts.FLNAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valFlowFilterEntryStruct,
										VtnServiceJsonConsts.FLOWLISTNAME));
					}
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_ACTION_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						if (IpcDataUnitWrapper
								.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.ACTION)
								.equalsIgnoreCase(
										UncStructIndexEnum.FlowfilterAction.UPLL_FLOWFILTER_ACT_PASS
												.getValue())) {
							setValueToJsonObject(validBit,
									vRouterflowFilterEntry,
									VtnServiceJsonConsts.ACTIONTYPE,
									VtnServiceJsonConsts.PASS);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.ACTION)
								.equalsIgnoreCase(
										UncStructIndexEnum.FlowfilterAction.UPLL_FLOWFILTER_ACT_DROP
												.getValue())) {
							setValueToJsonObject(validBit,
									vRouterflowFilterEntry,
									VtnServiceJsonConsts.ACTIONTYPE,
									VtnServiceJsonConsts.DROP);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.ACTION)
								.equalsIgnoreCase(
										UncStructIndexEnum.FlowfilterAction.UPLL_FLOWFILTER_ACT_REDIRECT
												.getValue())) {
							setValueToJsonObject(validBit,
									vRouterflowFilterEntry,
									VtnServiceJsonConsts.ACTIONTYPE,
									VtnServiceJsonConsts.REDIRECT);
						} else {
							LOG.debug("Invalid value for Action type parameter");
						}
						LOG.debug("Action type :"
								+ IpcDataUnitWrapper.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.ACTION));
					}
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_NWM_NAME_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vRouterflowFilterEntry,
								VtnServiceJsonConsts.NMGNAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valFlowFilterEntryStruct,
										VtnServiceJsonConsts.NWMNAME));
					}
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_PRIORITY_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vRouterflowFilterEntry,
								VtnServiceJsonConsts.PRIORITY,
								IpcDataUnitWrapper.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceJsonConsts.PRIORITY));
					}
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_DSCP_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vRouterflowFilterEntry,
								VtnServiceJsonConsts.DSCP,
								IpcDataUnitWrapper.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceJsonConsts.DSCP));
					}

					final JsonObject redirectDst = new JsonObject();
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_REDIRECT_NODE_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, redirectDst,
								VtnServiceJsonConsts.VNODENAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.REDIRECTNODE));
					}
					// Query for if_name currently mapping as per FD ie network
					// monitor group name
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_REDIRECT_PORT_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, redirectDst,
								VtnServiceJsonConsts.IFNAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.REDIRECTPORT));
					}
					// Direction
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_REDIRECT_DIRECTION_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						if (IpcDataUnitWrapper
								.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.REDIRECTDIRECTION)
								.equals(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_IN
										.getValue())) {
							setValueToJsonObject(validBit, redirectDst,
									VtnServiceJsonConsts.DIRECTION,
									VtnServiceJsonConsts.IN);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.REDIRECTDIRECTION)
								.equals(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_OUT
										.getValue())) {
							setValueToJsonObject(validBit, redirectDst,
									VtnServiceJsonConsts.DIRECTION,
									VtnServiceJsonConsts.OUT);
						} else {
							LOG.debug("Direction : Invalid");
						}
						LOG.debug("Direction :"
								+ IpcDataUnitWrapper.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.REDIRECTDIRECTION));
					}
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_MODIFY_DST_MAC_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, redirectDst,
								VtnServiceJsonConsts.MACDSTADDR,
								IpcDataUnitWrapper.getMacAddress(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.MODIFYDSTMACADDR));
					}
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_MODIFY_SRC_MAC_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, redirectDst,
								VtnServiceJsonConsts.MACSRCADDR,
								IpcDataUnitWrapper.getMacAddress(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.MODIFYSRCMACADDR));
					}
					vRouterflowFilterEntry.add(
							VtnServiceJsonConsts.REDIRECTDST, redirectDst);

				}
				if (getType.equals(VtnServiceJsonConsts.SHOW)
						&& dataType
								.equalsIgnoreCase(VtnServiceJsonConsts.STATE)
						&& opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
					LOG.debug("Case : Show and targetdb :State ");
					final IpcStruct valFlowFilterEntryStStruct = (IpcStruct) responsePacket[index++];
					if (opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
						LOG.debug("op : detail");
						validBit = valFlowFilterEntryStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.ValFlowfilterEntryStIndex.UPLL_IDX_NWM_STATUS_FFES
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							setValueToJsonObject(validBit,
									vRouterflowFilterEntry,
									VtnServiceJsonConsts.NMG_STATUS,
									IpcDataUnitWrapper.getIpcStructUint8Value(
											valFlowFilterEntryStStruct,
											VtnServiceIpcConsts.NWM_STATUS));
						}
						final PomStatsIndex pomStatsIndexSet = new PomStatsIndex();
						pomStatsIndexSet
								.setSoftware(UncStructIndexEnum.ValFlowfilterEntryStIndex.UPLL_IDX_SOFTWARE_FFES
										.ordinal());
						pomStatsIndexSet
								.setExistingFlow(UncStructIndexEnum.ValFlowfilterEntryStIndex.UPLL_IDX_EXIST_FFES
										.ordinal());
						pomStatsIndexSet
								.setExpiredFlow(UncStructIndexEnum.ValFlowfilterEntryStIndex.UPLL_IDX_EXPIRE_FFES
										.ordinal());
						pomStatsIndexSet
								.setTotal(UncStructIndexEnum.ValFlowfilterEntryStIndex.UPLL_IDX_TOTAL_FFES
										.ordinal());
						LOG.debug("call getPomStats : for statics information");
						getPomStats(vRouterflowFilterEntry,
								valFlowFilterEntryStStruct, pomStatsIndexSet);
						final String flowListName = VtnServiceJsonConsts.FLOWLIST;
						final JsonObject flowListJson = new JsonObject();
						final String flowListEntriesName = VtnServiceJsonConsts.FLOWLISTENTRIES;
						final JsonArray flowListEntriesJsonArray = new JsonArray();
						LOG.debug("call getPomStatsFLowList : for statics information of flowList");
						index = getPomStatsFlowList(responsePacket, index,
								flowListEntriesJsonArray);
						flowListJson.add(flowListEntriesName,
								flowListEntriesJsonArray);
						vRouterflowFilterEntry.add(flowListName, flowListJson);

					} else {
						LOG.debug("Show ,Operation : normal and target db :state Skip flowList value strutures ");
						// increasing index to eliminate flow list entry
						// structures in case of show and op : normal
						index = responsePacket.length - 1;
					}
				}
				if (getType.equals(VtnServiceJsonConsts.LIST)
						&& opType.equalsIgnoreCase(VtnServiceJsonConsts.NORMAL)) {
					LOG.debug("List ,Operation : normal Skip value strutures ");
					index++;
				}
				// add current json object to array, if it has been initialized
				// earlier
				if (null != vRouterflowFilterEntryArray) {
					vRouterflowFilterEntryArray.add(vRouterflowFilterEntry);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != vRouterflowFilterEntryArray) {
				root.add(rootJsonName, vRouterflowFilterEntryArray);
			} else {
				root.add(rootJsonName, vRouterflowFilterEntry);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVRouterInterfaceFlowFilterEntryResponse");

		return root;
	}

	// getVTunnelResponse Response structure

	public JsonObject getVTunnelResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody, final String getType) {
		LOG.trace("Start getVTunnelResponse");
		final JsonObject root = new JsonObject();
		JsonArray vTunnelArray = null;
		LOG.debug("getType: " + getType);
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}

		/*
		 * data type will be required to resolve the response structures
		 */
		String dataType = VtnServiceJsonConsts.STATE;
		if (requestBody.has(VtnServiceJsonConsts.TARGETDB)) {
			dataType = requestBody.get(VtnServiceJsonConsts.TARGETDB)
					.getAsString();
		}

		String rootJsonName;
		/*
		 * get type (show or list) will be required to resolve root json name
		 * here it will be vRouter for show and vRouters for list
		 */
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.VTUNNEL;
		} else {
			rootJsonName = VtnServiceJsonConsts.VTUNNELS;
			// json array will be required for list type of cases
			vTunnelArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject vTunnel = null;

		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			vTunnel = new JsonObject();
			vTunnel.addProperty(
					VtnServiceJsonConsts.COUNT,
					IpcDataUnitWrapper
							.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, vTunnel);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {

				vTunnel = new JsonObject();
				byte validBit;

				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;

				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyVrtStruct = (IpcStruct) responsePacket[index++];
				vTunnel.addProperty(VtnServiceJsonConsts.VTUNNELNAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								keyVrtStruct, VtnServiceIpcConsts.VTUNNELNAME));
				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */
				if (getType.equals(VtnServiceJsonConsts.SHOW)
						|| opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
					LOG.debug("Case : Show or detail");
					/*
					 * add valid informations from value structure
					 */
					final IpcStruct valVtunnelStruct = (IpcStruct) responsePacket[index++];

					validBit = valVtunnelStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVtunnelIndex.UPLL_IDX_DESC_VTNL
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vTunnel,
								VtnServiceJsonConsts.DESCRIPTION,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVtunnelStruct,
										VtnServiceIpcConsts.DESCRIPTION));
					}
					validBit = valVtunnelStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVtunnelIndex.UPLL_IDX_CONTROLLER_ID_VTNL
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vTunnel,
								VtnServiceJsonConsts.CONTROLLERID,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVtunnelStruct,
										VtnServiceIpcConsts.CONTROLLERID));
					}
					validBit = valVtunnelStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVtunnelIndex.UPLL_IDX_VTN_NAME_VTNL
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vTunnel,
								VtnServiceJsonConsts.VTNNAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVtunnelStruct,
										VtnServiceIpcConsts.VTNNAME));
					}
					validBit = valVtunnelStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVtunnelIndex.UPLL_IDX_VTEP_GRP_NAME_VTNL
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vTunnel,
								VtnServiceJsonConsts.VTEPGROUPNAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVtunnelStruct,
										VtnServiceIpcConsts.VTEPGRPNAME));
					}
					validBit = valVtunnelStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVtunnelIndex.UPLL_IDX_LABEL_VTNL
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vTunnel,
								VtnServiceJsonConsts.LABEL,
								IpcDataUnitWrapper.getIpcStructUint32Value(
										valVtunnelStruct,
										VtnServiceIpcConsts.LABEL));
					}
					validBit = valVtunnelStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVtunnelIndex.UPLL_IDX_DOMAIN_ID_VTNL
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vTunnel,
								VtnServiceJsonConsts.DOMAINID,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVtunnelStruct,
										VtnServiceJsonConsts.DOMAINID));
					}
					/*
					 * If data type is set as "state", then value structure will
					 * also contain the state information
					 */
					if (dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
						LOG.debug("targetdb : State");
						final IpcStruct valVtunnelStStruct = (IpcStruct) responsePacket[index++];
						validBit = valVtunnelStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.ValVtunnelStIndex.UPLL_IDX_OPER_STATUS_VTNLS
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVtunnelStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_UP
											.getValue())) {
								setValueToJsonObject(validBit, vTunnel,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.UP);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVtunnelStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_DOWN
											.getValue())) {
								setValueToJsonObject(validBit, vTunnel,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.DOWN);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVtunnelStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_UNKNOWN
											.getValue())) {
								setValueToJsonObject(validBit, vTunnel,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.UNKNOWN);
							} else {
								LOG.debug("Type: Invalid value");
							}
							LOG.debug("OPERSTATUS:"
									+ IpcDataUnitWrapper
											.getIpcStructUint8Value(
													valVtunnelStStruct,
													VtnServiceIpcConsts.OPERSTATUS));
						}

					}
				} else {
					LOG.debug("Operation : normal Skip value struture");
					index++;
					if (dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
						LOG.debug("Operation : normal and target db :state Skip St value struture ");
						index++;
					}
				}
				// add current json object to array, if it has been initialized
				// earlier
				if (null != vTunnelArray) {
					vTunnelArray.add(vTunnel);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != vTunnelArray) {
				root.add(rootJsonName, vTunnelArray);
			} else {
				root.add(rootJsonName, vTunnel);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVTunnelResponse");
		return root;
	}

	public JsonObject getVLinkResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody, final String getType) {
		LOG.trace("Start getVLinkResponse");
		final JsonObject root = new JsonObject();
		JsonArray vLinkArray = null;
		LOG.debug("getType: " + getType);
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}

		/*
		 * data type will be required to resolve the response structures
		 */
		String dataType = VtnServiceJsonConsts.STATE;
		if (requestBody.has(VtnServiceJsonConsts.TARGETDB)) {
			dataType = requestBody.get(VtnServiceJsonConsts.TARGETDB)
					.getAsString();
		}

		String rootJsonName;
		/*
		 * get type (show or list) will be required to resolve root json name
		 * here it will be vRouter for show and vRouters for list
		 */
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.VLINK;
		} else {
			rootJsonName = VtnServiceJsonConsts.VLINKS;
			// json array will be required for list type of cases
			vLinkArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject vLink = null;

		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			vLink = new JsonObject();
			vLink.addProperty(
					VtnServiceJsonConsts.COUNT,
					IpcDataUnitWrapper
							.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, vLink);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {

				vLink = new JsonObject();
				byte validBit;

				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;

				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyVrtStruct = (IpcStruct) responsePacket[index++];
				vLink.addProperty(VtnServiceJsonConsts.VLINKNAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								keyVrtStruct, VtnServiceIpcConsts.VLINK_NAME));
				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */
				if (getType.equals(VtnServiceJsonConsts.SHOW)
						|| opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
					LOG.debug("Case : Show or detail");
					/*
					 * add valid informations from value structure
					 */
					final IpcStruct valVLinkStruct = (IpcStruct) responsePacket[index++];

					validBit = valVLinkStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_ADMIN_STATUS_VLNK
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valVLinkStruct,
										VtnServiceJsonConsts.ADMIN_STATUS)
								.equalsIgnoreCase(
										UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_ENABLE
												.getValue())) {
							setValueToJsonObject(validBit, vLink,
									VtnServiceJsonConsts.ADMINSTATUS,
									VtnServiceJsonConsts.ENABLE);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valVLinkStruct,
										VtnServiceJsonConsts.ADMIN_STATUS)
								.equalsIgnoreCase(
										UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_DISABLE
												.getValue())) {
							setValueToJsonObject(validBit, vLink,
									VtnServiceJsonConsts.ADMINSTATUS,
									VtnServiceJsonConsts.DISABLE);
						} else {
							LOG.debug("Admin Status: Invalid value");
						}
						LOG.debug("Admin Status :"
								+ IpcDataUnitWrapper.getIpcStructUint8Value(
										valVLinkStruct,
										VtnServiceJsonConsts.ADMIN_STATUS));
					}

					validBit = valVLinkStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_VNODE1_NAME_VLNK
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vLink,
								VtnServiceJsonConsts.VNODE1NAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVLinkStruct,
										VtnServiceJsonConsts.VNODE1NAME));
					}
					validBit = valVLinkStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_VNODE1_IF_NAME_VLNK
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vLink,
								VtnServiceJsonConsts.IF1NAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVLinkStruct,
										VtnServiceIpcConsts.VNODE1IFNAME));
					}
					validBit = valVLinkStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_VNODE2_NAME_VLNK
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vLink,
								VtnServiceJsonConsts.VNODE2NAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVLinkStruct,
										VtnServiceJsonConsts.VNODE2NAME));
					}
					validBit = valVLinkStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_VNODE2_IF_NAME_VLNK
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vLink,
								VtnServiceJsonConsts.IF2NAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVLinkStruct,
										VtnServiceIpcConsts.VNODE2IFNAME));
					}
					validBit = valVLinkStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_DESCRIPTION_VLNK
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vLink,
								VtnServiceJsonConsts.DESCRIPTION,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVLinkStruct,
										VtnServiceIpcConsts.DESCRIPTION));
					}
					// boundaryMap detail added
					final String boundaryMap = VtnServiceJsonConsts.BOUNDARYMAP;
					final JsonObject boundaryMapJson = new JsonObject();

					validBit = valVLinkStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_BOUNDARY_NAME_VLNK
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, boundaryMapJson,
								VtnServiceJsonConsts.BOUNDARYID,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVLinkStruct,
										VtnServiceIpcConsts.BOUNDARY_NAME));
					}

					// get label_type
					validBit = valVLinkStruct.getByte(
							VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_LABEL_TYPE_VLNK.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
						if (ValLabelType.UPLL_LABEL_TYPE_VLAN.getValue().equals(
								IpcDataUnitWrapper.getIpcStructUint8Value(
										valVLinkStruct,
										VtnServiceIpcConsts.LABEL_TYPE))) {

							// get label
							validBit = valVLinkStruct.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_LABEL_VLINK
											.ordinal());
							if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
									.ordinal()
									&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
								if (VtnServiceJsonConsts.VAL_FFFF == Long.parseLong(IpcDataUnitWrapper
										.getIpcStructUint32Value(valVLinkStruct,
												VtnServiceIpcConsts.LABEL))) {
									setValueToJsonObject(validBit, boundaryMapJson,
											VtnServiceJsonConsts.NO_VLAN_ID,
											VtnServiceJsonConsts.TRUE);
								} else {
									setValueToJsonObject(validBit, boundaryMapJson,
											VtnServiceJsonConsts.VLANID,
											IpcDataUnitWrapper.getIpcStructUint32Value(
													valVLinkStruct,
													VtnServiceIpcConsts.LABEL));
								}
							}
						}
					}

					/*
					 * If data type is set as "state", then value structure will
					 * also contain the state information
					 */
					if (dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
						LOG.debug("targetdb : State");
						final IpcStruct valVLinkStStruct = (IpcStruct) responsePacket[index++];
						validBit = valVLinkStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.ValVlinkStIndex.UPLL_IDX_OPER_STATUS_VLNKS
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVLinkStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_UP
											.getValue())) {
								setValueToJsonObject(validBit, vLink,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.UP);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVLinkStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_DOWN
											.getValue())) {
								setValueToJsonObject(validBit, vLink,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.DOWN);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVLinkStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_UNKNOWN
											.getValue())) {
								setValueToJsonObject(validBit, vLink,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.UNKNOWN);
							} else {
								LOG.debug("Operstatus: Invalid value");
							}
							LOG.debug("Operstatus :"
									+ IpcDataUnitWrapper
											.getIpcStructUint8Value(
													valVLinkStStruct,
													VtnServiceIpcConsts.OPERSTATUS));
						}
					}
					// added in vlink- boundary MAp
					vLink.add(boundaryMap, boundaryMapJson);
				} else {
					LOG.debug("Operation : normal Skip value struture");
					index++;
					if (dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
						LOG.debug("Operation : normal and target db :state Skip St value struture ");
						index++;
					}
				}
				// add current json object to array, if it has been initialized
				// earlier
				if (null != vLinkArray) {
					vLinkArray.add(vLink);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != vLinkArray) {
				root.add(rootJsonName, vLinkArray);
			} else {
				root.add(rootJsonName, vLink);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVLinkResponse");

		return root;
	}

	public JsonObject getVBridgeFlowFilterResourceResponse(
			final IpcDataUnit[] responsePacket, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start getVBridgeFlowFilterResourceResponse");
		final JsonObject root = new JsonObject();

		final String rootJsonName = VtnServiceJsonConsts.FLOWFILTER;
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject flowFilter = null;
		for (int index = 0; index < responsePacket.length; index++) {

			flowFilter = new JsonObject();

			// There is no use of key type
			LOG.debug("Skip key type: no use");
			index++;
			/*
			 * add mandatory informations from key structure
			 */
			final IpcStruct keyVtnFlowFilter = (IpcStruct) responsePacket[index++];
			if (IpcDataUnitWrapper
					.getIpcStructUint8Value(keyVtnFlowFilter,
							VtnServiceIpcConsts.DIRECTION)
					.equals(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_IN
							.getValue())) {
				flowFilter.addProperty(VtnServiceJsonConsts.FFTYPE,
						VtnServiceJsonConsts.IN);
				LOG.debug("FF Type :"
						+ IpcDataUnitWrapper
								.getIpcStructUint8Value(keyVtnFlowFilter,
										VtnServiceIpcConsts.DIRECTION));
			} else if (IpcDataUnitWrapper
					.getIpcStructUint8Value(keyVtnFlowFilter,
							VtnServiceIpcConsts.DIRECTION)
					.equals(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_OUT
							.getValue())) {
				flowFilter.addProperty(VtnServiceJsonConsts.FFTYPE,
						VtnServiceJsonConsts.OUT);
				LOG.debug("FF Type :"
						+ IpcDataUnitWrapper
								.getIpcStructUint8Value(keyVtnFlowFilter,
										VtnServiceIpcConsts.DIRECTION));
			} else {
				LOG.debug("Invalid value for FFTYPE parameter");
			}
		}
		/*
		 * finally add single object to root json object and return the same.
		 */
		root.add(rootJsonName, flowFilter);
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVBridgeFlowFilterResourceResponse");
		return root;
	}

	public JsonObject getVRouterInterfaceResponse(
			final IpcDataUnit[] responsePacketIf, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start getVRouterInterfaceResponse");
		final JsonObject root = new JsonObject();
		JsonArray vrtInterfacesArray = null;
		LOG.debug("getType: " + getType);
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}

		/*
		 * data type will be required to resolve the response structures
		 */
		String dataType = VtnServiceJsonConsts.STATE;
		if (requestBody.has(VtnServiceJsonConsts.TARGETDB)) {
			dataType = requestBody.get(VtnServiceJsonConsts.TARGETDB)
					.getAsString();
		}

		String rootJsonName;

		/*
		 * get type (show or list) will be required to resolve root json name
		 * here it will be vRouter for show and vRouters for list
		 */
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.INTERFACE;
		} else {
			rootJsonName = VtnServiceJsonConsts.INTERFACES;
			// json array will be required for list type of cases
			vrtInterfacesArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject vrtInterface = null;

		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			vrtInterface = new JsonObject();
			vrtInterface
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacketIf[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, vrtInterface);
		} else {

			for (int index = 0; index < responsePacketIf.length; index++) {

				vrtInterface = new JsonObject();
				byte validBit;

				// There is no use of key type so skipped it.
				LOG.debug("Skip key type: no use");
				index++;

				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyVrtIfStruct = (IpcStruct) responsePacketIf[index++];
				vrtInterface.addProperty(VtnServiceJsonConsts.IFNAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								keyVrtIfStruct, VtnServiceIpcConsts.IFNAME));

				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */
				if (getType.equals(VtnServiceJsonConsts.SHOW)
						|| opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
					LOG.debug("Case : Show or detail");
					/*
					 * add valid informations from value structure
					 */
					final IpcStruct valVrtIfStruct = (IpcStruct) responsePacketIf[index++];

					validBit = valVrtIfStruct.getByte(
							VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVrtIfIndex.UPLL_IDX_DESC_VI
									.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vrtInterface,
								VtnServiceJsonConsts.DESCRIPTION,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVrtIfStruct,
										VtnServiceJsonConsts.DESCRIPTION));
					}
					validBit = valVrtIfStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVrtIfIndex.UPLL_IDX_ADMIN_ST_VI
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valVrtIfStruct,
										VtnServiceIpcConsts.ADMIN_STATUS)
								.equals(UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_ENABLE
										.getValue())) {
							setValueToJsonObject(validBit, vrtInterface,
									VtnServiceJsonConsts.ADMINSTATUS,
									VtnServiceJsonConsts.ENABLE);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valVrtIfStruct,
										VtnServiceIpcConsts.ADMIN_STATUS)
								.equals(UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_DISABLE
										.getValue())) {
							setValueToJsonObject(validBit, vrtInterface,
									VtnServiceJsonConsts.ADMINSTATUS,
									VtnServiceJsonConsts.DISABLE);
						} else {
							LOG.debug("Adminstatus : Invalid");
						}
						LOG.debug("Adminstatus :"
								+ IpcDataUnitWrapper.getIpcStructUint8Value(
										valVrtIfStruct,
										VtnServiceIpcConsts.ADMIN_STATUS));
					}

					validBit = valVrtIfStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVrtIfIndex.UPLL_IDX_IP_ADDR_VI
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vrtInterface,
								VtnServiceJsonConsts.IPADDR,
								IpcDataUnitWrapper.getIpcStructIpv4Value(
										valVrtIfStruct,
										VtnServiceIpcConsts.IP_ADDR));
					}
					validBit = valVrtIfStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVrtIfIndex.UPLL_IDX_PREFIXLEN_VI
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vrtInterface,
								VtnServiceJsonConsts.PREFIX,
								IpcDataUnitWrapper.getIpcStructUint8Value(
										valVrtIfStruct,
										VtnServiceIpcConsts.PREFIXLEN));
					}
					validBit = valVrtIfStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVrtIfIndex.UPLL_IDX_MAC_ADDR_VI
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vrtInterface,
								VtnServiceJsonConsts.MACADDR,
								IpcDataUnitWrapper.getMacAddress(
										valVrtIfStruct,
										VtnServiceIpcConsts.MACADDR));
					}
					if (dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
						LOG.debug("targetdb : State");
						final IpcStruct valVrtIfStStruct = (IpcStruct) responsePacketIf[index++];
						validBit = valVrtIfStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.ValVrtIfStIndex.UPLL_IDX_OPER_STATUS_VRTIS
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVrtIfStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_UP
											.getValue())) {
								setValueToJsonObject(validBit, vrtInterface,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.UP);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVrtIfStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_DOWN
											.getValue())) {
								setValueToJsonObject(validBit, vrtInterface,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.DOWN);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVrtIfStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_UNKNOWN
											.getValue())) {
								setValueToJsonObject(validBit, vrtInterface,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.UNKNOWN);
							} else {
								LOG.debug("Operstatus : Invalid");
							}
							LOG.debug("operstatus :"
									+ IpcDataUnitWrapper
											.getIpcStructUint8Value(
													valVrtIfStStruct,
													VtnServiceIpcConsts.OPERSTATUS));
						}

					}

				} else {
					LOG.debug("Operation : normal Skip value struture");
					index++;
					if (dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
						LOG.debug("Operation : normal and target db :state Skip St value struture ");
						index++;
					}
				}

				// add current json object to array, if it has been
				// initialized
				// earlier
				if (null != vrtInterfacesArray) {
					vrtInterfacesArray.add(vrtInterface);
				}

			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != vrtInterfacesArray) {
				root.add(rootJsonName, vrtInterfacesArray);
			} else {
				root.add(rootJsonName, vrtInterface);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVRouterInterfaceResponse");

		return root;
	}

	public JsonObject getVTepInterfaceResponse(
			final IpcDataUnit[] responsePacketInterface,
			final JsonObject requestBody, final String getType) {
		LOG.trace("Start getVTepInterfaceResponse");
		final JsonObject root = new JsonObject();
		JsonArray vtepInterfaceArray = null;
		LOG.debug("getType: " + getType);
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}

		/*
		 * data type will be required to resolve the response structures
		 */
		String dataType = VtnServiceJsonConsts.STATE;
		if (requestBody.has(VtnServiceJsonConsts.TARGETDB)) {
			dataType = requestBody.get(VtnServiceJsonConsts.TARGETDB)
					.getAsString();
		}

		String rootJsonName;
		/*
		 * get type (show or list) will be required to resolve root json name
		 * here it will be vRouter for show and vRouters for list
		 */
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.INTERFACE;
		} else {
			rootJsonName = VtnServiceJsonConsts.INTERFACES;
			// json array will be required for list type of cases
			vtepInterfaceArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject vtepInterface = null;

		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			vtepInterface = new JsonObject();
			vtepInterface
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacketInterface[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, vtepInterface);
		} else {
			for (int index = 0; index < responsePacketInterface.length; index++) {

				vtepInterface = new JsonObject();
				byte validBit;

				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;
				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyVtepIfStruct = (IpcStruct) responsePacketInterface[index++];
				vtepInterface.addProperty(VtnServiceJsonConsts.IFNAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								keyVtepIfStruct, VtnServiceIpcConsts.IFNAME));

				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */
				if (opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)
						|| getType.equals(VtnServiceJsonConsts.SHOW)) {
					LOG.debug("Case : Show or detail");
					/*
					 * add valid informations from value structure
					 */

					final IpcStruct valVtepIfStruct = (IpcStruct) responsePacketInterface[index++];

					validBit = valVtepIfStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVtepIfIndex.UPLL_IDX_DESC_VTEPI
											.ordinal());

					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vtepInterface,
								VtnServiceJsonConsts.DESCRIPTION,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVtepIfStruct,
										VtnServiceJsonConsts.DESCRIPTION));
					}

					validBit = valVtepIfStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVtepIfIndex.UPLL_IDX_ADMIN_ST_VTEPI
											.ordinal());

					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {

						if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valVtepIfStruct,
										VtnServiceIpcConsts.ADMIN_STATUS)
								.equalsIgnoreCase(
										UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_ENABLE
												.getValue())) {
							setValueToJsonObject(validBit, vtepInterface,
									VtnServiceJsonConsts.ADMINSTATUS,
									VtnServiceJsonConsts.ENABLE);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valVtepIfStruct,
										VtnServiceIpcConsts.ADMIN_STATUS)
								.equalsIgnoreCase(
										UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_DISABLE
												.getValue())) {
							setValueToJsonObject(validBit, vtepInterface,
									VtnServiceJsonConsts.ADMINSTATUS,
									VtnServiceJsonConsts.DISABLE);
						} else {
							LOG.debug("Adminstatus : Invalid value");
						}
						LOG.debug("Adminstatus :"
								+ IpcDataUnitWrapper.getIpcStructUint8Value(
										valVtepIfStruct,
										VtnServiceIpcConsts.ADMIN_STATUS));
					}
					if (dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
						LOG.debug("targetdb : State");
						final IpcStruct valVtepIfStStruct = (IpcStruct) responsePacketInterface[index++];

						validBit = valVtepIfStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.ValVtepIfStIndex.UPLL_IDX_IF_OPER_STATUS_VTEPIS
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {

							if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVtepIfStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equalsIgnoreCase(
											UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_UP
													.getValue())) {
								setValueToJsonObject(validBit, vtepInterface,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.UP);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVtepIfStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equalsIgnoreCase(
											UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_DOWN
													.getValue())) {
								setValueToJsonObject(validBit, vtepInterface,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.DOWN);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVtepIfStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equalsIgnoreCase(
											UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_UNKNOWN
													.getValue())) {
								setValueToJsonObject(validBit, vtepInterface,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.UNKNOWN);
							} else {
								LOG.debug("Operstatus : Invalid value");
							}
							LOG.debug("Operstatus :"
									+ IpcDataUnitWrapper
											.getIpcStructUint8Value(
													valVtepIfStStruct,
													VtnServiceIpcConsts.OPERSTATUS));
						}

					}

				} else {
					LOG.debug("Operation : normal Skip value struture");
					index++;
					if (dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
						LOG.debug("Operation : normal and target db :state Skip St value struture ");
						index++;
					}
				}
				// add current json object to array, if it has been initialized
				// earlier
				if (null != vtepInterfaceArray) {
					vtepInterfaceArray.add(vtepInterface);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != vtepInterfaceArray) {
				root.add(rootJsonName, vtepInterfaceArray);
			} else {
				root.add(rootJsonName, vtepInterface);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVTepInterfaceResponse");

		return root;
	}

	public JsonObject getVBypassInterfaceResponse(
			final IpcDataUnit[] responsePacketIf, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start getVBypassInterfaceResponse");
		final JsonObject root = new JsonObject();
		JsonArray vBypassInterfacesArray = null;
		LOG.debug("getType: " + getType);
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}

		/*
		 * data type will be required to resolve the response structures
		 */

		String rootJsonName;

		/*
		 * get type (show or list) will be required to resolve root json name
		 * here it will be vRouter for show and vRouters for list
		 */
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.INTERFACE;
		} else {
			rootJsonName = VtnServiceJsonConsts.INTERFACES;
			// json array will be required for list type of cases
			vBypassInterfacesArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject vBypassInterface = null;

		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			vBypassInterface = new JsonObject();
			vBypassInterface
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacketIf[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, vBypassInterface);
		} else {

			for (int index = 0; index < responsePacketIf.length; index++) {

				vBypassInterface = new JsonObject();
				byte validBit;

				// There is no use of key type so skipped it.
				LOG.debug("Skip key type: no use");
				index++;

				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyVukIfStruct = (IpcStruct) responsePacketIf[index++];
				vBypassInterface.addProperty(VtnServiceJsonConsts.IFNAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								keyVukIfStruct, VtnServiceIpcConsts.IFNAME));

				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */
				if (getType.equals(VtnServiceJsonConsts.SHOW)
						|| opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
					LOG.debug("Case : Show or detail");
					/*
					 * add valid informations from value structure
					 */
					final IpcStruct valVukIfStruct = (IpcStruct) responsePacketIf[index++];

					validBit = valVukIfStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVunkIfIndex.UPLL_IDX_DESC_VUNI
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vBypassInterface,
								VtnServiceJsonConsts.DESCRIPTION,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVukIfStruct,
										VtnServiceJsonConsts.DESCRIPTION));
					}
					validBit = valVukIfStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVunkIfIndex.UPLL_IDX_ADMIN_ST_VUNI
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valVukIfStruct,
										VtnServiceIpcConsts.ADMIN_STATUS)
								.equals(UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_ENABLE
										.getValue())) {
							setValueToJsonObject(validBit, vBypassInterface,
									VtnServiceJsonConsts.ADMINSTATUS,
									VtnServiceJsonConsts.ENABLE);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valVukIfStruct,
										VtnServiceIpcConsts.ADMIN_STATUS)
								.equals(UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_DISABLE
										.getValue())) {
							setValueToJsonObject(validBit, vBypassInterface,
									VtnServiceJsonConsts.ADMINSTATUS,
									VtnServiceJsonConsts.DISABLE);
						} else {
							LOG.debug("Adminstatus : Invalid value");
						}
						LOG.debug("Adminstatus :"
								+ IpcDataUnitWrapper.getIpcStructUint8Value(
										valVukIfStruct,
										VtnServiceIpcConsts.ADMIN_STATUS));
					}
				} else {
					LOG.debug("Operation : normal Skip value struture");
					index++;
				}

				// add current json object to array, if it has been initialized
				// earlier
				if (null != vBypassInterfacesArray) {
					vBypassInterfacesArray.add(vBypassInterface);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != vBypassInterfacesArray) {
				root.add(rootJsonName, vBypassInterfacesArray);
			} else {
				root.add(rootJsonName, vBypassInterface);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVBypassInterfaceResponse");

		return root;
	}

	public JsonObject getVTepGroupResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody, final String getType) {
		LOG.trace("Start getVTepGroupResponse");
		JsonObject vTepGroup = null;
		JsonArray vTepGroupArray = null;
		LOG.debug("getType: " + getType);
		byte validBit;
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}

		String rootJsonName = VtnServiceJsonConsts.VTEPGROUP;
		final JsonObject root = new JsonObject();

		if (!getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.VTEPGROUPS;
			// json array will be required for list type of cases
			vTepGroupArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			vTepGroup = new JsonObject();
			vTepGroup
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, vTepGroup);
		} else {
			IpcStruct keyVTepGroupStruct = null;
			IpcStruct valVTepGroupStruct = null;
			for (int index = 0; index < responsePacket.length; index++) {
				vTepGroup = new JsonObject();
				// There is no use of key type so skipping it
				LOG.debug("Skip key type: no use");
				index++;

				keyVTepGroupStruct = (IpcStruct) responsePacket[index++];
				valVTepGroupStruct = (IpcStruct) responsePacket[index++];

				vTepGroup.addProperty(VtnServiceJsonConsts.VTEPGROUPNAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								keyVTepGroupStruct,
								VtnServiceIpcConsts.VTEPGRP_NAME));

				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */

				if (VtnServiceJsonConsts.SHOW.equals(getType)
						|| VtnServiceJsonConsts.DETAIL.equalsIgnoreCase(opType)) {
					LOG.debug("Case : Show or detail");
					validBit = valVTepGroupStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.val_vtep_grp_index.UPLL_IDX_CONTROLLER_ID_VTEPG
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vTepGroup,
								VtnServiceJsonConsts.CONTROLLERID,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVTepGroupStruct,
										VtnServiceJsonConsts.CONTROLLERID));
					}
					validBit = valVTepGroupStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.val_vtep_grp_index.UPLL_IDX_DESCRIPTION_VTEPG
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vTepGroup,
								VtnServiceJsonConsts.DESCRIPTION,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVTepGroupStruct,
										VtnServiceJsonConsts.DESCRIPTION));
					}

				}
				// add current json object to array, if it has been initialized
				// earlier
				if (null != vTepGroupArray) {
					vTepGroupArray.add(vTepGroup);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != vTepGroupArray) {
				root.add(rootJsonName, vTepGroupArray);
			} else {
				root.add(rootJsonName, vTepGroup);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVTepGroupResponse");

		return root;
	}

	public JsonObject getVtepGroupMembers(final IpcDataUnit[] responsePacket,
			final JsonObject vTepGroup) {
		LOG.trace("Start getVtepGroupMembers");
		final JsonArray membersArray = new JsonArray();
		JsonObject member = null;

		final String memberJsonName = VtnServiceJsonConsts.MEMBERVTEPS;
		for (int index = 0; index < responsePacket.length; index++) {

			member = new JsonObject();
			// There is no use of key type
			LOG.debug("Skip key type: no use");
			index++;

			/*
			 * add mandatory informations from key structure
			 */
			final IpcStruct keyVTepGroupMemberStruct = (IpcStruct) responsePacket[index++];
			member.addProperty(VtnServiceJsonConsts.VTEPNAME,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							keyVTepGroupMemberStruct,
							VtnServiceJsonConsts.VTEPMEMBERNAME));
			index++;
			if (vTepGroup.has(VtnServiceJsonConsts.MEMBERVTEPS)
					&& null != vTepGroup.get(VtnServiceJsonConsts.MEMBERVTEPS)) {
				vTepGroup.getAsJsonArray(VtnServiceJsonConsts.MEMBERVTEPS).add(
						member);
			}
			membersArray.add(member);
		}
		LOG.debug("member Json: " + membersArray);
		if (!vTepGroup.has(VtnServiceJsonConsts.MEMBERVTEPS)) {
			vTepGroup.add(memberJsonName, membersArray);
		}

		LOG.debug("response Json: " + vTepGroup.toString());
		LOG.trace("Complete getVtepGroupMembers");

		return vTepGroup;
	}

	/**
	 * Function to create VBridgeInterface Response (Show / List)
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return
	 */
	public JsonObject getVBridgeInterfaceResponse(
			final IpcDataUnit[] responsePacket, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start getVBridgeInterfaceResponse");
		final JsonObject root = new JsonObject();
		JsonArray vbrInterfaceList = null;
		LOG.debug("getType: " + getType);
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}
		/*
		 * data type will be required to resolve the response structures
		 */
		String dataType = VtnServiceJsonConsts.STATE;
		if (requestBody.has(VtnServiceJsonConsts.TARGETDB)) {
			dataType = requestBody.get(VtnServiceJsonConsts.TARGETDB)
					.getAsString();
		}
		String rootJsonName = null;
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.INTERFACE;
		} else {
			rootJsonName = VtnServiceJsonConsts.INTERFACES;
			// json array will be required for list type of cases
			vbrInterfaceList = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject vbrInterface = null;
		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			vbrInterface = new JsonObject();
			vbrInterface
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, vbrInterface);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {
				vbrInterface = new JsonObject();
				byte validBit;
				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;
				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyVbrInterfaceStruct = (IpcStruct) responsePacket[index++];
				vbrInterface.addProperty(VtnServiceJsonConsts.IFNAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								keyVbrInterfaceStruct,
								VtnServiceIpcConsts.IFNAME));
				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */
				if (getType.equals(VtnServiceJsonConsts.SHOW)
						|| opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
					LOG.debug("Case : Show or detail");
					/*
					 * add valid informations from value structure
					 */
					final IpcStruct valVbrInterfaceStruct = (IpcStruct) responsePacket[index++];

					validBit = valVbrInterfaceStruct.getByte(
							VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVbrIfIndex.UPLL_IDX_DESC_VBRI
									.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vbrInterface,
								VtnServiceJsonConsts.DESCRIPTION,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVbrInterfaceStruct,
										VtnServiceIpcConsts.DESCRIPTION));
					}
					validBit = valVbrInterfaceStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVbrIfIndex.UPLL_IDX_ADMIN_STATUS_VBRI
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valVbrInterfaceStruct,
										VtnServiceIpcConsts.ADMIN_STATUS)
								.equals(UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_ENABLE
										.getValue())) {
							setValueToJsonObject(validBit, vbrInterface,
									VtnServiceJsonConsts.ADMINSTATUS,
									VtnServiceJsonConsts.ENABLE);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valVbrInterfaceStruct,
										VtnServiceIpcConsts.ADMIN_STATUS)
								.equals(UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_DISABLE
										.getValue())) {
							setValueToJsonObject(validBit, vbrInterface,
									VtnServiceJsonConsts.ADMINSTATUS,
									VtnServiceJsonConsts.DISABLE);
						} else {
							LOG.debug("Adminstatus : Invalid");
						}
						LOG.debug("Adminstatus :"
								+ IpcDataUnitWrapper.getIpcStructUint8Value(
										valVbrInterfaceStruct,
										VtnServiceIpcConsts.ADMIN_STATUS));
					}
					if (VtnServiceJsonConsts.STATE.equals(dataType)) {
						LOG.debug("targetdb : State");
						final IpcStruct valVbrIfStStruct = (IpcStruct) responsePacket[index++];
						validBit = valVbrIfStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.ValVrtIfStIndex.UPLL_IDX_OPER_STATUS_VRTIS
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVbrIfStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_UP
											.getValue())) {
								setValueToJsonObject(validBit, vbrInterface,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.UP);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVbrIfStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_DOWN
											.getValue())) {
								setValueToJsonObject(validBit, vbrInterface,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.DOWN);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVbrIfStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_UNKNOWN
											.getValue())) {
								setValueToJsonObject(validBit, vbrInterface,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.UNKNOWN);
							} else {
								LOG.debug("Operstatus : Invalid");
							}
							LOG.debug("Operstatus :"
									+ IpcDataUnitWrapper
											.getIpcStructUint8Value(
													valVbrIfStStruct,
													VtnServiceIpcConsts.OPERSTATUS));
						}
					}
				} else {
					LOG.debug("Operation : normal Skip value struture");
					index++;
					if (dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
						LOG.debug("Operation : normal and target db :state Skip St value struture ");
						index++;
					}
				}
				// add current json object to array, if it has been
				// initialized
				// earlier
				if (null != vbrInterfaceList) {
					vbrInterfaceList.add(vbrInterface);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != vbrInterfaceList) {
				root.add(rootJsonName, vbrInterfaceList);
			} else {
				root.add(rootJsonName, vbrInterface);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVBridgeInterfaceResponse");

		return root;
	}

	public JsonObject getNeighborResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody, final String getType) {
		LOG.trace("Start getNeighborResponse");
		/*
		 * here if the type is og List type then vbridge Interface List array
		 * object gets initialized
		 */

		JsonObject neighbor = null;

		int index = 0;

		neighbor = new JsonObject();
		byte validBit;
		if (responsePacket != null && responsePacket.length > 0) {
			// There is no use of key type
			LOG.debug("Skip key type: no use");
			index++;
			// no use of key value
			LOG.debug("Skip key Structure: no use");
			index++;

			final IpcStruct valVtnNeighborStruct = (IpcStruct) responsePacket[index++];
			validBit = valVtnNeighborStruct
					.getByte(
							VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVtnNeighborIndex.UPLL_IDX_CONN_VNODE_NAME_VN
									.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, neighbor,
						VtnServiceJsonConsts.VNODENAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valVtnNeighborStruct,
								VtnServiceIpcConsts.CONNECTED_VNODE_NAME));
			}
			validBit = valVtnNeighborStruct
					.getByte(
							VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVtnNeighborIndex.UPLL_IDX_CONN_VNODE_IF_NAME_VN
									.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, neighbor,
						VtnServiceJsonConsts.IFNAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valVtnNeighborStruct,
								VtnServiceIpcConsts.CONNECTED_IF_NAME));
			}
			validBit = valVtnNeighborStruct
					.getByte(
							VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVtnNeighborIndex.UPLL_IDX_CONN_VLINK_NAME_VN
									.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, neighbor,
						VtnServiceJsonConsts.VLINKNAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valVtnNeighborStruct,
								VtnServiceIpcConsts.CONNECTED_VLINK_NAME));
			}
		}
		LOG.debug(" Neighbor response Json:" + neighbor.toString());
		LOG.trace("Complete getNeighborResponse");

		return neighbor;
	}

	/**
	 * Function to create VBridge Response (Show / List)
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return
	 */
	public JsonObject getVBridgeResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody, final String getType) {
		LOG.trace("Start getVBridgeResponse");
		final JsonObject root = new JsonObject();
		JsonArray vBridgesArray = null;
		LOG.debug("getType: " + getType);
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}
		/*
		 * data type will be required to resolve the response structures
		 */
		String dataType = VtnServiceJsonConsts.STATE;
		if (requestBody.has(VtnServiceJsonConsts.TARGETDB)) {
			dataType = requestBody.get(VtnServiceJsonConsts.TARGETDB)
					.getAsString();
		}
		String rootJsonName;
		/*
		 * get type (show or list) will be required to resolve root json name
		 * here it will be vBridge for show and vBridges for list
		 */
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.VBRIDGE;
		} else {
			rootJsonName = VtnServiceJsonConsts.VBRIDGES;
			// json array will be required for list type of cases
			vBridgesArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject vBridge = null;
		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			vBridge = new JsonObject();
			vBridge.addProperty(
					VtnServiceJsonConsts.COUNT,
					IpcDataUnitWrapper
							.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, vBridge);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {
				vBridge = new JsonObject();
				byte validBit;
				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;
				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyVbrStruct = (IpcStruct) responsePacket[index++];
				vBridge.addProperty(VtnServiceJsonConsts.VBRIDGENAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								keyVbrStruct, VtnServiceIpcConsts.VBRIDGE_NAME));
				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */
				if (getType.equals(VtnServiceJsonConsts.SHOW)
						|| opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
					LOG.debug("Case : Show or detail");
					/*
					 * add valid informations from value structure
					 */
					final IpcStruct valVbrStruct = (IpcStruct) responsePacket[index++];
					validBit = valVbrStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVbrIndex.UPLL_IDX_CONTROLLER_ID_VBR
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vBridge,
								VtnServiceJsonConsts.CONTROLLERID,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVbrStruct,
										VtnServiceJsonConsts.CONTROLLERID));
					}
					validBit = valVbrStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVbrIndex.UPLL_IDX_DOMAIN_ID_VBR
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vBridge,
								VtnServiceJsonConsts.DOMAINID,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVbrStruct,
										VtnServiceJsonConsts.DOMAINID));
					}
					validBit = valVbrStruct.getByte(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVbrIndex.UPLL_IDX_DESC_VBR
									.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vBridge,
								VtnServiceJsonConsts.DESCRIPTION,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVbrStruct,
										VtnServiceIpcConsts.VBRDESCRIPTION));
					}
					/*
					 * If data type is set as "state", then value structure will
					 * also contain the state information
					 */
					if (dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
						LOG.debug("targetdb : State");
						final IpcStruct valVbrStStruct = (IpcStruct) responsePacket[index++];
						validBit = valVbrStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.ValVbrStIndex.UPLL_IDX_OPER_STATUS_VBRS
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVbrStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_UP
											.getValue())) {
								setValueToJsonObject(validBit, vBridge,
										VtnServiceJsonConsts.STATUS,
										VtnServiceJsonConsts.UP);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVbrStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_DOWN
											.getValue())) {
								setValueToJsonObject(validBit, vBridge,
										VtnServiceJsonConsts.STATUS,
										VtnServiceJsonConsts.DOWN);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVbrStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_UNKNOWN
											.getValue())) {
								setValueToJsonObject(validBit, vBridge,
										VtnServiceJsonConsts.STATUS,
										VtnServiceJsonConsts.UNKNOWN);
							} else {
								LOG.debug("Status : Invalid");
							}
							LOG.debug("Status :"
									+ IpcDataUnitWrapper
											.getIpcStructUint8Value(
													valVbrStStruct,
													VtnServiceIpcConsts.OPERSTATUS));
						}
					}
				} else {
					LOG.debug("Operation : normal Skip value struture");
					index++;
					if (dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
						LOG.debug("Operation : normal and target db :state Skip St value struture ");
						index++;
					}
				}
				// add current json object to array, if it has been initialized
				// earlier
				if (null != vBridgesArray) {
					vBridgesArray.add(vBridge);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != vBridgesArray) {
				root.add(rootJsonName, vBridgesArray);
			} else {
				root.add(rootJsonName, vBridge);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVBridgeResponse");

		return root;
	}

	public JsonObject getPortMapResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody, final String getType,
			final String ifType) {
		LOG.trace("Start getPortMapResponse");
		final JsonObject root = new JsonObject();

		final String rootJsonName = VtnServiceJsonConsts.PORTMAP;
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject portMap = null;

		String dataType = VtnServiceJsonConsts.STATE;
		if (requestBody.has(VtnServiceJsonConsts.TARGETDB)) {
			dataType = requestBody.get(VtnServiceJsonConsts.TARGETDB)
					.getAsString();
		}

		for (int index = 0; index < responsePacket.length; index++) {

			portMap = new JsonObject();
			byte validBit = 0;
			// There is no use of key type
			LOG.debug("Skip key type: no use");
			index++;

			/*
			 * There is no use of key structure
			 */
			LOG.debug("Skip key Structure: no use");
			index++;
			/*
			 * add valid informations from value structure
			 */
			final IpcStruct valIfStruct = (IpcStruct) responsePacket[index++];
			if (ifType != null && !ifType.isEmpty()) {
				if (ifType
						.equalsIgnoreCase(VtnServiceJsonConsts.VBRIDGE_INTERFACE_PORTMAP)) {
					validBit = valIfStruct.getByte(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVbrIfIndex.UPLL_IDX_PM_VBRI
									.ordinal());
				} else if (ifType
						.equalsIgnoreCase(VtnServiceJsonConsts.VTEP_INTERFACE_PORTMAP)) {
					validBit = valIfStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVtepIfIndex.UPLL_IDX_PORT_MAP_VTEPI
											.ordinal());
				} else if (ifType
						.equalsIgnoreCase(VtnServiceJsonConsts.VTUNNEL_INTERFACE_PORTMAP)) {
					validBit = valIfStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVtunnelIfIndex.UPLL_IDX_PORT_MAP_VTNL_IF
											.ordinal());
				} else if (ifType
						.equalsIgnoreCase(VtnServiceJsonConsts.VTERMINAL_INTERFACE_PORTMAP)) {
					validBit = valIfStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.valVtermIfIndex.UPLL_IDX_PM_VTERMI
											.ordinal());
				} else {
					LOG.debug("Incorrect ifType:" + ifType);
				}
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {

					final IpcStruct valPortMapStruct = IpcDataUnitWrapper
							.getInnerIpcStruct(valIfStruct,
									VtnServiceJsonConsts.PORTMAP);
					/*
					 * add valid switch_id from value structure
					 */
					validBit = valPortMapStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_LOGICAL_PORT_ID_PM
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, portMap,
								VtnServiceJsonConsts.LOGICAL_PORT_ID,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valPortMapStruct,
										VtnServiceIpcConsts.LOGICAL_PORT_ID));
					}
					/*
					 * add valid Novlanid from value structure
					 */
					validBit = valPortMapStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_VLAN_ID_PM
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, portMap,
								VtnServiceJsonConsts.VLANID,
								IpcDataUnitWrapper.getIpcStructUint16Value(
										valPortMapStruct,
										VtnServiceJsonConsts.VLANID));
						// }
						LOG.debug("Vlan Id :"
								+ IpcDataUnitWrapper.getIpcStructUint16Value(
										valPortMapStruct,
										VtnServiceJsonConsts.VLANID));
					}
					/*
					 * add valid tagged from value structure
					 */
					validBit = valPortMapStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_TAGGED_PM
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valPortMapStruct,
										VtnServiceIpcConsts.TAGGED)
								.equals(UncStructIndexEnum.vlan_tagged.UPLL_VLAN_TAGGED
										.getValue())) {
							setValueToJsonObject(validBit, portMap,
									VtnServiceJsonConsts.TAGGED,
									VtnServiceJsonConsts.TRUE);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valPortMapStruct,
										VtnServiceIpcConsts.TAGGED)
								.equals(UncStructIndexEnum.vlan_tagged.UPLL_VLAN_UNTAGGED
										.getValue())) {
							setValueToJsonObject(validBit, portMap,
									VtnServiceJsonConsts.TAGGED,
									VtnServiceJsonConsts.FALSE);
						}
						LOG.debug("Tagged :"
								+ IpcDataUnitWrapper.getIpcStructUint8Value(
										valPortMapStruct,
										VtnServiceIpcConsts.TAGGED));
					}
				}
				if (dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
					LOG.debug("In target db state case-skip St structure ");
					index++;

				}
			} else {
				LOG.debug("IfType:: " + "is either null or incorrect");
			}
		}
		/*
		 * finally add single object to root json object and return the same.
		 */
		root.add(rootJsonName, portMap);
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getPortMapResponse");
		return root;
	}

	public JsonObject getVlanMapResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody, final String getType) {
		LOG.trace("Start getVlanMapResponse");
		final JsonObject root = new JsonObject();
		JsonArray vlanMapsArray = null;
		LOG.debug("getType: " + getType);
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}
		/*
		 * data type will be required to resolve the response structures
		 */
		String rootJsonName;
		/*
		 * get type (show or list) will be required to resolve root json name
		 * here it will be vlanMap for show and vlanMaps for list
		 */
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.VLANMAP;
		} else {
			rootJsonName = VtnServiceJsonConsts.VLANMAPS;
			// json array will be required for list type of cases
			vlanMapsArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject vlanMap = null;

		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			vlanMap = new JsonObject();
			vlanMap.addProperty(
					VtnServiceJsonConsts.COUNT,
					IpcDataUnitWrapper
							.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, vlanMap);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {

				vlanMap = new JsonObject();
				byte validBit;

				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;
				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyVlanMapStruct = (IpcStruct) responsePacket[index++];
				String logicalPortId = "";
				String vlanMapId = "";
				final String lpid_valid = IpcDataUnitWrapper
						.getIpcStructUint8Value(keyVlanMapStruct,
								VtnServiceIpcConsts.LPID_VALID);
				LOG.debug("LpidValid :"
						+ IpcDataUnitWrapper.getIpcStructUint8Value(
								keyVlanMapStruct,
								VtnServiceIpcConsts.LPID_VALID));
				if (lpid_valid
						.equalsIgnoreCase(UncStructIndexEnum.PfcStatus.PFC_TRUE
								.getValue())) {
					logicalPortId = IpcDataUnitWrapper
							.getIpcStructUint8ArrayValue(keyVlanMapStruct,
									VtnServiceIpcConsts.LOGICAL_PORT_ID);
					vlanMapId = VtnServiceJsonConsts.LPID
							+ VtnServiceJsonConsts.VLANMAPIDSEPERATOR
							+ logicalPortId;
					LOG.debug("LogicalPortId :"
							+ IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
									keyVlanMapStruct,
									VtnServiceIpcConsts.LOGICAL_PORT_ID));
					LOG.debug("VlanMapId: " + vlanMapId);
				} else {
					vlanMapId = VtnServiceJsonConsts.NOLPID;
					LOG.debug("VlanMapId: " + vlanMapId);
				}
				vlanMap.addProperty(VtnServiceJsonConsts.VLANMAPID, vlanMapId);

				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */
				if (getType.equals(VtnServiceJsonConsts.SHOW)
						|| opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
					LOG.debug("Case : Show or detail");
					/*
					 * add valid informations from value structure
					 */
					if (!logicalPortId.equals(VtnServiceConsts.EMPTY_STRING)) {
						vlanMap.addProperty(
								VtnServiceJsonConsts.LOGICAL_PORT_ID,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										keyVlanMapStruct,
										VtnServiceIpcConsts.LOGICAL_PORT_ID));
						LOG.debug("Logical_port_id :"
								+ IpcDataUnitWrapper
										.getIpcStructUint8ArrayValue(
												keyVlanMapStruct,
												VtnServiceIpcConsts.LOGICAL_PORT_ID));
					}

					final IpcStruct valVlanMapStruct = (IpcStruct) responsePacket[index++];
					validBit = valVlanMapStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVlanMapIndex.UPLL_IDX_VLAN_ID_VM
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						if (IpcDataUnitWrapper
								.getIpcStructUint16HexaValue(valVlanMapStruct,
										VtnServiceJsonConsts.VLANID)
								.equalsIgnoreCase(
										VtnServiceIpcConsts.VLAN_ID_DEFAULT_VALUE)) {
							vlanMap.addProperty(
									VtnServiceJsonConsts.NO_VLAN_ID,
									VtnServiceJsonConsts.TRUE);
						} else {
							setValueToJsonObject(validBit, vlanMap,
									VtnServiceJsonConsts.VLANID,
									IpcDataUnitWrapper.getIpcStructUint16Value(
											valVlanMapStruct,
											VtnServiceJsonConsts.VLANID));
						}
						LOG.debug("VlanId :"
								+ IpcDataUnitWrapper
										.getIpcStructUint16HexaValue(
												valVlanMapStruct,
												VtnServiceJsonConsts.VLANID));
					}
				} else {
					LOG.debug("Operation : normal Skip value struture");
					index++;
				}
				if (null != vlanMapsArray) {
					vlanMapsArray.add(vlanMap);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != vlanMapsArray) {
				root.add(rootJsonName, vlanMapsArray);
			} else {
				root.add(rootJsonName, vlanMap);
			}

		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVlanMapResponse");
		return root;
	}

	public JsonObject getVBridgeInterfaceFlowFilterEntryResponse(
			final IpcDataUnit[] responsePacket, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start getVBridgeInterfaceFlowFilterEntryResponse");
		final JsonObject root = new JsonObject();
		JsonArray vbrIfFlowFilterEntryArray = null;
		LOG.debug("getType: " + getType);
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}
		/*
		 * data type will be required to resolve the response structures
		 */
		String dataType = VtnServiceJsonConsts.STATE;
		if (requestBody.has(VtnServiceJsonConsts.TARGETDB)) {
			dataType = requestBody.get(VtnServiceJsonConsts.TARGETDB)
					.getAsString();
		}
		String rootJsonName;
		/*
		 * get type (show or list) will be required to resolve root json name
		 * here it will be vtn for show and vtns for list
		 */
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.FLOWFILTERENTRY;
		} else {
			rootJsonName = VtnServiceJsonConsts.FLOWFILTERENTRIES;
			// json array will be required for list type of cases
			vbrIfFlowFilterEntryArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject vbrIfFlowFilterEntry = null;

		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			vbrIfFlowFilterEntry = new JsonObject();
			vbrIfFlowFilterEntry
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, vbrIfFlowFilterEntry);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {

				vbrIfFlowFilterEntry = new JsonObject();
				byte validBit;

				// There is no use of key type
				index++;
				LOG.debug("Skip key type: no use");
				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyVbrIfFlowFilterEntryStruct = (IpcStruct) responsePacket[index++];
				vbrIfFlowFilterEntry.addProperty(VtnServiceJsonConsts.SEQNUM,
						IpcDataUnitWrapper.getIpcStructUint16Value(
								keyVbrIfFlowFilterEntryStruct,
								VtnServiceIpcConsts.SEQUENCENUM));

				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */

				if (getType.equals(VtnServiceJsonConsts.LIST)
						&& opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)
						|| getType.equals(VtnServiceJsonConsts.SHOW)) {
					LOG.debug("Case : Show or List with detail");
					/*
					 * add valid informations from value structure
					 */
					final IpcStruct valFlowFilterEntryStruct = (IpcStruct) responsePacket[index++];

					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_FLOWLIST_NAME_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vbrIfFlowFilterEntry,
								VtnServiceJsonConsts.FLNAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valFlowFilterEntryStruct,
										VtnServiceJsonConsts.FLOWLISTNAME));
					}
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_ACTION_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						if (IpcDataUnitWrapper
								.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.ACTION)
								.equalsIgnoreCase(
										UncStructIndexEnum.FlowfilterAction.UPLL_FLOWFILTER_ACT_PASS
												.getValue())) {
							setValueToJsonObject(validBit,
									vbrIfFlowFilterEntry,
									VtnServiceJsonConsts.ACTIONTYPE,
									VtnServiceJsonConsts.PASS);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.ACTION)
								.equalsIgnoreCase(
										UncStructIndexEnum.FlowfilterAction.UPLL_FLOWFILTER_ACT_DROP
												.getValue())) {
							setValueToJsonObject(validBit,
									vbrIfFlowFilterEntry,
									VtnServiceJsonConsts.ACTIONTYPE,
									VtnServiceJsonConsts.DROP);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.ACTION)
								.equalsIgnoreCase(
										UncStructIndexEnum.FlowfilterAction.UPLL_FLOWFILTER_ACT_REDIRECT
												.getValue())) {
							setValueToJsonObject(validBit,
									vbrIfFlowFilterEntry,
									VtnServiceJsonConsts.ACTIONTYPE,
									VtnServiceJsonConsts.REDIRECT);
						}
						LOG.debug("Action type :"
								+ IpcDataUnitWrapper.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.ACTION));
					}
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_NWM_NAME_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vbrIfFlowFilterEntry,
								VtnServiceJsonConsts.NMGNAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.NWMNAME));
					}
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_PRIORITY_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vbrIfFlowFilterEntry,
								VtnServiceJsonConsts.PRIORITY,
								IpcDataUnitWrapper.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceJsonConsts.PRIORITY));
					}
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_DSCP_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vbrIfFlowFilterEntry,
								VtnServiceJsonConsts.DSCP,
								IpcDataUnitWrapper.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceJsonConsts.DSCP));
					}
					final JsonObject redirectDst = new JsonObject();
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_REDIRECT_NODE_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, redirectDst,
								VtnServiceJsonConsts.VNODENAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.REDIRECTNODE));
					}
					// Query for if_name currently mapping as per FD ie network
					// monitor group name
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_REDIRECT_PORT_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, redirectDst,
								VtnServiceJsonConsts.IFNAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.REDIRECTPORT));
					}
					// Direction
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_REDIRECT_DIRECTION_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						if (IpcDataUnitWrapper
								.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.REDIRECTDIRECTION)
								.equals(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_IN
										.getValue())) {
							setValueToJsonObject(validBit, redirectDst,
									VtnServiceJsonConsts.DIRECTION,
									VtnServiceJsonConsts.IN);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.REDIRECTDIRECTION)
								.equals(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_OUT
										.getValue())) {
							setValueToJsonObject(validBit, redirectDst,
									VtnServiceJsonConsts.DIRECTION,
									VtnServiceJsonConsts.OUT);
						} else {
							LOG.debug("Direction : Invalid");
						}
						LOG.debug("Direction :"
								+ IpcDataUnitWrapper.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.REDIRECTDIRECTION));
					}
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_MODIFY_DST_MAC_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, redirectDst,
								VtnServiceJsonConsts.MACDSTADDR,
								IpcDataUnitWrapper.getMacAddress(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.MODIFYDSTMACADDR));
					}
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_MODIFY_SRC_MAC_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, redirectDst,
								VtnServiceJsonConsts.MACSRCADDR,
								IpcDataUnitWrapper.getMacAddress(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.MODIFYSRCMACADDR));
					}
					vbrIfFlowFilterEntry.add(VtnServiceJsonConsts.REDIRECTDST,
							redirectDst);
				}
				if (getType.equals(VtnServiceJsonConsts.SHOW)
						&& dataType
								.equalsIgnoreCase(VtnServiceJsonConsts.STATE)
						&& opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
					LOG.debug("Case : Show and targetdb :State ");
					final IpcStruct valFlowFilterEntryStStruct = (IpcStruct) responsePacket[index++];
					if (opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
						LOG.debug("op : detail");
						validBit = valFlowFilterEntryStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.ValFlowfilterEntryStIndex.UPLL_IDX_NWM_STATUS_FFES
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							setValueToJsonObject(validBit,
									vbrIfFlowFilterEntry,
									VtnServiceJsonConsts.NMG_STATUS,
									IpcDataUnitWrapper.getIpcStructUint8Value(
											valFlowFilterEntryStStruct,
											VtnServiceIpcConsts.NWM_STATUS));
						}
						final PomStatsIndex pomStatsIndexSet = new PomStatsIndex();
						pomStatsIndexSet
								.setSoftware(UncStructIndexEnum.ValFlowfilterEntryStIndex.UPLL_IDX_SOFTWARE_FFES
										.ordinal());
						pomStatsIndexSet
								.setExistingFlow(UncStructIndexEnum.ValFlowfilterEntryStIndex.UPLL_IDX_EXIST_FFES
										.ordinal());
						pomStatsIndexSet
								.setExpiredFlow(UncStructIndexEnum.ValFlowfilterEntryStIndex.UPLL_IDX_EXPIRE_FFES
										.ordinal());
						pomStatsIndexSet
								.setTotal(UncStructIndexEnum.ValFlowfilterEntryStIndex.UPLL_IDX_TOTAL_FFES
										.ordinal());
						LOG.debug("call getPomStats : for statics information");
						getPomStats(vbrIfFlowFilterEntry,
								valFlowFilterEntryStStruct, pomStatsIndexSet);
						final String flowListName = VtnServiceJsonConsts.FLOWLIST;
						final JsonObject flowListJson = new JsonObject();
						final String flowListEntriesName = VtnServiceJsonConsts.FLOWLISTENTRIES;
						final JsonArray flowListEntriesJsonArray = new JsonArray();
						LOG.debug("call getPomStatsFLowList : for statics information of flowList");
						index = getPomStatsFlowList(responsePacket, index,
								flowListEntriesJsonArray);
						flowListJson.add(flowListEntriesName,
								flowListEntriesJsonArray);
						vbrIfFlowFilterEntry.add(flowListName, flowListJson);
					} else {
						LOG.debug("Show ,Operation : normal and target db :state Skip flowList value strutures ");
						// increasing index to eliminate flow list entry
						// structures in case of show and op : normal
						index = responsePacket.length - 1;
					}
				}
				if (getType.equals(VtnServiceJsonConsts.LIST)
						&& opType.equalsIgnoreCase(VtnServiceJsonConsts.NORMAL)) {
					LOG.debug("List ,Operation : normal Skip value strutures ");
					index++;
				}
				// add current json object to array, if it has been initialized
				// earlier
				if (null != vbrIfFlowFilterEntryArray) {
					vbrIfFlowFilterEntryArray.add(vbrIfFlowFilterEntry);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != vbrIfFlowFilterEntryArray) {
				root.add(rootJsonName, vbrIfFlowFilterEntryArray);
			} else {
				root.add(rootJsonName, vbrIfFlowFilterEntry);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVBridgeInterfaceFlowFilterEntryResponse");

		return root;
	}

	public JsonObject getVTunnelInterfaceResourceResponse(
			final IpcDataUnit[] responsePacketIf, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start getVTunnelInterfaceResourceResponse");
		final JsonObject root = new JsonObject();
		JsonArray vTunnelInterfacesArray = null;
		LOG.debug("getType: " + getType);
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}

		/*
		 * data type will be required to resolve the response structures
		 */
		String dataType = VtnServiceJsonConsts.STATE;
		if (requestBody.has(VtnServiceJsonConsts.TARGETDB)) {
			dataType = requestBody.get(VtnServiceJsonConsts.TARGETDB)
					.getAsString();
		}

		String rootJsonName;

		/*
		 * get type (show or list) will be required to resolve root json name
		 * here it will be vRouter for show and vRouters for list
		 */
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.INTERFACE;
		} else {
			rootJsonName = VtnServiceJsonConsts.INTERFACES;
			// json array will be required for list type of cases
			vTunnelInterfacesArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject vTunnelInterface = null;

		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			vTunnelInterface = new JsonObject();
			vTunnelInterface
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacketIf[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, vTunnelInterface);
		} else {

			for (int index = 0; index < responsePacketIf.length; index++) {

				vTunnelInterface = new JsonObject();
				byte validBit;

				// There is no use of key type so skipped it.
				LOG.debug("Skip key type: no use");
				index++;

				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyVTunnelIfStruct = (IpcStruct) responsePacketIf[index++];
				vTunnelInterface
						.addProperty(VtnServiceJsonConsts.IFNAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										keyVTunnelIfStruct,
										VtnServiceIpcConsts.IFNAME));

				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */
				if (getType.equals(VtnServiceJsonConsts.SHOW)
						|| opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
					LOG.debug("Case : Show or detail");
					/*
					 * add valid informations from value structure
					 */
					final IpcStruct valVtunnelIfStruct = (IpcStruct) responsePacketIf[index++];
					validBit = valVtunnelIfStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.val_vtunnel_if_index.UPLL_IDX_DESC_VTNL_IF
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vTunnelInterface,
								VtnServiceJsonConsts.DESCRIPTION,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVtunnelIfStruct,
										VtnServiceIpcConsts.DESCRIPTION));
					}

					validBit = valVtunnelIfStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.val_vtunnel_if_index.UPLL_IDX_ADMIN_ST_VTNL_IF
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valVtunnelIfStruct,
										VtnServiceJsonConsts.ADMIN_STATUS)
								.equals(UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_ENABLE
										.getValue())) {
							setValueToJsonObject(validBit, vTunnelInterface,
									VtnServiceJsonConsts.ADMINSTATUS,
									VtnServiceJsonConsts.ENABLE);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valVtunnelIfStruct,
										VtnServiceJsonConsts.ADMIN_STATUS)
								.equals(UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_DISABLE
										.getValue())) {
							setValueToJsonObject(validBit, vTunnelInterface,
									VtnServiceJsonConsts.ADMINSTATUS,
									VtnServiceJsonConsts.DISABLE);
						} else {
							LOG.debug("Adminstatus : Invalid");
						}
						LOG.debug("Adminstatus :"
								+ IpcDataUnitWrapper.getIpcStructUint8Value(
										valVtunnelIfStruct,
										VtnServiceJsonConsts.ADMIN_STATUS));
					}

					if (dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
						LOG.debug("targetdb : State");
						final IpcStruct valVtunnelIfStruct1 = (IpcStruct) responsePacketIf[index++];

						validBit = valVtunnelIfStruct1
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.val_vtunnel_if_st_index.UPLL_IDX_IF_OPER_STATUS_VTNLI
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							if (IpcDataUnitWrapper
									.getIpcStructUint8Value(
											valVtunnelIfStruct1,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_UP
											.getValue())) {
								setValueToJsonObject(validBit,
										vTunnelInterface,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.UP);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(
											valVtunnelIfStruct1,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_DOWN
											.getValue())) {
								setValueToJsonObject(validBit,
										vTunnelInterface,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.DOWN);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(
											valVtunnelIfStruct1,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_UNKNOWN
											.getValue())) {
								setValueToJsonObject(validBit,
										vTunnelInterface,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.UNKNOWN);
							} else {
								LOG.debug("Operstatus: Invalid");
							}
							LOG.debug("Operstatus :"
									+ IpcDataUnitWrapper
											.getIpcStructUint8Value(
													valVtunnelIfStruct1,
													VtnServiceIpcConsts.OPERSTATUS));
						}

					}

				} else {
					LOG.debug("Operation : normal Skip value struture");
					index++;
					if (dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
						LOG.debug("Operation : normal and target db :state Skip St value struture ");
						index++;
					}
				}

				// add current json object to array, if it has been initialized
				// earlier
				if (null != vTunnelInterfacesArray) {
					vTunnelInterfacesArray.add(vTunnelInterface);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != vTunnelInterfacesArray) {
				root.add(rootJsonName, vTunnelInterfacesArray);
			} else {
				root.add(rootJsonName, vTunnelInterface);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVTunnelInterfaceResourceResponse");

		return root;
	}

	public JsonObject getStaticIpRouteResponse(
			final IpcDataUnit[] responsePacket, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start getStaticIpRouteResponse");
		final JsonObject root = new JsonObject();
		JsonArray staticIpRouteArray = null;
		LOG.debug("getType: " + getType);

		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}

		String rootJsonName;
		/*
		 * get type (show or list) will be required to resolve root json name
		 * here it will be flowList for show and flowLists for list
		 */
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.STATIC_IPROUTE;
		} else {
			rootJsonName = VtnServiceJsonConsts.STATIC_IPROUTES;
			// json array will be required for list type of cases
			staticIpRouteArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject staticIpRoute = null;

		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			staticIpRoute = new JsonObject();
			staticIpRoute
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, staticIpRoute);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {

				staticIpRoute = new JsonObject();

				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;

				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyStaticIpRouteStruct = (IpcStruct) responsePacket[index++];
				final String dst_addr = IpcDataUnitWrapper
						.getIpcStructIpv4Value(keyStaticIpRouteStruct,
								VtnServiceIpcConsts.DST_ADDR);
				final String prefixlen = IpcDataUnitWrapper
						.getIpcStructUint8Value(keyStaticIpRouteStruct,
								VtnServiceIpcConsts.DST_ADDR_PREFIXLEN);
				final String nextHopAddr = IpcDataUnitWrapper
						.getIpcStructIpv4Value(keyStaticIpRouteStruct,
								VtnServiceIpcConsts.NEXT_HOP_ADDR);

				final String staticIpRouteStr = dst_addr
						+ VtnServiceJsonConsts.HYPHEN + nextHopAddr
						+ VtnServiceJsonConsts.HYPHEN + prefixlen;
				staticIpRoute.addProperty(VtnServiceJsonConsts.STATICIPROUTEID,
						staticIpRouteStr);
				if (getType.equals(VtnServiceJsonConsts.SHOW)
						|| opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
					LOG.debug("Case : Show or detail");
					staticIpRoute.addProperty(VtnServiceJsonConsts.IPADDR,
							dst_addr);
					staticIpRoute.addProperty(VtnServiceJsonConsts.PREFIX,
							prefixlen);
					staticIpRoute.addProperty(VtnServiceJsonConsts.NEXTHOPADDR,
							nextHopAddr);

					final IpcStruct valStaticIpRouteStruct = (IpcStruct) responsePacket[index++];
					byte validBit;
					validBit = valStaticIpRouteStruct.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValStaticIpRouteIndex
									.UPLL_IDX_NWM_NAME_SIR.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, staticIpRoute,
								VtnServiceJsonConsts.NMGNAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valStaticIpRouteStruct,
										VtnServiceIpcConsts.NWM_NAME));
					}
					validBit = valStaticIpRouteStruct.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValStaticIpRouteIndex
									.UPLL_IDX_GROUP_METRIC_SIR.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
						setValueToJsonObject(validBit, staticIpRoute,
								VtnServiceJsonConsts.GROUPMETRIC,
								IpcDataUnitWrapper.getIpcStructUint16Value(
										valStaticIpRouteStruct,
										VtnServiceIpcConsts.GROUP_METRIC));
					}
				} else {
					// no use of value structure
					LOG.debug("no use of value structure");
					index++;
				}
				// add current json object to array, if it has been initialized
				// earlier
				if (null != staticIpRouteArray) {
					staticIpRouteArray.add(staticIpRoute);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != staticIpRouteArray) {
				root.add(rootJsonName, staticIpRouteArray);
			} else {
				root.add(rootJsonName, staticIpRoute);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getStaticIpRouteResponse");
		return root;
	}

	public JsonObject getVBridgeInterfaceFlowFilterResource(
			final IpcDataUnit[] responsePacket, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start getVBridgeInterfaceFlowFilterResource");
		final JsonObject root = new JsonObject();

		final String rootJsonName = VtnServiceJsonConsts.FLOWFILTER;
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject flowFilter = null;
		for (int index = 0; index < responsePacket.length; index++) {

			flowFilter = new JsonObject();

			// There is no use of key type
			LOG.debug("Skip key type: no use");
			index++;
			/*
			 * add mandatory informations from key structure
			 */
			final IpcStruct keyVbrVintFlowFilter = (IpcStruct) responsePacket[index++];
			if (IpcDataUnitWrapper
					.getIpcStructUint8Value(keyVbrVintFlowFilter,
							VtnServiceIpcConsts.DIRECTION)
					.equals(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_IN
							.getValue())) {
				flowFilter.addProperty(VtnServiceJsonConsts.FFTYPE,
						VtnServiceJsonConsts.IN);
				LOG.debug("FF Type :"
						+ IpcDataUnitWrapper.getIpcStructUint8Value(
								keyVbrVintFlowFilter,
								VtnServiceIpcConsts.DIRECTION));
			} else if (IpcDataUnitWrapper
					.getIpcStructUint8Value(keyVbrVintFlowFilter,
							VtnServiceIpcConsts.DIRECTION)
					.equals(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_OUT
							.getValue())) {
				flowFilter.addProperty(VtnServiceJsonConsts.FFTYPE,
						VtnServiceJsonConsts.OUT);
				LOG.debug("FF Type :"
						+ IpcDataUnitWrapper.getIpcStructUint8Value(
								keyVbrVintFlowFilter,
								VtnServiceIpcConsts.DIRECTION));
			} else {
				LOG.debug("Invalid value for FFTYPE parameter");
			}
		}

		/*
		 * finally add single object to root json object and return the same.
		 */
		root.add(rootJsonName, flowFilter);
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVBridgeInterfaceFlowFilterResource");

		return root;
	}

	public JsonObject getVtepResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody, final String getType) {

		LOG.trace("Start getVtepResponse");
		final JsonObject root = new JsonObject();
		JsonArray vtepsArray = null;
		LOG.debug("getType: " + getType);
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}

		/*
		 * data type will be required to resolve the response structures
		 */
		String dataType = VtnServiceJsonConsts.STATE;
		if (requestBody.has(VtnServiceJsonConsts.TARGETDB)) {
			dataType = requestBody.get(VtnServiceJsonConsts.TARGETDB)
					.getAsString();
		}

		String rootJsonName;
		/*
		 * get type (show or list) will be required to resolve root json name
		 * here it will be vtn for show and vtns for list
		 */
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.VTEP;
		} else {
			rootJsonName = VtnServiceJsonConsts.VTEPS;
			// json array will be required for list type of cases
			vtepsArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject vtep = null;

		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			vtep = new JsonObject();
			vtep.addProperty(
					VtnServiceJsonConsts.COUNT,
					IpcDataUnitWrapper
							.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, vtep);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {

				vtep = new JsonObject();
				byte validBit;

				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;

				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyVtepStruct = (IpcStruct) responsePacket[index++];
				vtep.addProperty(VtnServiceJsonConsts.VTEPNAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								keyVtepStruct, VtnServiceIpcConsts.VTEPNAME));

				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */
				if (getType.equals(VtnServiceJsonConsts.SHOW)
						|| opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
					LOG.debug("Case : Show or detail");
					/*
					 * add valid informations from value structure
					 */

					final IpcStruct valVtepStruct = (IpcStruct) responsePacket[index++];
					validBit = valVtepStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.val_vtep_index.UPLL_IDX_CONTROLLER_ID_VTEP// controller
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vtep,
								VtnServiceJsonConsts.CONTROLLERID,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVtepStruct,
										VtnServiceIpcConsts.CONTROLLERID));
					}

					validBit = valVtepStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.val_vtep_index.UPLL_IDX_DESC_VTEP
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vtep,
								VtnServiceJsonConsts.DESCRIPTION,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVtepStruct,
										VtnServiceIpcConsts.DESCRIPTION));
					}
					validBit = valVtepStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.val_vtep_index.UPLL_IDX_DOMAIN_ID_VTEP
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vtep,
								VtnServiceJsonConsts.DOMAINID,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVtepStruct,
										VtnServiceJsonConsts.DOMAINID));
					}

					/*
					 * If data type is set as "state", then value structure will
					 * also contain the state information
					 */
					if (dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
						LOG.debug("targetdb : State");
						final IpcStruct valVtepStStruct = (IpcStruct) responsePacket[index++];

						/*
						 * If response is required in detail format then use the
						 * State value structure
						 */

						validBit = valVtepStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.val_vtep_st_index.UPLL_IDX_OPER_STATUS_VTEPS
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVtepStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_UP
											.getValue())) {
								setValueToJsonObject(validBit, vtep,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.UP);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVtepStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_DOWN
											.getValue())) {
								setValueToJsonObject(validBit, vtep,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.DOWN);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVtepStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_UNKNOWN
											.getValue())) {
								setValueToJsonObject(validBit, vtep,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.UNKNOWN);
							} else {
								LOG.debug("Operstatus : Invalid");
							}
							LOG.debug("Operstatus :"
									+ IpcDataUnitWrapper
											.getIpcStructUint8Value(
													valVtepStStruct,
													VtnServiceIpcConsts.OPERSTATUS));
						}

					}
				} else {
					LOG.debug("Operation : normal Skip value struture");
					index++;
					if (dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
						LOG.debug("Operation : normal and target db :state Skip St value struture ");
						index++;
					}
				}
				// add current json object to array, if it has been initialized
				// earlier
				if (null != vtepsArray) {
					vtepsArray.add(vtep);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != vtepsArray) {
				root.add(rootJsonName, vtepsArray);
			} else {
				root.add(rootJsonName, vtep);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVtepResponse");
		return root;
	}

	/**
	 * data from val_vtn_flow_filter_entry
	 * 
	 * @param flowFilterEntry
	 * @param valFlowFilterEntryStruct
	 */
	public void getValVtnFlowFilterEntry(final JsonObject flowFilterEntry,
			final IpcStruct valFlowFilterEntryStruct) {
		LOG.trace("Start getValVtnFlowFilterEntry");
		byte validBit;
		validBit = valFlowFilterEntryStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnFlowfilterEntryIndex.UPLL_IDX_FLOWLIST_NAME_VFFE
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, flowFilterEntry,
					VtnServiceJsonConsts.FLNAME,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valFlowFilterEntryStruct,
							VtnServiceJsonConsts.FLOWLISTNAME));
		}
		validBit = valFlowFilterEntryStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnFlowfilterEntryIndex.UPLL_IDX_ACTION_VFFE
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			if (IpcDataUnitWrapper
					.getIpcStructUint8Value(valFlowFilterEntryStruct,
							VtnServiceIpcConsts.ACTION)
					.equalsIgnoreCase(
							UncStructIndexEnum.FlowfilterAction.UPLL_FLOWFILTER_ACT_PASS
									.getValue())) {
				setValueToJsonObject(validBit, flowFilterEntry,
						VtnServiceJsonConsts.ACTIONTYPE,
						VtnServiceJsonConsts.PASS);
			} else {
				LOG.debug("Action Type : Invalid");
			}
			LOG.debug("Action Type :"
					+ IpcDataUnitWrapper.getIpcStructUint8Value(
							valFlowFilterEntryStruct,
							VtnServiceIpcConsts.ACTION));
		}
		validBit = valFlowFilterEntryStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnFlowfilterEntryIndex.UPLL_IDX_NWN_NAME_VFFE
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, flowFilterEntry,
					VtnServiceJsonConsts.NMGNAME,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valFlowFilterEntryStruct,
							VtnServiceIpcConsts.NWMNAME));
		}
		validBit = valFlowFilterEntryStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnFlowfilterEntryIndex.UPLL_IDX_PRIORITY_VFFE
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, flowFilterEntry,
					VtnServiceJsonConsts.PRIORITY,
					IpcDataUnitWrapper.getIpcStructUint8Value(
							valFlowFilterEntryStruct,
							VtnServiceJsonConsts.PRIORITY));
		}
		validBit = valFlowFilterEntryStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnFlowfilterEntryIndex.UPLL_IDX_DSCP_VFFE
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, flowFilterEntry,
					VtnServiceJsonConsts.DSCP,
					IpcDataUnitWrapper
							.getIpcStructUint8Value(valFlowFilterEntryStruct,
									VtnServiceJsonConsts.DSCP));
		}
		LOG.debug("getValVtnFlowFilterEntry Json: "
				+ flowFilterEntry.toString());
		LOG.trace("Complete getValVtnFlowFilterEntry");
	}

	/**
	 * Create Json with the root fixed parameters part for VTN Station Show API
	 * 
	 * @param vtnStation
	 * @param valVtnstationControllerSt
	 */
	private void createVtnStationConstJson(final JsonObject vtnStation,
			final IpcStruct valVtnstationControllerSt) {
		LOG.trace("Start createVtnStationConstJson");
		byte validBit;
		// for station_id parameter
		validBit = valVtnstationControllerSt
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_STATION_ID_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, vtnStation,
					VtnServiceJsonConsts.STATIONID,
					IpcDataUnitWrapper.getIpcStructUint64Value(
							valVtnstationControllerSt,
							VtnServiceIpcConsts.STATIONID));
		}

		// for createdtime parameter
		validBit = valVtnstationControllerSt
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_CREATED_TIME_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, vtnStation,
					VtnServiceJsonConsts.CREATEDTIME,
					IpcDataUnitWrapper.getIpcStructUint64Value(
							valVtnstationControllerSt,
							VtnServiceIpcConsts.CREATED_TIME));
		}

		// for macaddr parameter
		validBit = valVtnstationControllerSt
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_MAC_ADDR_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, vtnStation,
					VtnServiceJsonConsts.MACADDR,
					// IpcDataUnitWrapper.getIpcStructUint8ArrayValue(valVtnstationControllerSt,
					// VtnServiceIpcConsts.MAC_ADDR_VTNSTATION));
					IpcDataUnitWrapper.getMacAddress(valVtnstationControllerSt,
							VtnServiceIpcConsts.MAC_ADDR_VTNSTATION));
		}

		// for map_type parameter
		validBit = valVtnstationControllerSt
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_MAP_TYPE_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			if (UncStructIndexEnum.ValVbrIfMapType.UPLL_IF_OFS_MAP.getValue()
					.equals(IpcDataUnitWrapper.getIpcStructUint8Value(
							valVtnstationControllerSt,
							VtnServiceIpcConsts.MAP_TYPE))) {
				setValueToJsonObject(validBit, vtnStation,
						VtnServiceJsonConsts.MAPTYPE,
						VtnServiceJsonConsts.PORTMAP);
			} else if (UncStructIndexEnum.ValVbrIfMapType.UPLL_IF_VLAN_MAP
					.getValue().equals(
							IpcDataUnitWrapper.getIpcStructUint8Value(
									valVtnstationControllerSt,
									VtnServiceIpcConsts.MAP_TYPE))) {
				setValueToJsonObject(validBit, vtnStation,
						VtnServiceJsonConsts.MAPTYPE,
						VtnServiceJsonConsts.VLANMAP);
			}
			LOG.debug("MapType :"
					+ IpcDataUnitWrapper.getIpcStructUint8Value(
							valVtnstationControllerSt,
							VtnServiceIpcConsts.MAP_TYPE));
		}

		// for map_status parameter
		validBit = valVtnstationControllerSt
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_MAP_STATUS_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			if (UncStructIndexEnum.ValVtnMapStatus.UPLL_VTN_MAP_VALID
					.getValue().equals(
							IpcDataUnitWrapper.getIpcStructUint8Value(
									valVtnstationControllerSt,
									VtnServiceIpcConsts.MAP_STATUS))) {
				setValueToJsonObject(validBit, vtnStation,
						VtnServiceJsonConsts.MAPSTATUS,
						VtnServiceJsonConsts.VALID);
			} else if (UncStructIndexEnum.ValVtnMapStatus.UPLL_VTN_MAP_INVALID
					.getValue().equals(
							IpcDataUnitWrapper.getIpcStructUint8Value(
									valVtnstationControllerSt,
									VtnServiceIpcConsts.MAP_STATUS))) {
				setValueToJsonObject(validBit, vtnStation,
						VtnServiceJsonConsts.MAPSTATUS,
						VtnServiceJsonConsts.INVALID);
			}
			LOG.debug("Mapstatus :"
					+ IpcDataUnitWrapper.getIpcStructUint8Value(
							valVtnstationControllerSt,
							VtnServiceIpcConsts.MAP_STATUS));
		}

		// for vtn_name parameter
		validBit = valVtnstationControllerSt
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VTN_NAME_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, vtnStation,
					VtnServiceJsonConsts.VTNNAME,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valVtnstationControllerSt,
							VtnServiceIpcConsts.VTNNAME));
		}

		// for domain_id parameter
		validBit = valVtnstationControllerSt
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_DOMAIN_ID_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, vtnStation,
					VtnServiceJsonConsts.DOMAINID,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valVtnstationControllerSt,
							VtnServiceIpcConsts.DOMAINID));
		}
		// for vnode_type parameter
		validBit = valVtnstationControllerSt
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VNODE_TYPE_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {

			int vnodeVal = Integer.parseInt(IpcDataUnitWrapper
					.getIpcStructUint8Value(valVtnstationControllerSt,
							VtnServiceIpcConsts.VNODETYPE));
			String vnodeType = null;
			if (vnodeVal == UncStructIndexEnum.ValVnodeType.UPLL_VNODE_VBRIDGE
					.ordinal()) {
				vnodeType = VtnServiceJsonConsts.VBRIDGE;
			} else if (vnodeVal == UncStructIndexEnum.ValVnodeType.UPLL_VNODE_VROUTER
					.ordinal()) {
				vnodeType = VtnServiceJsonConsts.VROUTER;
			} else if (vnodeVal == UncStructIndexEnum.ValVnodeType.UPLL_VNODE_VTEP
					.ordinal()) {
				vnodeType = VtnServiceJsonConsts.VTEP;
			} else if (vnodeVal == UncStructIndexEnum.ValVnodeType.UPLL_VNODE_VTERMINAL
					.ordinal()) {
				vnodeType = VtnServiceJsonConsts.VTERMINAL;
			} else if (vnodeVal == UncStructIndexEnum.ValVnodeType.UPLL_VNODE_VTUNNEL
					.ordinal()) {
				vnodeType = VtnServiceJsonConsts.VTUNNEL;
			} else if (vnodeVal == UncStructIndexEnum.ValVnodeType.UPLL_VNODE_VUNKNOWN
					.ordinal()) {
				vnodeType = VtnServiceJsonConsts.VBYPASS;
			}
			setValueToJsonObject(validBit, vtnStation,
					VtnServiceJsonConsts.VNODETYPE, vnodeType);
		}
		// for vnode_name parameter
		validBit = valVtnstationControllerSt
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VNODE_NAME_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, vtnStation,
					VtnServiceJsonConsts.VNODENAME,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valVtnstationControllerSt,
							VtnServiceIpcConsts.VNODENAME));
		}

		final JsonObject interfaceObj = new JsonObject();

		// for if_name parameter
		validBit = valVtnstationControllerSt
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VNODE_IF_NAME_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, interfaceObj,
					VtnServiceJsonConsts.IFNAME,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valVtnstationControllerSt,
							VtnServiceIpcConsts.VNODEIF_NAME));
		}

		// for if_status parameter
		validBit = valVtnstationControllerSt
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VNODE_IF_STATUS_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			if (UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_UP.getValue()
					.equals(IpcDataUnitWrapper.getIpcStructUint8Value(
							valVtnstationControllerSt,
							VtnServiceIpcConsts.VNODEIF_STATUS))) {
				setValueToJsonObject(validBit, interfaceObj,
						VtnServiceJsonConsts.OPERSTATUS,
						VtnServiceJsonConsts.UP);
			} else if (UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_DOWN
					.getValue().equals(
							IpcDataUnitWrapper.getIpcStructUint8Value(
									valVtnstationControllerSt,
									VtnServiceIpcConsts.VNODEIF_STATUS))) {
				setValueToJsonObject(validBit, interfaceObj,
						VtnServiceJsonConsts.OPERSTATUS,
						VtnServiceJsonConsts.DOWN);
			} else if (UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_UNKNOWN
					.getValue().equals(
							IpcDataUnitWrapper.getIpcStructUint8Value(
									valVtnstationControllerSt,
									VtnServiceIpcConsts.VNODEIF_STATUS))) {
				setValueToJsonObject(validBit, interfaceObj,
						VtnServiceJsonConsts.OPERSTATUS,
						VtnServiceJsonConsts.UNKNOWN);
			} else {
				LOG.debug("Operstatus :Invalid");
			}
			LOG.debug("Operstatus :"
					+ IpcDataUnitWrapper.getIpcStructUint8Value(
							valVtnstationControllerSt,
							VtnServiceIpcConsts.VNODEIF_STATUS));
		}

		vtnStation.add(VtnServiceJsonConsts.INTERFACE, interfaceObj);

		// for switch_id parameter
		validBit = valVtnstationControllerSt
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_DATAPATH_ID_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, vtnStation,
					VtnServiceJsonConsts.SWITCHID,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valVtnstationControllerSt,
							VtnServiceIpcConsts.SWITCHID));
		}

		// for port_name parameter
		validBit = valVtnstationControllerSt
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_PORT_NAME_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, vtnStation,
					VtnServiceJsonConsts.PORTNAME,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valVtnstationControllerSt,
							VtnServiceIpcConsts.PORTNAME));
		}

		// for vlan_id parameter
		validBit = valVtnstationControllerSt
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VLAN_ID_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			if (IpcDataUnitWrapper
					.getIpcStructUint16HexaValue(valVtnstationControllerSt,
							VtnServiceJsonConsts.VLANID)
					.equalsIgnoreCase(VtnServiceIpcConsts.VLAN_ID_DEFAULT_VALUE)) {
				vtnStation.addProperty(VtnServiceJsonConsts.NO_VLAN_ID,
						VtnServiceJsonConsts.TRUE);
			} else {
				setValueToJsonObject(validBit, vtnStation,
						VtnServiceJsonConsts.VLANID,
						IpcDataUnitWrapper.getIpcStructUint16Value(
								valVtnstationControllerSt,
								VtnServiceJsonConsts.VLANID));
			}
		}
		LOG.debug("vtnStation Json: " + vtnStation.toString());
		LOG.trace("Complete createVtnStationConstJson");
	}

	/**
	 * Create Json for statistics of VTN Station Show API
	 * 
	 * @param vtnStationStats
	 * @param valVtnstationControllerStat
	 */
	private void addVtnStationStatsData(final JsonObject vtnStationStats,
			final IpcStruct valVtnstationControllerStat) {
		LOG.trace("Start addVtnStationStatsData");
		/*
		 * prepare Json for openflow controller
		 */
		LOG.trace("Call createOfControllerJson to get OPENFLOWCONTROLLER info ");
		final JsonObject ofController = createOfControllerJson(valVtnstationControllerStat);

		/*
		 * prepare Json for openflow network
		 */
		LOG.trace("Call createOfNetworkJson to get OPENFLOWNW info ");
		final JsonObject ofNetwork = createOfNetworkJson(valVtnstationControllerStat);

		vtnStationStats.add(VtnServiceJsonConsts.OPENFLOWCONTROLLER,
				ofController);
		vtnStationStats.add(VtnServiceJsonConsts.OPENFLOWNW, ofNetwork);
		LOG.debug("Stats Json: " + vtnStationStats.toString());
		LOG.trace("Complete addVtnStationStatsData");
	}

	/**
	 * Create Json for open flow controller of VTN Station Show API
	 * 
	 * @param valVtnstationControllerStat
	 * @return
	 */
	private JsonObject createOfControllerJson(
			final IpcStruct valVtnstationControllerStat) {
		LOG.trace("Start createOfControllerJson");
		byte validBit;
		final JsonObject ofController = new JsonObject();

		// for all_rx Json
		final JsonObject allRx = new JsonObject();

		validBit = valVtnstationControllerStat
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnStationControllerStatIndex.UPLL_IDX_ALL_RX_PKT_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, allRx, VtnServiceJsonConsts.PACKETS,
					IpcDataUnitWrapper.getIpcStructUint64Value(
							valVtnstationControllerStat,
							VtnServiceIpcConsts.allRxPkt));
		}

		validBit = valVtnstationControllerStat
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnStationControllerStatIndex.UPLL_IDX_ALL_RX_BYTS_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, allRx, VtnServiceJsonConsts.OCTETS,
					IpcDataUnitWrapper.getIpcStructUint64Value(
							valVtnstationControllerStat,
							VtnServiceIpcConsts.allRxBytes));
		}

		ofController.add(VtnServiceJsonConsts.ALLRX, allRx);

		// for all_tx Json
		final JsonObject allTx = new JsonObject();

		validBit = valVtnstationControllerStat
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnStationControllerStatIndex.UPLL_IDX_ALL_TX_PKT_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, allTx, VtnServiceJsonConsts.PACKETS,
					IpcDataUnitWrapper.getIpcStructUint64Value(
							valVtnstationControllerStat,
							VtnServiceIpcConsts.allTxPkt));
		}

		validBit = valVtnstationControllerStat
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnStationControllerStatIndex.UPLL_IDX_ALL_TX_BYTS_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, allTx, VtnServiceJsonConsts.OCTETS,
					IpcDataUnitWrapper.getIpcStructUint64Value(
							valVtnstationControllerStat,
							VtnServiceIpcConsts.allTxBytes));
		}

		ofController.add(VtnServiceJsonConsts.ALLTX, allTx);
		LOG.debug("OPENFLOWCONTROLLER Json: " + ofController.toString());
		LOG.trace("Complete createOfControllerJson");
		return ofController;
	}

	/**
	 * Create Json for open flow network of VTN Station Show API
	 * 
	 * @param valVtnstationControllerStat
	 * @return
	 */
	private JsonObject createOfNetworkJson(
			final IpcStruct valVtnstationControllerStat) {
		LOG.trace("Start createOfNetworkJson");
		byte validBit;
		final JsonObject ofNetwork = new JsonObject();

		// for all_rx Json
		final JsonObject allRx = new JsonObject();

		validBit = valVtnstationControllerStat
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnStationControllerStatIndex.UPLL_IDX_ALL_NW_RX_PKT_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, allRx, VtnServiceJsonConsts.PACKETS,
					IpcDataUnitWrapper.getIpcStructUint64Value(
							valVtnstationControllerStat,
							VtnServiceIpcConsts.allNWRxPkt));
		}

		validBit = valVtnstationControllerStat
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnStationControllerStatIndex.UPLL_IDX_ALL_NW_RX_BYTS_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, allRx, VtnServiceJsonConsts.OCTETS,
					IpcDataUnitWrapper.getIpcStructUint64Value(
							valVtnstationControllerStat,
							VtnServiceIpcConsts.allNWRxBytes));
		}

		ofNetwork.add(VtnServiceJsonConsts.ALLRX, allRx);

		// for all_tx Json
		final JsonObject allTx = new JsonObject();

		validBit = valVtnstationControllerStat
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnStationControllerStatIndex.UPLL_IDX_ALL_NW_TX_PKT_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, allTx, VtnServiceJsonConsts.PACKETS,
					IpcDataUnitWrapper.getIpcStructUint64Value(
							valVtnstationControllerStat,
							VtnServiceIpcConsts.allNWTxPkt));
		}

		validBit = valVtnstationControllerStat
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnStationControllerStatIndex.UPLL_IDX_ALL_NW_TX_BYTS_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, allTx, VtnServiceJsonConsts.OCTETS,
					IpcDataUnitWrapper.getIpcStructUint64Value(
							valVtnstationControllerStat,
							VtnServiceIpcConsts.allNWTxBytes));
		}

		ofNetwork.add(VtnServiceJsonConsts.ALLTX, allTx);

		// for existing_rx Json
		final JsonObject existingRx = new JsonObject();

		validBit = valVtnstationControllerStat
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnStationControllerStatIndex.UPLL_IDX_EXST_RX_PKT_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, existingRx,
					VtnServiceJsonConsts.PACKETS,
					IpcDataUnitWrapper.getIpcStructUint64Value(
							valVtnstationControllerStat,
							VtnServiceIpcConsts.existingRxPkt));
		}

		validBit = valVtnstationControllerStat
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnStationControllerStatIndex.UPLL_IDX_EXST_RX_BYTS_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, existingRx,
					VtnServiceJsonConsts.OCTETS,
					IpcDataUnitWrapper.getIpcStructUint64Value(
							valVtnstationControllerStat,
							VtnServiceIpcConsts.existingRxBytes));
		}

		ofNetwork.add(VtnServiceJsonConsts.EXISTINGRX, existingRx);

		// for existing_tx Json
		final JsonObject existingTx = new JsonObject();

		validBit = valVtnstationControllerStat
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnStationControllerStatIndex.UPLL_IDX_EXST_TX_PKT_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, existingTx,
					VtnServiceJsonConsts.PACKETS,
					IpcDataUnitWrapper.getIpcStructUint64Value(
							valVtnstationControllerStat,
							VtnServiceIpcConsts.existingTxPkt));
		}

		validBit = valVtnstationControllerStat
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnStationControllerStatIndex.UPLL_IDX_EXST_TX_BYTS_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, existingTx,
					VtnServiceJsonConsts.OCTETS,
					IpcDataUnitWrapper.getIpcStructUint64Value(
							valVtnstationControllerStat,
							VtnServiceIpcConsts.existingTxBytes));
		}

		ofNetwork.add(VtnServiceJsonConsts.EXISTINGTX, existingTx);

		// for expired_rx Json
		final JsonObject expiredRx = new JsonObject();

		validBit = valVtnstationControllerStat
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnStationControllerStatIndex.UPLL_IDX_EXPD_RX_PKT_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, expiredRx,
					VtnServiceJsonConsts.PACKETS,
					IpcDataUnitWrapper.getIpcStructUint64Value(
							valVtnstationControllerStat,
							VtnServiceIpcConsts.expiredRxPkt));
		}

		validBit = valVtnstationControllerStat
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnStationControllerStatIndex.UPLL_IDX_EXPD_RX_BYTS_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, expiredRx,
					VtnServiceJsonConsts.OCTETS,
					IpcDataUnitWrapper.getIpcStructUint64Value(
							valVtnstationControllerStat,
							VtnServiceIpcConsts.expiredRxBytes));
		}

		ofNetwork.add(VtnServiceJsonConsts.EXPIREDRX, expiredRx);

		// for expired_tx Json
		final JsonObject expiredTx = new JsonObject();

		validBit = valVtnstationControllerStat
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnStationControllerStatIndex.UPLL_IDX_EXPD_TX_PKT_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, expiredTx,
					VtnServiceJsonConsts.PACKETS,
					IpcDataUnitWrapper.getIpcStructUint64Value(
							valVtnstationControllerStat,
							VtnServiceIpcConsts.expiredTxPkt));
		}

		validBit = valVtnstationControllerStat
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnStationControllerStatIndex.UPLL_IDX_EXPD_TX_BYTS_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, expiredTx,
					VtnServiceJsonConsts.OCTETS,
					IpcDataUnitWrapper.getIpcStructUint64Value(
							valVtnstationControllerStat,
							VtnServiceIpcConsts.expiredTxBytes));
		}

		ofNetwork.add(VtnServiceJsonConsts.EXPIREDTX, expiredTx);

		// for all_drop_rx Json
		final JsonObject allDropRx = new JsonObject();

		validBit = valVtnstationControllerStat
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnStationControllerStatIndex.UPLL_IDX_ALL_DRP_RX_PKT_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, allDropRx,
					VtnServiceJsonConsts.PACKETS,
					IpcDataUnitWrapper.getIpcStructUint64Value(
							valVtnstationControllerStat,
							VtnServiceIpcConsts.allDropRxPkt));
		}

		validBit = valVtnstationControllerStat
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnStationControllerStatIndex.UPLL_IDX_ALL_DRP_RX_BYTS_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, allDropRx,
					VtnServiceJsonConsts.OCTETS,
					IpcDataUnitWrapper.getIpcStructUint64Value(
							valVtnstationControllerStat,
							VtnServiceIpcConsts.allDropRxBytes));
		}

		ofNetwork.add(VtnServiceJsonConsts.ALLDROPRX, allDropRx);

		// for existing_drop_rx Json
		final JsonObject existingDropRx = new JsonObject();

		validBit = valVtnstationControllerStat
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnStationControllerStatIndex.UPLL_IDX_EXST_DRP_RX_PKT_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, existingDropRx,
					VtnServiceJsonConsts.PACKETS,
					IpcDataUnitWrapper.getIpcStructUint64Value(
							valVtnstationControllerStat,
							VtnServiceIpcConsts.existingDropRxPkt));
		}

		validBit = valVtnstationControllerStat
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnStationControllerStatIndex.UPLL_IDX_EXST_DRP_RX_BYTS_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, existingDropRx,
					VtnServiceJsonConsts.OCTETS,
					IpcDataUnitWrapper.getIpcStructUint64Value(
							valVtnstationControllerStat,
							VtnServiceIpcConsts.existingDropRxBytes));
		}

		ofNetwork.add(VtnServiceJsonConsts.EXISTINGDROPRX, existingDropRx);

		// for expired_drop_rx Json
		final JsonObject expiredDropRx = new JsonObject();

		validBit = valVtnstationControllerStat
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnStationControllerStatIndex.UPLL_IDX_EXPD_DRP_RX_PKT_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, expiredDropRx,
					VtnServiceJsonConsts.PACKETS,
					IpcDataUnitWrapper.getIpcStructUint64Value(
							valVtnstationControllerStat,
							VtnServiceIpcConsts.expiredDropRxPkt));
		}

		validBit = valVtnstationControllerStat
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnStationControllerStatIndex.UPLL_IDX_EXPD_DRP_RX_BYTS_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, expiredDropRx,
					VtnServiceJsonConsts.OCTETS,
					IpcDataUnitWrapper.getIpcStructUint64Value(
							valVtnstationControllerStat,
							VtnServiceIpcConsts.expiredDropRxBytes));
		}

		ofNetwork.add(VtnServiceJsonConsts.EXPIREDDROPRX, expiredDropRx);
		LOG.debug("OPENFLOWNW Json: " + ofNetwork.toString());
		LOG.trace("Complete createOfNetworkJson");
		return ofNetwork;
	}

	/**
	 * Create Json response for Show VTN Station API
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param show
	 * @return
	 */
	public JsonObject getVtnStationResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody, final String show) {

		LOG.trace("Start getVtnStationResponse");
		final JsonObject root = new JsonObject();
		JsonArray vtnStationsArray = null;

		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}

		final String rootJsonName = VtnServiceJsonConsts.VTNSTATIONS;
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject vtnStation = null;

		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			vtnStation = new JsonObject();
			vtnStation
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, vtnStation);
		} else {
			vtnStationsArray = new JsonArray();
			LOG.debug("Skip Key Type, Key Structure and Count of VTN-Stations");
			for (int index = 3; index < responsePacket.length;) {

				vtnStation = new JsonObject();
				byte validBit;

				final IpcStruct valVtnstationControllerSt = (IpcStruct) responsePacket[index++];
				createVtnStationConstJson(vtnStation, valVtnstationControllerSt);

				if (opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
					LOG.debug("op : detail");
					final IpcStruct valVtnstationControllerStat = (IpcStruct) responsePacket[index++];

					final JsonObject vtnStationStats = new JsonObject();
					addVtnStationStatsData(vtnStationStats,
							valVtnstationControllerStat);
					vtnStation.add(VtnServiceJsonConsts.STATISTICS,
							vtnStationStats);
				}

				// for ipaddrs parameter
				validBit = valVtnstationControllerSt
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_IPV4_COUNT_VSCS
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					final int ipv4_count = Integer.parseInt(IpcDataUnitWrapper
							.getIpcStructUint32Value(valVtnstationControllerSt,
									VtnServiceIpcConsts.IPV4_COUNT));
					final JsonArray ipaddrsArray = new JsonArray();
					for (int i = 0; i < ipv4_count; i++) {
						final JsonPrimitive ipaddrs = new JsonPrimitive(
								IpcDataUnitWrapper
										.getIpcDataUnitValue(responsePacket[index++]));
						ipaddrsArray.add(ipaddrs);
					}
					if (ipaddrsArray.size() > 0) {
						vtnStation.add(VtnServiceJsonConsts.IPADDRS,
								ipaddrsArray);
					}
					LOG.debug("count of ipv4 address : " + ipv4_count);

				}
				// for ipv6addrs parameter
				validBit = valVtnstationControllerSt
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_IPV6_COUNT_VSCS
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					final int ipv6_count = Integer.parseInt(IpcDataUnitWrapper
							.getIpcStructUint32Value(valVtnstationControllerSt,
									VtnServiceIpcConsts.IPV6_COUNT));
					final JsonArray ipaddrsArray = new JsonArray();
					for (int i = 0; i < ipv6_count; i++) {
						final JsonPrimitive ipaddrs = new JsonPrimitive(
								IpcDataUnitWrapper
										.getIpcDataUnitValue(responsePacket[index++]));
						ipaddrsArray.add(ipaddrs);
					}
					if (ipaddrsArray.size() > 0) {
						vtnStation.add(VtnServiceJsonConsts.IPV6ADDRS,
								ipaddrsArray);
					}
					LOG.debug("count of ipv6 address : " + ipv6_count);
				}

				// add current json object to array, if it has been initialized
				// earlier
				if (null != vtnStationsArray) {
					vtnStationsArray.add(vtnStation);
				}
			}
			root.add(rootJsonName, vtnStationsArray);
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVtnStationResponse");
		return root;
	}

	// for iproute
	/**
	 * Used for Show IpRoute response
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return JsonObject
	 */
	public JsonObject getIpRouteResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody, final String getType) {
		LOG.trace("Start getIpRouteResponse");
		final JsonObject root = new JsonObject();
		JsonArray ipRouteArray = null;

		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}

		String rootJsonName;

		rootJsonName = VtnServiceJsonConsts.IPROUTES;
		LOG.debug("Json Name :" + rootJsonName);

		ipRouteArray = new JsonArray();

		JsonObject ipRouteList = null;

		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			ipRouteList = new JsonObject();
			String count = VtnServiceConsts.ZERO;
			if (responsePacket.length != 0) {
				count = IpcDataUnitWrapper
						.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]);
			}
			ipRouteList.addProperty(VtnServiceJsonConsts.COUNT, count);
			root.add(rootJsonName, ipRouteList);
		} else {
			int index = 0;

			// There is no use of key type
			LOG.debug("Skip key type: no use");
			index++;

			/*
			 * skip key structure, no value need from this structure
			 */
			LOG.debug("Skip key Structure: no use");
			index++;
			// get count of value structure
			Long count = 0L;
			if (responsePacket.length != 0) {
				count = Long
						.valueOf(IpcDataUnitWrapper
								.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			}
			LOG.debug("Count of value structure: " + count);
			/*
			 * moving from count to value structure parameter
			 */
			index++;
			for (int indexValueStructure = 0; indexValueStructure < count; indexValueStructure++) {
				/*
				 * add valid informations from value structure
				 */
				ipRouteList = new JsonObject();
				byte validBit;
				final IpcStruct vaVvrtIpRouteSt = (IpcStruct) responsePacket[index++];

				validBit = vaVvrtIpRouteSt
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtIpRouteStIndex.UPLL_IDX_DESTINATION_VIRS
										.ordinal());

				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					setValueToJsonObject(validBit, ipRouteList,
							VtnServiceJsonConsts.DSTADDR,
							IpcDataUnitWrapper.getIpcStructIpv4Value(
									vaVvrtIpRouteSt,
									VtnServiceIpcConsts.DESTINATION));
				}
				validBit = vaVvrtIpRouteSt
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtIpRouteStIndex.UPLL_IDX_GATEWAY_VIRS
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					setValueToJsonObject(validBit, ipRouteList,
							VtnServiceJsonConsts.GATEWAY,
							IpcDataUnitWrapper.getIpcStructIpv4Value(
									vaVvrtIpRouteSt,
									VtnServiceJsonConsts.GATEWAY));
				}
				validBit = vaVvrtIpRouteSt
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtIpRouteStIndex.UPLL_IDX_PREFIXLEN_VIRS
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					setValueToJsonObject(validBit, ipRouteList,
							VtnServiceJsonConsts.PREFIX,
							IpcDataUnitWrapper.getIpcStructUint8Value(
									vaVvrtIpRouteSt,
									VtnServiceIpcConsts.PREFIXLEN));
				}
				validBit = vaVvrtIpRouteSt
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtIpRouteStIndex.UPLL_IDX_FLAGS_VIRS
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					setValueToJsonObject(validBit, ipRouteList,
							VtnServiceJsonConsts.FLAGS,
							IpcDataUnitWrapper
									.getIpcStructUint16HexaValue(
											vaVvrtIpRouteSt,
											VtnServiceJsonConsts.FLAGS));
				}
				validBit = vaVvrtIpRouteSt
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtIpRouteStIndex.UPLL_IDX_GR_METRIC_VIRS
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					setValueToJsonObject(validBit, ipRouteList,
							VtnServiceJsonConsts.METRIC,
							IpcDataUnitWrapper.getIpcStructUint16Value(
									vaVvrtIpRouteSt,
									VtnServiceJsonConsts.METRIC));
				}
				validBit = vaVvrtIpRouteSt
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtIpRouteStIndex.UPLL_IDX_USE_VIRS
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					setValueToJsonObject(validBit, ipRouteList,
							VtnServiceJsonConsts.USE,
							IpcDataUnitWrapper.getIpcStructUint32Value(
									vaVvrtIpRouteSt, VtnServiceJsonConsts.USE));
				}
				validBit = vaVvrtIpRouteSt
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtIpRouteStIndex.UPLL_IDX_IF_NAME_VIRS
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					setValueToJsonObject(validBit, ipRouteList,
							VtnServiceJsonConsts.IFNAME,
							IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
									vaVvrtIpRouteSt,
									VtnServiceJsonConsts.IFNAME));
				}
				validBit = vaVvrtIpRouteSt
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtIpRouteStIndex.UPLL_IDX_NW_MONITOR_GR_VIRS
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					setValueToJsonObject(validBit, ipRouteList,
							VtnServiceJsonConsts.NMG_NAME,
							IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
									vaVvrtIpRouteSt,
									VtnServiceIpcConsts.NWMONITOR_GR));
				}
				validBit = vaVvrtIpRouteSt
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtIpRouteStIndex.UPLL_IDX_GR_METRIC_VIRS
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					setValueToJsonObject(validBit, ipRouteList,
							VtnServiceJsonConsts.GROUPMETRIC,
							IpcDataUnitWrapper.getIpcStructUint16Value(
									vaVvrtIpRouteSt,
									VtnServiceIpcConsts.GROUP_METRIC));
				}

				// add current json object to array, if it has been
				// initialized
				// earlier
				if (null != ipRouteArray) {
					ipRouteArray.add(ipRouteList);
				}
			}

			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != ipRouteArray) {
				root.add(rootJsonName, ipRouteArray);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getIpRouteResponse");
		return root;
	}

	// U12 Implementaion

	/**
	 * Used for Vtnmapping Response
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return JsonObject
	 */
	public JsonObject getVtnMappingResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody, final String getType) {

		LOG.trace("Start getVtnMappingResponse");
		final JsonObject root = new JsonObject();
		JsonObject vtnMapping = null;
		JsonArray vtnMappingArray = null;
		JsonArray vtnMappingInfosJsonArray = null;

		LOG.debug("getType: " + getType);
		String rootJsonName;
		/*
		 * get type (show or list) will be required to resolve root json name
		 * here it will be mapping for show and mappings for list
		 */
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.MAPPING;
		} else {
			rootJsonName = VtnServiceJsonConsts.MAPPINGS;
			// json array will be required for list type of cases
			vtnMappingArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);

		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}

		for (int index = 0; index < responsePacket.length; index++) {
			vtnMapping = new JsonObject();
			LOG.debug("initial index :" + index);
			// ignore key-type
			index++;
			/*
			 * add mandatory informations from key structure
			 */
			LOG.debug("domainId index :" + index);
			final IpcStruct keyVtnControllerStruct = (IpcStruct) responsePacket[index++];
			final String controllerId = IpcDataUnitWrapper
					.getIpcStructUint8ArrayValue(keyVtnControllerStruct,
							VtnServiceIpcConsts.CONTROLLERNAME);
			final String domainId = IpcDataUnitWrapper
					.getIpcStructUint8ArrayValue(keyVtnControllerStruct,
							VtnServiceIpcConsts.DOMAINID);

			vtnMapping.addProperty(VtnServiceJsonConsts.MAPPINGID, controllerId
					+ VtnServiceJsonConsts.HYPHEN + domainId);

			vtnMapping.addProperty(VtnServiceJsonConsts.CONTROLLERID,
					controllerId);
			vtnMapping.addProperty(VtnServiceJsonConsts.DOMAINID, domainId);
			// get count of value structure
			LOG.debug("count index :" + index);
			final int count = Integer.valueOf(IpcDataUnitWrapper
					.getIpcDataUnitValue(responsePacket[index++]));

			if (opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)
					|| getType.equalsIgnoreCase(VtnServiceJsonConsts.SHOW)) {
				vtnMappingInfosJsonArray = new JsonArray();

				for (int valIndex = 0; valIndex < count; valIndex++) {
					final JsonObject vtnMappingInfojsonObject = new JsonObject();
					/*
					 * this part is always required in Show, but not required in
					 * List + "normal" op type
					 */

					byte validBit;
					/*
					 * add valid informations from value structure
					 */
					LOG.debug("valVtnMappingStruct index :" + index);
					final IpcStruct valVtnMappingStruct = (IpcStruct) responsePacket[index++];
					validBit = valVtnMappingStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.valVtnMappingControllerStIndex.UPLL_IDX_SWITCH_ID_VMCS
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit,
								vtnMappingInfojsonObject,
								VtnServiceJsonConsts.SWITCHID,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVtnMappingStruct,
										VtnServiceIpcConsts.SWITCHID));
					}
					validBit = valVtnMappingStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.valVtnMappingControllerStIndex.UPLL_IDX_PORT_NAME_VMCS
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit,
								vtnMappingInfojsonObject,
								VtnServiceJsonConsts.PORTNAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVtnMappingStruct,
										VtnServiceIpcConsts.PORTNAME));
					}

					validBit = valVtnMappingStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.valVtnMappingControllerStIndex.UPLL_IDX_LOGICAL_PORT_ID_VMCS
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit,
								vtnMappingInfojsonObject,
								VtnServiceJsonConsts.LOGICAL_PORT_ID,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVtnMappingStruct,
										VtnServiceIpcConsts.LOGICAL_PORT_ID));
					}

					validBit = valVtnMappingStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.valVtnMappingControllerStIndex.UPLL_IDX_VLAN_ID_VMCS
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {

						String vLanIdValue = IpcDataUnitWrapper
								.getIpcStructUint16Value(valVtnMappingStruct,
										VtnServiceIpcConsts.VLANID);

						if (!vLanIdValue
								.equalsIgnoreCase(VtnServiceJsonConsts.VLAN_ID_65535)) {
							setValueToJsonObject(validBit,
									vtnMappingInfojsonObject,
									VtnServiceJsonConsts.VLANID, vLanIdValue);
						}
					}
					validBit = valVtnMappingStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.valVtnMappingControllerStIndex.UPLL_IDX_TAGGED_VMCS
											.ordinal());

					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valVtnMappingStruct,
										VtnServiceIpcConsts.TAGGED)
								.equals(UncStructIndexEnum.vlan_tagged.UPLL_VLAN_TAGGED
										.getValue())) {
							setValueToJsonObject(validBit,
									vtnMappingInfojsonObject,
									VtnServiceJsonConsts.TAGGED,
									VtnServiceJsonConsts.TRUE);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valVtnMappingStruct,
										VtnServiceIpcConsts.TAGGED)
								.equals(UncStructIndexEnum.vlan_tagged.UPLL_VLAN_UNTAGGED
										.getValue())) {
							setValueToJsonObject(validBit,
									vtnMappingInfojsonObject,
									VtnServiceJsonConsts.TAGGED,
									VtnServiceJsonConsts.FALSE);
						}
						LOG.debug("Tagged :"
								+ IpcDataUnitWrapper.getIpcStructUint8Value(
										valVtnMappingStruct,
										VtnServiceIpcConsts.TAGGED));
					}

					validBit = valVtnMappingStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.valVtnMappingControllerStIndex.UPLL_IDX_MAP_TYPE_VMCS
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {

						if (UncStructIndexEnum.ValVbrIfMapType.UPLL_IF_OFS_MAP
								.getValue()
								.equals(IpcDataUnitWrapper
										.getIpcStructUint8Value(
												valVtnMappingStruct,
												VtnServiceIpcConsts.MAP_TYPE))) {
							setValueToJsonObject(validBit,
									vtnMappingInfojsonObject,
									VtnServiceJsonConsts.MAPTYPE,
									VtnServiceJsonConsts.PORTMAP);
						} else if (UncStructIndexEnum.ValVbrIfMapType.UPLL_IF_VLAN_MAP
								.getValue()
								.equals(IpcDataUnitWrapper
										.getIpcStructUint8Value(
												valVtnMappingStruct,
												VtnServiceIpcConsts.MAP_TYPE))) {
							setValueToJsonObject(validBit,
									vtnMappingInfojsonObject,
									VtnServiceJsonConsts.MAPTYPE,
									VtnServiceJsonConsts.VLANMAP);
						} else {
							LOG.debug("MapType : invalid");
						}
						LOG.debug("MapType :"
								+ IpcDataUnitWrapper.getIpcStructUint8Value(
										valVtnMappingStruct,
										VtnServiceIpcConsts.MAP_TYPE));

					}
					// for vnode_type parameter
					validBit = valVtnMappingStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.valVtnMappingControllerStIndex.UPLL_IDX_VNODE_TYPE_VMCS
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {

						int vnodeVal = Integer.parseInt(IpcDataUnitWrapper
								.getIpcStructUint8Value(
										valVtnMappingStruct,
										VtnServiceIpcConsts.VNODETYPE));
						String vnodeType = null;
						if (vnodeVal == UncStructIndexEnum.ValVnodeType.UPLL_VNODE_VBRIDGE
								.ordinal()) {
							vnodeType = VtnServiceJsonConsts.VBRIDGE;
						} else if (vnodeVal == UncStructIndexEnum.ValVnodeType.UPLL_VNODE_VROUTER
								.ordinal()) {
							vnodeType = VtnServiceJsonConsts.VROUTER;
						} else if (vnodeVal == UncStructIndexEnum.ValVnodeType.UPLL_VNODE_VTEP
								.ordinal()) {
							vnodeType = VtnServiceJsonConsts.VTEP;
						} else if (vnodeVal == UncStructIndexEnum.ValVnodeType.UPLL_VNODE_VTERMINAL
								.ordinal()) {
							vnodeType = VtnServiceJsonConsts.VTERMINAL;
						} else if (vnodeVal == UncStructIndexEnum.ValVnodeType.UPLL_VNODE_VTUNNEL
								.ordinal()) {
							vnodeType = VtnServiceJsonConsts.VTUNNEL;
						} else if (vnodeVal == UncStructIndexEnum.ValVnodeType.UPLL_VNODE_VUNKNOWN
								.ordinal()) {
							vnodeType = VtnServiceJsonConsts.VBYPASS;
						}
						setValueToJsonObject(validBit,
								vtnMappingInfojsonObject,
								VtnServiceJsonConsts.VNODETYPE, vnodeType);
					}
					// for vnode_name parameter
					validBit = valVtnMappingStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.valVtnMappingControllerStIndex.UPLL_IDX_VNODE_NAME_VMCS
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit,
								vtnMappingInfojsonObject,
								VtnServiceJsonConsts.VNODENAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVtnMappingStruct,
										VtnServiceIpcConsts.VNODENAME));
					}

					validBit = valVtnMappingStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.valVtnMappingControllerStIndex.UPLL_IDX_VNODE_IF_NAME_VMCS
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit,
								vtnMappingInfojsonObject,
								VtnServiceJsonConsts.IFNAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVtnMappingStruct,
										VtnServiceIpcConsts.VNODEIF_NAME));
					}

					vtnMappingInfosJsonArray.add(vtnMappingInfojsonObject);
				}
				vtnMapping.add(VtnServiceJsonConsts.MAPPINGINFOS,
						vtnMappingInfosJsonArray);
			} else {
				vtnMapping.remove(VtnServiceJsonConsts.CONTROLLERID);
				vtnMapping.remove(VtnServiceJsonConsts.DOMAINID);
				index = index + count;
			}
			if (null != vtnMappingArray) {
				vtnMappingArray.add(vtnMapping);
			}
		}
		/*
		 * finally add either array or single object to root json object and
		 * return the same.
		 */
		if (null != vtnMappingArray) {
			LOG.debug("List VTN Mapping JSON :" + vtnMappingArray);
			root.add(rootJsonName, vtnMappingArray);
		} else {
			LOG.debug("Show VTN Mapping JSON :" + vtnMapping);
			root.add(rootJsonName, vtnMapping);
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVtnMappingResponse");

		return root;
	}

	/**
	 * Used for Show VTN Data Flow response
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return JsonObject
	 */
	public JsonObject getVtnDataFlowResponse(
			final IpcDataUnit[] responsePacket, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start getVtnDataFlowResponse");
		final JsonObject root = new JsonObject();
		final JsonArray vtnDataFlowArray = new JsonArray();
		if (responsePacket.length != 0) {
			int index = 2;
			final int totalFlowCount = Integer.parseInt(IpcDataUnitWrapper
					.getIpcDataUnitValue(responsePacket[index++]));

			for (int i = 0; i < totalFlowCount; i++) {
				final IpcStruct valVtnDataFlowStruct = (IpcStruct) responsePacket[index++];
				LOG.debug("totalFlowCount:" + totalFlowCount);
				final JsonObject vtnDataflow = new JsonObject();
				byte validBit = 0;
				validBit = valVtnDataFlowStruct
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.valVtnDataflowIndex.UPLL_IDX_REASON_VVD
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					{
						final int reason = Integer.parseInt(IpcDataUnitWrapper
								.getIpcStructUint32Value(valVtnDataFlowStruct,
										VtnServiceIpcConsts.REASON));
						// getting type of reason field a
						String reasonForJson = VtnServiceConsts.EMPTY_STRING;
						if (reason == UncStructIndexEnum.UncDataflowReason.UNC_DF_RES_SUCCESS
								.ordinal()) {
							reasonForJson = VtnServiceJsonConsts.REASON_SUCCESS;
						} else if (reason == UncStructIndexEnum.UncDataflowReason.UNC_DF_RES_OPERATION_NOT_SUPPORTED
								.ordinal()) {
							reasonForJson = VtnServiceJsonConsts.REASON_NOT_SUPP;
						} else if (reason == UncStructIndexEnum.UncDataflowReason.UNC_DF_RES_EXCEEDS_FLOW_LIMIT
								.ordinal()) {
							reasonForJson = VtnServiceJsonConsts.REASON_EXCD_LIM;
						} else if (reason == UncStructIndexEnum.UncDataflowReason.UNC_DF_RES_CTRLR_DISCONNECTED
								.ordinal()) {
							reasonForJson = VtnServiceJsonConsts.REASON_CTRL_DISC;
						} else if (reason == UncStructIndexEnum.UncDataflowReason.UNC_DF_RES_EXCEEDS_HOP_LIMIT
								.ordinal()) {
							reasonForJson = VtnServiceJsonConsts.REASON_EXCD_HOP;
						} else if (reason == UncStructIndexEnum.UncDataflowReason.UNC_DF_RES_DST_NOT_REACHED
								.ordinal()) {
							reasonForJson = VtnServiceJsonConsts.REASON_DST_NOT_REACHED;
						} else if (reason == UncStructIndexEnum.UncDataflowReason.UNC_DF_RES_FLOW_NOT_FOUND
								.ordinal()) {
							reasonForJson = VtnServiceJsonConsts.REASON_FLOW_NOTFOUND;
						} else if (reason == UncStructIndexEnum.UncDataflowReason.UNC_DF_RES_SYSTEM_ERROR
								.ordinal()) {
							reasonForJson = VtnServiceJsonConsts.REASON_SYS_ERROR;
						}
						// assigning reason field in dataflow Json
						setValueToJsonObject(validBit, vtnDataflow,
								VtnServiceJsonConsts.REASON, reasonForJson);
						LOG.debug("reason:" + reasonForJson);
					}
				}
				final JsonArray ctrlDomainDataFlowArray = new JsonArray();
				validBit = valVtnDataFlowStruct
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.valVtnDataflowIndex.UPLL_IDX_CTRLR_DOMAIN_COUNT_VVD
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					final int controllerDomainCount = Integer
							.parseInt(IpcDataUnitWrapper
									.getIpcStructUint32Value(
											valVtnDataFlowStruct,
											VtnServiceIpcConsts.CONTROLLER_DOMAIN_COUNT));
					LOG.debug("Controller Domain Count:"
							+ controllerDomainCount);

					for (int j = 0; j < controllerDomainCount; j++) {
						final AtomicInteger atomicIndex = new AtomicInteger(
								index);
						ctrlDomainDataFlowArray
								.add(getControllerDomainDataFlow(
										responsePacket, atomicIndex,
										requestBody));
						index = atomicIndex.get();
					}

					vtnDataflow.add(
							VtnServiceJsonConsts.CONTROLLER_DOMAIN_DATAFLOWS,
							ctrlDomainDataFlowArray);
					LOG.debug("VTN Data Flow Json:" + vtnDataflow);
				}
				vtnDataFlowArray.add(vtnDataflow);
				LOG.debug("vtn dataFlowArray Json:" + vtnDataFlowArray);
			}
		}
		root.add(VtnServiceJsonConsts.DATAFLOWS, vtnDataFlowArray);
		LOG.debug("root Json :" + root);
		LOG.trace("Complete getVtnDataFlowResponse");
		return root;

	}

	/**
	 * get ControllerDomainDataFlow method is Used for VtnDataFlow Response
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return JsonObject
	 */
	private JsonObject getControllerDomainDataFlow(
			final IpcDataUnit[] responsePacket, final AtomicInteger index,
			final JsonObject requestBody) {
		LOG.trace("getControllerDomainDataFlow started");
		byte validBit;
		final JsonObject controllerDomainFlow = new JsonObject();
		final IpcStruct valVtnDataFlowCmnStruct = (IpcStruct) responsePacket[index
				.getAndIncrement()];
		validBit = valVtnDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.valVtnDataflowCmnIndex.UPLL_IDX_CONTROLLER_ID_VVDC
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controllerDomainFlow,
					VtnServiceJsonConsts.CONTROLLERID,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valVtnDataFlowCmnStruct,
							VtnServiceIpcConsts.CONTROLLER_ID));
		}

		validBit = valVtnDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.valVtnDataflowCmnIndex.UPLL_IDX_CONTROLLER_TYPE_VVDC
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			if (IpcDataUnitWrapper
					.getIpcStructUint8Value(valVtnDataFlowCmnStruct,
							VtnServiceIpcConsts.CONTROLER_TYPE)
					.equals(UncPhysicalStructIndexEnum.UpplTypeIndex.UNC_CT_UNKNOWN
							.getValue())) {
				setValueToJsonObject(validBit, controllerDomainFlow,
						VtnServiceJsonConsts.CONTROLER_TYPE,
						VtnServiceJsonConsts.BYPASS);

			} else if (IpcDataUnitWrapper
					.getIpcStructUint8Value(valVtnDataFlowCmnStruct,
							VtnServiceIpcConsts.CONTROLER_TYPE).equals(
							UncPhysicalStructIndexEnum.UpplTypeIndex.UNC_CT_PFC
									.getValue())) {
				setValueToJsonObject(validBit, controllerDomainFlow,
						VtnServiceJsonConsts.CONTROLER_TYPE,
						VtnServiceJsonConsts.PFC);

			} else if (IpcDataUnitWrapper
					.getIpcStructUint8Value(valVtnDataFlowCmnStruct,
							VtnServiceIpcConsts.CONTROLER_TYPE).equals(
							UncPhysicalStructIndexEnum.UpplTypeIndex.UNC_CT_VNP
									.getValue())) {
				setValueToJsonObject(validBit, controllerDomainFlow,
						VtnServiceJsonConsts.CONTROLER_TYPE,
						VtnServiceJsonConsts.VNP);

			} else if (IpcDataUnitWrapper
					.getIpcStructUint8Value(valVtnDataFlowCmnStruct,
							VtnServiceIpcConsts.CONTROLER_TYPE).equals(
							UncPhysicalStructIndexEnum.UpplTypeIndex.UNC_CT_POLC
									.getValue())) {
				String polc = VtnServiceInitManager.getConfigurationMap()
						.getCommonConfigValue(VtnServiceConsts.CONF_FILE_FIELD_POLC);
				setValueToJsonObject(validBit, controllerDomainFlow,
						VtnServiceJsonConsts.CONTROLER_TYPE,
						polc);

			} else if (IpcDataUnitWrapper
					.getIpcStructUint8Value(valVtnDataFlowCmnStruct,
							VtnServiceIpcConsts.CONTROLER_TYPE).equals(
							UncPhysicalStructIndexEnum.UpplTypeIndex.UNC_CT_HPVANC
									.getValue())) {
				String hpctr = VtnServiceInitManager.getConfigurationMap()
						.getCommonConfigValue(VtnServiceConsts.CONF_FILE_FIELD_HPVANC);
				setValueToJsonObject(validBit, controllerDomainFlow,
						VtnServiceJsonConsts.CONTROLER_TYPE,
						hpctr);
			} else if (IpcDataUnitWrapper
					.getIpcStructUint8Value(valVtnDataFlowCmnStruct,
							VtnServiceIpcConsts.CONTROLER_TYPE).equals(
							UncPhysicalStructIndexEnum.UpplTypeIndex.UNC_CT_ODC
									.getValue())) {
				String odc = VtnServiceInitManager.getConfigurationMap()
						.getCommonConfigValue(VtnServiceConsts.CONF_FILE_FIELD_ODC);
				setValueToJsonObject(validBit, controllerDomainFlow,
						VtnServiceJsonConsts.CONTROLER_TYPE,
						odc);
			} else {
				LOG.info("Controller Type invalid");
			}
			LOG.debug("Controller Type :"
					+ IpcDataUnitWrapper.getIpcStructUint8Value(
							valVtnDataFlowCmnStruct,
							VtnServiceIpcConsts.CONTROLER_TYPE));
		}

		validBit = valVtnDataFlowCmnStruct.getByte(VtnServiceIpcConsts.VALID,
				UncStructIndexEnum.valVtnDataflowCmnIndex.UPLL_IDX_FLOW_ID_VVDC
						.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controllerDomainFlow,
					VtnServiceJsonConsts.FLOW_ID,
					IpcDataUnitWrapper.getIpcStructUint64Value(
							valVtnDataFlowCmnStruct,
							VtnServiceIpcConsts.FLOW_ID));
		}

		validBit = valVtnDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.valVtnDataflowCmnIndex.UPLL_IDX_CREATED_TIME_VVDC
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controllerDomainFlow,
					VtnServiceJsonConsts.CREATEDTIME,
					IpcDataUnitWrapper.getIpcStructUint64Value(
							valVtnDataFlowCmnStruct,
							VtnServiceIpcConsts.CREATED_TIME));
		}

		validBit = valVtnDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.valVtnDataflowCmnIndex.UPLL_IDX_IDLE_TIMEOUT_VVDC
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controllerDomainFlow,
					VtnServiceJsonConsts.IDLETIMEOUT,
					IpcDataUnitWrapper.getIpcStructUint32Value(
							valVtnDataFlowCmnStruct,
							VtnServiceIpcConsts.IDLE_TIMEOUT));
		}

		validBit = valVtnDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.valVtnDataflowCmnIndex.UPLL_IDX_HARD_TIMEOUT_VVDC
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controllerDomainFlow,
					VtnServiceJsonConsts.HARDTIMEOUT,
					IpcDataUnitWrapper.getIpcStructUint32Value(
							valVtnDataFlowCmnStruct,
							VtnServiceIpcConsts.HARD_TIMEOUT));
		}

		validBit = valVtnDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.valVtnDataflowCmnIndex.UPLL_IDX_INGRESS_VNODE_VVDC
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controllerDomainFlow,
					VtnServiceJsonConsts.INGRESS_VNODE_NAME,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valVtnDataFlowCmnStruct,
							VtnServiceIpcConsts.INGRESS_VNODE));
		}

		validBit = valVtnDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.valVtnDataflowCmnIndex.UPLL_IDX_INGRESS_VINTERFACE_VVDC
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controllerDomainFlow,
					VtnServiceJsonConsts.INGRESS_IF_NAME,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valVtnDataFlowCmnStruct,
							VtnServiceIpcConsts.INGRESS_VINTERFACE));
		}

		validBit = valVtnDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.valVtnDataflowCmnIndex.UPLL_IDX_INGRESS_SWITCH_ID_VVDC
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controllerDomainFlow,
					VtnServiceJsonConsts.INGRESS_SWITCH_ID,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valVtnDataFlowCmnStruct,
							VtnServiceIpcConsts.INGRESS_SWITCH_ID));
		}

		validBit = valVtnDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.valVtnDataflowCmnIndex.UPLL_IDX_INGRESS_PORT_ID_VVDC
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controllerDomainFlow,
					VtnServiceJsonConsts.INGRESS_PORT_NAME,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valVtnDataFlowCmnStruct,
							VtnServiceIpcConsts.INGRESS_PORT_ID));
		}

		validBit = valVtnDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.valVtnDataflowCmnIndex.UPLL_IDX_INGRESS_LOGICAL_PORT_ID_VVDC
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controllerDomainFlow,
					VtnServiceJsonConsts.INGRESS_LOGICAL_PORT_ID,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valVtnDataFlowCmnStruct,
							VtnServiceIpcConsts.INGRESS_LOGICAL_PORT_ID));
		}

		validBit = valVtnDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.valVtnDataflowCmnIndex.UPLL_IDX_INGRESS_DOMAIN_VVDC
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controllerDomainFlow,
					VtnServiceJsonConsts.INGRESS_DOMAIN_ID,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valVtnDataFlowCmnStruct,
							VtnServiceIpcConsts.INGRESS_DOMAIN));
		}

		validBit = valVtnDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.valVtnDataflowCmnIndex.UPLL_IDX_EGRESS_VNODE_VVDC
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controllerDomainFlow,
					VtnServiceJsonConsts.EGRESS_VNODE_NAME,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valVtnDataFlowCmnStruct,
							VtnServiceIpcConsts.EGRESS_VNODE));
		}

		validBit = valVtnDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.valVtnDataflowCmnIndex.UPLL_IDX_EGRESS_VINTERFACE_VVDC
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controllerDomainFlow,
					VtnServiceJsonConsts.EGRESS_IF_NAME,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valVtnDataFlowCmnStruct,
							VtnServiceIpcConsts.EGRESS_VINTERFACE));
		}

		validBit = valVtnDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.valVtnDataflowCmnIndex.UPLL_IDX_EGRESS_SWITCH_ID_VVDC
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controllerDomainFlow,
					VtnServiceJsonConsts.EGRESS_SWITCH_ID,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valVtnDataFlowCmnStruct,
							VtnServiceIpcConsts.EGRESS_SWITCH_ID));
		}

		validBit = valVtnDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.valVtnDataflowCmnIndex.UPLL_IDX_EGRESS_PORT_ID_VVDC
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controllerDomainFlow,
					VtnServiceJsonConsts.EGRESS_PORT_NAME,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valVtnDataFlowCmnStruct,
							VtnServiceIpcConsts.EGRESS_PORT_ID));
		}

		validBit = valVtnDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.valVtnDataflowCmnIndex.UPLL_IDX_EGRESS_LOGICAL_PORT_ID_VVDC
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controllerDomainFlow,
					VtnServiceJsonConsts.EGRESS_LOGICAL_PORT_ID,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valVtnDataFlowCmnStruct,
							VtnServiceIpcConsts.EGRESS_LOGICAL_PORT_ID));
		}

		validBit = valVtnDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.valVtnDataflowCmnIndex.UPLL_IDX_EGRESS_DOMAIN_VVDC
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controllerDomainFlow,
					VtnServiceJsonConsts.EGRESS_DOMAIN_ID,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valVtnDataFlowCmnStruct,
							VtnServiceIpcConsts.EGRESS_DOMAIN));
		}

		validBit = valVtnDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.valVtnDataflowCmnIndex.UPLL_IDX_MATCH_COUNT_VVDC
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {

			controllerDomainFlow.add(
					VtnServiceJsonConsts.MATCH,
					getDataFlowMatchInfo(responsePacket, index, validBit,
							controllerDomainFlow, valVtnDataFlowCmnStruct));

		}
		validBit = valVtnDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.valVtnDataflowCmnIndex.UPLL_IDX_ACTION_COUNT_VVDC
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			controllerDomainFlow.add(
					VtnServiceJsonConsts.ACTION,
					getDataFlowActionInfo(responsePacket, index,
							controllerDomainFlow, valVtnDataFlowCmnStruct));
			LOG.debug(" Controller domain data flow Json :"
					+ controllerDomainFlow);
		}

		validBit = valVtnDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.valVtnDataflowCmnIndex.UPLL_IDX_PATH_INFO_COUNT_VVDC
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {

			controllerDomainFlow.add(
					VtnServiceJsonConsts.PATHINFOS,
					getDataFlowPathInfo(responsePacket, index,
							valVtnDataFlowCmnStruct));
			LOG.debug(" Controller domain data flow Json :"
					+ controllerDomainFlow);
		}
		LOG.trace("getControllerDomainDataFlow completed");
		return controllerDomainFlow;
	}

	/**
	 * This method is Used to generate Flow path info Json
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return JsonObject
	 */
	private JsonArray getDataFlowPathInfo(final IpcDataUnit[] responsePacket,
			final AtomicInteger index, final IpcStruct valVtnDataFlowCmnStruct) {
		LOG.trace("getDataFlowPathInfo stated");
		final JsonArray pathInfoArray = new JsonArray();
		final int pathInfoCount = Integer.parseInt(IpcDataUnitWrapper
				.getIpcStructUint32Value(valVtnDataFlowCmnStruct,
						VtnServiceIpcConsts.PATH_INFO_COUNT));
		LOG.debug("path_info_count:" + pathInfoCount);
		for (int k = 0; k < pathInfoCount; k++) {
			final JsonObject pathInfoBoundry = new JsonObject();
			byte validBit;
			final IpcStruct valVtnDataFlowPathInfo = (IpcStruct) responsePacket[index
					.getAndIncrement()];
			validBit = valVtnDataFlowPathInfo
					.getByte(
							VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.valVtnDataflowPathInfo.UPLL_IDX_IN_VNODE_VVDPI
									.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, pathInfoBoundry,
						VtnServiceJsonConsts.IN_VNODE_NAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valVtnDataFlowPathInfo,
								VtnServiceIpcConsts.IN_VNODE));

			}

			validBit = valVtnDataFlowPathInfo
					.getByte(
							VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.valVtnDataflowPathInfo.UPLL_IDX_IN_VIF_VVDPI
									.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, pathInfoBoundry,
						VtnServiceJsonConsts.IN_IF_NAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valVtnDataFlowPathInfo,
								VtnServiceIpcConsts.IN_VIF));

			}

			validBit = valVtnDataFlowPathInfo
					.getByte(
							VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.valVtnDataflowPathInfo.UPLL_IDX_OUT_VNODE_VVDPI
									.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, pathInfoBoundry,
						VtnServiceJsonConsts.OUT_VNODE_NAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valVtnDataFlowPathInfo,
								VtnServiceIpcConsts.OUT_VNODE));

			}

			validBit = valVtnDataFlowPathInfo
					.getByte(
							VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.valVtnDataflowPathInfo.UPLL_IDX_OUT_VIF_VVDPI
									.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, pathInfoBoundry,
						VtnServiceJsonConsts.OUT_IF_NAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valVtnDataFlowPathInfo,
								VtnServiceIpcConsts.OUT_VIF));

			}

			validBit = valVtnDataFlowPathInfo
					.getByte(
							VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.valVtnDataflowPathInfo.UPLL_IDX_VLINK_FLAG_VVDPI
									.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {

				final int linkValue = Integer.parseInt(IpcDataUnitWrapper
						.getIpcStructUint8Value(valVtnDataFlowPathInfo,
								VtnServiceIpcConsts.VLINK_FLAG));
				String link = VtnServiceConsts.EMPTY_STRING;
				if (linkValue == UncStructIndexEnum.valVtnDataflowPathInfoVlinkType.UPLL_DATAFLOW_PATH_VLINK_NOT_EXISTS
						.ordinal()) {
					link = VtnServiceJsonConsts.NOT_EXISTS;
				} else if (linkValue == UncStructIndexEnum.valVtnDataflowPathInfoVlinkType.UPLL_DATAFLOW_PATH_VLINK_EXISTS
						.ordinal()) {
					link = VtnServiceJsonConsts.EXISTS;
				}
				setValueToJsonObject(validBit, pathInfoBoundry,
						VtnServiceJsonConsts.VLINK_FLAG, link);
			}
			validBit = valVtnDataFlowPathInfo
					.getByte(
							VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.valVtnDataflowPathInfo.UPLL_IDX_STATUS_VVDPI
									.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {

				final int statusValue = Integer.parseInt(IpcDataUnitWrapper
						.getIpcStructUint8Value(valVtnDataFlowPathInfo,
								VtnServiceIpcConsts.STATUS));
				String status = VtnServiceConsts.EMPTY_STRING;
				if (statusValue == UncStructIndexEnum.valVtnDataflowPathInfoStatusType.UPLL_DATAFLOW_PATH_STATUS_NORMAL
						.ordinal()) {
					status = VtnServiceJsonConsts.NORMAL;
				} else if (statusValue == UncStructIndexEnum.valVtnDataflowPathInfoStatusType.UPLL_DATAFLOW_PATH_STATUS_DROP
						.ordinal()) {
					status = VtnServiceJsonConsts.DROP;
				}
				setValueToJsonObject(validBit, pathInfoBoundry,
						VtnServiceJsonConsts.STATUS, status);

			}
			pathInfoArray.add(pathInfoBoundry);

		}
		LOG.debug("pathInfoArray Json:" + pathInfoArray);
		LOG.trace("getDataFlowPathInfo completed");
		return pathInfoArray;
	}

	/**
	 * This method is Used to generate Match Flow info Json
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return JsonObject
	 */
	private JsonObject getDataFlowMatchInfo(final IpcDataUnit[] responsePacket,
			final AtomicInteger index, final byte validBit,
			final JsonObject controlerFlow,
			final IpcStruct valDfDataFlowCmnStruct) {
		LOG.trace("getDataFlowMatchInfo started");
		final int matchCount = Integer.parseInt(IpcDataUnitWrapper
				.getIpcStructUint32Value(valDfDataFlowCmnStruct,
						VtnServiceIpcConsts.MATCH_COUNT));
		LOG.debug("MATCH_COUNT:" + matchCount);
		// match JsonObject will hold all below jsonObject as per requiremnts
		final JsonObject match = new JsonObject();

		JsonArray inportJsonArray = null;
		JsonArray srcMacJsonArray = null;
		JsonArray dstMacJsonArray = null;
		JsonArray srcMaskJsonArray = null;
		JsonArray dstMaskJsonArray = null;
		JsonArray macEtherTypeJsonArray = null;
		JsonArray vlanIdJsonArray = null;
		JsonArray vlanPriorityJsonArray = null;
		JsonArray ipTosJsonArray = null;
		JsonArray ipProtoJsonArray = null;
		JsonArray ipDstAddrJsonArray = null;
		JsonArray ipDstAddrMaskJsonArray = null;
		JsonArray ipSrcAddrJsonArray = null;
		JsonArray ipSrcAddrMaskJsonArray = null;
		JsonArray l4DstPortIcmpTypeJsonArray = null;
		JsonArray l4DstPortIcmpTypeMaskJsonArray = null;
		JsonArray l4SrcPortIcmpTypeJsonArray = null;
		JsonArray l4SrcPortIcmpTypeMaskJsonArray = null;
		JsonArray ipV6DstAddJsonArray = null;
		JsonArray ipV6DstAddrMaskJsonArray = null;
		JsonArray ipV6SrcAddrJsonArray = null;
		JsonArray ipV6SrcAddrMaskJsonArray = null;
		JsonPrimitive element = null;
		for (int i = 0; i < matchCount; i++) {
			final IpcStruct valDfFlowMatchStruct = (IpcStruct) responsePacket[index
					.getAndIncrement()];

			final int matchtype = Integer.parseInt(IpcDataUnitWrapper
					.getIpcStructUint32Value(valDfFlowMatchStruct,
							VtnServiceIpcConsts.MATCH_TYPE));
			LOG.debug("MATCH TYPE:" + matchtype);
			// match type will help in resolving response in match info
			if (matchtype == UncStructIndexEnum.UncDataflowFlowMatchType.UNC_MATCH_IN_PORT
					.ordinal()) {
				final IpcStruct valDfFlowMatchInPort = (IpcStruct) responsePacket[index
						.getAndIncrement()];
				// set inport
				element = new JsonPrimitive(IpcDataUnitWrapper
						.getIpcStructUint32Value(valDfFlowMatchInPort,
								VtnServiceIpcConsts.IN_PORT).toString());

				if (null == inportJsonArray) {
					inportJsonArray = new JsonArray();
				}
				inportJsonArray.add(element);
				LOG.debug("set validBit for in_port :" + validBit);

			} else if (matchtype == UncStructIndexEnum.UncDataflowFlowMatchType.UNC_MATCH_DL_DST
					.ordinal()) {
				final IpcStruct valDfFlowMatchDlAddr = (IpcStruct) responsePacket[index
						.getAndIncrement()];

				element = new JsonPrimitive(IpcDataUnitWrapper.getMacAddress(
						valDfFlowMatchDlAddr, VtnServiceIpcConsts.DL_ADDR));
				if (null == dstMacJsonArray) {
					dstMacJsonArray = new JsonArray();
				}
				dstMacJsonArray.add(element);
				LOG.debug("set validbit for macdst :" + validBit);

				final String s = IpcDataUnitWrapper.getIpcStructUint8Value(
						valDfFlowMatchDlAddr, VtnServiceIpcConsts.V_MASK);

				if (Integer.parseInt(s) == UncStructIndexEnum.Valid.UNC_VF_VALID
						.ordinal()) {
					element = new JsonPrimitive(IpcDataUnitWrapper
							.getMacAddress(valDfFlowMatchDlAddr,
									VtnServiceIpcConsts.DL_ADDR_MASK)
							.toString());
					if (null == dstMaskJsonArray) {
						dstMaskJsonArray = new JsonArray();
					}
					dstMaskJsonArray.add(element);
					LOG.debug("set validbit for macdst :" + validBit);
				}
			} else if (matchtype == UncStructIndexEnum.UncDataflowFlowMatchType.UNC_MATCH_DL_SRC
					.ordinal()) {
				final IpcStruct valDfFlowMatchDlAddr = (IpcStruct) responsePacket[index
						.getAndIncrement()];

				element = new JsonPrimitive(IpcDataUnitWrapper.getMacAddress(
						valDfFlowMatchDlAddr, VtnServiceIpcConsts.DL_ADDR)
						.toString());
				if (null == srcMacJsonArray) {
					srcMacJsonArray = new JsonArray();
				}
				srcMacJsonArray.add(element);
				LOG.debug("set validbit for macsrc  :" + validBit);

				final String s = IpcDataUnitWrapper.getIpcStructUint8Value(
						valDfFlowMatchDlAddr, VtnServiceIpcConsts.V_MASK);

				if (Integer.parseInt(s) == UncStructIndexEnum.Valid.UNC_VF_VALID
						.ordinal()) {
					element = new JsonPrimitive(IpcDataUnitWrapper
							.getMacAddress(valDfFlowMatchDlAddr,
									VtnServiceIpcConsts.DL_ADDR_MASK)
							.toString());
					if (null == srcMaskJsonArray) {
						srcMaskJsonArray = new JsonArray();
					}
					srcMaskJsonArray.add(element);
					LOG.debug("set validbit for macdst  :" + validBit);
				}
			} else if (matchtype == UncStructIndexEnum.UncDataflowFlowMatchType.UNC_MATCH_DL_TYPE
					.ordinal()) {
				final IpcStruct valDfFlowMatchDlType = (IpcStruct) responsePacket[index
						.getAndIncrement()];
				element = new JsonPrimitive(IpcDataUnitWrapper
						.getIpcStructUint16HexaValue(valDfFlowMatchDlType,
								VtnServiceIpcConsts.DL_TYPE).toString());
				if (null == macEtherTypeJsonArray) {
					macEtherTypeJsonArray = new JsonArray();
				}
				macEtherTypeJsonArray.add(element);
				LOG.debug("set validbit for etherntype :" + validBit);

			} else if (matchtype == UncStructIndexEnum.UncDataflowFlowMatchType.UNC_MATCH_VLAN_ID
					.ordinal()) {
				final IpcStruct valDfFlowMatchVlanVid = (IpcStruct) responsePacket[index
						.getAndIncrement()];

				element = new JsonPrimitive(IpcDataUnitWrapper
						.getIpcStructUint16Value(valDfFlowMatchVlanVid,
								VtnServiceIpcConsts.VLAN_ID).toString());
				if (null == vlanIdJsonArray) {
					vlanIdJsonArray = new JsonArray();
				}
				if (element.getAsString().equals(
						VtnServiceJsonConsts.VLAN_ID_65535)) {
					element = new JsonPrimitive(VtnServiceJsonConsts.EMPTY);
					vlanIdJsonArray.add(element);
				} else {
					vlanIdJsonArray.add(element);
				}

				LOG.debug("set validbit for vlan_id  :" + validBit);

			} else if (matchtype == UncStructIndexEnum.UncDataflowFlowMatchType.UNC_MATCH_VLAN_PCP
					.ordinal()) {
				final IpcStruct valDfFlowMatchVlanpcp = (IpcStruct) responsePacket[index
						.getAndIncrement()];

				element = new JsonPrimitive(IpcDataUnitWrapper
						.getIpcStructUint8Value(valDfFlowMatchVlanpcp,
								VtnServiceIpcConsts.VLAN_PCP).toString());
				if (null == vlanPriorityJsonArray) {
					vlanPriorityJsonArray = new JsonArray();
				}
				vlanPriorityJsonArray.add(element);
				LOG.debug("set validbit for vlanpriority  :" + validBit);

			} else if (matchtype == UncStructIndexEnum.UncDataflowFlowMatchType.UNC_MATCH_IP_TOS
					.ordinal()) {

				final IpcStruct valDfFlowMatchIpTos = (IpcStruct) responsePacket[index
						.getAndIncrement()];

				final String hexString = UnsignedInteger.toHexString(Long
						.valueOf(IpcDataUnitWrapper
								.getIpcStructUint8Value(valDfFlowMatchIpTos,
										VtnServiceIpcConsts.IP_TOS)));
				element = new JsonPrimitive("0x" + hexString);
				if (null == ipTosJsonArray) {
					ipTosJsonArray = new JsonArray();
				}
				ipTosJsonArray.add(element);
				LOG.debug("set validbit for iptos :" + validBit);

			} else if (matchtype == UncStructIndexEnum.UncDataflowFlowMatchType.UNC_MATCH_IP_PROTO
					.ordinal()) {

				final IpcStruct valDfFlowMatchIpProto = (IpcStruct) responsePacket[index
						.getAndIncrement()];

				element = new JsonPrimitive(IpcDataUnitWrapper
						.getIpcStructUint8Value(valDfFlowMatchIpProto,
								VtnServiceIpcConsts.IP_PROTO).toString());
				if (null == ipProtoJsonArray) {
					ipProtoJsonArray = new JsonArray();
				}
				ipProtoJsonArray.add(element);
				LOG.debug("set validbit for  ipproto :" + validBit);

			} else if (matchtype == UncStructIndexEnum.UncDataflowFlowMatchType.UNC_MATCH_IPV4_SRC
					.ordinal()) {
				final IpcStruct valDfFlowMatchIpv4Addr = (IpcStruct) responsePacket[index
						.getAndIncrement()];

				element = new JsonPrimitive(IpcDataUnitWrapper
						.getIpcStructIpv4Value(valDfFlowMatchIpv4Addr,
								VtnServiceIpcConsts.IPV4_ADDR).toString());
				if (null == ipSrcAddrJsonArray) {
					ipSrcAddrJsonArray = new JsonArray();
				}
				ipSrcAddrJsonArray.add(element);
				LOG.debug("set validbit for ipsrc :" + validBit);

				final String s = IpcDataUnitWrapper.getIpcStructUint8Value(
						valDfFlowMatchIpv4Addr, VtnServiceIpcConsts.V_MASK);
				if (Integer.parseInt(s) == UncStructIndexEnum.Valid.UNC_VF_VALID
						.ordinal()) {
					element = new JsonPrimitive(IpcDataUnitWrapper
							.getIpcStructIpv4Value(valDfFlowMatchIpv4Addr,
									VtnServiceIpcConsts.IPV4_ADDR_MASK)
							.toString());
					if (null == ipSrcAddrMaskJsonArray) {
						ipSrcAddrMaskJsonArray = new JsonArray();

					}
					ipSrcAddrMaskJsonArray.add(element);
					LOG.debug("set validBit for ipv4_mask:" + validBit);
				}
			} else if (matchtype == UncStructIndexEnum.UncDataflowFlowMatchType.UNC_MATCH_IPV4_DST
					.ordinal()) {
				final IpcStruct valDfFlowMatchIpv4Addr = (IpcStruct) responsePacket[index
						.getAndIncrement()];

				element = new JsonPrimitive(IpcDataUnitWrapper
						.getIpcStructIpv4Value(valDfFlowMatchIpv4Addr,
								VtnServiceIpcConsts.IPV4_ADDR).toString());
				if (null == ipDstAddrJsonArray) {
					ipDstAddrJsonArray = new JsonArray();
				}
				ipDstAddrJsonArray.add(element);
				LOG.debug("set validbit for ipdst  :" + validBit);

				final String s = IpcDataUnitWrapper.getIpcStructUint8Value(
						valDfFlowMatchIpv4Addr, VtnServiceIpcConsts.V_MASK);
				if (Integer.parseInt(s) == UncStructIndexEnum.Valid.UNC_VF_VALID
						.ordinal()) {
					element = new JsonPrimitive(IpcDataUnitWrapper
							.getIpcStructIpv4Value(valDfFlowMatchIpv4Addr,
									VtnServiceIpcConsts.IPV4_ADDR_MASK)
							.toString());
					if (null == ipDstAddrMaskJsonArray) {
						ipDstAddrMaskJsonArray = new JsonArray();
					}
					ipDstAddrMaskJsonArray.add(element);
					LOG.debug("set validbit for ipv4_mask:" + validBit);
				}
			} else if (matchtype == UncStructIndexEnum.UncDataflowFlowMatchType.UNC_MATCH_IPV6_SRC
					.ordinal()) {

				final IpcStruct valdfflowmatchIpv6Addr = (IpcStruct) responsePacket[index
						.getAndIncrement()];

				element = new JsonPrimitive(IpcDataUnitWrapper
						.getIpcStructIpv6Value(valdfflowmatchIpv6Addr,
								VtnServiceIpcConsts.IPV6_ADDR).toString());
				if (null == ipV6SrcAddrJsonArray) {
					ipV6SrcAddrJsonArray = new JsonArray();
				}
				ipV6SrcAddrJsonArray.add(element);
				LOG.debug("set validbit for ipv6src  :" + validBit);

				final String s = IpcDataUnitWrapper.getIpcStructUint8Value(
						valdfflowmatchIpv6Addr, VtnServiceIpcConsts.V_MASK);

				if (Integer.parseInt(s) == UncStructIndexEnum.Valid.UNC_VF_VALID
						.ordinal()) {
					element = new JsonPrimitive(IpcDataUnitWrapper
							.getIpcStructIpv6Value(valdfflowmatchIpv6Addr,
									VtnServiceIpcConsts.IPV6_ADDR_MASK)
							.toString());
					if (null == ipV6SrcAddrMaskJsonArray) {
						ipV6SrcAddrMaskJsonArray = new JsonArray();
					}
					ipV6SrcAddrMaskJsonArray.add(element);
					LOG.debug("set validbit for ipv6_mask:" + validBit);
				}
			} else if (matchtype == UncStructIndexEnum.UncDataflowFlowMatchType.UNC_MATCH_IPV6_DST
					.ordinal()) {

				final IpcStruct valdfflowmatchIpv6Addr = (IpcStruct) responsePacket[index
						.getAndIncrement()];

				element = new JsonPrimitive(IpcDataUnitWrapper
						.getIpcStructIpv6Value(valdfflowmatchIpv6Addr,
								VtnServiceIpcConsts.IPV6_ADDR).toString());
				if (null == ipV6DstAddJsonArray) {
					ipV6DstAddJsonArray = new JsonArray();
				}
				ipV6DstAddJsonArray.add(element);
				LOG.debug("set validbit for ipv6dst  :" + validBit);

				final String s = IpcDataUnitWrapper.getIpcStructUint8Value(
						valdfflowmatchIpv6Addr, VtnServiceIpcConsts.V_MASK);

				if (Integer.parseInt(s) == UncStructIndexEnum.Valid.UNC_VF_VALID
						.ordinal()) {
					element = new JsonPrimitive(IpcDataUnitWrapper
							.getIpcStructIpv6Value(valdfflowmatchIpv6Addr,
									VtnServiceIpcConsts.IPV6_ADDR_MASK)
							.toString());
					if (null == ipV6DstAddrMaskJsonArray) {
						ipV6DstAddrMaskJsonArray = new JsonArray();
					}
					ipV6DstAddrMaskJsonArray.add(element);
					LOG.debug("set validbit for ipv6_mask:" + validBit);
				}
			} else if (matchtype == UncStructIndexEnum.UncDataflowFlowMatchType.UNC_MATCH_TP_SRC
					.ordinal()) {

				final IpcStruct valDfFlowMatchTpPort = (IpcStruct) responsePacket[index
						.getAndIncrement()];

				element = new JsonPrimitive(IpcDataUnitWrapper
						.getIpcStructUint16Value(valDfFlowMatchTpPort,
								VtnServiceIpcConsts.TP_PORT).toString());
				if (null == l4SrcPortIcmpTypeJsonArray) {
					l4SrcPortIcmpTypeJsonArray = new JsonArray();
				}
				l4SrcPortIcmpTypeJsonArray.add(element);

				LOG.debug("set validbit for tpsrc :" + validBit);

				final String s = IpcDataUnitWrapper.getIpcStructUint8Value(
						valDfFlowMatchTpPort, VtnServiceIpcConsts.V_MASK);

				if (Integer.parseInt(s) == UncStructIndexEnum.Valid.UNC_VF_VALID
						.ordinal()) {

					element = new JsonPrimitive(IpcDataUnitWrapper
							.getIpcStructUint16Value(valDfFlowMatchTpPort,
									VtnServiceIpcConsts.TP_PORT_MASK)
							.toString());
					if (null == l4SrcPortIcmpTypeMaskJsonArray) {
						l4SrcPortIcmpTypeMaskJsonArray = new JsonArray();
					}
					l4SrcPortIcmpTypeMaskJsonArray.add(element);
					LOG.debug("set validbit for tpsrcmask :" + validBit);
				}
			} else if (matchtype == UncStructIndexEnum.UncDataflowFlowMatchType.UNC_MATCH_TP_DST
					.ordinal()) {

				final IpcStruct valDfFlowMatchTpPort = (IpcStruct) responsePacket[index
						.getAndIncrement()];

				element = new JsonPrimitive(IpcDataUnitWrapper
						.getIpcStructUint16Value(valDfFlowMatchTpPort,
								VtnServiceIpcConsts.TP_PORT).toString());
				if (null == l4DstPortIcmpTypeJsonArray) {
					l4DstPortIcmpTypeJsonArray = new JsonArray();
				}
				l4DstPortIcmpTypeJsonArray.add(element);
				LOG.debug("set validbit for tpdst  :" + validBit);

				final String s = IpcDataUnitWrapper.getIpcStructUint8Value(
						valDfFlowMatchTpPort, VtnServiceIpcConsts.V_MASK);

				if (Integer.parseInt(s) == UncStructIndexEnum.Valid.UNC_VF_VALID
						.ordinal()) {
					element = new JsonPrimitive(IpcDataUnitWrapper
							.getIpcStructUint16Value(valDfFlowMatchTpPort,
									VtnServiceIpcConsts.TP_PORT_MASK)
							.toString());

					if (null == l4DstPortIcmpTypeMaskJsonArray) {
						l4DstPortIcmpTypeMaskJsonArray = new JsonArray();
					}
					l4DstPortIcmpTypeMaskJsonArray.add(element);
				}
			} else {
				LOG.debug("Type : invalid");
			}

		}
		if (null != inportJsonArray) {
			match.add(VtnServiceJsonConsts.INPORT, inportJsonArray);
		}
		if (null != dstMacJsonArray) {
			match.add(VtnServiceJsonConsts.MACDSTADDR, dstMacJsonArray);
		}
		if (dstMaskJsonArray != null) {
			match.add(VtnServiceJsonConsts.MACDSTADDR_MASK, dstMaskJsonArray);
		}
		if (srcMacJsonArray != null) {
			match.add(VtnServiceJsonConsts.MACSRCADDR, srcMacJsonArray);
		}
		if (srcMaskJsonArray != null) {
			match.add(VtnServiceJsonConsts.MACSRCADDR_MASK, srcMaskJsonArray);
		}
		if (macEtherTypeJsonArray != null) {
			match.add(VtnServiceJsonConsts.MACETHERTYPE, macEtherTypeJsonArray);
		}
		if (vlanIdJsonArray != null) {
			match.add(VtnServiceJsonConsts.VLAN_ID, vlanIdJsonArray);
		}
		if (vlanPriorityJsonArray != null) {
			match.add(VtnServiceJsonConsts.VLAN_PRIORITY, vlanPriorityJsonArray);
		}
		if (ipTosJsonArray != null) {
			match.add(VtnServiceJsonConsts.IPTOS, ipTosJsonArray);
		}
		if (ipProtoJsonArray != null) {
			match.add(VtnServiceJsonConsts.IPPROTO, ipProtoJsonArray);
		}
		if (ipSrcAddrJsonArray != null) {
			match.add(VtnServiceJsonConsts.IPSRCADDR, ipSrcAddrJsonArray);
		}
		if (ipSrcAddrMaskJsonArray != null) {
			match.add(VtnServiceJsonConsts.IPSRCADDR_MASK,
					ipSrcAddrMaskJsonArray);
		}
		if (ipDstAddrJsonArray != null) {
			match.add(VtnServiceJsonConsts.IPDSTADDR, ipDstAddrJsonArray);
		}
		if (ipDstAddrMaskJsonArray != null) {
			match.add(VtnServiceJsonConsts.IPDSTADDR_MASK,
					ipDstAddrMaskJsonArray);
		}
		if (ipV6SrcAddrJsonArray != null) {
			match.add(VtnServiceJsonConsts.IPV6SRCADDR, ipV6SrcAddrJsonArray);
		}
		if (ipV6SrcAddrMaskJsonArray != null) {
			match.add(VtnServiceJsonConsts.IPV6SRCADDR_MASK,
					ipV6SrcAddrMaskJsonArray);
		}
		if (ipV6DstAddJsonArray != null) {
			match.add(VtnServiceJsonConsts.IPV6DSTADDR, ipV6DstAddJsonArray);
		}
		if (ipV6DstAddrMaskJsonArray != null) {
			match.add(VtnServiceJsonConsts.IPV6DSTADDR_MASK,
					ipV6DstAddrMaskJsonArray);
		}
		if (l4SrcPortIcmpTypeJsonArray != null) {
			match.add(VtnServiceJsonConsts.L4SRCPORT_ICMPTYPE,
					l4SrcPortIcmpTypeJsonArray);
		}
		if (l4SrcPortIcmpTypeMaskJsonArray != null) {
			match.add(VtnServiceJsonConsts.L4SRCPORT_ICMPTYPE_MASK,
					l4SrcPortIcmpTypeMaskJsonArray);
		}
		if (l4DstPortIcmpTypeJsonArray != null) {
			match.add(VtnServiceJsonConsts.L4DSTPORT_ICMPTYPE,
					l4DstPortIcmpTypeJsonArray);
		}
		if (l4DstPortIcmpTypeMaskJsonArray != null) {
			match.add(VtnServiceJsonConsts.L4DSTPORT_ICMPTYPE_MASK,
					l4DstPortIcmpTypeMaskJsonArray);
		}
		LOG.debug("match Json :" + match);
		LOG.trace("getDataFlowMatchInfo completed");
		return match;
	}

	/**
	 * This method is Used to generate Action Flow info Json
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return JsonObject
	 */
	private JsonObject getDataFlowActionInfo(
			final IpcDataUnit[] responsePacket, final AtomicInteger index,
			final JsonObject controlerFlow,
			final IpcStruct valVtnDataFlowCmnStruct) {
		LOG.trace("getDataFlowActionInfo started");
		final int actionCount = Integer.parseInt(IpcDataUnitWrapper
				.getIpcStructUint32Value(valVtnDataFlowCmnStruct,
						VtnServiceIpcConsts.ACTION_COUNT));
		LOG.debug("acount_count:" + actionCount);
		final JsonObject action = new JsonObject();
		JsonArray outputPortJsonArray = null;
		JsonArray enqueuePortJsonArray = null;
		JsonArray queueIdJsonArray = null;
		JsonArray setDstMacAddrJsonArray = null;
		JsonArray setSrcMAcAddrJsonArray = null;
		JsonArray setVlanIdJsonArray = null;
		JsonArray setVlanPriorityJsonArray = null;
		JsonArray setDstAddrJsonArray = null;
		JsonArray setSrcIpAddrJsonArray = null;
		JsonArray setIpTosJsonArray = null;
		JsonArray setDstL4PortIcmpTypeJsonArray = null;
		JsonArray setSrcL4PortIcmpTypeJsonArray = null;
		JsonArray setIpV6DstAddrJsonArray = null;
		JsonArray setIpv6SrcAddrJsonArray = null;
		JsonArray setStripVlanJsonArray = null;
		JsonPrimitive element = null;
		int actionType;
		for (int i = 0; i < actionCount; i++) {

			final IpcStruct valDataFlowAction = (IpcStruct) responsePacket[index
					.getAndIncrement()];

			actionType = Integer.parseInt(IpcDataUnitWrapper
					.getIpcStructUint32Value(valDataFlowAction,
							VtnServiceIpcConsts.ACTION_TYPE));
			LOG.debug("actiontype :" + actionType);
			if (actionType == UncStructIndexEnum.UncDataflowFlowActionType.UNC_ACTION_OUTPUT
					.ordinal()) {

				final IpcStruct valDfFlowActionOutputPort = (IpcStruct) responsePacket[index
						.getAndIncrement()];
				element = new JsonPrimitive(IpcDataUnitWrapper
						.getIpcStructUint32Value(valDfFlowActionOutputPort,
								VtnServiceIpcConsts.OUTPUT_PORT).toString());
				if (null == outputPortJsonArray) {
					outputPortJsonArray = new JsonArray();
				}
				outputPortJsonArray.add(element);

			} else if (actionType == UncStructIndexEnum.UncDataflowFlowActionType.UNC_ACTION_SET_ENQUEUE
					.ordinal()) {

				final IpcStruct valDfFlowActionEnqueuePort = (IpcStruct) responsePacket[index
						.getAndIncrement()];
				element = new JsonPrimitive(IpcDataUnitWrapper
						.getIpcStructUint32Value(valDfFlowActionEnqueuePort,
								VtnServiceIpcConsts.OUTPUT_PORT).toString());
				if (null == enqueuePortJsonArray) {
					enqueuePortJsonArray = new JsonArray();
				}
				enqueuePortJsonArray.add(element);

				element = new JsonPrimitive(IpcDataUnitWrapper
						.getIpcStructUint16Value(valDfFlowActionEnqueuePort,
								VtnServiceIpcConsts.ENQUEUE_ID).toString());
				if (null == queueIdJsonArray) {
					queueIdJsonArray = new JsonArray();

				}
				queueIdJsonArray.add(element);

			} else if (actionType == UncStructIndexEnum.UncDataflowFlowActionType.UNC_ACTION_SET_DL_DST
					.ordinal()) {
				final IpcStruct valDfFlowActionSetDlAddr = (IpcStruct) responsePacket[index
						.getAndIncrement()];
				element = new JsonPrimitive(IpcDataUnitWrapper.getMacAddress(
						valDfFlowActionSetDlAddr, VtnServiceIpcConsts.DL_ADDR)
						.toString());
				if (null == setDstMacAddrJsonArray) {
					setDstMacAddrJsonArray = new JsonArray();
				}
				setDstMacAddrJsonArray.add(element);

			} else if (actionType == UncStructIndexEnum.UncDataflowFlowActionType.UNC_ACTION_SET_DL_SRC
					.ordinal()) {
				final IpcStruct valDfFlowActionSetDlAddr = (IpcStruct) responsePacket[index
						.getAndIncrement()];

				element = new JsonPrimitive(IpcDataUnitWrapper.getMacAddress(
						valDfFlowActionSetDlAddr, VtnServiceIpcConsts.DL_ADDR)
						.toString());
				if (null == setSrcMAcAddrJsonArray) {
					setSrcMAcAddrJsonArray = new JsonArray();
				}
				setSrcMAcAddrJsonArray.add(element);

			} else if (actionType == UncStructIndexEnum.UncDataflowFlowActionType.UNC_ACTION_SET_VLAN_ID
					.ordinal()) {
				final IpcStruct valDfFlowActionSetVlanId = (IpcStruct) responsePacket[index
						.getAndIncrement()];

				element = new JsonPrimitive(IpcDataUnitWrapper
						.getIpcStructUint16Value(valDfFlowActionSetVlanId,
								VtnServiceIpcConsts.VLAN_ID).toString());

				if (null == setVlanIdJsonArray) {
					setVlanIdJsonArray = new JsonArray();

				}
				if (element.getAsString().equals(
						VtnServiceJsonConsts.VLAN_ID_65535)) {
					element = new JsonPrimitive(VtnServiceJsonConsts.EMPTY);
					setVlanIdJsonArray.add(element);
				} else {
					setVlanIdJsonArray.add(element);
				}
			} else if (actionType == UncStructIndexEnum.UncDataflowFlowActionType.UNC_ACTION_SET_VLAN_PCP
					.ordinal()) {
				final IpcStruct valDfFlowActionSetVlanPcp = (IpcStruct) responsePacket[index
						.getAndIncrement()];

				element = new JsonPrimitive(IpcDataUnitWrapper
						.getIpcStructUint8Value(valDfFlowActionSetVlanPcp,
								VtnServiceIpcConsts.VLAN_PCP).toString());
				if (null == setVlanPriorityJsonArray) {
					setVlanPriorityJsonArray = new JsonArray();

				}
				setVlanPriorityJsonArray.add(element);

			} else if (actionType == UncStructIndexEnum.UncDataflowFlowActionType.UNC_ACTION_STRIP_VLAN
					.ordinal()) {
				element = new JsonPrimitive(VtnServiceJsonConsts.TRUE);
				if (null == setStripVlanJsonArray) {
					setStripVlanJsonArray = new JsonArray();
				}
				setStripVlanJsonArray.add(element);
				index.getAndIncrement();
			} else if (actionType == UncStructIndexEnum.UncDataflowFlowActionType.UNC_ACTION_SET_IPV4_DST
					.ordinal()) {
				final IpcStruct valDfFlowActionSetIpv4 = (IpcStruct) responsePacket[index
						.getAndIncrement()];
				element = new JsonPrimitive(IpcDataUnitWrapper
						.getIpcStructIpv4Value(valDfFlowActionSetIpv4,
								VtnServiceIpcConsts.IPV4_ADDR).toString());

				if (null == setDstAddrJsonArray) {
					setDstAddrJsonArray = new JsonArray();

				}
				setDstAddrJsonArray.add(element);

			} else if (actionType == UncStructIndexEnum.UncDataflowFlowActionType.UNC_ACTION_SET_IPV4_SRC
					.ordinal()) {
				final IpcStruct valDfFlowActionSetIpv4 = (IpcStruct) responsePacket[index
						.getAndIncrement()];

				element = new JsonPrimitive(IpcDataUnitWrapper
						.getIpcStructIpv4Value(valDfFlowActionSetIpv4,
								VtnServiceIpcConsts.IPV4_ADDR).toString());
				if (null == setSrcIpAddrJsonArray) {
					setSrcIpAddrJsonArray = new JsonArray();
				}
				setSrcIpAddrJsonArray.add(element);

			} else if (actionType == UncStructIndexEnum.UncDataflowFlowActionType.UNC_ACTION_SET_IP_TOS
					.ordinal()) {
				final IpcStruct valDfFlowActionSetIpTos = (IpcStruct) responsePacket[index
						.getAndIncrement()];

				final String hexString = UnsignedInteger.toHexString(Long
						.valueOf(IpcDataUnitWrapper.getIpcStructUint8Value(
								valDfFlowActionSetIpTos,
								VtnServiceIpcConsts.IP_TOS)));
				element = new JsonPrimitive("0x" + hexString);

				if (null == setIpTosJsonArray) {
					setIpTosJsonArray = new JsonArray();
				}
				setIpTosJsonArray.add(element);

			} else if (actionType == UncStructIndexEnum.UncDataflowFlowActionType.UNC_ACTION_SET_TP_DST
					.ordinal()) {
				final IpcStruct valDfFlowActionSetTpPort = (IpcStruct) responsePacket[index
						.getAndIncrement()];

				element = new JsonPrimitive(IpcDataUnitWrapper
						.getIpcStructUint16Value(valDfFlowActionSetTpPort,
								VtnServiceIpcConsts.TP_PORT).toString());
				if (null == setDstL4PortIcmpTypeJsonArray) {
					setDstL4PortIcmpTypeJsonArray = new JsonArray();
				}
				setDstL4PortIcmpTypeJsonArray.add(element);

			} else if (actionType == UncStructIndexEnum.UncDataflowFlowActionType.UNC_ACTION_SET_TP_SRC
					.ordinal()) {
				final IpcStruct valDfFlowActionSetTpPort = (IpcStruct) responsePacket[index
						.getAndIncrement()];

				element = new JsonPrimitive(IpcDataUnitWrapper
						.getIpcStructUint16Value(valDfFlowActionSetTpPort,
								VtnServiceIpcConsts.TP_PORT).toString());
				if (null == setSrcL4PortIcmpTypeJsonArray) {
					setSrcL4PortIcmpTypeJsonArray = new JsonArray();
				}
				setSrcL4PortIcmpTypeJsonArray.add(element);

			} else if (actionType == UncStructIndexEnum.UncDataflowFlowActionType.UNC_ACTION_SET_IPV6_DST
					.ordinal()) {

				final IpcStruct valDfFlowActionSetIpv6 = (IpcStruct) responsePacket[index
						.getAndIncrement()];

				element = new JsonPrimitive(
						IpcDataUnitWrapper.getIpcStructIpv6Value(
								valDfFlowActionSetIpv6,
								VtnServiceIpcConsts.IPV6_ADDR));
				if (null == setIpV6DstAddrJsonArray) {
					setIpV6DstAddrJsonArray = new JsonArray();
				}
				setIpV6DstAddrJsonArray.add(element);

			} else if (actionType == UncStructIndexEnum.UncDataflowFlowActionType.UNC_ACTION_SET_IPV6_SRC
					.ordinal()) {

				final IpcStruct valDfFlowActionSetIpv6 = (IpcStruct) responsePacket[index
						.getAndIncrement()];

				element = new JsonPrimitive(
						IpcDataUnitWrapper.getIpcStructIpv6Value(
								valDfFlowActionSetIpv6,
								VtnServiceIpcConsts.IPV6_ADDR));
				if (null == setIpv6SrcAddrJsonArray) {
					setIpv6SrcAddrJsonArray = new JsonArray();
				}
				setIpv6SrcAddrJsonArray.add(element);

			} else {
				LOG.debug("Type : invalid");
			}
		}
		if (outputPortJsonArray != null) {
			action.add(VtnServiceJsonConsts.OUTPUTPORT, outputPortJsonArray);
		}
		if (enqueuePortJsonArray != null) {
			action.add(VtnServiceJsonConsts.ENQUEUEPORT, enqueuePortJsonArray);
		}
		if (queueIdJsonArray != null) {
			action.add(VtnServiceJsonConsts.QUEUE_ID, queueIdJsonArray);
		}
		if (setDstMacAddrJsonArray != null) {
			action.add(VtnServiceJsonConsts.SETMACDSTADDR,
					setDstMacAddrJsonArray);
		}
		if (setSrcMAcAddrJsonArray != null) {
			action.add(VtnServiceJsonConsts.SETMACSRCADDR,
					setSrcMAcAddrJsonArray);
		}
		if (setVlanIdJsonArray != null) {
			action.add(VtnServiceJsonConsts.SETVLAN_ID, setVlanIdJsonArray);
		}
		if (setVlanPriorityJsonArray != null) {
			action.add(VtnServiceJsonConsts.SETVLAN_PRIORITY,
					setVlanPriorityJsonArray);
		}
		if (setStripVlanJsonArray != null) {
			action.add(VtnServiceJsonConsts.STRIPVLAN, setStripVlanJsonArray);
		}
		if (setDstAddrJsonArray != null) {
			action.add(VtnServiceJsonConsts.SETIPDSTADDR, setDstAddrJsonArray);
		}
		if (setSrcIpAddrJsonArray != null) {
			action.add(VtnServiceJsonConsts.SETIPSRCADDR, setSrcIpAddrJsonArray);
		}
		if (setIpTosJsonArray != null) {
			action.add(VtnServiceJsonConsts.SETIPTOS, setIpTosJsonArray);
		}
		if (setDstL4PortIcmpTypeJsonArray != null) {
			action.add(VtnServiceJsonConsts.SETL4DSTPORT_ICMPTYPE,
					setDstL4PortIcmpTypeJsonArray);
		}
		if (setSrcL4PortIcmpTypeJsonArray != null) {
			action.add(VtnServiceJsonConsts.SETL4SRCPORT_ICMPTYPE,
					setSrcL4PortIcmpTypeJsonArray);
		}
		if (setIpV6DstAddrJsonArray != null) {
			action.add(VtnServiceJsonConsts.SETIPV6DSTADDR,
					setIpV6DstAddrJsonArray);
		}
		if (setIpv6SrcAddrJsonArray != null) {
			action.add(VtnServiceJsonConsts.SETIPV6SRCADDR,
					setIpv6SrcAddrJsonArray);
		}
		LOG.debug("action json:" + action);
		return action;
	}

	/**
	 * Used for VTerminalInterface FlowFilter Response
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return JsonObject
	 */
	public JsonObject getVTerminalInterfaceFlowFilterResponse(
			final IpcDataUnit[] responsePacket, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start getVTerminalInterfaceFlowFilterResponse");
		final JsonObject root = new JsonObject();

		final String rootJsonName = VtnServiceJsonConsts.FLOWFILTER;
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject vTerminalIFflowFilter = null;
		for (int index = 0; index < responsePacket.length; index++) {

			vTerminalIFflowFilter = new JsonObject();

			// There is no use of key type
			LOG.debug("Skip key type: no use");
			index++;
			/*
			 * add mandatory informations from key structure
			 */
			final IpcStruct keyVTerminalIfFlowFilter = (IpcStruct) responsePacket[index++];
			if (IpcDataUnitWrapper
					.getIpcStructUint8Value(keyVTerminalIfFlowFilter,
							VtnServiceIpcConsts.DIRECTION)
					.equals(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_IN
							.getValue())) {
				vTerminalIFflowFilter.addProperty(VtnServiceJsonConsts.FFTYPE,
						VtnServiceJsonConsts.IN);
				LOG.debug("FF Type :"
						+ IpcDataUnitWrapper.getIpcStructUint8Value(
								keyVTerminalIfFlowFilter,
								VtnServiceIpcConsts.DIRECTION));
			} else if (IpcDataUnitWrapper
					.getIpcStructUint8Value(keyVTerminalIfFlowFilter,
							VtnServiceIpcConsts.DIRECTION)
					.equals(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_OUT
							.getValue())) {
				vTerminalIFflowFilter.addProperty(VtnServiceJsonConsts.FFTYPE,
						VtnServiceJsonConsts.OUT);
				LOG.debug("FF Type :"
						+ IpcDataUnitWrapper.getIpcStructUint8Value(
								keyVTerminalIfFlowFilter,
								VtnServiceIpcConsts.DIRECTION));
			} else {
				LOG.debug("Invalid value for FFTYPE parameter");
			}
		}

		/*
		 * finally add single object to root json object and return the same.
		 */
		root.add(rootJsonName, vTerminalIFflowFilter);
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVTerminalInterfaceFlowFilterResponse");
		return root;
	}

	/**
	 * Used for VTerminalInterface FlowFilterEntry Response
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return JsonObject
	 */
	public JsonObject getVTerminalInterfaceFlowFilterEntryResponse(
			final IpcDataUnit[] responsePacket, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start getVTerminalInterfaceFlowFilterEntryResponse");
		final JsonObject root = new JsonObject();
		JsonArray vTermIfFlowFilterEntryArray = null;
		LOG.debug("getType: " + getType);
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}
		/*
		 * data type will be required to resolve the response structures
		 */
		String dataType = VtnServiceJsonConsts.STATE;
		if (requestBody.has(VtnServiceJsonConsts.TARGETDB)) {
			dataType = requestBody.get(VtnServiceJsonConsts.TARGETDB)
					.getAsString();
		}
		String rootJsonName;
		/*
		 * get type (show or list) will be required to resolve root json name
		 * here it will be vtn for show and vtns for list
		 */
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.FLOWFILTERENTRY;
		} else {
			rootJsonName = VtnServiceJsonConsts.FLOWFILTERENTRIES;
			// json array will be required for list type of cases
			vTermIfFlowFilterEntryArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject vTermIfFlowFilterEntry = null;

		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			vTermIfFlowFilterEntry = new JsonObject();
			vTermIfFlowFilterEntry
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, vTermIfFlowFilterEntry);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {

				vTermIfFlowFilterEntry = new JsonObject();
				byte validBit;

				// There is no use of key type
				index++;
				LOG.debug("Skip key type: no use");
				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyVTerminalIfFlowFilterEntryStruct = (IpcStruct) responsePacket[index++];
				vTermIfFlowFilterEntry.addProperty(VtnServiceJsonConsts.SEQNUM,
						IpcDataUnitWrapper.getIpcStructUint16Value(
								keyVTerminalIfFlowFilterEntryStruct,
								VtnServiceIpcConsts.SEQUENCENUM));

				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */

				if (getType.equals(VtnServiceJsonConsts.LIST)
						&& opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)
						|| getType.equals(VtnServiceJsonConsts.SHOW)) {
					LOG.debug("Case : Show or List with detail");
					/*
					 * add valid informations from value structure
					 */
					final IpcStruct valFlowFilterEntryStruct = (IpcStruct) responsePacket[index++];

					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_FLOWLIST_NAME_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vTermIfFlowFilterEntry,
								VtnServiceJsonConsts.FLNAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valFlowFilterEntryStruct,
										VtnServiceJsonConsts.FLOWLISTNAME));
					}
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_ACTION_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						if (IpcDataUnitWrapper
								.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.ACTION)
								.equalsIgnoreCase(
										UncStructIndexEnum.FlowfilterAction.UPLL_FLOWFILTER_ACT_PASS
												.getValue())) {
							setValueToJsonObject(validBit,
									vTermIfFlowFilterEntry,
									VtnServiceJsonConsts.ACTIONTYPE,
									VtnServiceJsonConsts.PASS);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.ACTION)
								.equalsIgnoreCase(
										UncStructIndexEnum.FlowfilterAction.UPLL_FLOWFILTER_ACT_DROP
												.getValue())) {
							setValueToJsonObject(validBit,
									vTermIfFlowFilterEntry,
									VtnServiceJsonConsts.ACTIONTYPE,
									VtnServiceJsonConsts.DROP);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.ACTION)
								.equalsIgnoreCase(
										UncStructIndexEnum.FlowfilterAction.UPLL_FLOWFILTER_ACT_REDIRECT
												.getValue())) {
							setValueToJsonObject(validBit,
									vTermIfFlowFilterEntry,
									VtnServiceJsonConsts.ACTIONTYPE,
									VtnServiceJsonConsts.REDIRECT);
						}
						LOG.debug("Action type :"
								+ IpcDataUnitWrapper.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.ACTION));
					}
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_NWM_NAME_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vTermIfFlowFilterEntry,
								VtnServiceJsonConsts.NMGNAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.NWMNAME));
					}
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_PRIORITY_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vTermIfFlowFilterEntry,
								VtnServiceJsonConsts.PRIORITY,
								IpcDataUnitWrapper.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceJsonConsts.PRIORITY));
					}
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_DSCP_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vTermIfFlowFilterEntry,
								VtnServiceJsonConsts.DSCP,
								IpcDataUnitWrapper.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceJsonConsts.DSCP));
					}
					final JsonObject redirectDst = new JsonObject();
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_REDIRECT_NODE_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, redirectDst,
								VtnServiceJsonConsts.VNODENAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.REDIRECTNODE));
					}
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_REDIRECT_PORT_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, redirectDst,
								VtnServiceJsonConsts.IFNAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.REDIRECTPORT));
					}
					// Direction
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_REDIRECT_DIRECTION_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						if (IpcDataUnitWrapper
								.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.REDIRECTDIRECTION)
								.equals(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_IN
										.getValue())) {
							setValueToJsonObject(validBit, redirectDst,
									VtnServiceJsonConsts.DIRECTION,
									VtnServiceJsonConsts.IN);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.REDIRECTDIRECTION)
								.equals(UncStructIndexEnum.FlowfilterDirection.UPLL_FLOWFILTER_DIR_OUT
										.getValue())) {
							setValueToJsonObject(validBit, redirectDst,
									VtnServiceJsonConsts.DIRECTION,
									VtnServiceJsonConsts.OUT);
						} else {
							LOG.debug("Direction : Invalid");
						}
						LOG.debug("Direction :"
								+ IpcDataUnitWrapper.getIpcStructUint8Value(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.REDIRECTDIRECTION));
					}
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_MODIFY_DST_MAC_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, redirectDst,
								VtnServiceJsonConsts.MACDSTADDR,
								IpcDataUnitWrapper.getMacAddress(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.MODIFYDSTMACADDR));
					}
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_MODIFY_SRC_MAC_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, redirectDst,
								VtnServiceJsonConsts.MACSRCADDR,
								IpcDataUnitWrapper.getMacAddress(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.MODIFYSRCMACADDR));
					}
					vTermIfFlowFilterEntry.add(
							VtnServiceJsonConsts.REDIRECTDST, redirectDst);
				}
				if (getType.equals(VtnServiceJsonConsts.SHOW)
						&& dataType
								.equalsIgnoreCase(VtnServiceJsonConsts.STATE)
						&& opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
					LOG.debug("Case : Show and targetdb :State ");
					final IpcStruct valFlowFilterEntryStStruct = (IpcStruct) responsePacket[index++];
					if (opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
						LOG.debug("op : detail");
						validBit = valFlowFilterEntryStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.ValFlowfilterEntryStIndex.UPLL_IDX_NWM_STATUS_FFES
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							setValueToJsonObject(validBit,
									vTermIfFlowFilterEntry,
									VtnServiceJsonConsts.NMG_STATUS,
									IpcDataUnitWrapper.getIpcStructUint8Value(
											valFlowFilterEntryStStruct,
											VtnServiceIpcConsts.NWM_STATUS));
						}
						final PomStatsIndex pomStatsIndexSet = new PomStatsIndex();
						pomStatsIndexSet
								.setSoftware(UncStructIndexEnum.ValFlowfilterEntryStIndex.UPLL_IDX_SOFTWARE_FFES
										.ordinal());
						pomStatsIndexSet
								.setExistingFlow(UncStructIndexEnum.ValFlowfilterEntryStIndex.UPLL_IDX_EXIST_FFES
										.ordinal());
						pomStatsIndexSet
								.setExpiredFlow(UncStructIndexEnum.ValFlowfilterEntryStIndex.UPLL_IDX_EXPIRE_FFES
										.ordinal());
						pomStatsIndexSet
								.setTotal(UncStructIndexEnum.ValFlowfilterEntryStIndex.UPLL_IDX_TOTAL_FFES
										.ordinal());
						LOG.debug("call getPomStats : for statics information");
						getPomStats(vTermIfFlowFilterEntry,
								valFlowFilterEntryStStruct, pomStatsIndexSet);
						final String flowListName = VtnServiceJsonConsts.FLOWLIST;
						final JsonObject flowListJson = new JsonObject();
						final String flowListEntriesName = VtnServiceJsonConsts.FLOWLISTENTRIES;
						final JsonArray flowListEntriesJsonArray = new JsonArray();
						LOG.debug("call getPomStatsFLowList : for statics information of flowList");
						index = getPomStatsFlowList(responsePacket, index,
								flowListEntriesJsonArray);
						flowListJson.add(flowListEntriesName,
								flowListEntriesJsonArray);
						vTermIfFlowFilterEntry.add(flowListName, flowListJson);
					} else {
						LOG.debug("Show ,Operation : normal and target db :state Skip flowList value strutures ");
						// increasing index to eliminate flow list entry
						// structures in case of show and op : normal
						index = responsePacket.length - 1;
					}
				}
				if (getType.equals(VtnServiceJsonConsts.LIST)
						&& opType.equalsIgnoreCase(VtnServiceJsonConsts.NORMAL)) {
					LOG.debug("List ,Operation : normal Skip value strutures ");
					index++;
				}
				// add current json object to array, if it has been initialized
				// earlier
				if (null != vTermIfFlowFilterEntryArray) {
					vTermIfFlowFilterEntryArray.add(vTermIfFlowFilterEntry);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != vTermIfFlowFilterEntryArray) {
				root.add(rootJsonName, vTermIfFlowFilterEntryArray);
			} else {
				root.add(rootJsonName, vTermIfFlowFilterEntry);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVTerminalInterfaceFlowFilterEntryResponse");

		return root;
	}

	/**
	 * Used for Vterminal Response
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return JsonObject
	 */
	public JsonObject getVTerminalResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody, final String getType) {
		LOG.trace("Start getVTerminalResponse");
		final JsonObject root = new JsonObject();
		JsonArray vterminalsArray = null;
		LOG.debug("getType: " + getType);
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}
		/*
		 * data type will be required to resolve the response structures
		 */
		String dataType = VtnServiceJsonConsts.STATE;
		if (requestBody.has(VtnServiceJsonConsts.TARGETDB)) {
			dataType = requestBody.get(VtnServiceJsonConsts.TARGETDB)
					.getAsString();
		}
		String rootJsonName;
		/*
		 * get type (show or list) will be required to resolve root json name
		 * here it will be vtn for show and vtns for list
		 */
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.VTERMINAL;

		} else {
			rootJsonName = VtnServiceJsonConsts.VTERMINALS;
			// json array will be required for list type of cases
			vterminalsArray = new JsonArray();

		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject vterminal = null;
		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			vterminal = new JsonObject();
			vterminal
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, vterminal);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {
				vterminal = new JsonObject();
				byte validBit;
				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;
				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyVTerminalStruct = (IpcStruct) responsePacket[index++];
				vterminal.addProperty(VtnServiceJsonConsts.VTERMINAL_NAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								keyVTerminalStruct,
								VtnServiceIpcConsts.VTERMINAL_NAME));
				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */
				if (getType.equals(VtnServiceJsonConsts.SHOW)
						|| opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
					LOG.debug("Case : Show or detail");
					/*
					 * add valid informations from value structure
					 */
					final IpcStruct valVTerminalStruct = (IpcStruct) responsePacket[index++];
					validBit = valVTerminalStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.valVtermIndex.UPLL_IDX_CONTROLLER_ID_VTERM
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vterminal,
								VtnServiceJsonConsts.CONTROLLERID,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVTerminalStruct,
										VtnServiceIpcConsts.CONTROLLERID));
					}
					validBit = valVTerminalStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.valVtermIndex.UPLL_IDX_DESC_VTERM
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vterminal,
								VtnServiceJsonConsts.DESCRIPTION,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVTerminalStruct,
										VtnServiceIpcConsts.VTERM_DESCRIPTION));
					}
					validBit = valVTerminalStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.valVtermIndex.UPLL_IDX_DOMAIN_ID_VTERM
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vterminal,
								VtnServiceJsonConsts.DOMAINID,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVTerminalStruct,
										VtnServiceIpcConsts.DOMAINID));
					}
					/*
					 * If data type is set as "state", then value structure will
					 * also contain the state information
					 */
					if (dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
						LOG.debug("targetdb : State");
						final IpcStruct valVtnStStruct = (IpcStruct) responsePacket[index++];
						/*
						 * If response is required in detail format then use the
						 * State value structure
						 */
						validBit = valVtnStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.valVtermStIndex.UPLL_IDX_OPER_STATUS_VTERMS
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVtnStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_UP
											.getValue())) {
								setValueToJsonObject(validBit, vterminal,
										VtnServiceJsonConsts.STATUS,
										VtnServiceJsonConsts.UP);

							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVtnStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_DOWN
											.getValue())) {
								setValueToJsonObject(validBit, vterminal,
										VtnServiceJsonConsts.STATUS,
										VtnServiceJsonConsts.DOWN);

							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVtnStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_UNKNOWN
											.getValue())) {
								setValueToJsonObject(validBit, vterminal,
										VtnServiceJsonConsts.STATUS,
										VtnServiceJsonConsts.UNKNOWN);

							} else {
								LOG.debug("Operstatus invalid");
							}
							LOG.debug("Operstatus :"
									+ IpcDataUnitWrapper
											.getIpcStructUint8Value(
													valVtnStStruct,
													VtnServiceIpcConsts.OPERSTATUS));
						}

					}
				} else {
					LOG.debug("Operation : normal Skip value struture");
					index++;
					if (dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
						LOG.debug("Operation : normal and target db :state Skip St value struture ");
						index++;
					}
				}
				// add current json object to array, if it has been initialized
				// earlier
				if (null != vterminalsArray) {
					vterminalsArray.add(vterminal);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != vterminalsArray) {
				root.add(rootJsonName, vterminalsArray);
			} else {
				root.add(rootJsonName, vterminal);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVTerminalResponse");

		return root;
	}

	/**
	 * Used for VTerminalInterface Response
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return JsonObject
	 */
	public JsonObject getVTerminalInterfaceResponse(
			final IpcDataUnit[] responsePacket, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start getVTerminalInterfaceResponse");
		final JsonObject root = new JsonObject();
		JsonArray vTermInterfaceArray = null;
		LOG.debug("getType: " + getType);
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}
		/*
		 * data type will be required to resolve the response structures
		 */
		String dataType = VtnServiceJsonConsts.STATE;
		if (requestBody.has(VtnServiceJsonConsts.TARGETDB)) {
			dataType = requestBody.get(VtnServiceJsonConsts.TARGETDB)
					.getAsString();
		}
		String rootJsonName = null;
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.INTERFACE;
		} else {
			rootJsonName = VtnServiceJsonConsts.INTERFACES;
			// json array will be required for list type of cases
			vTermInterfaceArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject vTermInterface = null;
		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			vTermInterface = new JsonObject();
			vTermInterface
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, vTermInterface);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {
				vTermInterface = new JsonObject();
				byte validBit;
				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;
				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyVtermInterfaceStruct = (IpcStruct) responsePacket[index++];
				vTermInterface.addProperty(VtnServiceJsonConsts.IFNAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								keyVtermInterfaceStruct,
								VtnServiceIpcConsts.IFNAME));
				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */
				if (getType.equals(VtnServiceJsonConsts.SHOW)
						|| opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
					LOG.debug("Case : Show or detail");
					/*
					 * add valid informations from value structure
					 */
					final IpcStruct valVTermInterfaceStruct = (IpcStruct) responsePacket[index++];

					validBit = valVTermInterfaceStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.valVtermIfIndex.UPLL_IDX_DESC_VTERMI
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vTermInterface,
								VtnServiceJsonConsts.DESCRIPTION,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVTermInterfaceStruct,
										VtnServiceIpcConsts.DESCRIPTION));
					}
					validBit = valVTermInterfaceStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.valVtermIfIndex.UPLL_IDX_ADMIN_STATUS_VTERMI
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						if (IpcDataUnitWrapper
								.getIpcStructUint8Value(
										valVTermInterfaceStruct,
										VtnServiceIpcConsts.ADMIN_STATUS)
								.equals(UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_ENABLE
										.getValue())) {
							setValueToJsonObject(validBit, vTermInterface,
									VtnServiceJsonConsts.ADMINSTATUS,
									VtnServiceJsonConsts.ENABLE);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(
										valVTermInterfaceStruct,
										VtnServiceIpcConsts.ADMIN_STATUS)
								.equals(UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_DISABLE
										.getValue())) {
							setValueToJsonObject(validBit, vTermInterface,
									VtnServiceJsonConsts.ADMINSTATUS,
									VtnServiceJsonConsts.DISABLE);
						} else {
							LOG.debug("Adminstatus : Invalid");
						}
						LOG.debug("Adminstatus :"
								+ IpcDataUnitWrapper.getIpcStructUint8Value(
										valVTermInterfaceStruct,
										VtnServiceIpcConsts.ADMIN_STATUS));
					}
					if (VtnServiceJsonConsts.STATE.equals(dataType)) {
						LOG.debug("targetdb : State");
						final IpcStruct valVbrIfStStruct = (IpcStruct) responsePacket[index++];
						validBit = valVbrIfStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.valVtermIfStIndex.UPLL_IDX_OPER_STATUS_VTERMIS
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVbrIfStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_UP
											.getValue())) {
								setValueToJsonObject(validBit, vTermInterface,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.UP);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVbrIfStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_DOWN
											.getValue())) {
								setValueToJsonObject(validBit, vTermInterface,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.DOWN);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valVbrIfStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equals(UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_UNKNOWN
											.getValue())) {
								setValueToJsonObject(validBit, vTermInterface,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.UNKNOWN);
							} else {
								LOG.debug("Operstatus : Invalid");
							}
							LOG.debug("Operstatus :"
									+ IpcDataUnitWrapper
											.getIpcStructUint8Value(
													valVbrIfStStruct,
													VtnServiceIpcConsts.OPERSTATUS));
						}
					}
				} else {
					LOG.debug("Operation : normal Skip value struture");
					index++;
					if (dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
						LOG.debug("Operation : normal and target db :state Skip St value struture ");
						index++;
					}
				}
				// add current json object to array, if it has been
				// initialized
				// earlier
				if (null != vTermInterfaceArray) {
					vTermInterfaceArray.add(vTermInterface);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != vTermInterfaceArray) {
				root.add(rootJsonName, vTermInterfaceArray);
			} else {
				root.add(rootJsonName, vTermInterface);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVTerminalInterfaceResponse");

		return root;
	}

	/**
	 * Used for PolicingProfile Response
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return JsonObject
	 */
	public JsonObject getPolicingProfileResponse(IpcDataUnit[] responsePacket,
			JsonObject requestBody, String getType) {

		LOG.trace("Start getPolicingProfileResponse");
		final JsonObject root = new JsonObject();
		JsonArray policingProfileJsonArray = null;
		LOG.debug("getType: " + getType);

		// operation type will be required to resolve the response type
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}

		String rootJsonName = null;
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.POLICINGPROFILE;
		} else {
			// json array will be required for list type of cases
			policingProfileJsonArray = new JsonArray();
			rootJsonName = VtnServiceJsonConsts.POLICINGPROFILES;
		}

		LOG.debug("Json Name :" + rootJsonName);
		JsonObject policingProfileJsonObj = null;
		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			policingProfileJsonObj = new JsonObject();
			// Create Json for Count
			policingProfileJsonObj
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, policingProfileJsonObj);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {
				policingProfileJsonObj = new JsonObject();
				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;

				// add mandatory informations from key structure
				final IpcStruct keyPolicinProfileStruct = (IpcStruct) responsePacket[index++];
				policingProfileJsonObj.addProperty(
						VtnServiceJsonConsts.PROFILE_NAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								keyPolicinProfileStruct,
								VtnServiceIpcConsts.POLICING_PROFILE_NAME));
				if (null != policingProfileJsonArray) {
					policingProfileJsonArray.add(policingProfileJsonObj);
				}
				index++;
			}
			// finally add either array or single object to root json object and
			// return the same.
			if (null != policingProfileJsonArray) {
				root.add(rootJsonName, policingProfileJsonArray);
			} else {
				root.add(rootJsonName, policingProfileJsonObj);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getPolicingProfileResponse");
		return root;
	}

	/**
	 * Create response for Policing Profile Entry Show/List API
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return JsonObject
	 */
	public JsonObject getPolicingProfileEntryResponse(
			final IpcDataUnit[] responsePacket, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start getPolicingProfileEntryResponse");
		final JsonObject root = new JsonObject();
		JsonArray profileEntriesJsonArray = null;
		LOG.debug("getType: " + getType);
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}
		// root Json Name for Show or List
		String rootJsonName = null;
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.POLICINGPROFILEENTRY;
		} else {
			rootJsonName = VtnServiceJsonConsts.POLICINGPROFILEENTRIES;
			// json array will be required for list type of cases
			profileEntriesJsonArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject profileEntryJson = null;
		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			profileEntryJson = new JsonObject();
			profileEntryJson
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, profileEntryJson);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {
				profileEntryJson = new JsonObject();
				JsonObject tworatethreecolorJson = new JsonObject();
				JsonObject meterJson = new JsonObject();
				byte validBit;
				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;
				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyPolicingProfileEntryStruct = (IpcStruct) responsePacket[index++];
				profileEntryJson.addProperty(VtnServiceJsonConsts.SEQNUM,
						IpcDataUnitWrapper.getIpcStructUint8Value(
								keyPolicingProfileEntryStruct,
								VtnServiceIpcConsts.SEQUENCENUM));

				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */
				if (getType.equals(VtnServiceJsonConsts.LIST)
						&& opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)
						|| getType.equals(VtnServiceJsonConsts.SHOW)) {
					LOG.debug("Case : Show or detail");
					/*
					 * add valid informations from value structure
					 */
					final IpcStruct valPolicingProfileEntryStruct = (IpcStruct) responsePacket[index++];

					validBit = valPolicingProfileEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.valPolicingProfileEntryIndex.UPLL_IDX_FLOWLIST_PPE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, profileEntryJson,
								VtnServiceJsonConsts.FLNAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valPolicingProfileEntryStruct,
										VtnServiceIpcConsts.FLOWLIST));
					}
					// rate parameter of Ipc Struct
					validBit = valPolicingProfileEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.valPolicingProfileEntryIndex.UPLL_IDX_RATE_PPE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						int rate = Integer.parseInt(IpcDataUnitWrapper
								.getIpcStructUint8Value(
										valPolicingProfileEntryStruct,
										VtnServiceIpcConsts.RATE));
						if (rate == UncStructIndexEnum.ValPolicingProfileRateType.UPLL_POLICINGPROFILE_RATE_KBPS
								.ordinal()) {
							setValueToJsonObject(validBit, meterJson,
									VtnServiceJsonConsts.UNIT,
									VtnServiceJsonConsts.KBPS);
						} else if (rate == UncStructIndexEnum.ValPolicingProfileRateType.UPLL_POLICINGPROFILE_RATE_PPS
								.ordinal()) {
							setValueToJsonObject(validBit, meterJson,
									VtnServiceJsonConsts.UNIT,
									VtnServiceJsonConsts.PPS);
						} else {
							LOG.debug("Invalid value of rate");
						}
						LOG.debug("rate :" + rate);
					}

					// cir parameter of Ipc Struct
					validBit = valPolicingProfileEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.valPolicingProfileEntryIndex.UPLL_IDX_CIR_PPE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, meterJson,
								VtnServiceJsonConsts.CIR,
								IpcDataUnitWrapper.getIpcStructUint32Value(
										valPolicingProfileEntryStruct,
										VtnServiceIpcConsts.CIR));
					}

					// cbs parameter of Ipc Struct
					validBit = valPolicingProfileEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.valPolicingProfileEntryIndex.UPLL_IDX_CBS_PPE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, meterJson,
								VtnServiceJsonConsts.CBS,
								IpcDataUnitWrapper.getIpcStructUint32Value(
										valPolicingProfileEntryStruct,
										VtnServiceIpcConsts.CBS));
					}

					// cbs parameter of Ipc Struct
					validBit = valPolicingProfileEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.valPolicingProfileEntryIndex.UPLL_IDX_PIR_PPE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, meterJson,
								VtnServiceJsonConsts.PIR,
								IpcDataUnitWrapper.getIpcStructUint32Value(
										valPolicingProfileEntryStruct,
										VtnServiceIpcConsts.PIR));
					}

					// pbs parameter of Ipc Struct
					validBit = valPolicingProfileEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.valPolicingProfileEntryIndex.UPLL_IDX_PBS_PPE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, meterJson,
								VtnServiceJsonConsts.PBS,
								IpcDataUnitWrapper.getIpcStructUint32Value(
										valPolicingProfileEntryStruct,
										VtnServiceIpcConsts.PBS));
					}
					// add meter json to tworatethreecolor json
					tworatethreecolorJson.add(VtnServiceJsonConsts.METER,
							meterJson);

					List<String> paramNames = new ArrayList<String>();
					// set green json to tworatethreecolor json
					paramNames.add(VtnServiceIpcConsts.GREENACTION);
					paramNames.add(VtnServiceIpcConsts.GREENACTIONDSCP);
					paramNames.add(VtnServiceIpcConsts.GREENACTIONPRIORITY);
					paramNames
							.add(VtnServiceIpcConsts.GREENACTIONDROPPRECEDENCE);
					tworatethreecolorJson
							.add(VtnServiceJsonConsts.GREENACTION,
									createActionJson(
											valPolicingProfileEntryStruct,
											paramNames,
											UncStructIndexEnum.valPolicingProfileEntryIndex.UPLL_IDX_GREEN_ACTION_PPE
													.ordinal()));
					// set yellow json to tworatethreecolor json
					paramNames.clear();
					paramNames.add(VtnServiceIpcConsts.YELLOWACTION);
					paramNames.add(VtnServiceIpcConsts.YELLOWACTIONDSCP);
					paramNames.add(VtnServiceIpcConsts.YELLOWACTIONPRIORITY);
					paramNames
							.add(VtnServiceIpcConsts.YELLOWACTIONDROPPRECEDENCE);
					tworatethreecolorJson
							.add(VtnServiceJsonConsts.YELLOWACTION,
									createActionJson(
											valPolicingProfileEntryStruct,
											paramNames,
											UncStructIndexEnum.valPolicingProfileEntryIndex.UPLL_IDX_YELLOW_ACTION_PPE
													.ordinal()));
					// set red json to tworatethreecolor json
					paramNames.clear();
					paramNames.add(VtnServiceIpcConsts.REDACTION);
					paramNames.add(VtnServiceIpcConsts.REDACTIONDSCP);
					paramNames.add(VtnServiceIpcConsts.REDACTIONPRIORITY);
					paramNames.add(VtnServiceIpcConsts.REDACTIONDROPPRECEDENCE);
					tworatethreecolorJson
							.add(VtnServiceJsonConsts.REDACTION,
									createActionJson(
											valPolicingProfileEntryStruct,
											paramNames,
											UncStructIndexEnum.valPolicingProfileEntryIndex.UPLL_IDX_RED_ACTION_PPE
													.ordinal()));

					profileEntryJson.add(
							VtnServiceJsonConsts.TWORATETHREECOLOR,
							tworatethreecolorJson);
				}

				/*
				 * add current json object to array, if it has been initialized
				 * earlier
				 */
				if (getType.equals(VtnServiceJsonConsts.LIST)
						&& opType.equalsIgnoreCase(VtnServiceJsonConsts.NORMAL)) {
					LOG.debug("List ,Operation : normal Skip value strutures ");
					index++;
				}
				// add current json object to array, if it has been initialized
				// earlier
				if (null != profileEntriesJsonArray) {
					profileEntriesJsonArray.add(profileEntryJson);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != profileEntriesJsonArray) {
				root.add(rootJsonName, profileEntriesJsonArray);
			} else {
				root.add(rootJsonName, profileEntryJson);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getPolicingProfileEntryResponse");
		return root;
	}

	/**
	 * Create JSON object for green/yellow/red actions with all parameters
	 * inside that
	 * 
	 * @param valPolicingProfileEntryStruct
	 * @param paramNames
	 *            - parameter names
	 * @param startIndex
	 */
	private JsonObject createActionJson(
			final IpcStruct valPolicingProfileEntryStruct,
			List<String> paramNames, int startIndex) {
		byte validBit;
		JsonObject actionJson = new JsonObject();
		validBit = valPolicingProfileEntryStruct.getByte(
				VtnServiceIpcConsts.VALID, startIndex++);
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			int actionType = Integer.parseInt(IpcDataUnitWrapper
					.getIpcStructUint8Value(valPolicingProfileEntryStruct,
							paramNames.get(0)));
			if (actionType == UncStructIndexEnum.ValPolicingProfileAction.UPLL_POLICINGPROFILE_ACT_PASS
					.ordinal()) {
				setValueToJsonObject(validBit, actionJson,
						VtnServiceJsonConsts.TYPE, VtnServiceJsonConsts.PASS);
			} else if (actionType == UncStructIndexEnum.ValPolicingProfileAction.UPLL_POLICINGPROFILE_ACT_DROP
					.ordinal()) {
				setValueToJsonObject(validBit, actionJson,
						VtnServiceJsonConsts.TYPE, VtnServiceJsonConsts.DROP);
			} else if (actionType == UncStructIndexEnum.ValPolicingProfileAction.UPLL_POLICINGPROFILE_ACT_PENALTY
					.ordinal()) {
				setValueToJsonObject(validBit, actionJson,
						VtnServiceJsonConsts.TYPE, VtnServiceJsonConsts.PENALTY);
			} else {
				LOG.debug("Invalid value of type");
			}
		}

		validBit = valPolicingProfileEntryStruct.getByte(
				VtnServiceIpcConsts.VALID, startIndex++);
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, actionJson,
					VtnServiceJsonConsts.PRIORITY,
					IpcDataUnitWrapper.getIpcStructUint8Value(
							valPolicingProfileEntryStruct, paramNames.get(1)));
		}

		validBit = valPolicingProfileEntryStruct.getByte(
				VtnServiceIpcConsts.VALID, startIndex++);
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, actionJson,
					VtnServiceJsonConsts.DSCP,
					IpcDataUnitWrapper.getIpcStructUint8Value(
							valPolicingProfileEntryStruct, paramNames.get(2)));
		}

		validBit = valPolicingProfileEntryStruct.getByte(
				VtnServiceIpcConsts.VALID, startIndex++);
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, actionJson,
					VtnServiceJsonConsts.DROPPRECEDENCE,
					IpcDataUnitWrapper.getIpcStructUint8Value(
							valPolicingProfileEntryStruct, paramNames.get(3)));
		}
		return actionJson;
	}

	/**
	 * Used for PolicingMap Response
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return JsonObject
	 */
	public JsonObject getVtnPolicingMapResponse(IpcDataUnit[] responsePacket,
			JsonObject requestBody, String getType) {
		LOG.trace("Start getVtnPolicingMapResponse()");
		final JsonObject root = new JsonObject();
		final JsonObject policingmapJson = new JsonObject();
		// flag to be set in case detail information
		boolean detailedInfo = true;
		if (requestBody.has(VtnServiceJsonConsts.TARGETDB)
				&& !requestBody.get(VtnServiceJsonConsts.TARGETDB)
						.getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
			// create response with only profile_name
			detailedInfo = false;
		} else {
			if (requestBody.has(VtnServiceJsonConsts.CONTROLLERID)
					&& requestBody.has(VtnServiceJsonConsts.DOMAINID)) {
				// create response with detail information
				detailedInfo = true;
			} else {
				// create response with only profile_name
				detailedInfo = false;
			}
		}

		// ignore key-type and key-strucrure, as they are have no information
		// that is reqiued in response
		int index = 2;
		if (detailedInfo) {
			/*
			 * there is no use of key-type, key-structure, val_policingmap and
			 * val_policingmap_controller so start response packet iteration
			 * from 4th index
			 */
			getDetailPolicingMap(responsePacket, policingmapJson, 4,
					VtnServiceJsonConsts.DETAIL);
		} else {
			final IpcStruct valPolicingMapStruct = (IpcStruct) responsePacket[index];
			byte validBit = valPolicingMapStruct
					.getByte(
							VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValPolicingMapIndex.UPLL_IDX_POLICERNAME_PM
									.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, policingmapJson,
						VtnServiceJsonConsts.PROFILE_NAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valPolicingMapStruct,
								VtnServiceIpcConsts.POLICER_NAME));
			}
		}
		root.add(VtnServiceJsonConsts.POLICINGMAP, policingmapJson);
		LOG.trace("Start getVtnPolicingMapResponse()");
		LOG.debug("response Json: " + root.toString());
		return root;
	}

	/**
	 * Create response for show policing-map APIs. common for vBridge/vBridge
	 * Interface/vTerminal Interface
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return JsonObject
	 */
	public JsonObject getPolicingMapResponse(IpcDataUnit[] responsePacket,
			JsonObject requestBody, String getType) {

		LOG.trace("Start getPolicingMapResponse()");
		final JsonObject root = new JsonObject();
		JsonObject policingMapJsonObject = new JsonObject();

		LOG.debug("getType: " + getType);
		// set default valiue of op, if not set
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}
		// set default valiue of targetdb, if not set
		String targetDB = VtnServiceJsonConsts.STATE;
		if (requestBody.has(VtnServiceJsonConsts.TARGETDB)) {
			targetDB = requestBody.get(VtnServiceJsonConsts.TARGETDB)
					.getAsString();
		}
		// there is no use of key-type and key-structure, so start response
		// packet iteration from 2nd index
		int index = 2;
		final IpcStruct valPolicingMapStruct = (IpcStruct) responsePacket[index++];
		byte validBit = valPolicingMapStruct.getByte(VtnServiceIpcConsts.VALID,
				UncStructIndexEnum.ValPolicingMapIndex.UPLL_IDX_POLICERNAME_PM
						.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, policingMapJsonObject,
					VtnServiceJsonConsts.PROFILE_NAME,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valPolicingMapStruct,
							VtnServiceIpcConsts.POLICER_NAME));
		}
		// opType and targetDB is used for Detail case
		if (targetDB.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
			/*
			 * there is no use of key-type, key-structure and val_policingmap,
			 * so start responsepacket iteration from 3rd index
			 */
			getDetailPolicingMap(responsePacket, policingMapJsonObject, 3,
					opType);
		}
		root.add(VtnServiceJsonConsts.POLICINGMAP, policingMapJsonObject);
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getPolicingMapResponse");
		return root;
	}

	/**
	 * Create response for show policing-map APIs with detail case. common for
	 * vBridge/vBridge Interface/vTerminal Interface
	 * 
	 * @param responsePacket
	 * @param profileJsonObject
	 */
	private void getDetailPolicingMap(IpcDataUnit[] responsePacket,
			JsonObject policingmapJson, int index, String opType) {
		JsonArray switchesjsonArray = null;
		JsonArray policingProfileEntriesjsonArray;
		JsonObject policingProfileEntriesjsonObject;
		JsonObject switchesjsonObject;
		byte validBit;
		policingProfileEntriesjsonArray = new JsonArray();
		while (index < responsePacket.length) {
			final IpcStruct valPolicingMapControllerSt = (IpcStruct) responsePacket[index++];
			policingProfileEntriesjsonObject = new JsonObject();
			// sequence number
			validBit = valPolicingMapControllerSt
					.getByte(
							VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValPolicingMapControllerStIndex.UPLL_IDX_SEQ_NUM_PMCS
									.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit,
						policingProfileEntriesjsonObject,
						VtnServiceJsonConsts.SEQNUM,
						IpcDataUnitWrapper.getIpcStructUint8Value(
								valPolicingMapControllerSt,
								VtnServiceIpcConsts.SEQUENCENUM));
			}
			if (opType.equals(VtnServiceJsonConsts.DETAIL)) {
				JsonObject statisticJsonObject = new JsonObject();
				// set pom-stats for total
				validBit = valPolicingMapControllerSt
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValPolicingMapControllerStIndex.UPLL_IDX_TOTAL_PMCS
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					statisticJsonObject.add(VtnServiceJsonConsts.TOTAL,
							setPomStatusToJson(IpcDataUnitWrapper
									.getInnerIpcStruct(
											valPolicingMapControllerSt,
											VtnServiceIpcConsts.TOTAL)));
				}

				// set pom-stats for green_yellow
				validBit = valPolicingMapControllerSt
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValPolicingMapControllerStIndex.UPLL_IDX_GREEN_YELLOW_PMCS
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					statisticJsonObject.add(VtnServiceJsonConsts.GREEN_YELLOW,
							setPomStatusToJson(IpcDataUnitWrapper
									.getInnerIpcStruct(
											valPolicingMapControllerSt,
											VtnServiceIpcConsts.GREEN_YELLOW)));
				}

				// set pom-stats for red
				validBit = valPolicingMapControllerSt
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValPolicingMapControllerStIndex.UPLL_IDX_RED_PMCS
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					statisticJsonObject.add(VtnServiceJsonConsts.RED,
							setPomStatusToJson(IpcDataUnitWrapper
									.getInnerIpcStruct(
											valPolicingMapControllerSt,
											VtnServiceIpcConsts.RED)));
				}

				policingProfileEntriesjsonObject.add(
						VtnServiceJsonConsts.STATISTICS, statisticJsonObject);
			}

			// there is no use of val_policingprofile_entry
			index++;

			while (index < responsePacket.length) {
				IpcStruct valPolicingMapSwitchSt = (IpcStruct) responsePacket[index++];
				if (valPolicingMapSwitchSt.getName().equalsIgnoreCase(
						VtnServiceIpcConsts.ValPolicingMapSwitchSt)) {
					if (switchesjsonArray == null) {
						switchesjsonArray = new JsonArray();
					}
					switchesjsonObject = new JsonObject();
					validBit = valPolicingMapSwitchSt
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValPolicingMapSwitchStIndex.UPLL_IDX_POLICER_ID_PMSS
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, switchesjsonObject,
								VtnServiceJsonConsts.POLICERID,
								IpcDataUnitWrapper.getIpcStructUint32Value(
										valPolicingMapSwitchSt,
										VtnServiceIpcConsts.POLICERID));
					}

					validBit = valPolicingMapSwitchSt
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValPolicingMapSwitchStIndex.UPLL_IDX_SWITCH_ID_PMSS
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, switchesjsonObject,
								VtnServiceJsonConsts.SWITCH_ID,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valPolicingMapSwitchSt,
										VtnServiceIpcConsts.SWITCHID));
					}

					validBit = valPolicingMapSwitchSt
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValPolicingMapSwitchStIndex.UPLL_IDX_VBR_NAME_PMSS
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, switchesjsonObject,
								VtnServiceJsonConsts.VNODENAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valPolicingMapSwitchSt,
										VtnServiceIpcConsts.VBRNAME));
					}

					validBit = valPolicingMapSwitchSt
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValPolicingMapSwitchStIndex.UPLL_IDX_IF_NAME_PMSS
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, switchesjsonObject,
								VtnServiceJsonConsts.IFNAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valPolicingMapSwitchSt,
										VtnServiceIpcConsts.IFNAME));
					}

					validBit = valPolicingMapSwitchSt
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValPolicingMapSwitchStIndex.UPLL_IDX_PORT_NAME_PMSS
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, switchesjsonObject,
								VtnServiceJsonConsts.PORTNAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valPolicingMapSwitchSt,
										VtnServiceIpcConsts.PORTNAME));
					}

					validBit = valPolicingMapSwitchSt
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValPolicingMapSwitchStIndex.UPLL_IDX_VLAN_ID_PMSS
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, switchesjsonObject,
								VtnServiceJsonConsts.VLAN_ID,
								IpcDataUnitWrapper.getIpcStructUint16Value(
										valPolicingMapSwitchSt,
										VtnServiceIpcConsts.VLANID1));
					}

					validBit = valPolicingMapSwitchSt
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValPolicingMapSwitchStIndex.UPLL_IDX_STATUS_PMSS
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, switchesjsonObject,
								VtnServiceJsonConsts.STATUS,
								IpcDataUnitWrapper.getIpcStructUint8Value(
										valPolicingMapSwitchSt,
										VtnServiceIpcConsts.STATUS));
					}
					if (opType.equals(VtnServiceJsonConsts.DETAIL)) {
						JsonObject switchStatisticJsonObject = new JsonObject();
						// set pom-stats for total
						validBit = valPolicingMapSwitchSt
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.ValPolicingMapSwitchStIndex.UPLL_IDX_TOTAL_PMSS
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							switchStatisticJsonObject
									.add(VtnServiceJsonConsts.TOTAL,
											setPomStatusToJson(IpcDataUnitWrapper
													.getInnerIpcStruct(
															valPolicingMapSwitchSt,
															VtnServiceIpcConsts.TOTAL)));
						}

						// set pom-stats for green_yellow
						validBit = valPolicingMapSwitchSt
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.ValPolicingMapSwitchStIndex.UPLL_IDX_GREEN_YELLOW_PMSS
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							switchStatisticJsonObject
									.add(VtnServiceJsonConsts.GREEN_YELLOW,
											setPomStatusToJson(IpcDataUnitWrapper
													.getInnerIpcStruct(
															valPolicingMapSwitchSt,
															VtnServiceIpcConsts.GREEN_YELLOW)));
						}

						// set pom-stats for red
						validBit = valPolicingMapSwitchSt
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.ValPolicingMapSwitchStIndex.UPLL_IDX_RED_PMSS
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							switchStatisticJsonObject.add(
									VtnServiceJsonConsts.RED,
									setPomStatusToJson(IpcDataUnitWrapper
											.getInnerIpcStruct(
													valPolicingMapSwitchSt,
													VtnServiceIpcConsts.RED)));
						}
						switchesjsonObject.add(VtnServiceJsonConsts.STATISTICS,
								switchStatisticJsonObject);
					}
					switchesjsonArray.add(switchesjsonObject);
				} else {
					index--;
					break;
				}
			}
			if (switchesjsonArray != null) {
				policingProfileEntriesjsonObject.add(
						VtnServiceJsonConsts.SWITCHES, switchesjsonArray);
			}
			policingProfileEntriesjsonArray
					.add(policingProfileEntriesjsonObject);
		}
		policingmapJson.add(VtnServiceJsonConsts.POLICINGPROFILEENTRIES,
				policingProfileEntriesjsonArray);
	}

	/**
	 * Create JSON with packets/octes information from pom_stats
	 * 
	 * @param pomStatus
	 *            - IpcStruct containing information for packets/bytes
	 * @return - JsonObject
	 */
	private JsonElement setPomStatusToJson(IpcStruct pomStatus) {
		JsonObject pomJson = new JsonObject();
		byte validBit;
		/*
		 * set information for packets, after checking valid bit
		 */
		validBit = pomStatus.getByte(VtnServiceIpcConsts.VALID,
				UncStructIndexEnum.ValPomStatsIndex.UPLL_IDX_STATS_PACKETS
						.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, pomJson,
					VtnServiceJsonConsts.PACKETS,
					IpcDataUnitWrapper.getIpcStructUint64Value(pomStatus,
							VtnServiceIpcConsts.PACKETS));
		}

		/*
		 * set information for octets, after checking valid bit
		 */
		validBit = pomStatus.getByte(VtnServiceIpcConsts.VALID,
				UncStructIndexEnum.ValPomStatsIndex.UPLL_IDX_STATS_BYTES
						.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, pomJson,
					VtnServiceJsonConsts.OCTETS,
					IpcDataUnitWrapper.getIpcStructUint64Value(pomStatus,
							VtnServiceIpcConsts.BYTES));
		}
		return pomJson;
	}

	/**
	 * Create response for show Path Map Entry APIs.
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return
	 */
	public JsonObject getPathMapEntryResponse(IpcDataUnit[] responsePacket,
			JsonObject requestBody, String getType) {
		LOG.trace("Start getPathMapEntryResponse");
		final JsonObject root = new JsonObject();
		JsonArray pathMapEntriesJsonArray = null;
		LOG.debug("getType: " + getType);
		// operation type will be required to resolve the response type
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}

		// root json name as per Show or List Response
		String rootJsonName = null;
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.PATHMAPENTRY;
		} else {
			// json array will be required for list type of cases
			pathMapEntriesJsonArray = new JsonArray();
			rootJsonName = VtnServiceJsonConsts.PATHMAPENTRIES;
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject pathMapEntryJsonObj = null;
		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			pathMapEntryJsonObj = new JsonObject();
			// Create Json for Count
			pathMapEntryJsonObj
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, pathMapEntryJsonObj);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {
				pathMapEntryJsonObj = new JsonObject();
				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;

				// add mandatory informations from key structure
				final IpcStruct keyPathMapEntryStruct = (IpcStruct) responsePacket[index++];
				pathMapEntryJsonObj.addProperty(VtnServiceJsonConsts.SEQNUM,
						IpcDataUnitWrapper.getIpcStructUint16Value(
								keyPathMapEntryStruct,
								VtnServiceIpcConsts.SEQUENCENUM));

				final IpcStruct valPathMapEntryStruct = (IpcStruct) responsePacket[index++];
				// ignore fl_name setting for List with summary information
				if (getType.equalsIgnoreCase(VtnServiceJsonConsts.SHOW)
						|| (getType.equalsIgnoreCase(VtnServiceJsonConsts.LIST) && opType
								.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL))) {
					byte validBit = valPathMapEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVtnPathMapEntryIndex.UPLL_IDX_FLOWLIST_NAME_VPME
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, pathMapEntryJsonObj,
								VtnServiceJsonConsts.FLNAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valPathMapEntryStruct,
										VtnServiceIpcConsts.FLOWLIST_NAME));
					}
				}

				/*
				 * ignore val_vtn_pathmap_ppolicy_entry value structures
				 * continue loop, if same structure is coming again and again
				 * till response packet length. Break the loop when NULL
				 */
				while (index < responsePacket.length) {
					final IpcStruct valVtnPathmapPpolicyEntry = (IpcStruct) responsePacket[index++];
					if (valVtnPathmapPpolicyEntry != null
							&& valVtnPathmapPpolicyEntry
									.getName()
									.equalsIgnoreCase(
											VtnServiceIpcConsts.ValVtnPathMapPPolicyEntry)) {
						continue;
					} else {
						index--;
						break;
					}
				}
				if (null != pathMapEntriesJsonArray) {
					pathMapEntriesJsonArray.add(pathMapEntryJsonObj);
				}
			}
			// finally add either array or single object to root json object and
			// return the same.
			if (null != pathMapEntriesJsonArray) {
				root.add(rootJsonName, pathMapEntriesJsonArray);
			} else {
				root.add(rootJsonName, pathMapEntryJsonObj);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getPathMapEntryResponse");
		return root;
	}

	/**
	 * Create response for show Path Policy Entry APIs.
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return JsonObject
	 */
	public JsonObject getPathPolicyEntryResponse(IpcDataUnit[] responsePacket,
			JsonObject requestBody, String getType) {
		LOG.trace("Start getPathPolicyEntryResponse()");
		final JsonObject root = new JsonObject();
		JsonArray pPolicyEntryArray = null;
		/*
		 * op type will be required to resolve the response type
		 */
		LOG.debug("getType: " + getType);
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}
		/*
		 * data type will be required to resolve the response structures
		 */
		String dataType = VtnServiceJsonConsts.STATE;
		if (requestBody.has(VtnServiceJsonConsts.TARGETDB)) {
			dataType = requestBody.get(VtnServiceJsonConsts.TARGETDB)
					.getAsString();
		}
		String rootJsonName;
		/*
		 * get type (show or list) will be required to resolve root json name
		 * here it will be vtn for show and vtns for list
		 */
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.PATHPOLICYENTRY;
		} else {
			rootJsonName = VtnServiceJsonConsts.PATHPOLICYENTRIES;
			// json array will be required for list type of cases
			pPolicyEntryArray = new JsonArray();
		}

		LOG.debug("Json Name :" + rootJsonName);
		JsonObject pPolicyEntry = null;

		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			pPolicyEntry = new JsonObject();
			pPolicyEntry
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, pPolicyEntry);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {
				pPolicyEntry = new JsonObject();
				byte validBit;
				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;
				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyVtnpPolicyEntryStruct = (IpcStruct) responsePacket[index++];

				String controllerId = IpcDataUnitWrapper
						.getIpcStructUint8ArrayValue(keyVtnpPolicyEntryStruct,
								VtnServiceIpcConsts.CONTROLLERID);
				String domainId = IpcDataUnitWrapper
						.getIpcStructUint8ArrayValue(keyVtnpPolicyEntryStruct,
								VtnServiceIpcConsts.DOMAINID);
				pPolicyEntry.addProperty(VtnServiceJsonConsts.ENTRYID,
						controllerId + VtnServiceJsonConsts.HYPHEN + domainId);

				if (getType.equals(VtnServiceJsonConsts.LIST)
						&& opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)
						|| getType.equals(VtnServiceJsonConsts.SHOW)) {
					LOG.debug("Case : Show or List with detail");
					pPolicyEntry.addProperty(VtnServiceJsonConsts.CONTROLLERID,
							controllerId);
					pPolicyEntry.addProperty(VtnServiceJsonConsts.DOMAINID,
							domainId);

					/*
					 * add valid informations from value structure
					 */
					final IpcStruct valpPolicyEntryStruct = (IpcStruct) responsePacket[index++];

					validBit = valpPolicyEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVtnPathmapPpolicyEntryIndex.UPLL_IDX_POLICY_ID_VPMPPE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, pPolicyEntry,
								VtnServiceJsonConsts.POLICYID,
								IpcDataUnitWrapper.getIpcStructUint8Value(
										valpPolicyEntryStruct,
										VtnServiceIpcConsts.POLICYID));
					}
					validBit = valpPolicyEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVtnPathmapPpolicyEntryIndex.UPLL_IDX_AGING_TIME_OUT_VPMPPE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, pPolicyEntry,
								VtnServiceJsonConsts.AGEOUTTIME,
								IpcDataUnitWrapper.getIpcStructUint16Value(
										valpPolicyEntryStruct,
										VtnServiceIpcConsts.AGINGOUTTIME));
					}
				} else if (getType.equals(VtnServiceJsonConsts.LIST)
						&& opType.equalsIgnoreCase(VtnServiceJsonConsts.NORMAL)) {
					// skip val struct
					index++;
				}

				if (getType.equals(VtnServiceJsonConsts.SHOW)
						&& dataType
								.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
					LOG.debug("state db case");
					// skip val_vtn_pathmap_entry
					index++;

					// val structure val_vtn_pathmap_ppolicy_entry_stat
					final IpcStruct valpPolicyEntryStStruct = (IpcStruct) responsePacket[index++];
					validBit = valpPolicyEntryStStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVtnPathmapPpolicyEntryStat.UPLL_IDX_POLICY_TYPE_VVPMPPES
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, pPolicyEntry,
								VtnServiceJsonConsts.POLICYTYPE,
								IpcDataUnitWrapper.getIpcStructUint8Value(
										valpPolicyEntryStStruct,
										VtnServiceIpcConsts.POLICYTYPE));
					}

					if (opType.equals(VtnServiceJsonConsts.DETAIL)) {
						LOG.info("call getPolicyEntryPomStats : for statics information");
						getPolicyEntryPomStats(pPolicyEntry,
								IpcDataUnitWrapper.getInnerIpcStruct(
										valpPolicyEntryStStruct,
										VtnServiceIpcConsts.PMAP_HWSTATS));
					}
					// flowlist entry count
					int flowListEntryCount = Integer
							.parseInt(IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacket[index++]));

					JsonArray flowListEntriesArray = new JsonArray();
					for (int i = 0; i < flowListEntryCount; i++) {
						// val structure val_vtn_pathmap_ppolicy_flow_entry_stat
						final IpcStruct valFlowFilterEntryStStruct = (IpcStruct) responsePacket[index++];
						if (opType.equals(VtnServiceJsonConsts.DETAIL)) {
							JsonObject flowListEntry = new JsonObject();
							validBit = valFlowFilterEntryStStruct
									.getByte(
											VtnServiceIpcConsts.VALID,
											UncStructIndexEnum.ValVtnPathmapPpolicyFlowEntryStatIndex.UPLL_IDX_SEQ_NUM_PMPPFES
													.ordinal());
							if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
									.ordinal()
									&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
											.ordinal()) {
								setValueToJsonObject(
										validBit,
										flowListEntry,
										VtnServiceJsonConsts.SEQNUM,
										IpcDataUnitWrapper
												.getIpcStructUint16Value(
														valFlowFilterEntryStStruct,
														VtnServiceIpcConsts.SEQUENCENUM));
							}
							validBit = valFlowFilterEntryStStruct
									.getByte(
											VtnServiceIpcConsts.VALID,
											UncStructIndexEnum.ValVtnPathmapPpolicyFlowEntryStatIndex.UPLL_IDX_FL_STATS_PMPPFES
													.ordinal());
							if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
									.ordinal()
									&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
											.ordinal()) {
								LOG.debug("call getPolicyEntryPomStats : for statics information");
								getPolicyEntryPomStats(flowListEntry,
										IpcDataUnitWrapper.getInnerIpcStruct(
												valFlowFilterEntryStStruct,
												VtnServiceIpcConsts.FL_HWSTATS));
							}
							flowListEntriesArray.add(flowListEntry);
						}
					}
					if (opType.equals(VtnServiceJsonConsts.DETAIL)) {
						JsonObject flowlistJsonObj = new JsonObject();
						flowlistJsonObj.add(
								VtnServiceJsonConsts.FLOWLISTENTRIES,
								flowListEntriesArray);
						pPolicyEntry.add(VtnServiceJsonConsts.FLOWLIST,
								flowlistJsonObj);
					}

					// link weight count
					int linkWeightCount = Integer.parseInt(IpcDataUnitWrapper
							.getIpcDataUnitValue(responsePacket[index++]));

					JsonArray linkWeightArray = new JsonArray();
					for (int i = 0; i < linkWeightCount; i++) {
						// val structure
						// val_pmap_ppolicy_entry_flowlist_weight_st
						final IpcStruct valPmapPpolicyEntryFlowlistWeightSt = (IpcStruct) responsePacket[index++];
						JsonObject linkWeightObj = new JsonObject();
						validBit = valPmapPpolicyEntryFlowlistWeightSt
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.valPmapPpolicyEntryFlWeightStIndex.UPLL_IDX_SWITCH_ID_PMPPFLW
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							setValueToJsonObject(
									validBit,
									linkWeightObj,
									VtnServiceJsonConsts.SWITCHID,
									IpcDataUnitWrapper
											.getIpcStructUint8ArrayValue(
													valPmapPpolicyEntryFlowlistWeightSt,
													VtnServiceIpcConsts.SWITCHID));
						}
						validBit = valPmapPpolicyEntryFlowlistWeightSt
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.valPmapPpolicyEntryFlWeightStIndex.UPLL_IDX_PORT_ID_PMPPFLW
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							setValueToJsonObject(
									validBit,
									linkWeightObj,
									VtnServiceJsonConsts.PORTNAME,
									IpcDataUnitWrapper
											.getIpcStructUint8ArrayValue(
													valPmapPpolicyEntryFlowlistWeightSt,
													VtnServiceIpcConsts.PORT_ID));
						}
						validBit = valPmapPpolicyEntryFlowlistWeightSt
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.valPmapPpolicyEntryFlWeightStIndex.UPLL_IDX_LINK_WEIGHT_PMPPFLW
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							setValueToJsonObject(
									validBit,
									linkWeightObj,
									VtnServiceJsonConsts.WEIGHT,
									IpcDataUnitWrapper
											.getIpcStructUint32Value(
													valPmapPpolicyEntryFlowlistWeightSt,
													VtnServiceIpcConsts.LINK_WEIGHT));
						}
						linkWeightArray.add(linkWeightObj);
					}
					pPolicyEntry.add(VtnServiceJsonConsts.LINK_WEIGHTS,
							linkWeightArray);

				}

				// If object has been initialized earlier, add it to array
				if (null != pPolicyEntryArray) {
					pPolicyEntryArray.add(pPolicyEntry);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != pPolicyEntryArray) {
				root.add(rootJsonName, pPolicyEntryArray);
			} else {
				root.add(rootJsonName, pPolicyEntry);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getPathPolicyEntryResponse()");
		return root;
	}

	/**
	 * Create JSON containing statistics information for Path Policy Entry API
	 * 
	 * @param targetJson
	 * @param responseStruct
	 * @param pomStatsIndexSet
	 */
	public void getPolicyEntryPomStats(final JsonObject targetJson,
			final IpcStruct responseStruct) {
		LOG.trace("Start getPolicyEntryPomStats");
		byte validBit;

		final JsonObject statisticsFFJson = new JsonObject();
		final JsonObject softwareFFJson = new JsonObject();
		final JsonObject existFFJson = new JsonObject();
		final JsonObject expireFFJson = new JsonObject();
		final JsonObject totalFFJson = new JsonObject();

		/*
		 * set packets and octats informaation for software pom-stats
		 */
		validBit = responseStruct.getByte(VtnServiceIpcConsts.VALID,
				UncStructIndexEnum.PomPmapStatsIndex.UPLL_IDX_SOFT_PPSI
						.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			LOG.info("information addition is in-progress for software pom-stats");
			IpcStruct softStruct = IpcDataUnitWrapper.getInnerIpcStruct(
					responseStruct, VtnServiceIpcConsts.SOFT);
			validBit = softStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValPomStatsIndex.UPLL_IDX_STATS_PACKETS
							.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, softwareFFJson,
						VtnServiceJsonConsts.PACKETS,
						IpcDataUnitWrapper.getIpcStructUint64Value(softStruct,
								VtnServiceIpcConsts.PACKETS));
			}
			validBit = softStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValPomStatsIndex.UPLL_IDX_STATS_BYTES
							.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, softwareFFJson,
						VtnServiceJsonConsts.OCTETS,
						IpcDataUnitWrapper.getIpcStructUint64Value(softStruct,
								VtnServiceIpcConsts.BYTES));
			}
		}

		/*
		 * set packets and octats informaation for existing flow pom-stats
		 */
		validBit = responseStruct.getByte(VtnServiceIpcConsts.VALID,
				UncStructIndexEnum.PomPmapStatsIndex.UPLL_IDX_EXIST_FLOW_PPSI
						.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			LOG.info("information addition is in-progress for existing flow pom-stats");
			IpcStruct existFlowStruct = IpcDataUnitWrapper.getInnerIpcStruct(
					responseStruct, VtnServiceIpcConsts.EXISTFLOW);
			validBit = existFlowStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValPomStatsIndex.UPLL_IDX_STATS_PACKETS
							.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, existFFJson,
						VtnServiceJsonConsts.PACKETS,
						IpcDataUnitWrapper.getIpcStructUint64Value(
								existFlowStruct, VtnServiceJsonConsts.PACKETS));
			}
			validBit = existFlowStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValPomStatsIndex.UPLL_IDX_STATS_BYTES
							.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, existFFJson,
						VtnServiceJsonConsts.OCTETS,
						IpcDataUnitWrapper.getIpcStructUint64Value(
								existFlowStruct, VtnServiceIpcConsts.BYTES));
			}
		}

		/*
		 * set packets and octats informaation for expired flow pom-stats
		 */
		validBit = responseStruct.getByte(VtnServiceIpcConsts.VALID,
				UncStructIndexEnum.PomPmapStatsIndex.UPLL_IDX_EXPIRED_FLOW_PPSI
						.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			LOG.info("information addition is in-progress for expired flow pom-stats");
			IpcStruct expiredFlowStruct = IpcDataUnitWrapper.getInnerIpcStruct(
					responseStruct, VtnServiceIpcConsts.EXPIRED_FLOW);
			validBit = expiredFlowStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValPomStatsIndex.UPLL_IDX_STATS_PACKETS
							.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, expireFFJson,
						VtnServiceJsonConsts.PACKETS,
						IpcDataUnitWrapper
								.getIpcStructUint64Value(expiredFlowStruct,
										VtnServiceJsonConsts.PACKETS));
			}
			validBit = expiredFlowStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValPomStatsIndex.UPLL_IDX_STATS_BYTES
							.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, expireFFJson,
						VtnServiceJsonConsts.OCTETS,
						IpcDataUnitWrapper.getIpcStructUint64Value(
								expiredFlowStruct, VtnServiceIpcConsts.BYTES));
			}
		}

		/*
		 * set packets and octats informaation for total pom-stats
		 */
		validBit = responseStruct.getByte(VtnServiceIpcConsts.VALID,
				UncStructIndexEnum.PomPmapStatsIndex.UPLL_IDX_TOTAL_PPSI
						.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			LOG.info("information addition is in-progress for total pom-stats");
			IpcStruct totalStruct = IpcDataUnitWrapper.getInnerIpcStruct(
					responseStruct, VtnServiceIpcConsts.TOTAL);
			validBit = totalStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValPomStatsIndex.UPLL_IDX_STATS_PACKETS
							.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, totalFFJson,
						VtnServiceJsonConsts.PACKETS,
						IpcDataUnitWrapper.getIpcStructUint64Value(totalStruct,
								VtnServiceIpcConsts.PACKETS));
			}
			validBit = totalStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValPomStatsIndex.UPLL_IDX_STATS_BYTES
							.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, totalFFJson,
						VtnServiceJsonConsts.OCTETS,
						IpcDataUnitWrapper.getIpcStructUint64Value(totalStruct,
								VtnServiceIpcConsts.BYTES));
			}
		}

		statisticsFFJson.add(VtnServiceJsonConsts.SOFTWARE, softwareFFJson);
		statisticsFFJson.add(VtnServiceIpcConsts.EXISTINGFLOW, existFFJson);
		statisticsFFJson.add(VtnServiceIpcConsts.EXPIREDFLOW, expireFFJson);
		statisticsFFJson.add(VtnServiceJsonConsts.TOTAL, totalFFJson);
		targetJson.add(VtnServiceJsonConsts.STATISTICS, statisticsFFJson);

		LOG.debug("statics Json for policy-entry: " + targetJson.toString());
		LOG.trace("complete getPolicyEntryPomStats");
	}


	/**
	 * Used for Show vBridge expand response
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return JsonObject
	 */
	public JsonObject getVbridgeExpandResourceResponse(
			final IpcDataUnit[] responsePacket, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start getVbridgeExpandResourceResponse");
		final JsonObject root = new JsonObject();
		final String rootJsonName = VtnServiceJsonConsts.EXPANDING;
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject expanding = new JsonObject();

		int index = 0;

		// skip key Type
		index++;

		// skip key structure
		index++;

		JsonArray vbridges = new JsonArray();
		JsonArray vtunnels = new JsonArray();
		JsonArray vlinks = new JsonArray();

		// vbridges
		index = getVbrExpands(responsePacket, index, vbridges);

		index++;
		// vtunnels
		index = getVtunnelExpands(responsePacket, index, vtunnels);

		// vlinks
		index++;
		index = getVlinkExpands(responsePacket, index, vlinks);

		if (vbridges.size() > 0) {
			expanding.add(VtnServiceJsonConsts.VBRIDGES, vbridges);
		}

		if (vtunnels.size() > 0) {
			expanding.add(VtnServiceJsonConsts.VTUNNELS, vtunnels);
		}

		if (vlinks.size() > 0) {
			expanding.add(VtnServiceJsonConsts.VLINKS, vlinks);
		}

		root.add(rootJsonName, expanding);

		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVbridgeExpandResourceResponse");
		return root;
	}


	/**
	 * Get vbr port map expand from IPC
	 * 
	 * @param responsePacket
	 * @param index
	 * @param vbrExpands
	 * @return int
	 */
	private int getVbrExpands(final IpcDataUnit[] responsePacket,
			int index, JsonArray vbrExpands) {
		JsonObject vbfExpand = null;
		IpcStruct valVbrExpandStruct = null;
		byte validBit = (byte)UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal();
		long size = ((IpcUint32) (responsePacket[index])).longValue();

		for (int i = 0; i < size; i++) {
			vbfExpand = new JsonObject();
			index++;
			valVbrExpandStruct = (IpcStruct)responsePacket[index];
			
			// get vbridge_name
			validBit = valVbrExpandStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValVbrExpandIndex.UPLL_IDX_VBRIDGE_NAME_VBRE
							.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, vbfExpand,
						VtnServiceJsonConsts.VBRIDGENAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valVbrExpandStruct,
								VtnServiceIpcConsts.VBRIDGE_NAME));
			}
			
			// get controller_id
			validBit = valVbrExpandStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValVbrExpandIndex.UPLL_IDX_CONTROLLER_ID_VBRE
							.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, vbfExpand,
						VtnServiceJsonConsts.CONTROLLERID,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valVbrExpandStruct,
								VtnServiceIpcConsts.CONTROLLER_ID));
			}

			// get domain_id
			validBit = valVbrExpandStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValVbrExpandIndex.UPLL_IDX_DOMAIN_ID_VBRE
							.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, vbfExpand,
						VtnServiceJsonConsts.DOMAINID,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valVbrExpandStruct,
								VtnServiceIpcConsts.DOMAINID));
			}

			// get label
			validBit = valVbrExpandStruct.getByte(
					VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValVbrExpandIndex.UPLL_IDX_LABEL_VBRE
							.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, vbfExpand,
						VtnServiceJsonConsts.LABEL,
						IpcDataUnitWrapper.getIpcStructUint32Value(
								valVbrExpandStruct,
								VtnServiceIpcConsts.LABEL));
			}

			// get controller_vtn_name
			validBit = valVbrExpandStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValVbrExpandIndex.UPLL_IDX_CONTROLLER_VTN_NAME_VBRE
							.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, vbfExpand,
						VtnServiceJsonConsts.CONTROLLER_VTN_NAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valVbrExpandStruct,
								VtnServiceIpcConsts.CONTROLLER_VTN_NAME));
			}

			// get controller_vtn_label
			validBit = valVbrExpandStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValVbrExpandIndex.UPLL_IDX_CONTROLLER_VTN_LABEL_VBRE
							.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
				setValueToJsonObject(validBit, vbfExpand,
						VtnServiceJsonConsts.CONTROLLER_VTN_LABEL,
						IpcDataUnitWrapper.getIpcStructUint32Value(
								valVbrExpandStruct,
								VtnServiceIpcConsts.CONTROLLER_VTN_LABEL));
			}

			// portmaps
			index++;
			JsonArray vbrPortmapExpands = new JsonArray();
			index = getVbrPortmapExpands(responsePacket, index, vbrPortmapExpands);

			// interfaces
			index++;
			JsonArray vbrIfExpands =  new JsonArray();
			index = getVbrIfExpands(responsePacket, index, vbrIfExpands);

			if (vbrIfExpands.size() > 0) {
				vbfExpand.add(VtnServiceJsonConsts.INTERFACES, vbrIfExpands);
			}

			if (vbrPortmapExpands.size() > 0) {
				vbfExpand.add(VtnServiceJsonConsts.PORTMAPS, vbrPortmapExpands);
			}

			vbrExpands.add(vbfExpand);
		}

		return index;
	}
	
	/**
	 * Get vbr port map expand from IPC
	 * 
	 * @param responsePacket
	 * @param index
	 * @param vbrPortmapExpands
	 * @return int
	 */
	private int getVbrPortmapExpands(final IpcDataUnit[] responsePacket,
			int index, JsonArray vbrPortmapExpands) {
		JsonObject vbfPortmapExpand = null;
		IpcStruct valVbrPortmapExpandStruct = null;
		byte validBit = (byte)UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal();
		long size = ((IpcUint32) (responsePacket[index])).longValue();

		for (int i = 0; i < size; i++) {
			vbfPortmapExpand = new JsonObject();
			index++;
			valVbrPortmapExpandStruct = (IpcStruct)responsePacket[index];

			// get portmap_id
			validBit = valVbrPortmapExpandStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValVbrPortmapExpandIndex.UPLL_IDX_PORTMAP_ID_VBRPME.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
				setValueToJsonObject(validBit, vbfPortmapExpand,
						VtnServiceJsonConsts.PORTMAP_NAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valVbrPortmapExpandStruct,
								VtnServiceIpcConsts.PORTMAP_ID));
			}

			// get logical_port_id
			validBit = valVbrPortmapExpandStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValVbrPortmapExpandIndex.UPLL_IDX_LOGICAL_PORT_ID_VBRPME.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
				setValueToJsonObject(validBit, vbfPortmapExpand,
						VtnServiceJsonConsts.LOGICAL_PORT_ID,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valVbrPortmapExpandStruct,
								VtnServiceIpcConsts.LOGICAL_PORT_ID));
			}

			// get label_type
			validBit = valVbrPortmapExpandStruct.getByte(
					VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValVbrPortmapExpandIndex.UPLL_IDX_LABEL_TYPE_VBRPME.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
				if (ValLabelType.UPLL_LABEL_TYPE_VLAN.getValue().equals(
						IpcDataUnitWrapper.getIpcStructUint8Value(
								valVbrPortmapExpandStruct,
								VtnServiceIpcConsts.LABEL_TYPE))) {
					setValueToJsonObject(validBit, vbfPortmapExpand,
							VtnServiceJsonConsts.LABEL_TYPE,
							VtnServiceJsonConsts.VLAN_ID);

					// get label
					validBit = valVbrPortmapExpandStruct.getByte(
							VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVbrPortmapExpandIndex.UPLL_IDX_LABEL_VBRPME
									.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						if (VtnServiceJsonConsts.VAL_FFFF == Long.parseLong(IpcDataUnitWrapper
								.getIpcStructUint32Value(valVbrPortmapExpandStruct,
										VtnServiceIpcConsts.LABEL))) {
							setValueToJsonObject(validBit, vbfPortmapExpand,
									VtnServiceJsonConsts.LABEL,
									VtnServiceJsonConsts.NO_VLAN_ID);
						} else if (VtnServiceJsonConsts.VAL_FFFE == Long.parseLong(IpcDataUnitWrapper
								.getIpcStructUint32Value(valVbrPortmapExpandStruct,
										VtnServiceIpcConsts.LABEL))) {
							setValueToJsonObject(validBit, vbfPortmapExpand,
									VtnServiceJsonConsts.LABEL,
									VtnServiceJsonConsts.ANY_VLAN_ID);
						} else {
							setValueToJsonObject(validBit, vbfPortmapExpand,
									VtnServiceJsonConsts.LABEL,
									IpcDataUnitWrapper.getIpcStructUint32Value(
											valVbrPortmapExpandStruct,
											VtnServiceJsonConsts.LABEL));
						}
					}
				}
			}

			vbrPortmapExpands.add(vbfPortmapExpand);
		}

		return index;
	}


	/**
	 * Get vbr interface expand from IPC
	 * 
	 * @param responsePacket
	 * @param index
	 * @param vbrIfExpands
	 * @return int
	 */
	private int getVbrIfExpands(final IpcDataUnit[] responsePacket,
			int index, JsonArray vbrIfExpands) {
		JsonObject vbfIfExpand = null;
		IpcStruct valVbrIfExpandStruct = null;
		byte validBit = (byte)UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal();
		long size = ((IpcUint32) (responsePacket[index])).longValue();

		for (int i = 0; i < size; i++) {
			vbfIfExpand = new JsonObject();
			index++;
			valVbrIfExpandStruct = (IpcStruct)responsePacket[index];
			
			// get if_name
			validBit = valVbrIfExpandStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValVbrIfExpandIndex.UPLL_IDX_IF_NAME_VBRIE.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
				setValueToJsonObject(validBit, vbfIfExpand,
						VtnServiceJsonConsts.IFNAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valVbrIfExpandStruct,
								VtnServiceIpcConsts.IFNAME));
			}

			// get connected_vnode_name
			validBit = valVbrIfExpandStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValVbrIfExpandIndex.UPLL_IDX_CONN_VNODE_NAME_VBRIE.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
				setValueToJsonObject(validBit, vbfIfExpand,
						VtnServiceJsonConsts.CONNECTED_VNODE_NAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valVbrIfExpandStruct,
								VtnServiceIpcConsts.CONNECTED_VNODE_NAME));
			}

			// get connected_if_name
			validBit = valVbrIfExpandStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValVbrIfExpandIndex.UPLL_IDX_CONN_VNODE_IF_NAME_VBRIE.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
				setValueToJsonObject(validBit, vbfIfExpand,
						VtnServiceJsonConsts.CONNECTED_IF_NAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valVbrIfExpandStruct,
								VtnServiceIpcConsts.CONNECTED_IF_NAME));
			}

			// get connected_vlink_name
			validBit = valVbrIfExpandStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValVbrIfExpandIndex.UPLL_IDX_CONN_VLINK_NAME_VBRIE.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
				setValueToJsonObject(validBit, vbfIfExpand,
						VtnServiceJsonConsts.CONNECTED_VLK_NAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valVbrIfExpandStruct,
								VtnServiceIpcConsts.CONNECTED_VLINK_NAME));
			}

			vbrIfExpands.add(vbfIfExpand);
		}
		
		return index;
	}

	/**
	 * Get vtnunel expand from IPC
	 * 
	 * @param responsePacket
	 * @param index
	 * @param vtunnelExpands
	 * @return int
	 */
	private int getVtunnelExpands(final IpcDataUnit[] responsePacket,
			int index, JsonArray vtunnelExpands) {
		JsonObject vtunnelExpand = null;
		IpcStruct valvtunnelExpandStruct = null;
		byte validBit = (byte)UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal();
		long size = ((IpcUint32) (responsePacket[index])).longValue();
		
		for (int i = 0; i < size; i++) {
			vtunnelExpand = new JsonObject();
			index++;
			valvtunnelExpandStruct = (IpcStruct)responsePacket[index];

			// get vtunnel_name
			validBit = valvtunnelExpandStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValVtunnelExpandIndex.UPLL_IDX_VTUNNEL_NAME_VTNLE.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
				setValueToJsonObject(validBit, vtunnelExpand,
						VtnServiceJsonConsts.VTUNNELNAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valvtunnelExpandStruct,
								VtnServiceIpcConsts.VTUNNELNAME));
			}
			
			// get controller_id
			validBit = valvtunnelExpandStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValVtunnelExpandIndex.UPLL_IDX_CONTROLLER_ID_VTNLE.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
				setValueToJsonObject(validBit, vtunnelExpand,
						VtnServiceJsonConsts.CONTROLLERID,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valvtunnelExpandStruct,
								VtnServiceIpcConsts.CONTROLLER_ID));
			}

			// get domain_id
			validBit = valvtunnelExpandStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValVtunnelExpandIndex.UPLL_IDX_DOMAIN_ID_VTNLE.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
				setValueToJsonObject(validBit, vtunnelExpand,
						VtnServiceJsonConsts.DOMAINID,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valvtunnelExpandStruct,
								VtnServiceIpcConsts.DOMAINID));
			}

			// get label
			validBit = valvtunnelExpandStruct.getByte(
					VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValVtunnelExpandIndex.UPLL_IDX_LABEL_VTNLE.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
				setValueToJsonObject(validBit, vtunnelExpand,
						VtnServiceJsonConsts.LABEL,
						IpcDataUnitWrapper.getIpcStructUint32Value(
								valvtunnelExpandStruct,
								VtnServiceIpcConsts.LABEL));
			}

			index++;
			JsonArray vtunnelIfExpands = new JsonArray();
			index = getVtunnelIfExpands(responsePacket, index, vtunnelIfExpands);
			if (vtunnelIfExpands.size() > 0) {
				vtunnelExpand.add(VtnServiceJsonConsts.INTERFACES, vtunnelIfExpands);
			}

			vtunnelExpands.add(vtunnelExpand);
		}

		return index;
	}

	/**
	 * Get vtnunel interface expand from IPC
	 * 
	 * @param responsePacket
	 * @param index
	 * @param vtunnelIfExpands
	 * @return int
	 */
	private int getVtunnelIfExpands(final IpcDataUnit[] responsePacket,
			int index, JsonArray vtunnelIfExpands) {
		JsonObject vtunnelIfExpand = null;
		IpcStruct valvtunnelIfExpandStruct = null;
		byte validBit = (byte)UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal();
		long size = ((IpcUint32) (responsePacket[index])).longValue();
		
		for (int i = 0; i < size; i++) {
			vtunnelIfExpand = new JsonObject();
			index++;
			valvtunnelIfExpandStruct = (IpcStruct)responsePacket[index];

			// get if_name
			validBit = valvtunnelIfExpandStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValVtunnelIfExpandIndex.UPLL_IDX_IF_NAME_VTNLIE.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
				setValueToJsonObject(validBit, vtunnelIfExpand,
						VtnServiceJsonConsts.IFNAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valvtunnelIfExpandStruct,
								VtnServiceIpcConsts.IFNAME));
			}

			// get connected_vnode_name
			validBit = valvtunnelIfExpandStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValVtunnelIfExpandIndex.UPLL_IDX_CONN_VNODE_NAME_VTNLIE.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
				setValueToJsonObject(validBit, vtunnelIfExpand,
						VtnServiceJsonConsts.CONNECTED_VNODE_NAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valvtunnelIfExpandStruct,
								VtnServiceIpcConsts.CONNECTED_VNODE_NAME));
			}

			// get connected_if_name
			validBit = valvtunnelIfExpandStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValVtunnelIfExpandIndex.UPLL_IDX_CONN_VNODE_IF_NAME_VTNLIE.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
				setValueToJsonObject(validBit, vtunnelIfExpand,
						VtnServiceJsonConsts.CONNECTED_IF_NAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valvtunnelIfExpandStruct,
								VtnServiceIpcConsts.CONNECTED_IF_NAME));
			}

			// get connected_vlink_name
			validBit = valvtunnelIfExpandStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValVtunnelIfExpandIndex.UPLL_IDX_CONN_VLINK_NAME_VTNLIE.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
				setValueToJsonObject(validBit, vtunnelIfExpand,
						VtnServiceJsonConsts.CONNECTED_VLK_NAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valvtunnelIfExpandStruct,
								VtnServiceIpcConsts.CONNECTED_VLINK_NAME));
			}

			vtunnelIfExpands.add(vtunnelIfExpand);
		}
		
		return index;
	}

	/**
	 * Get vlink expand from IPC
	 * 
	 * @param responsePacket
	 * @param index
	 * @param vlinkExpands
	 * @return int
	 */
	private int getVlinkExpands(final IpcDataUnit[] responsePacket,
			int index, JsonArray vlinkExpands) {
		JsonObject vlinkExpand = null;
		IpcStruct valvlinkExpandStruct = null;
		byte validBit = (byte)UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal();
		long size = ((IpcUint32) (responsePacket[index])).longValue();

		for (int i = 0; i < size; i++) {
			vlinkExpand = new JsonObject();
			index++;
			valvlinkExpandStruct = (IpcStruct)responsePacket[index];

			// get vnode1_name
			validBit = valvlinkExpandStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValVlinkExpandIndex.UPLL_IDX_VNODE1_NAME_VLNKE.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
				setValueToJsonObject(validBit, vlinkExpand,
						VtnServiceJsonConsts.VNODE1NAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valvlinkExpandStruct,
								VtnServiceIpcConsts.VNODE1_NAME));
			}

			// get vnode1_ifname
			validBit = valvlinkExpandStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValVlinkExpandIndex.UPLL_IDX_VNODE1_IF_NAME_VLNKE.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
				setValueToJsonObject(validBit, vlinkExpand,
						VtnServiceJsonConsts.IF1NAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valvlinkExpandStruct,
								VtnServiceIpcConsts.VNODE1_IFNAME));
			}

			// get vnode2_name
			validBit = valvlinkExpandStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValVlinkExpandIndex.UPLL_IDX_VNODE2_NAME_VLNKE.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
				setValueToJsonObject(validBit, vlinkExpand,
						VtnServiceJsonConsts.VNODE2NAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valvlinkExpandStruct,
								VtnServiceIpcConsts.VNODE2_NAME));
			}

			// get vnode2_ifname
			validBit = valvlinkExpandStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValVlinkExpandIndex.UPLL_IDX_VNODE2_IF_NAME_VLNKE.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
				setValueToJsonObject(validBit, vlinkExpand,
						VtnServiceJsonConsts.IF2NAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valvlinkExpandStruct,
								VtnServiceIpcConsts.VNODE2_IFNAME));
			}

			// get vlink_name
			validBit = valvlinkExpandStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValVlinkExpandIndex.UPLL_IDX_VLINK_NAME_VLNKE.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
				setValueToJsonObject(validBit, vlinkExpand,
						VtnServiceJsonConsts.VLKNAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valvlinkExpandStruct,
								VtnServiceIpcConsts.VLINK_NAME));
			}

			// get boundary_name
			validBit = valvlinkExpandStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValVlinkExpandIndex.UPLL_IDX_BOUNDARY_NAME_VLNKE.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
				setValueToJsonObject(validBit, vlinkExpand,
						VtnServiceJsonConsts.BOUNDARYID,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valvlinkExpandStruct,
								VtnServiceIpcConsts.BOUNDARY_NAME));
			}

			// get label_type
			validBit = valvlinkExpandStruct.getByte(VtnServiceIpcConsts.VALID,
					UncStructIndexEnum.ValVlinkExpandIndex
							.UPLL_IDX_LABEL_TYPE_VLINKE.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				if (ValLabelType.UPLL_LABEL_TYPE_VLAN.getValue().equals(
						IpcDataUnitWrapper.getIpcStructUint8Value(
								valvlinkExpandStruct,
								VtnServiceIpcConsts.LABEL_TYPE))) {
					setValueToJsonObject(validBit, vlinkExpand,
							VtnServiceJsonConsts.LABEL_TYPE,
							VtnServiceJsonConsts.VLAN_ID);

					// get label
					validBit = valvlinkExpandStruct.getByte(
							VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVlinkExpandIndex.UPLL_IDX_LABEL_VLINKE.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
						setValueToJsonObject(validBit, vlinkExpand,
								VtnServiceJsonConsts.LABEL,
								IpcDataUnitWrapper.getIpcStructUint32Value(
										valvlinkExpandStruct,VtnServiceJsonConsts.LABEL));
					}
				}
			}

			vlinkExpands.add(vlinkExpand);
		}
		return index;
	}

	/**
	 * Function to create Label Response (Show / List)
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return
	 */
	public JsonObject getLabelResponse(
			final IpcDataUnit[] responsePacket, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start getLabelResponse");
		final JsonObject root = new JsonObject();
		JsonArray labelList = null;
		LOG.debug("getType: " + getType);
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}

		String rootJsonName = null;
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.LABEL;
		} else {
			rootJsonName = VtnServiceJsonConsts.LABELS;
			// json array will be required for list type of cases
			labelList = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject label = null;
		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			label = new JsonObject();
			label.addProperty(VtnServiceJsonConsts.COUNT,
							  IpcDataUnitWrapper.getIpcDataUnitValue(
									  responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, label);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {
				label = new JsonObject();
				byte validBit;
				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;
				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyLabelStruct = (IpcStruct) responsePacket[index++];
				if (!getType.equals(VtnServiceJsonConsts.SHOW)) {
					label.addProperty(VtnServiceJsonConsts.LABEL_NAME,
							IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
									keyLabelStruct,
									VtnServiceIpcConsts.UNW_LABEL_ID));
				}

				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */
				if (getType.equals(VtnServiceJsonConsts.LIST)
						&& opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)
						|| getType.equals(VtnServiceJsonConsts.SHOW)) {
					LOG.debug("Case : Show or detail");
					/*
					 * add valid informations from value structure
					 */
					final IpcStruct valLabelStruct = (IpcStruct) responsePacket[index++];
					validBit = valLabelStruct.getByte(
							VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValUnwLabelIndex.UPLL_IDX_MAX_COUNT_UNWL
									.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, label,
								VtnServiceJsonConsts.MAX_COUNT,
								IpcDataUnitWrapper.getIpcStructUint32Value(
										valLabelStruct,
										VtnServiceIpcConsts.MAX_COUNT));
					}
					validBit = valLabelStruct.getByte(
							VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValUnwLabelIndex.UPLL_IDX_RAISING_THRESHOLD_UNWL
									.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, label,
								VtnServiceJsonConsts.RISING_THRESHOLD,
								IpcDataUnitWrapper.getIpcStructUint32Value(
										valLabelStruct,
										VtnServiceIpcConsts.RAISING_THRESHOLD));
					}
					validBit = valLabelStruct.getByte(
							VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValUnwLabelIndex.UPLL_IDX_FALLING_THRESHOLD_UNWL
									.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, label,
								VtnServiceJsonConsts.FALLING_THRESHOLD,
								IpcDataUnitWrapper.getIpcStructUint32Value(
										valLabelStruct,
										VtnServiceIpcConsts.FALLING_THRESHOLD));
					}
				} else {
					// There is no use of val structure
					LOG.debug("Skip val structure: no use");
					index++;
				}

				// add current json object to array, if it has been
				// initialized earlier
				if (null != labelList) {
					labelList.add(label);
				}
			}

			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != labelList) {
				root.add(rootJsonName, labelList);
			} else {
				root.add(rootJsonName, label);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getLabelResponse");

		return root;
	}

	/**
	 * Function to create Label Range Response (Show / List)
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return
	 */
	public JsonObject getLabelRangeResponse(
			final IpcDataUnit[] responsePacket, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start getLabelRangeResponse");
		final JsonObject root = new JsonObject();
		JsonArray labelRangeList = null;
		LOG.debug("getType: " + getType);
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}

		String rootJsonName = null;
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.RANGE;
		} else {
			rootJsonName = VtnServiceJsonConsts.RANGES;
			// json array will be required for list type of cases
			labelRangeList = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject labelRange = null;
		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			labelRange = new JsonObject();
			labelRange.addProperty(VtnServiceJsonConsts.COUNT,
					IpcDataUnitWrapper.getIpcDataUnitValue(
							responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, labelRange);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {
				labelRange = new JsonObject();
				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;
				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyVbrInterfaceStruct = (IpcStruct) responsePacket[index++];
				String min = IpcDataUnitWrapper.getIpcStructUint32Value(
						keyVbrInterfaceStruct,
						VtnServiceIpcConsts.RANGE_MIN);
				String max = IpcDataUnitWrapper.getIpcStructUint32Value(
						keyVbrInterfaceStruct,
						VtnServiceIpcConsts.RANGE_MAX);

				labelRange.addProperty(VtnServiceJsonConsts.RANGE_ID,
						min + VtnServiceConsts.UNDERSCORE + max);

				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */
				if (getType.equals(VtnServiceJsonConsts.LIST)
						&& opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)
						|| getType.equals(VtnServiceJsonConsts.SHOW)) {
					labelRange.addProperty(VtnServiceJsonConsts.RANGE_MIN, min);
					labelRange.addProperty(VtnServiceJsonConsts.RANGE_MAX, max);
				}

				// There is no use of val structure
				LOG.debug("Skip val structure: no use");
				index++;

				// add current json object to array, if it has been
				// initialized earlier
				if (null != labelRangeList) {
					labelRangeList.add(labelRange);
				}
			}

			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != labelRangeList) {
				root.add(rootJsonName, labelRangeList);
			} else {
				root.add(rootJsonName, labelRange);
			}
		}

		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getLabelRangeResponse");

		return root;
	}

	/**
	 * Gets the VTN Unified Network response.
	 * 
	 * @param responsePacket
	 *            the response packet
	 * @param requestBody
	 *            the request body
	 * @param getType
	 *            the get type
	 * @return the VTN Unified Network response
	 */
	public JsonObject getVTNUnifiedNetworkResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody, final String getType) {
		LOG.trace("Start getVTNUnifiedNetworkResponse");
		final JsonObject root = new JsonObject();
		JsonArray vtnUnifiedNwsArray = null;

		/*
		 * operation type will be required to resolve the response type
		 */
		LOG.debug("getType: " + getType);
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}

		String rootJsonName;
		/*
		 * get type (show or list) will be required to resolve root json name
		 * here it will be VTN Unified Network for show and VTN Unified Networks for list
		 */
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.UNIFIED_NW;
		} else {
			rootJsonName = VtnServiceJsonConsts.UNIFIED_NWS;
			// json array will be required for list type of cases
			vtnUnifiedNwsArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject vtnUnifiedNw = null;
		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			vtnUnifiedNw = new JsonObject();
			vtnUnifiedNw.addProperty(VtnServiceJsonConsts.COUNT,
					IpcDataUnitWrapper.getIpcDataUnitValue(
						responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, vtnUnifiedNw);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {

				vtnUnifiedNw = new JsonObject();
				byte validBit;
				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;
				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyVrtStruct = (IpcStruct) responsePacket[index++];
				vtnUnifiedNw.addProperty(VtnServiceJsonConsts.UNIFIED_NETWORK_NAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								keyVrtStruct, VtnServiceIpcConsts.UNIFIED_NW_ID));
				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */
				if (getType.equals(VtnServiceJsonConsts.SHOW) || opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
					LOG.debug("Case : Show or detail");
					/*
					 * add valid informations from value structure
					 */
					final IpcStruct valVrtStruct = (IpcStruct) responsePacket[index++];
					validBit = valVrtStruct.getByte(VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVtnUnifiedIndex.UPLL_IDX_SPINE_ID_VUNW.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
						setValueToJsonObject(validBit, vtnUnifiedNw,
								VtnServiceJsonConsts.SPINE_DOMAIN_NAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVrtStruct,
										VtnServiceIpcConsts.SPINE_ID));
					}
				} else {
					LOG.debug("Operation : normal Skip value struture");
					index++;
				}
				// add current json object to array, if it has been initialized
				// earlier
				if (null != vtnUnifiedNwsArray) {
					vtnUnifiedNwsArray.add(vtnUnifiedNw);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != vtnUnifiedNwsArray) {
				root.add(rootJsonName, vtnUnifiedNwsArray);
			} else {
				root.add(rootJsonName, vtnUnifiedNw);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVTNUnifiedNetworkResponse");
		return root;
	}

	/**
	 * Gets the Spine Domain response.
	 * 
	 * @param responsePacket
	 *            the response packet
	 * @param requestBody
	 *            the request body
	 * @param getType
	 *            the get type
	 * @return the Spine Domain response
	 */
	public JsonObject getSpineDomainResponse(
			final IpcDataUnit[] responsePacket, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start getSpineDomainResponse");
		final JsonObject root = new JsonObject();
		JsonArray spineDomainArray = null;
		JsonArray assigendLabelsJsonArray = null;
		long usedCount = 0;
		LOG.debug("getType: " + getType);
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}
		/*
		 * data type will be required to resolve the response structures
		 */
		String dataType = VtnServiceJsonConsts.STATE;
		if (requestBody.has(VtnServiceJsonConsts.TARGETDB)) {
			dataType = requestBody.get(VtnServiceJsonConsts.TARGETDB).getAsString();
		}
		String rootJsonName;
		/*
		 * get type (show or list) will be required to resolve root json name
		 * here it will be Spine Domain for show and Spine Domain for list
		 */
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.SPINE_DOMAIN;
		} else {
			rootJsonName = VtnServiceJsonConsts.SPINE_DOMAINS;
			// json array will be required for list type of cases
			spineDomainArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject spineDomain = null;

		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			spineDomain = new JsonObject();
			spineDomain.addProperty(VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, spineDomain);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {

				spineDomain = new JsonObject();
				byte validBit;

				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;
				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keySpineDomainStruct = (IpcStruct) responsePacket[index++];
				spineDomain.addProperty(VtnServiceJsonConsts.SPINE_DOMAIN_NAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										keySpineDomainStruct,
										VtnServiceIpcConsts.UNW_SPINE_ID));

				if ((getType.equals(VtnServiceJsonConsts.LIST)
						&& opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL))
						|| getType.equals(VtnServiceJsonConsts.SHOW)) {
					LOG.debug("Case : SHOW or List with DETAIL");
					/*
					 * add valid informations from value structure
					 */
					final IpcStruct valSpineDomainStruct = (IpcStruct) responsePacket[index++];

					validBit = valSpineDomainStruct.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValUnwSpineDomainIndex.UPLL_IDX_SPINE_CONTROLLER_ID_UNWS.ordinal());

					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
						setValueToJsonObject(validBit, spineDomain,VtnServiceJsonConsts.CONTROLLERID,
										IpcDataUnitWrapper.getIpcStructUint8ArrayValue(valSpineDomainStruct,
																	VtnServiceIpcConsts.SPINE_CONTROLLER_ID));
					}
					validBit = valSpineDomainStruct.getByte(
								VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValUnwSpineDomainIndex.UPLL_IDX_SPINE_DOMAIN_ID_UNWS.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
						setValueToJsonObject(validBit, spineDomain,
								VtnServiceJsonConsts.DOMAINID,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valSpineDomainStruct,
										VtnServiceIpcConsts.SPINE_DOMAIN_ID));
					}
					validBit = valSpineDomainStruct.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValUnwSpineDomainIndex.UPLL_IDX_UNW_LABEL_ID_UNWS.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
						setValueToJsonObject(validBit, spineDomain,
								VtnServiceJsonConsts.LABEL_NAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valSpineDomainStruct,
										VtnServiceIpcConsts.UNW_LABEL_ID));
					}
				} else {
					LOG.debug("List normal Skip value strutures ");
					index++;
				}

				if (getType.equals(VtnServiceJsonConsts.LIST)
						&& dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)
						&& opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)
						|| getType.equals(VtnServiceJsonConsts.SHOW)
						&& dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
					LOG.debug("Case : Show with DT_STATE or List with DT_STATE");

					/*
					 * If data type is set as "state", then value structure will
					 * also contain the state information
					 */
					final IpcStruct valSpineDomainStStruct = (IpcStruct) responsePacket[index++];
					validBit = valSpineDomainStStruct.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValUnwSpineDomainStIndex.UPLL_IDX_MAX_COUNT_UNWS_ST.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
						setValueToJsonObject(validBit, spineDomain,
								VtnServiceJsonConsts.MAX_COUNT,
								IpcDataUnitWrapper.getIpcStructUint32Value(
										valSpineDomainStStruct,
										VtnServiceIpcConsts.MAX_COUNT));

					}

					usedCount = 0;
					validBit = valSpineDomainStStruct.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValUnwSpineDomainStIndex.UPLL_IDX_USED_COUNT_UNWS_ST.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
						setValueToJsonObject(validBit, spineDomain,
								VtnServiceJsonConsts.USED_COUNT,
								IpcDataUnitWrapper.getIpcStructUint32Value(
										valSpineDomainStStruct,
										VtnServiceIpcConsts.USED_COUNT));

						usedCount = Long.parseLong(IpcDataUnitWrapper.getIpcStructUint32Value(
										valSpineDomainStStruct,
										VtnServiceIpcConsts.USED_COUNT));
					}

					validBit = valSpineDomainStStruct.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValUnwSpineDomainStIndex.UPLL_IDX_ALARM_STATUS_UNWS_ST.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
						if (IpcDataUnitWrapper.getIpcStructUint8Value(
								valSpineDomainStStruct,
								VtnServiceIpcConsts.SPINE_DOMAIN_ALARM_STATUS).equals("0")) {

							setValueToJsonObject(validBit, spineDomain,
									VtnServiceJsonConsts.ALARM_STATUS,
									VtnServiceJsonConsts.FALSE);

						} else {
							setValueToJsonObject(validBit, spineDomain,
									VtnServiceJsonConsts.ALARM_STATUS,
									VtnServiceJsonConsts.TRUE);
						}
					}

					// assigned_labels detail added
					assigendLabelsJsonArray = new JsonArray();

					for (long i = 0; i < usedCount; i++) {
						final JsonObject assigendLabelsJson = new JsonObject();
						final IpcStruct valSpineDomainAssignedLabelStruct = (IpcStruct) responsePacket[index++];

						validBit = valSpineDomainAssignedLabelStruct.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.ValUnwSpineDomainAssignedLabelIndex.UPLL_IDX_LABEL_UNWSAL.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
							setValueToJsonObject(validBit, assigendLabelsJson,
									VtnServiceJsonConsts.LABEL,
									IpcDataUnitWrapper.getIpcStructUint32Value(
											valSpineDomainAssignedLabelStruct,
											VtnServiceIpcConsts.LABEL));
						}

						validBit = valSpineDomainAssignedLabelStruct.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.ValUnwSpineDomainAssignedLabelIndex.UPLL_IDX_VTN_ID_UNWSAL.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
							setValueToJsonObject(validBit,assigendLabelsJson,
									VtnServiceJsonConsts.VTNNAME,
									IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
													valSpineDomainAssignedLabelStruct,
													VtnServiceIpcConsts.VTN_ID));
						}

						validBit = valSpineDomainAssignedLabelStruct.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.ValUnwSpineDomainAssignedLabelIndex.UPLL_IDX_VNODE_ID_UNWSAL.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
							setValueToJsonObject(validBit,assigendLabelsJson,
									VtnServiceJsonConsts.VNODENAME,
									IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
													valSpineDomainAssignedLabelStruct,
													VtnServiceIpcConsts.VNODEID));
						}
						assigendLabelsJsonArray.add(assigendLabelsJson);
					}
					// added in spineDomain- assigendLabels MAp
					if (assigendLabelsJsonArray.size() > 0) {
						spineDomain.add(VtnServiceJsonConsts.ASSIGNED_LABELS,
								assigendLabelsJsonArray);
					}
				} else if (getType.equals(VtnServiceJsonConsts.LIST)
						&& opType.equalsIgnoreCase(VtnServiceJsonConsts.NORMAL)
						&& dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
					LOG.debug("List ,Operation : normal Skip value strutures ");
					index++;
				}
				// add current json object to array, if it has been initialized
				// earlier
				if (null != spineDomainArray) {
					spineDomainArray.add(spineDomain);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != spineDomainArray) {
				root.add(rootJsonName, spineDomainArray);
			} else {
				root.add(rootJsonName, spineDomain);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getSpineDomainResponse");

		return root;
	}

	/**
	 * Used for SpineDomainFdb response
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return JsonObject
	 */
	public JsonObject getSpineDomainFdbResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody, final String getType) {
		LOG.trace("Start getSpineDomainFdbResponse");
		final JsonObject root = new JsonObject();
		final JsonObject fdbEntry = new JsonObject();
		int index = 0;
		LOG.debug("getType: " + getType);

		final String rootJsonName = VtnServiceJsonConsts.FDBUSAGE;
		LOG.debug("Json Name :" + rootJsonName);

		// There is no use of key type, key structure 
		LOG.debug("Skip key type, key structure: no use");
		index = index + 2;

		/*
		 * add valid informations from value structure
		 * value struct  val_unw_spine_domain_fdbentry 
		 */
		final IpcStruct valUnwSpineDomainFdbEntryStruct = (IpcStruct) responsePacket[index++];
		byte validBit;
		validBit = valUnwSpineDomainFdbEntryStruct.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValUnwSpineDomainFdbEntryIndex
						.UPLL_IDX_MAX_COUNT_UNWSDF.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
			setValueToJsonObject(validBit, fdbEntry,
					VtnServiceJsonConsts.TOTAL_MAX_COUNT,
					IpcDataUnitWrapper.getIpcStructUint64Value(
					valUnwSpineDomainFdbEntryStruct, VtnServiceIpcConsts.MAX_COUNT));
		}

		validBit = valUnwSpineDomainFdbEntryStruct.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValUnwSpineDomainFdbEntryIndex
						.UPLL_IDX_MAX_SWITCH_ID_UNWSDF.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
			setValueToJsonObject(validBit, fdbEntry,
					VtnServiceJsonConsts.TOTAL_MAX_SW_ID,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
					valUnwSpineDomainFdbEntryStruct, VtnServiceIpcConsts.MAX_SW_ID));
		}

		validBit = valUnwSpineDomainFdbEntryStruct.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValUnwSpineDomainFdbEntryIndex
						.UPLL_IDX_MIN_COUNT_UNWSDF.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
			setValueToJsonObject(validBit, fdbEntry,
					VtnServiceJsonConsts.TOTAL_MIN_COUNT,
					IpcDataUnitWrapper.getIpcStructUint64Value(
					valUnwSpineDomainFdbEntryStruct, VtnServiceIpcConsts.MIN_COUNT));
		}

		validBit = valUnwSpineDomainFdbEntryStruct.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValUnwSpineDomainFdbEntryIndex
						.UPLL_IDX_MIN_SWITCH_ID_UNWSDF.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
			setValueToJsonObject(validBit, fdbEntry,
					VtnServiceJsonConsts.TOTAL_MIN_SW_ID,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
					valUnwSpineDomainFdbEntryStruct, VtnServiceIpcConsts.MIN_SW_ID));
		}

		validBit = valUnwSpineDomainFdbEntryStruct.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValUnwSpineDomainFdbEntryIndex
						.UPLL_IDX_AVG_COUNT_UNWSDF.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
			setValueToJsonObject(validBit, fdbEntry,
					VtnServiceJsonConsts.TOTAL_AVG_COUNT,
					IpcDataUnitWrapper.getIpcStructUint64Value(
					valUnwSpineDomainFdbEntryStruct, VtnServiceIpcConsts.AVG_COUNT));
		}

		validBit = valUnwSpineDomainFdbEntryStruct.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValUnwSpineDomainFdbEntryIndex
						.UPLL_IDX_NO_OF_SWITCHES_UNWSDF.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
			setValueToJsonObject(validBit, fdbEntry,
					VtnServiceJsonConsts.NUM_OF_SW,
					IpcDataUnitWrapper.getIpcStructUint32Value(
					valUnwSpineDomainFdbEntryStruct, VtnServiceIpcConsts.NUM_OF_SW));
		}

		validBit = valUnwSpineDomainFdbEntryStruct.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValUnwSpineDomainFdbEntryIndex
						.UPLL_IDX_VTN_COUNT_UNWSDF.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
			long vtnFdbCnt = Long.parseLong(IpcDataUnitWrapper.getIpcStructUint32Value(
								valUnwSpineDomainFdbEntryStruct,
								VtnServiceIpcConsts.VTN_COUNT));
			LOG.debug("Count of vtn fdbusage :" + vtnFdbCnt);
			final JsonArray vtnFdbArray = new JsonArray();
			for (long num = 0; num < vtnFdbCnt; num++) {
				final JsonObject vtnFdbObj = new JsonObject();
				//value struct val_unw_spine_domain_fdbentry_vtn
				final IpcStruct valUnwSpineDomainFdbEntryVtnStruct = (IpcStruct) responsePacket[index++];

				validBit = valUnwSpineDomainFdbEntryVtnStruct.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValUnwSpineDomainFdbEntryVtnIndex
								.UPLL_IDX_VTN_ID_UNWSDFV.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
					setValueToJsonObject(validBit, vtnFdbObj,
							VtnServiceJsonConsts.VTNNAME,
							IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valUnwSpineDomainFdbEntryVtnStruct, VtnServiceIpcConsts.VTN_ID));
				}

				validBit = valUnwSpineDomainFdbEntryVtnStruct.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValUnwSpineDomainFdbEntryVtnIndex
								.UPLL_IDX_VLAN_ID_UNWSDFV.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
					setValueToJsonObject(validBit, vtnFdbObj,
							VtnServiceJsonConsts.VLANID,
							IpcDataUnitWrapper.getIpcStructUint32Value(
							valUnwSpineDomainFdbEntryVtnStruct, VtnServiceIpcConsts.VLANID));
				}

				validBit = valUnwSpineDomainFdbEntryVtnStruct.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValUnwSpineDomainFdbEntryVtnIndex
								.UPLL_IDX_MAX_COUNT_UNWSDFV.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
					setValueToJsonObject(validBit, vtnFdbObj,
							VtnServiceJsonConsts.MAX_COUNT,
							IpcDataUnitWrapper.getIpcStructUint64Value(
							valUnwSpineDomainFdbEntryVtnStruct, VtnServiceIpcConsts.MAX_COUNT));
				}

				validBit = valUnwSpineDomainFdbEntryVtnStruct.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValUnwSpineDomainFdbEntryVtnIndex
								.UPLL_IDX_MAX_SWITCH_ID_UNWSDFV.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
					setValueToJsonObject(validBit, vtnFdbObj,
							VtnServiceJsonConsts.MAX_SW_ID,
							IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valUnwSpineDomainFdbEntryVtnStruct, VtnServiceIpcConsts.MAX_SW_ID));
				}

				validBit = valUnwSpineDomainFdbEntryVtnStruct.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValUnwSpineDomainFdbEntryVtnIndex
								.UPLL_IDX_MIN_COUNT_UNWSDFV.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
					setValueToJsonObject(validBit, vtnFdbObj,
							VtnServiceJsonConsts.MIN_COUNT,
							IpcDataUnitWrapper.getIpcStructUint64Value(
							valUnwSpineDomainFdbEntryVtnStruct, VtnServiceIpcConsts.MIN_COUNT));
				}

				validBit = valUnwSpineDomainFdbEntryVtnStruct.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValUnwSpineDomainFdbEntryVtnIndex
								.UPLL_IDX_MIN_SWITCH_ID_UNWSDFV.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
					setValueToJsonObject(validBit, vtnFdbObj,
							VtnServiceJsonConsts.MIN_SW_ID,
							IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valUnwSpineDomainFdbEntryVtnStruct, VtnServiceIpcConsts.MIN_SW_ID));
				}

				validBit = valUnwSpineDomainFdbEntryVtnStruct.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValUnwSpineDomainFdbEntryVtnIndex
								.UPLL_IDX_AVG_COUNT_UNWSDFV.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED.ordinal()) {
					setValueToJsonObject(validBit, vtnFdbObj,
							VtnServiceJsonConsts.AVG_COUNT,
							IpcDataUnitWrapper.getIpcStructUint64Value(
							valUnwSpineDomainFdbEntryVtnStruct, VtnServiceIpcConsts.AVG_COUNT));
				}

				vtnFdbArray.add(vtnFdbObj);
			}

			if (vtnFdbArray.size() > 0) {
				fdbEntry.add(VtnServiceJsonConsts.VTN_FDBUSAGES, vtnFdbArray);
			}
		}
		/*
		 * finally add array to root json object and return the same.
		 */
		root.add(rootJsonName, fdbEntry);
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getSpineDomainFdbResponse");
		return root;
	}
}
