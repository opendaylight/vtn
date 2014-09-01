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

import org.apache.commons.net.util.SubnetUtils;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.RestResource;
import org.opendaylight.vtn.javaapi.annotation.UNCField;
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.init.VtnServiceInitManager;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.openstack.beans.StaticRouteBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VRouterBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VtnBean;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;
import org.opendaylight.vtn.javaapi.openstack.convertor.StaticRouteResourceGenerator;
import org.opendaylight.vtn.javaapi.openstack.dao.StaticRouteDao;
import org.opendaylight.vtn.javaapi.openstack.dao.VRouterDao;
import org.opendaylight.vtn.javaapi.openstack.dao.VtnDao;
import org.opendaylight.vtn.javaapi.openstack.validation.RouteResourceValidator;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;

/**
 * Resource class for handling POST and GET requests for Static Routes
 */
@UNCVtnService(path = VtnServiceOpenStackConsts.ROUTES_PATH)
public class RoutesResource extends AbstractResource {

	/* Logger instance */
	private static final Logger LOG = Logger.getLogger(RoutesResource.class
			.getName());

	@UNCField(VtnServiceOpenStackConsts.TENANT_ID)
	private String tenantId;

	@UNCField(VtnServiceOpenStackConsts.ROUTER_ID)
	private String routerId;

	/**
	 * Constructor that initialize the validation instance for current resource
	 * instance
	 */
	public RoutesResource() {
		setValidator(new RouteResourceValidator(this));
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

	/**
	 * Handler method for POST operation of Route
	 * 
	 * @see org.opendaylight.vtn.javaapi.resources.AbstractResource#post(com
	 *      .google.gson.JsonObject)
	 */
	@Override
	public int post(JsonObject requestBody) {
		LOG.trace("Start RoutesResource#post()");

		int errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();

		boolean isCommitRequired = false;
		String generatedRouteId = null;
		Connection connection = null;

		try {
			connection = VtnServiceInitManager.getDbConnectionPoolMap()
					.getConnection();

			/*
			 * Check for instances that they exists or not, if not then return
			 * 404 error
			 */
			if (checkForNotFoundResources(connection)) {

				generatedRouteId = convertRouteId(requestBody);

				StaticRouteBean staticRouteBean = new StaticRouteBean();
				staticRouteBean.setVtnName(getTenantId());
				staticRouteBean.setVrtName(getRouterId());
				staticRouteBean.setRouteName(generatedRouteId);

				StaticRouteDao stRouteDao = new StaticRouteDao();
				final int status = stRouteDao.insert(connection,
						staticRouteBean);
				if (status == 1) {
					LOG.info("Resource insertion successful at database operation.");

					final RestResource restResource = new RestResource();

					errorCode = createStaticRoute(requestBody, restResource);

					if (errorCode == UncCommonEnum.UncResultCode.UNC_SUCCESS
							.getValue()) {
						LOG.info("static-route is successful at UNC.");
						isCommitRequired = true;
						final JsonObject response = new JsonObject();
						response.addProperty(VtnServiceOpenStackConsts.ID,
								generatedRouteId);
						setInfo(response);
					} else {
						LOG.error("static-route creation is failed at UNC.");
					}
					checkForSpecificErrors(restResource.getInfo());
				} else {
					LOG.error("Resource insertion failed at database operation.");
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
								VtnServiceOpenStackConsts.ROUTE_ID,
								generatedRouteId));
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
		LOG.trace("Complete RoutesResource#post()");
		return errorCode;
	}

	/**
	 * Handler method for GET operation of Route
	 * 
	 * @see org.opendaylight.vtn.javaapi.resources.AbstractResource#get(com.
	 *      google.gson.JsonObject)
	 */
	@Override
	public int get(JsonObject queryString) {
		LOG.trace("Start RoutesResource#get()");
		int errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();

		String generatedRouteId = null;
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

				errorCode = getStaticRoutes(restResource);

				if (errorCode == UncCommonEnum.UncResultCode.UNC_SUCCESS
						.getValue()) {
					LOG.info("static-route retrieval is successful at UNC.");
					final JsonObject response = StaticRouteResourceGenerator
							.convertListResponseBody(restResource.getInfo());
					setInfo(response);
				} else {
					LOG.error("static-route retrieval is failed at UNC.");
				}
				checkForSpecificErrors(restResource.getInfo());
			} else {
				LOG.error("Resource not found error.");
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
								VtnServiceOpenStackConsts.ROUTE_ID,
								generatedRouteId));
			} else {
				createErrorInfo(UncResultCode.UNC_INTERNAL_SERVER_ERROR
						.getValue());
			}
		} finally {
			if (connection != null) {
				LOG.info("Free connection...");
				VtnServiceInitManager.getDbConnectionPoolMap().freeConnection(
						connection);
			}
		}
		LOG.trace("Complete RoutesResource#get()");
		return errorCode;
	}

	/**
	 * Get Static-Route from UNC
	 * 
	 * @param restResource
	 *            - RestResource instance
	 * @return - erorrCode, 200 for Success
	 */
	private int getStaticRoutes(final RestResource restResource) {
		/*
		 * Create request body for static-route list
		 */
		final JsonObject staticRouteRequestBody = StaticRouteResourceGenerator
				.getListRequestBody();

		/*
		 * execute list static-route request
		 */
		StringBuilder sb = new StringBuilder();
		sb.append(VtnServiceOpenStackConsts.VTN_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getTenantId());
		sb.append(VtnServiceOpenStackConsts.VROUTER_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getRouterId());
		sb.append(VtnServiceOpenStackConsts.STATIC_ROUTE_PATH);

		restResource.setPath(sb.toString());
		restResource.setSessionID(getSessionID());
		restResource.setConfigID(getConfigID());

		return restResource.get(staticRouteRequestBody);
	}

	/**
	 * Create Static-Route at UNC
	 * 
	 * @param requestBody
	 *            - OpenStack request body
	 * @param restResource
	 *            - RestResource instance
	 * @return - erorrCode, 200 for Success
	 */
	private int createStaticRoute(JsonObject requestBody,
			final RestResource restResource) {
		/*
		 * Create request body for static-route creation
		 */
		final JsonObject staticRouteRequestBody = StaticRouteResourceGenerator
				.getCreateStaticRouteRequestBody(requestBody);

		/*
		 * execute create static-route request
		 */
		StringBuilder sb = new StringBuilder();
		sb.append(VtnServiceOpenStackConsts.VTN_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getTenantId());
		sb.append(VtnServiceOpenStackConsts.VROUTER_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getRouterId());
		sb.append(VtnServiceOpenStackConsts.STATIC_ROUTE_PATH);

		restResource.setPath(sb.toString());
		restResource.setSessionID(getSessionID());
		restResource.setConfigID(getConfigID());

		return restResource.post(staticRouteRequestBody);
	}

	/**
	 * Generate UNC formatted route_id
	 * 
	 * @param requestBody
	 *            - OpenStack request body
	 * @return - generated route_id
	 */
	private String convertRouteId(JsonObject requestBody) {
		String destination = requestBody.get(
				VtnServiceOpenStackConsts.DESTNATION).getAsString();
		String ipAndPrefix[] = destination.split(VtnServiceConsts.SLASH);

		int prefix = Integer.parseInt(ipAndPrefix[1]);
		String routeId = null;
		if (prefix == 0) {
			routeId = ipAndPrefix[0]
					+ VtnServiceConsts.HYPHEN
					+ requestBody.get(VtnServiceOpenStackConsts.NEXTHOP)
							.getAsString() + VtnServiceConsts.HYPHEN
					+ VtnServiceConsts.DEFAULT_IP;
		} else {
			final SubnetUtils subnetUtils = new SubnetUtils(destination);
			routeId = ipAndPrefix[0]
					+ VtnServiceConsts.HYPHEN
					+ requestBody.get(VtnServiceOpenStackConsts.NEXTHOP)
							.getAsString() + VtnServiceConsts.HYPHEN
					+ subnetUtils.getInfo().getNetmask();
		}
		return routeId;
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
