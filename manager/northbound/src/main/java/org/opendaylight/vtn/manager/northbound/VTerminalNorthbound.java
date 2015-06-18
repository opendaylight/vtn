/*
 * Copyright (c) 2014-2015 NEC Corporation
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
import static java.net.HttpURLConnection.HTTP_NOT_FOUND;
import static java.net.HttpURLConnection.HTTP_OK;
import static java.net.HttpURLConnection.HTTP_UNAUTHORIZED;
import static java.net.HttpURLConnection.HTTP_UNAVAILABLE;
import static java.net.HttpURLConnection.HTTP_UNSUPPORTED_TYPE;

import java.util.List;

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

import org.opendaylight.vtn.manager.IVTNManager;
import org.opendaylight.vtn.manager.VTerminal;
import org.opendaylight.vtn.manager.VTerminalConfig;
import org.opendaylight.vtn.manager.VTerminalPath;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenantPath;

import org.opendaylight.controller.sal.authorization.Privilege;
import org.opendaylight.controller.sal.utils.Status;

/**
 * This class provides Northbound REST APIs to handle vTerminal.
 *
 * <p>
 *   <strong>vTerminal</strong> is isolated input and output terminal inside
 *   VTN. vTerminal can have only one virtual interface, and it can map a
 *   physical switch port by port mapping.
 * </p>
 * <p>
 *   vTerminal is always used in conjunction with redirection by flow filter.
 * </p>
 * <ul>
 *   <li>
 *     An incoming packet from the virtual interface inside the vTerminal is
 *     always dropped unless it is redirected to other virtual node by
 *     flow filter.
 *   </li>
 *   <li>
 *     A packet is never routed to the virtual interface inside the vTerminal
 *     unless flow filter redirects the packet to that interface.
 *   </li>
 * </ul>
 *
 * @since  Helium
 */
@Path("/default/vtns/{tenantName}/vterminals")
public class VTerminalNorthbound extends VTNNorthBoundBase {
    /**
     * Return information about all the vTerminals present in the specified
     * VTN.
     *
     * @param tenantName  The name of the VTN.
     * @return  <strong>vterminals</strong> element contains information about
     *          all the vTerminals in the VTN specified by the requested URI.
     */
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(VTerminalList.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "The specified VTN does not exist."),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public VTerminalList getTerminals(
            @PathParam("tenantName") String tenantName) {
        checkPrivilege(Privilege.READ);

        IVTNManager mgr = getVTNManager();
        VTenantPath path = new VTenantPath(tenantName);
        try {
            List<VTerminal> list = mgr.getTerminals(path);
            return new VTerminalList(list);
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Return information about the specified vTerminal inside the specified
     * VTN.
     *
     * @param tenantName  The name of the VTN.
     * @param termName    The name of the vTerminal.
     * @return  <strong>vterminal</strong> element contains information about
     *          the vTerminal specified by the requested URI.
     */
    @Path("{termName}")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(VTerminal.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified VTN does not exist.</li>" +
                      "<li>The specified vTerminal does not exist.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public VTerminal getTerminal(
            @PathParam("tenantName") String tenantName,
            @PathParam("termName") String termName) {
        checkPrivilege(Privilege.READ);

        IVTNManager mgr = getVTNManager();
        VTerminalPath path = new VTerminalPath(tenantName, termName);
        try {
            return mgr.getTerminal(path);
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Create a new vTerminal inside the specified VTN.
     *
     * @param uriInfo     Requested URI information.
     * @param tenantName  The name of the VTN.
     * @param termName
     *   The name of the vTerminal to be created.
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
     * @param vtconf
     *   <strong>vterminalconf</strong> element specifies the vTerminal
     *   configuration information.
     *   <ul>
     *     <li>
     *       The description of the vTerminal is not registered if
     *       <strong>description</strong> attribute is omitted.
     *     </li>
     *   </ul>
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{termName}")
    @POST
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(TypeHint.NO_CONTENT.class)
    @ResponseHeaders({
        @ResponseHeader(name = "Location",
                        description = "URI corresponding to the newly " +
                        "created vTerminal, which is the same URI specified " +
                        "in request.")})
    @StatusCodes({
        @ResponseCode(code = HTTP_CREATED,
                      condition = "vTerminal was created successfully."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "<ul>" +
                      "<li>Incorrect XML or JSON data is specified " +
                      "in Request body.</li>" +
                      "<li>Incorrect vTerminal name is specified to " +
                      "<u>{termName}</u>.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "The specified VTN does not exist."),
        @ResponseCode(code = HTTP_CONFLICT,
                      condition = "The vTerminal specified by the request " +
                      "URI already exists."),
        @ResponseCode(code = HTTP_UNSUPPORTED_TYPE,
                      condition = "Unsupported data type is specified in " +
                      "<strong>Content-Type</strong> header."),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public Response addTerminal(
            @Context UriInfo uriInfo,
            @PathParam("tenantName") String tenantName,
            @PathParam("termName") String termName,
            @TypeHint(VTerminalConfig.class) VTerminalConfig vtconf) {
        checkPrivilege(Privilege.WRITE);

        IVTNManager mgr = getVTNManager();
        VTerminalPath path = new VTerminalPath(tenantName, termName);
        Status status = mgr.addTerminal(path, vtconf);
        if (status.isSuccess()) {
            return Response.created(uriInfo.getAbsolutePath()).build();
        }

        throw getException(status);
    }

    /**
     * Modify configuration of existing vTerminal in the specified VTN.
     *
     * @param tenantName  The name of the VTN.
     * @param termName    The name of the vTerminal.
     * @param all
     *   A boolean value to determine the treatment of attributes omitted in
     *   <strong>vterminalconf</strong> element.
     *   <ul>
     *     <li>
     *       If <strong>true</strong> is specified, all the attributes related
     *       to vTerminal are modified.
     *       <ul>
     *         <li>
     *           The description of the vTerminal will be deleted if
     *           <strong>description</strong> attribute is omitted.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       If <strong>false</strong> is specified, omitted attributes are not
     *       modified.
     *     </li>
     *   </ul>
     * @param vtconf
     *   <strong>vterminalconf</strong> element specifies the vTerminal
     *   configuration information to be applied.
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{termName}")
    @PUT
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(TypeHint.NO_CONTENT.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "<ul>" +
                      "<li>Incorrect XML or JSON data is specified " +
                      "in Request body.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified VTN does not exist.</li>" +
                      "<li>The specified vTerminal does not exist.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_UNSUPPORTED_TYPE,
                      condition = "Unsupported data type is specified in " +
                      "<strong>Content-Type</strong> header."),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public Response modifyTerminal(
            @PathParam("tenantName") String tenantName,
            @PathParam("termName") String termName,
            @DefaultValue("false") @QueryParam("all") boolean all,
            @TypeHint(VTerminalConfig.class) VTerminalConfig vtconf) {
        checkPrivilege(Privilege.WRITE);

        IVTNManager mgr = getVTNManager();
        VTerminalPath path = new VTerminalPath(tenantName, termName);
        Status status = mgr.modifyTerminal(path, vtconf, all);
        if (status.isSuccess()) {
            return Response.ok().build();
        }

        throw getException(status);
    }

    /**
     * Delete the specified vTerminal.
     *
     * <p>
     *   A virtual interface inside the specified vTerminal will also be
     *   deleted.
     * </p>
     *
     * @param tenantName  The name of the VTN.
     * @param termName    The name of the vTerminal.
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{termName}")
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
                      "<li>The specified VTN does not exist.</li>" +
                      "<li>The specified vTerminal does not exist.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public Response deleteTerminal(
            @PathParam("tenantName") String tenantName,
            @PathParam("termName") String termName) {
        checkPrivilege(Privilege.WRITE);

        IVTNManager mgr = getVTNManager();
        VTerminalPath path = new VTerminalPath(tenantName, termName);
        Status status = mgr.removeTerminal(path);
        if (status.isSuccess()) {
            return Response.ok().build();
        }

        throw getException(status);
    }
}
