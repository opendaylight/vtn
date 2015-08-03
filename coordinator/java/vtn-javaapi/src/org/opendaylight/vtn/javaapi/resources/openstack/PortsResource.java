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
import org.opendaylight.vtn.javaapi.openstack.beans.FlowListBean;
import org.opendaylight.vtn.javaapi.openstack.beans.FreeCounterBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VBridgeBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VBridgeInterfaceBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VtnBean;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;
import org.opendaylight.vtn.javaapi.openstack.convertor.MapResourceGenerator;
import org.opendaylight.vtn.javaapi.openstack.convertor.VbrResourcesGenerator;
import org.opendaylight.vtn.javaapi.openstack.dao.DestinationControllerDao;
import org.opendaylight.vtn.javaapi.openstack.dao.FlowFilterDao;
import org.opendaylight.vtn.javaapi.openstack.dao.FlowListDao;
import org.opendaylight.vtn.javaapi.openstack.dao.FreeCounterDao;
import org.opendaylight.vtn.javaapi.openstack.dao.VBridgeDao;
import org.opendaylight.vtn.javaapi.openstack.dao.VBridgeInterfaceDao;
import org.opendaylight.vtn.javaapi.openstack.dao.VtnDao;
import org.opendaylight.vtn.javaapi.openstack.dao.CommonDao;
import org.opendaylight.vtn.javaapi.openstack.dbmanager.ResourceIdManager;
import org.opendaylight.vtn.javaapi.openstack.validation.PortResourceValidator;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;

/**
 * Resource class for handling POST requests for Port
 */
@UNCVtnService(path = VtnServiceOpenStackConsts.PORTS_PATH)
public class PortsResource extends AbstractResource {

	/* Logger instance */
	private static final Logger LOG = Logger.getLogger(PortsResource.class
			.getName());

	private String controllerId = null;
	private String logicalPortId = null;

	@UNCField(VtnServiceOpenStackConsts.TENANT_ID)
	private String tenantId;

	@UNCField(VtnServiceOpenStackConsts.NET_ID)
	private String netId;

	/**
	 * Constructor that initialize the validation instance for current resource
	 * instance
	 */
	public PortsResource() {
		setValidator(new PortResourceValidator(this));
	}

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
	 * Handler method for POST operation of Port
	 * 
	 * @see org.opendaylight.vtn.javaapi.resources.AbstractResource#post(com
	 *      .google.gson.JsonObject)
	 */
	@Override
	public int post(JsonObject requestBody) {
		LOG.trace("Start NetworksResource#post()");

		int errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();

		boolean isCommitRequired = false;
		String generatedVbrIfName = null;

		int counter = -1;
		Connection connection = null;
		CommonDao comDao = new CommonDao();
		final VBridgeInterfaceBean vInterfaceBean = 
				new VBridgeInterfaceBean();

		try {
			connection = VtnServiceInitManager.getDbConnectionPoolMap()
					.getConnection();

			/*
			 * Check for instances that they exists or not, if not then return
			 * 404 error
			 */
			if (checkForNotFoundResources(connection)) {
				final ResourceIdManager resourceIdManager = 
						new ResourceIdManager();
				/*
				 * generate "id" is it is not present in request body
				 */
				if (!requestBody.has(VtnServiceOpenStackConsts.ID)) {
					LOG.info("Resource id auto-generation is required.");

					final FreeCounterBean freeCounterBean = 
							new FreeCounterBean();
					freeCounterBean.setResourceId(VtnServiceOpenStackConsts
							.PORT_RES_ID);
					freeCounterBean.setVtnName(getTenantId());

					counter = resourceIdManager.getResourceCounter(connection,
							freeCounterBean);
					if (counter != -1) {
						LOG.debug("Resource id auto-generation is " +
								"successfull : " + counter);
						// if id is generated successfully
						generatedVbrIfName = VtnServiceOpenStackConsts.IF_PREFIX
								+ counter;
						requestBody.addProperty(VtnServiceOpenStackConsts.ID,
								generatedVbrIfName);

						vInterfaceBean.setVbrIfId(counter);
						vInterfaceBean.setVbrIfStatus(VtnServiceOpenStackConsts.AUTOGENERATED);
					}
				} else {
					generatedVbrIfName = VtnServiceOpenStackConsts.IF_PREFIX
							+ requestBody.get(VtnServiceOpenStackConsts.ID)
									.getAsString();
					requestBody.addProperty(VtnServiceOpenStackConsts.ID,
							generatedVbrIfName);
					counter = 0;
					//init if id
					vInterfaceBean.setVbrIfId(counter);
					vInterfaceBean.setVbrIfStatus(VtnServiceOpenStackConsts.MANUALINPUT);
					if (comDao.isValidInteger(generatedVbrIfName.substring(6))) {
						//update if id
						vInterfaceBean.setVbrIfId(Integer
										.parseInt(generatedVbrIfName.substring(6)));
						final FreeCounterBean freeCounterBean = new FreeCounterBean();
						freeCounterBean
									.setResourceId(VtnServiceOpenStackConsts.PORT_RES_ID);
						freeCounterBean.setVtnName(getTenantId());
						freeCounterBean.setResourceCounter(Integer
									.parseInt(generatedVbrIfName.substring(6)));
						final FreeCounterDao freeCounterDao = new FreeCounterDao();
						if (freeCounterDao.isCounterFound(connection,
									freeCounterBean)) {
							freeCounterDao.deleteCounter(connection,freeCounterBean);
						}	
					}	
				}

				LOG.debug("Counter : " + counter);
				LOG.debug("if_name : " + generatedVbrIfName);
				LOG.debug("if_status : " + vInterfaceBean.getVbrIfStatus());

				if (counter >= 0) {
					/*
					 * resource insertion in database, if is is successful then
					 * continue to execute operations at UNC. Otherwise return
					 * HTTP 409
					 */
					vInterfaceBean.setVtnName(getTenantId());
					vInterfaceBean.setVbrName(getNetId());
					vInterfaceBean.setVbrIfName(generatedVbrIfName);
					/*
					 * initialize with default port-map setting. It should be
					 * updated when vlan-map operation is performed
					 */
					vInterfaceBean.setMapType(VtnServiceJsonConsts.PORTMAP);
					vInterfaceBean.setLogicalPortId(VtnServiceJsonConsts.NONE);

					final VBridgeInterfaceDao vInterfaceDao = 
							new VBridgeInterfaceDao();
					int status = vInterfaceDao.insert(connection,
							vInterfaceBean);

					if (status == 1) {
						LOG.info("Resource insertion successful at " +
								"database operation.");

						final RestResource restResource = new RestResource();

						final JsonElement port = requestBody
								.get(VtnServiceOpenStackConsts.PORT);

						if (port.isJsonNull()
								|| port.getAsString().equalsIgnoreCase(
										VtnServiceOpenStackConsts.NULL)) {
							/*
							 * if port is specified as NULL then vlan-map
							 * operations are required to be performed
							 */
							errorCode = performVlanMapOperations(requestBody,
									restResource);

							if (errorCode == UncCommonEnum.UncResultCode
									.UNC_SUCCESS.getValue()) {
								LOG.info("vlan-map Creation at UNC is " +
										"successful.");

								vInterfaceBean.setMapType(VtnServiceJsonConsts
										.VLANMAP);
								vInterfaceBean.setLogicalPortId(logicalPortId);

								/**
								 * if operation is performed for vlan-map then
								 * update map_type and logical_port_id in
								 * database
								 */
								if (vInterfaceDao.updateVlanMapInfo(connection,
										vInterfaceBean) == 1) {
									isCommitRequired = true;
									if (counter != 0) {
										final JsonObject response = 
												new JsonObject();
										response.addProperty(
												VtnServiceOpenStackConsts.ID,
												String.valueOf(counter));
										setInfo(response);
									}
									LOG.info("map_type and logical_port_id " +
											"successfully updated in " +
											"database.");
								} else {
									errorCode = UncResultCode.UNC_SERVER_ERROR
											.getValue();
									LOG.error("map_type and logical_port_id " +
											"update failed in database.");
								}
							} else {
								LOG.error("vlan-map Creation at UNC is " +
										"failed.");
							}
						} else {
							/*
							 * if port is not specified as NULL then port-map
							 * operations are required to be performed
							 */
							if (setControllerId(connection, requestBody) 
									== Boolean.TRUE) {
								errorCode = performPortMapOperations(
										requestBody, restResource);
								/*
								 * Filter operations.
								 */
								if (errorCode == UncCommonEnum.UncResultCode
										.UNC_SUCCESS.getValue()) {
									LOG.info("port-map Creation at UNC is "
											+ "successful.");
									/*
									 * if filters is specified as no empty, then
									 * filter operations are required to
									 * performed.
									 */
									if (requestBody
											.has(VtnServiceOpenStackConsts
													.FILTERS)
											&& requestBody
												.get(VtnServiceOpenStackConsts
														.FILTERS)
															.getAsJsonArray()
																.size() > 0) {
										errorCode = performFilterOperations(
												requestBody, restResource,
												connection);
										if (errorCode == 
												UncCommonEnum.UncResultCode
													.UNC_SUCCESS
														.getValue()) {
											LOG.info("Filter Creation at "
													+ "UNC is successful.");
										} else {
											errorCode = UncResultCode
													.UNC_SERVER_ERROR
														.getValue();
											LOG.error("Filter Creation at "
													+ "UNC is failed.");
										}
									}
								} else {
									errorCode = UncResultCode.UNC_SERVER_ERROR
											.getValue();
									LOG.error("port-map Creation at UNC is " +
											"failed.");
								}

								if (errorCode == UncCommonEnum.UncResultCode
										.UNC_SUCCESS.getValue()) {
									isCommitRequired = true;
									if (counter != 0) {
										final JsonObject response = 
												new JsonObject();
										response.addProperty(
												VtnServiceOpenStackConsts.ID,
												String.valueOf(counter));
										setInfo(response);
									}
								}
							} else {
								LOG.error("Error ocurred while setting " +
										"controller_id");
							}
						}
						checkForSpecificErrors(restResource.getInfo());
					} else {
						LOG.error("Resource insertion failed at " +
								"database operation.");
					}
				} else {
					LOG.error("Error occurred while generation of id.");
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
				if (counter != 0) {
					generatedVbrIfName = String.valueOf(counter);
				}
				createErrorInfo(
						UncResultCode.UNC_CONFLICT_FOUND.getValue(),
						getCutomErrorMessage(
								UncResultCode.UNC_CONFLICT_FOUND.getMessage(),
								VtnServiceOpenStackConsts.IF_ID,
								generatedVbrIfName));
			} else {
				createErrorInfo(UncResultCode.UNC_INTERNAL_SERVER_ERROR
						.getValue());
			}
		} finally {
			if (connection != null && !isCommitRequired) {
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
		LOG.trace("Complete NetworksResource#post()");
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
		VtnBean vtnBean = new VtnBean();
		vtnBean.setVtnName(getTenantId());
		if (new VtnDao().isVtnFound(connection, vtnBean)) {
			VBridgeBean vBridgeBean = new VBridgeBean();
			vBridgeBean.setVtnName(getTenantId());
			vBridgeBean.setVbrName(getNetId());
			if (new VBridgeDao().isVbrFound(connection, vBridgeBean)) {
				resourceFound = true;
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
							VtnServiceOpenStackConsts.TENANT_ID,
							getTenantId()));
		}
		return resourceFound;
	}

	/**
	 * Perform port-map creation related operation at UNC
	 * 
	 * @param requestBody
	 *            - OpenStack request body
	 * @param restResource
	 *            - RestResource instance
	 * @return - erorrCode, 200 for Success
	 */
	private int performPortMapOperations(JsonObject requestBody,
			RestResource restResource) {
		int errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();

		/*
		 * Create request body for vBridge interface creation
		 */
		final JsonObject vbrIfRequestBody = VbrResourcesGenerator
				.getCreateVbrIfRequestBody(requestBody);

		StringBuilder sb = new StringBuilder();
		sb.append(VtnServiceOpenStackConsts.VTN_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getTenantId());
		sb.append(VtnServiceOpenStackConsts.VBRIDGE_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getNetId());
		sb.append(VtnServiceOpenStackConsts.INTERFACE_PATH);

		restResource.setPath(sb.toString());
		restResource.setSessionID(getSessionID());
		restResource.setConfigID(getConfigID());

		errorCode = restResource.post(vbrIfRequestBody);

		if (errorCode == UncResultCode.UNC_SUCCESS.getValue()) {
			final String portName = getPortName(requestBody, restResource);
			if (portName != null) {
				requestBody
						.addProperty(VtnServiceJsonConsts.PORTNAME, portName);
				errorCode = createPortMap(requestBody, restResource);
			} else {
				errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();
				LOG.error("Port Name not found.");
			}
		}

		return errorCode;
	}

	/**
	 * Retrieve port_name from UNC for given port_id
	 * 
	 * @param requestBody
	 *            - OpenStack request body
	 * @param restResource
	 *            - RestResource instance
	 * @return - erorrCode, 200 for Success
	 */
	private String
			getPortName(JsonObject requestBody, RestResource restResource) {
		String portName = null;
		final JsonObject portNameRequestBody = MapResourceGenerator
				.getPortNameRequestBody(requestBody);

		StringBuilder sb = new StringBuilder();
		sb.append(VtnServiceOpenStackConsts.CTRL_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(this.controllerId);
		sb.append(VtnServiceOpenStackConsts.SWITCH_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getSwitchId(requestBody.get(
				VtnServiceOpenStackConsts.DATAPATH_ID).getAsString()));
		sb.append(VtnServiceOpenStackConsts.PHY_PORTS_PATH);

		restResource.setPath(sb.toString());
		restResource.setSessionID(getSessionID());
		restResource.setConfigID(getConfigID());

		if (restResource.get(portNameRequestBody) == UncResultCode.UNC_SUCCESS
				.getValue()) {
			JsonArray ports = restResource.getInfo()
					.get(VtnServiceJsonConsts.PORTS).getAsJsonArray();
			if (ports != null && ports.size() > 0) {
				portName = ports.get(0).getAsJsonObject()
						.get(VtnServiceJsonConsts.PORTNAME).getAsString();
			}
		}

		return portName;
	}

	/**
	 * Retrieve controller_id from database and set to request body
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param requestBody
	 *            - OpenStack JSON request body
	 * @return - true, only if controller_id is set
	 * @throws SQLException
	 */
	private boolean setControllerId(Connection connection,
			JsonObject requestBody) throws SQLException {
		boolean status = true;
		final DestinationControllerDao destControllerDao = 
				new DestinationControllerDao();
		final String controllerId = destControllerDao
				.getDestinationController(connection);

		if (controllerId != null && !controllerId.isEmpty()) {
			LOG.info("Database retrieval is successful for Controller id : "
					+ controllerId);
			requestBody.addProperty(VtnServiceJsonConsts.CONTROLLERID,
					controllerId);
			this.controllerId = controllerId;
		} else {
			LOG.error("Database retrieval is failed for Controller id.");
			createErrorInfo(
					UncResultCode.UNC_CTRL_NOT_FOUND.getValue(),
					getCutomErrorMessage(
							UncResultCode.UNC_CTRL_NOT_FOUND.getMessage(),
							VtnServiceJsonConsts.CONTROLLERID,
							VtnServiceConsts.EMPTY_STRING));
			status = false;
		}
		return status;
	}

	/**
	 * Resolve switch_id from datapath_id
	 * 
	 * @param datapathId
	 *            - OpenStack formatted datapath_id
	 * @return - switch_id
	 */
	private String getSwitchId(String datapathId) {
		datapathId = datapathId.substring(2, datapathId.length());
		final StringBuilder sb = new StringBuilder();
		for (int toPrepend = 16 - datapathId.length(); 
				toPrepend > 0; toPrepend--) {
			sb.append('0');
		}
		datapathId = sb.append(datapathId).toString();
		final String logicalPortIdPartFirst = datapathId.substring(0, 4);
		final String logicalPortIdPartSecond = datapathId.substring(4, 8);
		final String logicalPortIdPartThird = datapathId.substring(8, 12);
		final String logicalPortIdPartFour = datapathId.substring(12, 16);

		return logicalPortIdPartFirst + VtnServiceConsts.HYPHEN
				+ logicalPortIdPartSecond + VtnServiceConsts.HYPHEN
				+ logicalPortIdPartThird + VtnServiceConsts.HYPHEN
				+ logicalPortIdPartFour;
	}

	/**
	 * Perform Vlan-Map related operations at UNC
	 * 
	 * @param requestBody
	 *            - OpenStack request body
	 * @param restResource
	 *            - RestResource instance
	 * @return - erorrCode, 200 for Success
	 */
	private int performVlanMapOperations(JsonObject requestBody,
			RestResource restResource) {
		int errorCode = UncResultCode.UNC_INTERNAL_SERVER_ERROR.getValue();
		int mapModeValue;
		// read property for map-mode
		mapModeValue = Integer.parseInt(VtnServiceInitManager
				.getConfigurationMap().getMapModeValue());

		if (mapModeValue == 0) {
			// if map-mode is specified as 0, then return conflict error if
			// count from UNC is greater than 0. If count is 0 then create
			// vlan-map
			if (getVlanMapCount(restResource) > 0) {
				createErrorInfo(UncResultCode.UNC_CONFLICT_FOUND.getValue());
			} else {
				errorCode = createVlanMap(requestBody, restResource);
			}
		} else if (mapModeValue == 1 || mapModeValue == 2) {
			// if map-mode value is 1 or 2 then create vlan-map without
			// checking count
			errorCode = createVlanMap(requestBody, restResource);
		} else {
			throw new IllegalArgumentException("incorrect map-mode value.");
		}
		return errorCode;
	}

	/**
	 * Retrieve vlan-map count from UNC
	 * 
	 * @param restResource
	 *            - RestResource instance
	 * @return - erorrCode, 200 for Success
	 */
	private int getVlanMapCount(RestResource restResource) {
		int count = -1;
		JsonObject vlanmapRequestBody;
		/*
		 * Create request body for vlan-map count
		 */
		vlanmapRequestBody = MapResourceGenerator.getVLanMapCountRequestBody();

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

		if (restResource.get(vlanmapRequestBody) == UncResultCode.UNC_SUCCESS
				.getValue()) {
			count = restResource.getInfo().get(VtnServiceJsonConsts.VLANMAPS)
					.getAsJsonObject().get(VtnServiceJsonConsts.COUNT)
					.getAsInt();
		}
		return count;
	}

	/**
	 * Create Vlan-Map at UNC
	 * 
	 * @param requestBody
	 *            - OpenStack request body
	 * @param restResource
	 *            - RestResource instance
	 * @return - erorrCode, 200 for Success
	 */
	private int
			createVlanMap(JsonObject requestBody, RestResource restResource) {
		/*
		 * Create request body for vlan-map creation
		 */
		final JsonObject vlanmapRequestBody = MapResourceGenerator
				.getCreateVlanMapRequestBody(requestBody);

		if (vlanmapRequestBody.get(VtnServiceJsonConsts.VLANMAP)
				.getAsJsonObject().has(VtnServiceJsonConsts.LOGICAL_PORT_ID)) {
			logicalPortId = vlanmapRequestBody
					.get(VtnServiceJsonConsts.VLANMAP).getAsJsonObject()
					.get(VtnServiceJsonConsts.LOGICAL_PORT_ID).getAsString();
		} else {
			logicalPortId = VtnServiceJsonConsts.NOLPID;
		}

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

		return restResource.post(vlanmapRequestBody);
	}

	/**
	 * Create Port-Map at UNC
	 * 
	 * @param requestBody
	 *            - OpenStack request body
	 * @param restResource
	 *            - RestResource instance
	 * @return - erorrCode, 200 for Success
	 */
	private int
			createPortMap(JsonObject requestBody, RestResource restResource) {
		/*
		 * Create request body for port-map creation
		 */
		final JsonObject vlanmapRequestBody = MapResourceGenerator
				.getCreatePortMapRequestBody(requestBody);

		StringBuilder sb = new StringBuilder();
		sb.append(VtnServiceOpenStackConsts.VTN_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getTenantId());
		sb.append(VtnServiceOpenStackConsts.VBRIDGE_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getNetId());
		sb.append(VtnServiceOpenStackConsts.INTERFACE_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(requestBody.get(VtnServiceOpenStackConsts.ID).getAsString());
		sb.append(VtnServiceOpenStackConsts.PORTMAP_PATH);

		restResource.setPath(sb.toString());
		restResource.setSessionID(getSessionID());
		restResource.setConfigID(getConfigID());

		return restResource.put(vlanmapRequestBody);
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
	private int performFilterOperations(JsonObject requestBody,
			RestResource restResource,
			Connection connection) throws SQLException {
		int errorCode = UncResultCode.UNC_INTERNAL_SERVER_ERROR.getValue();

		/*
		 * Delete duplicate flow filter.
		 */
		Set<String> filterSet = new HashSet<String>();
		JsonArray filters = requestBody
				.getAsJsonArray(VtnServiceOpenStackConsts.FILTERS);
		Iterator<JsonElement> iterator = filters.iterator();
		while (iterator.hasNext()) {
			JsonElement element = iterator.next();
			filterSet.add(element.getAsString());
			
			// check filter id if exist.
			if (!hasFilterId(connection, element.getAsString())) {
				LOG.error("Resource not found error.");
				return UncResultCode.UNC_NOT_FOUND.getValue();
			}
			
			LOG.debug("filter_id: %s", element.getAsString());
		}

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
		
		if (UncResultCode.UNC_SUCCESS.getValue() == errorCode) {
			LOG.info("filter in Creation at UNC is successful.");
			ArrayList<FlowFilterVbrBean> vbridgeList = 
					new ArrayList<FlowFilterVbrBean>();
			for (String filterID : filterSet) {
				FlowFilterVbrBean vbrBean = new FlowFilterVbrBean();
				vbrBean.setVtnName(getTenantId());
				vbrBean.setVbrName(getNetId());
				vbrBean.setVbrIfName(requestBody
						.get(VtnServiceOpenStackConsts.ID).getAsString());
				vbrBean.setFlName(filterID);
				vbridgeList.add(vbrBean);
			}

			// Insert db for flow filter.
			final FlowFilterDao flowFilterDao = new FlowFilterDao();
			int status = flowFilterDao
					.insertVbridgeFilterInfo(connection, vbridgeList);
			if (1 == status) {
				LOG.info("Resource insertion successful at database " +
						 "operation.");
				for(String filterID : filterSet) {
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
					flowFilterRequestBody.
						add(VtnServiceJsonConsts.FLOWFILTERENTRY,
							new JsonObject());
					JsonObject flowfilter = flowFilterRequestBody
							.getAsJsonObject(
									VtnServiceJsonConsts.FLOWFILTERENTRY);
					flowfilter.addProperty(VtnServiceJsonConsts.SEQNUM,
										   String.valueOf((Integer
												   .parseInt(priority, 16))));
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
						LOG.error("Failed to create flow filter entry(%s) " +
								"at UNC.", filterID);
						break;
					}
				}
			} else {
				LOG.error("Failed to insert Resource(filter for interface " +
						"of vbridge) at database operation.");
				errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();
			}
		} else {
			LOG.error("Failed to create type(in) of flow filter at UNC.");
		}

		return errorCode;
	}
	
	/**
	 * Create type of flow filter at UNC
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
		sb.append(requestBody.get(VtnServiceOpenStackConsts.ID).getAsString());
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
		sb.append(requestBody.get(VtnServiceOpenStackConsts.ID).getAsString());
		sb.append(VtnServiceOpenStackConsts.FLOWFILTER_PATH);
		sb.append(VtnServiceOpenStackConsts.IN_PATH);
		sb.append(VtnServiceOpenStackConsts.FLOWFILTER_ENTRY_PATH);

		restResource.setPath(sb.toString());
		restResource.setSessionID(getSessionID());
		restResource.setConfigID(getConfigID());
		return restResource.post(flowfilterEntryRequest);
	}
	
	/**
	 * Checks that specified instances in URI exists in system or not. If they
	 * are not exists then prepare error JSON for 404 Not Found
	 * 
	 * @param connection
	 *            - Database Connection instance
	 * @param filterId
	 *            - Flow filter id
	 * @return - true, only if all instances exist
	 * @throws SQLException
	 */
	private boolean hasFilterId(Connection connection, String filterId)
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
							VtnServiceOpenStackConsts.FILTER_RES_ID, filterId));
		}
		return resourceFound;
	}
}
