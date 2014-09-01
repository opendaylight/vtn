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

import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.RestResource;
import org.opendaylight.vtn.javaapi.annotation.UNCField;
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
import org.opendaylight.vtn.javaapi.init.VtnServiceInitManager;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.openstack.beans.FreeCounterBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VRouterBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VtnBean;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;
import org.opendaylight.vtn.javaapi.openstack.dao.VRouterDao;
import org.opendaylight.vtn.javaapi.openstack.dao.VtnDao;
import org.opendaylight.vtn.javaapi.openstack.dbmanager.ResourceIdManager;
import org.opendaylight.vtn.javaapi.openstack.validation.RouterResourceValidator;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;

/**
 * Resource class for handling PUT and DELETE requests for Router
 */
@UNCVtnService(path = VtnServiceOpenStackConsts.ROUTER_PATH)
public class RouterResource extends AbstractResource {

	/* Logger instance */
	private static final Logger LOG = Logger.getLogger(RouterResource.class
			.getName());

	@UNCField(VtnServiceOpenStackConsts.TENANT_ID)
	private String tenantId;

	@UNCField(VtnServiceOpenStackConsts.ROUTER_ID)
	private String routerId;

	/**
	 * Constructor that initialize the validation instance for current resource
	 * instance
	 */
	public RouterResource() {
		setValidator(new RouterResourceValidator(this));
	}

	/**
	 * Getter for tenantId
	 * 
	 * @return
	 */
	public String getTenantId() {
		return tenantId;
	}

	/**
	 * Getter for routerId
	 * 
	 * @return
	 */
	public String getRouterId() {
		return routerId;
	}

	// PUT method's is retained, as it might be required in future
	// /**
	// * Handler method for PUT operation of Router
	// *
	// * @see
	// org.opendaylight.vtn.javaapi.resources.AbstractResource#put(com.
	// * google.gson.JsonObject)
	// */
	// @Override
	// public int put(JsonObject requestBody){
	// LOG.trace("Start RouterResource#put()");
	//
	// int errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();
	//
	// Connection connection = null;
	//
	// try {
	// connection = VtnServiceInitManager.getDbConnectionPoolMap()
	// .getConnection();
	//
	// /*
	// * Check for instances that they exists or not, if not then return
	// * 404 error
	// */
	// if (checkForNotFoundResources(connection)) {
	//
	// final RestResource restResource = new RestResource();
	//
	// errorCode = updateVRouter(requestBody, restResource);
	//
	// if (errorCode == UncCommonEnum.UncResultCode.UNC_SUCCESS
	// .getValue()) {
	// LOG.error("vRouter Update at UNC is successful.");
	// } else {
	// LOG.info("vRouter Update at UNC is failed.");
	// }
	// checkForSpecificErrors(restResource.getInfo());
	// } else {
	// LOG.error("Resource not found error.");
	// }
	// /*
	// * set response, if it is not set during above processing
	// */
	// if (errorCode != UncResultCode.UNC_SUCCESS.getValue()) {
	// if (getInfo() == null) {
	// createErrorInfo(UncResultCode.UNC_INTERNAL_SERVER_ERROR
	// .getValue());
	// }
	// }
	// } catch (final SQLException exception) {
	// LOG.error("Internal server error : " + exception);
	// errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();
	// createErrorInfo(UncResultCode.UNC_INTERNAL_SERVER_ERROR.getValue());
	// } finally {
	// if (connection != null) {
	// LOG.info("Free connection...");
	// VtnServiceInitManager.getDbConnectionPoolMap().freeConnection(
	// connection);
	// }
	// }
	// LOG.trace("Complete RouterResource#put()");
	// return errorCode;
	// }

	/**
	 * Handler method for DELETE operation of Router
	 * 
	 * @see org.opendaylight.vtn.javaapi.resources.AbstractResource#delete(com
	 *      .google.gson.JsonObject)
	 */
	@Override
	public int delete() {
		LOG.trace("Start RouterResource#delete()");

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
				final VRouterBean vRouterBean = new VRouterBean();
				vRouterBean.setVtnName(getTenantId());
				vRouterBean.setVrtName(getRouterId());

				final FreeCounterBean freeCounterBean = new FreeCounterBean();
				freeCounterBean
						.setResourceId(VtnServiceOpenStackConsts.ROUTER_RES_ID);
				freeCounterBean.setVtnName(getTenantId());
				freeCounterBean.setResourceCounter(0);

				final ResourceIdManager resourceIdManager = new ResourceIdManager();

				if (resourceIdManager.deleteResourceId(connection,
						freeCounterBean, vRouterBean)) {
					LOG.info("Deletion operation from database is successfull.");

					final RestResource restResource = new RestResource();

					errorCode = deleteVRouter(restResource);

					if (errorCode == UncCommonEnum.UncResultCode.UNC_SUCCESS
							.getValue()) {
						LOG.error("vRouter Deletion successful at UNC.");
						isCommitRequired = true;
					} else {
						LOG.error("vRouter Deletion failed at UNC.");
					}
					checkForSpecificErrors(restResource.getInfo());
				} else {
					LOG.info("Deletion operation from database is falied.");
					connection.rollback();
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
				LOG.info("Resource insertion successful in database.");
			} else {
				connection.rollback();
				LOG.info("Resource insertion is roll-backed.");
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
		LOG.trace("Complete RouterResource#delete()");
		return errorCode;
	}

	// /**
	// * Update vRouter at UNC
	// *
	// * @param requestBody
	// * - OpenStack request body
	// * @param restResource
	// * - RestResource instance
	// * @return - erorrCode, 200 for Success
	// */
	// private int updateVRouter(JsonObject requestBody,
	// final RestResource restResource) {
	// /*
	// * Create request body for vRouter update
	// */
	// final JsonObject vrtRequestBody = VrtResourcesGenerator
	// .getUpdateVrtRequestBody(requestBody);
	//
	// /*
	// * execute update vBridge request
	// */
	// StringBuilder sb = new StringBuilder();
	// sb.append(VtnServiceOpenStackConsts.VTN_PATH);
	// sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
	// sb.append(getTenantId());
	// sb.append(VtnServiceOpenStackConsts.VROUTER_PATH);
	// sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
	// sb.append(getRouterId());
	//
	// restResource.setPath(sb.toString());
	// restResource.setSessionID(getSessionID());
	// restResource.setConfigID(getConfigID());
	//
	// return restResource.put(vrtRequestBody);
	// }

	/**
	 * Delete vRouter at UNC
	 * 
	 * @param restResource
	 *            - RestResource instance
	 * @return - erorrCode, 200 for Success
	 */
	private int deleteVRouter(final RestResource restResource) {
		/*
		 * execute delete vRouter request
		 */
		StringBuilder sb = new StringBuilder();
		sb.append(VtnServiceOpenStackConsts.VTN_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getTenantId());
		sb.append(VtnServiceOpenStackConsts.VROUTER_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getRouterId());

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
				resourceFound = true;
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
