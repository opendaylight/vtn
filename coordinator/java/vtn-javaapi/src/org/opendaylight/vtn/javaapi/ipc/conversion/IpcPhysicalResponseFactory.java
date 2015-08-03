/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi.ipc.conversion;

import java.util.concurrent.atomic.AtomicInteger;

import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import com.google.gson.JsonPrimitive;
import org.opendaylight.vtn.core.ipc.IpcDataUnit;
import org.opendaylight.vtn.core.ipc.IpcStruct;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.core.util.UnsignedInteger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceIpcConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.init.VtnServiceInitManager;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcDataUnitWrapper;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncPhysicalStructIndexEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncStructIndexEnum;

/**
 * A factory for creating Response objects for Physical APIs.
 */
public class IpcPhysicalResponseFactory {
	private static final Logger LOG = Logger
			.getLogger(IpcPhysicalResponseFactory.class.getName());

	/**
	 * Gets the domain response.
	 * 
	 * @param responsePacket
	 *            the response packet
	 * @param requestBody
	 *            the request body
	 * @param getType
	 *            the get type
	 * @return the domain response
	 */
	public JsonObject getDomainResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody, final String getType) {
		LOG.trace("Start getDomainResponse");
		final JsonObject root = new JsonObject();
		JsonArray domainsArray = null;
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
			rootJsonName = VtnServiceJsonConsts.DOMAIN;
		} else {
			rootJsonName = VtnServiceJsonConsts.DOMAINS;
			// json array will be required for list type of cases
			domainsArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject domain = null;
		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			domain = new JsonObject();
			domain.addProperty(
					VtnServiceJsonConsts.COUNT,
					IpcDataUnitWrapper
							.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, domain);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {

				domain = new JsonObject();
				byte validBit;
				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;
				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyCtrDomainStruct = (IpcStruct) responsePacket[index++];
				domain.addProperty(VtnServiceJsonConsts.DOMAINID,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								keyCtrDomainStruct,
								VtnServiceIpcConsts.DOMAIN_NAME));
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
					IpcStruct valCtrDomainStruct = (IpcStruct) responsePacket[index++];

					if (dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
						LOG.debug("targetdb : State");
						final IpcStruct valCtrDomainStSruct = valCtrDomainStruct;
						validBit = valCtrDomainStSruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValDomainStIndex.kIdxDomainStDomain
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							// valCtrDomainStruct=
							// (IpcStruct)valCtrDomainStSruct.get(VtnServiceJsonConsts.DOMAIN);
							valCtrDomainStruct = IpcDataUnitWrapper
									.getInnerIpcStruct(valCtrDomainStSruct,
											VtnServiceJsonConsts.DOMAIN);
							LOG.debug("call getValCtrDomainStruct to get data from value structure ValCtrDomainStruct");
							getValCtrDomainStruct(domain, valCtrDomainStruct);
						}
						validBit = valCtrDomainStSruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValDomainStIndex.kIdxDomainStOperStatus
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							if (IpcDataUnitWrapper
									.getIpcStructUint8Value(
											valCtrDomainStSruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equalsIgnoreCase(
											UncPhysicalStructIndexEnum.UpplDomainOperStatus.UPPL_SWITCH_OPER_DOWN
													.getValue())) {
								setValueToJsonObject(validBit, domain,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.DOWN);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(
											valCtrDomainStSruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equalsIgnoreCase(
											UncPhysicalStructIndexEnum.UpplDomainOperStatus.UPPL_SWITCH_OPER_UP
													.getValue())) {
								setValueToJsonObject(validBit, domain,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.UP);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(
											valCtrDomainStSruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equalsIgnoreCase(
											UncPhysicalStructIndexEnum.UpplDomainOperStatus.UPPL_SWITCH_OPER_UNKNOWN
													.getValue())) {
								setValueToJsonObject(validBit, domain,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.UNKNOWN);
							} else {
								LOG.debug("Operstatus : invalid");
							}
							LOG.debug("Operstatus :"
									+ IpcDataUnitWrapper
											.getIpcStructUint8Value(
													valCtrDomainStSruct,
													VtnServiceIpcConsts.OPERSTATUS));
						}
					} else {
						getValCtrDomainStruct(domain, valCtrDomainStruct);
					}

				} else {
					LOG.debug("Operation : normal Skip value struture");
					index++;
				}
				// add current json object to array, if it has been initialized
				// earlier
				if (null != domainsArray) {
					domainsArray.add(domain);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != domainsArray) {
				root.add(rootJsonName, domainsArray);
			} else {
				root.add(rootJsonName, domain);
			}
		}
		LOG.debug("Response Json: " + root.toString());
		LOG.trace("Complete getDomainResponse");
		return root;
	}

	/**
	 * Gets the val ctr domain struct.
	 * 
	 * @param domain
	 *            the domain
	 * @param valCtrDomainStruct
	 *            the val ctr domain struct
	 * @return the val ctr domain struct
	 */
	public void getValCtrDomainStruct(final JsonObject domain,
			final IpcStruct valCtrDomainStruct) {
		LOG.trace("Start getValCtrDomainStruct");
		byte validBit;
		validBit = valCtrDomainStruct.getByte(VtnServiceIpcConsts.VALID,
				UncPhysicalStructIndexEnum.UpplValDomainIndex.kIdxDomainType
						.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			if (IpcDataUnitWrapper
					.getIpcStructUint8Value(valCtrDomainStruct,
							VtnServiceIpcConsts.TYPE)
					.equalsIgnoreCase(
							UncPhysicalStructIndexEnum.UpplDomainType.UPPL_DOMAIN_TYPE_DEFAULT
									.getValue())) {
				setValueToJsonObject(validBit, domain,
						VtnServiceJsonConsts.TYPE, VtnServiceJsonConsts.DEFAULT);
			} else if (IpcDataUnitWrapper
					.getIpcStructUint8Value(valCtrDomainStruct,
							VtnServiceIpcConsts.TYPE)
					.equalsIgnoreCase(
							UncPhysicalStructIndexEnum.UpplDomainType.UPPL_DOMAIN_TYPE_NORMAL
									.getValue())) {
				setValueToJsonObject(validBit, domain,
						VtnServiceJsonConsts.TYPE, VtnServiceJsonConsts.NORMAL);
			} else if(IpcDataUnitWrapper
					.getIpcStructUint8Value(valCtrDomainStruct,
							VtnServiceIpcConsts.TYPE)
					.equalsIgnoreCase(
							UncPhysicalStructIndexEnum.UpplDomainType.UPPL_DOMAIN_TYPE_PF_LEAF
									.getValue())) {
				setValueToJsonObject(validBit, domain,
						VtnServiceJsonConsts.TYPE, VtnServiceJsonConsts.LEAF);
			} else if (IpcDataUnitWrapper
					.getIpcStructUint8Value(valCtrDomainStruct,
							VtnServiceIpcConsts.TYPE)
					.equalsIgnoreCase(
							UncPhysicalStructIndexEnum.UpplDomainType.UPPL_DOMAIN_TYPE_PF_SPINE
									.getValue())) {
				setValueToJsonObject(validBit, domain,
						VtnServiceJsonConsts.TYPE, VtnServiceJsonConsts.SPINE);
			} else {
				LOG.debug("Type : invalid");
			}
			LOG.debug("Type :"
					+ IpcDataUnitWrapper.getIpcStructUint8Value(
							valCtrDomainStruct, VtnServiceIpcConsts.TYPE));
		}
		validBit = valCtrDomainStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.UpplValDomainIndex.kIdxDomainDescription
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, domain,
					VtnServiceJsonConsts.DESCRIPTION,
					IpcDataUnitWrapper
							.getIpcStructUint8ArrayValue(valCtrDomainStruct,
									VtnServiceIpcConsts.DESCRIPTION));
		}
		LOG.debug("ValCtrDomainStruct response Json: " + domain.toString());
		LOG.trace("Complete getValCtrDomainStruct");
	}

	/**
	 * Gets the boundary response.
	 * 
	 * @param responsePacket
	 *            the response packet
	 * @param requestBody
	 *            the request body
	 * @param getType
	 *            the get type
	 * @return the boundary response
	 */
	public JsonObject getBoundaryResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody, final String getType) {
		LOG.trace("Start getBoundaryResponse");
		final JsonObject root = new JsonObject();
		JsonArray boundaryArray = null;
		LOG.debug("getType: " + getType);
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}
		/*
		 * * data type will be required to resolve the response structures
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
			rootJsonName = VtnServiceJsonConsts.BOUNDARY;
		} else {
			rootJsonName = VtnServiceJsonConsts.BOUNDARIES;
			// json array will be required for list type of cases
			boundaryArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject boundary = null;
		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			boundary = new JsonObject();
			boundary.addProperty(
					VtnServiceJsonConsts.COUNT,
					IpcDataUnitWrapper
							.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, boundary);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {
				boundary = new JsonObject();
				byte validBit;
				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;
				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyBoundaryStruct = (IpcStruct) responsePacket[index++];
				boundary.addProperty(VtnServiceJsonConsts.BOUNDARYID,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								keyBoundaryStruct,
								VtnServiceJsonConsts.BOUNDARYID));
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
					IpcStruct valBoundaryStruct = (IpcStruct) responsePacket[index++];
					if (dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
						LOG.debug("targetdb : State");
						final IpcStruct valBoundaryStSruct = valBoundaryStruct;
						validBit = valBoundaryStSruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValBoundaryStIndex.kIdxBoundaryStBoundary
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							valBoundaryStruct = IpcDataUnitWrapper
									.getInnerIpcStruct(valBoundaryStSruct,
											VtnServiceJsonConsts.BOUNDARY);
							LOG.debug("call getValBoundaryStruct to get data from value structure ValBoundaryStruct");
							getValBoundaryStruct(boundary, valBoundaryStruct);
						}
						validBit = valBoundaryStSruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValBoundaryStIndex.kIdxBoundaryStOperStatus
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valBoundaryStSruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equalsIgnoreCase(
											UncPhysicalStructIndexEnum.UpplBoundaryOperStatus.UPPL_BOUNDARY_OPER_DOWN
													.getValue())) {
								setValueToJsonObject(validBit, boundary,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.DOWN);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valBoundaryStSruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equalsIgnoreCase(
											UncPhysicalStructIndexEnum.UpplBoundaryOperStatus.UPPL_BOUNDARY_OPER_UP
													.getValue())) {
								setValueToJsonObject(validBit, boundary,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.UP);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valBoundaryStSruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equalsIgnoreCase(
											UncPhysicalStructIndexEnum.UpplBoundaryOperStatus.UPPL_BOUNDARY_OPER_UNKNOWN
													.getValue())) {
								setValueToJsonObject(validBit, boundary,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.UNKNOWN);
							} else {
								LOG.debug("Operstatus : invalid");
							}
							LOG.debug("Operstatus :"
									+ IpcDataUnitWrapper
											.getIpcStructUint8Value(
													valBoundaryStSruct,
													VtnServiceIpcConsts.OPERSTATUS));
						}
					} else {
						getValBoundaryStruct(boundary, valBoundaryStruct);
					}
				} else {
					LOG.debug("Operation : normal Skip value struture");
					index++;
				}
				// add current json object to array, if it has been initialized
				// earlier
				if (null != boundaryArray) {
					boundaryArray.add(boundary);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != boundaryArray) {
				root.add(rootJsonName, boundaryArray);
			} else {
				root.add(rootJsonName, boundary);
			}
		}
		LOG.debug("Response Json: " + root.toString());
		LOG.trace("Complete getBoundaryResponse");
		return root;
	}

	/**
	 * Gets the val boundary struct.
	 * 
	 * @param boundary
	 *            the boundary
	 * @param valBoundaryStruct
	 *            the val boundary struct
	 * @return the val boundary struct
	 */
	public void getValBoundaryStruct(final JsonObject boundary,
			final IpcStruct valBoundaryStruct) {
		LOG.trace("Start getValBoundaryStruct");
		byte validBit;
		final String linkJsonName = VtnServiceJsonConsts.LINK;
		final JsonObject link = new JsonObject();
		validBit = valBoundaryStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryDescription
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, boundary,
					VtnServiceJsonConsts.DESCRIPTION,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valBoundaryStruct, VtnServiceIpcConsts.DESCRIPTION));
		}
		validBit = valBoundaryStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryControllerName1
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, link,
					VtnServiceJsonConsts.CONTROLLER1ID,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valBoundaryStruct,
							VtnServiceIpcConsts.CONTROLLERNAME1));
		}
		validBit = valBoundaryStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryDomainName1
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, link,
					VtnServiceJsonConsts.DOMAIN1ID,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valBoundaryStruct, VtnServiceIpcConsts.DOMAINNAME1));
		}
		validBit = valBoundaryStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryLogicalPortId1
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, link,
					VtnServiceJsonConsts.LOGICALPORT1ID,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valBoundaryStruct,
							VtnServiceIpcConsts.LOGICALPORTID1));
		}
		validBit = valBoundaryStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryControllerName2
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, link,
					VtnServiceJsonConsts.CONTROLLER2ID,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valBoundaryStruct,
							VtnServiceIpcConsts.CONTROLLERNAME2));
		}
		validBit = valBoundaryStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryDomainName2
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, link,
					VtnServiceJsonConsts.DOMAIN2ID,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valBoundaryStruct, VtnServiceIpcConsts.DOMAINNAME2));
		}
		validBit = valBoundaryStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.UpplValBoundaryIndex.kIdxBoundaryLogicalPortId2
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, link,
					VtnServiceJsonConsts.LOGICALPORT2ID,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valBoundaryStruct,
							VtnServiceIpcConsts.LOGICALPORTID2));
		}
		boundary.add(linkJsonName, link);
		LOG.debug("boundary Json: " + boundary.toString());
		LOG.trace("Complete getValBoundaryStruct");
	}

	/**
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return
	 */
	public JsonObject getControllerResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody, final String getType) {
		LOG.trace("Start getControllerResponse");
		final JsonObject root = new JsonObject();
		JsonArray ControllerArray = null;
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
		 * here it will be Controller for show and Controller for list
		 */
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.CONTROLLER;
		} else {
			rootJsonName = VtnServiceJsonConsts.CONTROLLERS;
			// json array will be required for list type of cases
			ControllerArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject controller = null;
		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			controller = new JsonObject();
			controller
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, controller);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {

				controller = new JsonObject();
				byte validBit;
				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;
				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyControllerStruct = (IpcStruct) responsePacket[index++];
				controller.addProperty(VtnServiceJsonConsts.CONTROLLERID,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								keyControllerStruct,
								VtnServiceJsonConsts.CONTROLLERNAME));
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
					IpcStruct valControllerStruct = (IpcStruct) responsePacket[index++];
					if (dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
						LOG.debug("targetdb : State");
						final IpcStruct valControllerStSruct = valControllerStruct;
						validBit = valControllerStSruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValCtrStIndex.kIdxController
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							valControllerStruct = IpcDataUnitWrapper
									.getInnerIpcStruct(valControllerStSruct,
											VtnServiceJsonConsts.CONTROLLER);
							getValControllerStruct(controller,
									valControllerStruct);
						}
						validBit = valControllerStSruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValCtrStIndex.kIdxActualVersion
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							setValueToJsonObject(
									validBit,
									controller,
									VtnServiceJsonConsts.ACTUALVERSION,
									IpcDataUnitWrapper
											.getIpcStructUint8ArrayValue(
													valControllerStSruct,
													VtnServiceJsonConsts.ACTUALVERSION));
						}
						validBit = valControllerStSruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValCtrStIndex.kIdxOperStatus
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							if (IpcDataUnitWrapper
									.getIpcStructUint8Value(
											valControllerStSruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equalsIgnoreCase(
											UncStructIndexEnum.UpplControllerOperStatus.UPPL_CONTROLLER_OPER_DOWN
													.getValue())) {
								setValueToJsonObject(validBit, controller,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.DOWN);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(
											valControllerStSruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equalsIgnoreCase(
											UncStructIndexEnum.UpplControllerOperStatus.UPPL_CONTROLLER_OPER_UP
													.getValue())) {
								setValueToJsonObject(validBit, controller,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.UP);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(
											valControllerStSruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equalsIgnoreCase(
											UncStructIndexEnum.UpplControllerOperStatus.UPPL_CONTROLLER_OPER_WAITING_AUDIT
													.getValue())) {
								setValueToJsonObject(validBit, controller,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.WAITING_AUDIT);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(
											valControllerStSruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equalsIgnoreCase(
											UncStructIndexEnum.UpplControllerOperStatus.UPPL_CONTROLLER_OPER_AUDITING
													.getValue())) {
								setValueToJsonObject(validBit, controller,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.AUDITING);
							} else {
								LOG.debug("Operstatus : invalid");
							}
							LOG.debug("Operstatus :"
									+ IpcDataUnitWrapper
											.getIpcStructUint8Value(
													valControllerStSruct,
													VtnServiceIpcConsts.OPERSTATUS));
						}
					} else {
						// added in case target db is not state
						getValControllerStruct(controller, valControllerStruct);
					}

				} else {
					LOG.debug("Operation : normal Skip value struture");
					index++;
				}
				// add current json object to array, if it has been initialized
				// earlier
				if (null != ControllerArray) {
					ControllerArray.add(controller);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != ControllerArray) {
				root.add(rootJsonName, ControllerArray);
			} else {
				root.add(rootJsonName, controller);
			}
		}
		LOG.debug("Response Json: " + root.toString());
		LOG.trace("Complete getControllerResponse");
		return root;
	}

	/**
	 * Method to get Val Ctr structure for Get API.
	 * 
	 */
	private void getValControllerStruct(final JsonObject controller,
			final IpcStruct valControllerStruct) {
		LOG.trace("Start getValControllerStruct");
		byte validBit;
		validBit = valControllerStruct.getByte(VtnServiceIpcConsts.VALID,
				UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxDescription
						.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controller,
					VtnServiceJsonConsts.DESCRIPTION,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valControllerStruct,
							VtnServiceIpcConsts.DESCRIPTION));
		}
		validBit = valControllerStruct.getByte(VtnServiceIpcConsts.VALID,
				UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxIpAddress
						.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controller,
					VtnServiceJsonConsts.IPADDR,
					IpcDataUnitWrapper
							.getIpcStructIpv4Value(valControllerStruct,
									VtnServiceIpcConsts.IP_ADDRESS));
		}

		validBit = valControllerStruct.getByte(VtnServiceIpcConsts.VALID,
				UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxType.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			if (IpcDataUnitWrapper.getIpcStructUint8Value(valControllerStruct,
					VtnServiceIpcConsts.TYPE).equalsIgnoreCase(
					UncPhysicalStructIndexEnum.UpplTypeIndex.UNC_CT_UNKNOWN
							.getValue())) {
				setValueToJsonObject(validBit, controller,
						VtnServiceJsonConsts.TYPE, VtnServiceJsonConsts.BYPASS);
			} else if (IpcDataUnitWrapper.getIpcStructUint8Value(
					valControllerStruct, VtnServiceIpcConsts.TYPE)
					.equalsIgnoreCase(
							UncPhysicalStructIndexEnum.UpplTypeIndex.UNC_CT_PFC
									.getValue())) {
				setValueToJsonObject(validBit, controller,
						VtnServiceJsonConsts.TYPE, VtnServiceJsonConsts.PFC);
			} else if (IpcDataUnitWrapper.getIpcStructUint8Value(
					valControllerStruct, VtnServiceIpcConsts.TYPE)
					.equalsIgnoreCase(
							UncPhysicalStructIndexEnum.UpplTypeIndex.UNC_CT_VNP
									.getValue())) {
				setValueToJsonObject(validBit, controller,
						VtnServiceJsonConsts.TYPE, VtnServiceJsonConsts.VNP);
			} else if (IpcDataUnitWrapper.getIpcStructUint8Value(
					valControllerStruct, VtnServiceIpcConsts.TYPE)
					.equalsIgnoreCase(
							UncPhysicalStructIndexEnum.UpplTypeIndex.UNC_CT_POLC
									.getValue())) {
				String polc = VtnServiceInitManager.getConfigurationMap()
						.getCommonConfigValue(VtnServiceConsts.CONF_FILE_FIELD_POLC);
				setValueToJsonObject(validBit, controller,
						VtnServiceJsonConsts.TYPE, polc);
			} else if (IpcDataUnitWrapper.getIpcStructUint8Value(
					valControllerStruct, VtnServiceIpcConsts.TYPE)
					.equalsIgnoreCase(
							UncPhysicalStructIndexEnum.UpplTypeIndex.UNC_CT_HPVANC
									.getValue())) {
				String hpctr = VtnServiceInitManager.getConfigurationMap()
						.getCommonConfigValue(VtnServiceConsts.CONF_FILE_FIELD_HPVANC);
				setValueToJsonObject(validBit, controller,
						VtnServiceJsonConsts.TYPE, hpctr);
				
			} else if (IpcDataUnitWrapper.getIpcStructUint8Value(
					valControllerStruct, VtnServiceIpcConsts.TYPE)
					.equalsIgnoreCase(
							UncPhysicalStructIndexEnum.UpplTypeIndex.UNC_CT_ODC
									.getValue())) {
				String odc = VtnServiceInitManager.getConfigurationMap()
						.getCommonConfigValue(VtnServiceConsts.CONF_FILE_FIELD_ODC);
				setValueToJsonObject(validBit, controller,
						VtnServiceJsonConsts.TYPE, odc);
				
			} else {
				LOG.info("Type : invalid");
			}
			LOG.debug("Type :"
					+ IpcDataUnitWrapper.getIpcStructUint8Value(
							valControllerStruct, VtnServiceIpcConsts.TYPE));
		}
		validBit = valControllerStruct.getByte(VtnServiceIpcConsts.VALID,
				UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxEnableAudit
						.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			if (IpcDataUnitWrapper
					.getIpcStructUint8Value(valControllerStruct,
							VtnServiceIpcConsts.ENABLE_AUDIT)
					.equalsIgnoreCase(
							UncPhysicalStructIndexEnum.UpplControllerAuditStatus.UPPL_AUTO_AUDIT_DISABLED
									.getValue())) {
				setValueToJsonObject(validBit, controller,
						VtnServiceJsonConsts.AUDITSTATUS,
						VtnServiceJsonConsts.DISABLE);
			} else if (IpcDataUnitWrapper
					.getIpcStructUint8Value(valControllerStruct,
							VtnServiceIpcConsts.ENABLE_AUDIT)
					.equalsIgnoreCase(
							UncPhysicalStructIndexEnum.UpplControllerAuditStatus.UPPL_AUTO_AUDIT_ENABLED
									.getValue())) {
				setValueToJsonObject(validBit, controller,
						VtnServiceJsonConsts.AUDITSTATUS,
						VtnServiceJsonConsts.ENABLE);
			} else {
				LOG.debug("Auditstatus : invalid");
			}
			LOG.debug("Auditstatus :"
					+ IpcDataUnitWrapper.getIpcStructUint8Value(
							valControllerStruct,
							VtnServiceIpcConsts.ENABLE_AUDIT));
		}
		validBit = valControllerStruct.getByte(VtnServiceIpcConsts.VALID,
				UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxUser.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controller,
					VtnServiceJsonConsts.USERNAME,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valControllerStruct, VtnServiceIpcConsts.USER));
		}
		validBit = valControllerStruct.getByte(VtnServiceIpcConsts.VALID,
				UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxPassword
						.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controller,
					VtnServiceJsonConsts.PASSWORD,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valControllerStruct, VtnServiceJsonConsts.PASSWORD));
		}
		validBit = valControllerStruct.getByte(VtnServiceIpcConsts.VALID,
				UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxVersion
						.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controller,
					VtnServiceJsonConsts.VERSION,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valControllerStruct, VtnServiceJsonConsts.VERSION));
		}
		validBit = valControllerStruct.getByte(VtnServiceIpcConsts.VALID,
				UncPhysicalStructIndexEnum.UpplValCtrIndex.kIdxPort
						.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controller,
					VtnServiceJsonConsts.PORT,
					IpcDataUnitWrapper.getIpcStructUint16Value(
							valControllerStruct, VtnServiceJsonConsts.PORT));
		}
		LOG.trace("Complete getValControllerStruct");
	}

	/**
	 * This method will return the port details from response packet
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return
	 */
	public JsonObject getSwitchPortResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody,
			final String getType) {
		return getSwitchPortResponse(responsePacket, requestBody);
	}

	/**
	 * This method will return the port details from response packet
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @return
	 */
	public JsonObject getSwitchPortResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody) {
		LOG.trace("Start getSwitchPortResponse");
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}
		final String rootJsonName = VtnServiceJsonConsts.PORTS;
		final JsonObject root = new JsonObject();
		JsonObject switchPort = null;
		final JsonArray switchPortArray = new JsonArray();
		LOG.debug("Json Name :" + rootJsonName);
		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			switchPort = new JsonObject();
			int count = VtnServiceJsonConsts.VAL_1;
			if (!requestBody.has(VtnServiceJsonConsts.PORTNAME)) {
				count = Integer
						.parseInt(IpcDataUnitWrapper
								.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			} else {
				if (responsePacket.length == 0) {
					count = responsePacket.length;
				}
			}
			switchPort.addProperty(VtnServiceJsonConsts.COUNT, count);
			root.add(rootJsonName, switchPort);
		} else {
			IpcStruct valPortStruct = null;
			IpcStruct valPortStStruct = null;
			IpcStruct valPortStatsStruct = null;
			// json array will be required for list type of cases

			for (int index = 0; index < responsePacket.length; index++) {
				switchPort = new JsonObject();
				byte validBit;

				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;

				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keySwitchPortStruct = (IpcStruct) responsePacket[index++];
				switchPort.addProperty(VtnServiceJsonConsts.PORTNAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								keySwitchPortStruct,
								VtnServiceIpcConsts.PORT_ID));

				if (!opType.equalsIgnoreCase(VtnServiceJsonConsts.NORMAL)) {
					LOG.debug("Case : not Normal or Show");
					/*
					 * this part is always required in Show, but not required in
					 * List + "normal" op type
					 */
					LOG.debug("targetdb : State");
					valPortStatsStruct = (IpcStruct) responsePacket[index++];
					if (opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
						// getting inner structure from Port Stats struct by
						// checking valid
						// bit.
						validBit = valPortStatsStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.upplVaPortStatsIndex.kIdxPortStatSt
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							valPortStStruct = IpcDataUnitWrapper
									.getInnerIpcStruct(valPortStatsStruct,
											VtnServiceIpcConsts.PORT_ST_VAL);
						}
					} else {
						valPortStStruct = valPortStatsStruct;
					}

					if (valPortStStruct != null) {

						// getting inner structure from St struct by checking
						// valid bit.
						validBit = valPortStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValPortStIndex.kIdxPortSt
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							valPortStruct = IpcDataUnitWrapper
									.getInnerIpcStruct(valPortStStruct,
											VtnServiceIpcConsts.PORT);
							validBit = valPortStruct
									.getByte(
											VtnServiceIpcConsts.VALID,
											UncPhysicalStructIndexEnum.UpplValPortIndex.kIdxPortNumber
													.ordinal());
							if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
									.ordinal()
									&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
											.ordinal()) {
								setValueToJsonObject(
										validBit,
										switchPort,
										VtnServiceJsonConsts.PORT_ID,
										IpcDataUnitWrapper
												.getIpcStructUint32Value(
														valPortStruct,
														VtnServiceIpcConsts.PORT_NUMBER));
							}

							validBit = valPortStruct
									.getByte(
											VtnServiceIpcConsts.VALID,
											UncPhysicalStructIndexEnum.UpplValPortIndex.kIdxPortDescription
													.ordinal());
							if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
									.ordinal()
									&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
											.ordinal()) {
								setValueToJsonObject(
										validBit,
										switchPort,
										VtnServiceJsonConsts.DESCRIPTION,
										IpcDataUnitWrapper
												.getIpcStructUint8ArrayValue(
														valPortStruct,
														VtnServiceIpcConsts.DESCRIPTION));
							}
							validBit = valPortStruct
									.getByte(
											VtnServiceIpcConsts.VALID,
											UncPhysicalStructIndexEnum.UpplValPortIndex.kIdxPortAdminStatus
													.ordinal());
							if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
									.ordinal()
									&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
											.ordinal()) {
								if (IpcDataUnitWrapper
										.getIpcStructUint8Value(
												valPortStruct,
												VtnServiceJsonConsts.ADMIN_STATUS)
										.equalsIgnoreCase(
												UncPhysicalStructIndexEnum.UpplPortAdminStatus.UPPL_PORT_ADMIN_UP
														.getValue())) {
									setValueToJsonObject(validBit, switchPort,
											VtnServiceJsonConsts.ADMINSTATUS,
											VtnServiceJsonConsts.UP);
								} else if (IpcDataUnitWrapper
										.getIpcStructUint8Value(
												valPortStruct,
												VtnServiceJsonConsts.ADMIN_STATUS)
										.equalsIgnoreCase(
												UncPhysicalStructIndexEnum.UpplPortAdminStatus.UPPL_PORT_ADMIN_DOWN
														.getValue())) {
									setValueToJsonObject(validBit, switchPort,
											VtnServiceJsonConsts.ADMINSTATUS,
											VtnServiceJsonConsts.DOWN);
								} else {
									LOG.debug("Adminstatus : invalid");
								}
								LOG.debug("AdminStatus :"
										+ IpcDataUnitWrapper
												.getIpcStructUint8Value(
														valPortStruct,
														VtnServiceJsonConsts.ADMIN_STATUS));
							}
							validBit = valPortStruct
									.getByte(
											VtnServiceIpcConsts.VALID,
											UncPhysicalStructIndexEnum.UpplValPortIndex.kIdxPortTrunkAllowedVlan
													.ordinal());
							if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
									.ordinal()
									&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
											.ordinal()) {
								setValueToJsonObject(
										validBit,
										switchPort,
										VtnServiceJsonConsts.TRUNK_ALLOWED_VLAN,
										IpcDataUnitWrapper
												.getIpcStructUint16Value(
														valPortStruct,
														VtnServiceIpcConsts.TRUNK_ALLOWED_VLAN));
							}
						}

						// using valPortStStruct for other parameters
						validBit = valPortStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValPortStIndex.kIdxPortOperStatus
												.ordinal());

						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valPortStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equalsIgnoreCase(
											UncPhysicalStructIndexEnum.UpplPortOperStatus.UPPL_PORT_OPER_UP
													.getValue())) {
								setValueToJsonObject(validBit, switchPort,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.UP);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valPortStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equalsIgnoreCase(
											UncPhysicalStructIndexEnum.UpplPortOperStatus.UPPL_PORT_OPER_DOWN
													.getValue())) {
								setValueToJsonObject(validBit, switchPort,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.DOWN);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valPortStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equalsIgnoreCase(
											UncPhysicalStructIndexEnum.UpplPortOperStatus.UPPL_PORT_OPER_UNKNOWN
													.getValue())) {
								setValueToJsonObject(validBit, switchPort,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.UNKNOWN);
							} else {
								LOG.debug("Operstatus : invalid");
							}
							LOG.debug("Operstatus :"
									+ IpcDataUnitWrapper
											.getIpcStructUint8Value(
													valPortStStruct,
													VtnServiceIpcConsts.OPERSTATUS));
						}
						validBit = valPortStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValPortStIndex.kIdxPortMacAddress
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							setValueToJsonObject(validBit, switchPort,
									VtnServiceJsonConsts.MACADDR,
									IpcDataUnitWrapper.getMacAddress(
											valPortStStruct,
											VtnServiceIpcConsts.PORT_MAC_ADDR));
						}

						validBit = valPortStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValPortStIndex.kIdxPortSpeed
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							setValueToJsonObject(validBit, switchPort,
									VtnServiceJsonConsts.SPEED,
									IpcDataUnitWrapper.getIpcStructUint64Value(
											valPortStStruct,
											VtnServiceIpcConsts.SPEED));
						}
						validBit = valPortStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValPortStIndex.kIdxPortDuplex
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							final String duplexVal = IpcDataUnitWrapper
									.getIpcStructUint8Value(valPortStStruct,
											VtnServiceIpcConsts.DUPLEX);
							if (null != duplexVal
									&& !duplexVal.isEmpty()
									&& duplexVal
											.equalsIgnoreCase(String
													.valueOf(UncPhysicalStructIndexEnum.UpplPortDuplex.UPPL_PORT_DUPLEX_HALF
															.getValue()))) {
								setValueToJsonObject(validBit, switchPort,
										VtnServiceJsonConsts.DUPLEX,
										VtnServiceIpcConsts.HALF);
							} else if (null != duplexVal
									&& !duplexVal.isEmpty()
									&& duplexVal
											.equalsIgnoreCase(String
													.valueOf(UncPhysicalStructIndexEnum.UpplPortDuplex.UPPL_PORT_DUPLEX_FULL
															.getValue()))) {
								setValueToJsonObject(validBit, switchPort,
										VtnServiceJsonConsts.DUPLEX,
										VtnServiceIpcConsts.FULL);
							} else {
								LOG.debug("Duplex : invalid");
							}
							LOG.debug("Duplex :"
									+ IpcDataUnitWrapper
											.getIpcStructUint8Value(
													valPortStStruct,
													VtnServiceIpcConsts.DUPLEX));
						}
						validBit = valPortStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValPortStIndex.kIdxPortAlarmsStatus
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							setValueToJsonObject(
									validBit,
									switchPort,
									VtnServiceJsonConsts.ALARMSSTATUS,
									IpcDataUnitWrapper
											.getIpcStructUint64HexaValue(
													valPortStStruct,
													VtnServiceIpcConsts.ALARM_STATUS));
						}
						validBit = valPortStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValPortStIndex.kIdxPortDirection
												.ordinal());

						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							final String direction = IpcDataUnitWrapper
									.getIpcStructUint8Value(valPortStStruct,
											VtnServiceIpcConsts.DIRECTION);
							if (null != direction
									&& !direction.isEmpty()
									&& direction
											.equals(String
													.valueOf(UncPhysicalStructIndexEnum.UpplPortDirection.UPPL_PORT_DIR_INTERNEL
															.getValue()))) {
								setValueToJsonObject(validBit, switchPort,
										VtnServiceJsonConsts.DIRECTION,
										VtnServiceJsonConsts.DIRECTION_INTERNAL);
							} else if (null != direction
									&& !direction.isEmpty()
									&& direction
											.equals(String
													.valueOf(UncPhysicalStructIndexEnum.UpplPortDirection.UPPL_PORT_DIR_EXTERNAL
															.getValue()))) {
								setValueToJsonObject(validBit, switchPort,
										VtnServiceJsonConsts.DIRECTION,
										VtnServiceJsonConsts.DIRECTION_EXTERNAL);
							} else if (null != direction
									&& !direction.isEmpty()
									&& direction
											.equals(String
													.valueOf(UncPhysicalStructIndexEnum.UpplPortDirection.UPPL_PORT_DIR_UNKNOWN
															.getValue()))) {
								setValueToJsonObject(validBit, switchPort,
										VtnServiceJsonConsts.DIRECTION,
										VtnServiceJsonConsts.DIRECTION_UNKNOWN);
							} else {
								LOG.debug("Direction : invalid");
							}
							LOG.debug("Direction :"
									+ IpcDataUnitWrapper
											.getIpcStructUint8Value(
													valPortStStruct,
													VtnServiceIpcConsts.DIRECTION));
						}
						validBit = valPortStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValPortStIndex.kIdxPortLogicalPortId
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							setValueToJsonObject(
									validBit,
									switchPort,
									VtnServiceJsonConsts.LOGICAL_PORT_ID,
									IpcDataUnitWrapper
											.getIpcStructUint8ArrayValue(
													valPortStStruct,
													VtnServiceIpcConsts.LOGICAL_PORT_ID));
						}

						if (opType
								.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
							LOG.debug("op : detail");
							final JsonObject vtnStationStats = createPortStatisticsJson(valPortStatsStruct);
							switchPort.add(VtnServiceJsonConsts.STATISTICS,
									vtnStationStats);
						}
					}
				} else {
					index++;
				}
				switchPortArray.add(switchPort);
			}
			/*
			 * finally add array to root json object and return the same.
			 */
			root.add(rootJsonName, switchPortArray);
		}
		LOG.debug("Response Json: " + root.toString());
		LOG.trace("Complete getSwitchPortResponse");
		return root;
	}

	/**
	 * Create Json Port Statistics information
	 * 
	 * @param createPortStatisticsJson
	 * @return
	 */
	private JsonObject createPortStatisticsJson(final IpcStruct valPortStats) {
		LOG.trace("Start createPortStatisticsJson");
		byte validBit;
		final JsonObject switchPortStats = new JsonObject();

		validBit = valPortStats
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.upplVaPortStatsIndex.kIdxPortStatRxPackets
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, switchPortStats,
					VtnServiceJsonConsts.RX_PACKETS,
					IpcDataUnitWrapper.getIpcStructUint64Value(valPortStats,
							VtnServiceIpcConsts.RX_PACKETS));
		}
		validBit = valPortStats
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.upplVaPortStatsIndex.kIdxPortStatRxBytes
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, switchPortStats,
					VtnServiceJsonConsts.RX_BYTES,
					IpcDataUnitWrapper.getIpcStructUint64Value(valPortStats,
							VtnServiceIpcConsts.RX_BYTES));
		}
		validBit = valPortStats
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.upplVaPortStatsIndex.kIdxPortStatRxDropped
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, switchPortStats,
					VtnServiceJsonConsts.RX_DROPPED,
					IpcDataUnitWrapper.getIpcStructUint64Value(valPortStats,
							VtnServiceIpcConsts.RX_DROPPED));
		}
		validBit = valPortStats
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.upplVaPortStatsIndex.kIdxPortStatRxErrors
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, switchPortStats,
					VtnServiceJsonConsts.RX_ERRORS,
					IpcDataUnitWrapper.getIpcStructUint64Value(valPortStats,
							VtnServiceIpcConsts.RX_ERRORS));
		}
		validBit = valPortStats
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.upplVaPortStatsIndex.kIdxPortStatRxFrameErr
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, switchPortStats,
					VtnServiceJsonConsts.RX_FRAMEERR,
					IpcDataUnitWrapper.getIpcStructUint64Value(valPortStats,
							VtnServiceIpcConsts.RX_FRAME_ERR));
		}
		validBit = valPortStats
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.upplVaPortStatsIndex.kIdxPortStatRxCrcErr
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, switchPortStats,
					VtnServiceJsonConsts.RX_CRCERR,
					IpcDataUnitWrapper.getIpcStructUint64Value(valPortStats,
							VtnServiceIpcConsts.RX_CRC_ERRS));
		}
		validBit = valPortStats
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.upplVaPortStatsIndex.kIdxPortStatRxOverErr
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, switchPortStats,
					VtnServiceJsonConsts.RX_OVERERR,
					IpcDataUnitWrapper.getIpcStructUint64Value(valPortStats,
							VtnServiceIpcConsts.RX_OVER_ERR));
		}
		validBit = valPortStats
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.upplVaPortStatsIndex.kIdxPortStatTxPackets
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, switchPortStats,
					VtnServiceJsonConsts.TX_PACKETS,
					IpcDataUnitWrapper.getIpcStructUint64Value(valPortStats,
							VtnServiceIpcConsts.TX_PACKETS));
		}
		validBit = valPortStats
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.upplVaPortStatsIndex.kIdxPortStatTxBytes
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, switchPortStats,
					VtnServiceJsonConsts.TX_BYTES,
					IpcDataUnitWrapper.getIpcStructUint64Value(valPortStats,
							VtnServiceIpcConsts.TX_BYTES));
		}
		validBit = valPortStats
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.upplVaPortStatsIndex.kIdxPortStatTxDropped
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, switchPortStats,
					VtnServiceJsonConsts.TX_DROPPED,
					IpcDataUnitWrapper.getIpcStructUint64Value(valPortStats,
							VtnServiceIpcConsts.TX_DROPPED));
		}
		validBit = valPortStats
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.upplVaPortStatsIndex.kIdxPortStatTxErrors
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, switchPortStats,
					VtnServiceJsonConsts.TX_ERRORS,
					IpcDataUnitWrapper.getIpcStructUint64Value(valPortStats,
							VtnServiceIpcConsts.TX_ERRORS));
		}
		validBit = valPortStats
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.upplVaPortStatsIndex.kIdxPortStatCollisions
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, switchPortStats,
					VtnServiceJsonConsts.COLLISIONS,
					IpcDataUnitWrapper.getIpcStructUint64Value(valPortStats,
							VtnServiceIpcConsts.COLLISIONS));
		}
		LOG.debug("Statistics Json: " + switchPortStats.toString());
		LOG.trace("Complete createPortStatisticsJson");
		return switchPortStats;
	}

	/**
	 * Gets the Switch port member response.
	 * 
	 * @param responsePacket
	 *            the response packet
	 * @param switchPort
	 *            the switchPort Json Object
	 * @param getType
	 *            the operation type
	 * @return the domain logical port member response
	 */
	public JsonObject getSwitchPortMemberResponse(
			final IpcDataUnit[] responsePacket, final JsonObject switchPort,
			final String getType) {
		LOG.trace("Start getSwitchPortMemberResponse");
		final JsonObject switchPortNeighbour = new JsonObject();
		int index = 0;
		byte validBit;
		if (responsePacket != null && responsePacket.length > 0) {
			IpcStruct valPortStNeighbourStruct = null;
			LOG.debug("getType: " + getType);
			// skipping key type
			LOG.debug("Skip key type: no use");
			index++;
			// skipping key structure
			LOG.debug("Skip key structure: no use");
			index++;
			valPortStNeighbourStruct = (IpcStruct) responsePacket[index++];
			validBit = valPortStNeighbourStruct
					.getByte(
							VtnServiceIpcConsts.VALID,
							UncPhysicalStructIndexEnum.UpplValPortNeighborIndex.kIdxPortConnectedSwitchId
									.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, switchPortNeighbour,
						VtnServiceJsonConsts.CONNECTED_SWITCH_ID,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valPortStNeighbourStruct,
								VtnServiceIpcConsts.CONNECTED_SWITCH_ID));
			}
			validBit = valPortStNeighbourStruct
					.getByte(
							VtnServiceIpcConsts.VALID,
							UncPhysicalStructIndexEnum.UpplValPortNeighborIndex.kIdxPortConnectedPortId
									.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, switchPortNeighbour,
						VtnServiceJsonConsts.CONNECTED_PORT_ID,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valPortStNeighbourStruct,
								VtnServiceIpcConsts.CONNECTED_PORT_ID));
			}
			validBit = valPortStNeighbourStruct.getByte(VtnServiceIpcConsts.VALID,
							UncPhysicalStructIndexEnum.UpplValPortNeighborIndex.kIdxPortConnectedControllerId
									.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, switchPortNeighbour,
						VtnServiceJsonConsts.CONTROLLERID,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valPortStNeighbourStruct,
								VtnServiceIpcConsts.CONNECTED_CONTROLLER_ID));
			}
		}
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			switchPort.get(VtnServiceJsonConsts.PORT).getAsJsonObject()
					.add(VtnServiceJsonConsts.NEIGHBOR, switchPortNeighbour);
		} else {
			switchPort.add(VtnServiceJsonConsts.NEIGHBOR, switchPortNeighbour);
		}
		LOG.debug("Response Json: " + switchPort.toString());
		LOG.trace("Complete getSwitchPortMemberResponse");
		return switchPort;
	}

	/**
	 * Gets the link response.
	 * 
	 * @param responsePacket
	 *            the response packet
	 * @param requestBody
	 *            the request body
	 * @param getType
	 *            the get type
	 * @return the link response
	 */
	public JsonObject getLinkResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody, final String getType) {
		return getLinkResponse(responsePacket, requestBody);
	}

	/**
	 * Gets the link response.
	 * 
	 * @param responsePacket
	 *            the response packet
	 * @param requestBody
	 *            the request body
	 * @return the link response
	 */
	public JsonObject getLinkResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody) {
		LOG.trace("Start getLinkResponse");
		final JsonObject root = new JsonObject();
		final JsonArray linksArray = new JsonArray();
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}
		final String rootJsonName = VtnServiceJsonConsts.LINKS;
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject links = null;
		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			links = new JsonObject();
			int count = VtnServiceJsonConsts.VAL_1;
			if (!requestBody.has(VtnServiceJsonConsts.LINKNAME)) {
				count = Integer
						.parseInt(IpcDataUnitWrapper
								.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			} else {
				if (responsePacket.length == 0) {
					count = responsePacket.length;
				}
			}
			links.addProperty(VtnServiceJsonConsts.COUNT, count);
			root.add(rootJsonName, links);

		} else {
			for (int index = 0; index < responsePacket.length; index++) {
				links = new JsonObject();
				byte validBit;
				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;
				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keyLinkStruct = (IpcStruct) responsePacket[index++];
				String linkname;
				final String switch1_id = IpcDataUnitWrapper
						.getIpcStructUint8ArrayValue(keyLinkStruct,
								VtnServiceIpcConsts.SWITCH_ID1);
				final String switch2_id = IpcDataUnitWrapper
						.getIpcStructUint8ArrayValue(keyLinkStruct,
								VtnServiceIpcConsts.SWITCH_ID2);
				final String port1_name = IpcDataUnitWrapper
						.getIpcStructUint8ArrayValue(keyLinkStruct,
								VtnServiceIpcConsts.PORT_ID1);
				final String port2_name = IpcDataUnitWrapper
						.getIpcStructUint8ArrayValue(keyLinkStruct,
								VtnServiceIpcConsts.PORT_ID2);
				linkname = switch1_id + VtnServiceJsonConsts.LINKCONCATENATOR
						+ port1_name + VtnServiceJsonConsts.LINKCONCATENATOR
						+ switch2_id + VtnServiceJsonConsts.LINKCONCATENATOR
						+ port2_name;
				links.addProperty(VtnServiceJsonConsts.LINKNAME, linkname);
				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */
				if (opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
					LOG.debug("Case : Show or detail");
					links.addProperty(VtnServiceJsonConsts.SWITCH1ID,
							switch1_id);
					links.addProperty(VtnServiceJsonConsts.SWITCH2ID,
							switch2_id);
					links.addProperty(VtnServiceJsonConsts.PORT1_NAME,
							port1_name);
					links.addProperty(VtnServiceJsonConsts.PORT2_NAME,
							port2_name);
					/*
					 * add valid informations from value structure
					 */
					final IpcStruct valLinkStStruct = (IpcStruct) responsePacket[index++];
					validBit = valLinkStStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncPhysicalStructIndexEnum.UpplValLinkStIndex.kIdxLinkStLink
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {

						final IpcStruct valLinkStruct = IpcDataUnitWrapper
								.getInnerIpcStruct(valLinkStStruct,
										VtnServiceIpcConsts.LINK_VAL);

						validBit = valLinkStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValLinkIndex.kIdxLinkDescription
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							setValueToJsonObject(
									validBit,
									links,
									VtnServiceJsonConsts.DESCRIPTION,
									IpcDataUnitWrapper
											.getIpcStructUint8ArrayValue(
													valLinkStruct,
													VtnServiceIpcConsts.DESCRIPTION));
						}
					}
					// from link st structure
					validBit = valLinkStStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncPhysicalStructIndexEnum.UpplValLinkStIndex.kIdxLinkStOperStatus
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valLinkStStruct,
										VtnServiceIpcConsts.OPERSTATUS)
								.equalsIgnoreCase(
										UncPhysicalStructIndexEnum.UpplSwitchOperStatus.UPPL_SWITCH_OPER_UP
												.getValue())) {
							setValueToJsonObject(validBit, links,
									VtnServiceJsonConsts.OPERSTATUS,
									VtnServiceJsonConsts.UP);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valLinkStStruct,
										VtnServiceIpcConsts.OPERSTATUS)
								.equalsIgnoreCase(
										UncPhysicalStructIndexEnum.UpplSwitchOperStatus.UPPL_SWITCH_OPER_DOWN
												.getValue())) {
							setValueToJsonObject(validBit, links,
									VtnServiceJsonConsts.OPERSTATUS,
									VtnServiceJsonConsts.DOWN);
						} else if (IpcDataUnitWrapper
								.getIpcStructUint8Value(valLinkStStruct,
										VtnServiceIpcConsts.OPERSTATUS)
								.equalsIgnoreCase(
										UncPhysicalStructIndexEnum.UpplSwitchOperStatus.UPPL_SWITCH_OPER_UNKNOWN
												.getValue())) {
							setValueToJsonObject(validBit, links,
									VtnServiceJsonConsts.OPERSTATUS,
									VtnServiceJsonConsts.UNKNOWN);
						} else {
							LOG.debug("Operstatus : invalid");
						}
						LOG.debug("Operstatus :"
								+ IpcDataUnitWrapper.getIpcStructUint8Value(
										valLinkStStruct,
										VtnServiceIpcConsts.OPERSTATUS));
					}
				} else {
					LOG.debug("Operation : normal Skip value struture");
					index++;
				}
				// add current json object to array
				linksArray.add(links);
			}
			/*
			 * finally add array to root json object and return the same.
			 */
			root.add(rootJsonName, linksArray);
		}
		LOG.debug("Response Json: " + root.toString());
		LOG.trace("Complete getLinkResponse");
		return root;
	}

	/**
	 * Sets the value to json object.
	 * 
	 * @param validBit
	 *            the valid bit
	 * @param json
	 *            the json
	 * @param key
	 *            the key
	 * @param value
	 *            the value
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
	 * Gets the switch response.
	 * 
	 * @param responsePacket
	 *            the response packet
	 * @param requestBody
	 *            the request body
	 * @param getType
	 *            the get type
	 * @return the switch response
	 */
	public JsonObject getSwitchResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody, final String getType) {
		LOG.trace("Start getSwitchResponse");
		final JsonObject root = new JsonObject();
		JsonArray switchesArray = null;
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
			rootJsonName = VtnServiceJsonConsts.SWITCH;
		} else {
			rootJsonName = VtnServiceJsonConsts.SWITCHES;
			// json array will be required for list type of cases
			switchesArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject switches = null;
		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			switches = new JsonObject();
			switches.addProperty(
					VtnServiceJsonConsts.COUNT,
					IpcDataUnitWrapper
							.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, switches);
		} else {
			JsonObject statisticsJson = null;
			for (int index = 0; index < responsePacket.length; index++) {

				switches = new JsonObject();
				byte validBit;
				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;
				/*
				 * add mandatory informations from key structure
				 */
				final IpcStruct keySwitchStruct = (IpcStruct) responsePacket[index++];
				switches.addProperty(VtnServiceJsonConsts.SWITCHID,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								keySwitchStruct, VtnServiceIpcConsts.SWITCHID));
				/*
				 * this part is always required in Show, but not required in
				 * List + "normal" op type
				 */
				if (getType.equals(VtnServiceJsonConsts.SHOW)
						|| opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {

					LOG.debug("Case : Show or detail");

					IpcStruct valSwitchStStruct = (IpcStruct) responsePacket[index++];

					if (getType.equals(VtnServiceJsonConsts.SHOW)
							&& opType
									.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
						LOG.debug("Case : Show with detail");
						final IpcStruct valSwitchStDetailStruct = valSwitchStStruct;
						statisticsJson = new JsonObject();
						validBit = valSwitchStDetailStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValSwitchStDetailIndex.kIdxSwitchSt
												.ordinal());

						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							valSwitchStStruct = valSwitchStDetailStruct
									.getInner(VtnServiceIpcConsts.SWITCH_ST_VAL);
						} else {
							valSwitchStStruct = null;
						}

						validBit = valSwitchStDetailStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValSwitchStDetailIndex.kIdxSwitchStatFlowCount
												.ordinal());

						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {

							setValueToJsonObject(validBit, statisticsJson,
									VtnServiceJsonConsts.FLOWCOUNT,
									IpcDataUnitWrapper.getIpcStructUint32Value(
											valSwitchStDetailStruct,
											VtnServiceIpcConsts.FLOW_COUNT));
						}
					}

					if (valSwitchStStruct != null) {
						/*
						 * add valid informations from value structure
						 */
						validBit = valSwitchStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValSwitchStIndex.kIdxSwitch
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							final IpcStruct valSwitchStruct = IpcDataUnitWrapper
									.getInnerIpcStruct(valSwitchStStruct,
											VtnServiceIpcConsts.SWITCH_VAL);
							validBit = valSwitchStruct
									.getByte(
											VtnServiceIpcConsts.VALID,
											UncPhysicalStructIndexEnum.UpplValSwitchIndex.kIdxSwitchDescription
													.ordinal());
							if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
									.ordinal()
									&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
											.ordinal()) {
								setValueToJsonObject(
										validBit,
										switches,
										VtnServiceJsonConsts.DESCRIPTION,
										IpcDataUnitWrapper
												.getIpcStructUint8ArrayValue(
														valSwitchStruct,
														VtnServiceIpcConsts.DESCRIPTION));
							}
							validBit = valSwitchStruct
									.getByte(
											VtnServiceIpcConsts.VALID,
											UncPhysicalStructIndexEnum.UpplValSwitchIndex.kIdxSwitchModel
													.ordinal());
							if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
									.ordinal()
									&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
											.ordinal()) {
								setValueToJsonObject(
										validBit,
										switches,
										VtnServiceJsonConsts.MODEL,
										IpcDataUnitWrapper
												.getIpcStructUint8ArrayValue(
														valSwitchStruct,
														VtnServiceJsonConsts.MODEL));
							}
							validBit = valSwitchStruct
									.getByte(
											VtnServiceIpcConsts.VALID,
											UncPhysicalStructIndexEnum.UpplValSwitchIndex.kIdxSwitchAdminStatus
													.ordinal());
							if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
									.ordinal()
									&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
											.ordinal()) {
								if (IpcDataUnitWrapper
										.getIpcStructUint8Value(
												valSwitchStruct,
												VtnServiceJsonConsts.ADMIN_STATUS)
										.equalsIgnoreCase(
												UncPhysicalStructIndexEnum.UpplSwitchAdminStatus.UPPL_SWITCH_ADMIN_DOWN
														.getValue())) {
									setValueToJsonObject(validBit, switches,
											VtnServiceJsonConsts.ADMINSTATUS,
											VtnServiceJsonConsts.DOWN);
								} else if (IpcDataUnitWrapper
										.getIpcStructUint8Value(
												valSwitchStruct,
												VtnServiceJsonConsts.ADMIN_STATUS)
										.equalsIgnoreCase(
												UncPhysicalStructIndexEnum.UpplSwitchAdminStatus.UPPL_SWITCH_ADMIN_UP
														.getValue())) {
									setValueToJsonObject(validBit, switches,
											VtnServiceJsonConsts.ADMINSTATUS,
											VtnServiceJsonConsts.UP);
								} else {
									LOG.debug("Adminstatus : invalid");
								}
								LOG.debug("Adminstatus :"
										+ IpcDataUnitWrapper
												.getIpcStructUint8Value(
														valSwitchStruct,
														VtnServiceJsonConsts.ADMIN_STATUS));
							}
							validBit = valSwitchStruct
									.getByte(
											VtnServiceIpcConsts.VALID,
											UncPhysicalStructIndexEnum.UpplValSwitchIndex.kIdxSwitchIPAddress
													.ordinal());
							if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
									.ordinal()
									&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
											.ordinal()) {
								setValueToJsonObject(
										validBit,
										switches,
										VtnServiceJsonConsts.IPADDR,
										IpcDataUnitWrapper
												.getIpcStructIpv4Value(
														valSwitchStruct,
														VtnServiceIpcConsts.IP_ADDRESS));
							}
							validBit = valSwitchStruct
									.getByte(
											VtnServiceIpcConsts.VALID,
											UncPhysicalStructIndexEnum.UpplValSwitchIndex.kIdxSwitchIPV6Address
													.ordinal());
							if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
									.ordinal()
									&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
											.ordinal()) {
								setValueToJsonObject(
										validBit,
										switches,
										VtnServiceJsonConsts.IPV6ADDR,
										IpcDataUnitWrapper
												.getIpcStructIpv6Value(
														valSwitchStruct,
														VtnServiceIpcConsts.IPV6_ADDRESS));
							}
							validBit = valSwitchStruct
									.getByte(
											VtnServiceIpcConsts.VALID,
											UncPhysicalStructIndexEnum.UpplValSwitchIndex.kIdxSwitchDomainName
													.ordinal());
							if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
									.ordinal()
									&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
											.ordinal()) {
								setValueToJsonObject(
										validBit,
										switches,
										VtnServiceJsonConsts.DOMAINID,
										IpcDataUnitWrapper
												.getIpcStructUint8ArrayValue(
														valSwitchStruct,
														VtnServiceIpcConsts.DOMAIN_NAME));
							}
						}
						// from switch st structure
						validBit = valSwitchStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValSwitchStIndex.kIdxSwitchOperStatus
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valSwitchStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equalsIgnoreCase(
											UncPhysicalStructIndexEnum.UpplSwitchOperStatus.UPPL_SWITCH_OPER_UP
													.getValue())) {
								setValueToJsonObject(validBit, switches,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.UP);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valSwitchStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equalsIgnoreCase(
											UncPhysicalStructIndexEnum.UpplSwitchOperStatus.UPPL_SWITCH_OPER_DOWN
													.getValue())) {
								setValueToJsonObject(validBit, switches,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.DOWN);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valSwitchStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equalsIgnoreCase(
											UncPhysicalStructIndexEnum.UpplSwitchOperStatus.UPPL_SWITCH_OPER_UNKNOWN
													.getValue())) {
								setValueToJsonObject(validBit, switches,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.UNKNOWN);
							} else {
								LOG.debug("Operstatus : invalid");
							}
							LOG.debug("Operstatus :"
									+ IpcDataUnitWrapper
											.getIpcStructUint8Value(
													valSwitchStStruct,
													VtnServiceIpcConsts.OPERSTATUS));
						}
						validBit = valSwitchStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValSwitchStIndex.kIdxSwitchManufacturer
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							setValueToJsonObject(
									validBit,
									switches,
									VtnServiceJsonConsts.MANUFACTURER,
									IpcDataUnitWrapper
											.getIpcStructUint8ArrayValue(
													valSwitchStStruct,
													VtnServiceJsonConsts.MANUFACTURER));
						}
						validBit = valSwitchStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValSwitchStIndex.kIdxSwitchHardware
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							setValueToJsonObject(
									validBit,
									switches,
									VtnServiceJsonConsts.HARDWARE,
									IpcDataUnitWrapper
											.getIpcStructUint8ArrayValue(
													valSwitchStStruct,
													VtnServiceJsonConsts.HARDWARE));
						}
						validBit = valSwitchStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValSwitchStIndex.kIdxSwitchSoftware
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							setValueToJsonObject(
									validBit,
									switches,
									VtnServiceJsonConsts.SOFTWARE,
									IpcDataUnitWrapper
											.getIpcStructUint8ArrayValue(
													valSwitchStStruct,
													VtnServiceJsonConsts.SOFTWARE));
						}
						validBit = valSwitchStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValSwitchStIndex.kIdxSwitchAlarmStatus
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							setValueToJsonObject(
									validBit,
									switches,
									VtnServiceJsonConsts.ALARMSSTATUS,
									IpcDataUnitWrapper
											.getIpcStructUint64HexaValue(
													valSwitchStStruct,
													VtnServiceIpcConsts.ALARM_STATUS));
						}
					}
				} else {
					LOG.debug("Operation : normal Skip value struture");
					index++;
				}
				// add current json object to array, if it has been initialized
				// earlier
				if (null != switchesArray) {
					switchesArray.add(switches);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != switchesArray) {
				root.add(rootJsonName, switchesArray);
			} else {
				if (statisticsJson != null
						&& opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
					switches.add(VtnServiceJsonConsts.STATISTICS,
							statisticsJson);
				}
				root.add(rootJsonName, switches);
			}
		}
		LOG.debug("Response Json: " + root.toString());
		LOG.trace("Complete getSwitchResponse");
		return root;
	}

	/**
	 * Gets the domain logical port response.
	 * 
	 * @param responsePacket
	 *            the response packet
	 * @param requestBody
	 *            the request body
	 * @param getType
	 *            the get type
	 * @return the domain logical port response
	 */
	public JsonObject getDomainLogicalPortResponse(
			final IpcDataUnit[] responsePacket, final JsonObject requestBody,
			final String getType) {
		return getDomainLogicalPortResponse(responsePacket, requestBody);
	}
	
	/**
	 * Gets the domain logical port response.
	 * 
	 * @param responsePacket
	 *            the response packet
	 * @param requestBody
	 *            the request body
	 * @return the domain logical port response
	 */
	public JsonObject getDomainLogicalPortResponse(
			final IpcDataUnit[] responsePacket, final JsonObject requestBody) {
		LOG.trace("Start getDomainLogicalPortResponse");
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}
		final String rootJsonName = VtnServiceJsonConsts.LOGICALPORTS;
		final JsonArray logicalPortsArray = new JsonArray();
		final JsonObject root = new JsonObject();
		LOG.debug("Json Name :" + rootJsonName);
		String dataType = VtnServiceJsonConsts.STATE;
		if (requestBody.has(VtnServiceJsonConsts.TARGETDB)
				&& null != requestBody.get(VtnServiceJsonConsts.TARGETDB)) {
			dataType = requestBody.get(VtnServiceJsonConsts.TARGETDB)
					.getAsString();
		}
		JsonObject logicalPort = null;
		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			logicalPort = new JsonObject();
			int count = VtnServiceJsonConsts.VAL_1;
			if (!requestBody.has(VtnServiceJsonConsts.LOGICAL_PORT_ID)) {
				count = Integer
						.parseInt(IpcDataUnitWrapper
								.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			} else {
				if (responsePacket.length == 0) {
					count = responsePacket.length;
				}
			}
			logicalPort.addProperty(VtnServiceJsonConsts.COUNT, count);
			root.add(rootJsonName, logicalPort);

		} else {
			IpcStruct valLogicalPortStruct = null;
			IpcStruct valLogicalPortStStruct = null;
			byte validBit;
			for (int index = 0; index < responsePacket.length; index++) {
				logicalPort = new JsonObject();
				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;
				/*
				 * add mandatory informations from key structure
				 */if (dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
					LOG.debug("targetdb : State");
					final IpcStruct keyLogicalPortStruct = (IpcStruct) responsePacket[index++];
					logicalPort.addProperty(
							VtnServiceJsonConsts.LOGICAL_PORT_ID,
							IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
									keyLogicalPortStruct,
									VtnServiceIpcConsts.PORT_ID));
					if (opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
						LOG.debug("Case : Show or detail");
						valLogicalPortStStruct = (IpcStruct) responsePacket[index++];
						validBit = valLogicalPortStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValLogicalPortStIndex.kIdxLogicalPortSt
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							valLogicalPortStruct = IpcDataUnitWrapper
									.getInnerIpcStruct(valLogicalPortStStruct,
											VtnServiceIpcConsts.LOGICAL_PORT);
						}
						validBit = valLogicalPortStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValLogicalPortIndex.kIdxLogicalPortDescription
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							setValueToJsonObject(
									validBit,
									logicalPort,
									VtnServiceJsonConsts.DESCRIPTION,
									IpcDataUnitWrapper
											.getIpcStructUint8ArrayValue(
													valLogicalPortStruct,
													VtnServiceIpcConsts.DESCRIPTION));
						}
						validBit = valLogicalPortStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValLogicalPortIndex.kIdxLogicalPortType
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							if (IpcDataUnitWrapper
									.getIpcStructUint8Value(
											valLogicalPortStruct,
											VtnServiceIpcConsts.PORTTYPE)
									.equalsIgnoreCase(
											UncPhysicalStructIndexEnum.UpplLogicalPortType.UPPL_LP_PHYSICAL_PORT
													.getValue())) {
								setValueToJsonObject(validBit, logicalPort,
										VtnServiceJsonConsts.TYPE,
										VtnServiceJsonConsts.PORT);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(
											valLogicalPortStruct,
											VtnServiceIpcConsts.PORTTYPE)
									.equalsIgnoreCase(
											UncPhysicalStructIndexEnum.UpplLogicalPortType.UPPL_LP_SUBDOMAIN
													.getValue())) {
								setValueToJsonObject(validBit, logicalPort,
										VtnServiceJsonConsts.TYPE,
										VtnServiceJsonConsts.SUBDOMAIN);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(
											valLogicalPortStruct,
											VtnServiceIpcConsts.PORTTYPE)
									.equalsIgnoreCase(
											UncPhysicalStructIndexEnum.UpplLogicalPortType.UPPL_LP_SWITCH
													.getValue())) {
								setValueToJsonObject(validBit, logicalPort,
										VtnServiceJsonConsts.TYPE,
										VtnServiceJsonConsts.SWITCH);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(
											valLogicalPortStruct,
											VtnServiceIpcConsts.PORTTYPE)
									.equalsIgnoreCase(
											UncPhysicalStructIndexEnum.UpplLogicalPortType.UPPL_LP_TRUNK_PORT
													.getValue())) {
								setValueToJsonObject(validBit, logicalPort,
										VtnServiceJsonConsts.TYPE,
										VtnServiceJsonConsts.TRUNK);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(
											valLogicalPortStruct,
											VtnServiceIpcConsts.PORTTYPE)
									.equalsIgnoreCase(
											UncPhysicalStructIndexEnum.UpplLogicalPortType.UPPL_LP_TUNNEL_ENDPOINT
													.getValue())) {
								setValueToJsonObject(validBit, logicalPort,
										VtnServiceJsonConsts.TYPE,
										VtnServiceJsonConsts.TUNNEL_ENDPOINT);
							} else if (IpcDataUnitWrapper
                                    .getIpcStructUint8Value(
                                                    valLogicalPortStruct,
                                                    VtnServiceIpcConsts.PORTTYPE)
                                    .equalsIgnoreCase(
											UncPhysicalStructIndexEnum.UpplLogicalPortType.UPPL_LP_PORT_GROUP
													.getValue())) {
								setValueToJsonObject(validBit, logicalPort,
                                            VtnServiceJsonConsts.TYPE,
                                            VtnServiceJsonConsts.PORT_GROUP);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(
											valLogicalPortStruct,
											VtnServiceIpcConsts.PORTTYPE)
									.equalsIgnoreCase(
										UncPhysicalStructIndexEnum.UpplLogicalPortType.UPPL_LP_MAPPING_GROUP
										.getValue())) {
								setValueToJsonObject(validBit, logicalPort,
										VtnServiceJsonConsts.TYPE,
										VtnServiceJsonConsts.MAPPING_GROUP);
							} else {
								LOG.debug("PortType : invalid");
							}
							LOG.debug("PortType :"
									+ IpcDataUnitWrapper
											.getIpcStructUint8Value(
													valLogicalPortStruct,
													VtnServiceIpcConsts.PORTTYPE));
						}
						validBit = valLogicalPortStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValLogicalPortIndex.kIdxLogicalPortSwitchId
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							setValueToJsonObject(
									validBit,
									logicalPort,
									VtnServiceJsonConsts.SWITCHID,
									IpcDataUnitWrapper
											.getIpcStructUint8ArrayValue(
													valLogicalPortStruct,
													VtnServiceIpcConsts.SWITCHID));
						}
						validBit = valLogicalPortStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValLogicalPortIndex.kIdxLogicalPortPhysicalPortId
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							setValueToJsonObject(
									validBit,
									logicalPort,
									VtnServiceJsonConsts.PORTNAME,
									IpcDataUnitWrapper
											.getIpcStructUint8ArrayValue(
													valLogicalPortStruct,
													VtnServiceIpcConsts.PHYSICAL_PORT_ID));
						}
						validBit = valLogicalPortStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValLogicalPortIndex.kIdxLogicalPortOperDownCriteria
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							if (IpcDataUnitWrapper
									.getIpcStructUint8Value(
											valLogicalPortStruct,
											VtnServiceIpcConsts.OPERDOWNCRITERIA)
									.equalsIgnoreCase(
											UncPhysicalStructIndexEnum.UpplLogicalPortOperDownCriteria.UPPL_OPER_DOWN_CRITERIA_ALL
													.getValue())) {
								setValueToJsonObject(validBit, logicalPort,
										VtnServiceJsonConsts.OPERDOWN_CRITERIA,
										VtnServiceJsonConsts.ALL);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(
											valLogicalPortStruct,
											VtnServiceIpcConsts.OPERDOWNCRITERIA)
									.equalsIgnoreCase(
											UncPhysicalStructIndexEnum.UpplLogicalPortOperDownCriteria.UPPL_OPER_DOWN_CRITERIA_ANY
													.getValue())) {
								setValueToJsonObject(validBit, logicalPort,
										VtnServiceJsonConsts.OPERDOWN_CRITERIA,
										VtnServiceJsonConsts.ANY);
							} else {
								LOG.debug("OperDown Criteria : invalid");
							}
							LOG.debug("OperDown Criteria :"
									+ IpcDataUnitWrapper
											.getIpcStructUint8Value(
													valLogicalPortStruct,
													VtnServiceIpcConsts.OPERDOWNCRITERIA));
						}
						// from logical port st structure
						validBit = valLogicalPortStStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValLogicalPortStIndex.kIdxLogicalPortStOperStatus
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							if (IpcDataUnitWrapper
									.getIpcStructUint8Value(
											valLogicalPortStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equalsIgnoreCase(
											UncPhysicalStructIndexEnum.UpplLogicalPortOperStatus.UPPL_LOGICAL_PORT_OPER_UP
													.getValue())) {
								setValueToJsonObject(validBit, logicalPort,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.UP);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(
											valLogicalPortStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equalsIgnoreCase(
											UncPhysicalStructIndexEnum.UpplLogicalPortOperStatus.UPPL_LOGICAL_PORT_OPER_DOWN
													.getValue())) {
								setValueToJsonObject(validBit, logicalPort,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.DOWN);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(
											valLogicalPortStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equalsIgnoreCase(
											UncPhysicalStructIndexEnum.UpplLogicalPortOperStatus.UPPL_LOGICAL_PORT_OPER_UNKNOWN
													.getValue())) {
								setValueToJsonObject(validBit, logicalPort,
										VtnServiceJsonConsts.OPERSTATUS,
										VtnServiceJsonConsts.UNKNOWN);
							} else {
								LOG.debug("Operstatus : invalid");
							}
							LOG.debug("Operstatus :"
									+ IpcDataUnitWrapper
											.getIpcStructUint8Value(
													valLogicalPortStStruct,
													VtnServiceIpcConsts.OPERSTATUS));
						}
					} else {
						LOG.debug("Operation : normal Skip value struture");
						index++;
					}
				}
				// add current json object to array
				logicalPortsArray.add(logicalPort);
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			root.add(rootJsonName, logicalPortsArray);
		}
		LOG.debug("Response Json: " + root.toString());
		LOG.trace("Complete getDomainLogicalPortResponse");
		return root;
	}

	/**
	 * Gets the domain logical port member response.
	 * 
	 * @param responsePacket
	 *            the response packet
	 * @return the domain logical port member response
	 */
	public JsonArray getDomainLogicalPortMemberResponse(
			final IpcDataUnit[] responsePacket) {
		LOG.trace("Start getDomainLogicalPortMemberResponse");
		final JsonArray membersArray = new JsonArray();
		// String rootJsonName=VtnServiceJsonConsts.MEMBER_PORTS;
		JsonObject member = null;
		for (int index = 0; index < responsePacket.length; index++) {
			member = new JsonObject();
			// There is no use of key type
			LOG.debug("Skip key type: no use");
			index++;
			/*
			 * add mandatory informations from key structure
			 */
			final IpcStruct keyLogicalMemberPortStruct = (IpcStruct) responsePacket[index++];
			member.addProperty(VtnServiceJsonConsts.SWITCHID,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							keyLogicalMemberPortStruct,
							VtnServiceIpcConsts.SWITCHEID));
			member.addProperty(VtnServiceJsonConsts.PORTNAME,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							keyLogicalMemberPortStruct,
							VtnServiceIpcConsts.PHYSICAL_PORT_ID));

			membersArray.add(member);
		}
		LOG.debug("Member Json: " + membersArray.toString());
		LOG.trace("Complete getDomainLogicalPortMemberResponse");
		return membersArray;
	}

	/**
	 * Gets the domain logical port Neighbor response.
	 * 
	 * @param responsePacket
	 *            the response packet
	 * @return the domain logical port Neighbor response
	 */
	public JsonObject getDomainLogicalPortNeighborResponse(
			final IpcDataUnit[] responsePacket) {
		LOG.trace("Start getDomainLogicalPortNeighborResponse");
		int index = 0;
		byte validBit;
		boolean isHasNeighbor = false;
		final JsonObject Neighbor = new JsonObject();

		if (responsePacket != null && responsePacket.length > 0) {
			// skipping key type
			LOG.debug("Skip key type and key structure: no use");
			index = index + 2;

			final IpcStruct ValLmPortStNeighborStruct = (IpcStruct) responsePacket[index++];
			validBit = ValLmPortStNeighborStruct.getByte(
							VtnServiceIpcConsts.VALID,
							UncPhysicalStructIndexEnum.UpplValLmPortNeighborIndex.kIdxLmPort
									.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				IpcStruct valPortStruct = IpcDataUnitWrapper
						.getInnerIpcStruct(ValLmPortStNeighborStruct,
								VtnServiceJsonConsts.PORT);
				validBit = valPortStruct.getByte(
								VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UpplValPortIndex.kIdxPortDescription
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					isHasNeighbor = true;
					setValueToJsonObject(validBit, Neighbor,
							VtnServiceJsonConsts.DESCRIPTION,
							IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
									valPortStruct,
									VtnServiceIpcConsts.DESCRIPTION));
				}
				validBit = valPortStruct.getByte(
								VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UpplValPortIndex.kIdxPortNumber
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					isHasNeighbor = true;
					setValueToJsonObject(validBit, Neighbor,
							VtnServiceJsonConsts.PORT_ID,
							IpcDataUnitWrapper.getIpcStructUint32Value(
									valPortStruct,
									VtnServiceIpcConsts.PORT_NUMBER));
				}
				validBit = valPortStruct.getByte(
								VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UpplValPortIndex.kIdxPortAdminStatus
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					if (IpcDataUnitWrapper.getIpcStructUint8Value(
								valPortStruct, VtnServiceJsonConsts.ADMIN_STATUS)
								.equalsIgnoreCase(UncPhysicalStructIndexEnum.UpplPortAdminStatus
								.UPPL_PORT_ADMIN_UP.getValue())) {
						isHasNeighbor = true;
						setValueToJsonObject(validBit, Neighbor,
								VtnServiceJsonConsts.ADMINSTATUS, VtnServiceJsonConsts.UP);
					} else if (IpcDataUnitWrapper.getIpcStructUint8Value(
									valPortStruct, VtnServiceJsonConsts.ADMIN_STATUS)
									.equalsIgnoreCase(UncPhysicalStructIndexEnum.UpplPortAdminStatus
									.UPPL_PORT_ADMIN_DOWN.getValue())) {
						isHasNeighbor = true;
						setValueToJsonObject(validBit, Neighbor,
								VtnServiceJsonConsts.ADMINSTATUS, VtnServiceJsonConsts.DOWN);
					} else {
						LOG.debug("Adminstatus : invalid");
					}
				}
				validBit = valPortStruct.getByte(
								VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UpplValPortIndex.kIdxPortTrunkAllowedVlan
										.ordinal());
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					isHasNeighbor = true;
					setValueToJsonObject(validBit, Neighbor,
							VtnServiceJsonConsts.TRUNK_ALLOWED_VLAN,
							IpcDataUnitWrapper.getIpcStructUint16Value(
									valPortStruct,
									VtnServiceIpcConsts.TRUNK_ALLOWED_VLAN));
				}
			}
			validBit = ValLmPortStNeighborStruct.getByte(
							VtnServiceIpcConsts.VALID,
							UncPhysicalStructIndexEnum.UpplValLmPortNeighborIndex
							.kIdxLmPortConnectedSwitchId.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				isHasNeighbor = true;
				setValueToJsonObject(validBit, Neighbor,
						VtnServiceJsonConsts.CONNECTEDSWITCHID,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								ValLmPortStNeighborStruct,
								VtnServiceIpcConsts.CONNECTED_SWITCH_ID));
			}
			validBit = ValLmPortStNeighborStruct.getByte(
							VtnServiceIpcConsts.VALID,
							UncPhysicalStructIndexEnum.UpplValLmPortNeighborIndex
							.kIdxLmPortConnectedPortId.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				isHasNeighbor = true;
				setValueToJsonObject(validBit, Neighbor,
						VtnServiceJsonConsts.CONNECTEDPORTNAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								ValLmPortStNeighborStruct,
								VtnServiceIpcConsts.CONNECTED_PORT_ID));
			}
			validBit = ValLmPortStNeighborStruct.getByte(
							VtnServiceIpcConsts.VALID,
							UncPhysicalStructIndexEnum.UpplValLmPortNeighborIndex
							.kIdxLmPortConnectedControllerId.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				isHasNeighbor = true;
				setValueToJsonObject(validBit, Neighbor,
						VtnServiceJsonConsts.CONNECTEDCONTROLLERID,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								ValLmPortStNeighborStruct,
								VtnServiceIpcConsts.CONNECTED_CONTROLLER_ID));
			}
		}
		LOG.debug("neighbor Json: " + Neighbor.toString());
		LOG.trace("Complete getDomainLogicalPortNeighborResponse");
		if (!isHasNeighbor) {
			return null;
		}
		return Neighbor;
	}

	/**
	 * Gets the domain logical port boundary response.
	 * 
	 * @param responsePacket
	 *            the response packet
	 * @return the domain logical port boundary response
	 */
	public JsonObject getDomainLogicalPortBoundaryResponse(
			final IpcDataUnit[] responsePacket) {
		LOG.trace("Start getDomainLogicalPortBoundaryResponse");
		final JsonObject logicalPortBoudary = new JsonObject();
		int index = 0;
		byte validBit;
		boolean isHasBoundary = false;
		if (responsePacket != null && responsePacket.length > 0) {
			// skipping key type
			LOG.debug("Skip key type and key structure: no use");
			index = index + 2;

			final IpcStruct valLogicalPortBoundaryStruct = (IpcStruct) responsePacket[index++];
			validBit = valLogicalPortBoundaryStruct.getByte(
							VtnServiceIpcConsts.VALID,
							UncPhysicalStructIndexEnum.UpplValLogicalPortBoundaryStIndex
							.kIdxLogicalPortBoundaryCandidate.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				if (IpcDataUnitWrapper.getIpcStructUint8Value(
						valLogicalPortBoundaryStruct,
						VtnServiceIpcConsts.BOUNDARY_CANDIDATE)
						.equalsIgnoreCase(UncPhysicalStructIndexEnum
						.UpplLogicalPortBoundaryCandidate.UPPL_LP_BDRY_CANDIDATE_YES.getValue())) {
					isHasBoundary = true;
					setValueToJsonObject(validBit, logicalPortBoudary,
							VtnServiceJsonConsts.BOUNDARY_CANDIDATE,
							VtnServiceJsonConsts.YES);
				} else if (IpcDataUnitWrapper.getIpcStructUint8Value(
						valLogicalPortBoundaryStruct,
						VtnServiceIpcConsts.BOUNDARY_CANDIDATE)
						.equalsIgnoreCase(UncPhysicalStructIndexEnum
						.UpplLogicalPortBoundaryCandidate.UPPL_LP_BDRY_CANDIDATE_NO.getValue())) {
					isHasBoundary = true;
					setValueToJsonObject(validBit, logicalPortBoudary,
							VtnServiceJsonConsts.BOUNDARY_CANDIDATE,
							VtnServiceJsonConsts.NO);
				} else {
					LOG.debug("boundary_candidate : invalid");
				}
			}
			validBit = valLogicalPortBoundaryStruct.getByte(
							VtnServiceIpcConsts.VALID,
							UncPhysicalStructIndexEnum.UpplValLogicalPortBoundaryStIndex
							.kIdxLogicalPortBoundaryConnectedDomain.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				isHasBoundary = true;
				setValueToJsonObject(validBit, logicalPortBoudary,
						VtnServiceJsonConsts.DOMAINID,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valLogicalPortBoundaryStruct,
								VtnServiceIpcConsts.CONNECTED_DOMAIN));
			}
			validBit = valLogicalPortBoundaryStruct.getByte(
							VtnServiceIpcConsts.VALID,
							UncPhysicalStructIndexEnum.UpplValLogicalPortBoundaryStIndex
							.kIdxLogicalPortBoundaryConnectedController.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				isHasBoundary = true;
				setValueToJsonObject(validBit, logicalPortBoudary,
						VtnServiceJsonConsts.CONTROLLERID,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valLogicalPortBoundaryStruct,
								VtnServiceIpcConsts.CONNECTED_CONTROLLER));
			}
		}
		LOG.debug("Response Json: " + logicalPortBoudary.toString());
		LOG.trace("Complete getDomainLogicalPortBoundaryResponse");
		if (!isHasBoundary) {
			return null;
		}
		return logicalPortBoudary;
	}

	/**
	 * Used for Show Data Flow response
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return JsonObject
	 */

	public JsonObject getDataFlowResponse(final IpcDataUnit[] responsePacket,
			final JsonObject requestBody, final String getType) {
		LOG.trace("Start getDataFlowPhysicalResponse");
		final JsonObject root = new JsonObject();
		final JsonArray dataFlowArray = new JsonArray();
		if (responsePacket.length != 0) {
			LOG.debug("get Type:" + getType);
			// key type and key structure are not used
			int index = 2;
			// nElements(nElems) parameter for Total number of data flow
			// information
			// contained in this responsePacket
			final int nElements = Integer.parseInt(IpcDataUnitWrapper
					.getIpcDataUnitValue(responsePacket[index++]));
			LOG.debug("nElements:" + nElements);
			for (int i = 0; i < nElements; i++) {
				// getting value of dataflowStruct form response packet
				final IpcStruct valDfDataFlowStruct = (IpcStruct) responsePacket[index++];
				// dataflow JsonObject is outermost Json
				final JsonObject dataflow = new JsonObject();
				byte validBit = 0;

				validBit = valDfDataFlowStruct
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UncValDfDataflowIndex.kidxDfDataFlowReason
										.ordinal());
				validBit = 1;
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					final int reason = Integer.parseInt(IpcDataUnitWrapper
							.getIpcStructUint32Value(valDfDataFlowStruct,
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
					setValueToJsonObject(validBit, dataflow,
							VtnServiceJsonConsts.REASON, reasonForJson);
					LOG.debug("reason:" + reasonForJson);
				}
				LOG.debug("validBit for reason:" + validBit);

				final JsonArray controlFlowArray = new JsonArray();
				validBit = valDfDataFlowStruct
						.getByte(
								VtnServiceIpcConsts.VALID,
								UncPhysicalStructIndexEnum.UncValDfDataflowIndex.kidxDfDataFlowControllerCount
										.ordinal());
				validBit = 1;
				if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
						.ordinal()
						&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
								.ordinal()) {
					// control count for getting number of controller element in
					// controllerdataflowJson
					final int controllerCount = Integer
							.parseInt(IpcDataUnitWrapper
									.getIpcStructUint32Value(
											valDfDataFlowStruct,
											VtnServiceIpcConsts.CONTROLLER_COUNT));
					LOG.debug("Controller Count:" + controllerCount);
					for (int j = 0; j < controllerCount; j++) {
						final AtomicInteger a = new AtomicInteger(index);
						controlFlowArray.add(getControllerDataFlow(
								responsePacket, a, requestBody));
						index = a.get();
					}
					// adding controller flow json array to dataflow Json
					dataflow.add(VtnServiceJsonConsts.CONTROLLER_DATAFLOWS,
							controlFlowArray);
					LOG.debug("dataflow Json:" + dataflow);
				}
				// adding dataflow json to dataflowJson array
				dataFlowArray.add(dataflow);
				LOG.debug("dataFlowArray Json:" + dataFlowArray);
			}
		}
		root.add(VtnServiceJsonConsts.DATAFLOWS, dataFlowArray);
		LOG.debug("root Json :" + root);
		LOG.trace("Complete getDataFlowPhysicalResponse");
		return root;
	}

	/**
	 * Used for Show Controller Data Flow response
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param getType
	 * @return JsonObject
	 */
	public JsonObject getControllerDataFlowResponse(
			final IpcDataUnit[] responsePacket, final JsonObject requestBody,
			final String getType) {
		LOG.trace("Start ControllergetDataFlowPhysicalResponse");
		final JsonObject root = new JsonObject();
		final JsonArray dataFlowArray = new JsonArray();
		if (responsePacket.length != 0) {
			// key type and key structure is not used
			int index = 2;

			final AtomicInteger a = new AtomicInteger(index);
			dataFlowArray.add(getControllerDataFlow(responsePacket, a,
					requestBody));
			index = a.get();
		}
		root.add(VtnServiceJsonConsts.DATAFLOWS, dataFlowArray);
		LOG.debug("root Json :" + root);
		LOG.trace("Complete getControllerDataFlowPhysicalResponse");
		return root;
	}

	private JsonObject getControllerDataFlow(
			final IpcDataUnit[] responsePacket, final AtomicInteger index,
			final JsonObject requestBody) {
		LOG.trace("getControllerDataFlow started");
		byte validBit;
		final JsonObject controlerFlow = new JsonObject();
		JsonObject statisticJson = null;
		// used as an indicator for detail information i.e if statistic is set
		// in detail optype
		boolean isStatisticJson = false;
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}
		// if optype is detail then adding statistic information to the
		// jsonObject
		if (opType.equals(VtnServiceIpcConsts.DETAIL)) {
			final IpcStruct valDfDataFlowSt = (IpcStruct) responsePacket[index
					.getAndIncrement()];
			statisticJson = new JsonObject();
			/*
			 * isStatisticJson is and indicator used for adding
			 * StatisticsJsonObject at last as per given response
			 */
			isStatisticJson = true;
			validBit = valDfDataFlowSt
					.getByte(
							VtnServiceIpcConsts.VALID,
							UncPhysicalStructIndexEnum.UncValDfDataflowStIndex.kidxDfDataFlowStPackets
									.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, statisticJson,
						VtnServiceJsonConsts.PACKETS,
						IpcDataUnitWrapper.getIpcStructInt64Value(
								valDfDataFlowSt, VtnServiceIpcConsts.PACKETS));
			}
			validBit = valDfDataFlowSt
					.getByte(
							VtnServiceIpcConsts.VALID,
							UncPhysicalStructIndexEnum.UncValDfDataflowStIndex.kidxDfDataFlowStOctets
									.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, statisticJson,
						VtnServiceJsonConsts.OCTETS,
						IpcDataUnitWrapper.getIpcStructInt64Value(
								valDfDataFlowSt, VtnServiceIpcConsts.OCTETS));
			}

			validBit = valDfDataFlowSt
					.getByte(
							VtnServiceIpcConsts.VALID,
							UncPhysicalStructIndexEnum.UncValDfDataflowStIndex.kidxDfDataFlowStDuration
									.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, statisticJson,
						VtnServiceJsonConsts.DURATION,
						IpcDataUnitWrapper.getIpcStructUint32Value(
								valDfDataFlowSt, VtnServiceIpcConsts.DURATION));
			}
		}

		final IpcStruct valDfDataFlowCmnStruct = (IpcStruct) responsePacket[index
				.getAndIncrement()];
		validBit = valDfDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.UncValDfDataflowCmnIndex.kidxDfDataFlowControllerName
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controlerFlow,
					VtnServiceJsonConsts.CONTROLLERID,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valDfDataFlowCmnStruct,
							VtnServiceIpcConsts.CONTROLLER_NAME));
		}
		LOG.debug("set valid Bit for Controller name:" + validBit);

		validBit = valDfDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.UncValDfDataflowCmnIndex.kidxDfDataFlowControllerType
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			/*
			 * controllertype is used to get values in ordinal form of
			 * controller bypass,vnp and pfc as 0,1,2 respectively
			 */
			final int controllerType = Integer.parseInt(IpcDataUnitWrapper
					.getIpcStructUint8Value(valDfDataFlowCmnStruct,
							VtnServiceIpcConsts.CONTROLER_TYPE));

			if (controllerType == UncPhysicalStructIndexEnum.UpplTypeIndex.UNC_CT_UNKNOWN
					.ordinal()) {
				setValueToJsonObject(validBit, controlerFlow,
						VtnServiceJsonConsts.CONTROLER_TYPE,
						VtnServiceJsonConsts.BYPASS);
			} else if (controllerType == UncPhysicalStructIndexEnum.UpplTypeIndex.UNC_CT_VNP
					.ordinal()) {
				setValueToJsonObject(validBit, controlerFlow,
						VtnServiceJsonConsts.CONTROLER_TYPE,
						VtnServiceJsonConsts.VNP);
			} else if (controllerType == UncPhysicalStructIndexEnum.UpplTypeIndex.UNC_CT_PFC
					.ordinal()) {
				setValueToJsonObject(validBit, controlerFlow,
						VtnServiceJsonConsts.CONTROLER_TYPE,
						VtnServiceJsonConsts.PFC);
			} else if (controllerType == UncPhysicalStructIndexEnum.UpplTypeIndex.UNC_CT_POLC
					.ordinal()) {
				String polc = VtnServiceInitManager.getConfigurationMap()
						.getCommonConfigValue(VtnServiceConsts.CONF_FILE_FIELD_POLC);
				setValueToJsonObject(validBit, controlerFlow,
						VtnServiceJsonConsts.CONTROLER_TYPE,
						polc);
			} else if (controllerType == UncPhysicalStructIndexEnum.UpplTypeIndex.UNC_CT_HPVANC
					.ordinal()) {
				String hpctr = VtnServiceInitManager.getConfigurationMap()
						.getCommonConfigValue(VtnServiceConsts.CONF_FILE_FIELD_HPVANC);
				setValueToJsonObject(validBit, controlerFlow,
						VtnServiceJsonConsts.CONTROLER_TYPE,
						hpctr);
			} else if (controllerType == UncPhysicalStructIndexEnum.UpplTypeIndex.UNC_CT_ODC
					.ordinal()) {
				String odc = VtnServiceInitManager.getConfigurationMap()
						.getCommonConfigValue(VtnServiceConsts.CONF_FILE_FIELD_ODC);
				setValueToJsonObject(validBit, controlerFlow,
						VtnServiceJsonConsts.CONTROLER_TYPE,
						odc);
			} else {
				LOG.info("Controller Type invalid");
			}
		}
		LOG.debug("set valid Bit for Controller type :" + validBit);

		validBit = valDfDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.UncValDfDataflowCmnIndex.kidxDfDataFlowFlowId
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controlerFlow,
					VtnServiceJsonConsts.FLOW_ID,
					IpcDataUnitWrapper
							.getIpcStructUint64Value(valDfDataFlowCmnStruct,
									VtnServiceIpcConsts.FLOW_ID));
		}
		LOG.debug("set valid Bit for Flow id :" + validBit);

		validBit = valDfDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.UncValDfDataflowCmnIndex.kidxDfDataFlowStatus
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			final int status = Integer.parseInt(IpcDataUnitWrapper
					.getIpcStructUint32Value(valDfDataFlowCmnStruct,
							VtnServiceIpcConsts.STATUS));
			// getting type of reason field and assigning reason value to
			// reasonForjson string
			String statusForJson = VtnServiceConsts.EMPTY_STRING;
			if (status == UncPhysicalStructIndexEnum.UncDataflowStatus.UNC_DF_STAT_INIT
					.ordinal()) {
				statusForJson = VtnServiceJsonConsts.STATUS_INIT;
			} else if (status == UncPhysicalStructIndexEnum.UncDataflowStatus.UNC_DF_STAT_ACTIVATING
					.ordinal()) {
				statusForJson = VtnServiceJsonConsts.STATUS_ACTIVATING;
			} else if (status == UncPhysicalStructIndexEnum.UncDataflowStatus.UNC_DF_STAT_ACTIVE
					.ordinal()) {
				statusForJson = VtnServiceJsonConsts.STATUS_ACTIVE;
			} else if (status == UncPhysicalStructIndexEnum.UncDataflowStatus.UNC_DF_STAT_SWITCHING
					.ordinal()) {
				statusForJson = VtnServiceJsonConsts.STATUS_SWITCHING;
			}

			// assigning reason field in dataflow Json
			setValueToJsonObject(validBit, controlerFlow,
					VtnServiceJsonConsts.STATUS, statusForJson);
			LOG.debug("set Json reason and  validBit:" + validBit);
		}
		LOG.debug("set valid Bit for status :" + validBit);

		validBit = valDfDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.UncValDfDataflowCmnIndex.kidxDfDataFlowFlowType
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			if (IpcDataUnitWrapper.getIpcStructUint32Value(
					valDfDataFlowCmnStruct, VtnServiceIpcConsts.FLOW_TYPE)
					.equals(VtnServiceConsts.ZERO)) {
				setValueToJsonObject(validBit, controlerFlow,
						VtnServiceJsonConsts.TYPE, VtnServiceJsonConsts.VTN);
			}
		}
		LOG.debug("set valid Bit for flow type :" + validBit);

		validBit = valDfDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.UncValDfDataflowCmnIndex.kidxDfDataFlowPolicyIndex
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controlerFlow,
					VtnServiceJsonConsts.POLICY_INDEX,
					IpcDataUnitWrapper.getIpcStructUint32Value(
							valDfDataFlowCmnStruct,
							VtnServiceIpcConsts.POLICY_INDEX));
		}
		LOG.debug("set validBit for policy index :" + validBit);

		validBit = valDfDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.UncValDfDataflowCmnIndex.kidxDfDataFlowVtnId
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controlerFlow,
					VtnServiceJsonConsts.VTN_ID,
					IpcDataUnitWrapper.getIpcStructUint32Value(
							valDfDataFlowCmnStruct, VtnServiceIpcConsts.VTN_ID));
		}
		LOG.debug("set validBit for  vtn id   :" + validBit);

		validBit = valDfDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.UncValDfDataflowCmnIndex.kidxDfDataFlowIngressSwitchId
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controlerFlow,
					VtnServiceJsonConsts.INGRESS_SWITCH_ID,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valDfDataFlowCmnStruct,
							VtnServiceIpcConsts.INGRESS_SWITCH_ID));
		}
		LOG.debug("set validBit for ingress switch id:" + validBit);

		validBit = valDfDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.UncValDfDataflowCmnIndex.kidxDfDataFlowInPort
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controlerFlow,
					VtnServiceJsonConsts.INGRESS_PORT_NAME,
					IpcDataUnitWrapper
							.getIpcStructUint8ArrayValue(
									valDfDataFlowCmnStruct,
									VtnServiceIpcConsts.IN_PORT));
		}
		LOG.debug("set validBit for in_port_name  :" + validBit);

		validBit = valDfDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.UncValDfDataflowCmnIndex.kidxDfDataFlowInStationId
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controlerFlow,
					VtnServiceJsonConsts.INGRESS_STATION_ID,
					IpcDataUnitWrapper.getIpcStructUint64Value(
							valDfDataFlowCmnStruct,
							VtnServiceIpcConsts.IN_STATION_ID));
		}
		LOG.debug("set validBit for in_station_id :" + validBit);

		validBit = valDfDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.UncValDfDataflowCmnIndex.kidxDfDataFlowInDomain
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controlerFlow,
					VtnServiceJsonConsts.INGRESS_DOMAIN_ID,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valDfDataFlowCmnStruct,
							VtnServiceIpcConsts.IN_DOMAIN));
		}
		LOG.trace("set validBit for in_domain_id :" + validBit);

		validBit = valDfDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.UncValDfDataflowCmnIndex.kidxDfDataFlowEgressSwitchId
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controlerFlow,
					VtnServiceJsonConsts.EGRESS_SWITCH_ID,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valDfDataFlowCmnStruct,
							VtnServiceIpcConsts.EGRESS_SWITCH_ID));
		}
		LOG.debug("set validBit for egress_switch_id :" + validBit);

		validBit = valDfDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.UncValDfDataflowCmnIndex.kidxDfDataFlowOutPort
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controlerFlow,
					VtnServiceJsonConsts.EGRESS_PORT_NAME,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valDfDataFlowCmnStruct,
							VtnServiceIpcConsts.OUT_PORT));
		}
		LOG.debug("set validBit for out_port_name:" + validBit);

		validBit = valDfDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.UncValDfDataflowCmnIndex.kidxDfDataFlowOutStationId
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controlerFlow,
					VtnServiceJsonConsts.EGRESS_STATION_ID,
					IpcDataUnitWrapper.getIpcStructUint64Value(
							valDfDataFlowCmnStruct,
							VtnServiceIpcConsts.OUT_STATION_ID));
		}
		LOG.debug("set validBit for out_station_id :" + validBit);

		validBit = valDfDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.UncValDfDataflowCmnIndex.kidxDfDataFlowOutDomain
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			setValueToJsonObject(validBit, controlerFlow,
					VtnServiceJsonConsts.EGRESS_DOMAIN_ID,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							valDfDataFlowCmnStruct,
							VtnServiceIpcConsts.OUT_DOMAIN));
		}
		LOG.debug("set validBit for in_domain_id :" + validBit);

		validBit = valDfDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.UncValDfDataflowCmnIndex.kidxDfDataFlowMatchCount
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			controlerFlow.add(
					VtnServiceJsonConsts.MATCH,
					getDataFlowMatchInfo(responsePacket, index, validBit,
							controlerFlow, valDfDataFlowCmnStruct));
			LOG.debug("controller flow Json:" + controlerFlow);
		}

		validBit = valDfDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.UncValDfDataflowCmnIndex.kidxDfDataFlowActionCount
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {
			controlerFlow.add(
					VtnServiceJsonConsts.ACTION,
					getDataFlowActionInfo(responsePacket, index, controlerFlow,
							valDfDataFlowCmnStruct));
			LOG.debug("controller flow Json:" + controlerFlow);
		}

		validBit = valDfDataFlowCmnStruct
				.getByte(
						VtnServiceIpcConsts.VALID,
						UncPhysicalStructIndexEnum.UncValDfDataflowCmnIndex.kidxDfDataFlowPathInfoCount
								.ordinal());
		if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
				.ordinal()
				&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
						.ordinal()) {

			controlerFlow.add(
					VtnServiceJsonConsts.PATHINFOS,
					getDataFlowPathInfo(responsePacket, index,
							valDfDataFlowCmnStruct));
			LOG.debug("controller flow Json:" + controlerFlow);

		}

		if (isStatisticJson == true) {
			controlerFlow.add(VtnServiceJsonConsts.STATISTICS, statisticJson);
		}

		LOG.trace("getControllerDataFlow completed");
		return controlerFlow;
	}

	private JsonArray getDataFlowPathInfo(final IpcDataUnit[] responsePacket,
			final AtomicInteger index, final IpcStruct valDfDataFlowCmnStruct) {
		LOG.trace("getDataFlowPathInfo stated");
		final JsonArray pathinfoArray = new JsonArray();
		// used to get pathinfo count from val_df_dataflow_cmn struct
		final int pathInfoCount = Integer.parseInt(IpcDataUnitWrapper
				.getIpcStructUint32Value(valDfDataFlowCmnStruct,
						VtnServiceIpcConsts.PATH_INFO_COUNT));
		LOG.debug("path_info_count:" + pathInfoCount);
		for (int k = 0; k < pathInfoCount; k++) {
			final JsonObject pathinfo = new JsonObject();
			byte validBit;
			final IpcStruct valDfDataFlowPathInfo = (IpcStruct) responsePacket[index
					.getAndIncrement()];

			validBit = valDfDataFlowPathInfo
					.getByte(
							VtnServiceIpcConsts.VALID,
							UncPhysicalStructIndexEnum.UncValDfDataflowPathInfoIndex.kidxDfDataFlowPathInfoSwitchId
									.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, pathinfo,
						VtnServiceJsonConsts.SWITCHEID,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valDfDataFlowPathInfo,
								VtnServiceIpcConsts.SWITCHEID));
				LOG.trace("set switchid and  validBit:" + validBit);
			}

			validBit = valDfDataFlowPathInfo
					.getByte(
							VtnServiceIpcConsts.VALID,
							UncPhysicalStructIndexEnum.UncValDfDataflowPathInfoIndex.kidxDfDataFlowPathInfoInPort
									.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, pathinfo,
						VtnServiceJsonConsts.IN_PORT_NAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valDfDataFlowPathInfo,
								VtnServiceIpcConsts.IN_PORT));
				LOG.trace("set in_port and  validBit:" + validBit);
			}

			validBit = valDfDataFlowPathInfo
					.getByte(
							VtnServiceIpcConsts.VALID,
							UncPhysicalStructIndexEnum.UncValDfDataflowPathInfoIndex.kidxDfDataFlowPathInfoOutPort
									.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, pathinfo,
						VtnServiceJsonConsts.OUT_PORT_NAME,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								valDfDataFlowPathInfo,
								VtnServiceIpcConsts.OUT_PORT));
				LOG.debug("set validBit for out_port :" + validBit);
			}
			pathinfoArray.add(pathinfo);
		}
		LOG.debug("pathinfoArray Json:" + pathinfoArray);
		LOG.trace("getDataFlowPathInfo completed");
		return pathinfoArray;
	}

	private JsonObject getDataFlowActionInfo(
			final IpcDataUnit[] responsePacket, final AtomicInteger index,
			final JsonObject controlerFlow,
			final IpcStruct valDfDataFlowCmnStruct) {
		final int actionCount = Integer.parseInt(IpcDataUnitWrapper
				.getIpcStructUint32Value(valDfDataFlowCmnStruct,
						VtnServiceIpcConsts.ACTION_COUNT));
		LOG.debug("ACTION_COUNT:" + actionCount);

		// jsonObject action will holf all below json instance
		final JsonObject action = new JsonObject();
		// jsonObject objects are created only when required information from
		// response packet
		// here we have just used refernces
		JsonArray outputPortJsonArray = null;
		JsonArray enqueuePortJsonArray = null;
		JsonArray queueIdJsonArray = null;
		JsonArray setMacDstAddrJsonArray = null;
		JsonArray setMacSrcAddrJsonArray = null;
		JsonArray setVlanIdJsonArray = null;
		JsonArray setVlanPriorityJsonArray = null;
		JsonArray setIpDstAddrJsonArray = null;
		JsonArray setIpSrcAddrJsonArray = null;
		JsonArray setIpTosJsonArray = null;
		JsonArray setL4DstPortIcmpTypeJsonArray = null;
		JsonArray setL4SrcPortIcmpTypeJsonArray = null;
		JsonArray setIpV6DstAddrJsonArray = null;
		JsonArray setIpv6SrcAddrJsonArray = null;
		JsonArray setStripVlanJsonArray = null;
		JsonPrimitive element = null;
		for (int i = 0; i < actionCount; i++) {
			final IpcStruct valDfFlowAction = (IpcStruct) responsePacket[index
					.getAndIncrement()];

			LOG.trace("getDataFlowActionInfo started");
			int actionType;
			// actiontype will help in resolving action as per response
			actionType = Integer.parseInt(IpcDataUnitWrapper
					.getIpcStructUint32Value(valDfFlowAction,
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
				LOG.debug("set  out_port  ");

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
				LOG.debug("set  enqueueport  ");

				element = new JsonPrimitive(IpcDataUnitWrapper
						.getIpcStructUint16Value(valDfFlowActionEnqueuePort,
								VtnServiceIpcConsts.ENQUEUE_ID).toString());
				if (null == queueIdJsonArray) {
					queueIdJsonArray = new JsonArray();
				}
				queueIdJsonArray.add(element);
				LOG.debug("set  enqueueid  ");

			} else if (actionType == UncStructIndexEnum.UncDataflowFlowActionType.UNC_ACTION_SET_DL_DST
					.ordinal()) {
				final IpcStruct valDfFlowActionSetDlAddr = (IpcStruct) responsePacket[index
						.getAndIncrement()];
				element = new JsonPrimitive(IpcDataUnitWrapper.getMacAddress(
						valDfFlowActionSetDlAddr, VtnServiceIpcConsts.DL_ADDR)
						.toString());

				if (null == setMacDstAddrJsonArray) {
					setMacDstAddrJsonArray = new JsonArray();
				}
				setMacDstAddrJsonArray.add(element);
				LOG.debug("set macdst  ");

			} else if (actionType == UncStructIndexEnum.UncDataflowFlowActionType.UNC_ACTION_SET_DL_SRC
					.ordinal()) {
				final IpcStruct valDfFlowActionSetDlAddr = (IpcStruct) responsePacket[index
						.getAndIncrement()];
				element = new JsonPrimitive(IpcDataUnitWrapper.getMacAddress(
						valDfFlowActionSetDlAddr, VtnServiceIpcConsts.DL_ADDR)
						.toString());
				if (null == setMacSrcAddrJsonArray) {
					setMacSrcAddrJsonArray = new JsonArray();
				}
				setMacSrcAddrJsonArray.add(element);
				LOG.debug("set macsrc  ");

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

				LOG.debug("set vlan_id ");

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
				LOG.debug("set  vlanpriority   :");

			} else if (actionType == UncStructIndexEnum.UncDataflowFlowActionType.UNC_ACTION_STRIP_VLAN
					.ordinal()) {
				element = new JsonPrimitive(VtnServiceJsonConsts.TRUE);
				if (null == setStripVlanJsonArray) {
					setStripVlanJsonArray = new JsonArray();
				}
				setStripVlanJsonArray.add(element);
				index.getAndIncrement();

			} else if (actionType == UncStructIndexEnum.UncDataflowFlowActionType.UNC_ACTION_SET_IPV4_SRC
					.ordinal()) {
				final IpcStruct valDfFlowActionSetIpv4 = (IpcStruct) responsePacket[index
						.getAndIncrement()];
				element = new JsonPrimitive(IpcDataUnitWrapper
						.getIpcStructIpv4Value(valDfFlowActionSetIpv4,
								VtnServiceIpcConsts.IPV4_ADDR).toString());
				if (null == setIpSrcAddrJsonArray) {
					setIpSrcAddrJsonArray = new JsonArray();
				}
				setIpSrcAddrJsonArray.add(element);
				LOG.debug("set ipsrc :");

			} else if (actionType == UncStructIndexEnum.UncDataflowFlowActionType.UNC_ACTION_SET_IPV4_DST
					.ordinal()) {
				final IpcStruct valDfFlowActionSetIpv4 = (IpcStruct) responsePacket[index
						.getAndIncrement()];
				element = new JsonPrimitive(IpcDataUnitWrapper
						.getIpcStructIpv4Value(valDfFlowActionSetIpv4,
								VtnServiceIpcConsts.IPV4_ADDR).toString());
				if (null == setIpDstAddrJsonArray) {
					setIpDstAddrJsonArray = new JsonArray();
				}
				setIpDstAddrJsonArray.add(element);
				LOG.debug("set ipdst ");

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
				LOG.debug("set r iptos ");

			} else if (actionType == UncStructIndexEnum.UncDataflowFlowActionType.UNC_ACTION_SET_TP_SRC
					.ordinal()) {
				final IpcStruct valDfFlowActionSetTpPort = (IpcStruct) responsePacket[index
						.getAndIncrement()];

				element = new JsonPrimitive(IpcDataUnitWrapper
						.getIpcStructUint16Value(valDfFlowActionSetTpPort,
								VtnServiceIpcConsts.TP_PORT).toString());
				if (null == setL4SrcPortIcmpTypeJsonArray) {
					setL4SrcPortIcmpTypeJsonArray = new JsonArray();
				}
				setL4SrcPortIcmpTypeJsonArray.add(element);
				LOG.debug("set  tpsrc ");

			} else if (actionType == UncStructIndexEnum.UncDataflowFlowActionType.UNC_ACTION_SET_TP_DST
					.ordinal()) {
				final IpcStruct valDfFlowActionSetTpPort = (IpcStruct) responsePacket[index
						.getAndIncrement()];

				element = new JsonPrimitive(IpcDataUnitWrapper
						.getIpcStructUint16Value(valDfFlowActionSetTpPort,
								VtnServiceIpcConsts.TP_PORT).toString());
				if (null == setL4DstPortIcmpTypeJsonArray) {
					setL4DstPortIcmpTypeJsonArray = new JsonArray();
				}
				setL4DstPortIcmpTypeJsonArray.add(element);
				LOG.debug("set    tpdst  ");

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
				LOG.debug("set   ipv6src  ");

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
				LOG.debug("set validBit for ipv6dst");
			}
			if (outputPortJsonArray != null) {
				action.add(VtnServiceJsonConsts.OUTPUTPORT, outputPortJsonArray);
			}
			if (enqueuePortJsonArray != null) {
				action.add(VtnServiceJsonConsts.ENQUEUEPORT,
						enqueuePortJsonArray);
			}
			if (queueIdJsonArray != null) {
				action.add(VtnServiceJsonConsts.QUEUE_ID, queueIdJsonArray);
			}
			if (setMacDstAddrJsonArray != null) {
				action.add(VtnServiceJsonConsts.SETMACDSTADDR,
						setMacDstAddrJsonArray);
			}
			if (setMacSrcAddrJsonArray != null) {
				action.add(VtnServiceJsonConsts.SETMACSRCADDR,
						setMacSrcAddrJsonArray);
			}
			if (setVlanIdJsonArray != null) {
				action.add(VtnServiceJsonConsts.SETVLAN_ID, setVlanIdJsonArray);
			}
			if (setVlanPriorityJsonArray != null) {
				action.add(VtnServiceJsonConsts.SETVLAN_PRIORITY,
						setVlanPriorityJsonArray);
			}
			if (setStripVlanJsonArray != null) {
				action.add(VtnServiceJsonConsts.STRIPVLAN,
						setStripVlanJsonArray);
			}
			if (setIpSrcAddrJsonArray != null) {
				action.add(VtnServiceJsonConsts.SETIPSRCADDR,
						setIpSrcAddrJsonArray);
			}
			if (setIpDstAddrJsonArray != null) {
				action.add(VtnServiceJsonConsts.SETIPDSTADDR,
						setIpDstAddrJsonArray);
			}
			if (setIpTosJsonArray != null) {
				action.add(VtnServiceJsonConsts.SETIPTOS, setIpTosJsonArray);
			}
			if (setL4SrcPortIcmpTypeJsonArray != null) {
				action.add(VtnServiceJsonConsts.SETL4SRCPORT_ICMPTYPE,
						setL4SrcPortIcmpTypeJsonArray);
			}
			if (setL4DstPortIcmpTypeJsonArray != null) {
				action.add(VtnServiceJsonConsts.SETL4DSTPORT_ICMPTYPE,
						setL4DstPortIcmpTypeJsonArray);
			}
			if (setIpv6SrcAddrJsonArray != null) {
				action.add(VtnServiceJsonConsts.SETIPV6SRCADDR,
						setIpv6SrcAddrJsonArray);
			}
			if (setIpV6DstAddrJsonArray != null) {
				action.add(VtnServiceJsonConsts.SETIPV6DSTADDR,
						setIpV6DstAddrJsonArray);
			}
		}
		LOG.debug("action json:" + action);
		LOG.trace("getDataFlowActionInfo completed");
		return action;

	}

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
	 * Create response for the controller path policy
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param list
	 * @return
	 */
	public JsonObject getCtrPathPolicyResponse(IpcDataUnit[] responsePacket,
			JsonObject requestBody, String getType) {

		LOG.info("Start getCtrPathPolicyResponse");
		final JsonObject root = new JsonObject();
		JsonArray pathPolicies = null;
		JsonObject pathPolicyObj = new JsonObject();
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
			rootJsonName = VtnServiceJsonConsts.PATHPOLICY;
		} else {
			rootJsonName = VtnServiceJsonConsts.PATHPOLICIES;
			// json array will be required for list type of cases
			pathPolicies = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);

		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			// for count case
			pathPolicyObj
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, pathPolicyObj);
		} else {
			for (int index = 0; index < responsePacket.length; index++) {

				// There is no use of key type
				LOG.debug("Skip key type: no use");
				index++;

				final IpcStruct keyCtrlPpolicyStruct = (IpcStruct) responsePacket[index++];
				pathPolicyObj.addProperty(VtnServiceJsonConsts.POLICYID,
						IpcDataUnitWrapper.getIpcStructUint16Value(
								keyCtrlPpolicyStruct,
								VtnServiceIpcConsts.POLICYID));

				// If pathPolicies is initialized, add object to array
				if (null != pathPolicies) {
					pathPolicies.add(pathPolicyObj);
				}
			}
			/*
			 * finally add either array or single object to root JSON object and
			 * return the same.
			 */
			if (null != pathPolicies) {
				root.add(rootJsonName, pathPolicies);
			} else {
				root.add(rootJsonName, pathPolicyObj);
			}
		}
		LOG.debug("Response Json: " + root.toString());
		LOG.trace("Complete getCtrPathPolicyResponse");
		return root;
	}

	/**
	 * Create response for the controller path policy Link weight
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param list
	 * @return
	 */
	public JsonArray getCtrPathPolicyLinkResponse(IpcDataUnit[] responsePacket,
			JsonObject requestBody, String getType) {
		LOG.trace("Start getCtrPathPolicyLinkResponse");
		JsonArray linkWeights = new JsonArray();
		for (int index = 0; index < responsePacket.length; index++) {
			JsonObject linkWeightObj = new JsonObject();
			// There is no use of key type
			LOG.debug("Skip key type: no use");
			index++;

			final IpcStruct keyCtrlPpolicyLinkStruct = (IpcStruct) responsePacket[index++];
			linkWeightObj.addProperty(VtnServiceJsonConsts.SWITCH_ID,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							keyCtrlPpolicyLinkStruct,
							VtnServiceIpcConsts.SWITCHID));
			linkWeightObj.addProperty(VtnServiceJsonConsts.PORTNAME,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							keyCtrlPpolicyLinkStruct,
							VtnServiceIpcConsts.PORT_ID));

			IpcStruct valLinkWeightStruct = (IpcStruct) responsePacket[index++];
			byte validBit = valLinkWeightStruct
					.getByte(
							VtnServiceIpcConsts.VALID,
							UncPhysicalStructIndexEnum.UpplValCtrPpolicyLinkWeightIndex.kIdxLinkWeight
									.ordinal());
			if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
					.ordinal()
					&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
							.ordinal()) {
				setValueToJsonObject(validBit, linkWeightObj,
						VtnServiceJsonConsts.WEIGHT,
						IpcDataUnitWrapper
								.getIpcStructUint32Value(valLinkWeightStruct,
										VtnServiceIpcConsts.WEIGHT));
			}
			// Adding JSON element to the array
			linkWeights.add(linkWeightObj);
		}
		LOG.debug("Response Json: " + linkWeights.toString());
		LOG.trace("Complete getCtrPathPolicyLinkResponse");
		return linkWeights;
	}

	/**
	 * Create response for the controller path policy disable switches
	 * 
	 * @param responsePacket
	 * @param requestBody
	 * @param list
	 * @return
	 */
	public JsonArray getCtrPathPolicyDisableResponse(
			IpcDataUnit[] responsePacket, JsonObject requestBody, String list) {
		LOG.info("Start getCtrPathPolicyDisableResponse");
		JsonArray disableSwitches = new JsonArray();
		for (int index = 0; index < responsePacket.length; index++) {
			JsonObject disableSwitch = new JsonObject();
			// There is no use of key type
			LOG.debug("Skip key type: no use");
			index++;

			final IpcStruct keyCtrlPpolicyDisableStruct = (IpcStruct) responsePacket[index++];
			disableSwitch.addProperty(VtnServiceJsonConsts.SWITCH_ID,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							keyCtrlPpolicyDisableStruct,
							VtnServiceIpcConsts.SWITCHID));
			disableSwitch.addProperty(VtnServiceJsonConsts.PORTNAME,
					IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
							keyCtrlPpolicyDisableStruct,
							VtnServiceIpcConsts.PORT_ID));

			// Adding JSON element to the array
			disableSwitches.add(disableSwitch);
		}
		LOG.debug("Response Json: " + disableSwitches.toString());
		LOG.trace("Complete getCtrPathPolicyDisableResponse");
		return disableSwitches;
	}

}
