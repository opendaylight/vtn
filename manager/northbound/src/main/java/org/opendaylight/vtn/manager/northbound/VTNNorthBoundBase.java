/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import java.security.Principal;

import javax.ws.rs.WebApplicationException;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.SecurityContext;

import org.opendaylight.vtn.manager.IVTNFlowDebugger;
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
        if (context != null) {
            Principal p = context.getUserPrincipal();
            if (p != null) {
                userName = p.getName();
            }
        }
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
        if (code == StatusCode.BADREQUEST) {
            return new UnsupportedMediaTypeException(desc);
        }
        if (code == StatusCode.CONFLICT) {
            return new ResourceConflictException(desc);
        }
        if (code == StatusCode.NOTACCEPTABLE) {
            return new NotAcceptableException(desc);
        }
        if (code == StatusCode.NOTFOUND) {
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
        IVTNManager mgr = (IVTNManager)
            getContainerService(IVTNManager.class, containerName);
        if (mgr == null) {
            serviceUnavailable("VTN Manager");
        }

        return mgr;
    }

    /**
     * Return the VTN flow debugger service associated with the specified
     * container.
     *
     * @param containerName  The container name.
     * @return  The VTN flow debugger service associated with the specified
     *          container.
     * @throws ServiceUnavailableException
     *    Unable to get VTN manager service.
     * @throws ResourceNotFoundException
     *    Invalid container name is specified.
     */
    protected IVTNFlowDebugger getVTNFlowDebugger(String containerName) {
        IVTNFlowDebugger debugger = (IVTNFlowDebugger)
            getContainerService(IVTNFlowDebugger.class, containerName);
        if (debugger == null) {
            serviceUnavailable("VTN Flow Debugger");
        }

        return debugger;
    }

    /**
     * Return the specified OSGi service bound to the specified container.
     *
     * @param cls            The targer class.
     * @param containerName  The container name.
     * @return  The OSGi service instance associated with the specified
     *          container.
     * @throws ServiceUnavailableException
     *    Unable to get OSGi service.
     * @throws ResourceNotFoundException
     *    Invalid container name is specified.
     */
    private Object getContainerService(Class<?> cls, String containerName) {
        IContainerManager containerManager = (IContainerManager)ServiceHelper.
            getGlobalInstance(IContainerManager.class, this);
        if (containerManager == null) {
            serviceUnavailable("Container");
        }

        if (!containerManager.doesContainerExist(containerName)) {
            String msg = containerName + ": " +
                RestMessages.NOCONTAINER.toString();
            throw new ResourceNotFoundException(msg);
        }

        return ServiceHelper.getInstance(cls, containerName, this);
    }
}
