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

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.RestResource;
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.init.VtnServiceInitManager;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;
import org.opendaylight.vtn.javaapi.openstack.dao.DestinationControllerDao;
import org.opendaylight.vtn.javaapi.openstack.dao.VtnDao;
import org.opendaylight.vtn.javaapi.openstack.validation.DestinationControllerResourceValidator;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;

/**
 * Resource class for handling PUT, DELETE and GET requests for Destination
 * Controller
 */
@UNCVtnService(path = VtnServiceOpenStackConsts.DEST_CTRL_PATH)
public class DestinationControllerResource extends AbstractResource {

	/* Logger instance */
	private static final Logger LOG = Logger
			.getLogger(DestinationControllerResource.class.getName());

	/**
	 * Constructor that initialize the validation instance for current resource
	 * instance
	 */
	public DestinationControllerResource() {
		setValidator(new DestinationControllerResourceValidator(this));
	}

	/**
	 * Handler method for PUT operation of DestinationController
	 * 
	 * @see org.opendaylight.vtn.javaapi.resources.AbstractResource#put(com.
	 *      google.gson.JsonObject)
	 */
	@Override
	public int put(JsonObject requestBody) {
		LOG.trace("Start DestinationControllerResource#put()");

		int errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();

		boolean isCommitRequired = false;
		Connection connection = null;

		final String controllerId = requestBody.get(
				VtnServiceOpenStackConsts.ID).getAsString();

		try {
			LOG.debug("Set operation for controller_id : " + controllerId);

			connection = VtnServiceInitManager.getDbConnectionPoolMap()
					.getConnection();

			final RestResource restResource = new RestResource();

			/*
			 * execute Get Controller request
			 */
			StringBuilder sb = new StringBuilder();
			sb.append(VtnServiceOpenStackConsts.CTRL_PATH);
			sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
			sb.append(controllerId);

			restResource.setPath(sb.toString());
			restResource.setSessionID(getSessionID());
			restResource.setConfigID(getConfigID());
			errorCode = restResource.get(new JsonObject());

			if (errorCode == UncResultCode.UNC_SUCCESS.getValue()) {
				if (restResource.getInfo().get(VtnServiceJsonConsts.CONTROLLER)
						.isJsonNull()
						|| restResource.getInfo().getAsJsonObject()
								.get(VtnServiceJsonConsts.CONTROLLER)
								.getAsJsonObject()
								.get(VtnServiceJsonConsts.CONTROLLERID)
								.getAsString().isEmpty()) {

					LOG.error("Controller does not exist at UNC.");

					createErrorInfo(
							UncResultCode.UNC_CTRL_NOT_FOUND.getValue(),
							getCutomErrorMessage(
									UncResultCode.UNC_CTRL_NOT_FOUND
											.getMessage(),
									VtnServiceJsonConsts.CONTROLLERID,
									controllerId));
				} else {
					LOG.info("Controller exists at UNC.");

					final DestinationControllerDao destControllerDao = new DestinationControllerDao();
					final int status = destControllerDao
							.setDestinationController(connection, controllerId);

					if (status == 1) {
						isCommitRequired = true;
						errorCode = UncResultCode.UNC_SUCCESS.getValue();
						LOG.info("Database insertion is successful for Controller id.");
					} else {
						LOG.error("Database insertion is failed for Controller id.");
					}
				}
			}

			/*
			 * If all processing are OK, commit all the database transaction
			 * made for current connection. Otherwise do the roll-back
			 */
			if (isCommitRequired) {
				connection.commit();
				LOG.info("commit successful");
			} else {
				connection.rollback();
				LOG.info("roll-back successful");
			}

			/*
			 * set response, if it is not set during processing for set
			 * destination controller
			 */
			if (errorCode != UncResultCode.UNC_SUCCESS.getValue()) {
				if (getInfo() == null) {
					createErrorInfo(UncResultCode.UNC_INTERNAL_SERVER_ERROR
							.getValue());
				}
			}

		} catch (final SQLException exception) {
			LOG.error(exception, "Internal server error ocuurred.");
			if (exception.getSQLState().equalsIgnoreCase(
					VtnServiceOpenStackConsts.CONFLICT_SQL_STATE)) {
				LOG.error("Conflict found during setting controller id");
				createErrorInfo(
						UncResultCode.UNC_CONFLICT_FOUND.getValue(),
						getCutomErrorMessage(
								UncResultCode.UNC_CONFLICT_FOUND.getMessage(),
								VtnServiceJsonConsts.CONTROLLERID, controllerId));
			} else {
				createErrorInfo(UncResultCode.UNC_INTERNAL_SERVER_ERROR
						.getValue());
			}
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
		LOG.trace("Complete DestinationControllerResource#put()");
		return errorCode;
	}

	/**
	 * Handler method for DELETE operation of DestinationController
	 * 
	 * @see org.opendaylight.vtn.javaapi.resources.AbstractResource#delete()
	 */
	@Override
	public int delete() {
		LOG.trace("Start DestinationControllerResource#delete()");
		int errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();

		boolean isCommitRequired = false;

		Connection connection = null;

		try {
			connection = VtnServiceInitManager.getDbConnectionPoolMap()
					.getConnection();

			final VtnDao vtnDao = new VtnDao();
			if (vtnDao.isVtnExist(connection)) {
				LOG.error("Database deletion is failed, because tenants is exist.");
					createErrorInfo(
							UncResultCode.UNC_METHOD_NOT_ALLOWED.getValue(),
									UncResultCode.UNC_METHOD_NOT_ALLOWED.getMessage());
			} else {
				LOG.info("Tenant is not exists at UNC.");

				final DestinationControllerDao destControllerDao = new DestinationControllerDao();
				final int status = destControllerDao
						.deleteDestinationController(connection);

				if (status == 1) {
					isCommitRequired = true;
					errorCode = UncResultCode.UNC_SUCCESS.getValue();
					LOG.info("Database deletion is successful for Controller id.");
				} else {
					LOG.error("Database deletion is failed for Controller id.");
					createErrorInfo(
							UncResultCode.UNC_CTRL_NOT_FOUND.getValue(),
							getCutomErrorMessage(
									UncResultCode.UNC_CTRL_NOT_FOUND.getMessage(),
									VtnServiceJsonConsts.CONTROLLERID,
									VtnServiceConsts.EMPTY_STRING));
				}

				/*
			 	* If all processing are OK, commit all the database transaction
			 	* made for current connection. Otherwise do the roll-back
			 	*/
				if (isCommitRequired) {
					connection.commit();
					LOG.info("commit successful");
				} else {
					connection.rollback();
					LOG.info("roll-back successful");
				}

				/*
			 	* set response, if it is not set during processing for set
			 	* destination controller
			 	*/
				if (errorCode != UncResultCode.UNC_SUCCESS.getValue()) {
					if (getInfo() == null) {
						createErrorInfo(UncResultCode.UNC_INTERNAL_SERVER_ERROR
								.getValue());
					}
				}
			}
		} catch (final SQLException exception) {
			LOG.error(exception, "Internal server error ocuurred.");
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
		LOG.trace("Complete DestinationControllerResource#delete()");
		return errorCode;
	}

	/**
	 * Handler method for GET operation of DestinationController
	 * 
	 * @see org.opendaylight.vtn.javaapi.resources.AbstractResource#get()
	 */
	@Override
	public int get() {
		LOG.trace("Start DestinationControllerResource#get()");

		int errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();

		Connection connection = null;

		try {
			connection = VtnServiceInitManager.getDbConnectionPoolMap()
					.getConnection();

			final DestinationControllerDao destControllerDao = new DestinationControllerDao();
			final String controllerId = destControllerDao
					.getDestinationController(connection);

			final JsonObject root = new JsonObject();
			if (controllerId != null && !controllerId.isEmpty()) {
				errorCode = UncResultCode.UNC_SUCCESS.getValue();
				LOG.info("Database retrieval is successful for Controller id.");
				root.addProperty(VtnServiceOpenStackConsts.ID, controllerId);
				setInfo(root);
			} else {
				LOG.info("Controller id not available.");
				createErrorInfo(
						UncResultCode.UNC_CTRL_NOT_FOUND.getValue(),
						getCutomErrorMessage(
								UncResultCode.UNC_CTRL_NOT_FOUND.getMessage(),
								VtnServiceJsonConsts.CONTROLLERID,
								VtnServiceConsts.EMPTY_STRING));
			}
		} catch (final SQLException exception) {
			LOG.error(exception, "Internal server error ocuurred.");
			createErrorInfo(UncResultCode.UNC_INTERNAL_SERVER_ERROR.getValue());
		} finally {
			if (connection != null) {
				LOG.info("Free connection...");
				VtnServiceInitManager.getDbConnectionPoolMap().freeConnection(
						connection);
			}
		}
		LOG.trace("Complete DestinationControllerResource#get()");
		return errorCode;
	}
}
