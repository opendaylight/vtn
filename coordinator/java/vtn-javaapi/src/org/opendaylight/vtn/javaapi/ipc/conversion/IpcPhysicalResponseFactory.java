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
import org.opendaylight.vtn.core.ipc.IpcDataUnit;
import org.opendaylight.vtn.core.ipc.IpcStruct;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceIpcConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
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
			} else {
				LOG.debug("Type : invalid");
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
			final JsonObject requestBody, final String getType) {
		LOG.trace("Start getSwitchPortResponse");
		LOG.debug("getType: " + getType);
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}
		String rootJsonName = null;
		final JsonObject root = new JsonObject();
		JsonObject switchPort = null;
		JsonArray switchPortArray = null;
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.PORT;
		} else {
			rootJsonName = VtnServiceJsonConsts.PORTS;
			switchPortArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			switchPort = new JsonObject();
			switchPort
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
			root.add(rootJsonName, switchPort);
		} else {
			IpcStruct valPortStruct = null;
			IpcStruct valPortStStruct = null;
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

				if (!opType.equalsIgnoreCase(VtnServiceJsonConsts.NORMAL)
						|| getType.equalsIgnoreCase(VtnServiceJsonConsts.SHOW)) {
					LOG.debug("Case : not Normal or Show");
					/*
					 * this part is always required in Show, but not required in
					 * List + "normal" op type
					 */
					LOG.debug("targetdb : State");
					valPortStStruct = (IpcStruct) responsePacket[index++];

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
								+ IpcDataUnitWrapper.getIpcStructUint8Value(
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
								+ IpcDataUnitWrapper.getIpcStructUint8Value(
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
						setValueToJsonObject(validBit, switchPort,
								VtnServiceJsonConsts.ALARMSSTATUS,
								IpcDataUnitWrapper.getIpcStructUint64HexaValue(
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
								+ IpcDataUnitWrapper.getIpcStructUint8Value(
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
						setValueToJsonObject(validBit, switchPort,
								VtnServiceJsonConsts.LOGICAL_PORT_ID,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
										valPortStStruct,
										VtnServiceIpcConsts.LOGICAL_PORT_ID));
					}
					// getting inner structure from St struct by checking valid
					// bit.
					validBit = valPortStStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncPhysicalStructIndexEnum.UpplValPortStIndex.kIdxPortSt
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						valPortStruct = IpcDataUnitWrapper.getInnerIpcStruct(
								valPortStStruct, VtnServiceIpcConsts.PORT);
						validBit = valPortStruct
								.getByte(
										VtnServiceIpcConsts.VALID,
										UncPhysicalStructIndexEnum.UpplValPortIndex.kIdxPortNumber
												.ordinal());
						if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
								.ordinal()
								&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
										.ordinal()) {
							setValueToJsonObject(validBit, switchPort,
									VtnServiceJsonConsts.PORT_ID,
									IpcDataUnitWrapper.getIpcStructUint32Value(
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
									.getIpcStructUint8Value(valPortStruct,
											VtnServiceJsonConsts.ADMIN_STATUS)
									.equalsIgnoreCase(
											UncPhysicalStructIndexEnum.UpplPortAdminStatus.UPPL_PORT_ADMIN_UP
													.getValue())) {
								setValueToJsonObject(validBit, switchPort,
										VtnServiceJsonConsts.ADMINSTATUS,
										VtnServiceJsonConsts.UP);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valPortStruct,
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
				} else {
					index++;
				}
				if (null != switchPortArray) {
					switchPortArray.add(switchPort);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != switchPortArray) {
				root.add(rootJsonName, switchPortArray);
			} else {
				root.add(rootJsonName, switchPort);
			}
		}
		LOG.debug("Response Json: " + root.toString());
		LOG.trace("Complete getSwitchPortResponse");
		return root;
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
		LOG.trace("Start getLinkResponse");
		final JsonObject root = new JsonObject();
		JsonArray linksArray = null;
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
		 * here it will be link for show and links for list
		 */
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.LINK;
		} else {
			rootJsonName = VtnServiceJsonConsts.LINKS;
			// json array will be required for list type of cases
			linksArray = new JsonArray();
		}
		LOG.debug("Json Name :" + rootJsonName);
		JsonObject links = null;
		if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			/*
			 * Create Json for Count
			 */
			links = new JsonObject();
			links.addProperty(
					VtnServiceJsonConsts.COUNT,
					IpcDataUnitWrapper
							.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
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
				String linkname = "";
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
				if (getType.equals(VtnServiceJsonConsts.SHOW)
						|| opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
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
				// add current json object to array, if it has been initialized
				// earlier
				if (null != linksArray) {
					linksArray.add(links);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != linksArray) {
				root.add(rootJsonName, linksArray);
			} else {
				root.add(rootJsonName, links);
			}
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
					/*
					 * add valid informations from value structure
					 */
					final IpcStruct valSwitchStStruct = (IpcStruct) responsePacket[index++];
					validBit = valSwitchStStruct
							.getByte(
									VtnServiceIpcConsts.VALID,
									UncPhysicalStructIndexEnum.UpplValSwitchStIndex.kIdxSwitch
											.ordinal());
					if (validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_INVALID
							.ordinal()
							&& validBit != (byte) UncStructIndexEnum.Valid.UNC_VF_NOT_SUPPORTED
									.ordinal()) {
						// IpcStruct valSwitchStruct=
						// (IpcStruct)valSwitchStStruct.get(VtnServiceIpcConsts.SWITCH_VAL);
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
							setValueToJsonObject(validBit, switches,
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
									.getIpcStructUint8Value(valSwitchStruct,
											VtnServiceJsonConsts.ADMIN_STATUS)
									.equalsIgnoreCase(
											UncPhysicalStructIndexEnum.UpplSwitchAdminStatus.UPPL_SWITCH_ADMIN_DOWN
													.getValue())) {
								setValueToJsonObject(validBit, switches,
										VtnServiceJsonConsts.ADMINSTATUS,
										VtnServiceJsonConsts.DOWN);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(valSwitchStruct,
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
							setValueToJsonObject(validBit, switches,
									VtnServiceJsonConsts.IPADDR,
									IpcDataUnitWrapper.getIpcStructIpv4Value(
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
							setValueToJsonObject(validBit, switches,
									VtnServiceJsonConsts.IPV6ADDR,
									IpcDataUnitWrapper.getIpcStructIpv6Value(
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
								+ IpcDataUnitWrapper.getIpcStructUint8Value(
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
						setValueToJsonObject(validBit, switches,
								VtnServiceJsonConsts.MANUFACTURER,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
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
						setValueToJsonObject(validBit, switches,
								VtnServiceJsonConsts.HARDWARE,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
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
						setValueToJsonObject(validBit, switches,
								VtnServiceJsonConsts.SOFTWARE,
								IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
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
						setValueToJsonObject(validBit, switches,
								VtnServiceJsonConsts.ALARMSSTATUS,
								IpcDataUnitWrapper.getIpcStructUint64HexaValue(
										valSwitchStStruct,
										VtnServiceIpcConsts.ALARM_STATUS));
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
		LOG.trace("Start getDomainLogicalPortResponse");
		/*
		 * operation type will be required to resolve the response type
		 */
		String opType = VtnServiceJsonConsts.NORMAL;
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
		}
		String rootJsonName;
		JsonArray logicalPortsArray = null;
		final JsonObject root = new JsonObject();
		LOG.debug("getType: " + getType);
		if (getType.equals(VtnServiceJsonConsts.SHOW)) {
			rootJsonName = VtnServiceJsonConsts.LOGICALPORT;
		} else {
			rootJsonName = VtnServiceJsonConsts.LOGICALPORTS;
			// json array will be required for list type of cases
			logicalPortsArray = new JsonArray();
		}
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
			logicalPort
					.addProperty(
							VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper
									.getIpcDataUnitValue(responsePacket[VtnServiceConsts.IPC_COUNT_INDEX]));
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
					if (getType.equals(VtnServiceJsonConsts.SHOW)
							|| opType
									.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
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
										VtnServiceIpcConsts.OPERSTATUS,
										VtnServiceJsonConsts.UP);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(
											valLogicalPortStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equalsIgnoreCase(
											UncPhysicalStructIndexEnum.UpplLogicalPortOperStatus.UPPL_LOGICAL_PORT_OPER_DOWN
													.getValue())) {
								setValueToJsonObject(validBit, logicalPort,
										VtnServiceIpcConsts.OPERSTATUS,
										VtnServiceJsonConsts.DOWN);
							} else if (IpcDataUnitWrapper
									.getIpcStructUint8Value(
											valLogicalPortStStruct,
											VtnServiceIpcConsts.OPERSTATUS)
									.equalsIgnoreCase(
											UncPhysicalStructIndexEnum.UpplLogicalPortOperStatus.UPPL_LOGICAL_PORT_OPER_UNKNOWN
													.getValue())) {
								setValueToJsonObject(validBit, logicalPort,
										VtnServiceIpcConsts.OPERSTATUS,
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
				// add current json object to array, if it has been initialized
				// earlier
				if (null != logicalPortsArray) {
					logicalPortsArray.add(logicalPort);
				}
			}
			/*
			 * finally add either array or single object to root json object and
			 * return the same.
			 */
			if (null != logicalPortsArray) {
				root.add(rootJsonName, logicalPortsArray);
			} else {
				root.add(rootJsonName, logicalPort);
			}
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
}
