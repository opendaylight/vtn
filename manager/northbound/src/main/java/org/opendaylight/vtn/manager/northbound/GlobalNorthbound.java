/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import static java.net.HttpURLConnection.HTTP_INTERNAL_ERROR;
import static java.net.HttpURLConnection.HTTP_OK;
import static java.net.HttpURLConnection.HTTP_UNAUTHORIZED;
import static java.net.HttpURLConnection.HTTP_UNAVAILABLE;

import javax.ws.rs.GET;
import javax.ws.rs.Path;
import javax.ws.rs.Produces;
import javax.ws.rs.core.MediaType;

import org.codehaus.enunciate.jaxrs.ResponseCode;
import org.codehaus.enunciate.jaxrs.StatusCodes;
import org.codehaus.enunciate.jaxrs.TypeHint;

import org.opendaylight.vtn.manager.BundleVersion;
import org.opendaylight.vtn.manager.IVTNGlobal;

import org.opendaylight.controller.sal.utils.ServiceHelper;
import org.opendaylight.controller.sal.authorization.Privilege;

/**
 * This class provides Northbound REST APIs to handle global, which means
 * container independent, resources managed by the VTN Manager.
 */
@Path("/")
public class GlobalNorthbound extends VTNNorthBoundBase {
    /**
     * Return the version information of the VTN Manager.
     *
     * @return  <strong>version</strong> element represents version
     *          information of the VTN Manager.
     */
    @Path("version")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(ManagerVersion.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public ManagerVersion getManagerVersion() {
        checkPrivilege(Privilege.READ);

        IVTNGlobal global = getVTNGloabl();
        int api = global.getApiVersion();
        BundleVersion bundle = global.getBundleVersion();
        return new ManagerVersion(api, bundle);
    }

    /**
     * Return the VTN Manager global service.
     *
     * @return  The OSGi service instance specified by the given class.
     * @throws org.opendaylight.controller.northbound.commons.exception.ServiceUnavailableException
     *    Unable to get OSGi service.
     */
    private IVTNGlobal getVTNGloabl() {
        IVTNGlobal global = (IVTNGlobal)
            ServiceHelper.getGlobalInstance(IVTNGlobal.class, this);
        if (global == null) {
            throw serviceUnavailable("VTN Global Service");
        }

        return global;
    }
}
