/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import static java.net.HttpURLConnection.HTTP_BAD_REQUEST;
import static java.net.HttpURLConnection.HTTP_CONFLICT;
import static java.net.HttpURLConnection.HTTP_CREATED;
import static java.net.HttpURLConnection.HTTP_INTERNAL_ERROR;
import static java.net.HttpURLConnection.HTTP_NOT_ACCEPTABLE;
import static java.net.HttpURLConnection.HTTP_NOT_FOUND;
import static java.net.HttpURLConnection.HTTP_OK;
import static java.net.HttpURLConnection.HTTP_UNAUTHORIZED;
import static java.net.HttpURLConnection.HTTP_UNAVAILABLE;
import static java.net.HttpURLConnection.HTTP_UNSUPPORTED_TYPE;

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
import javax.ws.rs.core.Context;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.Response;
import javax.ws.rs.core.UriInfo;

import org.codehaus.enunciate.jaxrs.ResponseCode;
import org.codehaus.enunciate.jaxrs.ResponseHeader;
import org.codehaus.enunciate.jaxrs.ResponseHeaders;
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
 * This class provides Northbound REST APIs to handle VTN
 * (virtual tenant network).
 */
@Path("/{containerName}/vtns")
public class VTenantNorthbound extends VTNNorthBoundBase {
    /**
     * Return information about all the VTNs present in the specified
     * container.
     *
     * @param containerName  The name of the container.
     * @return  <strong>vtns</strong> element contains information about all
     *          the VTNs present in the container specified by the requested
     *          URI.
     */
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(VTenantList.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "The specified container does not exist."),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
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
     * Return information about the specified VTN inside the specified
     * container.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the VTN.
     * @return  <strong>vtn</strong> element contains information about the
     *          VTN specified by the requested URI.
     */
    @Path("{tenantName}")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(VTenant.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified container does not exist.</li>" +
                      "<li>The specified VTN does not exist.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
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
     * Create a new VTN inside the specified container.
     *
     * @param uriInfo        Requested URI information.
     * @param containerName  The name of the container.
     * @param tenantName
     *   The name of the VTN to be created.
     *   <ul>
     *     <li>
     *       The length of the name must be greater than <strong>0</strong>
     *       and less than <strong>32</strong>.
     *     </li>
     *     <li>
     *       The name must consist of US-ASCII alphabets, numbers, and
     *       underscore ({@code '_'}).
     *     </li>
     *     <li>
     *       The name must start with an US-ASCII alphabet or number.
     *     </li>
     *   </ul>
     * @param tconf
     *   <strong>vtnconf</strong> element specifies the VTN configuration
     *   information.
     *   <ul>
     *     <li>
     *       The description of the VTN is not registered if
     *       <strong>description</strong> attribute is omitted.
     *     </li>
     *     <li>
     *       Idle timeout of flow entry will be treated as <strong>300</strong>
     *       if <strong>idleTimeout</strong> attribute is omitted.
     *     </li>
     *     <li>
     *       Hard timeout of flow entry will be treated as <strong>0</strong>
     *       if <strong>hardTimeout</strong> attribute is omitted.
     *     </li>
     *   </ul>
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{tenantName}")
    @POST
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(TypeHint.NO_CONTENT.class)
    @ResponseHeaders({
        @ResponseHeader(name = "Location",
                        description = "URI corresponding to the newly " +
                        "created VTN, which is the same URI specified in " +
                        "request.")})
    @StatusCodes({
        @ResponseCode(code = HTTP_CREATED,
                      condition = "VTN was created successfully."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "Incorrect XML or JSON data is specified " +
                      "in Request body."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "The specified container does not exist."),
        @ResponseCode(code = HTTP_NOT_ACCEPTABLE,
                      condition = "\"default\" is specified to " +
                      "<u>{containerName}</u> and a container other than " +
                      "the default container is present."),
        @ResponseCode(code = HTTP_CONFLICT,
                      condition = "The VTN specified by the requested URI " +
                      "already exists."),
        @ResponseCode(code = HTTP_UNSUPPORTED_TYPE,
                      condition = "<ul>" +
                      "<li>Unsupported data type is specified in " +
                      "<strong>Content-Type</strong> header.</li>" +
                      "<li>Incorrect VTN name is specified to " +
                      "<u>{tenantName}</u>.</li>" +
                      "<li>Incorrect value is configured in " +
                      "<strong>vtnconf</strong> element for " +
                      "<strong>idleTimeout</strong> or " +
                      "<strong>hardTimeout</strong> attribute.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public Response addTenant(
            @Context UriInfo uriInfo,
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName,
            @TypeHint(VTenantConfig.class) VTenantConfig tconf) {
        checkPrivilege(containerName, Privilege.WRITE);

        IVTNManager mgr = getVTNManager(containerName);
        VTenantPath path = new VTenantPath(tenantName);
        Status status = mgr.addTenant(path, tconf);
        if (status.isSuccess()) {
            return Response.created(uriInfo.getAbsolutePath()).build();
        }

        throw getException(status);
    }

    /**
     * Modify configuration of existing VTN in the specified container.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the VTN.
     * @param all
     *   A boolean value to determine the treatment of attributes omitted in
     *   <strong>vtnconf</strong> element.
     *   <ul>
     *     <li>
     *       If <strong>true</strong> is specified, all the attributes related
     *       to VTN are modified.
     *       <ul>
     *         <li>
     *           The description of the VTN will be deleted if
     *           <strong>description</strong> attribute is omitted.
     *         </li>
     *         <li>
     *           Idle timeout of flow entry will be treated as
     *           <strong>300</strong> if <strong>idleTimeout</strong>
     *           attribute is omitted.
     *         </li>
     *         <li>
     *           Hard timeout of flow entry will be treated as
     *           <strong>0</strong> if <strong>hardTimeout</strong>
     *           attribute is omitted.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       If <strong>false</strong> is specified, omitted attributes are not
     *       modified.
     *     </li>
     *   </ul>
     * @param tconf
     *   <strong>vtnconf</strong> element specifies the VTN configuration
     *   information to be applied.
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{tenantName}")
    @PUT
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(TypeHint.NO_CONTENT.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "Incorrect XML or JSON data is specified " +
                      "in Request body."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified container does not exist.</li>" +
                      "<li>The specified VTN does not exist.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_NOT_ACCEPTABLE,
                      condition = "\"default\" is specified to " +
                      "<u>{containerName}</u> and a container other than " +
                      "the default container is present."),
        @ResponseCode(code = HTTP_UNSUPPORTED_TYPE,
                      condition = "<ul>" +
                      "<li>Unsupported data type is specified in " +
                      "<strong>Content-Type</strong> header.</li>" +
                      "<li>Incorrect value is configured in " +
                      "<strong>vtnconf</strong> element for " +
                      "<strong>idleTimeout</strong> or " +
                      "<strong>hardTimeout</strong> attribute.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public Response modifyTenant(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName,
            @DefaultValue("false") @QueryParam("all") boolean all,
            @TypeHint(VTenantConfig.class) VTenantConfig tconf) {
        checkPrivilege(containerName, Privilege.WRITE);

        IVTNManager mgr = getVTNManager(containerName);
        VTenantPath path = new VTenantPath(tenantName);
        Status status = mgr.modifyTenant(path, tconf, all);
        if (status.isSuccess()) {
            return Response.ok().build();
        }

        throw getException(status);
    }

    /**
     * Delete the specified VTN.
     *
     * <p>
     *   All the virtual networking node in the specified VTN, such as vBridge,
     *   will also be deleted.
     * </p>
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the VTN.
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{tenantName}")
    @DELETE
    @TypeHint(TypeHint.NO_CONTENT.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified container does not exist.</li>" +
                      "<li>The specified VTN does not exist.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_NOT_ACCEPTABLE,
                      condition = "\"default\" is specified to " +
                      "<u>{containerName}</u> and a container other than " +
                      "the default container is present."),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
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
     * Remove all flow entries in the specified VTN.
     *
     * <p>
     *   This API is provided only for debugging purpose, and is available
     *   only if the system property <strong>vtn.debug</strong> is defined as
     *   <strong>true</strong>.
     * </p>
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the VTN.
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{tenantName}/flows")
    @DELETE
    @TypeHint(TypeHint.NO_CONTENT.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified container does not exist.</li>" +
                      "<li>The specified VTN does not exist.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_NOT_ACCEPTABLE,
                      condition = "\"default\" is specified to " +
                      "<u>{containerName}</u> and a container other than " +
                      "the default container is present."),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
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
