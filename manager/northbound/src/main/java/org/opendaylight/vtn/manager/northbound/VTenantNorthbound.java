/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import javax.ws.rs.Consumes;
import javax.ws.rs.DELETE;
import javax.ws.rs.DefaultValue;
import javax.ws.rs.GET;
import javax.ws.rs.POST;
import javax.ws.rs.PUT;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.Produces;
import javax.ws.rs.QueryParam;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.Response;
import javax.xml.bind.JAXBElement;

import org.codehaus.enunciate.jaxrs.ResponseCode;
import org.codehaus.enunciate.jaxrs.StatusCodes;
import org.codehaus.enunciate.jaxrs.TypeHint;

import org.opendaylight.vtn.manager.IVTNFlowDebugger;
import org.opendaylight.vtn.manager.IVTNManager;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenant;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTenantPath;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.authorization.Privilege;

/**
 * Northbound REST APIs to handle virtual tenants.
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
@Path("/{containerName}/vtns")
public class VTenantNorthbound extends VTNNorthBoundBase {
    /**
     * Returns a list of all virtual tenants.
     *
     * @param containerName  The name of the container.
     * @return  A list of virtual tenants in the container.
     */
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(VTenantList.class)
    @StatusCodes({
            @ResponseCode(code = 200, condition = "Operation successful"),
            @ResponseCode(code = 401, condition = "Authentication failed"),
            @ResponseCode(code = 404, condition = "The specified resource does not exist"),
            @ResponseCode(code = 500, condition = "Failed due to internal error"),
            @ResponseCode(code = 503, condition = "One or more of Controller Services are unavailable")})
    public VTenantList getTenants(
            @PathParam("containerName") String containerName) {
        checkPrivilege(containerName, Privilege.READ);

        IVTNManager mgr = getVTNManager(containerName);

        try {
            return new VTenantList(mgr.getTenants());
        } catch (VTNException e) {
            throw getException(e.getStatus());
        }
    }

    /**
     * Returns a virtual tenant information specified by the given name.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the virtual tenant.
     * @return  Tenant information associated with the specified name.
     */
    @Path("{tenantName}")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(VTenant.class)
    @StatusCodes({
            @ResponseCode(code = 200, condition = "Operation successful"),
            @ResponseCode(code = 401, condition = "Authentication failed"),
            @ResponseCode(code = 404, condition = "The specified resource does not exist"),
            @ResponseCode(code = 500, condition = "Failed due to internal error"),
            @ResponseCode(code = 503, condition = "One or more of Controller Services are unavailable")})
    public VTenant getTenant(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName) {
        checkPrivilege(containerName, Privilege.READ);

        IVTNManager mgr = getVTNManager(containerName);
        VTenantPath path = new VTenantPath(tenantName);
        try {
            return mgr.getTenant(path);
        } catch (VTNException e) {
            throw getException(e.getStatus());
        }
    }

    /**
     * Add a new virtual tenant.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the virtual tenant.
     * @param param          Virtual tenant configuration.
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{tenantName}")
    @POST
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @StatusCodes({
            @ResponseCode(code = 201, condition = "Tenant created successfully"),
            @ResponseCode(code = 401, condition = "Authentication failed"),
            @ResponseCode(code = 404, condition = "The specified resource does not exist"),
            @ResponseCode(code = 406, condition = "Cannot operate on Default Container when other Containers are active"),
            @ResponseCode(code = 409, condition = "Failed to create tenant due to conflicting name"),
            @ResponseCode(code = 415, condition = "Invalid tenant name passed in tenantName parameter"),
            @ResponseCode(code = 500, condition = "Failed to create tenant. Failure Reason included in HTTP Error response"),
            @ResponseCode(code = 503, condition = "One or more of Controller services are unavailable")})
    public Response addTenant(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName,
            @TypeHint(VTenantConfig.class) JAXBElement<VTenantConfig> param) {
        checkPrivilege(containerName, Privilege.WRITE);

        IVTNManager mgr = getVTNManager(containerName);
        VTenantPath path = new VTenantPath(tenantName);
        Status status = mgr.addTenant(path, param.getValue());
        if (status.isSuccess()) {
            return Response.status(Response.Status.CREATED).build();
        }

        throw getException(status);
    }

    /**
     * Modify configuration of existing virtual tenant.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the virtual tenant.
     * @param all            If {@code true} is specified, all attributes
     *                       of the tenant are modified. {@code null} in
     *                       request body field is interpreted as default
     *                       value.
     *                       If {@code false} is specified, all fields to
     *                       which is assigned {@code null} are not modified.
     * @param param          Virtual tenant configuration.
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{tenantName}")
    @PUT
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @StatusCodes({
            @ResponseCode(code = 200, condition = "Tenant modified successfully"),
            @ResponseCode(code = 401, condition = "Authentication failed"),
            @ResponseCode(code = 404, condition = "The specified resource does not exist"),
            @ResponseCode(code = 406, condition = "Cannot operate on Default Container when other Containers are active"),
            @ResponseCode(code = 500, condition = "Failed to modify tenant. Failure Reason included in HTTP Error response"),
            @ResponseCode(code = 503, condition = "One or more of Controller services are unavailable")})
    public Response modifyTenant(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName,
            @DefaultValue("false") @QueryParam("all") boolean all,
            @TypeHint(VTenantConfig.class) JAXBElement<VTenantConfig> param) {
        checkPrivilege(containerName, Privilege.WRITE);

        IVTNManager mgr = getVTNManager(containerName);
        VTenantPath path = new VTenantPath(tenantName);
        Status status = mgr.modifyTenant(path, param.getValue(), all);
        if (status.isSuccess()) {
            return Response.ok().build();
        }

        throw getException(status);
    }

    /**
     * Delete a virtual tenant.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the virtual tenant.
     * @return Response as dictated by the HTTP Response code.
     */
    @Path("{tenantName}")
    @DELETE
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @StatusCodes({
            @ResponseCode(code = 200, condition = "Tenant deleted successfully"),
            @ResponseCode(code = 401, condition = "Authentication failed"),
            @ResponseCode(code = 404, condition = "The specified resource does not exist"),
            @ResponseCode(code = 406, condition = "Cannot operate on Default Container when other Containers are active"),
            @ResponseCode(code = 500, condition = "Failed to delete tenant. Failure Reason included in HTTP Error response"),
            @ResponseCode(code = 503, condition = "One or more of Controller service is unavailable") })
    public Response deleteTenant(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName) {
        checkPrivilege(containerName, Privilege.WRITE);

        IVTNManager mgr = getVTNManager(containerName);
        VTenantPath path = new VTenantPath(tenantName);
        Status status = mgr.removeTenant(path);
        if (status.isSuccess()) {
            return Response.ok().build();
        }

        throw getException(status);
    }

    /**
     * Remove all flow entries in the specified virtual tenant.
     *
     * <p>
     *   This interface is provided only for debugging purpose, and is
     *   available only if the system property {@code vtn.debug} is defined
     *   as {@code true}.
     * </p>
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the virtual tenant.
     * @return Response as dictated by the HTTP Response code.
     */
    @Path("{tenantName}/flows")
    @DELETE
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @StatusCodes({
            @ResponseCode(code = 200, condition = "Operation completed successfully"),
            @ResponseCode(code = 401, condition = "Authentication failed"),
            @ResponseCode(code = 404, condition = "The specified resource does not exist"),
            @ResponseCode(code = 406, condition = "Cannot operate on Default Container when other Containers are active"),
            @ResponseCode(code = 500, condition = "Failed to remove flow entries. Failure Reason included in HTTP Error response"),
            @ResponseCode(code = 503, condition = "One or more of Controller service is unavailable") })
    public Response removeAllFlows(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName) {
        checkPrivilege(containerName, Privilege.WRITE);

        IVTNFlowDebugger debugger = getVTNFlowDebugger(containerName);
        VTenantPath path = new VTenantPath(tenantName);
        Status status = debugger.removeAllFlows(path);
        if (status.isSuccess()) {
            return Response.ok().build();
        }

        throw getException(status);
    }
}
