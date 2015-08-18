/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
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
import static java.net.HttpURLConnection.HTTP_NO_CONTENT;
import static java.net.HttpURLConnection.HTTP_OK;
import static java.net.HttpURLConnection.HTTP_UNAUTHORIZED;
import static java.net.HttpURLConnection.HTTP_UNAVAILABLE;
import static java.net.HttpURLConnection.HTTP_UNSUPPORTED_TYPE;

import java.util.List;
import java.util.Set;
import java.util.HashSet;

import javax.ws.rs.Consumes;
import javax.ws.rs.DELETE;
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

import org.opendaylight.vtn.manager.DataLinkHost;
import org.opendaylight.vtn.manager.EthernetHost;
import org.opendaylight.vtn.manager.IVTNManager;
import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.MacMap;
import org.opendaylight.vtn.manager.MacMapConfig;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.controller.northbound.commons.exception.
    BadRequestException;
import org.opendaylight.controller.sal.authorization.Privilege;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnAclType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;

/**
 * This class provides Northbound REST APIs to handle MAC mapping in the
 * vBridge (virtual L2 bridge).
 *
 * @since  Helium
 */
@Path("/default/vtns/{tenantName}/vbridges/{bridgeName}/macmap")
public class MacMapNorthbound extends VTNNorthBoundBase {
    /**
     * A pseudo MAC address which indicates an undefined value.
     */
    private static final String  MAC_ADDR_ANY = "ANY";

    /**
     * Return information about MAC mapping configured in the specified
     * vBridge.
     *
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @return  <strong>macmap</strong> element contains information about
     *          MAC mapping specified by the requested URI.
     */
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(MacMapInfo.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "MAC mapping is not configured in the " +
                      "specified vBridge."),
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
    public MacMapInfo getMacMap(
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName) {
        checkPrivilege(Privilege.READ);

        IVTNManager mgr = getVTNManager();
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        try {
            MacMap mcmap = mgr.getMacMap(path);
            return (mcmap == null) ? null : new MacMapInfo(mcmap);
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Change whole MAC mapping configuration in the specified vBridge.
     *
     * <p>
     *   This API changes both the Map Allow and Map Deny list of MAC mapping.
     * </p>
     *
     * @param uriInfo     Requested URI information.
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @param mci
     *   <strong>macmapconf</strong> specifies the MAC mapping configuration
     *   information.
     *   <ul>
     *     <li>
     *       <strong>allow</strong> element specifies the host information
     *       to be configured in Map Allow list.
     *       <ul>
     *         <li>
     *           Out of the host information configured in Map Allow list,
     *           all host information which are not specified in
     *           <strong>allow</strong> element will be removed.
     *         </li>
     *         <li>
     *           The same MAC addresses on different VLAN cannot be mapped to
     *           the same vBridge. Thus multiple <strong>machost</strong>
     *           element with the same MAC address cannot be configured
     *           in <strong>allow</strong> element.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       <strong>deny</strong> element specifies the host information
     *       to be configured in Map Deny list.
     *       <ul>
     *         <li>
     *           Out of the host information configured in Map Deny list,
     *           all host information which are not specified in
     *           <strong>deny</strong> element will be removed.
     *         </li>
     *         <li>
     *           A valid MAC address must be configured in all
     *           <strong>machost</strong> elements specified in
     *           <strong>deny</strong> element.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       <strong>macmapconf</strong> must contain at least one
     *       <strong>machost</strong> element in <strong>allow</strong> or
     *       <strong>deny</strong> element.
     *     </li>
     *     <li>
     *       The following MAC addresses cannot be configured in
     *       <strong>machost</strong> element.
     *       <ul>
     *         <li>Zero <code>00:00:00:00:00:00</code></li>
     *         <li>Broadcast address <code>ff:ff:ff:ff:ff:ff</code></li>
     *         <li>Multicast address</li>
     *       </ul>
     *     </li>
     *   </ul>
     * @return  Response as dictated by the HTTP Response Status code.
     */
    @PUT
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(TypeHint.NO_CONTENT.class)
    @ResponseHeaders({
        @ResponseHeader(name = "Location",
                        description = "URI corresponding to the newly " +
                        "configured MAC mapping, which is the same URI " +
                        "specified in request. This header is set only " +
                        "if CREATED(201) is returned as response code.")})
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Existing MAC mapping configuration was " +
                      "changed successfully."),
        @ResponseCode(code = HTTP_CREATED,
                      condition = "MAC mapping was newly configured " +
                      "successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "MAC mapping configuration was not " +
                      "changed."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "<ul>" +
                      "<li>Incorrect XML or JSON data is specified " +
                      "in Request body.</li>" +
                      "<li>In <strong>macmapconf</strong> element, " +
                      "<strong>machost</strong> element with invalid " +
                      "MAC address or VLAN ID is configured.</li>" +
                      "<li><strong>macmapconf</strong> element does not " +
                      "contain <strong>machost</strong> element.</li>" +
                      "<li>Multiple <strong>machost</strong> element with " +
                      "the same MAC address is configured in " +
                      "<strong>allow</strong> element of " +
                      "<strong>macmapconf</strong>.</li>" +
                      "<li><strong>machost</strong> element without the " +
                      "MAC address is configured in <strong>deny</strong> " +
                      "element of <strong>macmapconf</strong>.</li>" +
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
                      condition = "Host information configured in " +
                      "Map Access list of MAC mapping, configured on " +
                      "another vBridge, is configured in " +
                      "<strong>allow</strong> element of " +
                      "<strong>macmapconf</strong>."),
        @ResponseCode(code = HTTP_UNSUPPORTED_TYPE,
                      condition = "Unsupported data type is specified in " +
                      "<strong>Content-Type</strong> header."),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public Response setMacMap(
            @Context UriInfo uriInfo,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @TypeHint(MacMapConfigInfo.class) MacMapConfigInfo mci) {
        MacMapConfig mcconf = toMacMapConfig(mci);
        return setMacMap(uriInfo, tenantName, bridgeName,
                         VtnUpdateOperationType.SET, mcconf);
    }

    /**
     * Modify the MAC mapping configuration of the specified vBridge.
     *
     * <p>
     *   This API performs one of the following actions.
     * </p>
     * <ul>
     *   <li>
     *     Add the specified host information to Map Allow list and
     *     Map Deny list of MAC mapping.
     *   </li>
     *   <li>
     *     Remove the specified host information from Map Allow list and
     *     Map Deny list of MAC mapping.
     *   </li>
     * </ul>
     *
     * @param uriInfo     Requested URI information.
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @param mci
     *   <strong>macmapconf</strong> specifies the MAC mapping configuration
     *   information to be added or removed.
     *   <ul>
     *     <li>
     *       If <strong>add</strong> is specified to
     *       <span style="text-decoration: underline;">action</span> parameter,
     *       host information configured in <strong>macmapconf</strong> will
     *       be added to MAC mapping configuration.
     *       <ul>
     *         <li>
     *           Host information in <strong>allow</strong> element will be
     *           added to Map Allow list.
     *           <ul>
     *             <li>
     *               The same MAC addresses on different VLAN cannot be mapped
     *               to the same vBridge. Thus multiple
     *               <strong>machost</strong> element with the same
     *               MAC address cannot be configured in <strong>allow</strong>
     *               element.
     *             </li>
     *           </ul>
     *         </li>
     *         <li>
     *           Host information in <strong>deny</strong> element will be
     *           added to Map Deny list.
     *           <ul>
     *             <li>
     *               A valid MAC address must be configured in all
     *               <strong>machost</strong> elements specified in
     *               <strong>deny</strong> element.
     *             </li>
     *           </ul>
     *         </li>
     *         <li>
     *           The following MAC addresses cannot be configured in
     *           <strong>machost</strong> element.
     *           <ul>
     *             <li>Zero <code>00:00:00:00:00:00</code></li>
     *             <li>Broadcast address <code>ff:ff:ff:ff:ff:ff</code></li>
     *             <li>Multicast address</li>
     *           </ul>
     *         </li>
     *         <li>
     *           It will be ignored if one tries to add host information
     *           present in existing access control list.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       If <strong>remove</strong> is specified to
     *       <span style="text-decoration: underline;">action</span> parameter,
     *       host information configured in <strong>macmapconf</strong> will
     *       be removed from MAC mapping configuration.
     *       <ul>
     *         <li>
     *           Host information in <strong>allow</strong> element will be
     *           removed from Map Allow list.
     *         </li>
     *         <li>
     *           Host information in <strong>deny</strong> element will be
     *           removed from Map Deny list.
     *         </li>
     *         <li>
     *           It will be ignored if one tries to remove host information
     *           which is not present in existing access control list.
     *         </li>
     *         <li>
     *           MAC mapping is removed if both Map Allow list and Map Deny
     *           list become empty.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       Map Allow list is not modified if <strong>allow</strong> element
     *       is empty.
     *     </li>
     *     <li>
     *       Map Deny list is not modified if <strong>deny</strong> element
     *       is empty.
     *     </li>
     *   </ul>
     * @param action
     *   Specify a string which determines how to change the MAC mapping
     *   configuration.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">add
     *     <dd style="margin-left: 1.5em">
     *       Add the configuration specified by <strong>macmapconf</strong>
     *       element to the current MAC mapping configuration.
     *
     *     <dt style="font-weight: bold;">remove
     *     <dd style="margin-left: 1.5em">
     *       Remove the configuration specified by <strong>macmapconf</strong>
     *       element from the current MAC mapping configuration.
     *   </dl>
     *   <p>
     *     If omitted, it will be treated as if <strong>add</strong> is
     *     specified.
     *   </p>
     * @return  Response as dictated by the HTTP Response Status code.
     */
    @POST
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(TypeHint.NO_CONTENT.class)
    @ResponseHeaders({
        @ResponseHeader(name = "Location",
                        description = "URI corresponding to the newly " +
                        "configured MAC mapping, which is the same URI " +
                        "specified in request. This header is set only " +
                        "if CREATED(201) is returned as response code.")})
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Existing MAC mapping configuration was " +
                      "changed or deleted successfully."),
        @ResponseCode(code = HTTP_CREATED,
                      condition = "MAC mapping was newly configured " +
                      "successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "MAC mapping configuration was not " +
                      "changed."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "<ul>" +
                      "<li>Incorrect XML or JSON data is specified " +
                      "in Request body.</li>" +
                      "<li>In <strong>macmapconf</strong> element, " +
                      "<strong>machost</strong> element with invalid " +
                      "MAC address or VLAN ID is configured.</li>" +
                      "<li><strong>macmapconf</strong> element does not " +
                      "contain <strong>machost</strong> element.</li>" +
                      "<li>\"add\" is passed to <u>action</u> " +
                      "and one of the following conditions is met." +
                      "<ul>" +
                      "<li>Multiple <strong>machost</strong> element with " +
                      "the same MAC address is configured in " +
                      "<strong>allow</strong> element of " +
                      "<strong>macmapconf</strong>.</li>" +
                      "<li><strong>machost</strong> element without the " +
                      "MAC address is configured in <strong>deny</strong> " +
                      "element of <strong>macmapconf</strong>.</li>" +
                      "</ul>" +
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
                      condition = "\"add\" is passed to " +
                      "<u>action</u>, and one of the following conditions " +
                      "is met." +
                      "<ul>" +
                      "<li>Host information configured in Map Allow list of " +
                      "MAC mapping, configured on another vBridge, is " +
                      "configured in <strong>allow</strong> element of" +
                      "<strong>macmapconf</strong>.</li>" +
                      "<li>Host information with the same MAC address and " +
                      "different VLAN ID when compared to " +
                      "<strong>allow</strong> element in " +
                      "<strong>macmapconf</strong> is already configured in " +
                      "Map Allow list of MAC mapping.</li>" +
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
    public Response modifyMacMap(
            @Context UriInfo uriInfo,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @QueryParam("action") String action,
            @TypeHint(MacMapConfigInfo.class) MacMapConfigInfo mci) {
        MacMapConfig mcconf = toMacMapConfig(mci);
        VtnUpdateOperationType op = toVtnUpdateOperationType(action);
        return setMacMap(uriInfo, tenantName, bridgeName, op, mcconf);
    }

    /**
     * Delete the MAC mapping configured in the specified vBridge.
     *
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @return  Response as dictated by the HTTP Response Status code.
     */
    @DELETE
    @TypeHint(TypeHint.NO_CONTENT.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "MAC mapping was deleted successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "MAC mapping is not configured in the " +
                      "specified vBridge."),
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
    public Response deleteMacMap(
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName) {
        return setMacMap(null, tenantName, bridgeName,
                         VtnUpdateOperationType.SET, null);
    }

    /**
     * Return the contents of Map Allow list of MAC mapping configured in the
     * specified vBridge.
     *
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @return  <strong>machosts</strong> element contains host information
     *          configured in Map Allow list of MAC mapping.
     */
    @Path("allow")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(MacHostSet.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "MAC mapping is not configured in the " +
                      "specified vBridge."),
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
    public MacHostSet getMacMapAllowed(
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName) {
        return getMacMapConfig(tenantName, bridgeName, VtnAclType.ALLOW);
    }

    /**
     * Change whole contents of Map Allow list of MAC mapping configured in
     * the specified vBridge.
     *
     * <p>
     *   This API will modify the Map Allow list of MAC mapping to the
     *   contents specified by request body.
     * </p>
     *
     * @param uriInfo     Requested URI information.
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @param mhset
     *   <strong>machosts</strong> specifies the host information to be
     *   configured in Map Allow list.
     *   <ul>
     *     <li>
     *       <strong>machosts</strong> element must contain at least one
     *       <strong>machost</strong> element.
     *     </li>
     *     <li>
     *       The same MAC addresses on different VLAN cannot be mapped to
     *       the same vBridge. Thus multiple <strong>machost</strong>
     *       element with the same MAC address cannot be configured
     *       in <strong>allow</strong> element.
     *     </li>
     *     <li>
     *       The following MAC addresses cannot be configured in
     *       <strong>machost</strong> element.
     *       <ul>
     *         <li>Zero <code>00:00:00:00:00:00</code></li>
     *         <li>Broadcast address <code>ff:ff:ff:ff:ff:ff</code></li>
     *         <li>Multicast address</li>
     *       </ul>
     *     </li>
     *   </ul>
     * @return  Response as dictated by the HTTP Response Status code.
     */
    @Path("allow")
    @PUT
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(TypeHint.NO_CONTENT.class)
    @ResponseHeaders({
        @ResponseHeader(name = "Location",
                        description = "URI corresponding to the newly " +
                        "configured Map Allow list of MAC mapping, " +
                        "which is the same URI specified in request. " +
                        "This header is set only if CREATED(201) is " +
                        "returned as response code.")})
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Existing MAC mapping configuration was " +
                      "changed successfully."),
        @ResponseCode(code = HTTP_CREATED,
                      condition = "MAC mapping was newly configured " +
                      "successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "MAC mapping configuration was not " +
                      "changed."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "<ul>" +
                      "<li>Incorrect XML or JSON data is specified " +
                      "in Request body.</li>" +
                      "<li><strong>machost</strong> element with invalid " +
                      "MAC address or VLAN ID is configured in " +
                      "<strong>machosts</strong>.</li>" +
                      "<li><strong>machosts</strong> element does not " +
                      "contain <strong>machost</strong> element.</li>" +
                      "<li>Multiple <strong>machost</strong> element with " +
                      "the same MAC address is configured in " +
                      "<strong>machosts</strong>.</li>" +
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
                      condition = "Host information configured in Map Allow " +
                      "list of MAC mapping, configured on another vBridge, " +
                      "is configured in <strong>machosts</strong>."),
        @ResponseCode(code = HTTP_UNSUPPORTED_TYPE,
                      condition = "Unsupported data type is specified in " +
                      "<strong>Content-Type</strong> header."),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public Response setMacMapAllowed(
            @Context UriInfo uriInfo,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @TypeHint(MacHostSet.class) MacHostSet mhset) {
        Set<DataLinkHost> dlhosts = toValidDataLinkHostSet(mhset);
        return setMacMap(uriInfo, tenantName, bridgeName,
                         VtnUpdateOperationType.SET, VtnAclType.ALLOW,
                         dlhosts);
    }

    /**
     * Modify the Map Allow list contents in MAC mapping configured in the
     * specified vBridge.
     *
     * <p>
     *   This API performs one of the following actions.
     * </p>
     * <ul>
     *   <li>
     *     Add the specified host information to Map Allow list of MAC mapping.
     *   </li>
     *   <li>
     *     Remove the specified host information from Map Allow list of
     *     MAC mapping.
     *   </li>
     * </ul>
     *
     * @param uriInfo     Requested URI information.
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @param mhset
     *   <strong>machosts</strong> specifies host information to be added
     *   or removed.
     *   <ul>
     *     <li>
     *       If <strong>add</strong> is specified to
     *       <span style="text-decoration: underline;">action</span> parameter,
     *       host information configured in <strong>machosts</strong> will be
     *       added to Map Allow list of MAC mapping.
     *       <ul>
     *         <li>
     *           The same MAC addresses on different VLAN cannot be mapped to
     *           the same vBridge. Thus multiple <strong>machost</strong>
     *           element with the same MAC address cannot be configured
     *           in <strong>machosts</strong>.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       If <strong>remove</strong> is specified to
     *       <span style="text-decoration: underline;">action</span> parameter,
     *       host information configured in <strong>machosts</strong> will be
     *       removed from Map Allow list of MAC mapping.
     *       <ul>
     *         <li>
     *           It will be ignored if one tries to remove host information
     *           which is not present in existing Map Allow list.
     *         </li>
     *         <li>
     *           MAC mapping is removed if both Map Allow list and Map Deny
     *           list become empty.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       Map Allow list is not modified if <strong>machosts</strong>
     *       is empty.
     *     </li>
     *     <li>
     *       The following MAC addresses cannot be configured in
     *       <strong>machost</strong> element.
     *       <ul>
     *         <li>Zero <code>00:00:00:00:00:00</code></li>
     *         <li>Broadcast address <code>ff:ff:ff:ff:ff:ff</code></li>
     *         <li>Multicast address</li>
     *       </ul>
     *     </li>
     *   </ul>
     * @param action
     *   Specify a string which determines how to change Map Allow list in
     *   MAC mapping.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">add
     *     <dd style="margin-left: 1.5em">
     *       Add host information configured in <strong>machosts</strong>
     *       to Map Allow list.
     *
     *     <dt style="font-weight: bold;">remove
     *     <dd style="margin-left: 1.5em">
     *       Remove host information configured in <strong>machosts</strong>
     *       from Map Allow list.
     *   </dl>
     *   <p>
     *     If omitted, it will be treated as if <strong>add</strong> is
     *     specified.
     *   </p>
     * @return  Response as dictated by the HTTP Response Status code.
     */
    @Path("allow")
    @POST
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(TypeHint.NO_CONTENT.class)
    @ResponseHeaders({
        @ResponseHeader(name = "Location",
                        description = "URI corresponding to the newly " +
                        "configured Map Allow list of MAC mapping, " +
                        "which is the same URI specified in request. " +
                        "This header is set only if CREATED(201) is " +
                        "returned as response code.")})
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Existing MAC mapping configuration was " +
                      "changed or deleted successfully."),
        @ResponseCode(code = HTTP_CREATED,
                      condition = "MAC mapping was newly configured " +
                      "successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "MAC mapping configuration was not " +
                      "changed."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "<ul>" +
                      "<li>Incorrect XML or JSON data is specified " +
                      "in Request body.</li>" +
                      "<li><strong>machost</strong> element with invalid " +
                      "MAC address or VLAN ID is configured in " +
                      "<strong>machosts</strong>.</li>" +
                      "<li><strong>machosts</strong> element does not " +
                      "contain <strong>machost</strong> element.</li>" +
                      "<li>\"add\" is passed to <u>action</u>, " +
                      "and multiple <strong>machost</strong> element with " +
                      "the same MAC address is configured in " +
                      "<strong>machosts</strong>.</li>" +
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
                      condition = "\"add\" is passed to " +
                      "<u>action</u>, and one of the following conditions " +
                      "is met." +
                      "<ul>" +
                      "<li>Host information configured in Map Allow list of " +
                      "MAC mapping, configured on another vBridge, is " +
                      "configured in <strong>machosts</strong></li>" +
                      "<li>Host information with the same MAC address and " +
                      "different VLAN ID when compared to " +
                      "<strong>machosts</strong> is already configured in " +
                      "Map Allow list of MAC mapping.</li>" +
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
    public Response modifyMacMapAllowed(
            @Context UriInfo uriInfo,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @QueryParam("action") String action,
            @TypeHint(MacHostSet.class) MacHostSet mhset) {
        Set<DataLinkHost> dlhosts = toValidDataLinkHostSet(mhset);
        VtnUpdateOperationType op = toVtnUpdateOperationType(action);
        return setMacMap(uriInfo, tenantName, bridgeName, op,
                         VtnAclType.ALLOW, dlhosts);
    }

    /**
     * Remove all contents in Map Allow list of MAC mapping configured in
     * the specified vBridge.
     *
     * <p>
     *   MAC mapping is removed if both Map Allow list and Map Deny list
     *   become empty.
     * </p>
     *
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @return  Response as dictated by the HTTP Response Status code.
     */
    @Path("allow")
    @DELETE
    @TypeHint(TypeHint.NO_CONTENT.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "MAC mapping configuration was not " +
                      "changed."),
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
    public Response deleteMacMapAllowed(
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName) {
        return setMacMap(null, tenantName, bridgeName,
                         VtnUpdateOperationType.SET, VtnAclType.ALLOW, null);
    }

    /**
     * Determine whether the specified host information is present in the
     * Map Allow list of MAC mapping configured in the specified vBridge.
     *
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @param macAddr
     *   A string representation of MAC address to be tested.
     *   <ul>
     *     <li>
     *       A MAC address should be specified in hexadecimal notation with
     *       {@code ':'} inserted between octets.
     *       (e.g. {@code 11:22:33:aa:bb:cc})
     *     </li>
     *     <li>
     *       <strong>ANY</strong> means a host information without
     *       specifying MAC address.
     *     </li>
     *   </ul>
     * @param vlan
     *   A string representation of VLAN ID to which the host belongs.
     *   <ul>
     *      <li>
     *        Allowed range of value is between <strong>0</strong> or more and
     *        <strong>4095</strong> or less.
     *      </li>
     *      <li>
     *        <strong>0</strong> implies untagged network.
     *      </li>
     *   </ul>
     * @return  <strong>machost</strong> element contains MAC address and
     *          VLAN ID specified by the requested URI.
     */
    @Path("allow/{macAddr}/{vlan}")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(MacHost.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "The specified host is configured in the " +
                      "Map Allow list of MAC mapping."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "<ul>" +
                      "<li>The specified host is not configured in the " +
                      "Map Allow list of MAC mapping.</li>" +
                      "<li>MAC mapping is not configured in the specified " +
                      "vBridge.<li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "<ul>" +
                      "<li>Invalid MAC address is passed to " +
                      "<u>{macAddr}</u>.</li>" +
                      "<li>Invalid VLAN ID is passed to <u>{vlan}</u>.</li>" +
                      "</ul>"),
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
    public MacHost getMacMapAllowedHost(
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @PathParam("macAddr") String macAddr,
            @PathParam("vlan") String vlan) {
        return getMacMapConfig(tenantName, bridgeName, VtnAclType.ALLOW,
                               macAddr, vlan);
    }

    /**
     * Add the host information specified by the requested URI to the
     * Map Allow list of MAC mapping configured in the specified vBridge.
     *
     * <p>
     *   Map Allow list is not modified if the host information specified by
     *   <span style="text-decoration: underline;">{macAddr}</span> and
     *   <span style="text-decoration: underline;">{vlan}</span> parameters
     *   is already configured in the Map Allow list.
     * </p>
     *
     * @param uriInfo     Requested URI information.
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @param macAddr
     *   A string representation of MAC address to be added.
     *   <ul>
     *     <li>
     *       A MAC address should be specified in hexadecimal notation with
     *       {@code ':'} inserted between octets.
     *       (e.g. {@code 11:22:33:aa:bb:cc})
     *     </li>
     *     <li>
     *       <strong>ANY</strong> means that a host information without
     *       specifying MAC address.
     *     </li>
     *     <li>
     *       The following MAC addresses cannot be specified.
     *       <ul>
     *         <li>Zero <code>00:00:00:00:00:00</code></li>
     *         <li>Broadcast address <code>ff:ff:ff:ff:ff:ff</code></li>
     *         <li>Multicast address</li>
     *       </ul>
     *     </li>
     *   </ul>
     * @param vlan
     *   A string representation of VLAN ID to which the host belongs.
     *   <ul>
     *      <li>
     *        Allowed range of value is between <strong>0</strong> or more and
     *        <strong>4095</strong> or less.
     *      </li>
     *      <li>
     *        <strong>0</strong> implies untagged network.
     *      </li>
     *   </ul>
     * @return  Response as dictated by the HTTP Response Status code.
     */
    @Path("allow/{macAddr}/{vlan}")
    @PUT
    @TypeHint(TypeHint.NO_CONTENT.class)
    @ResponseHeaders({
        @ResponseHeader(name = "Location",
                        description = "URI corresponding to the newly " +
                        "configured host information in the Map Allow list " +
                        "of MAC mapping, which is the same URI specified in " +
                        "request. This header is set only if CREATED(201) " +
                        "is returned as response code.")})
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "The specified host information was added " +
                      "into the Map Allow list of existing MAC mapping."),
        @ResponseCode(code = HTTP_CREATED,
                      condition = "MAC mapping was newly configured " +
                      "successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "MAC mapping configuration was not " +
                      "changed."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "<ul>" +
                      "<li>Invalid MAC address is passed to " +
                      "<u>{macAddr}</u>.</li>" +
                      "<li>Invalid VLAN ID is passed to <u>{vlan}</u>.</li>" +
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
                      condition = "<ul>" +
                      "<li>Host information configured in Map Allow list of " +
                      "MAC mapping, configured on another vBridge, " +
                      "is specified to <u>{macAddr}</u> and " +
                      "<u>{vlan}</u>.</li>" +
                      "<li>Host information withthe same MAC address and " +
                      "different VLAN ID when compared to host information " +
                      "specified in <u>{macAddr}</u> and <u>{vlan}</u> is " +
                      "already configured in Map Allow list of " +
                      "MAC mapping.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public Response setMacMapAllowedHost(
            @Context UriInfo uriInfo,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @PathParam("macAddr") String macAddr,
            @PathParam("vlan") String vlan) {
        return setMacMap(uriInfo, tenantName, bridgeName,
                         VtnUpdateOperationType.ADD, VtnAclType.ALLOW, macAddr,
                         vlan);
    }

    /**
     * Delete the host information specified by the requested URI from the
     * Map Allow list of MAC mapping configured in the specified vBridge.
     *
     * <ul>
     *   <li>
     *     Map Allow list is not modified if the host information specified by
     *     <span style="text-decoration: underline;">{macAddr}</span> and
     *     <span style="text-decoration: underline;">{vlan}</span> parameters
     *     is not configured in the Map Allow list.
     *   </li>
     *   <li>
     *     MAC mapping is removed if both Map Allow list and Map Deny list
     *     become empty.
     *   </li>
     * </ul>
     *
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @param macAddr
     *   A string representation of MAC address to be removed.
     *   <ul>
     *     <li>
     *       A MAC address should be specified in hexadecimal notation with
     *       {@code ':'} inserted between octets.
     *       (e.g. {@code 11:22:33:aa:bb:cc})
     *     </li>
     *     <li>
     *       <strong>ANY</strong> means that a host information without
     *       specifying MAC address.
     *     </li>
     *   </ul>
     * @param vlan
     *   A string representation of VLAN ID to which the host belongs.
     *   <ul>
     *      <li>
     *        Allowed range of value is between <strong>0</strong> or more and
     *        <strong>4095</strong> or less.
     *      </li>
     *      <li>
     *        <strong>0</strong> implies untagged network.
     *      </li>
     *   </ul>
     * @return  Response as dictated by the HTTP Response Status code.
     */
    @Path("allow/{macAddr}/{vlan}")
    @DELETE
    @TypeHint(TypeHint.NO_CONTENT.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "The specified host was removed from the " +
                      "Map Allow list of MAC mapping."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "<ul>" +
                      "<li>The specified host is not configured in the " +
                      "Map Allow list of MAC mapping.</li>" +
                      "<li>MAC mapping is not configured in the specified " +
                      "vBridge.<li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "<ul>" +
                      "<li>Invalid MAC address is passed to " +
                      "<u>{macAddr}</u>.</li>" +
                      "<li>Invalid VLAN ID is passed to <u>{vlan}</u>.</li>" +
                      "</ul>"),
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
    public Response deleteMacMapAllowedHost(
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @PathParam("macAddr") String macAddr,
            @PathParam("vlan") String vlan) {
        return setMacMap(null, tenantName, bridgeName,
                         VtnUpdateOperationType.REMOVE, VtnAclType.ALLOW,
                         macAddr, vlan);
    }

    /**
     * Return the contents of Map Deny list of MAC mapping configured in the
     * specified vBridge.
     *
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @return  <strong>machosts</strong> element contains host information
     *          configured in Map Deny list of MAC mapping.
     */
    @Path("deny")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(MacHostSet.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "MAC mapping is not configured in the " +
                      "specified vBridge."),
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
    public MacHostSet getMacMapDenied(
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName) {
        return getMacMapConfig(tenantName, bridgeName, VtnAclType.DENY);
    }

    /**
     * Change whole contents of Map Deny list of MAC mapping configured in
     * the specified vBridge.
     *
     * @param uriInfo     Requested URI information.
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @param mhset
     *   <strong>machosts</strong> specifies the host information to be
     *   configured in Map Deny list.
     *   <ul>
     *     <li>
     *       <strong>machosts</strong> element must contain at least one
     *       <strong>machost</strong> element.
     *     </li>
     *     <li>
     *       A valid MAC address must be configured in all
     *       <strong>machost</strong> elements specified in
     *       <strong>machosts</strong>.
     *     </li>
     *     <li>
     *       The following MAC addresses cannot be configured in
     *       <strong>machost</strong> element.
     *       <ul>
     *         <li>Zero <code>00:00:00:00:00:00</code></li>
     *         <li>Broadcast address <code>ff:ff:ff:ff:ff:ff</code></li>
     *         <li>Multicast address</li>
     *       </ul>
     *     </li>
     *   </ul>
     * @return  Response as dictated by the HTTP Response Status code.
     */
    @Path("deny")
    @PUT
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(TypeHint.NO_CONTENT.class)
    @ResponseHeaders({
        @ResponseHeader(name = "Location",
                        description = "URI corresponding to the newly " +
                        "configured Map Deny list of MAC mapping, " +
                        "which is the same URI specified in request. " +
                        "This header is set only if CREATED(201) is " +
                        "returned as response code.")})
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Existing MAC mapping configuration was " +
                      "changed successfully."),
        @ResponseCode(code = HTTP_CREATED,
                      condition = "MAC mapping was newly configured " +
                      "successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "MAC mapping configuration was not " +
                      "changed."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "<ul>" +
                      "<li>Incorrect XML or JSON data is specified " +
                      "in Request body.</li>" +
                      "<li><strong>machost</strong> element with invalid " +
                      "MAC address or VLAN ID is configured in " +
                      "<strong>machosts</strong>.</li>" +
                      "<li><strong>machosts</strong> element does not " +
                      "contain <strong>machost</strong> element.</li>" +
                      "<li><strong>machost</strong> element is configured " +
                      "without MAC address in " +
                      "<strong>machosts</strong>.</li>" +
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
    public Response setMacMapDenied(
            @Context UriInfo uriInfo,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @TypeHint(MacHostSet.class) MacHostSet mhset) {
        Set<DataLinkHost> dlhosts = toValidDataLinkHostSet(mhset);
        return setMacMap(uriInfo, tenantName, bridgeName,
                         VtnUpdateOperationType.SET, VtnAclType.DENY, dlhosts);
    }

    /**
     * Modify the Map Allow list contents in MAC mapping configured in the
     * specified vBridge.
     *
     * <p>
     *   This API performs one of the following actions.
     * </p>
     * <ul>
     *   <li>
     *     Add the specified host information to Map Deny list of MAC mapping.
     *   </li>
     *   <li>
     *     Remove the specified host information from Map Deny list of
     *     MAC mapping.
     *   </li>
     * </ul>
     *
     * @param uriInfo     Requested URI information.
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @param mhset
     *   <strong>machosts</strong> specifies host information to be added
     *   or removed.
     *   <ul>
     *     <li>
     *       If <strong>add</strong> is specified to
     *       <span style="text-decoration: underline;">action</span> parameter,
     *       host information configured in <strong>machosts</strong> will be
     *       added to Map Deny list of MAC mapping.
     *       <ul>
     *         <li>
     *           A valid MAC address must be configured in all
     *           <strong>machost</strong> elements.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       If <strong>remove</strong> is specified to
     *       <span style="text-decoration: underline;">action</span> parameter,
     *       host information configured in <strong>machosts</strong> will be
     *       removed from Map Deny list of MAC mapping.
     *       <ul>
     *         <li>
     *           It will be ignored if one tries to remove host information
     *           which is not present in existing Map Deny list.
     *         </li>
     *         <li>
     *           MAC mapping is removed if both Map Allow list and Map Deny
     *           list become empty.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       Map Deny list is not modified if <strong>machosts</strong>
     *       is empty.
     *     </li>
     *     <li>
     *       The following MAC addresses cannot be configured in
     *       <strong>machost</strong> element.
     *       <ul>
     *         <li>Zero <code>00:00:00:00:00:00</code></li>
     *         <li>Broadcast address <code>ff:ff:ff:ff:ff:ff</code></li>
     *         <li>Multicast address</li>
     *       </ul>
     *     </li>
     *   </ul>
     * @param action
     *   Specify a string which determines how to change Map Deny list in
     *   MAC mapping.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">add
     *     <dd style="margin-left: 1.5em">
     *       Add host information configured in <strong>machosts</strong>
     *       to Map Deny list.
     *
     *     <dt style="font-weight: bold;">remove
     *     <dd style="margin-left: 1.5em">
     *       Remove host information configured in <strong>machosts</strong>
     *       from Map Deny list.
     *   </dl>
     *   <p>
     *     If omitted, it will be treated as if <strong>add</strong> is
     *     specified.
     *   </p>
     * @return  Response as dictated by the HTTP Response Status code.
     */
    @Path("deny")
    @POST
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(TypeHint.NO_CONTENT.class)
    @ResponseHeaders({
        @ResponseHeader(name = "Location",
                        description = "URI corresponding to the newly " +
                        "configured Map Deny list of MAC mapping, " +
                        "which is the same URI specified in request. " +
                        "This header is set only if CREATED(201) is " +
                        "returned as response code.")})
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Existing MAC mapping configuration was " +
                      "changed or deleted successfully."),
        @ResponseCode(code = HTTP_CREATED,
                      condition = "MAC mapping was newly configured " +
                      "successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "MAC mapping configuration was not " +
                      "changed."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "<ul>" +
                      "<li>Incorrect XML or JSON data is specified " +
                      "in Request body.</li>" +
                      "<li><strong>machost</strong> element with invalid " +
                      "MAC address or VLAN ID is configured in " +
                      "<strong>machosts</strong>.</li>" +
                      "<li><strong>machosts</strong> element does not " +
                      "contain <strong>machost</strong> element.</li>" +
                      "<li>\"add\" is passed to <u>action</u>, " +
                      "and <strong>machost</strong> element is specified " +
                      "without MAC address in " +
                      "<strong>machosts</strong>.</li>" +
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
    public Response modifyMacMapDenied(
            @Context UriInfo uriInfo,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @QueryParam("action") String action,
            @TypeHint(MacHostSet.class) MacHostSet mhset) {
        Set<DataLinkHost> dlhosts = toValidDataLinkHostSet(mhset);
        VtnUpdateOperationType op = toVtnUpdateOperationType(action);
        return setMacMap(uriInfo, tenantName, bridgeName, op,
                         VtnAclType.DENY, dlhosts);
    }

    /**
     * Remove all contents in Map Deny list of MAC mapping configured in
     * the specified vBridge.
     *
     * <p>
     *   MAC mapping is removed if both Map Allow list and Map Deny list
     *   become empty.
     * </p>
     *
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @return  Response as dictated by the HTTP Response Status code.
     */
    @Path("deny")
    @DELETE
    @TypeHint(TypeHint.NO_CONTENT.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "MAC mapping configuration was not " +
                      "changed."),
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
    public Response deleteMacMapDenied(
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName) {
        return setMacMap(null, tenantName, bridgeName,
                         VtnUpdateOperationType.SET, VtnAclType.DENY, null);
    }

    /**
     * Determine whether the specified host information is present in the
     * Map Deny list of MAC mapping configured in the specified vBridge.
     *
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @param macAddr
     *   A string representation of MAC address to be tested.
     *   <ul>
     *     <li>
     *       A MAC address should be specified in hexadecimal notation with
     *       {@code ':'} inserted between octets.
     *       (e.g. {@code 11:22:33:aa:bb:cc})
     *     </li>
     *   </ul>
     * @param vlan
     *   A string representation of VLAN ID to which the host belongs.
     *   <ul>
     *      <li>
     *        Allowed range of value is between <strong>0</strong> or more and
     *        <strong>4095</strong> or less.
     *      </li>
     *      <li>
     *        <strong>0</strong> implies untagged network.
     *      </li>
     *   </ul>
     * @return  <strong>machost</strong> element contains MAC address and
     *          VLAN ID specified by the requested URI.
     */
    @Path("deny/{macAddr}/{vlan}")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(MacHost.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "The specified host is configured in the " +
                      "Map Deny list of MAC mapping."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "<ul>" +
                      "<li>The specified host is not configured in the " +
                      "Map Deny list of MAC mapping.</li>" +
                      "<li>MAC mapping is not configured in the specified " +
                      "vBridge.<li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "<ul>" +
                      "<li>Invalid MAC address is passed to " +
                      "<u>{macAddr}</u>.</li>" +
                      "<li>Invalid VLAN ID is passed to <u>{vlan}</u>.</li>" +
                      "</ul>"),
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
    public MacHost getMacMapDeniedHost(
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @PathParam("macAddr") String macAddr,
            @PathParam("vlan") String vlan) {
        return getMacMapConfig(tenantName, bridgeName, VtnAclType.DENY,
                               macAddr, vlan);
    }

    /**
     * Add the host information specified by the requested URI to the
     * Map Deny list of MAC mapping configured in the specified vBridge.
     *
     * <p>
     *   Map Deny list is not modified if the host information specified by
     *   <span style="text-decoration: underline;">{macAddr}</span> and
     *   <span style="text-decoration: underline;">{vlan}</span> parameters is
     *   already configured in the Map Deny list.
     * </p>
     *
     * @param uriInfo     Requested URI information.
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @param macAddr
     *   A string representation of MAC address to be added.
     *   <ul>
     *     <li>
     *       A MAC address should be specified in hexadecimal notation with
     *       {@code ':'} inserted between octets.
     *       (e.g. {@code 11:22:33:aa:bb:cc})
     *     </li>
     *     <li>
     *       The following MAC addresses cannot be specified.
     *       <ul>
     *         <li>Zero <code>00:00:00:00:00:00</code></li>
     *         <li>Broadcast address <code>ff:ff:ff:ff:ff:ff</code></li>
     *         <li>Multicast address</li>
     *       </ul>
     *     </li>
     *   </ul>
     * @param vlan
     *   A string representation of VLAN ID to which the host belongs.
     *   <ul>
     *      <li>
     *        Allowed range of value is between <strong>0</strong> or more and
     *        <strong>4095</strong> or less.
     *      </li>
     *      <li>
     *        <strong>0</strong> implies untagged network.
     *      </li>
     *   </ul>
     * @return  Response as dictated by the HTTP Response Status code.
     */
    @Path("deny/{macAddr}/{vlan}")
    @PUT
    @TypeHint(TypeHint.NO_CONTENT.class)
    @ResponseHeaders({
        @ResponseHeader(name = "Location",
                        description = "URI corresponding to the newly " +
                        "configured host information in the Map Deny list " +
                        "of MAC mapping, which is the same URI specified in " +
                        "request. This header is set only if CREATED(201) " +
                        "is returned as response code.")})
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "The specified host information was added " +
                      "into the Map Deny list of existing MAC mapping."),
        @ResponseCode(code = HTTP_CREATED,
                      condition = "MAC mapping was newly configured " +
                      "successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "MAC mapping configuration was not " +
                      "changed."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "<ul>" +
                      "<li>Invalid MAC address is passed to " +
                      "<u>{macAddr}</u>.</li>" +
                      "<li>\"ANY\" is passed to <u>{macAddr}</u>.</li>" +
                      "<li>Invalid VLAN ID is passed to <u>{vlan}</u>.</li>" +
                      "</ul>"),
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
    public Response setMacMapDeniedHost(
            @Context UriInfo uriInfo,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @PathParam("macAddr") String macAddr,
            @PathParam("vlan") String vlan) {
        return setMacMap(uriInfo, tenantName, bridgeName,
                         VtnUpdateOperationType.ADD, VtnAclType.DENY, macAddr,
                         vlan);
    }

    /**
     * Delete the host information specified by the requested URI from the
     * Map Deny list of MAC mapping configured in the specified vBridge.
     *
     * <ul>
     *   <li>
     *     Map Deny list is not modified if the host information specified by
     *     <span style="text-decoration: underline;">{macAddr}</span> and
     *     <span style="text-decoration: underline;">{vlan}</span> parameters
     *     is not configured in the Map Deny list.
     *   </li>
     *   <li>
     *     MAC mapping is removed if both Map Allow list and Map Deny list
     *     become empty.
     *   </li>
     * </ul>
     *
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @param macAddr
     *   A string representation of MAC address to be removed.
     *   <ul>
     *     <li>
     *       A MAC address should be specified in hexadecimal notation with
     *       {@code ':'} inserted between octets.
     *       (e.g. {@code 11:22:33:aa:bb:cc})
     *     </li>
     *   </ul>
     * @param vlan
     *   A string representation of VLAN ID to which the host belongs.
     *   <ul>
     *      <li>
     *        Allowed range of value is between <strong>0</strong> or more and
     *        <strong>4095</strong> or less.
     *      </li>
     *      <li>
     *        <strong>0</strong> implies untagged network.
     *      </li>
     *   </ul>
     * @return  Response as dictated by the HTTP Response Status code.
     */
    @Path("deny/{macAddr}/{vlan}")
    @DELETE
    @TypeHint(TypeHint.NO_CONTENT.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "The specified host was removed from the " +
                      "Map Deny list of MAC mapping."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "<ul>" +
                      "<li>The specified host is not configured in the " +
                      "Map Deny list of MAC mapping.</li>" +
                      "<li>MAC mapping is not configured in the specified " +
                      "vBridge.<li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "<ul>" +
                      "<li>Invalid MAC address is passed to " +
                      "<u>{macAddr}</u>.</li>" +
                      "<li>Invalid VLAN ID is passed to <u>{vlan}</u>.</li>" +
                      "</ul>"),
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
    public Response deleteMacMapDeniedHost(
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @PathParam("macAddr") String macAddr,
            @PathParam("vlan") String vlan) {
        return setMacMap(null, tenantName, bridgeName,
                         VtnUpdateOperationType.REMOVE, VtnAclType.DENY,
                         macAddr, vlan);
    }

    /**
     * Return a list of hosts where mapping is actually active based on
     * MAC mapping configured in the specified vBridge.
     *
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @return  <strong>macentries</strong> element contains information about
     *          all hosts actually mapped by MAC mapping.
     */
    @Path("mapped")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(MacEntryList.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "MAC mapping is not configured in the " +
                      "specified vBridge."),
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
    public MacEntryList getMacMappedHosts(
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName) {
        checkPrivilege(Privilege.READ);

        IVTNManager mgr = getVTNManager();
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        try {
            List<MacAddressEntry> list = mgr.getMacMappedHosts(path);
            return (list == null) ? null : new MacEntryList(list);
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Determine whether the specified MAC address is actually mapped on
     * the basis of the MAC mapping configured in the specified vBridge.
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
     *          the host actually mapped by MAC mapping.
     */
    @Path("mapped/{macAddr}")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(MacEntry.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "<ul>" +
                      "<li>The mapping with the MAC address specified in " +
                      "<u>{macAddr}</u> has not been activated based on the " +
                      "MAC mapping of the specified vBridge.</li>" +
                      "<li>MAC mapping is not configured in the specified " +
                      "vBridge.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "Invalid MAC address is passed to " +
                      "<u>{macAddr}</u>."),
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
    public MacEntry getMacMappedHost(
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @PathParam("macAddr") String macAddr) {
        checkPrivilege(Privilege.READ);

        EthernetAddress dladdr = parseEthernetAddress(macAddr);
        IVTNManager mgr = getVTNManager();
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        try {
            MacAddressEntry entry = mgr.getMacMappedHost(path, dladdr);
            return (entry == null) ? null : new MacEntry(entry);
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Return a {@link MacHostSet} instance which contains host information
     * configured in the access control list of MAC mapping.
     *
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @param aclType     A {@link VtnAclType} instance which determines
     *                    the type of the access control list.
     * @return  A {@link MacHostSet} instance if MAC mapping is configured.
     *          {@code null} is returned if not configured.
     */
    private MacHostSet getMacMapConfig(String tenantName, String bridgeName,
                                       VtnAclType aclType) {
        checkPrivilege(Privilege.READ);

        IVTNManager mgr = getVTNManager();
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        try {
            Set<DataLinkHost> dlhosts = mgr.getMacMapConfig(path, aclType);
            return (dlhosts == null) ? null : new MacHostSet(dlhosts);
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Determine whether the specified host is configured in the access control
     * list of the MAC mapping.
     *
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @param aclType     A {@link VtnAclType} instance which determines
     *                    the type of the access control list.
     * @param macAddr     A string representation of MAC address.
     * @param vlan        A string representation of VLAN ID.
     * @return  A {@link MacHost} instance is returned if the specified host
     *          is configured in the specified access control list.
     *          Otherwise {@code null} is returned.
     */
    private MacHost getMacMapConfig(String tenantName, String bridgeName,
                                    VtnAclType aclType, String macAddr,
                                    String vlan) {
        checkPrivilege(Privilege.READ);

        DataLinkHost dlhost = toDataLinkHost(macAddr, vlan);
        IVTNManager mgr = getVTNManager();
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        try {
            Set<DataLinkHost> dlhosts = mgr.getMacMapConfig(path, aclType);
            if (dlhosts != null && dlhosts.contains(dlhost)) {
                return new MacHost(dlhost);
            }

            return null;
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Change MAC mapping configuration.
     *
     * @param uriInfo     Requested URI information.
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @param op          A {@link VtnUpdateOperationType} instance which
     *                    determines how to change the configuration.
     * @param mcconf      A {@link MacMapConfig} instance to be applied.
     * @return  A {@link Response} instance which represents HTTP response
     *          to be returned.
     */
    private Response setMacMap(UriInfo uriInfo, String tenantName,
                               String bridgeName, VtnUpdateOperationType op,
                               MacMapConfig mcconf) {
        checkPrivilege(Privilege.WRITE);

        IVTNManager mgr = getVTNManager();
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        try {
            UpdateType type = mgr.setMacMap(path, op, mcconf);

            if (type == null) {
                return Response.noContent().build();
            }
            if (type == UpdateType.ADDED) {
                // Return CREATED with Location header.
                return Response.created(uriInfo.getAbsolutePath()).build();
            }

            return Response.ok().build();
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Change the specified access control list in the MAC mapping
     * configuration.
     *
     * @param uriInfo     Requested URI information.
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @param op          A {@link VtnUpdateOperationType} instance which
     *                    determines how to change the configuration.
     * @param aclType     A {@link VtnAclType} instance which determines
     *                    the access control list.
     * @param dlhosts     A set of {@link DataLinkHost} instances.
     * @return  A {@link Response} instance which represents HTTP response
     *          to be returned.
     */
    private Response setMacMap(UriInfo uriInfo, String tenantName,
                               String bridgeName, VtnUpdateOperationType op,
                               VtnAclType aclType, Set<DataLinkHost> dlhosts) {
        checkPrivilege(Privilege.WRITE);

        IVTNManager mgr = getVTNManager();
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        try {
            UpdateType type = mgr.setMacMap(path, op, aclType, dlhosts);

            if (type == null) {
                return Response.noContent().build();
            }
            if (type == UpdateType.ADDED) {
                // Return CREATED with Location header.
                return Response.created(uriInfo.getAbsolutePath()).build();
            }

            return Response.ok().build();
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Add/Remove the specified host to/from the specified access control list
     * in the MAC mapping configuration.
     *
     * @param uriInfo     Requested URI information.
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @param op          A {@link VtnUpdateOperationType} instance which
     *                    determines how to change the configuration.
     * @param aclType     A {@link VtnAclType} instance which determines
     *                    the access control list.
     * @param macAddr     A string representation of MAC address.
     * @param vlan        A string representation of VLAN ID.
     * @return  A {@link Response} instance which represents HTTP response
     *          to be returned.
     */
    private Response setMacMap(UriInfo uriInfo, String tenantName,
                               String bridgeName, VtnUpdateOperationType op,
                               VtnAclType aclType, String macAddr,
                               String vlan) {
        checkPrivilege(Privilege.WRITE);

        Set<DataLinkHost> dlhosts = new HashSet<DataLinkHost>();
        dlhosts.add(toDataLinkHost(macAddr, vlan));

        return setMacMap(uriInfo, tenantName, bridgeName, op, aclType, dlhosts);
    }

    /**
     * Convert the specified {@link MacMapConfigInfo} instance into
     * a {@link MacMapConfig} instance.
     *
     * @param mci  A {@link MacMapConfigInfo} instance.
     * @return     A {@link MacMapConfig} instance.
     * @throws BadRequestException
     *    {@code mci} is empty, or it contains at least one invalid MAC
     *    address.
     */
    private MacMapConfig toMacMapConfig(MacMapConfigInfo mci) {
        if (mci == null) {
            // This should never happen.
            String msg = "MAC mapping configuration cannot be null.";
            throw new BadRequestException(msg);
        }

        MacHostSet allow = mci.getAllowedHosts();
        MacHostSet deny = mci.getDeniedHosts();
        Set<DataLinkHost> allowedSet = toDataLinkHostSet(allow);
        Set<DataLinkHost> deniedSet = toDataLinkHostSet(deny);

        if (allowedSet == null && deniedSet == null) {
            String msg = "MAC mapping configuration cannot be empty";
            throw new BadRequestException(msg);
        }

        return new MacMapConfig(allowedSet, deniedSet);
    }

    /**
     * Convert a {@link MacHost} instance into a {@link DataLinkHost} instance.
     *
     * @param mhost   A {@link MacHost} instance.
     * @return  A {@link DataLinkHost} instance.
     * @throws BadRequestException
     *    An invalid MAC address is contained in {@code mhost}.
     */
    private DataLinkHost toDataLinkHost(MacHost mhost) {
        if (mhost == null) {
            // This should never happen.
            throw new BadRequestException("MAC host is null");
        }

        String saddr = mhost.getAddress();
        EthernetAddress eaddr = (saddr == null)
            ? null : parseEthernetAddress(saddr);

        return new EthernetHost(eaddr, mhost.getVlan());
    }

    /**
     * Construct a {@link DataLinkHost} instance from a string representation
     * of MAC address and VLAN ID.
     *
     * @param macAddr  A string representation of MAC address.
     * @param vlan     A string representation of VLAN ID.
     * @return  A {@link DataLinkHost} instance.
     * @throws BadRequestException
     *    An invalid string is specified.
     */
    private DataLinkHost toDataLinkHost(String macAddr, String vlan) {
        EthernetAddress eaddr = (MAC_ADDR_ANY.equalsIgnoreCase(macAddr))
            ? null : parseEthernetAddress(macAddr);
        short vid = parseVlanId(vlan);
        return new EthernetHost(eaddr, vid);
    }

    /**
     * Convert MAC address information in the specified {@link MacHostSet}
     * instance into a set of {@link DataLinkHost}.
     *
     * @param mhset  A {@link MacHostSet} instance.
     * @return  A set of {@link DataLinkHost} instance.
     *          {@code null} is returned if {@code mhset} is {@code null} or
     *          empty.
     * @throws BadRequestException
     *    An invalid MAC address is contained in {@code mhset}.
     */
    private Set<DataLinkHost> toDataLinkHostSet(MacHostSet mhset) {
        if (mhset == null) {
            return null;
        }

        Set<MacHost> hostSet = mhset.getHosts();
        if (hostSet == null) {
            return null;
        }

        Set<DataLinkHost> dlhosts = new HashSet<DataLinkHost>();
        for (MacHost mhost: hostSet) {
            dlhosts.add(toDataLinkHost(mhost));
        }

        return (dlhosts.isEmpty()) ? null : dlhosts;
    }

    /**
     * Convert MAC address information in the specified {@link MacHostSet}
     * instance into a set of {@link DataLinkHost}.
     *
     * @param mhset  A {@link MacHostSet} instance.
     * @return  A set of {@link DataLinkHost} instance.
     * @throws BadRequestException
     *    An invalid MAC address is contained in {@code mhset}, or it does not
     *    contain {@link MacHost} instance.
     */
    private Set<DataLinkHost> toValidDataLinkHostSet(MacHostSet mhset) {
        Set<DataLinkHost> dlhosts = toDataLinkHostSet(mhset);
        if (dlhosts == null) {
            throw new BadRequestException("Host set cannot be empty");
        }

        return dlhosts;
    }

    /**
     * Return an {@link VtnUpdateOperationType} instance specified by the given
     * string.
     *
     * @param action  A string which indicates the type of operation.
     * @return  A {@link VtnUpdateOperationType} instance.
     * @throws BadRequestException
     *    An invalid string is specified to {@code action}.
     */
    private VtnUpdateOperationType toVtnUpdateOperationType(String action) {
        if (action == null || action.equalsIgnoreCase("add")) {
            return VtnUpdateOperationType.ADD;
        }
        if (action.equalsIgnoreCase("remove")) {
            return VtnUpdateOperationType.REMOVE;
        }

        throw new BadRequestException("Unsupported action: " + action);
    }
}
