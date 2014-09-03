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

import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.RestResource;
import org.opendaylight.vtn.javaapi.annotation.UNCField;
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.init.VtnServiceInitManager;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.openstack.beans.FreeCounterBean;
import org.opendaylight.vtn.javaapi.openstack.beans.StaticRouteBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VRouterBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VtnBean;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;
import org.opendaylight.vtn.javaapi.openstack.dao.StaticRouteDao;
import org.opendaylight.vtn.javaapi.openstack.dao.VRouterDao;
import org.opendaylight.vtn.javaapi.openstack.dao.VtnDao;
import org.opendaylight.vtn.javaapi.openstack.dbmanager.ResourceIdManager;
import org.opendaylight.vtn.javaapi.openstack.validation.RouteResourceValidator;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;

/**
 * Resource class for handling DELETE requests for Static Routes
 */
@UNCVtnService(path = VtnServiceOpenStackConsts.ROUTE_PATH)
public class RouteResource extends AbstractResource {

	/* Logger instance */
	private static final Logger LOG = Logger.getLogger(RouteResource.class
			.getName());

	@UNCField(VtnServiceOpenStackConsts.TENANT_ID)
	private String tenantId;

	@UNCField(VtnServiceOpenStackConsts.ROUTER_ID)
	private String routerId;

	@UNCField(VtnServiceOpenStackConsts.ROUTE_ID)
	private String routeId;

	/**
	 * Constructor that initialize the validation instance for current resource
	 * instance
	 */
	public RouteResource() {
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
	 * Getter for routeId
	 * 
	 * @return
	 */
	public String getRouteId() {
		return routeId;
	}

	/**
	 * Handler method for DELETE operation of Route
	 * 
	 * @see org.opendaylight.vtn.javaapi.resources.AbstractResource#delete()
	 */
	@Override
	public int delete() {
		LOG.trace("Start RoutesResource#delete()");

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

				final StaticRouteBean staticRouteBean = new StaticRouteBean();
				staticRouteBean.setVtnName(getTenantId());
				staticRouteBean.setVrtName(getRouterId());
				staticRouteBean.setRouteName(getRouteId());

				final FreeCounterBean freeCounterBean = new FreeCounterBean();
				freeCounterBean
						.setResourceId(VtnServiceOpenStackConsts.DEFAULT_ROUTE);
				freeCounterBean.setVtnName(getTenantId());
				freeCounterBean.setResourceCounter(0);

				final ResourceIdManager resourceIdManager = new ResourceIdManager();

				if (resourceIdManager.deleteResourceId(connection,
						freeCounterBean, staticRouteBean)) {
					LOG.info("Deletion operation from database is successfull.");

					final RestResource restResource = new RestResource();

					errorCode = deleteStaticRoute(restResource);

					if (errorCode == UncCommonEnum.UncResultCode.UNC_SUCCESS
							.getValue()) {
						LOG.error("static-route Deletion successful at UNC.");
						isCommitRequired = true;
					} else {
						LOG.error("static-route Deletion failed at UNC.");
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
			createErrorInfo(UncResultCode.UNC_INTERNAL_SERVER_ERROR.getValue());
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
		LOG.trace("Complete RoutesResource#delete()");
		return errorCode;
	}

	/**
	 * Delete Static-Route at UNC
	 * 
	 * @param restResource
	 *            - RestResource instance
	 * @return - erorrCode, 200 for Success
	 */
	private int deleteStaticRoute(final RestResource restResource) {
		/*
		 * execute delete static-route request
		 */
		StringBuilder sb = new StringBuilder();
		sb.append(VtnServiceOpenStackConsts.VTN_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getTenantId());
		sb.append(VtnServiceOpenStackConsts.VROUTER_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getRouterId());
		sb.append(VtnServiceOpenStackConsts.STATIC_ROUTE_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(convertRouteId(getRouteId()));

		restResource.setPath(sb.toString());
		restResource.setSessionID(getSessionID());
		restResource.setConfigID(getConfigID());

		return restResource.delete();
	}

	/**
	 * Convert UNC formatted route_id from OpenStack formatted route_id
	 * 
	 * @param requestBody
	 *            - OpenStack request body
	 * @return - generated route_id
	 */
	private String convertRouteId(String osRouteId) {
		final String[] routeId = osRouteId.split(VtnServiceConsts.HYPHEN);
		String staticIpRouteId = null;
		if (routeId[2].equals(VtnServiceConsts.DEFAULT_IP)) {
			staticIpRouteId = routeId[0] + VtnServiceConsts.HYPHEN + routeId[1]
					+ VtnServiceConsts.HYPHEN + VtnServiceConsts.ZERO;
		} else {
			final SubnetUtils subnetUtils = new SubnetUtils(routeId[0],
					routeId[2]);
			staticIpRouteId = routeId[0]
					+ VtnServiceConsts.HYPHEN
					+ routeId[1]
					+ VtnServiceConsts.HYPHEN
					+ subnetUtils.getInfo().getCidrSignature()
							.split(VtnServiceConsts.SLASH)[1];
		}
		return staticIpRouteId;
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
				StaticRouteBean staticRouteBean = new StaticRouteBean();
				staticRouteBean.setVtnName(getTenantId());
				staticRouteBean.setVrtName(getRouterId());
				staticRouteBean.setRouteName(getRouteId());
				if (new StaticRouteDao().isStaticRouteFound(connection,
						staticRouteBean)) {
					resourceFound = true;
				} else {
					createErrorInfo(
							UncResultCode.UNC_NOT_FOUND.getValue(),
							getCutomErrorMessage(
									UncResultCode.UNC_NOT_FOUND.getMessage(),
									VtnServiceOpenStackConsts.ROUTE_ID,
									getRouteId()));
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
