/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import java.util.List;

import static java.net.HttpURLConnection.HTTP_BAD_REQUEST;
import static java.net.HttpURLConnection.HTTP_CONFLICT;
import static java.net.HttpURLConnection.HTTP_CREATED;
import static java.net.HttpURLConnection.HTTP_INTERNAL_ERROR;
import static java.net.HttpURLConnection.HTTP_NOT_FOUND;
import static java.net.HttpURLConnection.HTTP_NO_CONTENT;
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

import org.opendaylight.vtn.manager.IVTNManager;
import org.opendaylight.vtn.manager.PortMap;
import org.opendaylight.vtn.manager.PortMapConfig;
import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTerminalIfPath;
import org.opendaylight.vtn.manager.VTerminalPath;

import org.opendaylight.controller.northbound.commons.exception.
    BadRequestException;
import org.opendaylight.controller.sal.authorization.Privilege;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.utils.Status;

/**
 * This class provides Northbound REST APIs to handle virtual interface
 * in the vTerminal.
 *
 * @since  Helium
 */
@Path("/default/vtns/{tenantName}/vterminals/{termName}/interfaces")
public class VTerminalInterfaceNorthbound extends VTNNorthBoundBase {
    /**
     * Return information about all the virtual interfaces present in the
     * specified vTerminal.
     *
     * @param tenantName  The name of the VTN.
     * @param termName    The name of the vTerminal.
     * @return  <strong>interfaces</strong> element contains information about
     *          all the virtual interfaces in the vTerminal specified by the
     *          requested URI.
     */
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(VInterfaceList.class)
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
    public VInterfaceList getInterfaces(
            @PathParam("tenantName") String tenantName,
            @PathParam("termName") String termName) {
        checkPrivilege(Privilege.READ);

        IVTNManager mgr = getVTNManager();
        VTerminalPath path = new VTerminalPath(tenantName, termName);
        try {
            List<VInterface> list = mgr.getInterfaces(path);
            return new VInterfaceList(list);
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Return information about the specified virtual interface inside the
     * specified vTerminal.
     *
     * @param tenantName  The name of the VTN.
     * @param termName    The name of the vTerminal.
     * @param ifName      The name of the vTerminal interface.
     * @return  <strong>interface</strong> element contains information about
     *          the vTerminal interface specified by the requested URI.
     */
    @Path("{ifName}")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(VInterface.class)
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
                      "<li>The specified vTerminal interface does not " +
                      "exist.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public VInterface getInterface(
            @PathParam("tenantName") String tenantName,
            @PathParam("termName") String termName,
            @PathParam("ifName") String ifName) {
        checkPrivilege(Privilege.READ);

        IVTNManager mgr = getVTNManager();
        VTerminalIfPath path =
            new VTerminalIfPath(tenantName, termName, ifName);
        try {
            return mgr.getInterface(path);
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Create a new virtual interface inside the specified vTerminal.
     *
     * <p>
     *   Note that vTerminal can contain only one virtual interface.
     * </p>
     *
     * @param uriInfo     Requested URI information.
     * @param tenantName  The name of the VTN.
     * @param termName    The name of the vTerminal.
     * @param ifName
     *   The name of the vTerminal interface to be created.
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
     * @param iconf
     *   <strong>interfaceconf</strong> element specifies the vTerminal
     *   interface configuration information.
     *   <ul>
     *     <li>
     *       The description of the vTerminal interface is not registered if
     *       <strong>description</strong> attribute is omitted.
     *     </li>
     *     <li>
     *       Enable/disable configuration is treated as <strong>true</strong>
     *       if <strong>enabled</strong> attribute is omitted.
     *     </li>
     *   </ul>
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{ifName}")
    @POST
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(TypeHint.NO_CONTENT.class)
    @ResponseHeaders({
        @ResponseHeader(name = "Location",
                        description = "URI corresponding to the newly " +
                        "created vTerminal interface, which is the same URI " +
                        "specified in request.")})
    @StatusCodes({
        @ResponseCode(code = HTTP_CREATED,
                      condition = "vTerminal interface was created " +
                      "successfully."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "<ul>" +
                      "<li>Incorrect XML or JSON data is specified " +
                      "in Request body.</li>" +
                      "<li>Incorrect interface name is specified to " +
                      "<u>{ifName}</u>.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified VTN does not exist.</li>" +
                      "<li>The specified vTerminal does not exist.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_CONFLICT,
                      condition = "The vTerminal specified by the requested " +
                      "URI already contains a virtual interface."),
        @ResponseCode(code = HTTP_UNSUPPORTED_TYPE,
                      condition = "Unsupported data type is specified in " +
                      "<strong>Content-Type</strong> header."),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public Response addInterface(
            @Context UriInfo uriInfo,
            @PathParam("tenantName") String tenantName,
            @PathParam("termName") String termName,
            @PathParam("ifName") String ifName,
            @TypeHint(VInterfaceConfig.class) VInterfaceConfig iconf) {
        checkPrivilege(Privilege.WRITE);

        IVTNManager mgr = getVTNManager();
        VTerminalIfPath path =
            new VTerminalIfPath(tenantName, termName, ifName);
        Status status = mgr.addInterface(path, iconf);
        if (status.isSuccess()) {
            return Response.created(uriInfo.getAbsolutePath()).build();
        }

        throw getException(status);
    }

    /**
     * Modify configuration of existing virtual interface in the specified
     * vTerminal.
     *
     * @param tenantName  The name of the VTN.
     * @param termName    The name of the vTerminal.
     * @param ifName      The name of the vTerminal interface.
     * @param all
     *   A boolean value to determine the treatment of attributes omitted in
     *   <strong>interfaceconf</strong> element.
     *   <ul>
     *     <li>
     *       If <strong>true</strong> is specified, all the attributes related
     *       to vTerminal interface are modified.
     *       <ul>
     *         <li>
     *           The description of the vTerminal interface will be deleted if
     *           <strong>description</strong> attribute is omitted.
     *         </li>
     *         <li>
     *           Enable/disable configuration will be changed to
     *           <strong>true</strong> if <strong>enabled</strong> attribute
     *           is omitted.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       If <strong>false</strong> is specified, omitted attributes are not
     *       modified.
     *     </li>
     *   </ul>
     * @param iconf
     *   <strong>interfaceconf</strong> element specifies the vTerminal
     *   interface configuration information to be applied.
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{ifName}")
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
                      "<li>The specified VTN does not exist.</li>" +
                      "<li>The specified vTerminal does not exist.</li>" +
                      "<li>The specified vTerminal interface does not " +
                      "exist.</li>" +
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
    public Response modifyInterface(
            @PathParam("tenantName") String tenantName,
            @PathParam("termName") String termName,
            @PathParam("ifName") String ifName,
            @DefaultValue("false") @QueryParam("all") boolean all,
            @TypeHint(VInterfaceConfig.class) VInterfaceConfig iconf) {
        checkPrivilege(Privilege.WRITE);

        IVTNManager mgr = getVTNManager();
        VTerminalIfPath path =
            new VTerminalIfPath(tenantName, termName, ifName);
        Status status = mgr.modifyInterface(path, iconf, all);
        if (status.isSuccess()) {
            return Response.ok().build();
        }

        throw getException(status);
    }

    /**
     * Delete the specified vTerminal interface.
     *
     * @param tenantName  The name of the VTN.
     * @param termName    The name of the vTerminal.
     * @param ifName      The name of the vTerminal interface.
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{ifName}")
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
                      "<li>The specified vTerminal interface does not " +
                      "exist.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public Response deleteInterface(
            @PathParam("tenantName") String tenantName,
            @PathParam("termName") String termName,
            @PathParam("ifName") String ifName) {
        checkPrivilege(Privilege.WRITE);

        IVTNManager mgr = getVTNManager();
        VTerminalIfPath path =
            new VTerminalIfPath(tenantName, termName, ifName);
        Status status = mgr.removeInterface(path);
        if (status.isSuccess()) {
            return Response.ok().build();
        }

        throw getException(status);
    }

    /**
     * Return information about the port mapping configured in the specified
     * virtual interface in the vTerminal.
     *
     * @param tenantName  The name of the VTN.
     * @param termName    The name of the vTerminal.
     * @param ifName      The name of the vTerminal interface.
     * @return  <strong>portmap</strong> element contains information about
     *          the port mapping configured in the vTerminal interface
     *          specified by the requested URI.
     */
    @Path("{ifName}/portmap")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(PortMapInfo.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Port mapping is configured in the " +
                      "specified vTerminal interface."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "Port mapping is not configured in the " +
                      "specified vTerminal interface."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified VTN does not exist.</li>" +
                      "<li>The specified vTerminal does not exist.</li>" +
                      "<li>The specified vTerminal interface does not " +
                      "exist.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public PortMapInfo getPortMap(
            @PathParam("tenantName") String tenantName,
            @PathParam("termName") String termName,
            @PathParam("ifName") String ifName) {
        checkPrivilege(Privilege.READ);

        IVTNManager mgr = getVTNManager();
        VTerminalIfPath path =
            new VTerminalIfPath(tenantName, termName, ifName);
        try {
            PortMap pmap = mgr.getPortMap(path);
            if (pmap == null) {
                return null;
            }

            PortMapConfig pmconf = pmap.getConfig();
            NodeConnector nc = pmap.getNodeConnector();
            return new PortMapInfo(pmconf, nc);
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Configure port mapping in the specified virtual interface in the
     * vTerminal.
     *
     * <p>
     *   On successful completion, then ethernet frame that flows through
     *   the port of the physical switch specified by
     *   <strong>portmapconf</strong> element will get mapped to the vTerminal
     *   interface specified by the requested URI.
     * </p>
     *
     * <ul>
     *   <li>
     *     If port mapping is already configured in the specified vTerminal
     *     interface, the specified settings are applied after old
     *     configuration is deleted.
     *   </li>
     *   <li>
     *     This API does nothing and returns the status code
     *     <strong>HTTP_OK</strong> (200) immediately if port mapping with
     *     the same configuration information as <strong>portmapconf</strong>
     *     element is already configured in the specified vTerminal interface.
     *   </li>
     * </ul>
     *
     * @param tenantName  The name of the VTN.
     * @param termName    The name of the vTerminal.
     * @param ifName      The name of the vTerminal interface.
     * @param pmconf
     *   <strong>portmapconf</strong> specifies the port mapping configuration
     *   information.
     *   <ul>
     *     <li>
     *       <strong>node</strong> element, which specifies the physical switch
     *       to be mapped, must be configured.
     *       <ul>
     *         <li>
     *           Currently, only OpenFlow switch can be specified to
     *           <strong>node</strong> element.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       <strong>port</strong> element, which specifies the physical port
     *       of the switch specified by <strong>node</strong> element, must be
     *       configured.
     *       <ul>
     *         <li>
     *           Currently, it is possible to configure only the physical ports
     *           of OpenFlow switches.
     *         </li>
     *         <li>
     *           Port mapping configuration will succeed even if the specified
     *           physical port of the switch does not exist. Port mapping will
     *           come into effect whenever, at a later point in time, the
     *           specified physical port is found.
     *           <ul>
     *             <li>
     *               However, when mapping is actually done with the specified
     *               physical port, the port mapping will not be established if
     *               that physical port and VLAN ID are mapped to another
     *               virtual interface.
     *             </li>
     *           </ul>
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       <strong>vlan</strong> attribute specifies the VLAN network to be
     *       mapped by VLAN ID.
     *       <ul>
     *         <li>
     *           If a value between <strong>1</strong> or more and
     *           <strong>4095</strong> or less is configured, then the ethernet
     *           frames that have this VLAN ID configured will get mapped to
     *           the vTerminal interface.
     *         </li>
     *         <li>
     *           If <strong>0</strong> is configured, or <strong>vlan</strong>
     *           attribute is omitted, untagged ethernet frames will get mapped
     *           to the vTerminal interface.
     *         </li>
     *       </ul>
     *     </li>
     *   </ul>
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{ifName}/portmap")
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
                      "<li>Incorrect value is configured in " +
                      "<strong>portmapconf</strong> element.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified VTN does not exist.</li>" +
                      "<li>The specified vTerminal does not exist.</li>" +
                      "<li>The specified vTerminal interface does not " +
                      "exist.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_CONFLICT,
                      condition = "The specified physical switch port is " +
                      "present, and the specified VLAN ID is mapped to " +
                      "another virtual interface by port mapping."),
        @ResponseCode(code = HTTP_UNSUPPORTED_TYPE,
                      condition = "Unsupported data type is specified in " +
                      "<strong>Content-Type</strong> header."),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public Response setPortMap(
            @PathParam("tenantName") String tenantName,
            @PathParam("termName") String termName,
            @PathParam("ifName") String ifName,
            @TypeHint(PortMapConfig.class) PortMapConfig pmconf) {
        checkPrivilege(Privilege.WRITE);

        IVTNManager mgr = getVTNManager();
        VTerminalIfPath path =
            new VTerminalIfPath(tenantName, termName, ifName);
        if (pmconf == null) {
            // This should never happen.
            throw new BadRequestException("Port map configuration is null");
        }

        Status status = mgr.setPortMap(path, pmconf);
        if (status.isSuccess()) {
            return Response.ok().build();
        }

        throw getException(status);
    }

    /**
     * Delete port mapping configuration configured in the specified vTerminal
     * interface.
     *
     * <p>
     *   This API does nothing and returns the status code
     *   <strong>HTTP_OK</strong> (200) immediately if no port mapping is
     *   configured in the specified vTerminal interface.
     * </p>
     *
     * @param tenantName  The name of the VTN.
     * @param termName    The name of the vTerminal.
     * @param ifName      The name of the vTerminal interface.
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{ifName}/portmap")
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
                      "<li>The specified vTerminal interface does not " +
                      "exist.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public Response deletePortMap(
            @PathParam("tenantName") String tenantName,
            @PathParam("termName") String termName,
            @PathParam("ifName") String ifName) {
        checkPrivilege(Privilege.WRITE);

        IVTNManager mgr = getVTNManager();
        VTerminalIfPath path =
            new VTerminalIfPath(tenantName, termName, ifName);
        Status status = mgr.setPortMap(path, null);
        if (status.isSuccess()) {
            return Response.ok().build();
        }

        throw getException(status);
    }
}
