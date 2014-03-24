/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.resources;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.validation.ApiVersionResourceValidator;

/**
 * The Class ApiVersionResource. This class handles get requests for JavaAPI
 * version
 */
@UNCVtnService(path = "/api_version")
public class ApiVersionResource extends AbstractResource {

	private static final Logger LOG = Logger.getLogger(ApiVersionResource.class
			.getName());

	/**
	 * ApiVersionResource Constructor
	 */
	public ApiVersionResource() {
		LOG.trace("Start ApiVersionResource#ApiVersionResource()");
		setValidator(new ApiVersionResourceValidator(this));
		LOG.trace("Complete ApiVersionResource#ApiVersionResource()");
	}

	/**
	 * Implementation of get method of JavaAPI version
	 */
	@Override
	public final int get() {
		LOG.trace("Start ApiVersionResource#get()");
		int status = UncResultCode.UNC_SERVER_ERROR.getValue();
		final JsonObject root = new JsonObject();
		final JsonObject apiVersion = new JsonObject();
		apiVersion.addProperty(VtnServiceJsonConsts.VERSION,
				VtnServiceConsts.JAVAAPI_VERSION);
		root.add(VtnServiceJsonConsts.APIVERSION, apiVersion);
		setInfo(root);
		LOG.debug("Response object created successfully");
		status = UncResultCode.UNC_SUCCESS.getValue();
		LOG.trace("Complete ApiVersionResource#get()");
		return status;
	}

}
