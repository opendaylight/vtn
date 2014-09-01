/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.resources.openstack;

import java.sql.Connection;
import java.sql.SQLException;
import java.util.ArrayList;

import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.RestResource;
import org.opendaylight.vtn.javaapi.annotation.UNCField;
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.init.VtnServiceInitManager;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.openstack.beans.FlowFilterVbrBean;
import org.opendaylight.vtn.javaapi.openstack.beans.FlowFilterVrtBean;
import org.opendaylight.vtn.javaapi.openstack.beans.FlowListBean;
import org.opendaylight.vtn.javaapi.openstack.beans.FreeCounterBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VRouterInterfaceBean;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;
import org.opendaylight.vtn.javaapi.openstack.convertor.FlowListResourceGenerator;
import org.opendaylight.vtn.javaapi.openstack.dao.FlowFilterDao;
import org.opendaylight.vtn.javaapi.openstack.dao.FlowListDao;
import org.opendaylight.vtn.javaapi.openstack.dao.VRouterInterfaceDao;
import org.opendaylight.vtn.javaapi.openstack.dbmanager.ResourceIdManager;
import org.opendaylight.vtn.javaapi.openstack.validation.FilterResourceValidator;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;

/**
 * Resource class for handling requests for Filter
 */
@UNCVtnService(path = VtnServiceOpenStackConsts.FILTER_PATH)
public class FilterResource extends AbstractResource {

	/* Logger instance */
	private static final Logger LOG = Logger.getLogger(FilterResource.class
			.getName());

	@UNCField(VtnServiceOpenStackConsts.FILTER_ID)
	private String filterId;

	/**
	 * Getter of filterId
	 * 
	 * @return
	 */
	public String getFilterId() {
		return filterId;
	}

	/**
	 * Constructor that initialize the validation instance for current resource
	 * instance
	 */
	public FilterResource() {
		setValidator(new FilterResourceValidator(this));
	}

	/**
	 * Handler method for GET operation of Filter
	 * 
	 * @see org.opendaylight.vtn.javaapi.resources.AbstractResource#put()
	 */
	@Override
	public int put(JsonObject requestBody) {
		LOG.trace("Start FlowFilter#put()");
		int errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();
		Connection connection = null;
		boolean isCommitRequired = true;
		boolean isRollback = true;
		boolean isApply = requestBody
				.has(VtnServiceOpenStackConsts.APPLY_PORTS);
		boolean isPortList = true;
		try {
			connection = VtnServiceInitManager.getDbConnectionPoolMap()
					.getConnection();
			/*
			 * Check for instances that they exists or not, if not then return
			 * 404 error
			 */
			final FiltersResource filtersResource = new FiltersResource();
			if (checkForNotFoundResources(connection)
					&& null == filtersResource.checkForNotFoundResources(
							connection, requestBody)) {
				if (!requestBody.toString().equals("{}")) {
					String seqnum = Integer.valueOf(
							getFilterId().substring(5, 9), 16).toString();
					final RestResource restResource = new RestResource();
					ArrayList<FlowFilterVrtBean> setVrouterList = new ArrayList<FlowFilterVrtBean>();
					ArrayList<FlowFilterVbrBean> setVbridgeList = new ArrayList<FlowFilterVbrBean>();
					ArrayList<FlowFilterVrtBean> delVrouterList = new ArrayList<FlowFilterVrtBean>();
					ArrayList<FlowFilterVbrBean> delVbridgeList = new ArrayList<FlowFilterVbrBean>();
					final FlowFilterDao flowFilterDao = new FlowFilterDao();
					ArrayList<FlowFilterVrtBean> portIdList = flowFilterDao
							.getInterfaceList(connection, getFilterId());
					if (null == portIdList) {
						isPortList = false;
						portIdList = new ArrayList<FlowFilterVrtBean>();
					}
					JsonArray apply = new JsonArray();

					if (isApply) {
						apply = RemoveSameApply(requestBody
								.getAsJsonArray(VtnServiceOpenStackConsts.APPLY_PORTS));
					}
					if (!isApply) {
						/* if there is no apply_port, do nothing */
					} else if (apply.size() == 0 && isPortList) {
						/* if apply_port is empty, delete all infomation from DB */
						for (int i = 0; i < portIdList.size(); i++) {
							if (null == portIdList.get(i).getVrtName()
									|| portIdList.get(i).getVrtName().isEmpty()) {
								delVbridgeList.add(setFlowFilterVbrList(
										portIdList.get(i).getVtnName(),
										portIdList.get(i).getVbrName(),
										portIdList.get(i).getVrtIfName()));

							} else {
								delVrouterList.add(setFlowFilterVrtList(
										portIdList.get(i).getVtnName(),
										portIdList.get(i).getVrtName(),
										portIdList.get(i).getVbrName(),
										portIdList.get(i).getVrtIfName()));
							}
						}
					} else {
						/* apply_port and DB all has infomation */
						/* make set list for vbr and vrt */
						for (int i = 0; i < apply.size(); i++) {
							boolean flag = false;
							JsonObject jsOb = (JsonObject) apply.get(i);
							for (int j = 0; j < portIdList.size(); j++) {
								if (jsOb.has(VtnServiceOpenStackConsts.ROUTER)) {
									if (jsOb.get(
											VtnServiceOpenStackConsts.TENANT)
											.getAsString()
											.equals(portIdList.get(j)
													.getVtnName())
											&& jsOb.get(
													VtnServiceOpenStackConsts.ROUTER)
													.getAsString()
													.equals(portIdList.get(j)
															.getVrtName())
											&& jsOb.get(
													VtnServiceOpenStackConsts.INTERFACE)
													.getAsString()
													.equals(portIdList.get(j)
															.getVrtIfName()
															.substring(6))) {
										portIdList.remove(j);
										flag = true;
										break;
									}
								} else {
									if (jsOb.get(
											VtnServiceOpenStackConsts.TENANT)
											.getAsString()
											.equals(portIdList.get(j)
													.getVtnName())
											&& jsOb.get(
													VtnServiceOpenStackConsts.NETWORK)
													.getAsString()
													.equals(portIdList.get(j)
															.getVbrName())
											&& jsOb.get(
													VtnServiceOpenStackConsts.PORT)
													.getAsString()
													.equals(portIdList.get(j)
															.getVrtIfName()
															.substring(6))) {
										portIdList.remove(j);
										flag = true;
										break;
									}
								}
							}
							if (flag) {
								continue;
							}

							if (jsOb.has(VtnServiceOpenStackConsts.ROUTER)) {
								final VRouterInterfaceBean vInterfaceBean = new VRouterInterfaceBean();
								vInterfaceBean
										.setVrtIfId(Integer
												.parseInt(jsOb
														.get(VtnServiceOpenStackConsts.INTERFACE)
														.getAsString()));
								vInterfaceBean.setVtnName(jsOb.get(
										VtnServiceOpenStackConsts.TENANT)
										.getAsString());
								vInterfaceBean.setVrtName(jsOb.get(
										VtnServiceOpenStackConsts.ROUTER)
										.getAsString());
								vInterfaceBean
										.setVrtIfName(VtnServiceOpenStackConsts.IF_PREFIX
												+ jsOb.get(
														VtnServiceOpenStackConsts.INTERFACE)
														.getAsString());
								final VRouterInterfaceDao vrouterInterfaceDao = new VRouterInterfaceDao();
								FlowFilterVrtBean addFFVrtBean = setFlowFilterVrtList(
										jsOb.get(
												VtnServiceOpenStackConsts.TENANT)
												.getAsString(),
										jsOb.get(
												VtnServiceOpenStackConsts.ROUTER)
												.getAsString(),
										vrouterInterfaceDao.getVbridgeName(
												connection, vInterfaceBean),
										VtnServiceOpenStackConsts.IF_PREFIX
												+ jsOb.get(
														VtnServiceOpenStackConsts.INTERFACE)
														.getAsString());
								if (!setVrouterList.contains(addFFVrtBean)) {
									setVrouterList.add(addFFVrtBean);
								}

							} else {
								final VRouterInterfaceBean vInterfaceBean = new VRouterInterfaceBean();
								vInterfaceBean.setVtnName(jsOb.get(
										VtnServiceOpenStackConsts.TENANT)
										.getAsString());
								vInterfaceBean.setVbrName(jsOb.get(
										VtnServiceOpenStackConsts.NETWORK)
										.getAsString());
								vInterfaceBean.setVrtIfId(Integer.parseInt(jsOb
										.get(VtnServiceOpenStackConsts.PORT)
										.getAsString()));
								final VRouterInterfaceDao vrouterInterfaceDao = new VRouterInterfaceDao();
								String vrouter_name = vrouterInterfaceDao
										.getVrouterName(connection,
												vInterfaceBean);
								if (null != vrouter_name) {

									FlowFilterVrtBean addFFVrtBean = setFlowFilterVrtList(
											jsOb.get(
													VtnServiceOpenStackConsts.TENANT)
													.getAsString(),
											vrouter_name,
											jsOb.get(
													VtnServiceOpenStackConsts.NETWORK)
													.getAsString(),
											VtnServiceOpenStackConsts.IF_PREFIX
													+ jsOb.get(
															VtnServiceOpenStackConsts.PORT)
															.getAsString());
									if (!setVrouterList.contains(addFFVrtBean)) {
										setVrouterList.add(addFFVrtBean);
									}

								} else {
									FlowFilterVbrBean addFFVbrBean = setFlowFilterVbrList(
											jsOb.get(
													VtnServiceOpenStackConsts.TENANT)
													.getAsString(),
											jsOb.get(
													VtnServiceOpenStackConsts.NETWORK)
													.getAsString(),
											VtnServiceOpenStackConsts.IF_PREFIX
													+ jsOb.get(
															VtnServiceOpenStackConsts.PORT)
															.getAsString());
									if (!setVbridgeList.contains(addFFVbrBean)) {
										setVbridgeList.add(addFFVbrBean);
									}
								}
							}
						}

						/* make del list for vbr and vrt */
						for (int i = 0; i < portIdList.size(); i++) {
							if (null == portIdList.get(i).getVrtName()
									|| portIdList.get(i).getVrtName().isEmpty()) {
								delVbridgeList.add(setFlowFilterVbrList(
										portIdList.get(i).getVtnName(),
										portIdList.get(i).getVbrName(),
										portIdList.get(i).getVrtIfName()));

							} else {
								delVrouterList.add(setFlowFilterVrtList(
										portIdList.get(i).getVtnName(),
										portIdList.get(i).getVrtName(),
										portIdList.get(i).getVbrName(),
										portIdList.get(i).getVrtIfName()));
							}
						}
					}
					/* if (!isApply && !isPortList), set and del list is empty */
					/* insert and del the infomation from DB */
					int status = flowFilterDao.insertVrouterFilterInfo(
							connection, setVrouterList);
					if (1 != status) {
						isCommitRequired = false;
						LOG.error("Resource insertion flowFilterVrt failed at database operation.");
					}
					if (isCommitRequired) {
						status = flowFilterDao.insertVbridgeFilterInfo(
								connection, setVbridgeList);
						if (1 != status) {
							isCommitRequired = false;
							LOG.error("Resource insertion flowFilterVbr failed at database operation.");
						}
					}
					if (isCommitRequired) {
						status = flowFilterDao.deleteVbridgeFilterInfo(
								connection, delVbridgeList);
						if (1 != status) {
							isCommitRequired = false;
							LOG.error("Deletion operation flowFilterVbr from database is falied.");
						}
					}
					if (isCommitRequired) {
						status = flowFilterDao.deleteVrouterFilterInfo(
								connection, delVrouterList);
						if (1 != status) {
							isCommitRequired = false;
							LOG.error("Deletion operation flowFilterVrt from database is falied.");
						}
					}

					/* if insert and delete from DB is success */
					if (isCommitRequired == true) {
						errorCode = upDateFlowList(restResource, requestBody);
						if (errorCode != UncCommonEnum.UncResultCode.UNC_SUCCESS
								.getValue()) {
							isCommitRequired = false;
							errorCode = UncResultCode.UNC_SERVER_ERROR
									.getValue();
							LOG.error("Flow List Update at UNC is failed.");
						}
						if (!delVrouterList.isEmpty()
								&& true == isCommitRequired) {
							for (int i = 0; i < delVrouterList.size()
									&& true == isCommitRequired; i++) {
								isCommitRequired = deleteList(restResource,
										delVrouterList.get(i).getVtnName(),
										delVrouterList.get(i).getVbrName(),
										delVrouterList.get(i).getVrtIfName(),
										seqnum);
								if (true != isCommitRequired) {
									errorCode = UncResultCode.UNC_SERVER_ERROR
											.getValue();
								}
							}
						}

						if (!delVbridgeList.isEmpty()
								&& true == isCommitRequired) {
							for (int i = 0; i < delVbridgeList.size()
									&& true == isCommitRequired; i++) {
								isCommitRequired = deleteList(restResource,
										delVbridgeList.get(i).getVtnName(),
										delVbridgeList.get(i).getVbrName(),
										delVbridgeList.get(i).getVbrIfName(),
										seqnum);
								if (true != isCommitRequired) {
									errorCode = UncResultCode.UNC_SERVER_ERROR
											.getValue();
								}
							}
						}
						String priority = Integer.valueOf(
								getFilterId().substring(5, 9), 16).toString();
						String action = new String();
						if (getFilterId().substring(4, 5).equals(
								VtnServiceOpenStackConsts.FL_DROP)) {
							action = VtnServiceOpenStackConsts.S_DROP;
						} else {
							action = VtnServiceOpenStackConsts.S_PASS;
						}

						if (!setVrouterList.isEmpty()
								&& true == isCommitRequired) {
							for (int i = 0; i < setVrouterList.size()
									&& true == isCommitRequired; i++) {
								final FlowFilterVbrBean flowFilterVbrBean = setFlowFilterVbrList(
										setVrouterList.get(i).getVtnName(),
										setVrouterList.get(i).getVbrName(),
										setVrouterList.get(i).getVrtIfName());
								isCommitRequired = creatList(restResource,
										flowFilterVbrBean, priority, action);
								if (true != isCommitRequired) {
									errorCode = UncResultCode.UNC_SERVER_ERROR
											.getValue();
								}

							}
						}

						if (!setVbridgeList.isEmpty()
								&& true == isCommitRequired) {
							for (int i = 0; i < setVbridgeList.size()
									&& true == isCommitRequired; i++) {
								final FlowFilterVbrBean flowFilterVbrBean = setFlowFilterVbrList(
										setVbridgeList.get(i).getVtnName(),
										setVbridgeList.get(i).getVbrName(),
										setVbridgeList.get(i).getVbrIfName());
								isCommitRequired = creatList(restResource,
										flowFilterVbrBean, priority, action);
								if (true != isCommitRequired) {
									errorCode = UncResultCode.UNC_SERVER_ERROR
											.getValue();
								}
							}
						}
					}
					checkForSpecificErrors(restResource.getInfo());
				} else {
					errorCode = UncResultCode.UNC_SUCCESS.getValue();
					LOG.debug("requestbody is empty.");
				}
			} else {
				isCommitRequired = false;
				if (null != filtersResource.getInfo()) {
					setInfo(filtersResource.getInfo());
				}
				LOG.error("Resource not found error.");
			}

			/*
			 * If all processing are OK, the commit all the database transaction
			 * made for current connection. Otherwise do the roll-back
			 */
			if (isCommitRequired) {
				// connection.commit();
				setOpenStackConnection(connection);
				LOG.info("Resource insertion successful in database.");
				isRollback = false;
			} else {
				connection.rollback();
				LOG.info("Resource insertion is roll-backed.");
			}

			/*
			 * set response, if it is not set during processing for create
			 * tenant
			 */
			if (errorCode != UncResultCode.UNC_SUCCESS.getValue()) {
				if (getInfo() == null) {
					createErrorInfo(UncResultCode.UNC_INTERNAL_SERVER_ERROR
							.getValue());
				}
			}

		} catch (final SQLException exception) {
			LOG.error(exception, "Internal server error : " + exception);
			errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();
			createErrorInfo(UncResultCode.UNC_INTERNAL_SERVER_ERROR.getValue());
		} finally {
			if (connection != null && isRollback) {
				try {
					connection.rollback();
				} catch (final SQLException e) {
					LOG.error(e, "Rollback error : " + e);
				}
				LOG.info("Free connection...");
				VtnServiceInitManager.getDbConnectionPoolMap().freeConnection(
						connection);
			}
		}
		LOG.trace("Complete FlowFilter#put()");
		return errorCode;
	}

	/**
	 * remove the same apply_port
	 * 
	 * @param apply_port
	 *            - apply_ports
	 * @return - apply , the apply_port which remove the same.
	 */
	private JsonArray RemoveSameApply(JsonArray apply_port) {
		JsonArray apply = new JsonArray();
		for (int i = 0; i < apply_port.size(); i++) {
			boolean apply_flag = false;
			JsonObject jsOb_default = (JsonObject) apply_port.get(i);
			for (int j = i + 1; j < apply_port.size(); j++) {
				JsonObject jsOb = (JsonObject) apply_port.get(j);
				if (jsOb_default.toString().equals(jsOb.toString())) {
					apply_flag = true;
					break;
				}
			}
			if (!apply_flag) {
				apply.add(jsOb_default);
			}
		}
		return apply;
	}

	/**
	 * Up date for flow list
	 * 
	 * @param restResource
	 *            - RestResource instance
	 * @param requestBody
	 *            - Request Body
	 * @return - 200 , success
	 */
	private int upDateFlowList(RestResource restResource, JsonObject requestBody) {

		int errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();
		StringBuilder sb = new StringBuilder();
		int request = 0;
		int response = 0;
		boolean deleteFlag = false;

		/* URI:/flowlists/{fl_name}/flowlistentries/{seqnum} */
		/* This URI is always can be used for PUT and GET */
		sb.append(VtnServiceOpenStackConsts.FLOWLIST_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getFilterId());
		sb.append(VtnServiceOpenStackConsts.FLOWLIST_ENTRY_PATH);
		sb.append("/1");
		JsonObject flowListEntryRequest = new JsonObject();

		restResource.setPath(sb.toString());
		restResource.setSessionID(getSessionID());
		restResource.setConfigID(getConfigID());

		/* If not need to update L4 or ICMP */
		if (requestBody.has(VtnServiceOpenStackConsts.PROTOCOL)
				|| requestBody.has(VtnServiceOpenStackConsts.SRC_PORT)
				|| requestBody.has(VtnServiceOpenStackConsts.DST_PORT)) {
			JsonObject delFlowListEntryRequest = new JsonObject();
			JsonObject getFlowListEntryRequest = new JsonObject();
			/*
			 * { "targetdb": "{targetdb}" }
			 * 
			 * get flow list entry
			 */
			getFlowListEntryRequest.addProperty(VtnServiceJsonConsts.TARGETDB,
					VtnServiceJsonConsts.CANDIDATE);
			errorCode = restResource.get(getFlowListEntryRequest);
			if (errorCode != UncResultCode.UNC_SUCCESS.getValue()) {
				LOG.error("Flow List Get at UNC is failed.");
				return errorCode;
			}
			JsonObject responseBody = restResource.getInfo()
					.get(VtnServiceJsonConsts.FLOWLISTENTRY).getAsJsonObject();
			response = getProtocolInfo(responseBody);
			if (!responseBody.has(VtnServiceJsonConsts.IPPROTO)
					|| requestBody.has(VtnServiceOpenStackConsts.PROTOCOL)) {
				flowListEntryRequest = FlowListResourceGenerator
						.getFlowListEntryRequestBody(requestBody,
								VtnServiceConsts.PUT, "-1");
			} else {
				if (responseBody.get(VtnServiceJsonConsts.IPPROTO)
						.getAsString().equals("1")) {
					if (requestBody.has(VtnServiceOpenStackConsts.SRC_PORT)
							&& !requestBody
									.get(VtnServiceOpenStackConsts.SRC_PORT)
									.getAsString().isEmpty()) {
						Integer port = Integer.parseInt(requestBody.get(
								VtnServiceOpenStackConsts.SRC_PORT)
								.getAsString());
						if (port < VtnServiceJsonConsts.VAL_0
								|| port > VtnServiceJsonConsts.VAL_255) {
							createErrorInfo(
									UncResultCode.UNC_INVALID_ARGUMENT
											.getValue(),
									UncResultCode.UNC_INVALID_ARGUMENT
											.getMessage()
											+ VtnServiceOpenStackConsts.SRC_PORT
											+ VtnServiceConsts.COLON
											+ port.toString()
											+ VtnServiceConsts.CLOSE_SMALL_BRACES);
							LOG.error("Validation failure");
							return UncResultCode.UNC_SERVER_ERROR.getValue();
						}
					}
					if (requestBody.has(VtnServiceOpenStackConsts.DST_PORT)
							&& !requestBody
									.get(VtnServiceOpenStackConsts.DST_PORT)
									.getAsString().isEmpty()) {
						Integer port = Integer.parseInt(requestBody.get(
								VtnServiceOpenStackConsts.DST_PORT)
								.getAsString());
						if (port < VtnServiceJsonConsts.VAL_0
								|| port > VtnServiceJsonConsts.VAL_255) {
							createErrorInfo(
									UncResultCode.UNC_INVALID_ARGUMENT
											.getValue(),
									UncResultCode.UNC_INVALID_ARGUMENT
											.getMessage()
											+ VtnServiceOpenStackConsts.DST_PORT
											+ VtnServiceConsts.COLON
											+ port.toString()
											+ VtnServiceConsts.CLOSE_SMALL_BRACES);
							LOG.error("Validation failure");
							return UncResultCode.UNC_SERVER_ERROR.getValue();
						}
					}
				}
				flowListEntryRequest = FlowListResourceGenerator
						.getFlowListEntryRequestBody(requestBody, "put",
								responseBody.get(VtnServiceJsonConsts.IPPROTO)
										.getAsString());
			}
			request = getProtocolInfo(flowListEntryRequest.get(
					VtnServiceJsonConsts.FLOWLISTENTRY).getAsJsonObject());

			if (1 == response && 2 == request) {
				/* delete icmp */
				deleteFlag = true;
				delFlowListEntryRequest = FlowListResourceGenerator
						.getDelFlowListEntryRequestBody("1");
			} else if (2 == response && 1 == request) {
				/* delete L4 */
				deleteFlag = true;
				delFlowListEntryRequest = FlowListResourceGenerator
						.getDelFlowListEntryRequestBody("2");
			}

			if (deleteFlag) {
				errorCode = restResource.put(delFlowListEntryRequest);
				if (errorCode != UncResultCode.UNC_SUCCESS.getValue()) {
					LOG.error("Flow List Update for delete ICMP or L4 at UNC is failed.");
					return errorCode;
				}
			}
		} else {
			flowListEntryRequest = FlowListResourceGenerator
					.getFlowListEntryRequestBody(requestBody, "put", "-1");
		}

		errorCode = restResource.put(flowListEntryRequest);
		return errorCode;
	}

	/**
	 * Get the info for Body
	 * 
	 * @param body
	 *            - JsonObject Body
	 * @return - 1 , ICMP was setted. - 2 , L4 was setted. - 3 , ICMP and L4 are
	 *         not setted. - 0 , error.
	 */
	private int getProtocolInfo(JsonObject body) {

		if (body.has(VtnServiceJsonConsts.IPPROTO)
				&& body.get(VtnServiceJsonConsts.IPPROTO).getAsString()
						.equals("1")) {
			return 1;
		}
		if (body.has(VtnServiceJsonConsts.IPPROTO)
				&& !body.get(VtnServiceJsonConsts.IPPROTO).getAsString()
						.equals("1")) {
			return 2;
		}
		if (!body.has(VtnServiceJsonConsts.IPPROTO)
				&& (body.has(VtnServiceJsonConsts.L4SRCPORT) || body
						.has(VtnServiceJsonConsts.L4DSTPORT))) {
			return 2;
		}
		if (!body.has(VtnServiceJsonConsts.IPPROTO)
				&& !body.has(VtnServiceJsonConsts.L4SRCPORT)
				&& !body.has(VtnServiceJsonConsts.L4DSTPORT)) {
			return 3;
		}

		return 0;
	}

	/**
	 * creat for flow filter and filow filter entry
	 * 
	 * @param restResource
	 *            - RestResource instance
	 * @param flowFilterVbrBean
	 *            - Bean for FlowFilterVbr
	 * @param priority
	 *            - interface id
	 * @param action
	 *            - action for flow filter
	 * @return - true , success
	 */
	private boolean creatList(RestResource restResource,
			FlowFilterVbrBean flowFilterVbrBean, String priority, String action) {
		final FiltersResource filtersResource = new FiltersResource();
		int errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();
		if (0 == getFlowFilterEntriesCount(restResource,
				flowFilterVbrBean.getVtnName(), flowFilterVbrBean.getVbrName(),
				flowFilterVbrBean.getVbrIfName())) {
			errorCode = filtersResource.createFlowFilter(flowFilterVbrBean,
					restResource);
		} else {
			errorCode = UncCommonEnum.UncResultCode.UNC_SUCCESS.getValue();
		}
		if (errorCode != UncCommonEnum.UncResultCode.UNC_SUCCESS.getValue()) {

			errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();
			LOG.error("Flow Filter creation is failed at UNC.");
			return false;
		}
		errorCode = createFlowFilterEntries(priority, action,
				flowFilterVbrBean, restResource);
		if (errorCode != UncCommonEnum.UncResultCode.UNC_SUCCESS.getValue()) {
			errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();
			LOG.error("Flow Filter Entry creation is failed at UNC.");
			return false;
		}
		return true;
	}

	/**
	 * delete for flow filter and filow filter entry
	 * 
	 * @param restResource
	 *            - RestResource instance
	 * @param vtnName
	 *            - vtn name
	 * @param vbrName
	 *            - vbr name
	 * @param ifName
	 *            - if name
	 * @param seqnum
	 *            - seq num
	 * @return - true , success
	 */
	private boolean deleteList(RestResource restResource, String vtnName,
			String vbrName, String ifName, String seqnum) {

		int errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();
		errorCode = deleteFlowFilterEntries(restResource, vtnName, vbrName,
				ifName, seqnum);
		if (errorCode != UncCommonEnum.UncResultCode.UNC_SUCCESS.getValue()) {
			errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();
			LOG.error("Flow Filter Entry Deletion is failed at UNC.");
			return false;
		}
		int count = getFlowFilterEntriesCount(restResource, vtnName, vbrName,
				ifName);
		if (0 == count) {
			errorCode = deleteFlowFilter(restResource, vtnName, vbrName, ifName);
			if (errorCode != UncCommonEnum.UncResultCode.UNC_SUCCESS.getValue()) {
				errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();
				LOG.error("Flow Filter Deletion is failed at UNC.");
				return false;
			}
		} else if (count < 0) {
			errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();
			LOG.error("Get Flow Filter Entries Count is failed at UNC.");
			return false;
		}
		return true;
	}

	/**
	 * Handler method for GET operation of Filter
	 * 
	 * @see org.opendaylight.vtn.javaapi.resources.AbstractResource#del()
	 */
	@Override
	public int delete() {
		LOG.trace("Start FlowFilter#delete()");

		int errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();

		boolean isCommitRequired = true;
		boolean isRollback = true;
		Connection connection = null;

		try {
			connection = VtnServiceInitManager.getDbConnectionPoolMap()
					.getConnection();
			/*
			 * Check for instances that they exists or not, if not then return
			 * 404 error
			 */
			if (checkForNotFoundResources(connection)) {
				final FlowFilterDao flowFilterDao = new FlowFilterDao();
				final FlowListDao flowListDao = new FlowListDao();
				final FlowListBean flowListBean = new FlowListBean();
				flowListBean.setFlName(getFilterId());
				final FreeCounterBean freeCounterBean = new FreeCounterBean();

				freeCounterBean
						.setResourceId(VtnServiceOpenStackConsts.FILTER_RES_ID);
				freeCounterBean
						.setVtnName(VtnServiceOpenStackConsts.FILTER_VTN_NAME);
				freeCounterBean.setResourceCounter(flowListDao
						.getSpecifiedFlowListId(connection, getFilterId()));
				RestResource restResource = new RestResource();
				final ResourceIdManager resourceIdManager = new ResourceIdManager();
				ArrayList<FlowFilterVrtBean> portIdList = flowFilterDao
						.getInterfaceList(connection, getFilterId());
				if (resourceIdManager.deleteResourceId(connection,
						freeCounterBean, flowListBean)) {
					String seqnum = Integer.valueOf(
							getFilterId().substring(5, 9), 16).toString();
					if (null != portIdList) {
						for (int i = 0; i < portIdList.size()
								&& true == isCommitRequired; i++) {
							isCommitRequired = deleteList(restResource,
									portIdList.get(i).getVtnName(), portIdList
											.get(i).getVbrName(), portIdList
											.get(i).getVrtIfName(), seqnum);
						}
					}
					if (true == isCommitRequired) {
						restResource
								.setPath(VtnServiceOpenStackConsts.FLOWLIST_PATH
										+ VtnServiceOpenStackConsts.URI_CONCATENATOR
										+ getFilterId());
						restResource.setSessionID(getSessionID());
						restResource.setConfigID(getConfigID());

						errorCode = restResource.delete();
						if (errorCode != UncCommonEnum.UncResultCode.UNC_SUCCESS
								.getValue()) {
							isCommitRequired = false;
							errorCode = UncResultCode.UNC_SERVER_ERROR
									.getValue();
							LOG.error("Flow List Deletion is failed at UNC.");
						}
					}
					checkForSpecificErrors(restResource.getInfo());
				} else {
					isCommitRequired = false;
					LOG.info("Deletion operation from database is falied.");
				}

			} else {
				isCommitRequired = false;
				LOG.error("Resource not found error.");
			}
			/*
			 * If all processing are OK, the commit all the database transaction
			 * made for current connection. Otherwise do the roll-back
			 */
			if (isCommitRequired) {
				// connection.commit();
				setOpenStackConnection(connection);
				LOG.info("Resource deletion successful in database.");
				isRollback = false;
			} else {
				connection.rollback();
				LOG.info("Resource deletion is roll-backed.");
			}

			/*
			 * set response, if it is not set during above processing
			 */
			if (errorCode != UncResultCode.UNC_SUCCESS.getValue()) {
				if (getInfo() == null) {
					createErrorInfo(UncResultCode.UNC_INTERNAL_SERVER_ERROR
							.getValue());
				}
			}

		} catch (final SQLException exception) {
			LOG.error(exception, "Internal server error ocuurred.");
			errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();
			createErrorInfo(UncResultCode.UNC_INTERNAL_SERVER_ERROR.getValue());
		} finally {
			if (connection != null && isRollback) {
				try {
					connection.rollback();
				} catch (final SQLException e) {
					LOG.error(e, "Rollback error : " + e);
				}
				LOG.info("Free connection...");
				VtnServiceInitManager.getDbConnectionPoolMap().freeConnection(
						connection);
			}
		}
		LOG.trace("Complete FlowFilter#delete()");
		return errorCode;

	}

	/**
	 * Create Flow Filter at UNC
	 * 
	 * @param requestBody
	 *            - OpenStack request body
	 * @param flowFilterVrtBean
	 *            - flow Filter infomation
	 * @param restResource
	 *            - RestResource instance
	 * @return - erorrCode, 200 for Success
	 */
	private int createFlowFilterEntries(String priority, String action,
			FlowFilterVbrBean flowFilterVbrBean, RestResource restResource) {
		JsonObject flowfilterEntryRequest = new JsonObject();
		JsonObject flowfilter = new JsonObject();
		JsonObject redirectdst = new JsonObject();

		StringBuilder sb = new StringBuilder();

		flowfilter.addProperty(VtnServiceJsonConsts.SEQNUM, priority);
		flowfilter.addProperty(VtnServiceJsonConsts.FLNAME, getFilterId());
		flowfilter.addProperty(VtnServiceJsonConsts.ACTIONTYPE, action);
		flowfilter.add(VtnServiceJsonConsts.REDIRECTDST, redirectdst);
		flowfilterEntryRequest.add(VtnServiceJsonConsts.FLOWFILTERENTRY,
				flowfilter);

		sb.append(VtnServiceOpenStackConsts.VTN_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(flowFilterVbrBean.getVtnName());
		sb.append(VtnServiceOpenStackConsts.VBRIDGE_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(flowFilterVbrBean.getVbrName());
		sb.append(VtnServiceOpenStackConsts.INTERFACE_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(flowFilterVbrBean.getVbrIfName());
		sb.append(VtnServiceOpenStackConsts.FLOWFILTER_PATH);
		sb.append(VtnServiceOpenStackConsts.IN_PATH);
		sb.append(VtnServiceOpenStackConsts.FLOWFILTER_ENTRY_PATH);

		restResource.setPath(sb.toString());
		restResource.setSessionID(getSessionID());
		restResource.setConfigID(getConfigID());

		return restResource.post(flowfilterEntryRequest);

	}

	/**
	 * delete Flow Filter at UNC
	 * 
	 * @param requestBody
	 *            - OpenStack request body
	 * @param flowFilterVrtBean
	 *            - flow Filter infomation
	 * @param restResource
	 *            - RestResource instance
	 * @return - erorrCode, 200 for Success
	 */
	private int deleteFlowFilter(RestResource restResource, String vtnName,
			String vbrName, String ifName) {

		StringBuilder sb = new StringBuilder();
		sb.append(VtnServiceOpenStackConsts.VTN_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(vtnName);
		sb.append(VtnServiceOpenStackConsts.VBRIDGE_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(vbrName);
		sb.append(VtnServiceOpenStackConsts.INTERFACE_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(ifName);
		sb.append(VtnServiceOpenStackConsts.FLOWFILTER_PATH);
		sb.append(VtnServiceOpenStackConsts.IN_PATH);

		restResource.setPath(sb.toString());
		restResource.setSessionID(getSessionID());
		restResource.setConfigID(getConfigID());
		return restResource.delete();
	}

	/**
	 * get Flow Filter Entry at UNC
	 * 
	 * @param requestBody
	 *            - OpenStack request body
	 * @param flowFilterVrtBean
	 *            - flow Filter infomation
	 * @param restResource
	 *            - RestResource instance
	 * @return - erorrCode, 200 for Success
	 */
	public int getFlowFilterEntriesCount(RestResource restResource,
			String vtnName, String vbrName, String ifName) {
		int errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();
		JsonObject flowFilterCount = new JsonObject();
		flowFilterCount.addProperty(VtnServiceJsonConsts.OP,
				VtnServiceJsonConsts.COUNT);
		flowFilterCount.addProperty(VtnServiceJsonConsts.TARGETDB,
				VtnServiceJsonConsts.CANDIDATE);

		StringBuilder sb = new StringBuilder();

		sb.append(VtnServiceOpenStackConsts.VTN_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(vtnName);
		sb.append(VtnServiceOpenStackConsts.VBRIDGE_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(vbrName);
		sb.append(VtnServiceOpenStackConsts.INTERFACE_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(ifName);
		sb.append(VtnServiceOpenStackConsts.FLOWFILTER_PATH);
		sb.append(VtnServiceOpenStackConsts.IN_PATH);
		sb.append(VtnServiceOpenStackConsts.FLOWFILTER_ENTRY_PATH);
		// sb.append(VtnServiceOpenStackConsts.COUNT_PATH);

		long configId = restResource.getConfigID();
		long sessionID = restResource.getSessionID();

		restResource.setPath(sb.toString());
		restResource.setSessionID(sessionID);
		restResource.setConfigID(configId);
		errorCode = restResource.get(flowFilterCount);
		/*
		 * delete return always not 400, if 400 return , creat Flow Filter Entry
		 * will be faild.
		 */
		if (UncCommonEnum.UncResultCode.UNC_CLIENT_ERROR.getValue() == errorCode) {
			return 0;
		} else if (errorCode != UncCommonEnum.UncResultCode.UNC_SUCCESS
				.getValue()) {
			return -1;
		}
		return Integer.parseInt(restResource.getInfo()
				.get(VtnServiceJsonConsts.FLOWFILTERENTRIES).getAsJsonObject()
				.get(VtnServiceJsonConsts.COUNT).getAsString());

	}

	/**
	 * delete Flow Filter Entry at UNC
	 * 
	 * @param requestBody
	 *            - OpenStack request body
	 * @param flowFilterVrtBean
	 *            - flow Filter infomation
	 * @param restResource
	 *            - RestResource instance
	 * @return - erorrCode, 200 for Success
	 */
	private int deleteFlowFilterEntries(RestResource restResource,
			String vtnName, String vbrName, String ifName, String seqNum) {
		StringBuilder sb = new StringBuilder();
		sb.append(VtnServiceOpenStackConsts.VTN_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(vtnName);
		sb.append(VtnServiceOpenStackConsts.VBRIDGE_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(vbrName);
		sb.append(VtnServiceOpenStackConsts.INTERFACE_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(ifName);
		sb.append(VtnServiceOpenStackConsts.FLOWFILTER_PATH);
		sb.append(VtnServiceOpenStackConsts.IN_PATH);
		sb.append(VtnServiceOpenStackConsts.FLOWFILTER_ENTRY_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(seqNum);

		restResource.setPath(sb.toString());
		restResource.setSessionID(getSessionID());
		restResource.setConfigID(getConfigID());

		return restResource.delete();

	}

	/**
	 * set infomation to the FlowFilterVbrBean
	 * 
	 * @param vtnName
	 *            - vtn name
	 * @param vbrName
	 *            - vbr name
	 * @param vrtIfName
	 *            - if name
	 * @return - erorrCode, 200 for Success
	 */

	private FlowFilterVbrBean setFlowFilterVbrList(String vtnName,
			String vbrName, String vbrIfName) {
		final FlowFilterVbrBean flowFilterVbrBean = new FlowFilterVbrBean();
		flowFilterVbrBean.setFlName(getFilterId());
		flowFilterVbrBean.setVtnName(vtnName);
		flowFilterVbrBean.setVbrName(vbrName);
		flowFilterVbrBean.setVbrIfName(vbrIfName);
		return flowFilterVbrBean;

	}

	/**
	 * set infomation to the FlowFilterVbrBean
	 * 
	 * @param vtnName
	 *            - vtn name
	 * @param vbrName
	 *            - vbr name
	 * @param vrtIfName
	 *            - if name
	 * @return - erorrCode, 200 for Success
	 */

	private FlowFilterVrtBean setFlowFilterVrtList(String vtnName,
			String vrtName, String vbrName, String vrtIfName) {
		final FlowFilterVrtBean flowFilterVrtBean = new FlowFilterVrtBean();
		flowFilterVrtBean.setFlName(getFilterId());
		flowFilterVrtBean.setVtnName(vtnName);
		flowFilterVrtBean.setVbrName(vbrName);
		flowFilterVrtBean.setVrtName(vrtName);
		flowFilterVrtBean.setVrtIfName(vrtIfName);
		return flowFilterVrtBean;

	}

	/**
	 * Handler method for GET operation of Filter
	 * 
	 * @see org.opendaylight.vtn.javaapi.resources.AbstractResource#get()
	 */
	@Override
	public int get(final JsonObject requestBody) {
		LOG.trace("Start FilterResource#get()");

		int errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();

		Connection connection = null;

		try {
			connection = VtnServiceInitManager.getDbConnectionPoolMap()
					.getConnection();

			/*
			 * Check for instances that they exists or not, if not then return
			 * 404 error
			 */
			if (checkForNotFoundResources(connection)) {

				final JsonObject openStackResponse = new JsonObject();

				/*
				 * retrieval "action" and "priority" from "filter_id".
				 */
				String action = "pass";
				if ('d' == getFilterId().charAt(4)) {
					action = "drop";
				}

				openStackResponse.addProperty(VtnServiceOpenStackConsts.ACTION,
						action);

				String priority = getFilterId().substring(5, 9);

				openStackResponse.addProperty(
						VtnServiceOpenStackConsts.PRIORITY,
						String.valueOf((Integer.parseInt(priority, 16))));

				/*
				 * retrieval flow list entry information.
				 */
				final RestResource restResource = new RestResource();

				errorCode = getFlowListEntry(restResource);
				if (UncCommonEnum.UncResultCode.UNC_SUCCESS.getValue() == errorCode) {
					openStackResponse.addProperty(
							VtnServiceOpenStackConsts.SRC_MAC,
							VtnServiceJsonConsts.BLANK);
					openStackResponse.addProperty(
							VtnServiceOpenStackConsts.DST_MAC,
							VtnServiceJsonConsts.BLANK);
					openStackResponse.addProperty(
							VtnServiceOpenStackConsts.ETH_TYPE,
							VtnServiceJsonConsts.BLANK);
					openStackResponse.addProperty(
							VtnServiceOpenStackConsts.SRC_CIDR,
							VtnServiceJsonConsts.BLANK);
					openStackResponse.addProperty(
							VtnServiceOpenStackConsts.DST_CIDR,
							VtnServiceJsonConsts.BLANK);
					openStackResponse.addProperty(
							VtnServiceOpenStackConsts.PROTOCOL,
							VtnServiceJsonConsts.BLANK);
					openStackResponse.addProperty(
							VtnServiceOpenStackConsts.SRC_PORT,
							VtnServiceJsonConsts.BLANK);
					openStackResponse.addProperty(
							VtnServiceOpenStackConsts.DST_PORT,
							VtnServiceJsonConsts.BLANK);
					FlowListResourceGenerator.setFlowListResponseBody(
							openStackResponse, restResource.getInfo());
				}

				/*
				 * retrieval apply ports information.
				 */
				if (UncCommonEnum.UncResultCode.UNC_SUCCESS.getValue() == errorCode) {
					JsonArray applyPorts = getApplyPorts(connection);
					openStackResponse.add(
							VtnServiceOpenStackConsts.APPLY_PORTS, applyPorts);
					setInfo(openStackResponse);
				}

			} else {
				LOG.error("Resource not found error.");
			}
			/*
			 * set response, if it is not set during above processing
			 */
			if (errorCode != UncResultCode.UNC_SUCCESS.getValue()) {
				if (getInfo() == null) {
					createErrorInfo(UncResultCode.UNC_INTERNAL_SERVER_ERROR
							.getValue());
				}
			}
		} catch (final SQLException exception) {
			LOG.error(exception, "Internal server error : " + exception);
			errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();
			createErrorInfo(UncResultCode.UNC_INTERNAL_SERVER_ERROR.getValue());
		} finally {
			if (connection != null) {
				LOG.info("Free connection...");
				VtnServiceInitManager.getDbConnectionPoolMap().freeConnection(
						connection);
			}
		}
		LOG.trace("Complete FilterResource#get()");
		return errorCode;
	}

	/**
	 * Checks that specified instances in URI exists in system or not. If they
	 * are not exists then prepare error JSON for 404 Not Found
	 * 
	 * @param connection
	 *            - Database Connection instance
	 * @return - true, only if all instances exist
	 * @throws SQLException
	 */
	private boolean checkForNotFoundResources(Connection connection)
			throws SQLException {
		boolean resourceFound = false;
		FlowListBean filterBean = new FlowListBean();
		filterBean.setFlName(filterId);
		if (new FlowListDao().isFlowListFound(connection, filterBean)) {
			resourceFound = true;
		} else {
			createErrorInfo(
					UncResultCode.UNC_NOT_FOUND.getValue(),
					getCutomErrorMessage(
							UncResultCode.UNC_NOT_FOUND.getMessage(),
							VtnServiceOpenStackConsts.FILTER_ID, getFilterId()));
		}
		return resourceFound;
	}

	/**
	 * Retrieve flow list UNC
	 * 
	 * @param requestBody
	 *            - OpenStack request body
	 * @param restResource
	 *            - RestResource instance
	 * @return - erorrCode, 200 for Success
	 */
	private int getFlowListEntry(RestResource restResource) {

		/*
		 * Create request body for vBridge update
		 */
		final JsonObject flowListEntryRequestBody = FlowListResourceGenerator
				.getFlowListEntryRequestBody();

		StringBuilder sb = new StringBuilder();
		sb.append(VtnServiceOpenStackConsts.FLOWLIST_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getFilterId());
		sb.append(VtnServiceOpenStackConsts.FLOWLIST_ENTRY_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(1);

		restResource.setPath(sb.toString());
		restResource.setSessionID(getSessionID());
		restResource.setConfigID(getConfigID());

		return restResource.get(flowListEntryRequestBody);
	}

	/**
	 * Retrieve apply ports.
	 * 
	 * @param connection
	 *            - Database Connection instance
	 * @return - JsonArray, retrieved apply ports
	 * @throws SQLException
	 */
	private JsonArray getApplyPorts(Connection connection) throws SQLException {
		FlowFilterDao flowFilterDao = new FlowFilterDao();
		return flowFilterDao.getFlowFilters(connection, filterId);
	}
}
