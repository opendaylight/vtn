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
import java.util.List;

import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonPrimitive;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.RestResource;
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.init.VtnServiceInitManager;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.openstack.beans.FlowFilterVbrBean;
import org.opendaylight.vtn.javaapi.openstack.beans.FlowFilterVrtBean;
import org.opendaylight.vtn.javaapi.openstack.beans.FlowListBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VBridgeBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VBridgeInterfaceBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VRouterBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VRouterInterfaceBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VtnBean;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;
import org.opendaylight.vtn.javaapi.openstack.convertor.FlowListResourceGenerator;
import org.opendaylight.vtn.javaapi.openstack.dao.FlowFilterDao;
import org.opendaylight.vtn.javaapi.openstack.dao.FlowListDao;
import org.opendaylight.vtn.javaapi.openstack.dao.FreeCounterDao;
import org.opendaylight.vtn.javaapi.openstack.dao.VBridgeDao;
import org.opendaylight.vtn.javaapi.openstack.dao.VBridgeInterfaceDao;
import org.opendaylight.vtn.javaapi.openstack.dao.VRouterDao;
import org.opendaylight.vtn.javaapi.openstack.dao.VRouterInterfaceDao;
import org.opendaylight.vtn.javaapi.openstack.dao.VtnDao;
import org.opendaylight.vtn.javaapi.openstack.validation.FilterResourceValidator;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.openstack.dbmanager.ResourceIdManager;
import org.opendaylight.vtn.javaapi.openstack.beans.FreeCounterBean;

/**
 * Resource class for handling requests for Filters
 */
@UNCVtnService(path = VtnServiceOpenStackConsts.FILTERS_PATH)
public class FiltersResource extends AbstractResource {

	/* Logger instance */
	private static final Logger LOG = Logger.getLogger(FiltersResource.class
			.getName());

	/**
	 * Constructor that initialize the validation instance for current resource
	 * instance
	 */
	public FiltersResource() {
		setValidator(new FilterResourceValidator(this));
	}

	/**
	 * Handler method for POST operation of Flow Filter
	 * 
	 * @see org.opendaylight.vtn.javaapi.resources.AbstractResource#post(com
	 *      .google.gson.JsonObject)
	 */

	@Override
	public int post(JsonObject requestBody) {
		LOG.trace("Start FlowFilter#post()");

		int errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();

		boolean isCommitRequired = true;
		boolean isRollback = true;
		String generatedflName = null;
		Connection connection = null;
		int counter = -1;
		int priority = 0;
		boolean respons = false;
		boolean isApply = true;

		try {
			connection = VtnServiceInitManager.getDbConnectionPoolMap()
					.getConnection();
			/*
			 * Check for instances that they exists or not, if not then return
			 * 404 error
			 */
			if (null == checkForNotFoundResources(connection, requestBody)) {
				final FlowListBean flowListBean = new FlowListBean();
				if (!requestBody.has(VtnServiceOpenStackConsts.ID)) {
					final ResourceIdManager resourceIdManager = new ResourceIdManager();
					LOG.info("Resource id auto-generation is required.");
					FreeCounterBean freeCounterBean = new FreeCounterBean();
					freeCounterBean
							.setResourceId(VtnServiceOpenStackConsts.FILTER_RES_ID);
					freeCounterBean
							.setVtnName(VtnServiceOpenStackConsts.FILTER_VTN_NAME);

					counter = resourceIdManager.getResourceCounter(connection,
							freeCounterBean);
					if (-1 != counter) {
						LOG.debug("Resource id auto-generation is successfull : "
								+ counter);
						priority = Integer.valueOf(
								requestBody.get(
										VtnServiceOpenStackConsts.PRIORITY)
										.getAsString()).intValue();
						String sqnum = String.format("%04X", priority);
						if (requestBody.get(VtnServiceOpenStackConsts.ACTION)
								.getAsString()
								.equals(VtnServiceOpenStackConsts.S_DROP)) {
							generatedflName = VtnServiceOpenStackConsts.FL_PREFIX
									+ VtnServiceOpenStackConsts.FL_DROP
									+ sqnum
									+ VtnServiceOpenStackConsts.UNDER_LINE
									+ counter;
						} else {
							generatedflName = VtnServiceOpenStackConsts.FL_PREFIX
									+ VtnServiceOpenStackConsts.FL_PASS
									+ sqnum
									+ VtnServiceOpenStackConsts.UNDER_LINE
									+ counter;
						}
						respons = true;
						requestBody.addProperty(VtnServiceOpenStackConsts.ID,
								generatedflName);
						flowListBean
								.setFlStatus(VtnServiceOpenStackConsts.AUTOGENERATED);
						flowListBean.setFlId(counter);
					}
				} else {
					counter = 0;
					generatedflName = requestBody.get(
							VtnServiceOpenStackConsts.ID).getAsString();
					if (isValidSeqNum(generatedflName.substring(10))) {
						final FreeCounterBean freeCounterBean = new FreeCounterBean();
						freeCounterBean
								.setResourceId(VtnServiceOpenStackConsts.FILTER_RES_ID);
						freeCounterBean
								.setVtnName(VtnServiceOpenStackConsts.FILTER_VTN_NAME);
						freeCounterBean.setResourceCounter(Integer
								.parseInt(generatedflName.substring(10)));
						final FreeCounterDao freeCounterDao = new FreeCounterDao();
						if (freeCounterDao.isCounterFound(connection,
								freeCounterBean)) {
							freeCounterDao.deleteCounter(connection,
									freeCounterBean);

						}
						flowListBean.setFlId(freeCounterBean
								.getResourceCounter());
					} else {
						flowListBean.setFlId(0);
					}
					flowListBean
							.setFlStatus(VtnServiceOpenStackConsts.MANUALINPUT);
				}

				if (counter >= 0) {
					flowListBean.setFlName(generatedflName);
					final FlowListDao flowListDao = new FlowListDao();
					int status = flowListDao.insert(connection, flowListBean);
					if (1 == status) {
						final RestResource restResource = new RestResource();
						LOG.info("Resource insertion successful at database operation.");
						if (!requestBody
								.has(VtnServiceOpenStackConsts.APPLY_PORTS)) {
							isApply = false;
						}
						ArrayList<FlowFilterVrtBean> vrouterList = new ArrayList<FlowFilterVrtBean>();
						ArrayList<FlowFilterVbrBean> vbridgeList = new ArrayList<FlowFilterVbrBean>();
						if (isApply) {
							JsonArray apply = requestBody
									.getAsJsonArray(VtnServiceOpenStackConsts.APPLY_PORTS);
							for (int i = 0; i < apply.size(); i++) {
								JsonObject jsOb = (JsonObject) apply.get(i);
								if (jsOb.has(VtnServiceOpenStackConsts.ROUTER)) {
									final FlowFilterVrtBean vrtBean = new FlowFilterVrtBean();
									vrtBean.setVtnName(jsOb.get(
											VtnServiceOpenStackConsts.TENANT)
											.getAsString());
									vrtBean.setVrtName(jsOb.get(
											VtnServiceOpenStackConsts.ROUTER)
											.getAsString());
									vrtBean.setVrtIfName(VtnServiceOpenStackConsts.IF_PREFIX
											+ jsOb.get(
													VtnServiceOpenStackConsts.INTERFACE)
													.getAsString());
									vrtBean.setFlName(generatedflName);
									final VRouterInterfaceBean vInterfaceBean = new VRouterInterfaceBean();
									vInterfaceBean
											.setVrtIfId(Integer
													.parseInt(jsOb
															.get(VtnServiceOpenStackConsts.INTERFACE)
															.getAsString()));
									vInterfaceBean.setVtnName(vrtBean
											.getVtnName());
									vInterfaceBean.setVrtName(vrtBean
											.getVrtName());
									vInterfaceBean.setVrtIfName(vrtBean
											.getVrtIfName());
									final VRouterInterfaceDao vrouterInterfaceDao = new VRouterInterfaceDao();
									vrtBean.setVbrName(vrouterInterfaceDao
											.getVbridgeName(connection,
													vInterfaceBean));
									if(!vrouterList.contains(vrtBean)) {
										vrouterList.add(vrtBean);
									}

								} else {
									final VRouterInterfaceBean vInterfaceBean = new VRouterInterfaceBean();
									vInterfaceBean.setVtnName(jsOb.get(
											VtnServiceOpenStackConsts.TENANT)
											.getAsString());
									vInterfaceBean.setVbrName(jsOb.get(
											VtnServiceOpenStackConsts.NETWORK)
											.getAsString());
									vInterfaceBean
											.setVrtIfId(Integer
													.parseInt(jsOb
															.get(VtnServiceOpenStackConsts.PORT)
															.getAsString()));
									final VRouterInterfaceDao vrouterInterfaceDao = new VRouterInterfaceDao();
									String vrouter_name = vrouterInterfaceDao
											.getVrouterName(connection,
													vInterfaceBean);
									if (null != vrouter_name) {
										final FlowFilterVrtBean vrtBean = new FlowFilterVrtBean();
										vrtBean.setFlName(generatedflName);
										vrtBean.setVtnName(jsOb
												.get(VtnServiceOpenStackConsts.TENANT)
												.getAsString());
										vrtBean.setVrtName(vrouter_name);
										vrtBean.setVbrName(jsOb
												.get(VtnServiceOpenStackConsts.NETWORK)
												.getAsString());
										vrtBean.setVrtIfName(VtnServiceOpenStackConsts.IF_PREFIX
												+ jsOb.get(
														VtnServiceOpenStackConsts.PORT)
														.getAsString());
										if(!vrouterList.contains(vrtBean)) {
											vrouterList.add(vrtBean);
										}
									} else {
										FlowFilterVbrBean vbrBean = new FlowFilterVbrBean();
										vbrBean.setVtnName(jsOb
												.get(VtnServiceOpenStackConsts.TENANT)
												.getAsString());
										vbrBean.setVbrName(jsOb
												.get(VtnServiceOpenStackConsts.NETWORK)
												.getAsString());
										vbrBean.setVbrIfName(VtnServiceOpenStackConsts.IF_PREFIX
												+ jsOb.get(
														VtnServiceOpenStackConsts.PORT)
														.getAsString());
										vbrBean.setFlName(generatedflName);
										if(!vbridgeList.contains(vbrBean)) {
											vbridgeList.add(vbrBean);
										}
									}
								}
							}
							final FlowFilterDao flowFilterDao = new FlowFilterDao();
							int st = flowFilterDao.insertVrouterFilterInfo(
									connection, vrouterList);
							if (1 == st) {
								st = flowFilterDao.insertVbridgeFilterInfo(
										connection, vbridgeList);
								if (1 != st) {
									isCommitRequired = false;
									LOG.error("Resource insertion failed at database operation.");
								}
							} else {
								isCommitRequired = false;
								LOG.error("Resource insertion failed at database operation.");
							}

						}
						if (isCommitRequired) {
							errorCode = createFlowList(requestBody,
									restResource);
							if (errorCode == UncResultCode.UNC_SUCCESS
									.getValue()) {
								errorCode = createFlowListEntries(requestBody,
										restResource);
								if (errorCode == UncResultCode.UNC_SUCCESS
										.getValue()) {
									final FilterResource filterResource = new FilterResource();
									for (int i = 0; i < vrouterList.size(); i++) {
										final FlowFilterVbrBean vbrBean = new FlowFilterVbrBean();
										vbrBean.setFlName(generatedflName);
										vbrBean.setVtnName(vrouterList.get(i)
												.getVtnName());
										vbrBean.setVbrName(vrouterList.get(i)
												.getVbrName());
										vbrBean.setVbrIfName(vrouterList.get(i)
												.getVrtIfName());
										if (0 == filterResource
												.getFlowFilterEntriesCount(
														restResource,
														vrouterList.get(i)
																.getVtnName(),
														vrouterList.get(i)
																.getVbrName(),
														vrouterList.get(i)
																.getVrtIfName())) {
											errorCode = createFlowFilter(
													vbrBean, restResource);
										}
										if (errorCode == UncResultCode.UNC_SUCCESS
												.getValue()) {
											errorCode = createFlowFilterEntries(
													requestBody, vbrBean,
													restResource);
											if (errorCode != UncResultCode.UNC_SUCCESS
													.getValue()) {
												isCommitRequired = false;
												errorCode = UncResultCode.UNC_SERVER_ERROR
														.getValue();
												LOG.error("Flow Filter Entry creation is failed at UNC.");
												break;
											}
										} else {
											isCommitRequired = false;
											errorCode = UncResultCode.UNC_SERVER_ERROR
													.getValue();
											LOG.error("Flow Filter creation is failed at UNC.");
											break;
										}
									}
									for (int i = 0; i < vbridgeList.size()
											&& isCommitRequired == true; i++) {
										if (0 == filterResource
												.getFlowFilterEntriesCount(
														restResource,
														vbridgeList.get(i)
																.getVtnName(),
														vbridgeList.get(i)
																.getVbrName(),
														vbridgeList.get(i)
																.getVbrIfName())) {
											errorCode = createFlowFilter(
													vbridgeList.get(i),
													restResource);
										}
										if (errorCode == UncResultCode.UNC_SUCCESS
												.getValue()) {
											errorCode = createFlowFilterEntries(
													requestBody,
													vbridgeList.get(i),
													restResource);
											if (errorCode != UncResultCode.UNC_SUCCESS
													.getValue()) {
												isCommitRequired = false;
												errorCode = UncResultCode.UNC_SERVER_ERROR
														.getValue();
												LOG.error("Flow Filter Entry creation is failed at UNC.");
												break;
											}
										} else {
											isCommitRequired = false;
											errorCode = UncResultCode.UNC_SERVER_ERROR
													.getValue();
											LOG.error("Flow Filter creation is failed at UNC.");
											break;
										}
									}
								} else {
									isCommitRequired = false;
									errorCode = UncResultCode.UNC_SERVER_ERROR
											.getValue();
									LOG.error("Flow List Entry creation is failed at UNC.");
								}
							} else {
								isCommitRequired = false;
								errorCode = UncResultCode.UNC_SERVER_ERROR
										.getValue();
								LOG.error("FlowList creation is failed at UNC.");

							}
						}
						checkForSpecificErrors(restResource.getInfo());
					} else {
						isCommitRequired = false;
						LOG.error("Resource insertion failed at database operation.");
					}

				} else {
					isCommitRequired = false;
					LOG.error("Error occurred while generation of id.");
				}
			} else {
				isCommitRequired = false;
				LOG.error("Resource not found error.");
			}

			if (isCommitRequired && respons) {
				final JsonObject response = new JsonObject();
				response.addProperty(VtnServiceOpenStackConsts.ID,
						generatedflName);
				setInfo(response);
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
			if (exception.getSQLState().equalsIgnoreCase(
					VtnServiceOpenStackConsts.CONFLICT_SQL_STATE)) {
				LOG.error("Conflict found during creation of resource");
				createErrorInfo(
						UncResultCode.UNC_CONFLICT_FOUND.getValue(),
						getCutomErrorMessage(
								UncResultCode.UNC_CONFLICT_FOUND.getMessage(),
								VtnServiceOpenStackConsts.ID,
								String.valueOf(generatedflName)));
			} else {
				createErrorInfo(UncResultCode.UNC_INTERNAL_SERVER_ERROR
						.getValue());
			}
		} finally {
			if (connection != null && isRollback) {
				try {
					connection.rollback();
					LOG.info("roll-back successful.");
				} catch (final SQLException e) {
					LOG.error(e, "Rollback error : " + e);
				}
				LOG.info("Free connection...");
				VtnServiceInitManager.getDbConnectionPoolMap().freeConnection(
						connection);
			}
		}
		LOG.trace("Complete FlowFilter#post()");
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
	private int createFlowFilterEntries(JsonObject requestBody,
			FlowFilterVbrBean flowFilterVbrBean, RestResource restResource) {
		JsonObject flowfilterEntryRequest = new JsonObject();
		JsonObject flowfilter = new JsonObject();
		JsonObject redirectdst = new JsonObject();

		StringBuilder sb = new StringBuilder();

		flowfilter.addProperty(VtnServiceJsonConsts.SEQNUM,
				requestBody.get(VtnServiceJsonConsts.PRIORITY).getAsString());

		flowfilter.addProperty(VtnServiceJsonConsts.FLNAME,
				requestBody.get(VtnServiceJsonConsts.ID).getAsString());
		flowfilter.addProperty(VtnServiceJsonConsts.ACTIONTYPE, requestBody
				.get(VtnServiceJsonConsts.ACTION).getAsString());
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
	 * Create Flow Filter at UNC
	 * 
	 * @param flowFilterVrtBean
	 *            - vrouter and vrouter interface infomation
	 * @param restResource
	 *            - RestResource instance
	 * @return - erorrCode, 200 for Success
	 */
	public int createFlowFilter(FlowFilterVbrBean flowFilterVbrBean,
			RestResource restResource) {
		JsonObject flowfilterRequest = new JsonObject();
		JsonObject flowfilter = new JsonObject();
		StringBuilder sb = new StringBuilder();

		flowfilter.addProperty(VtnServiceJsonConsts.FFTYPE,
				VtnServiceJsonConsts.IN);
		flowfilterRequest.add(VtnServiceJsonConsts.FLOWFILTER, flowfilter);

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

		long configId = restResource.getConfigID();
		long sessionID = restResource.getSessionID();

		restResource.setPath(sb.toString());
		restResource.setSessionID(sessionID);
		restResource.setConfigID(configId);

		return restResource.post(flowfilterRequest);
	}

	/**
	 * Create Flow List Entry at UNC
	 * 
	 * @param requestBody
	 *            - OpenStack request body
	 * @param restResource
	 *            - RestResource instance
	 * @return - erorrCode, 200 for Success
	 */
	private int createFlowListEntries(JsonObject requestBody,
			RestResource restResource) {
		JsonObject flowListEntrRequestBody = new JsonObject();

		flowListEntrRequestBody = FlowListResourceGenerator
				.getFlowListEntryRequestBody(requestBody,
						VtnServiceConsts.POST, null);
		StringBuilder sb = new StringBuilder();
		sb.append(VtnServiceOpenStackConsts.FLOWLIST_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(requestBody.get(VtnServiceOpenStackConsts.ID).getAsString());
		sb.append(VtnServiceOpenStackConsts.FLOWLIST_ENTRY_PATH);

		restResource.setPath(sb.toString());

		restResource.setSessionID(getSessionID());
		restResource.setConfigID(getConfigID());

		return restResource.post(flowListEntrRequestBody);
	}

	/**
	 * Create Flow List at UNC
	 * 
	 * @param requestBody
	 *            - OpenStack request body
	 * @param restResource
	 *            - RestResource instance
	 * @return - erorrCode, 200 for Success
	 */
	private int createFlowList(JsonObject requestBody, RestResource restResource) {
		JsonObject flowListRequestBody = new JsonObject();
		JsonObject flowList = new JsonObject();

		flowList.addProperty(VtnServiceJsonConsts.FLNAME,
				requestBody.get(VtnServiceOpenStackConsts.ID).getAsString());
		flowListRequestBody.add(VtnServiceJsonConsts.FLOWLIST, flowList);

		restResource.setPath(VtnServiceOpenStackConsts.FLOWLIST_PATH);
		restResource.setSessionID(getSessionID());
		restResource.setConfigID(getConfigID());

		return restResource.post(flowListRequestBody);

	}

	/**
	 * Checks that specified instances in requestBody exists in system or not.
	 * If they are not exists then prepare error JSON for 404 Not Found
	 * 
	 * @param connection
	 *            - Database Connection instance
	 * @param requestBody
	 *            - OpenStack request body
	 * @return - true, only if all instances exist
	 * @throws SQLException
	 */
	public JsonObject checkForNotFoundResources(Connection connection,
			JsonObject requestBody) throws SQLException {

		JsonArray apply = new JsonArray();
		if (!requestBody.has(VtnServiceOpenStackConsts.APPLY_PORTS)) {
			return null;
		}
		apply = requestBody
				.getAsJsonArray(VtnServiceOpenStackConsts.APPLY_PORTS);
		final VtnDao vtnDao = new VtnDao();
		final VRouterDao vRouterDao = new VRouterDao();
		final VRouterInterfaceDao vRouterInterfaceDao = new VRouterInterfaceDao();
		final VBridgeDao vBridgeDao = new VBridgeDao();
		final VBridgeInterfaceDao vBridgeInterfaceDao = new VBridgeInterfaceDao();

		for (int i = 0; i < apply.size(); i++) {
			JsonObject jsOb = (JsonObject) apply.get(i);
			if (jsOb.has(VtnServiceOpenStackConsts.ROUTER)) {
				final VtnBean vtnBean = new VtnBean();
				vtnBean.setVtnName(jsOb.get(VtnServiceOpenStackConsts.TENANT)
						.getAsString());
				if (vtnDao.isVtnFound(connection, vtnBean)) {
					final VRouterBean vRouterBean = new VRouterBean();
					vRouterBean.setVtnName(jsOb.get(
							VtnServiceOpenStackConsts.TENANT).getAsString());
					vRouterBean.setVrtName(jsOb.get(
							VtnServiceOpenStackConsts.ROUTER).getAsString());
					if (vRouterDao.isVrtFound(connection, vRouterBean)) {
						final VRouterInterfaceBean vInterfaceBean = new VRouterInterfaceBean();
						vInterfaceBean
								.setVtnName(jsOb.get(
										VtnServiceOpenStackConsts.TENANT)
										.getAsString());
						vInterfaceBean
								.setVrtName(jsOb.get(
										VtnServiceOpenStackConsts.ROUTER)
										.getAsString());
						vInterfaceBean
								.setVrtIfName(VtnServiceOpenStackConsts.IF_PREFIX
										+ jsOb.get(
												VtnServiceOpenStackConsts.INTERFACE)
												.getAsString());
						if (!vRouterInterfaceDao.isVrtIfFound(connection,
								vInterfaceBean)) {
							createErrorInfo(
									UncResultCode.UNC_NOT_FOUND.getValue(),
									getCutomErrorMessage(
											UncResultCode.UNC_NOT_FOUND
													.getMessage(),
											VtnServiceOpenStackConsts.APPLY_PORTS,
											createPortErrorMsg(jsOb)));
							return getInfo();
						}
					} else {
						createErrorInfo(
								UncResultCode.UNC_NOT_FOUND.getValue(),
								getCutomErrorMessage(
										UncResultCode.UNC_NOT_FOUND
												.getMessage(),
										VtnServiceOpenStackConsts.APPLY_PORTS,
										createPortErrorMsg(jsOb)));
						return getInfo();
					}
				} else {
					createErrorInfo(
							UncResultCode.UNC_NOT_FOUND.getValue(),
							getCutomErrorMessage(
									UncResultCode.UNC_NOT_FOUND.getMessage(),
									VtnServiceOpenStackConsts.APPLY_PORTS,
									createPortErrorMsg(jsOb)));
					return getInfo();

				}
			} else {
				final VtnBean vtnBean = new VtnBean();
				vtnBean.setVtnName(jsOb.get(VtnServiceOpenStackConsts.TENANT)
						.getAsString());
				if (vtnDao.isVtnFound(connection, vtnBean)) {
					final VBridgeBean vBridgeBean = new VBridgeBean();
					vBridgeBean.setVtnName(jsOb.get(
							VtnServiceOpenStackConsts.TENANT).getAsString());
					vBridgeBean.setVbrName(jsOb.get(
							VtnServiceOpenStackConsts.NETWORK).getAsString());
					if (vBridgeDao.isVbrFound(connection, vBridgeBean)) {
						final VRouterInterfaceBean vInterfaceBean = new VRouterInterfaceBean();
						vInterfaceBean
								.setVtnName(jsOb.get(
										VtnServiceOpenStackConsts.TENANT)
										.getAsString());
						vInterfaceBean.setVbrName(jsOb.get(
								VtnServiceOpenStackConsts.NETWORK)
								.getAsString());
						vInterfaceBean.setVrtIfId(Integer.parseInt(jsOb.get(
								VtnServiceOpenStackConsts.PORT).getAsString()));
						if (null != vRouterInterfaceDao.getVrouterName(
								connection, vInterfaceBean)) {
							continue;
						} else {
							final VBridgeInterfaceBean vbInterfaceBean = new VBridgeInterfaceBean();
							vbInterfaceBean.setVtnName(jsOb.get(
									VtnServiceOpenStackConsts.TENANT)
									.getAsString());
							vbInterfaceBean.setVbrName(jsOb.get(
									VtnServiceOpenStackConsts.NETWORK)
									.getAsString());
							vbInterfaceBean
									.setVbrIfName(VtnServiceOpenStackConsts.IF_PREFIX
											+ jsOb.get(
													VtnServiceOpenStackConsts.PORT)
													.getAsString());
							if (!vBridgeInterfaceDao.isVbrIfFound(connection,
									vbInterfaceBean)) {
								createErrorInfo(
										UncResultCode.UNC_NOT_FOUND.getValue(),
										getCutomErrorMessage(
												UncResultCode.UNC_NOT_FOUND
														.getMessage(),
												VtnServiceOpenStackConsts.APPLY_PORTS,
												createPortErrorMsg(jsOb)));
								return getInfo();
							}
						}
					} else {
						createErrorInfo(
								UncResultCode.UNC_NOT_FOUND.getValue(),
								getCutomErrorMessage(
										UncResultCode.UNC_NOT_FOUND
												.getMessage(),
										VtnServiceOpenStackConsts.APPLY_PORTS,
										createPortErrorMsg(jsOb)));
						return getInfo();
					}
				} else {
					createErrorInfo(
							UncResultCode.UNC_NOT_FOUND.getValue(),
							getCutomErrorMessage(
									UncResultCode.UNC_NOT_FOUND.getMessage(),
									VtnServiceOpenStackConsts.APPLY_PORTS,
									createPortErrorMsg(jsOb)));
					return getInfo();

				}
			}
		}
		return null;
	}

	/**
	 * When port information is invalid, create error massage.
	 * 
	 * @param info
	 *            - Port information reference
	 * @return - result as true or false
	 */
	private String createPortErrorMsg(final JsonObject info) {
		StringBuffer msg = new StringBuffer();

		/* Format1: apply_ports:{tenant:os_vtn_1,network:os_vbr_#2,port:1} */
		/* Format2: apply_ports:{tenant:os_vtn_1,router:os_vrt_#,interface:2} */

		/* Append "apply_ports:{tenant:" */
		// msg.append(VtnServiceOpenStackConsts.APPLY_PORTS);
		//msg.append(VtnServiceConsts.COLON);
		msg.append(VtnServiceConsts.OPEN_CURLY_BRACES);
		msg.append(VtnServiceOpenStackConsts.TENANT);
		msg.append(VtnServiceConsts.COLON);

		if (info.has(VtnServiceOpenStackConsts.TENANT)) {
			JsonElement tenantId = info.get(VtnServiceOpenStackConsts.TENANT);
			if (!(tenantId.isJsonNull() || tenantId.getAsString().isEmpty())) {
				/* Append "os_vtn_1" */
				msg.append(tenantId.getAsString());
			}
		}

		/* Append "," */
		msg.append(VtnServiceConsts.COMMA);

		if (info.has(VtnServiceOpenStackConsts.ROUTER)) {
			/* Append "router:" */
			msg.append(VtnServiceOpenStackConsts.ROUTER);
			msg.append(VtnServiceConsts.COLON);

			JsonElement routerId = info.get(VtnServiceOpenStackConsts.ROUTER);
			if (!(routerId.isJsonNull() || routerId.getAsString().isEmpty())) {
				/* Append "os_vrt_#" */
				msg.append(routerId.getAsString());
			}

			/* Append ",interface:" */
			msg.append(VtnServiceConsts.COMMA);
			msg.append(VtnServiceOpenStackConsts.INTERFACE);
			msg.append(VtnServiceConsts.COLON);

			if (info.has(VtnServiceOpenStackConsts.INTERFACE)) {
				JsonElement ifId = info
						.get(VtnServiceOpenStackConsts.INTERFACE);
				if (!(ifId.isJsonNull() || ifId.getAsString().isEmpty())) {
					/* Append "2" */
					msg.append(ifId.getAsString());
				}
			}
		} else {
			/* Append "network:" */
			msg.append(VtnServiceOpenStackConsts.NETWORK);
			msg.append(VtnServiceConsts.COLON);

			if (info.has(VtnServiceOpenStackConsts.NETWORK)) {
				JsonElement netId = info.get(VtnServiceOpenStackConsts.NETWORK);
				if (!(netId.isJsonNull() || netId.getAsString().isEmpty())) {
					/* Append "os_vbr_#2" */
					msg.append(netId.getAsString());
				}
			}

			/* Append ",port:" */
			msg.append(VtnServiceConsts.COMMA);
			msg.append(VtnServiceOpenStackConsts.PORT);
			msg.append(VtnServiceConsts.COLON);

			if (info.has(VtnServiceOpenStackConsts.PORT)) {
				JsonElement portId = info.get(VtnServiceOpenStackConsts.PORT);
				if (!(portId.isJsonNull() || portId.getAsString().isEmpty())) {
					/* Append "1" */
					msg.append(portId.getAsString());
				}
			}
		}

		/* Append "}" */
		msg.append(VtnServiceConsts.CLOSE_CURLY_BRACES);

		return msg.toString();
	}

	/**
	 * Check String is digital
	 * 
	 * @param seqnum
	 *            - String
	 * @return - true, String is digital
	 */
	private static boolean isValidSeqNum(String seqnum) {
		try {
			if (seqnum.substring(0, 1).equals("0")) {
				return false;
			}
			Integer.parseInt(seqnum);
			return true;
		} catch (NumberFormatException e) {
			return false;
		}
	}

	/**
	 * Handler method for GET operation of Filters
	 * 
	 * @see org.opendaylight.vtn.javaapi.resources.AbstractResource#get()
	 */
	@Override
	public int get(final JsonObject requestBody) {
		LOG.trace("Start FiltersResource#get()");

		int errorCode = UncResultCode.UNC_SUCCESS.getValue();

		Connection connection = null;

		try {
			connection = VtnServiceInitManager.getDbConnectionPoolMap()
					.getConnection();

			FlowListDao flowListDao = new FlowListDao();
			final List<String> list = flowListDao.getFlowList(connection);

			JsonArray flowList = new JsonArray();
			final JsonObject response = new JsonObject();
			if (list.size() > 0) {
				LOG.info("filter list retrieval is successful at UNC.");
				for (String filter_id : list) {
					flowList.add(new JsonPrimitive(filter_id));
				}
			}

			response.add(VtnServiceOpenStackConsts.FILTERS, flowList);
			setInfo(response);
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
		LOG.trace("Complete FiltersResource#get()");
		return errorCode;
	}
}
