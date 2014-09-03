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
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.init.VtnServiceInitManager;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.openstack.beans.VRouterBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VtnBean;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;
import org.opendaylight.vtn.javaapi.openstack.convertor.VrtResourcesGenerator;
import org.opendaylight.vtn.javaapi.openstack.dao.DestinationControllerDao;
import org.opendaylight.vtn.javaapi.openstack.dao.VRouterDao;
import org.opendaylight.vtn.javaapi.openstack.dao.VtnDao;
import org.opendaylight.vtn.javaapi.openstack.validation.RouterResourceValidator;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;

/**
 * Resource class for handling PUT and DELETE requests for Static Routes
 */
@UNCVtnService(path = VtnServiceOpenStackConsts.ROUTERS_PATH)
public class RoutersResource extends AbstractResource {

	/* Logger instance */
	private static final Logger LOG = Logger.getLogger(RoutersResource.class
			.getName());

	@UNCField(VtnServiceOpenStackConsts.TENANT_ID)
	private String tenantId;

	/**
	 * Constructor that initialize the validation instance for current resource
	 * instance
	 */
	public RoutersResource() {
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
	 * Handler method for POST operation of Router
	 * 
	 * @see org.opendaylight.vtn.javaapi.resources.AbstractResource#post(com
	 *      .google.gson.JsonObject)
	 */
	@Override
	public int post(JsonObject requestBody) {

		LOG.trace("Start RoutersResource#post()");

		int errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();

		boolean isCommitRequired = false;
		String generatedVrtName = null;
		Connection connection = null;

		try {
			connection = VtnServiceInitManager.getDbConnectionPoolMap()
					.getConnection();

			/*
			 * Check for instances that they exists or not, if not then return
			 * 404 error
			 */
			if (checkForNotFoundResources(connection)) {

				/*
				 * generate "id" is it is not present in request body
				 */
				if (!requestBody.has(VtnServiceOpenStackConsts.ID)) {
					LOG.info("Resource id auto-generation is required.");
					generatedVrtName = VtnServiceOpenStackConsts.VRT_PREFIX;
					requestBody.addProperty(VtnServiceOpenStackConsts.ID,
							generatedVrtName);
				} else {
					LOG.info("Resource id auto-generation is not required.");
					generatedVrtName = requestBody.get(
							VtnServiceOpenStackConsts.ID).getAsString();
				}

				LOG.debug("vrt_name : " + generatedVrtName);

				if (setControllerId(connection, requestBody)) {
					/*
					 * resource insertion in database, if is is successful then
					 * continue to execute operations at UNC. Otherwise return
					 * HTTP 409
					 */
					final VRouterBean vRouterBean = new VRouterBean();
					vRouterBean.setVrtName(generatedVrtName);
					vRouterBean.setVtnName(getTenantId());

					final VRouterDao vRouterDao = new VRouterDao();
					final int status = vRouterDao.insert(connection,
							vRouterBean);

					if (status == 1) {
						LOG.info("Resource insertion successful at database operation.");

						final RestResource restResource = new RestResource();

						errorCode = createVRouter(requestBody, restResource);

						if (errorCode == UncCommonEnum.UncResultCode.UNC_SUCCESS
								.getValue()) {
							LOG.info("vRouter creation is successful at UNC.");
							isCommitRequired = true;
							if (generatedVrtName
									.equals(VtnServiceOpenStackConsts.VRT_PREFIX)) {
								final JsonObject response = new JsonObject();
								response.addProperty(
										VtnServiceOpenStackConsts.ID,
										generatedVrtName);
								setInfo(response);
							}
						} else {
							LOG.error("vRouter creation is failed at UNC.");
						}
						checkForSpecificErrors(restResource.getInfo());
					} else {
						LOG.error("Resource insertion failed at database operation.");
					}
				} else {
					LOG.error("Errot occurred while setting controller_id");
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
				if (exception.getMessage().contains("pk_os_vrt")) {
					createErrorInfo(
							UncResultCode.UNC_CONFLICT_FOUND.getValue(),
							getCutomErrorMessage(
									UncResultCode.UNC_CONFLICT_FOUND
											.getMessage(),
									VtnServiceOpenStackConsts.ROUTER_ID,
									generatedVrtName));
				} else {
					createErrorInfo(UncResultCode.UNC_CONFLICT_FOUND.getValue());
				}

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
		LOG.trace("Complete RoutersResource#post()");
		return errorCode;
	}

	/**
	 * Create vRouter at UNC
	 * 
	 * @param restResource
	 *            - RestResource instance
	 * @return - erorrCode, 200 for Success
	 */
	private int createVRouter(JsonObject requestBody,
			final RestResource restResource) {
		/*
		 * Create request body for vRouter creation
		 */
		final JsonObject vrtRequestBody = VrtResourcesGenerator
				.getCreateVrtRequestBody(requestBody);

		/*
		 * execute create vRouter request
		 */
		StringBuilder sb = new StringBuilder();
		sb.append(VtnServiceOpenStackConsts.VTN_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getTenantId());
		sb.append(VtnServiceOpenStackConsts.VROUTER_PATH);

		restResource.setPath(sb.toString());
		restResource.setSessionID(getSessionID());
		restResource.setConfigID(getConfigID());

		return restResource.post(vrtRequestBody);
	}

	/**
	 * Retrieve controller_id from database and add the parameter in request
	 * body
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param requestBody
	 *            - JSON request body
	 * @return - true, if controller_id is set
	 * @throws SQLException
	 */
	private boolean setControllerId(Connection connection,
			JsonObject requestBody) throws SQLException {
		boolean controllerAvailable = true;
		final DestinationControllerDao destControllerDao = new DestinationControllerDao();
		final String controllerId = destControllerDao
				.getDestinationController(connection);

		if (controllerId != null && !controllerId.isEmpty()) {
			LOG.info("Database retrieval is successful for Controller id : "
					+ controllerId);
			requestBody.addProperty(VtnServiceJsonConsts.CONTROLLERID,
					controllerId);
		} else {
			LOG.error("Database retrieval is failed for Controller id.");
			controllerAvailable = false;
			createErrorInfo(
					UncResultCode.UNC_CTRL_NOT_FOUND.getValue(),
					getCutomErrorMessage(
							UncResultCode.UNC_CTRL_NOT_FOUND.getMessage(),
							VtnServiceJsonConsts.CONTROLLERID,
							VtnServiceConsts.EMPTY_STRING));
		}
		return controllerAvailable;
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
			resourceFound = true;
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
