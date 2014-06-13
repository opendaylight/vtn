/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import static java.net.HttpURLConnection.HTTP_INTERNAL_ERROR;
import static java.net.HttpURLConnection.HTTP_NOT_ACCEPTABLE;
import static java.net.HttpURLConnection.HTTP_NOT_FOUND;
import static java.net.HttpURLConnection.HTTP_NO_CONTENT;
import static java.net.HttpURLConnection.HTTP_OK;
import static java.net.HttpURLConnection.HTTP_UNAUTHORIZED;
import static java.net.HttpURLConnection.HTTP_UNAVAILABLE;

import javax.ws.rs.DELETE;
import javax.ws.rs.DefaultValue;
import javax.ws.rs.GET;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.Produces;
import javax.ws.rs.QueryParam;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.Response;

import org.codehaus.enunciate.jaxrs.ResponseCode;
import org.codehaus.enunciate.jaxrs.StatusCodes;
import org.codehaus.enunciate.jaxrs.TypeHint;

import org.opendaylight.vtn.manager.IVTNFlowDebugger;
import org.opendaylight.vtn.manager.IVTNManager;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.flow.DataFlow;

import org.opendaylight.controller.sal.authorization.Privilege;
import org.opendaylight.controller.sal.utils.Status;

/**
 * This class provides Northbound REST APIs to handle data flow in the VTN.
 *
 * @since  Helium
 */
@Path("/{containerName}/vtns/{tenantName}/flows")
public class FlowNorthbound extends VTNNorthBoundBase {
    /**
     * Return summarized information about all data flows present in the
     * specified VTN.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the VTN.
     * @return
     *    <strong>dataflows</strong> element contains summarized information
     *    about all data flows present in the VTN specified by the requested
     *    URI. Note that below elements in <strong>dataflow</strong> element
     *    are always omitted.
     *    <ul>
     *      <li><strong>match</strong></li>
     *      <li><strong>actions</strong></li>
     *      <li><strong>vnoderoutes</strong></li>
     *      <li><strong>noderoutes</strong></li>
     *      <li><strong>statistics</strong></li>
     *    </ul>
     * @since  Helium
     */
    @Path("summary")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(DataFlowList.class)
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
    public DataFlowList getDataFlows(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName) {
        checkPrivilege(containerName, Privilege.READ);

        IVTNManager mgr = getVTNManager(containerName);
        VTenantPath path = new VTenantPath(tenantName);
        try {
            return new DataFlowList
                (mgr.getDataFlows(path, DataFlow.Mode.SUMMARY));
        } catch (VTNException e) {
            throw getException(e.getStatus());
        }
    }

    /**
     * Return summarized information about the data flow specified by the
     * flow identifier.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the VTN.
     * @param flowId         The identifier of the data flow.
     * @return
     *    <strong>dataflow</strong> element contains summarized information
     *    about the data flow present in the VTN specified by the requested
     *    URI. Note that below elements in <strong>dataflow</strong> element
     *    are always omitted.
     *    <ul>
     *      <li><strong>match</strong></li>
     *      <li><strong>actions</strong></li>
     *      <li><strong>vnoderoutes</strong></li>
     *      <li><strong>noderoutes</strong></li>
     *      <li><strong>statistics</strong></li>
     *    </ul>
     * @since  Helium
     */
    @Path("summary/{flowId}")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(DataFlow.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "The specified data flow does not exist " +
                      "in the specified VTN."),
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
    public DataFlow getDataFlow(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName,
            @PathParam("flowId") String flowId) {
        checkPrivilege(containerName, Privilege.READ);

        IVTNManager mgr = getVTNManager(containerName);
        VTenantPath path = new VTenantPath(tenantName);
        try {
            return mgr.getDataFlow(path, flowId, DataFlow.Mode.SUMMARY);
        } catch (VTNException e) {
            throw getException(e.getStatus());
        }
    }

    /**
     * Return detailed information about all data flows present in the
     * specified VTN.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the VTN.
     * @param update
     *   A boolean value to determine the way to get flow statistics.
     *   Default is <strong>false</strong>.
     *   <ul>
     *     <li>
     *       If <strong>true</strong> is specified, the VTN Manager gets
     *       statistics information from physical switches.
     *     </li>
     *     <li>
     *       If <strong>false</strong> is specified, the VTN Manager gets
     *       statistics information cached in the statistics manager.
     *       The statistics cache will be updated every 10 seconds.
     *     </li>
     *   </ul>
     * @return
     *    <strong>dataflows</strong> element contains detailed information
     *    about all data flows present in the VTN specified by the requested
     *    URI.
     * @since  Helium
     */
    @Path("detail")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(DataFlowList.class)
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
    public DataFlowList getDetailedDataFlows(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName,
            @DefaultValue("false") @QueryParam("update") boolean update) {
        checkPrivilege(containerName, Privilege.READ);

        IVTNManager mgr = getVTNManager(containerName);
        VTenantPath path = new VTenantPath(tenantName);
        DataFlow.Mode mode = (update)
            ? DataFlow.Mode.UPDATE_STATS
            : DataFlow.Mode.DETAIL;
        try {
            return new DataFlowList(mgr.getDataFlows(path, mode));
        } catch (VTNException e) {
            throw getException(e.getStatus());
        }
    }

    /**
     * Return detailed information about the data flow specified by the
     * flow identifier.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the VTN.
     * @param flowId         The identifier of the data flow.
     * @param update
     *   A boolean value to determine the way to get flow statistics.
     *   Default is <strong>false</strong>.
     *   <ul>
     *     <li>
     *       If <strong>true</strong> is specified, the VTN Manager gets
     *       statistics information from physical switches.
     *     </li>
     *     <li>
     *       If <strong>false</strong> is specified, the VTN Manager gets
     *       statistics information cached in the statistics manager.
     *       The statistics cache will be updated every 10 seconds.
     *     </li>
     *   </ul>
     * @return
     *    <strong>dataflow</strong> element contains detailed information
     *    about the data flow specified by the requested URI.
     * @since  Helium
     */
    @Path("detail/{flowId}")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(DataFlow.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "The specified data flow does not exist " +
                      "in the specified VTN."),
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
    public DataFlow getDetailedDataFlow(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName,
            @PathParam("flowId") String flowId,
            @DefaultValue("false") @QueryParam("update") boolean update) {
        checkPrivilege(containerName, Privilege.READ);

        IVTNManager mgr = getVTNManager(containerName);
        VTenantPath path = new VTenantPath(tenantName);
        DataFlow.Mode mode = (update)
            ? DataFlow.Mode.UPDATE_STATS
            : DataFlow.Mode.DETAIL;
        try {
            return mgr.getDataFlow(path, flowId, mode);
        } catch (VTNException e) {
            throw getException(e.getStatus());
        }
    }

    /**
     * Return the number of data flows present in the specified VTN.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the VTN.
     * @return
     *    <strong>integer</strong> element contains the number of data flows
     *    present in the VTN specified by the requested URI.
     * @since  Helium
     */
    @Path("count")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(XmlLongInteger.class)
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
    public XmlLongInteger getFlowCount(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName) {
        checkPrivilege(containerName, Privilege.READ);

        IVTNManager mgr = getVTNManager(containerName);
        VTenantPath path = new VTenantPath(tenantName);
        try {
            return new XmlLongInteger(mgr.getDataFlowCount(path));
        } catch (VTNException e) {
            throw getException(e.getStatus());
        }
    }

    /**
     * Remove all data flows in the specified VTN.
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
     * @since  Helium
     */
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
