/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import static java.net.HttpURLConnection.HTTP_BAD_REQUEST;
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
import javax.ws.rs.GET;
import javax.ws.rs.PUT;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.Produces;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.Response;
import javax.ws.rs.core.UriInfo;

import org.codehaus.enunciate.jaxrs.ResponseCode;
import org.codehaus.enunciate.jaxrs.ResponseHeader;
import org.codehaus.enunciate.jaxrs.ResponseHeaders;
import org.codehaus.enunciate.jaxrs.StatusCodes;
import org.codehaus.enunciate.jaxrs.TypeHint;

import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.flow.filter.FlowFilter;
import org.opendaylight.vtn.manager.flow.filter.FlowFilterId;

/**
 * This class provides Northbound REST APIs to handle flow filter in the
 * vBridge interface.
 *
 * <p>
 *   Each vBridge interface has two flow filter lists.
 * </p>
 * <dl style="margin-left: 1em;">
 *   <dt>Input flow filter list
 *   <dd style="margin-left: 1.5em">
 *     <p>
 *       Flow filters in this list are evaluated when a packet is forwarded to
 *       the vBridge interface. This list is evaluated at the following
 *       instances.
 *     </p>
 *     <ul>
 *       <li>
 *         When an incoming packet is mapped to the vBridge interface by
 *         port mapping.
 *       </li>
 *       <li>
 *         When a packet is redirected by another flow filter to the
 *         vBridge interface as an incoming packet.
 *       </li>
 *     </ul>
 *
 *   <dt>Output flow filter list
 *   <dd style="margin-left: 1.5em">
 *     <p>
 *       Flow filters in this list are evaluated when a packet is going to be
 *       transmitted to the physical network mapped to the vBridge interface.
 *       This list is evaluated at the following instances.
 *     </p>
 *     <ul>
 *       <li>
 *         When a packet is forwarded from the vBridge to the virtual
 *         interface.
 *       </li>
 *       <li>
 *         When a packet is redirected by another flow filter to the
 *         vBridge interface as an outgoing packet.
 *       </li>
 *     </ul>
 * </dl>
 *
 * @since Helium
 */
@Path("/default/vtns/{tenantName}/vbridges/{bridgeName}/interfaces/{ifName}/flowfilters")
public class VBridgeIfFlowFilterNorthbound extends VTNNorthBoundBase {
    /**
     * Return information about flow filters configured in the specified
     * vBridge interface.
     *
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @param ifName      The name of the vBridge interface.
     * @param listType
     *   The type of the flow filter list (case insensitive).
     *   <dl style="margin-left: 1em;">
     *     <dt>in
     *     <dd style="margin-left: 1.5em">
     *       Indicates the input flow filter list, which is evaluated when a
     *       packet is forwarded to the vBridge interface.
     *
     *     <dt>out
     *     <dd style="margin-left: 1.5em">
     *       Indicates the output flow filter list, which is evaluated when a
     *       packet is going to be transmitted to the physical network mapped
     *       to the vBridge interface.
     *   </dl>
     * @return
     *   <strong>flowfilters</strong> element contains information about the
     *   vBridge interface flow filter list specified by the requested URI.
     */
    @Path("{listType}")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(FlowFilterList.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "Invalid value is passed to " +
                      "<u>{listType}</u>."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified VTN does not exist.</li>" +
                      "<li>The specified vBridge does not exist.</li>" +
                      "<li>The specified vBridge interface does not " +
                      "exist.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public FlowFilterList getFlowFilters(
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @PathParam("ifName") String ifName,
            @PathParam("listType") String listType) {
        FlowFilterId fid = createFlowFilterId(tenantName, bridgeName, ifName,
                                              listType);
        return getFlowFilters(fid);
    }

    /**
     * Delete all flow filters in the specified vBridge interface flow filter
     * list.
     *
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @param ifName      The name of the vBridge interface.
     * @param listType
     *   The type of the flow filter list (case insensitive).
     *   <dl style="margin-left: 1em;">
     *     <dt>in
     *     <dd style="margin-left: 1.5em">
     *       Indicates the input flow filter list, which is evaluated when a
     *       packet is forwarded to the vBridge interface.
     *
     *     <dt>out
     *     <dd style="margin-left: 1.5em">
     *       Indicates the output flow filter list, which is evaluated when a
     *       packet is going to be transmitted to the physical network mapped
     *       to the vBridge interface.
     *   </dl>
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{listType}")
    @DELETE
    @TypeHint(TypeHint.NO_CONTENT.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Flow filters were deleted successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "No flow filter was configured in the " +
                      "specified vBridge interface flow filter list."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "Invalid value is passed to " +
                      "<u>{listType}</u>."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified VTN does not exist.</li>" +
                      "<li>The specified vBridge does not exist.</li>" +
                      "<li>The specified vBridge interface does not " +
                      "exist.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public Response deleteFlowFilters(
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @PathParam("ifName") String ifName,
            @PathParam("listType") String listType) {
        FlowFilterId fid = createFlowFilterId(tenantName, bridgeName, ifName,
                                              listType);
        return deleteFlowFilters(fid);
    }

    /**
     * Return information about the flow filter specified by
     * the flow filter index in the vBridge interface flow filter list.
     *
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @param ifName      The name of the vBridge interface.
     * @param listType
     *   The type of the flow filter list (case insensitive).
     *   <dl style="margin-left: 1em;">
     *     <dt>in
     *     <dd style="margin-left: 1.5em">
     *       Indicates the input flow filter list, which is evaluated when a
     *       packet is forwarded to the vBridge interface.
     *
     *     <dt>out
     *     <dd style="margin-left: 1.5em">
     *       Indicates the output flow filter list, which is evaluated when a
     *       packet is going to be transmitted to the physical network mapped
     *       to the vBridge interface.
     *   </dl>
     * @param index
     *   The index value which specifies the flow filter in the vBridge
     *   interface flow filter list. A string representation of an integer
     *   value must be specified.
     * @return  <strong>flowfilter</strong> element contains information
     *          about the flow filter specified by the requested URI.
     */
    @Path("{listType}/{index}")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(FlowFilter.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "The specified flow filter does not exist " +
                      "in the specified vBridge."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "Invalid value is passed to " +
                      "<u>{listType}</u>."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified VTN does not exist.</li>" +
                      "<li>The specified vBridge does not exist.</li>" +
                      "<li>The specified vBridge interface does not " +
                      "exist.</li>" +
                      "<li>A string passed to <u>{index}</u> can not be " +
                      "converted into an integer.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public FlowFilter getFlowFilter(
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @PathParam("ifName") String ifName,
            @PathParam("listType") String listType,
            @PathParam("index") int index) {
        FlowFilterId fid = createFlowFilterId(tenantName, bridgeName, ifName,
                                              listType);
        return getFlowFilter(fid, index);
    }

    /**
     * Create or modify the flow filter specified by the index number in the
     * vBridge interface flow filter list.
     *
     * <ul>
     *   <li>
     *     If the flow filter specified by
     *     <span style="text-decoration: underline;">{index}</span> does not
     *     exist in the vBridge interface flow filter list specified by
     *     <span style="text-decoration: underline;">{listType}</span>,
     *     a new flow filter will be associated with
     *     <span style="text-decoration: underline;">{index}</span> in the
     *     specified vBridge interface flow filter list.
     *   </li>
     *   <li>
     *     If the flow filter specified by
     *     <span style="text-decoration: underline;">{index}</span> already
     *     exists in the vBridge interface flow filter list specified by
     *     <span style="text-decoration: underline;">{listType}</span>,
     *     it will be modified as specified by <strong>flowfilter</strong>
     *     element.
     *   </li>
     * </ul>
     *
     * @param uriInfo     Requested URI information.
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @param ifName      The name of the vBridge interface.
     * @param listType
     *   The type of the flow filter list (case insensitive).
     *   <dl style="margin-left: 1em;">
     *     <dt>in
     *     <dd style="margin-left: 1.5em">
     *       Indicates the input flow filter list, which is evaluated when a
     *       packet is forwarded to the vBridge interface.
     *
     *     <dt>out
     *     <dd style="margin-left: 1.5em">
     *       Indicates the output flow filter list, which is evaluated when a
     *       packet is going to be transmitted to the physical network mapped
     *       to the vBridge interface.
     *   </dl>
     * @param index
     *   The index value which specifies the flow filter in the vBridge
     *   interface flow filter list.
     *   <ul>
     *     <li>
     *       A string representation of an integer value must be specified.
     *     </li>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>1</strong> to <strong>65535</strong>.
     *     </li>
     *     <li>
     *       This value is also used to determine the order of flow filter
     *       evaluation. Flow filters in the list are evaluated in ascending
     *       order of indices, and only the first matched flow filter is
     *       applied to the packet.
     *     </li>
     *   </ul>
     * @param ff
     *   <strong>flowfilter</strong> element specifies the configuration of the
     *   flow filter.
     *   <ul>
     *     <li>
     *       The <strong>index</strong> attribute in the
     *       <strong>flowfilter</strong> element is always ignored.
     *       The index number is determined by the
     *       <span style="text-decoration: underline;">{index}</span>
     *       parameter.
     *     </li>
     *     <li>
     *       Note that this API does not check whether the flow condition
     *       specified by the <strong>condition</strong> attribute in
     *       the <strong>flowfilter</strong> element actually exists or not.
     *       The flow filter will be invalidated if the specified
     *       flow condition does not exist.
     *     </li>
     *     <li>
     *       Note that this API does not check whether the destination
     *       virtual interface of the packet redirection, which is configured
     *       in the <strong>redirect</strong> element in the
     *       <strong>flowfilter</strong> element, actually exists or not.
     *       The packet will be discarded if the destination virtual interface
     *       is not found when the packet is going to be redirected by the
     *       flow filter.
     *     </li>
     *   </ul>
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{listType}/{index}")
    @PUT
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(TypeHint.NO_CONTENT.class)
    @ResponseHeaders({
        @ResponseHeader(name = "Location",
                        description = "URI corresponding to the newly " +
                        "created flow filter, which is the same URI " +
                        "specified in request. This header is set only if " +
                        "CREATED(201) is returned as response code.")})
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Existing flow filter was modified " +
                      "successfully."),
        @ResponseCode(code = HTTP_CREATED,
                      condition = "Flow filter was newly created " +
                      "successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "Flow filter was not changed."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "<ul>" +
                      "<li>Incorrect XML or JSON data is specified " +
                      "in Request body.</li>" +
                      "<li>Invalid value is passed to <u>{listType}</u>." +
                      "<li>Index number specified by <u>{index}</u> " +
                      "parameter is out of valid range.</li>" +
                      "<li>Incorrect value is configured in " +
                      "<strong>flowfilter</strong>.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified VTN does not exist.</li>" +
                      "<li>The specified vBridge does not exist.</li>" +
                      "<li>The specified vBridge interface does not " +
                      "exist.</li>" +
                      "<li>A string passed to <u>{index}</u> can not be " +
                      "converted into an integer.</li>" +
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
    public Response putFlowFilter(
            @Context UriInfo uriInfo,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @PathParam("ifName") String ifName,
            @PathParam("listType") String listType,
            @PathParam("index") int index,
            @TypeHint(FlowFilter.class) FlowFilter ff) {
        FlowFilterId fid = createFlowFilterId(tenantName, bridgeName, ifName,
                                              listType);
        return putFlowFilter(uriInfo, fid, index, ff);
    }

    /**
     * Delete the flow filter specified by the index number in the
     * specified vBridge interface flow filter list.
     *
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     * @param ifName      The name of the vBridge interface.
     * @param listType
     *   The type of the flow filter list (case insensitive).
     *   <dl style="margin-left: 1em;">
     *     <dt>in
     *     <dd style="margin-left: 1.5em">
     *       Indicates the input flow filter list, which is evaluated when a
     *       packet is forwarded to the vBridge interface.
     *
     *     <dt>out
     *     <dd style="margin-left: 1.5em">
     *       Indicates the output flow filter list, which is evaluated when a
     *       packet is going to be transmitted to the physical network mapped
     *       to the vBridge interface.
     *   </dl>
     * @param index
     *   The index value which specifies the flow filter in the vBridge
     *   interface flow filter list. A string representation of an integer
     *   value must be specified.
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{listType}/{index}")
    @DELETE
    @TypeHint(TypeHint.NO_CONTENT.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Flow filter was deleted successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "The specified flow filter does not exist " +
                      "in the specified vBridge."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "Invalid value is passed to " +
                      "<u>{listType}</u>."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified VTN does not exist.</li>" +
                      "<li>The specified vBridge does not exist.</li>" +
                      "<li>The specified vBridge interface does not " +
                      "exist.</li>" +
                      "<li>A string passed to <u>{index}</u> can not be " +
                      "converted into an integer.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public Response deleteFlowFilter(
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @PathParam("ifName") String ifName,
            @PathParam("listType") String listType,
            @PathParam("index") int index) {
        FlowFilterId fid = createFlowFilterId(tenantName, bridgeName, ifName,
                                              listType);
        return deleteFlowFilter(fid, index);
    }

    /**
     * Create a flow filter identifier that specifies the flow filter list
     * in the vBridge interface.
     *
     * @param tenant    The name of the tenant.
     * @param bridge    The name of the vBridge.
     * @param iface     The name of the interface.
     * @param listType  The type of the flow filter list.
     * @return  A {@link FlowFilterId} instance.
     */
    private FlowFilterId createFlowFilterId(String tenant, String bridge,
                                            String iface, String listType) {
        VBridgeIfPath path = new VBridgeIfPath(tenant, bridge, iface);
        return new FlowFilterId(path, getFlowFilterType(listType));
    }
}
