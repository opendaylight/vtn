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
import org.codehaus.enunciate.jaxrs.StatusCodes;
import org.codehaus.enunciate.jaxrs.TypeHint;

import org.opendaylight.vtn.manager.IVTNManager;
import org.opendaylight.vtn.manager.PortMap;
import org.opendaylight.vtn.manager.PortMapConfig;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.controller.northbound.commons.exception.UnsupportedMediaTypeException;
import org.opendaylight.controller.sal.authorization.Privilege;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.utils.Status;

/**
 * Northbound REST APIs to handle virtual interfaces attached to virtual
 * layer 2 bridge.
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
@Path("/{containerName}/vtns/{tenantName}/vbridges/{bridgeName}/interfaces")
public class VBridgeInterfaceNorthbound extends VTNNorthBoundBase {
    /**
     * Returns a list of all interfaces attached to the specified virtual L2
     * bridge.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the tenant.
     * @param bridgeName     The name of the bridge.
     * @return  A list of virtual interfaces attached to the bridge.
     */
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(VInterfaceList.class)
    @StatusCodes({
            @ResponseCode(code = 200, condition = "Operation successful"),
            @ResponseCode(code = 401, condition = "Authentication failed"),
            @ResponseCode(code = 404, condition = "The specified resource does not exist"),
            @ResponseCode(code = 500, condition = "Failed due to internal error"),
            @ResponseCode(code = 503, condition = "One or more of Controller Services are unavailable")})
    public VInterfaceList getBridgeInterfaces(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName) {
        checkPrivilege(containerName, Privilege.READ);

        IVTNManager mgr = getVTNManager(containerName);
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        try {
            List<VInterface> list =
                mgr.getBridgeInterfaces(path);
            return new VInterfaceList(list);
        } catch (VTNException e) {
            throw getException(e.getStatus());
        }
    }

    /**
     * Returns a virtual L2 bridge interface information specified by the
     * given name.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the virtual tenant.
     * @param bridgeName     The name of the virtual bridge.
     * @param ifName         The name of the virtual bridge interface.
     * @return  Interface information associated with the specified name.
     */
    @Path("{ifName}")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(VInterface.class)
    @StatusCodes({
            @ResponseCode(code = 200, condition = "Operation successful"),
            @ResponseCode(code = 401, condition = "Authentication failed"),
            @ResponseCode(code = 404, condition = "The specified resource does not exist"),
            @ResponseCode(code = 500, condition = "Failed due to internal error"),
            @ResponseCode(code = 503, condition = "One or more of Controller Services are unavailable")})
    public VInterface getBridgeInterface(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @PathParam("ifName") String ifName) {
        checkPrivilege(containerName, Privilege.READ);

        IVTNManager mgr = getVTNManager(containerName);
        VBridgeIfPath path = new VBridgeIfPath(tenantName, bridgeName, ifName);
        try {
            return mgr.getBridgeInterface(path);
        } catch (VTNException e) {
            throw getException(e.getStatus());
        }
    }

    /**
     * Add a new virtual interface to the virtual L2 bridge.
     *
     * @param uriInfo        Requested URI information.
     * @param containerName  The name of the container.
     * @param tenantName     The name of the virtual tenant.
     * @param bridgeName     The name of the virtual bridge.
     * @param ifName         The name of the virtual bridge interface.
     * @param iconf          Interface configuration.
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{ifName}")
    @POST
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @StatusCodes({
            @ResponseCode(code = 201, condition = "Interface created successfully"),
            @ResponseCode(code = 401, condition = "Authentication failed"),
            @ResponseCode(code = 404, condition = "The specified resource does not exist"),
            @ResponseCode(code = 406, condition = "Cannot operate on Default Container when other Containers are active"),
            @ResponseCode(code = 409, condition = "Failed to create interface due to conflicting name"),
            @ResponseCode(code = 415, condition = "Invalid interface name passed in ifName parameter"),
            @ResponseCode(code = 500, condition = "Failed to create interface. Failure Reason included in HTTP Error response"),
            @ResponseCode(code = 503, condition = "One or more of Controller services are unavailable")})
    public Response addBridgeInterface(
            @Context UriInfo uriInfo,
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @PathParam("ifName") String ifName,
            @TypeHint(VInterfaceConfig.class) VInterfaceConfig iconf) {
        checkPrivilege(containerName, Privilege.WRITE);

        IVTNManager mgr = getVTNManager(containerName);
        VBridgeIfPath path = new VBridgeIfPath(tenantName, bridgeName, ifName);
        Status status = mgr.addBridgeInterface(path, iconf);
        if (status.isSuccess()) {
            return Response.created(uriInfo.getRequestUri()).build();
        }

        throw getException(status);
    }

    /**
     * Modify configuration of existing virtual interface in the virtual L2
     * bridge.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the virtual tenant.
     * @param bridgeName     The name of the virtual bridge.
     * @param ifName         The name of the virtual bridge interface.
     * @param all            If {@code true} is specified, all attributes
     *                       of the interface are modified. {@code null} in
     *                       request body field is interpreted as default
     *                       value.
     *                       If {@code false} is specified, all fields to
     *                       which is assigned {@code null} are not modified.
     * @param iconf          Interface configuration.
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{ifName}")
    @PUT
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @StatusCodes({
            @ResponseCode(code = 200, condition = "Interface modified successfully"),
            @ResponseCode(code = 401, condition = "Authentication failed"),
            @ResponseCode(code = 404, condition = "The specified resource does not exist"),
            @ResponseCode(code = 406, condition = "Cannot operate on Default Container when other Containers are active"),
            @ResponseCode(code = 500, condition = "Failed to modify interface. Failure Reason included in HTTP Error response"),
            @ResponseCode(code = 503, condition = "One or more of Controller services are unavailable")})
    public Response modifyBridgeInterface(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @PathParam("ifName") String ifName,
            @DefaultValue("false") @QueryParam("all") boolean all,
            @TypeHint(VInterfaceConfig.class) VInterfaceConfig iconf) {
        checkPrivilege(containerName, Privilege.WRITE);

        IVTNManager mgr = getVTNManager(containerName);
        VBridgeIfPath path = new VBridgeIfPath(tenantName, bridgeName, ifName);
        Status status = mgr.modifyBridgeInterface(path, iconf, all);
        if (status.isSuccess()) {
            return Response.ok().build();
        }

        throw getException(status);
    }

    /**
     * Delete a virtual interface from the virtual L2 bridge.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the virtual tenant.
     * @param bridgeName     The name of the virtual bridge.
     * @param ifName         The name of the virtual bridge interface.
     * @return Response as dictated by the HTTP Response code.
     */
    @Path("{ifName}")
    @DELETE
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @StatusCodes({
            @ResponseCode(code = 200, condition = "Interface deleted successfully"),
            @ResponseCode(code = 401, condition = "Authentication failed"),
            @ResponseCode(code = 404, condition = "The specified resource does not exist"),
            @ResponseCode(code = 406, condition = "Cannot operate on Default Container when other Containers are active"),
            @ResponseCode(code = 500, condition = "Failed to delete interface. Failure Reason included in HTTP Error response"),
            @ResponseCode(code = 503, condition = "One or more of Controller service is unavailable") })
    public Response deleteBridgeInterface(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @PathParam("ifName") String ifName) {
        checkPrivilege(containerName, Privilege.WRITE);

        IVTNManager mgr = getVTNManager(containerName);
        VBridgeIfPath path = new VBridgeIfPath(tenantName, bridgeName, ifName);
        Status status = mgr.removeBridgeInterface(path);
        if (status.isSuccess()) {
            return Response.ok().build();
        }

        throw getException(status);
    }

    /**
     * Returns a port mapping configuration on the virtual L2 bridge interface.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the tenant.
     * @param bridgeName     The name of the bridge.
     * @param ifName         The name of the virtual bridge interface.
     * @return  A port mapping configuration. {@code null} is returned if no
     *          port mapping is configured.
     */
    @Path("{ifName}/portmap")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(PortMap.class)
    @StatusCodes({
            @ResponseCode(code = 200, condition = "Operation successful"),
            @ResponseCode(code = 204, condition = "Port mapping is not configured on the specified interface"),
            @ResponseCode(code = 401, condition = "Authentication failed"),
            @ResponseCode(code = 404, condition = "The specified resource does not exist"),
            @ResponseCode(code = 500, condition = "Failed due to internal error"),
            @ResponseCode(code = 503, condition = "One or more of Controller Services are unavailable")})
    public PortMapInfo getPortMap(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @PathParam("ifName") String ifName) {
        checkPrivilege(containerName, Privilege.READ);

        IVTNManager mgr = getVTNManager(containerName);
        VBridgeIfPath path = new VBridgeIfPath(tenantName, bridgeName, ifName);
        try {
            PortMap pmap = mgr.getPortMap(path);
            if (pmap == null) {
                return null;
            }

            PortMapConfig pmconf = pmap.getConfig();
            NodeConnector nc = pmap.getNodeConnector();
            return new PortMapInfo(pmconf, nc);
        } catch (VTNException e) {
            throw getException(e.getStatus());
        }
    }

    /**
     * Establish port mapping between the virtual L2 bridge interface and
     * the physical switch port.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the tenant.
     * @param bridgeName     The name of the bridge.
     * @param ifName         The name of the virtual bridge interface.
     * @param pmconf         Port mapping configuration.
     * @return Response as dictated by the HTTP Response code.
     */
    @Path("{ifName}/portmap")
    @PUT
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @StatusCodes({
            @ResponseCode(code = 200, condition = "Port mapping configured successfully"),
            @ResponseCode(code = 401, condition = "Authentication failed"),
            @ResponseCode(code = 404, condition = "The specified resource does not exist"),
            @ResponseCode(code = 406, condition = "Cannot operate on Default Container when other Containers are active"),
            @ResponseCode(code = 409, condition = "The specified port is already mapped"),
            @ResponseCode(code = 500, condition = "Failed to create mapping. Failure Reason included in HTTP Error response"),
            @ResponseCode(code = 503, condition = "One or more of Controller services are unavailable")})
    public Response setPortMap(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @PathParam("ifName") String ifName,
            @TypeHint(PortMapConfig.class) PortMapConfig pmconf) {
        checkPrivilege(containerName, Privilege.WRITE);

        IVTNManager mgr = getVTNManager(containerName);
        VBridgeIfPath path = new VBridgeIfPath(tenantName, bridgeName, ifName);
        if (pmconf == null) {
            // This should never happen.
            String desc = "Port map configuration is null";
            throw new UnsupportedMediaTypeException(desc);
        }

        Status status = mgr.setPortMap(path, pmconf);
        if (status.isSuccess()) {
            return Response.ok().build();
        }

        throw getException(status);
    }

    /**
     * Delete port mapping configuration.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the tenant.
     * @param bridgeName     The name of the bridge.
     * @param ifName         The name of the virtual bridge interface.
     * @return Response as dictated by the HTTP Response code.
     */
    @Path("{ifName}/portmap")
    @DELETE
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @StatusCodes({
            @ResponseCode(code = 200, condition = "Port mapping deleted successfully"),
            @ResponseCode(code = 401, condition = "Authentication failed"),
            @ResponseCode(code = 404, condition = "The specified resource does not exist"),
            @ResponseCode(code = 406, condition = "Cannot operate on Default Container when other Containers are active"),
            @ResponseCode(code = 500, condition = "Failed to delete mapping. Failure Reason included in HTTP Error response"),
            @ResponseCode(code = 503, condition = "One or more of Controller services are unavailable")})
    public Response unetPortMap(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @PathParam("ifName") String ifName) {
        checkPrivilege(containerName, Privilege.WRITE);

        IVTNManager mgr = getVTNManager(containerName);
        VBridgeIfPath path = new VBridgeIfPath(tenantName, bridgeName, ifName);
        Status status = mgr.setPortMap(path, null);
        if (status.isSuccess()) {
            return Response.ok().build();
        }

        throw getException(status);
    }
}
