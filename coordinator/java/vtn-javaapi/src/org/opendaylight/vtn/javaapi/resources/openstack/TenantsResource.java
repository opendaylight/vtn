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
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
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
 * Resource class for handling POST requests for Tenants
 */
@UNCVtnService(path = VtnServiceOpenStackConsts.TENANTS_PATH)
public class TenantsResource extends AbstractResource {

	/* Logger instance */
	private static final Logger LOG = Logger.getLogger(TenantsResource.class
			.getName());

	/**
	 * Constructor that initialize the validation instance for current resource
	 * instance
	 */
	public TenantsResource() {
		setValidator(new TenantResourceValidator(this));
	}

	/**
	 * Handler method for POST operation of Tenant
	 * 
	 * @see org.opendaylight.vtn.javaapi.resources.AbstractResource#post(com
	 *      .google.gson.JsonObject)
	 */
	@Override
	public int post(JsonObject requestBody) {
		LOG.trace("Start TenantsResource#post()");

		int errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();

		boolean isCommitRequired = false;
		String generatedVtnName = null;
		Connection connection = null;

		try {
			connection = VtnServiceInitManager.getDbConnectionPoolMap()
					.getConnection();

			final ResourceIdManager resourceIdManager = new ResourceIdManager();
			int counter = -1;
			/*
			 * auto-generation process, based on the availability of "id"
			 * parameter in request body
			 */
			if (!requestBody.has(VtnServiceOpenStackConsts.ID)) {
				LOG.info("Resource id auto-generation is required.");

				final FreeCounterBean freeCounterBean = new FreeCounterBean();
				freeCounterBean
						.setResourceId(VtnServiceOpenStackConsts.TENANT_RES_ID);
				freeCounterBean
						.setVtnName(VtnServiceOpenStackConsts.DEFAULT_VTN);

				counter = resourceIdManager.getResourceCounter(connection,
						freeCounterBean);

				if (counter != -1) {
					LOG.info("Resource id auto-generation is successful.");
					generatedVtnName = VtnServiceOpenStackConsts.VTN_PREFIX
							+ counter;
					requestBody.addProperty(VtnServiceOpenStackConsts.ID,
							generatedVtnName);
				} else {
					LOG.error("Resource id auto-generation is failed.");
				}
			} else {
				LOG.info("Resource id auto-generation is not required.");
				counter = 0;
				generatedVtnName = requestBody
						.get(VtnServiceOpenStackConsts.ID).getAsString();
			}

			LOG.debug("Counter : " + counter);
			LOG.debug("vtn_name : " + generatedVtnName);

			if (counter >= 0) {
				/*
				 * resource insertion in database, if is is successful then
				 * continue to execute operations at UNC. Otherwise return HTTP
				 * 409
				 */

				final VtnBean vtnBean = new VtnBean();
				vtnBean.setVtnId(counter);
				vtnBean.setVtnName(generatedVtnName);

				final VtnDao vtnDao = new VtnDao();
				final int status = vtnDao.insert(connection, vtnBean);
				if (status == 1) {
					LOG.info("Resource insertion successful at database operation.");

					final RestResource restResource = new RestResource();

					errorCode = createVtn(requestBody, restResource);

					if (errorCode == UncCommonEnum.UncResultCode.UNC_SUCCESS
							.getValue()) {
						LOG.info("VTN creation is successful at UNC.");
						isCommitRequired = true;
						if (counter != 0) {
							final JsonObject response = new JsonObject();
							response.addProperty(VtnServiceOpenStackConsts.ID,
									generatedVtnName);
							setInfo(response);
						}
					} else {
						LOG.error("VTN creation is failed at UNC.");
					}
					checkForSpecificErrors(restResource.getInfo());
				} else {
					LOG.error("Resource insertion failed at database operation.");
				}
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
								VtnServiceOpenStackConsts.TENANT_ID,
								generatedVtnName));
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
		LOG.trace("Complete TenantsResource#post()");
		return errorCode;
	}

	/**
	 * 
	 * @param requestBody
	 * @param restResource
	 * @return
	 */
	private int createVtn(JsonObject requestBody,
			final RestResource restResource) {
		int errorCode;
		/*
		 * Create request body for VTN creation
		 */
		final JsonObject vtnRequestBody = VtnResourcesGenerator
				.getCreateVtnRequestBody(requestBody);

		/*
		 * execute create VTN request
		 */
		restResource.setPath(VtnServiceOpenStackConsts.VTN_PATH);
		restResource.setSessionID(getSessionID());
		restResource.setConfigID(getConfigID());

		errorCode = restResource.post(vtnRequestBody);
		return errorCode;
	}
}
