/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
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

import java.net.URI;
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
import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.VBridge;
import org.opendaylight.vtn.manager.VBridgeConfig;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.VlanMapConfig;

import org.opendaylight.controller.northbound.commons.exception.
    ResourceNotFoundException;
import org.opendaylight.controller.sal.authorization.Privilege;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.Status;

/**
 * This class provides Northbound REST APIs to handle vBridge
 * (virtual L2 bridge).
 */
@Path("/default/vtns/{tenantName}/vbridges")
public class VBridgeNorthbound extends VTNNorthBoundBase {
    /**
     * Return information about all the vBridges present in the specified VTN.
     *
     * @param tenantName  The name of the VTN.
     * @return  <strong>vbridges</strong> element contains information about
     *          all the vBridges in the VTN specified by the requested URI.
     */
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(VBridgeList.class)
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
    public VBridgeList getBridges(@PathParam("tenantName") String tenantName) {
        checkPrivilege(Privilege.READ);

        IVTNManager mgr = getVTNManager();
        VTenantPath path = new VTenantPath(tenantName);
        try {
            List<VBridge> list = mgr.getBridges(path);
            return new VBridgeList(list);
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Return information about the specified vBridge inside the specified
     * VTN.
     *
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @return  <strong>vbridge</strong> element contains information about
     *          the vBridge specified by the requested URI.
     */
    @Path("{bridgeName}")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(VBridge.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified VTN does not exist.</li>" +
                      "<li>The specified vBridge does not exist.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public VBridge getBridge(
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName) {
        checkPrivilege(Privilege.READ);

        IVTNManager mgr = getVTNManager();
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        try {
            return mgr.getBridge(path);
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Create a new vBridge inside the specified VTN.
     *
     * @param uriInfo     Requested URI information.
     * @param tenantName  The name of the VTN.
     * @param bridgeName
     *   The name of the vBridge to be created.
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
     * @param bconf
     *   <strong>vbridgeconf</strong> element specifies the vBridge
     *   configuration information.
     *   <ul>
     *     <li>
     *       The description of the vBridge is not registered if
     *       <strong>description</strong> attribute is omitted.
     *     </li>
     *     <li>
     *       Age interval of MAC address table is treated as
     *       <strong>600</strong> if <strong>ageInterval</strong> attribute
     *       is omitted.
     *     </li>
     *   </ul>
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{bridgeName}")
    @POST
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(TypeHint.NO_CONTENT.class)
    @ResponseHeaders({
        @ResponseHeader(name = "Location",
                        description = "URI corresponding to the newly " +
                        "created vBridge, which is the same URI specified " +
                        "in request.")})
    @StatusCodes({
        @ResponseCode(code = HTTP_CREATED,
                      condition = "vBridge was created successfully."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "<ul>" +
                      "<li>Incorrect XML or JSON data is specified " +
                      "in Request body.</li>" +
                      "<li>Incorrect vBridge name is specified to " +
                      "<u>{bridgeName}</u>.</li>" +
                      "<li>Incorrect value is configured in " +
                      "<strong>vbridgeconf</strong> element for " +
                      "<strong>ageInterval</strong> attribute.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "The specified VTN does not exist."),
        @ResponseCode(code = HTTP_CONFLICT,
                      condition = "The vBridge specified by the request URI " +
                      "already exists."),
        @ResponseCode(code = HTTP_UNSUPPORTED_TYPE,
                      condition = "Unsupported data type is specified in " +
                      "<strong>Content-Type</strong> header."),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public Response addBridge(
            @Context UriInfo uriInfo,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @TypeHint(VBridgeConfig.class) VBridgeConfig bconf) {
        checkPrivilege(Privilege.WRITE);

        IVTNManager mgr = getVTNManager();
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        Status status = mgr.addBridge(path, bconf);
        if (status.isSuccess()) {
            return Response.created(uriInfo.getAbsolutePath()).build();
        }

        throw getException(status);
    }

    /**
     * Modify configuration of existing vBridge in the specified VTN.
     *
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @param all
     *   A boolean value to determine the treatment of attributes omitted in
     *   <strong>vbridgeconf</strong> element.
     *   <ul>
     *     <li>
     *       If <strong>true</strong> is specified, all the attributes related
     *       to vBridge are modified.
     *       <ul>
     *         <li>
     *           The description of the vBridge will be deleted if
     *           <strong>description</strong> attribute is omitted.
     *         </li>
     *         <li>
     *           Age interval of MAC address table is treated as
     *           <strong>600</strong> if <strong>ageInterval</strong>
     *           attribute is omitted.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       If <strong>false</strong> is specified, omitted attributes are not
     *       modified.
     *     </li>
     *   </ul>
     * @param bconf
     *   <strong>vbridgeconf</strong> element specifies the vBridge
     *   configuration information to be applied.
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{bridgeName}")
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
                      "<strong>vbridgeconf</strong> element for " +
                      "<strong>ageInterval</strong> attribute.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified VTN does not exist.</li>" +
                      "<li>The specified vBridge does not exist.</li>" +
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
    public Response modifyBridge(
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @DefaultValue("false") @QueryParam("all") boolean all,
            @TypeHint(VBridgeConfig.class) VBridgeConfig bconf) {
        checkPrivilege(Privilege.WRITE);

        IVTNManager mgr = getVTNManager();
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        Status status = mgr.modifyBridge(path, bconf, all);
        if (status.isSuccess()) {
            return Response.ok().build();
        }

        throw getException(status);
    }

    /**
     * Delete the specified vBridge.
     *
     * <p>
     *   All the virtual interfaces inside the specified vBridge will also
     *   be deleted.
     * </p>
     *
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{bridgeName}")
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
                      "<li>The specified vBridge does not exist.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public Response deleteBridge(
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName) {
        checkPrivilege(Privilege.WRITE);

        IVTNManager mgr = getVTNManager();
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        Status status = mgr.removeBridge(path);
        if (status.isSuccess()) {
            return Response.ok().build();
        }

        throw getException(status);
    }

    /**
     * Return information about all the VLAN mappings configured in the
     * specified vBridge.
     *
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @return  <strong>vlanmaps</strong> element contains information about
     *          all the VLAN mappings configured in the vBridge specified by
     *          the requested URI.
     */
    @Path("{bridgeName}/vlanmaps")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(VlanMapList.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified VTN does not exist.</li>" +
                      "<li>The specified vBridge does not exist.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public VlanMapList getVlanMaps(
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName) {
        checkPrivilege(Privilege.READ);

        IVTNManager mgr = getVTNManager();
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        try {
            List<VlanMap> list = mgr.getVlanMaps(path);
            return new VlanMapList(list);
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Configure VLAN mapping in the specified vBridge.
     *
     * @param uriInfo     Requested URI information.
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @param vlconf
     *   <strong>vlanmapconf</strong> specifies the VLAN mapping configuration
     *   information.
     *   <ul>
     *     <li>
     *       It is possible to specify physical switch to be mapped to the
     *       vBridge by <strong>node</strong> element.
     *       <ul>
     *         <li>
     *           Currently, only OpenFlow switch can be specified to
     *           <strong>node</strong> element.
     *         </li>
     *         <li>
     *           VLAN mapping configuration will succeed even if the physical
     *           switch specified by <strong>node</strong> element does not
     *           exist. VLAN mapping will come into effect whenever,
     *           at a later point in time, the specified physical switch is
     *           found.
     *         </li>
     *         <li>
     *           All the physical switches managed by the OpenDaylight
     *           controller will be mapped if <strong>node</strong> element
     *           is omitted.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       The VLAN network is mapped to the interface according to the
     *       VLAN ID configured in <strong>vlan</strong> attribute.
     *       <ul>
     *         <li>
     *           If a VLAN ID between <strong>1</strong> or more and
     *           <strong>4095</strong> or less is configured, then the
     *           ethernet frames that have this VLAN ID configured will get
     *           mapped to the vBridge.
     *         </li>
     *         <li>
     *           If <strong>0</strong> is configured, or <strong>vlan</strong>
     *           attribute is omitted, untagged ethernet frames will get
     *           mapped to the vBridge.
     *         </li>
     *       </ul>
     *     </li>
     *   </ul>
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{bridgeName}/vlanmaps")
    @POST
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(TypeHint.NO_CONTENT.class)
    @ResponseHeaders({
        @ResponseHeader(name = "Location",
                        description = "URI corresponding to the newly " +
                        "configured VLAN mapping.<ul>" +
                        "<li>The last path component of the URI is the " +
                        "identifier assigned to the VLAN mapping.</li>" +
                        "</ul>")})
    @StatusCodes({
        @ResponseCode(code = HTTP_CREATED,
                      condition = "VLAN mapping was configured successfully."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "<ul>" +
                      "<li>Incorrect XML or JSON data is specified " +
                      "in Request body.</li>" +
                      "<li>Incorrect value is configured in " +
                      "<strong>vlanmapconf</strong> element.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified VTN does not exist.</li>" +
                      "<li>The specified vBridge does not exist.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_CONFLICT,
                      condition = "The specified VLAN is already mapped " +
                      "to the specified vBridge or another vBridge."),
        @ResponseCode(code = HTTP_UNSUPPORTED_TYPE,
                      condition = "Unsupported data type is specified in " +
                      "<strong>Content-Type</strong> header."),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public Response addVlanMap(
            @Context UriInfo uriInfo,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @TypeHint(VlanMapConfig.class) VlanMapConfig vlconf) {
        checkPrivilege(Privilege.WRITE);

        IVTNManager mgr = getVTNManager();
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        try {
            VlanMap vmap = mgr.addVlanMap(path, vlconf);

            // Return CREATED with Location header.
            String id = vmap.getId();
            URI vmapUri = uriInfo.getAbsolutePathBuilder().path(id).build();
            return Response.created(vmapUri).build();
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Delete the specified VLAN mapping from the vBridge.
     *
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @param mapId       The identifier of the VLAN mapping to be deleted.
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{bridgeName}/vlanmaps/{mapId}")
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
                      "<li>The specified vBridge does not exist.</li>" +
                      "<li>The specified VLAN mapping is not configured in " +
                      "the specified vBridge.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public Response deleteVlanMap(
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @PathParam("mapId") String mapId) {
        checkPrivilege(Privilege.WRITE);

        IVTNManager mgr = getVTNManager();
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        Status status = mgr.removeVlanMap(path, mapId);
        if (status.isSuccess()) {
            return Response.ok().build();
        }

        throw getException(status);
    }

    /**
     * Return information about the VLAN mapping specified by the given ID.
     *
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @param mapId       The identifier of the VLAN mapping assigned by
     *                    the VTN Manager.
     * @return  <strong>vlanmap</strong> element contains information about
     *          the VLAN mapping specified by the requested URI.
     */
    @Path("{bridgeName}/vlanmaps/{mapId}")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(VlanMap.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified VTN does not exist.</li>" +
                      "<li>The specified vBridge does not exist.</li>" +
                      "<li>The specified VLAN mapping is not configured in " +
                      "the specified vBridge.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public VlanMap getVlanMap(
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @PathParam("mapId") String mapId) {
        checkPrivilege(Privilege.READ);

        IVTNManager mgr = getVTNManager();
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        try {
            return mgr.getVlanMap(path, mapId);
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Search for a VLAN mapping with the specified configuration information
     * in the specified vBridge.
     *
     * <p>
     *   If any VLAN mapping with the configuration information that exactly
     *   meets the specified conditions is present in the specified vBridge,
     *   information about that VLAN mapping is returned.
     * </p>
     *
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @param vlan
     *   VLAN ID mapped by VLAN mapping.
     *   <ul>
     *     <li>
     *       If omitted or <strong>0</strong> is specified, this API searches
     *       for a VLAN mapping which maps untagged ethernet frames.
     *     </li>
     *     <li>
     *       Note that this API never checks whether the specified VLAN ID is
     *       valid as VLAN ID. If an invalid VLAN ID, such as a negative value,
     *       is specified, the search always results in failure.
     *     </li>
     *   </ul>
     * @param node
     *   A string representation of a {@link Node} instance that corresponds
     *   to switch mapped by VLAN mapping.
     *   <ul>
     *     <li>
     *       For example, <strong>OF|00:11:22:33:44:55:66:77</strong> means
     *       an OpenFlow switch with the DPID 00:11:22:33:44:55:66:77.
     *     </li>
     *     <li>
     *       If omitted, this method searches for a VLAN mapping which maps
     *       all the switches managed by the OpenDaylight controller.
     *     </li>
     *   </ul>
     * @return <strong>vlanmap</strong> element contains information about
     *         the VLAN mapping specified by the requested URI and query
     *         parameters.
     */
    @Path("{bridgeName}/vlanmapsearch/byconf")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(VlanMap.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "Value specified in query parameter has " +
                      "an invalid format."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified VTN does not exist.</li>" +
                      "<li>The specified vBridge does not exist.</li>" +
                      "<li>There is no VLAN mapping, in the specified " +
                      "vBridge, which meets the conditions specified by " +
                      "query parameter.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public VlanMap getVlanMap(
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @QueryParam("vlan") String vlan,
            @QueryParam("node") String node) {
        checkPrivilege(Privilege.READ);

        IVTNManager mgr = getVTNManager();
        VBridgePath path = new VBridgePath(tenantName, bridgeName);

        short vid = parseVlanId(vlan);
        Node nd;
        if (node == null) {
            nd = null;
        } else {
            nd = parseNode(node);
        }

        VlanMapConfig vlconf = new VlanMapConfig(nd, vid);
        try {
            return mgr.getVlanMap(path, vlconf);
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Return information about all the MAC addresses learned in the specified
     * vBridge.
     *
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @return  <strong>macentries</strong> element contains information about
     *          all the MAC addresses learned in the vBridge specified by the
     *          requested URI.
     */
    @Path("{bridgeName}/mac")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(MacEntryList.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified VTN does not exist.</li>" +
                      "<li>The specified vBridge does not exist.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public MacEntryList getMacEntries(
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName) {
        checkPrivilege(Privilege.READ);

        IVTNManager mgr = getVTNManager();
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        try {
            List<MacAddressEntry> list = mgr.getMacEntries(path);
            return new MacEntryList(list);
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Delete entire MAC address information learned in the MAC address table
     * of the specified vBridge.
     *
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{bridgeName}/mac")
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
                      "<li>The specified vBridge does not exist.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public Response flushMacEntries(
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName) {
        checkPrivilege(Privilege.WRITE);

        IVTNManager mgr = getVTNManager();
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        Status status = mgr.flushMacEntries(path);
        if (status.isSuccess()) {
            return Response.ok().build();
        }

        throw getException(status);
    }

    /**
     * Search the MAC address table of the vBridge for the specified MAC
     * address.
     *
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @param macAddr
     *   A string representation of MAC address to be found.
     *   <ul>
     *     <li>
     *       A MAC address should be specified in hexadecimal notation with
     *       {@code ':'} inserted between octets.
     *       (e.g. {@code 11:22:33:aa:bb:cc})
     *     </li>
     *   </ul>
     * @return  <strong>macentry</strong> element contains information about
     *          the MAC address specified by the requested URI.
     */
    @Path("{bridgeName}/mac/{macAddr}")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(MacEntry.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "The MAC address is specified in " +
                      "unexpected format."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified VTN does not exist.</li>" +
                      "<li>The specified vBridge does not exist.</li>" +
                      "<li>The specified MAC address is not learned in the " +
                      "MAC address table of the specified vBridge.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public MacEntry getMacEntry(
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @PathParam("macAddr") String macAddr) {
        checkPrivilege(Privilege.READ);

        // Parse the given MAC address as Ethernet address.
        EthernetAddress dladdr = parseEthernetAddress(macAddr);
        IVTNManager mgr = getVTNManager();
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        try {
            MacAddressEntry entry = mgr.getMacEntry(path, dladdr);
            if (entry == null) {
                throw new ResourceNotFoundException("MAC address not found");
            }
            return new MacEntry(entry);
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Delete a specific MAC address information learned in the MAC address
     * table of the specified vBridge.
     *
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @param macAddr
     *   A string representation of MAC address to be found.
     *   <ul>
     *     <li>
     *       A MAC address should be specified in hexadecimal notation with
     *       {@code ':'} inserted between octets.
     *       (e.g. {@code 11:22:33:aa:bb:cc})
     *     </li>
     *   </ul>
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{bridgeName}/mac/{macAddr}")
    @DELETE
    @TypeHint(TypeHint.NO_CONTENT.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "The MAC address is specified in " +
                      "unexpected format."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified VTN does not exist.</li>" +
                      "<li>The specified vBridge does not exist.</li>" +
                      "<li>The specified MAC address is not learned in the " +
                      "MAC address table of the specified vBridge.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public Response removeMacEntry(
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @PathParam("macAddr") String macAddr) {
        checkPrivilege(Privilege.WRITE);

        // Parse the given MAC address as Ethernet address.
        EthernetAddress dladdr = parseEthernetAddress(macAddr);
        IVTNManager mgr = getVTNManager();
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        try {
            if (mgr.removeMacEntry(path, dladdr) == null) {
                throw new ResourceNotFoundException("MAC address not found");
            }
            return Response.ok().build();
        } catch (VTNException e) {
            throw getException(e);
        }
    }
}
