/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.resources.openstack;

import java.sql.Connection;
import java.sql.SQLException;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.RestResource;
import org.opendaylight.vtn.javaapi.annotation.UNCField;
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
import org.opendaylight.vtn.javaapi.init.VtnServiceInitManager;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.openstack.beans.FreeCounterBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VRouterBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VRouterInterfaceBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VtnBean;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;
import org.opendaylight.vtn.javaapi.openstack.convertor.VrtResourcesGenerator;
import org.opendaylight.vtn.javaapi.openstack.dao.VRouterDao;
import org.opendaylight.vtn.javaapi.openstack.dao.VRouterInterfaceDao;
import org.opendaylight.vtn.javaapi.openstack.dao.VtnDao;
import org.opendaylight.vtn.javaapi.openstack.dbmanager.ResourceIdManager;
import org.opendaylight.vtn.javaapi.openstack.validation.RouterInterfaceResourceValidator;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;

/**
 * Resource class for handling PUT and DELETE requests for Router Interface
 */
@UNCVtnService(path = VtnServiceOpenStackConsts.ROUTER_INTERFACE_PATH)
public class RouterInterfaceResource extends AbstractResource {

	/* Logger instance */
	private static final Logger LOG = Logger
			.getLogger(RouterInterfaceResource.class.getName());

	@UNCField(VtnServiceOpenStackConsts.TENANT_ID)
	private String tenantId;

	@UNCField(VtnServiceOpenStackConsts.ROUTER_ID)
	private String routerId;

	@UNCField(VtnServiceOpenStackConsts.IF_ID)
	private String interfaceId;

	/**
	 * Constructor that initialize the validation instance for current resource
	 * instance
	 */
	public RouterInterfaceResource() {
		setValidator(new RouterInterfaceResourceValidator(this));
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
	 * Getter of routerId
	 * 
	 * @return
	 */
	public String getRouterId() {
		return routerId;
	}

	/**
	 * Getter of interfaceId
	 * 
	 * @return
	 */
	public String getInterfaceId() {
		return interfaceId;
	}

	/**
	 * Handler method for PUT operation of Router Interface
	 * 
	 * @see org.opendaylight.vtn.javaapi.resources.AbstractResource#put(com.
	 *      google.gson.JsonObject)
	 */
	@Override
	public int put(JsonObject requestBody) {
		LOG.trace("Start RouterInterfaceResource#put()");

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

				final RestResource restResource = new RestResource();

				errorCode = updateVRouterInterface(requestBody, restResource);

				if (errorCode == UncCommonEnum.UncResultCode.UNC_SUCCESS
						.getValue()) {
					LOG.error("vBridge Update at UNC is successful.");
				} else {
					LOG.info("vBridge Update at UNC is failed.");
				}
				checkForSpecificErrors(restResource.getInfo());
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

		LOG.trace("Complete RouterInterfaceResource#put()");
		return errorCode;
	}

	/**
	 * Handler method for DELETE operation of Router Interface
	 * 
	 * @see org.opendaylight.vtn.javaapi.resources.AbstractResource#delete()
	 */
	@Override
	public int delete() {
		LOG.trace("Start RouterInterfaceResource#delete()");

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

				final VRouterInterfaceBean vInterfaceBean = new VRouterInterfaceBean();
				vInterfaceBean.setVrtIfId(Integer.parseInt(getInterfaceId()));
				vInterfaceBean.setVtnName(getTenantId());
				vInterfaceBean.setVrtName(getRouterId());
				vInterfaceBean.setVrtIfName(VtnServiceOpenStackConsts.IF_PREFIX
						+ getInterfaceId());

				final FreeCounterBean freeCounterBean = new FreeCounterBean();
				freeCounterBean
						.setResourceId(VtnServiceOpenStackConsts.PORT_RES_ID);
				freeCounterBean.setVtnName(getTenantId());
				freeCounterBean.setResourceCounter(vInterfaceBean.getVrtIfId());

				final VRouterInterfaceDao vInterfaceDao = new VRouterInterfaceDao();
				final String vbrName = vInterfaceDao.getVbridgeName(connection,
						vInterfaceBean);

				final ResourceIdManager resourceIdManager = new ResourceIdManager();

				if (resourceIdManager.deleteResourceId(connection,
						freeCounterBean, vInterfaceBean)) {
					LOG.info("Deletion operation from database is successfull.");

					final RestResource restResource = new RestResource();

					errorCode = deleteVLink(restResource);

					if (errorCode == UncCommonEnum.UncResultCode.UNC_SUCCESS
							.getValue()) {
						errorCode = deleteVRouterInterface(restResource);

						if (errorCode == UncCommonEnum.UncResultCode.UNC_SUCCESS
								.getValue()) {

							if (vbrName != null) {
								errorCode = deleteVBridgeInterface(
										restResource, vbrName);

								if (errorCode == UncCommonEnum.UncResultCode.UNC_SUCCESS
										.getValue()) {
									LOG.info("vBridge/vRouter interface and vLink Deletion successful at UNC.");
									isCommitRequired = true;
								} else {
									errorCode = UncResultCode.UNC_SERVER_ERROR
											.getValue();
									LOG.error("vBridge interface Deletion failed at UNC.");
								}
							} else {
								errorCode = UncResultCode.UNC_SERVER_ERROR
										.getValue();
								LOG.error("vbr_name not found.");
							}
						} else {
							errorCode = UncResultCode.UNC_SERVER_ERROR
									.getValue();
							LOG.error("vRouter interface Deletion failed at UNC.");
						}
					} else {
						errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();
						LOG.error("vlink Deletion failed at UNC.");
					}
					checkForSpecificErrors(restResource.getInfo());
				} else {
					LOG.info("Deletion operation from database is falied.");
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
		LOG.trace("Complete RouterInterfaceResource#delete()");
		return errorCode;
	}

	/**
	 * Update vRouter interface at UNC
	 * 
	 * @param requestBody
	 *            - OpenStack request body
	 * @param restResource
	 *            - RestResource instance
	 * @return - erorrCode, 200 for Success
	 */
	private int updateVRouterInterface(JsonObject requestBody,
			final RestResource restResource) {
		/*
		 * Create request body for vRouter interface update
		 */
		final JsonObject vrtIfRequestBody = VrtResourcesGenerator
				.getUpdateVrtIfRequestBody(requestBody);

		/*
		 * execute update vRouter interface request
		 */
		StringBuilder sb = new StringBuilder();
		sb.append(VtnServiceOpenStackConsts.VTN_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getTenantId());
		sb.append(VtnServiceOpenStackConsts.VROUTER_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getRouterId());
		sb.append(VtnServiceOpenStackConsts.INTERFACE_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(VtnServiceOpenStackConsts.IF_PREFIX);
		sb.append(getInterfaceId());

		restResource.setPath(sb.toString());
		restResource.setSessionID(getSessionID());
		restResource.setConfigID(getConfigID());

		return restResource.put(vrtIfRequestBody);
	}

	/**
	 * Delete vBridge interface at UNC
	 * 
	 * @param restResource
	 *            - RestResource instance
	 * @return - erorrCode, 200 for Success
	 */
	private int
			deleteVBridgeInterface(RestResource restResource, String vbrName) {
		int errorCode;
		/*
		 * execute delete vBridge interface request
		 */
		StringBuilder sb = new StringBuilder();
		sb.append(VtnServiceOpenStackConsts.VTN_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getTenantId());
		sb.append(VtnServiceOpenStackConsts.VBRIDGE_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(vbrName);
		sb.append(VtnServiceOpenStackConsts.INTERFACE_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(VtnServiceOpenStackConsts.IF_PREFIX);
		sb.append(getInterfaceId());

		restResource.setPath(sb.toString());
		restResource.setSessionID(getSessionID());
		restResource.setConfigID(getConfigID());

		errorCode = restResource.delete();
		return errorCode;
	}

	/**
	 * Delete vRouter interface at UNC
	 * 
	 * @param restResource
	 *            - RestResource instance
	 * @return - erorrCode, 200 for Success
	 */
	private int deleteVRouterInterface(RestResource restResource) {
		/*
		 * execute delete vRouter interface request
		 */
		StringBuilder sb = new StringBuilder();
		sb.append(VtnServiceOpenStackConsts.VTN_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getTenantId());
		sb.append(VtnServiceOpenStackConsts.VROUTER_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getRouterId());
		sb.append(VtnServiceOpenStackConsts.INTERFACE_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(VtnServiceOpenStackConsts.IF_PREFIX);
		sb.append(getInterfaceId());

		restResource.setPath(sb.toString());
		restResource.setSessionID(getSessionID());
		restResource.setConfigID(getConfigID());

		return restResource.delete();
	}

	/**
	 * Delete vLink at UNC
	 * 
	 * @param restResource
	 *            - RestResource instance
	 * @return - erorrCode, 200 for Success
	 */
	private int deleteVLink(RestResource restResource) {
		/*
		 * execute delete vLink request
		 */
		StringBuilder sb = new StringBuilder();
		sb.append(VtnServiceOpenStackConsts.VTN_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getTenantId());
		sb.append(VtnServiceOpenStackConsts.VLINK_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(VtnServiceOpenStackConsts.VLK_PREFIX);
		sb.append(getInterfaceId());

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
			VRouterBean vRouterBean = new VRouterBean();
			vRouterBean.setVtnName(getTenantId());
			vRouterBean.setVrtName(getRouterId());
			if (new VRouterDao().isVrtFound(connection, vRouterBean)) {
				VRouterInterfaceBean vInterfaceBean = new VRouterInterfaceBean();
				vInterfaceBean.setVtnName(getTenantId());
				vInterfaceBean.setVrtName(getRouterId());
				vInterfaceBean.setVrtIfName(VtnServiceOpenStackConsts.IF_PREFIX
						+ getInterfaceId());
				if (new VRouterInterfaceDao().isVrtIfFound(connection,
						vInterfaceBean)) {
					resourceFound = true;
				} else {
					createErrorInfo(
							UncResultCode.UNC_NOT_FOUND.getValue(),
							getCutomErrorMessage(
									UncResultCode.UNC_NOT_FOUND.getMessage(),
									VtnServiceOpenStackConsts.IF_ID,
									getInterfaceId()));
				}
			} else {
				createErrorInfo(
						UncResultCode.UNC_NOT_FOUND.getValue(),
						getCutomErrorMessage(
								UncResultCode.UNC_NOT_FOUND.getMessage(),
								VtnServiceOpenStackConsts.ROUTER_ID,
								getRouterId()));
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
}
