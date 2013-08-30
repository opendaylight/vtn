/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import java.util.List;

import javax.ws.rs.WebApplicationException;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.SecurityContext;

import org.opendaylight.vtn.manager.IVTNManager;

import org.opendaylight.controller.containermanager.IContainerManager;
import org.opendaylight.controller.northbound.commons.RestMessages;
import org.opendaylight.controller.northbound.commons.exception.InternalServerErrorException;
import org.opendaylight.controller.northbound.commons.exception.NotAcceptableException;
import org.opendaylight.controller.northbound.commons.exception.ResourceConflictException;
import org.opendaylight.controller.northbound.commons.exception.ResourceNotFoundException;
import org.opendaylight.controller.northbound.commons.exception.ServiceUnavailableException;
import org.opendaylight.controller.northbound.commons.exception.UnauthorizedException;
import org.opendaylight.controller.northbound.commons.exception.UnsupportedMediaTypeException;
import org.opendaylight.controller.northbound.commons.utils.NorthboundUtils;
import org.opendaylight.controller.sal.authorization.Privilege;
import org.opendaylight.controller.sal.utils.ServiceHelper;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * Base class of VTN Manager REST API.
 */
public abstract class VTNNorthBoundBase {
    /**
     * HTTP user name sent by a client.
     */
    private String  userName;

    /**
     * Set security context.
     *
     * @param context  A security context to be set.
     */
    @Context
    public void setSecurityContext(SecurityContext context) {
        if (context != null && context.getUserPrincipal() != null) userName = context.getUserPrincipal().getName();
    }

    /**
     * Return HTTP user name.
     *
     * @return  User name.
     */
    protected String getUserName() {
        return userName;
    }

    /**
     * Check access privilege.
     *
     * @param containerName  The name of the container.
     * @param priv  Access privilege.
     * @throws UnauthorizedException  A client is not authorized.
     */
    protected void checkPrivilege(String containerName, Privilege priv) {
        String name = getUserName();

        if (!NorthboundUtils.isAuthorized(name, containerName, priv, this)) {
            String msg = "User is not authorized to perform this operation " +
                "on container "+ containerName;
            throw new UnauthorizedException(msg);
        }
    }

    /**
     * Throw a ServiceUnavailableException.
     *
     * @param service  The name of unavailable service.
     * @throws ServiceUnavailableException  Always thrown.
     */
    protected void serviceUnavailable(String service)
        throws ServiceUnavailableException {
        String msg = service + ": " + RestMessages.SERVICEUNAVAILABLE;
        throw new ServiceUnavailableException(msg);
    }

    /**
     * Convert failure status returned by the VTN manager into web application
     * exception.
     *
     * @param status  Failure status.
     * @return  An exception.
     */
    protected WebApplicationException getException(Status status) {
        assert !status.isSuccess();

        StatusCode code = status.getCode();
        String desc = status.getDescription();
        if (code.equals(StatusCode.BADREQUEST)) {
            return new UnsupportedMediaTypeException(desc);
        }
        if (code.equals(StatusCode.CONFLICT)) {
            return new ResourceConflictException(desc);
        }
        if (code.equals(StatusCode.NOTACCEPTABLE)) {
            return new NotAcceptableException(desc);
        }
        if (code.equals(StatusCode.NOTFOUND)) {
            throw new ResourceNotFoundException(desc);
        }

        String msg = RestMessages.INTERNALERROR + ": " + desc;
        return new InternalServerErrorException(msg);
    }

    /**
     * Return the VTN manager service associated with the specified container.
     *
     * @param containerName  The container name.
     * @return  The VTN manager service associated with the specified
     *          container.
     * @throws ServiceUnavailableException
     *    Unable to get VTN manager service.
     * @throws ResourceNotFoundException
     *    Invalid container name is specified.
     */
    protected IVTNManager getVTNManager(String containerName) {
        IContainerManager containerManager = (IContainerManager)ServiceHelper.
            getGlobalInstance(IContainerManager.class, this);
        if (containerManager == null) {
            serviceUnavailable("Container");
        }

        boolean found = false;
        List<String> containerNames = containerManager.getContainerNames();
        for (String cName : containerNames) {
            if (cName.trim().equalsIgnoreCase(containerName.trim())) {
                found = true;
            }
        }

        if (!found) {
            String msg = containerName + ": " +
                RestMessages.NOCONTAINER.toString();
            throw new ResourceNotFoundException(msg);
        }

        IVTNManager mgr = (IVTNManager)ServiceHelper.
            getInstance(IVTNManager.class, containerName, this);
        if (mgr == null) {
            serviceUnavailable("VTN Manager");
        }

        return mgr;
    }
}
