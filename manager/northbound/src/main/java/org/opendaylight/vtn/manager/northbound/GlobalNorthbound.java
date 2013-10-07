/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import javax.ws.rs.GET;
import javax.ws.rs.Path;
import javax.ws.rs.Produces;
import javax.ws.rs.core.MediaType;

import org.codehaus.enunciate.jaxrs.ResponseCode;
import org.codehaus.enunciate.jaxrs.StatusCodes;
import org.codehaus.enunciate.jaxrs.TypeHint;

import org.opendaylight.vtn.manager.BundleVersion;
import org.opendaylight.vtn.manager.IVTNGlobal;

import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.controller.sal.utils.ServiceHelper;
import org.opendaylight.controller.sal.authorization.Privilege;

/**
 * Northbound REST APIs to handle container-independent resources.
 *
 * <br>
 * <br>
 * Authentication scheme : <b>HTTP Basic</b><br>
 * Authentication realm : <b>opendaylight</b><br>
 * Transport : <b>HTTP and HTTPS</b><br>
 * <br>
 * HTTPS Authentication is disabled by default. Administrator can enable it in
 * tomcat-server.xml after adding a proper keystore / SSL certificate from a
 * trusted authority.<br>
 * More info :
 * http://tomcat.apache.org/tomcat-7.0-doc/ssl-howto.html#Configuration
 */
@Path("/")
public class GlobalNorthbound extends VTNNorthBoundBase {
    /**
     * Returns the version information of the VTN Manager.
     *
     * @return  The version information of the VTN Manager.
     */
    @Path("version")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(ManagerVersion.class)
    @StatusCodes({
            @ResponseCode(code = 200, condition = "Operation successful"),
            @ResponseCode(code = 401, condition = "Authentication failed"),
            @ResponseCode(code = 500, condition = "Failed due to internal error"),
            @ResponseCode(code = 503, condition = "One or more of Controller Services are unavailable")})
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
            serviceUnavailable("VTN Global Service");
        }

        return global;
    }

    /**
     * Check access privilege.
     *
     * @param priv  Access privilege.
     * @throws org.opendaylight.controller.northbound.commons.exception.UnauthorizedException
     *    A client is not authorized.
     */
    private void checkPrivilege(Privilege priv) {
        checkPrivilege(GlobalConstants.DEFAULT.toString(), priv);
    }
}
