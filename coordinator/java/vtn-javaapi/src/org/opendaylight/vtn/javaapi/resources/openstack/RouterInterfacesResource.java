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
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.init.VtnServiceInitManager;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.openstack.beans.FreeCounterBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VBridgeBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VRouterBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VRouterInterfaceBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VtnBean;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;
import org.opendaylight.vtn.javaapi.openstack.convertor.VbrResourcesGenerator;
import org.opendaylight.vtn.javaapi.openstack.convertor.VrtResourcesGenerator;
import org.opendaylight.vtn.javaapi.openstack.dao.VBridgeDao;
import org.opendaylight.vtn.javaapi.openstack.dao.VRouterDao;
import org.opendaylight.vtn.javaapi.openstack.dao.VRouterInterfaceDao;
import org.opendaylight.vtn.javaapi.openstack.dao.VtnDao;
import org.opendaylight.vtn.javaapi.openstack.dbmanager.ResourceIdManager;
import org.opendaylight.vtn.javaapi.openstack.validation.RouterInterfaceResourceValidator;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;

/**
 * Resource class for handling POST requests for Router Interface
 */
@UNCVtnService(path = VtnServiceOpenStackConsts.ROUTER_INTERFACES_PATH)
public class RouterInterfacesResource extends AbstractResource {

	/* Logger instance */
	private static final Logger LOG = Logger
			.getLogger(RouterInterfacesResource.class.getName());

	@UNCField(VtnServiceOpenStackConsts.TENANT_ID)
	private String tenantId;

	@UNCField(VtnServiceOpenStackConsts.ROUTER_ID)
	private String routerId;

	/**
	 * Constructor that initialize the validation instance for current resource
	 * instance
	 */
	public RouterInterfacesResource() {
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
	 * Handler method for POST operation of Router Interface
	 * 
	 * @see org.opendaylight.vtn.javaapi.resources.AbstractResource#post(com
	 *      .google.gson.JsonObject)
	 */
	@Override
	public int post(JsonObject requestBody) {
		LOG.trace("Start RouterInterfacesResource#post()");

		int errorCode = UncResultCode.UNC_SERVER_ERROR.getValue();

		boolean isCommitRequired = false;
		String generatedIfName = null;
		Connection connection = null;
		int counter = -1;

		try {
			connection = VtnServiceInitManager.getDbConnectionPoolMap()
					.getConnection();
			/*
			 * Check for instances that they exists or not, if not then return
			 * 404 error
			 */
			if (checkForNotFoundResources(connection, requestBody)) {
				final ResourceIdManager resourceIdManager = new ResourceIdManager();

				/*
				 * generate "id" for vBridge interface, vRouter interface and
				 * vLink
				 */
				LOG.info("Resource id auto-generation is required.");

				final FreeCounterBean freeCounterBean = new FreeCounterBean();
				freeCounterBean
						.setResourceId(VtnServiceOpenStackConsts.PORT_RES_ID);
				freeCounterBean.setVtnName(getTenantId());

				counter = resourceIdManager.getResourceCounter(connection,
						freeCounterBean);

				if (counter != -1) {
					LOG.debug("Resource id auto-generation is successfull : "
							+ counter);

					if (counter > VtnServiceOpenStackConsts.MAX_ROUTER_IF_LIMIT) {
						LOG.warning("Router interface creation reached at maximum limit");
						createErrorInfo(UncResultCode.UNC_INTERNAL_SERVER_ERROR
								.getValue());
						return errorCode;
					}

					// if id is generated successfully
					generatedIfName = VtnServiceOpenStackConsts.IF_PREFIX
							+ counter;

					requestBody.addProperty(VtnServiceOpenStackConsts.ID,
							generatedIfName);

					requestBody.addProperty(VtnServiceJsonConsts.VLKNAME,
							VtnServiceOpenStackConsts.VLK_PREFIX + counter);
				} else {
					LOG.error("Resource id auto-generation is failed.");
				}

				if (counter >= 1) {
					/*
					 * resource insertion in database, if is is successful then
					 * continue to execute operations at UNC. Otherwise return
					 * HTTP 409
					 */
					final VRouterInterfaceBean vInterfaceBean = new VRouterInterfaceBean();
					vInterfaceBean.setVrtIfId(counter);
					vInterfaceBean.setVtnName(getTenantId());
					vInterfaceBean.setVrtName(getRouterId());
					vInterfaceBean.setVrtIfName(generatedIfName);
					vInterfaceBean.setVbrName(requestBody.get(
							VtnServiceOpenStackConsts.NET_ID).getAsString());

					final VRouterInterfaceDao vInterfaceDao = new VRouterInterfaceDao();
					final int status = vInterfaceDao.insert(connection,
							vInterfaceBean);

					if (status == 1) {
						LOG.info("Resource insertion successful at database operation.");

						final RestResource restResource = new RestResource();

						errorCode = createVBridgeInterface(requestBody,
								restResource);

						if (errorCode == UncResultCode.UNC_SUCCESS.getValue()) {
							errorCode = createVRouterInterface(requestBody,
									restResource);

							if (errorCode == UncResultCode.UNC_SUCCESS
									.getValue()) {
								errorCode = createVLink(requestBody,
										restResource);

								if (errorCode == UncCommonEnum.UncResultCode.UNC_SUCCESS
										.getValue()) {
									LOG.info("vlink creation is successful at UNC.");
									isCommitRequired = true;
									final JsonObject response = new JsonObject();
									response.addProperty(
											VtnServiceOpenStackConsts.ID,
											String.valueOf(counter));
									setInfo(response);
								} else {
									errorCode = UncResultCode.UNC_SERVER_ERROR
											.getValue();
									LOG.error("vlink creation is failed at UNC.");
								}
							} else {
								errorCode = UncResultCode.UNC_SERVER_ERROR
										.getValue();
								LOG.error("vRouter interface creation is failed at UNC.");
							}
						} else {
							errorCode = UncResultCode.UNC_SERVER_ERROR
									.getValue();
							LOG.error("vBridge interface creation is failed at UNC.");
						}
						checkForSpecificErrors(restResource.getInfo());
					} else {
						LOG.error("Resource insertion failed at database operation.");
					}
				} else {
					LOG.error("Error occurred while generation of id or setting controller_id");
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
								VtnServiceOpenStackConsts.IF_ID,
								String.valueOf(counter)));
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
		LOG.trace("Complete RouterInterfacesResource#post()");
		return errorCode;
	}

	/**
	 * Create vRouter interface at UNC
	 * 
	 * @param requestBody
	 *            - OpenStack request body
	 * @param restResource
	 *            - RestResource instance
	 * @return - erorrCode, 200 for Success
	 */
	private int createVRouterInterface(JsonObject requestBody,
			RestResource restResource) {
		/*
		 * Create request body for vRouter interface creation
		 */
		final JsonObject vrtIfRequestBody = VrtResourcesGenerator
				.getCreateVrtIfRequestBody(requestBody);

		/*
		 * execute create vRouter interface request
		 */
		StringBuilder sb = new StringBuilder();
		sb.append(VtnServiceOpenStackConsts.VTN_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getTenantId());
		sb.append(VtnServiceOpenStackConsts.VROUTER_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getRouterId());
		sb.append(VtnServiceOpenStackConsts.INTERFACE_PATH);

		restResource.setPath(sb.toString());
		restResource.setSessionID(getSessionID());
		restResource.setConfigID(getConfigID());

		return restResource.post(vrtIfRequestBody);
	}

	/**
	 * Create vBridge interface at UNC
	 * 
	 * @param requestBody
	 *            - OpenStack request body
	 * @param restResource
	 *            - RestResource instance
	 * @return - erorrCode, 200 for Success
	 */
	private int createVBridgeInterface(JsonObject requestBody,
			RestResource restResource) {
		/*
		 * Create request body for vBridge interface creation
		 */
		final JsonObject vbrIfRequestBody = VbrResourcesGenerator
				.getCreateVbrIfRequestBody(requestBody);

		/*
		 * execute create vBridge interface request
		 */
		StringBuilder sb = new StringBuilder();
		sb.append(VtnServiceOpenStackConsts.VTN_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getTenantId());
		sb.append(VtnServiceOpenStackConsts.VBRIDGE_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(requestBody.get(VtnServiceOpenStackConsts.NET_ID)
				.getAsString());
		sb.append(VtnServiceOpenStackConsts.INTERFACE_PATH);

		restResource.setPath(sb.toString());
		restResource.setSessionID(getSessionID());
		restResource.setConfigID(getConfigID());

		return restResource.post(vbrIfRequestBody);
	}

	/**
	 * Create vLink at UNC
	 * 
	 * @param requestBody
	 *            - OpenStack request body
	 * @param restResource
	 *            - RestResource instance
	 * @return - erorrCode, 200 for Success
	 */
	private int createVLink(JsonObject requestBody, RestResource restResource) {
		/*
		 * Create request body for vlink creation
		 */
		final JsonObject vlinkRequestBody = new JsonObject();
		final JsonObject vlink = new JsonObject();

		vlink.addProperty(VtnServiceJsonConsts.VLKNAME,
				requestBody.get(VtnServiceJsonConsts.VLKNAME).getAsString());

		vlink.addProperty(VtnServiceJsonConsts.VNODE1NAME,
				requestBody.get(VtnServiceOpenStackConsts.NET_ID).getAsString());
		vlink.addProperty(VtnServiceJsonConsts.IF1NAME,
				requestBody.get(VtnServiceOpenStackConsts.ID).getAsString());

		vlink.addProperty(VtnServiceJsonConsts.VNODE2NAME, getRouterId());
		vlink.addProperty(VtnServiceJsonConsts.IF2NAME,
				requestBody.get(VtnServiceOpenStackConsts.ID).getAsString());

		vlinkRequestBody.add(VtnServiceJsonConsts.VLINK, vlink);

		/*
		 * execute create vBridge interface request
		 */
		StringBuilder sb = new StringBuilder();
		sb.append(VtnServiceOpenStackConsts.VTN_PATH);
		sb.append(VtnServiceOpenStackConsts.URI_CONCATENATOR);
		sb.append(getTenantId());
		sb.append(VtnServiceOpenStackConsts.VLINK_PATH);

		restResource.setPath(sb.toString());
		restResource.setSessionID(getSessionID());
		restResource.setConfigID(getConfigID());

		return restResource.post(vlinkRequestBody);
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
	private boolean checkForNotFoundResources(Connection connection,
			JsonObject requestBody) throws SQLException {
		boolean resourceFound = false;
		VtnBean vtnBean = new VtnBean();
		vtnBean.setVtnName(getTenantId());
		if (new VtnDao().isVtnFound(connection, vtnBean)) {
			VRouterBean vRouterBean = new VRouterBean();
			vRouterBean.setVtnName(getTenantId());
			vRouterBean.setVrtName(getRouterId());
			if (new VRouterDao().isVrtFound(connection, vRouterBean)) {
				VBridgeBean vBridgeBean = new VBridgeBean();
				vBridgeBean.setVtnName(getTenantId());
				vBridgeBean.setVbrName(requestBody.get(
						VtnServiceOpenStackConsts.NET_ID).getAsString());
				if (new VBridgeDao().isVbrFound(connection, vBridgeBean)) {
					resourceFound = true;
				} else {
					createErrorInfo(
							UncResultCode.UNC_NOT_FOUND.getValue(),
							getCutomErrorMessage(
									UncResultCode.UNC_NOT_FOUND.getMessage(),
									VtnServiceOpenStackConsts.NET_ID,
									requestBody.get(
											VtnServiceOpenStackConsts.NET_ID)
											.getAsString()));
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
