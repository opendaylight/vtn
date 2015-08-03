/*
 * Copyright (c) 2013-2015 NEC Corporation
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
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
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
import org.opendaylight.vtn.javaapi.openstack.beans.VBridgeBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VBridgeInterfaceBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VtnBean;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;
import org.opendaylight.vtn.javaapi.openstack.dao.FlowFilterDao;
import org.opendaylight.vtn.javaapi.openstack.dao.FlowListDao;
import org.opendaylight.vtn.javaapi.openstack.dao.VBridgeDao;
import org.opendaylight.vtn.javaapi.openstack.dao.VBridgeInterfaceDao;
import org.opendaylight.vtn.javaapi.openstack.dao.VtnDao;
import org.opendaylight.vtn.javaapi.openstack.dbmanager.ResourceIdManager;
import org.opendaylight.vtn.javaapi.openstack.validation.PortResourceValidator;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;

/**
 * Resource class for handling DELETE requests for Port
 */
@UNCVtnService(path = VtnServiceOpenStackConsts.PORT_PATH)
public class PortResource extends AbstractResource {

	/* Logger instance */
	private static final Logger LOG = Logger.getLogger(PortResource.class
			.getName());

	@UNCField(VtnServiceOpenStackConsts.TENANT_ID)
	private String tenantId;

	@UNCField(VtnServiceOpenStackConsts.NET_ID)
	private String netId;

	@UNCField(VtnServiceOpenStackConsts.PORT_ID)
	private String portId;

	/**
	 * Getter of tenantId
	 * 
	 * @return
	 */
	public String getTenantId() {
		return tenantId;
	}

	/**
	 * Getter of netId
	 * 
	 * @return
	 */
	public String getNetId() {
		return netId;
	}

	/**
	 * Getter of portId
	 * 
	 * @return
	 */
	public String getPortId() {
		return portId;
	}

	/**
	 * Constructor that initialize the validation instance for current resource
	 * instance
	 */
	public PortResource() {
		setValidator(new PortResourceValidator(this));
	}

	/**
	 * Handler method for DELETE operation of Port
	 * 
	 * @see org.opendaylight.vtn.javaapi.resources.AbstractResource#delete()
	 */
	@Override
	public int delete() {
		LOG.trace("Start PortResource#delete()");

		int errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();

		boolean isCommitRequired = false;
		Connection connection = null;

		try {
			connection = VtnServiceInitManager.getDbConnectionPoolMap()
					.getConnection();

			/*
			 * Check for instances that they exists or not, if not then return
			 * 404 error
			 */
			if (checkForNotFoundResources(connection)) {
				final VBridgeInterfaceBean vInterfaceBean = new VBridgeInterfaceBean();
				vInterfaceBean.setVtnName(getTenantId());
				vInterfaceBean.setVbrName(getNetId());
				vInterfaceBean.setVbrIfName(VtnServiceOpenStackConsts.IF_PREFIX
						+ getPortId());

				final String counter = getPortId().replace(
						VtnServiceOpenStackConsts.IF_PREFIX,
						VtnServiceConsts.EMPTY_STRING);
				try {
					vInterfaceBean.setVbrIfId(Integer.parseInt(counter));
				} catch (final NumberFormatException e) {
					LOG.debug("Resource Id was not auto-generated during Create operation : "
							+ counter);
					vInterfaceBean.setVbrIfId(0);
				}

				final VBridgeInterfaceDao vBridgeInterfaceDao = new VBridgeInterfaceDao();

				/*
				 * retrieve map_type if vlan-map required to be delete
				 */
				final String mapType = vBridgeInterfaceDao.getMapType(
						connection, vInterfaceBean);

				/*
				 * retrieve logical_port_id if vlan-map required to be delete
				 */
				String logicalPortId = vBridgeInterfaceDao.getLogicalPortId(
						connection, vInterfaceBean);

				final ResourceIdManager resourceIdManager = new ResourceIdManager();

				final FreeCounterBean freeCounterBean = new FreeCounterBean();
				freeCounterBean
						.setResourceId(VtnServiceOpenStackConsts.PORT_RES_ID);
				freeCounterBean.setVtnName(getTenantId());
				freeCounterBean.setResourceCounter(vInterfaceBean.getVbrIfId());

				if (resourceIdManager.deleteResourceId(connection,
						freeCounterBean, vInterfaceBean)) {
					/*
					 * delete vlan-map or vbridge-interface on the basis of
					 * entry from database for map_type
					 */
					if (mapType != null) {
						final RestResource restResource = new RestResource();
						if (mapType
								.equalsIgnoreCase(VtnServiceJsonConsts.VLANMAP)) {
							LOG.info("vlan-map operation were performed during create Port. vlan-map deletion is required.");
							int mapModeValue;
							// read property for map-mode
							mapModeValue = Integer
									.parseInt(VtnServiceInitManager
											.getConfigurationMap()
											.getMapModeValue());
							/*
							 * map-mode value : 0 - delete vlan-map map-mode
							 * value : 1 - delete all vlan-map map-mode value :
							 * 2 - do nothing
							 */
							if (mapModeValue == 0) {
								errorCode = deleteVlanMapModeZero(
										logicalPortId, restResource);
							} else if (mapModeValue == 1) {
								// retrieve list of all vlan-maps
								JsonArray vlanmaps = getVlanMapList(
										restResource).get(
										VtnServiceJsonConsts.VLANMAPS)
										.getAsJsonArray();
								if (vlanmaps != null && vlanmaps.size() != 0) {
									boolean counterDeletion = deleteIfCounters(
											connection, vInterfaceBean,
											vBridgeInterfaceDao,
											resourceIdManager, freeCounterBean);
									if (counterDeletion) {
										// delete all vlan-map one by one
										for (JsonElement vlanmap : vlanmaps) {
											logicalPortId = vlanmap
													.getAsJsonObject()
													.get(VtnServiceJsonConsts.VLANMAPID)
													.getAsString();
											errorCode = deleteVlanMap(
													restResource, logicalPortId);
											if (errorCode != UncResultCode.UNC_SUCCESS
													.getValue()) {
												/*
												 * break if error occurred while
												 * deletion of vlan-map
												 */
												break;
											}
										}
									}
								} else {
									LOG.info("No vlan-map is configured");
								}
							} else if (mapModeValue == 2) {
								LOG.debug("map-mode value : 2");
								errorCode = UncResultCode.UNC_SUCCESS
										.getValue();
							} else {
								throw new IllegalArgumentException(
										"incorrect map-mode value.");
							}
						} else if (mapType
								.equalsIgnoreCase(VtnServiceJsonConsts.PORTMAP)) {
							LOG.info("port-map operation were performed during create Port. vBridge interface deletion is required.");
							errorCode = deleteVBridgeInterface(restResource);
						} else {
							LOG.error("Invalid Map type is resolved");
						}
						if (errorCode == UncCommonEnum.UncResultCode.UNC_SUCCESS
								.getValue()) {
							LOG.error("port-map/vlan-map Deletion successful at UNC.");
							isCommitRequired = true;
						} else {
							errorCode = UncResultCode.UNC_SERVER_ERROR
									.getValue();
							LOG.error("port-map/vlan-map Deletion failed at UNC.");
						}
						checkForSpecificErrors(restResource.getInfo());
					} else {
						LOG.error("Map type is not resolved");
					}
				} else {
					LOG.error("Deletion operation from database is falied.");
				}
			} else {
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
			if (connection != null && !isCommitRequired) {
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
		LOG.trace("Complete PortResource#delete()");
		return errorCode;
	}

	/**
	 * Delete all counters for specific vtn_name and vbr_name
	 * 
	 * @param connection
	 * @param vInterfaceBean
	 * @param vBridgeInterfaceDao
	 * @param resourceIdManager
	 * @param freeCounterBean
	 * @return
	 * @throws SQLException
	 */
	private boolean deleteIfCounters(Connection connection,
			final VBridgeInterfaceBean vInterfaceBean,
			final VBridgeInterfaceDao vBridgeInterfaceDao,
			final ResourceIdManager resourceIdManager,
			final FreeCounterBean freeCounterBean) throws SQLException {
		boolean counterDeletion = true;
		for (Integer ifId : vBridgeInterfaceDao.getVbrIfIds(connection,
				vInterfaceBean)) {
			freeCounterBean.setResourceCounter(ifId);
			vInterfaceBean.setVbrIfId(ifId);
			vInterfaceBean.setVbrIfName(VtnServiceOpenStackConsts.IF_PREFIX
					+ ifId);
			if (resourceIdManager.deleteResourceId(connection, freeCounterBean,
					vInterfaceBean)) {
				LOG.debug("counter is deleted for : " + ifId);
			} else {
				LOG.info("counter deletion is failed for : " + ifId);
				counterDeletion = false;
			}
		}
		return counterDeletion;
	}

	/**
	 * Perform delete vlan-map operations in case of map-mode is 0
	 * 
	 * @param errorCode
	 * @param logicalPortId
	 * @param restResource
	 * @return
	 */
	private int deleteVlanMapModeZero(String logicalPortId,
			final RestResource restResource) {
		int errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();
		;
		if (logicalPortId != null
				&& !logicalPortId.equalsIgnoreCase(VtnServiceJsonConsts.NONE)) {
			if (!logicalPortId.equalsIgnoreCase(VtnServiceJsonConsts.NOLPID)) {
				/*
				 * if logical_port_id not provided at the time of creation of
				 * vlan-map
				 */
				logicalPortId = VtnServiceJsonConsts.LPID
						+ VtnServiceConsts.HYPHEN + logicalPortId;
			}
			errorCode = deleteVlanMap(restResource, logicalPortId);
		}
		return errorCode;
	}

	/**
	 * Delete Port-Map at UNC
	 * 
	 * @param restResource
	 *            - RestResource instance
	 * @return - erorrCode, 200 for Success
	 */
	private int deleteVBridgeInterface(RestResource restResource) {
		int errorCode;
		/*
		 * execute delete port-map request
		 */
		StringBuilder sb = new StringBuilder();
		sb.append(VtnServiceOpenStackConsts.VTN_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getTenantId());
		sb.append(VtnServiceOpenStackConsts.VBRIDGE_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getNetId());
		sb.append(VtnServiceOpenStackConsts.INTERFACE_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(VtnServiceOpenStackConsts.IF_PREFIX);
		sb.append(getPortId());

		restResource.setPath(sb.toString());
		restResource.setSessionID(getSessionID());
		restResource.setConfigID(getConfigID());
		errorCode = restResource.delete();
		return errorCode;
	}

	/**
	 * Create Vlan-Map at UNC
	 * 
	 * @param restResource
	 *            - RestResource instance
	 * @return - erorrCode, 200 for Success
	 */
	private int deleteVlanMap(RestResource restResource, String logicalPortId) {
		/*
		 * execute delete vlan-map request
		 */
		StringBuilder sb = new StringBuilder();
		sb.append(VtnServiceOpenStackConsts.VTN_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getTenantId());
		sb.append(VtnServiceOpenStackConsts.VBRIDGE_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getNetId());
		sb.append(VtnServiceOpenStackConsts.VLANMAP_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(logicalPortId);

		restResource.setPath(sb.toString());
		restResource.setSessionID(getSessionID());
		restResource.setConfigID(getConfigID());
		return restResource.delete();
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
		VtnBean vtnBean = new VtnBean();
		vtnBean.setVtnName(getTenantId());
		if (new VtnDao().isVtnFound(connection, vtnBean)) {
			VBridgeBean vBridgeBean = new VBridgeBean();
			vBridgeBean.setVtnName(getTenantId());
			vBridgeBean.setVbrName(getNetId());
			if (new VBridgeDao().isVbrFound(connection, vBridgeBean)) {
				VBridgeInterfaceBean vInterfaceBean = new VBridgeInterfaceBean();
				vInterfaceBean.setVtnName(getTenantId());
				vInterfaceBean.setVbrName(getNetId());
				vInterfaceBean.setVbrIfName(VtnServiceOpenStackConsts.IF_PREFIX
						+ getPortId());
				if (new VBridgeInterfaceDao().isVbrIfFound(connection,
						vInterfaceBean)) {
					resourceFound = true;
				} else {
					createErrorInfo(
							UncResultCode.UNC_NOT_FOUND.getValue(),
							getCutomErrorMessage(
									UncResultCode.UNC_NOT_FOUND.getMessage(),
									VtnServiceOpenStackConsts.PORT_ID,
									getPortId()));
				}
			} else {
				createErrorInfo(
						UncResultCode.UNC_NOT_FOUND.getValue(),
						getCutomErrorMessage(
								UncResultCode.UNC_NOT_FOUND.getMessage(),
								VtnServiceOpenStackConsts.NET_ID, getNetId()));
			}
		} else {
			createErrorInfo(
					UncResultCode.UNC_NOT_FOUND.getValue(),
					getCutomErrorMessage(
							UncResultCode.UNC_NOT_FOUND.getMessage(),
							VtnServiceOpenStackConsts.TENANT_ID, getTenantId()));
		}
		return resourceFound;
	}

	/**
	 * Retrieve list of vlan-map UNC
	 * 
	 * @param requestBody
	 *            - OpenStack request body
	 * @param restResource
	 *            - RestResource instance
	 * @return - erorrCode, 200 for Success
	 */
	private JsonObject getVlanMapList(RestResource restResource) {
		StringBuilder sb = new StringBuilder();
		sb.append(VtnServiceOpenStackConsts.VTN_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getTenantId());
		sb.append(VtnServiceOpenStackConsts.VBRIDGE_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getNetId());
		sb.append(VtnServiceOpenStackConsts.VLANMAP_PATH);

		restResource.setPath(sb.toString());
		restResource.setSessionID(getSessionID());
		restResource.setConfigID(getConfigID());

		restResource.get(new JsonObject());

		return restResource.getInfo();
	}

	/**
	 * Handler method for PUT operation of Port
	 * 
	 * @see org.opendaylight.vtn.javaapi.resources.AbstractResource#put(com.
	 *      google.gson.JsonObject)
	 */
	@Override
	public int put(JsonObject requestBody) {
		LOG.trace("Start PortResource#put()");

		int errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();

		Connection connection = null;
		boolean isCommitRequired = false;

		try {
			connection = VtnServiceInitManager.getDbConnectionPoolMap()
					.getConnection();

			/*
			 * Check for instances that they exists or not, if not then return
			 * 404 error
			 */
			if (checkForNotFoundResources(connection)) {

				final RestResource restResource = new RestResource();

				errorCode = updatePort(requestBody, restResource, connection);

				if (errorCode == UncCommonEnum.UncResultCode.UNC_SUCCESS
						.getValue()) {
					isCommitRequired = true;
					LOG.info("VTN Update at UNC is successful.");
				} else {
					LOG.error("VTN Update at UNC is failed.");
				}
				checkForSpecificErrors(restResource.getInfo());
			} else {
				LOG.error("Resource not found error.");
			}

			/*
			 * If all processing are OK, the commit all the database transaction
			 * made for current connection. Otherwise do the roll-back
			 */
			if (isCommitRequired) {
				// connection.commit();
				setOpenStackConnection(connection);
				LOG.info("Resource update successful in database.");
			} else {
				connection.rollback();
				LOG.info("Resource update is roll-backed.");
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
			if (connection != null && !isCommitRequired) {
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
		LOG.trace("Complete PortResource#put()");
		return errorCode;
	}

	/**
	 * Perform Filter related operations at UNC
	 * 
	 * @param requestBody
	 *            - OpenStack request body
	 * @param restResource
	 *            - RestResource instance
	 * @param connection
	 *            - Database Connection instance
	 * @return - erorrCode, 200 for Success
	 * @throws SQLException 
	 */
	private int updatePort(JsonObject requestBody,
			RestResource restResource,
			Connection connection) throws SQLException {
		int errorCode = UncResultCode.UNC_SUCCESS.getValue();
		final FlowFilterDao flowFilterDao = new FlowFilterDao();
		boolean hasFilterIn = true;
		boolean inputEmpty = false;

		if (!requestBody.has(VtnServiceOpenStackConsts.FILTERS)) {
			return UncResultCode.UNC_SUCCESS.getValue();
		}

		Set<String> insertFilterIds = new HashSet<String>();
		Set<String> deleteFilterIds = new HashSet<String>();
		JsonArray filters = requestBody
				.getAsJsonArray(VtnServiceOpenStackConsts.FILTERS);
		if (filters.size() == 0) {
			inputEmpty = true;
		}

		/*
		 * Delete duplicate flow filter.
		 */
		FlowListDao flowListDao = new FlowListDao();
		FlowListBean flowListBean = new FlowListBean();
		Iterator<JsonElement> iterator = filters.iterator();
		while (iterator.hasNext()) {
			JsonElement element = iterator.next();
			insertFilterIds.add(element.getAsString());

			// check filter id if exist.
			flowListBean.setFlName(element.getAsString());
			if (!flowListDao.isFlowListFound(connection, flowListBean)) {
				createErrorInfo(
						UncResultCode.UNC_NOT_FOUND.getValue(),
						getCutomErrorMessage(
								UncResultCode.UNC_NOT_FOUND.getMessage(),
								VtnServiceOpenStackConsts.FILTER_RES_ID,
								element.getAsString()));
				LOG.error("Resource not found error(filterId: " + 
						element.getAsString() + ").");
				return UncResultCode.UNC_NOT_FOUND.getValue();
			}
			
			LOG.debug("filter_id: %s", element.getAsString());
		}

		/*
		 * if the port link with interface of router.
		 */
		FlowFilterVrtBean flowFilterVrtBean = new FlowFilterVrtBean();
		flowFilterVrtBean.setVtnName(tenantId);
		flowFilterVrtBean.setVbrName(netId);
		flowFilterVrtBean.setVrtIfName(VtnServiceOpenStackConsts.IF_PREFIX + portId);
		String vrouter = flowFilterDao.isLinkedWithRouter(connection,
				flowFilterVrtBean);

		/*
		 * Get list of filter id that related on port
		 */
		List<String>  listFilterIds = null;
		FlowFilterVbrBean flowFilterVbrBean = new FlowFilterVbrBean();
		flowFilterVbrBean.setVtnName(tenantId);
		flowFilterVbrBean.setVbrName(netId);
		flowFilterVbrBean.setVbrIfName(VtnServiceOpenStackConsts.IF_PREFIX + portId);
		listFilterIds = flowFilterDao.getFlowFiltersByPort(connection,
				flowFilterVbrBean, null != vrouter);
		if (listFilterIds.isEmpty()) {
			hasFilterIn = false;
		}

		/*
		 * find filte id to delete. then filte id to add in deleteFilterIds
		 */
		for (String item : listFilterIds) {
			if(!insertFilterIds.remove(item)) {
				deleteFilterIds.add(item);
			}
		}

		if (!deleteFilterIds.isEmpty()) {
			int status = -1;
			if (null != vrouter) {
				ArrayList<FlowFilterVrtBean> vrouterList = new ArrayList<FlowFilterVrtBean>();
				for (String filterID : deleteFilterIds) {
					FlowFilterVrtBean vrtBean = new FlowFilterVrtBean();
					vrtBean.setVtnName(getTenantId());
					vrtBean.setVrtName(vrouter);
					vrtBean.setVrtIfName(VtnServiceOpenStackConsts.IF_PREFIX + getPortId());
					vrtBean.setFlName(filterID);
					vrouterList.add(vrtBean);
				}

				// delete db for flow filter.
				status = flowFilterDao.deleteVrouterFilterInfo(connection,
						vrouterList);
			} else {
				ArrayList<FlowFilterVbrBean> vbridgeList = new ArrayList<FlowFilterVbrBean>();
				for (String filterID : deleteFilterIds) {
					FlowFilterVbrBean vbrBean = new FlowFilterVbrBean();
					vbrBean.setVtnName(getTenantId());
					vbrBean.setVbrName(getNetId());
					vbrBean.setVbrIfName(VtnServiceOpenStackConsts.IF_PREFIX + getPortId());
					vbrBean.setFlName(filterID);
					vbridgeList.add(vbrBean);
				}

				// delete db for flow filter.
				status = flowFilterDao.deleteVbridgeFilterInfo(connection,
						vbridgeList);
			}

			if (1 == status) {
				LOG.info("Resource delete successful at database operation.");
				for (String filterID : deleteFilterIds) {
					String priority = filterID.substring(5, 9);
					String seqnum =  String.valueOf((Integer.parseInt(priority, 16)));
					// send request.
					errorCode = deleteFlowFilter(restResource,
							getTenantId(), getNetId(),
							VtnServiceOpenStackConsts.IF_PREFIX + getPortId(),
							seqnum);
					if (UncResultCode.UNC_SUCCESS.getValue() != errorCode) {
						LOG.error("Failed to delete flow filter entry("
								+ filterID + ") at UNC.");
						return errorCode;
					}
				}
				
				if (inputEmpty) {
					// send request.
					errorCode = deleteFlowFilterIn(restResource, getTenantId(),
							getNetId(),
							VtnServiceOpenStackConsts.IF_PREFIX + getPortId());
					if (UncResultCode.UNC_SUCCESS.getValue() != errorCode) {
						LOG.error("Failed to delete flow filter in at UNC.");
						return errorCode;
					}
				}
				
			} else {
				LOG.error("Failed to delete Resource(filter for interface "
						+ "of vbridge) at database operation.");
				errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();
			}
		}

		if (!insertFilterIds.isEmpty()) {
			if (!hasFilterIn) {
				/*
				 * Create request body for type of filter creation
				 */
				JsonObject flowFilterInRequestBody = new JsonObject();
				flowFilterInRequestBody.add(VtnServiceJsonConsts.FLOWFILTER,
											new JsonObject());
				flowFilterInRequestBody
						.get(VtnServiceJsonConsts.FLOWFILTER)
						.getAsJsonObject()
						.addProperty(VtnServiceJsonConsts.FFTYPE,
								VtnServiceJsonConsts.IN);

				// send request.
				errorCode = createFlowFilterIn(requestBody, restResource,
						flowFilterInRequestBody);
				
				if (UncResultCode.UNC_SUCCESS.getValue() != errorCode) {
					LOG.error("Failed to create type(in) of flow filter at UNC.");
					return errorCode;
				}
				LOG.info("filter in Creation at UNC is successful.");
			}

			int status = -1;
			if (null != vrouter) {
				ArrayList<FlowFilterVrtBean> vrouterList = new ArrayList<FlowFilterVrtBean>();
				for (String filterID : insertFilterIds) {
					FlowFilterVrtBean vrtBean = new FlowFilterVrtBean();
					vrtBean.setVtnName(getTenantId());
					vrtBean.setVrtName(vrouter);
					vrtBean.setVrtIfName(VtnServiceOpenStackConsts.IF_PREFIX + getPortId());
					vrtBean.setVbrName(getNetId());
					vrtBean.setFlName(filterID);
					vrouterList.add(vrtBean);
				}

				// Insert db for flow filter.
				status = flowFilterDao.insertVrouterFilterInfo(connection,
						vrouterList);
			} else {
				ArrayList<FlowFilterVbrBean> vbridgeList = new ArrayList<FlowFilterVbrBean>();
				for (String filterID : insertFilterIds) {
					FlowFilterVbrBean vbrBean = new FlowFilterVbrBean();
					vbrBean.setVtnName(getTenantId());
					vbrBean.setVbrName(getNetId());
					vbrBean.setVbrIfName(VtnServiceOpenStackConsts.IF_PREFIX + getPortId());
					vbrBean.setFlName(filterID);
					vbridgeList.add(vbrBean);
				}

				// Insert db for flow filter.
				status = flowFilterDao.insertVbridgeFilterInfo(connection,
						vbridgeList);
			}

			if (1 == status) {
				LOG.info("Resource insertion successful at database "
						+ "operation.");
				for (String filterID : insertFilterIds) {
					/*
					 * Create request body for flow filter creation
					 */
					String action = VtnServiceOpenStackConsts.S_PASS;
					if (VtnServiceOpenStackConsts.X_DROP == filterID
							.charAt(4)) {
						action = VtnServiceOpenStackConsts.S_DROP;
					}

					String priority = filterID.substring(5, 9);

					JsonObject flowFilterRequestBody = new JsonObject();
					flowFilterRequestBody.add(
							VtnServiceJsonConsts.FLOWFILTERENTRY,
							new JsonObject());
					JsonObject flowfilter = flowFilterRequestBody
							.getAsJsonObject(VtnServiceJsonConsts.FLOWFILTERENTRY);
					flowfilter
							.addProperty(VtnServiceJsonConsts.SEQNUM,
									String.valueOf((Integer.parseInt(
											priority, 16))));
					flowfilter.addProperty(VtnServiceJsonConsts.FLNAME,
							filterID);
					flowfilter.addProperty(VtnServiceJsonConsts.ACTIONTYPE,
							action);
					flowfilter.add(VtnServiceJsonConsts.REDIRECTDST,
							new JsonObject());

					// send request.
					errorCode = createFlowFilter(requestBody, restResource,
							flowFilterRequestBody);
					if (UncResultCode.UNC_SUCCESS.getValue() != errorCode) {
						LOG.error("Failed to create flow filter entry("
								+ filterID + ") at UNC.");
						return errorCode;
					}
				}
			} else {
				LOG.error("Failed to insert Resource(filter for interface "
						+ "of vbridge) at database operation.");
				errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();
				return errorCode;
			}
		}

		return errorCode;
	}

	/**
	 * Create flow filter in at UNC
	 * 
	 * @param requestBody
	 *            - OpenStack request body
	 * @param restResource
	 *            - RestResource instance
	 * @param flowFilterInRequestBody
	 *            - Basic request body
	 * @return - erorrCode, 200 for Success
	 */
	private int createFlowFilterIn(JsonObject requestBody,
			RestResource restResource, JsonObject flowFilterInRequestBody) {
		StringBuilder sb = new StringBuilder();
		sb.append(VtnServiceOpenStackConsts.VTN_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getTenantId());
		sb.append(VtnServiceOpenStackConsts.VBRIDGE_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getNetId());
		sb.append(VtnServiceOpenStackConsts.INTERFACE_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(VtnServiceOpenStackConsts.IF_PREFIX + getPortId());
		sb.append(VtnServiceOpenStackConsts.FLOWFILTER_PATH);

		restResource.setPath(sb.toString());
		restResource.setSessionID(getSessionID());
		restResource.setConfigID(getConfigID());
		return restResource.post(flowFilterInRequestBody);
	}

	/**
	 * Create flow filter at UNC
	 * 
	 * @param requestBody
	 *            - OpenStack request body
	 * @param restResource
	 *            - RestResource instance
	 * @param flowfilterEntryRequest
	 *            - Basic request body
	 * @return - erorrCode, 200 for Success
	 */
	private int createFlowFilter(JsonObject requestBody,
			RestResource restResource, JsonObject flowfilterEntryRequest) {

		StringBuilder sb = new StringBuilder();
		sb.append(VtnServiceOpenStackConsts.VTN_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getTenantId());
		sb.append(VtnServiceOpenStackConsts.VBRIDGE_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getNetId());
		sb.append(VtnServiceOpenStackConsts.INTERFACE_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(VtnServiceOpenStackConsts.IF_PREFIX + getPortId());
		sb.append(VtnServiceOpenStackConsts.FLOWFILTER_PATH);
		sb.append(VtnServiceOpenStackConsts.IN_PATH);
		sb.append(VtnServiceOpenStackConsts.FLOWFILTER_ENTRY_PATH);

		restResource.setPath(sb.toString());
		restResource.setSessionID(getSessionID());
		restResource.setConfigID(getConfigID());
		return restResource.post(flowfilterEntryRequest);
	}

	/**
	 * Delete Flow Filter Entry at UNC
	 * 
	 * @param restResource
	 *            - RestResource instance
	 * @param vtnName
	 *            - VTN name
	 * @param vbrName
	 *            - vBridge name
	 * @param ifName
	 *            - Interface name
	 * @param seqNum
	 *            - Sequence number
	 * @return - erorrCode, 200 for Success
	 */
	private int deleteFlowFilter(RestResource restResource,
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
	 * Delete flow filter in at UNC
	 * 
	 * @param restResource
	 *            - RestResource instance
	 * @param vtnName
	 *            - VTN name
	 * @param vbrName
	 *            - vBridge name
	 * @param ifName
	 *            - Interface name
	 * @return - erorrCode, 200 for Success
	 */
	private int deleteFlowFilterIn(RestResource restResource, String vtnName,
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
}
