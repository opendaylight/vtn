/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.ipc.conversion;

import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import com.google.gson.JsonPrimitive;
import org.opendaylight.vtn.core.ipc.IpcDataUnit;
import org.opendaylight.vtn.core.ipc.IpcStruct;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceIpcConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.ipc.enums.PomStatsIndex;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncStructIndexEnum;

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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
							.ordinal()) {
				if (validBit == (byte) UncStructIndexEnum.Valid.UNC_VF_VALID
						.ordinal()) {
					setValueToJsonObject(validBit, ipAddress,
							VtnServiceJsonConsts.NETMASK,
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
			l2Domain.addProperty(
					VtnServiceJsonConsts.COUNT,
					IpcDataUnitWrapper
							.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, l2Domain);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {
				l2Domain = new JsonObject();
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
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
			macEntries
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
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
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				 * add valid port_name from value structure
				 */
				validBit = valVbrMacEntrySt
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.valVbrMacEntryStIndex.UPLL_IDX_IF_NAME_VMES
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
								.ordinal()) {
					setValueToJsonObject(validBit, macEntries,
							VtnServiceJsonConsts.PORTNAME,
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
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, redirectDst,
								VtnServiceJsonConsts.IFNAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.REDIRECTPORT));
					}
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_MODIFY_DST_MAC_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
						for (; index < responsePacket.length; index++) {
							// increasing index to eliminate flow list entry
							// structures in case of show and op : normal
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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

	public JsonObject getVUnknownResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody, final String getType) {
		LOG.trace("Start getVUnknownResponse");
		final JsonObject root = new JsonObject();
		JsonArray vUnknownArray = null;
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
		 * here it will be vunknown for show and vunknowns for list
		 */
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.VUNKNOWN;
		} else {
			rootJsonName = VtnServiceJsonConsts.VUNKNOWNS;
			// json array will be required for list type of cases
			vUnknownArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject vUnknownList = null;

		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			vUnknownList = new JsonObject();
			vUnknownList
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, vUnknownList);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {

				vUnknownList = new JsonObject();
				byte validBit;

				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;

				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyFlowListStruct = (IpcStruct) responsePacket[index++];
				vUnknownList.addProperty(VtnServiceJsonConsts.VUKNAME,
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vUnknownList,
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
									.ordinal()) {
						if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valVUnknownStruct,
										VtnServiceIpcConsts.TYPE)
								.equalsIgnoreCase(
										UncStructIndexEnum.ValVunknowntype.VUNKNOWN_TYPE_BRIDGE
												.getValue())) {
							setValueToJsonObject(validBit, vUnknownList,
									VtnServiceJsonConsts.TYPE,
									VtnServiceJsonConsts.BRIDGE);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valVUnknownStruct,
										VtnServiceIpcConsts.TYPE)
								.equalsIgnoreCase(
										UncStructIndexEnum.ValVunknowntype.VUNKNOWN_TYPE_ROUTER
												.getValue())) {
							setValueToJsonObject(validBit, vUnknownList,
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vUnknownList,
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vUnknownList,
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
				if (null != vUnknownArray) {
					vUnknownArray.add(vUnknownList);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != vUnknownArray) {
				root.add(rootJsonName, vUnknownArray);
			} else {
				root.add(rootJsonName, vUnknownList);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVUnknownResponse");
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
				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */
				if (getType.equals(VtnServiceJsonConsts.LIST)
						|| getType.equals(VtnServiceJsonConsts.SHOW)
						&& !dataType
								.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
					LOG.debug("Case : List or show, targetdb : not state");
					flowFilterEntry.addProperty(VtnServiceJsonConsts.SEQNUM,
							IpcDataUnitWrapper.getIpcStructUint16Value(
									keyFlowFilterEntryStruct,
									VtnServiceIpcConsts.SEQUENCENUM));
				}
				if (getType.equals(VtnServiceJsonConsts.LIST)
						&& opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)
						|| getType.equals(VtnServiceJsonConsts.SHOW)
						&& !dataType
								.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
					LOG.debug("Case : List ,type : detail or show, targetdb : not state");
					/*
					 * add valid informations from value structure
					 */
					final IpcStruct valFlowFilterEntryStruct = (IpcStruct) responsePacket[index++];
					LOG.debug("call getValVtnFlowFilterEntry to get data from value structure ValVtnFlowFilterEntry");
					getValVtnFlowFilterEntry(flowFilterEntry,
							valFlowFilterEntryStruct);
				} else if (getType.equals(VtnServiceJsonConsts.LIST)
						&& opType.equalsIgnoreCase(VtnServiceJsonConsts.NORMAL)) {
					LOG.debug("Operation : normal Skip value struture");
					index++;
				}
				if (getType.equals(VtnServiceJsonConsts.SHOW)
						&& dataType
								.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
					LOG.debug("Case : Show and  targetdb :state");
					final IpcStruct valVtnFlowFilterControllerStruct = (IpcStruct) responsePacket[index++];
					final IpcStruct valVtnFlowFilterControllerStStruct = (IpcStruct) responsePacket[index++];
					final IpcStruct valFlowFilterEntryStruct = (IpcStruct) responsePacket[index++];
					// from controller - seqnum
					validBit = valVtnFlowFilterControllerStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterControllerIndex.UPLL_IDX_SEQ_NUM_FFC
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
									.ordinal()) {
						flowFilterEntry.addProperty(
								VtnServiceJsonConsts.SEQNUM,
								IpcDataUnitWrapper.getIpcStructUint16Value(
										valVtnFlowFilterControllerStruct,
										VtnServiceIpcConsts.SEQUENCENUM));
					}
					LOG.debug("call getValVtnFlowFilterEntry to get data from value structure ValVtnFlowFilterEntry");
					getValVtnFlowFilterEntry(flowFilterEntry,
							valFlowFilterEntryStruct);
					if (opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
						LOG.debug("op : detail");
						validBit = valVtnFlowFilterControllerStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncStructIndexEnum.ValVtnFlowfilterControllerStIndex.UPLL_IDX_NWM_STATUS_VFFCS
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
						LOG.debug("Show ,Operation : normal and target db :state Skip flowList value strutures ");
						for (; index < responsePacket.length; index++) {
							// increasing index to eliminate flow list entry
							// structures in case of show and oop : normal
						}
					}

					// root.add(subRootJsonName, statistics);
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
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, softwareFFJson,
								VtnServiceIpcConsts.OCTETS,
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
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, existFFJson,
								VtnServiceIpcConsts.OCTETS,
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
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, expireFFJson,
								VtnServiceIpcConsts.OCTETS,
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
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, totalFFJson,
								VtnServiceIpcConsts.OCTETS,
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
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
			aRPEntries
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
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
			final Long count = Long
					.parseLong(IpcDataUnitWrapper
							.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
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
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, redirectDst,
								VtnServiceJsonConsts.IFNAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.REDIRECTPORT));
					}
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_MODIFY_DST_MAC_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
								.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
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
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
						for (; index < responsePacket.length; index++) {
							// increasing index to eliminate flow list entry
							// structures in case of show and oop : normal
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
									UncStructIndexEnum.ValVtunnelIndex.UPLL_IDX_VTN_NAME_VTNL
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, boundaryMapJson,
								VtnServiceJsonConsts.BOUNDARYID,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVLinkStruct,
										VtnServiceIpcConsts.BOUNDARY_NAME));
					}

					validBit = valVLinkStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_VLAN_ID_VLNK
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
									.ordinal()) {
						if (IpcDataUnitWrapper
								.getIpcStructUint16HexaValue(valVLinkStruct,
										VtnServiceJsonConsts.VLANID)
								.equalsIgnoreCase(
										VtnServiceIpcConsts.VLAN_ID_DEFAULT_VALUE)) {
							boundaryMapJson.addProperty(
									VtnServiceJsonConsts.NO_VLAN_ID,
									VtnServiceJsonConsts.TRUE);
						} else {
							setValueToJsonObject(validBit, boundaryMapJson,
									VtnServiceJsonConsts.VLANID,
									IpcDataUnitWrapper.getIpcStructUint16Value(
											valVLinkStruct,
											VtnServiceJsonConsts.VLANID));
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
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vrtInterface,
								VtnServiceJsonConsts.NETMASK,
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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

	public JsonObject getVUnknownInterfaceResponse(
			final IpcDataUnit[] responsePacketIf, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start getVUnknownInterfaceResponse");
		final JsonObject root = new JsonObject();
		JsonArray vukInterfacesArray = null;
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
			vukInterfacesArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject vukInterface = null;

		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			vukInterface = new JsonObject();
			vukInterface
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacketIf[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, vukInterface);
		} else {

			for (int index = 0; index < responsePacketIf.length; index++) {

				vukInterface = new JsonObject();
				byte validBit;

				// There is no use of key type so skipped it.
				LOG.debug("Skip key type: no use");
				index++;

				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyVukIfStruct = (IpcStruct) responsePacketIf[index++];
				vukInterface.addProperty(VtnServiceJsonConsts.IFNAME,
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vukInterface,
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
									.ordinal()) {
						if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valVukIfStruct,
										VtnServiceIpcConsts.ADMIN_STATUS)
								.equals(UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_ENABLE
										.getValue())) {
							setValueToJsonObject(validBit, vukInterface,
									VtnServiceJsonConsts.ADMINSTATUS,
									VtnServiceJsonConsts.ENABLE);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valVukIfStruct,
										VtnServiceIpcConsts.ADMIN_STATUS)
								.equals(UncStructIndexEnum.ValAdminStatus.UPLL_ADMIN_DISABLE
										.getValue())) {
							setValueToJsonObject(validBit, vukInterface,
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
				if (null != vukInterfacesArray) {
					vukInterfacesArray.add(vukInterface);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != vukInterfacesArray) {
				root.add(rootJsonName, vukInterfacesArray);
			} else {
				root.add(rootJsonName, vukInterface);
			}
		}
		LOG.debug("response Json: " + root.toString());
		LOG.trace("Complete getVUnknownInterfaceResponse");

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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
			final JsonObject requestBody, final String getType) {
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
			final IpcStruct valVbrIfStruct = (IpcStruct) responsePacket[index++];
			validBit = valVbrIfStruct
					.getByte(VtnServiceIpcConsts.VALID,
							UncStructIndexEnum.ValVbrIfIndex.UPLL_IDX_PM_VBRI
									.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
							.ordinal()) {

				IpcStruct valPortMapStruct = IpcDataUnitWrapper
						.getInnerIpcStruct(valVbrIfStruct,
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
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				validBit = valPortMapStruct.getByte(VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_VLAN_ID_PM
								.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
								.ordinal()) {

					setValueToJsonObject(validBit, portMap,
							VtnServiceJsonConsts.VLANID,
							IpcDataUnitWrapper.getIpcStructUint16Value(
									valPortMapStruct,
									VtnServiceJsonConsts.VLANID));
					LOG.debug("Vlan Id :"
							+ IpcDataUnitWrapper.getIpcStructUint16Value(
									valPortMapStruct,
									VtnServiceJsonConsts.VLANID));

				}
				/*
				 * add valid tagged from value structure
				 */
				validBit = valPortMapStruct.getByte(VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValPortMapIndex.UPLL_IDX_TAGGED_PM
								.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				LOG.debug("In target db state case-skip val_vbr_if_st ");
				index++;

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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, redirectDst,
								VtnServiceJsonConsts.IFNAME,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valFlowFilterEntryStruct,
										VtnServiceIpcConsts.REDIRECTPORT));
					}
					validBit = valFlowFilterEntryStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValFlowfilterEntryIndex.UPLL_IDX_MODIFY_DST_MAC_FFE
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
								.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
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
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
						for (; index < responsePacket.length; index++) {
							// increasing index to eliminate flow list entry
							// structures in case of show and oop : normal
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				byte validBit;

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
				final String nwm_name = IpcDataUnitWrapper
						.getIpcStructUint8ArrayValue(keyStaticIpRouteStruct,
								VtnServiceIpcConsts.NWM_NAME);
				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */
				final IpcStruct valStaticIpRouteStruct = (IpcStruct) responsePacket[index++];

				final String staticIpRouteStr = dst_addr
						+ VtnServiceJsonConsts.HYPHEN + nextHopAddr
						+ VtnServiceJsonConsts.HYPHEN + prefixlen
						+ VtnServiceJsonConsts.HYPHEN + nwm_name;
				staticIpRoute.addProperty(VtnServiceJsonConsts.STATICIPROUTEID,
						staticIpRouteStr);
				if (getType.equals(VtnServiceJsonConsts.SHOW)
						|| opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
					LOG.debug("Case : Show or detail");
					staticIpRoute.addProperty(VtnServiceJsonConsts.IPADDR,
							dst_addr);
					staticIpRoute.addProperty(VtnServiceJsonConsts.NETMASK,
							prefixlen);
					staticIpRoute.addProperty(VtnServiceJsonConsts.NEXTHOPADDR,
							nextHopAddr);
					staticIpRoute.addProperty(VtnServiceJsonConsts.NMG_NAME,
							nwm_name);

					validBit = valStaticIpRouteStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValStaticIpRouteIndex.UPLL_IDX_GROUP_METRIC_SIR
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, staticIpRoute,
								VtnServiceJsonConsts.GROUPMETRIC,
								IpcDataUnitWrapper.getIpcStructUint16Value(
										valStaticIpRouteStruct,
										VtnServiceIpcConsts.GROUP_METRIC));
					}

				} else {
					LOG.debug("Operation : normal Skip value struture");
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
									.ordinal()) {
						setValueToJsonObject(validBit, vtep,
								VtnServiceJsonConsts.DOMAINID,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valVtepStruct,
										VtnServiceJsonConsts.DOMAINID));
					}
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
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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

					 else {
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
			} else if (IpcDataUnitWrapper
					.getIpcStructUint8Value(valFlowFilterEntryStruct,
							VtnServiceIpcConsts.ACTION)
					.equalsIgnoreCase(
							UncStructIndexEnum.FlowfilterAction.UPLL_FLOWFILTER_ACT_DROP
									.getValue())) {
				setValueToJsonObject(validBit, flowFilterEntry,
						VtnServiceJsonConsts.ACTIONTYPE,
						VtnServiceJsonConsts.DROP);
			} else if (IpcDataUnitWrapper
					.getIpcStructUint8Value(valFlowFilterEntryStruct,
							VtnServiceIpcConsts.ACTION)
					.equalsIgnoreCase(
							UncStructIndexEnum.FlowfilterAction.UPLL_FLOWFILTER_ACT_REDIRECT
									.getValue())) {
				setValueToJsonObject(validBit, flowFilterEntry,
						VtnServiceJsonConsts.ACTIONTYPE,
						VtnServiceJsonConsts.REDIRECT);
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
						.ordinal()) {
			if (UncStructIndexEnum.ValVbrIfMapType.UPLL_IF_OFS_MAP.getValue()
					.equals(IpcDataUnitWrapper.getIpcStructUint8Value(
							valVtnstationControllerSt,
							VtnServiceIpcConsts.MAP_TYPE))) {
				setValueToJsonObject(validBit, vtnStation,
						VtnServiceJsonConsts.MAPTYPE,
						VtnServiceJsonConsts.OFS_MAP);
			} else if (UncStructIndexEnum.ValVbrIfMapType.UPLL_IF_VLAN_MAP
					.getValue().equals(
							IpcDataUnitWrapper.getIpcStructUint8Value(
									valVtnstationControllerSt,
									VtnServiceIpcConsts.MAP_TYPE))) {
				setValueToJsonObject(validBit, vtnStation,
						VtnServiceJsonConsts.MAPTYPE,
						VtnServiceJsonConsts.VLAN_MAP);
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, vtnStation,
					VtnServiceJsonConsts.DOMAINID,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valVtnstationControllerSt,
							VtnServiceIpcConsts.DOMAINID));
		}

		// for vbr_name parameter
		validBit = valVtnstationControllerSt
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VBR_NAME_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, vtnStation,
					VtnServiceJsonConsts.VBRIDGENAME,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valVtnstationControllerSt,
							VtnServiceIpcConsts.VBRNAME));
		}

		final JsonObject interfaceObj = new JsonObject();

		// for if_name parameter
		validBit = valVtnstationControllerSt
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VBR_IF_NAME_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, interfaceObj,
					VtnServiceJsonConsts.IFNAME,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valVtnstationControllerSt,
							VtnServiceIpcConsts.VBRIFNAME));
		}

		// for if_status parameter
		validBit = valVtnstationControllerSt
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_VBR_IF_STATUS_VSCS
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
						.ordinal()) {
			if (UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_UP.getValue()
					.equals(IpcDataUnitWrapper.getIpcStructUint8Value(
							valVtnstationControllerSt,
							VtnServiceIpcConsts.VBRIFSTATUS))) {
				setValueToJsonObject(validBit, interfaceObj,
						VtnServiceJsonConsts.OPERSTATUS,
						VtnServiceJsonConsts.UP);
			} else if (UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_DOWN
					.getValue().equals(
							IpcDataUnitWrapper.getIpcStructUint8Value(
									valVtnstationControllerSt,
									VtnServiceIpcConsts.VBRIFSTATUS))) {
				setValueToJsonObject(validBit, interfaceObj,
						VtnServiceJsonConsts.OPERSTATUS,
						VtnServiceJsonConsts.DOWN);
			} else if (UncStructIndexEnum.ValOperStatus.UPLL_OPER_STATUS_UNKNOWN
					.getValue().equals(
							IpcDataUnitWrapper.getIpcStructUint8Value(
									valVtnstationControllerSt,
									VtnServiceIpcConsts.VBRIFSTATUS))) {
				setValueToJsonObject(validBit, interfaceObj,
						VtnServiceJsonConsts.OPERSTATUS,
						VtnServiceJsonConsts.UNKNOWN);
			} else {
				LOG.debug("Operstatus :Invalid");
			}
			LOG.debug("Operstatus :"
					+ IpcDataUnitWrapper.getIpcStructUint8Value(
							valVtnstationControllerSt,
							VtnServiceIpcConsts.VBRIFSTATUS));
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
			for (int index = 0; index < responsePacket.length; index++) {

				vtnStation = new JsonObject();
				byte validBit;

				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;

				/*
				 * add mandatory informations from key structure
				 */
				// final IpcStruct keyVtnstationController = (IpcStruct)
				// responsePacket[index++];
				LOG.debug("Skip key Structure: no use");
				index++; // no need to use key structure

				final IpcStruct valVtnstationControllerSt = (IpcStruct) responsePacket[index++];
				createVtnStationConstJson(vtnStation, valVtnstationControllerSt);

				// for ipaddrs parameter
				validBit = valVtnstationControllerSt
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_IPV4_COUNT_VSCS
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
					vtnStation.add(VtnServiceJsonConsts.IPADDRS, ipaddrsArray);
					LOG.debug("count of ipv4 address : " + ipv4_count);

				} else {
					// for ipv6addrs parameter
					validBit = valVtnstationControllerSt
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncStructIndexEnum.ValVtnstationControllerStIndex.UPLL_IDX_IPV6_COUNT_VSCS
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
									.ordinal()) {
						final int ipv6_count = Integer
								.parseInt(IpcDataUnitWrapper
										.getIpcStructUint32Value(
												valVtnstationControllerSt,
												VtnServiceIpcConsts.IPV6_COUNT));
						final JsonArray ipaddrsArray = new JsonArray();
						for (int i = 0; i < ipv6_count; i++) {
							final JsonPrimitive ipaddrs = new JsonPrimitive(
									IpcDataUnitWrapper
											.getIpcDataUnitValue(responsePacket[index++]));
							ipaddrsArray.add(ipaddrs);
						}
						vtnStation.add(VtnServiceJsonConsts.IPV6ADDRS,
								ipaddrsArray);
						LOG.debug("count of ip64 address : " + ipv6_count);
					}
				}

				if (opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
					LOG.debug("op : detail");
					final IpcStruct valVtnstationControllerStat = (IpcStruct) responsePacket[index++];

					final JsonObject vtnStationStats = new JsonObject();
					addVtnStationStatsData(vtnStationStats,
							valVtnstationControllerStat);
					vtnStation.add(VtnServiceJsonConsts.STATISTICS,
							vtnStationStats);
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
			ipRouteList
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
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
			final Long count = Long
					.parseLong(IpcDataUnitWrapper
							.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
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
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
								.ordinal()) {
					setValueToJsonObject(validBit, ipRouteList,
							VtnServiceJsonConsts.NETMASK,
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
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
								.ordinal()) {
					setValueToJsonObject(validBit, ipRouteList,
							VtnServiceJsonConsts.USE,
							IpcDataUnitWrapper.getIpcStructUint32Value(
									vaVvrtIpRouteSt, VtnServiceJsonConsts.USE));
				}
				validBit = vaVvrtIpRouteSt
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtIpRouteStIndex.UPLL_IDX_USE_VIRS
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
								.ordinal()) {
					setValueToJsonObject(validBit, ipRouteList,
							VtnServiceJsonConsts.IFNAME,
							IpcDataUnitWrapper.getIpcStructUint32Value(
									vaVvrtIpRouteSt,
									VtnServiceJsonConsts.IFNAME));
				}
				validBit = vaVvrtIpRouteSt
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVrtIpRouteStIndex.UPLL_IDX_USE_VIRS
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SOPPORTED
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
}