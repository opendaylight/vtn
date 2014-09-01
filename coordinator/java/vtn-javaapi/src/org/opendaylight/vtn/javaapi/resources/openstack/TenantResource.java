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
import org.opendaylight.vtn.javaapi.init.VtnServiceInitManager;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.openstack.beans.FreeCounterBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VtnBean;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;
import org.opendaylight.vtn.javaapi.openstack.convertor.VtnResourcesGenerator;
import org.opendaylight.vtn.javaapi.openstack.dao.VtnDao;
import org.opendaylight.vtn.javaapi.openstack.dbmanager.ResourceIdManager;
import org.opendaylight.vtn.javaapi.openstack.validation.TenantResourceValidator;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;

/**
 * Resource class for handling PUT and DELETE requests for Tenants
 */
@UNCVtnService(path = VtnServiceOpenStackConsts.TENANT_PATH)
public class TenantResource extends AbstractResource {

	/* Logger instance */
	private static final Logger LOG = Logger.getLogger(TenantResource.class
			.getName());

	@UNCField(VtnServiceOpenStackConsts.TENANT_ID)
	private String tenantId;

	/**
	 * Constructor that initialize the validation instance for current resource
	 * instance
	 */
	public TenantResource() {
		setValidator(new TenantResourceValidator(this));
	}

	public String getTenantId() {
		return tenantId;
	}

	/**
	 * Handler method for PUT operation of Tenant
	 * 
	 * @see org.opendaylight.vtn.javaapi.resources.AbstractResource#put(com.
	 *      google.gson.JsonObject)
	 */
	@Override
	public int put(JsonObject requestBody) {
		LOG.trace("Start TenantsResource#put()");

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

				errorCode = updateVtn(requestBody, restResource);

				if (errorCode == UncCommonEnum.UncResultCode.UNC_SUCCESS
						.getValue()) {
					LOG.error("VTN Update at UNC is successful.");
				} else {
					LOG.info("VTN Update at UNC is failed.");
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
		LOG.trace("Complete TenantsResource#put()");
		return errorCode;
	}

	/**
	 * Handler method for DELETE operation of Tenant
	 * 
	 * @see org.opendaylight.vtn.javaapi.resources.AbstractResource#delete()
	 */
	@Override
	public int delete() {
		LOG.trace("Start TenantResource#delete()");

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

				final VtnBean vtnBean = new VtnBean();
				vtnBean.setVtnName(getTenantId());

				if (getTenantId().startsWith(VtnServiceOpenStackConsts.VTN_PREFIX)) {
					final String counter = getTenantId().replace(
							VtnServiceOpenStackConsts.VTN_PREFIX,
							VtnServiceConsts.EMPTY_STRING);
					try {
						vtnBean.setVtnId(Integer.parseInt(counter));
					} catch (final NumberFormatException e) {
						LOG.debug("Resource Id was not auto-generated during Create operation : "
								+ counter);
						vtnBean.setVtnId(0);
					}
				} else {
					vtnBean.setVtnId(0);
				}

				final FreeCounterBean freeCounterBean = new FreeCounterBean();
				freeCounterBean
						.setResourceId(VtnServiceOpenStackConsts.TENANT_RES_ID);
				freeCounterBean
						.setVtnName(VtnServiceOpenStackConsts.DEFAULT_VTN);
				freeCounterBean.setResourceCounter(vtnBean.getVtnId());

				final ResourceIdManager resourceIdManager = new ResourceIdManager();

				if (resourceIdManager.deleteResourceId(connection,
						freeCounterBean, vtnBean)) {
					LOG.info("Deletion operation from database is successfull.");

					final RestResource restResource = new RestResource();

					errorCode = deleteVtn(restResource);

					if (errorCode == UncCommonEnum.UncResultCode.UNC_SUCCESS
							.getValue()) {
						LOG.error("VTN Deletion successful at UNC.");
						isCommitRequired = true;
					} else {
						LOG.error("VTN Deletion failed at UNC.");
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
			 * If all processing are OK, commit all the database transaction
			 * made for current connection. Otherwise do the roll-back
			 */
			if (isCommitRequired) {
				// connection.commit();
				setOpenStackConnection(connection);
				LOG.info("commit successful");
			} else {
				connection.rollback();
				LOG.info("roll-back successful");
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
					LOG.info("roll-back successful.");
				} catch (final SQLException e) {
					LOG.error(e, "Rollback error : " + e);
				}
				LOG.info("Free connection...");
				VtnServiceInitManager.getDbConnectionPoolMap().freeConnection(
						connection);
			}
		}
		LOG.trace("Complete TenantResource#delete()");
		return errorCode;
	}

	/**
	 * 
	 * @param requestBody
	 * @param restResource
	 * @return
	 */
	private int updateVtn(JsonObject requestBody,
			final RestResource restResource) {
		/*
		 * Create request body for VTN update
		 */
		final JsonObject vtnRequestBody = VtnResourcesGenerator
				.getUpdateVtnRequestBody(requestBody);

		/*
		 * execute update VTN request
		 */
		StringBuilder sb = new StringBuilder();
		sb.append(VtnServiceOpenStackConsts.VTN_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getTenantId());

		restResource.setPath(sb.toString());
		restResource.setSessionID(getSessionID());
		restResource.setConfigID(getConfigID());

		return restResource.put(vtnRequestBody);
	}

	/**
	 * 
	 * @param restResource
	 * @return
	 */
	private int deleteVtn(final RestResource restResource) {
		/*
		 * execute delete VTN request
		 */
		restResource.setPath(VtnServiceOpenStackConsts.VTN_PATH
				+ VtnServiceOpenStackConsts.URI_CONCATENATOR + getTenantId());
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
