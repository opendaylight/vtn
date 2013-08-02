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
import javax.xml.bind.JAXBElement;

import org.codehaus.enunciate.jaxrs.ResponseCode;
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

import org.opendaylight.controller.northbound.commons.exception.ResourceNotFoundException;
import org.opendaylight.controller.northbound.commons.exception.UnsupportedMediaTypeException;
import org.opendaylight.controller.sal.authorization.Privilege;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.HexEncode;
import org.opendaylight.controller.sal.utils.Status;

/**
 * Northbound REST APIs to handle virtual layer 2 bridges.
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
@Path("/{containerName}/vtns/{tenantName}/vbridges")
public class VBridgeNorthbound extends VTNNorthBoundBase {
    /**
     * Request URI information.
     */
    @Context
    private UriInfo  uriInfo;

    /**
     * Returns a list of all virtual L2 bridges in the virtual tenant.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the tenant.
     * @return  A list of virtual L2 bridges in the tenant.
     */
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(VBridgeList.class)
    @StatusCodes({
            @ResponseCode(code = 200, condition = "Operation successful"),
            @ResponseCode(code = 401, condition = "Authentication failed"),
            @ResponseCode(code = 404, condition = "The specified resource does not exist"),
            @ResponseCode(code = 500, condition = "Failed due to internal error"),
            @ResponseCode(code = 503, condition = "One or more of Controller Services are unavailable")})
    public VBridgeList getBridges(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName) {
        checkPrivilege(containerName, Privilege.READ);

        IVTNManager mgr = getVTNManager(containerName);
        VTenantPath path = new VTenantPath(tenantName);
        try {
            List<VBridge> list = mgr.getBridges(path);
            return new VBridgeList(list);
        } catch (VTNException e) {
            throw getException(e.getStatus());
        }
    }

    /**
     * Returns a virtual L2 bridge information specified by the given name.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the virtual tenant.
     * @param bridgeName     The name of the virtual bridge.
     * @return  L2 bridge information associated with the specified name.
     */
    @Path("{bridgeName}")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(VBridge.class)
    @StatusCodes({
            @ResponseCode(code = 200, condition = "Operation successful"),
            @ResponseCode(code = 401, condition = "Authentication failed"),
            @ResponseCode(code = 404, condition = "The specified resource does not exist"),
            @ResponseCode(code = 500, condition = "Failed due to internal error"),
            @ResponseCode(code = 503, condition = "One or more of Controller Services are unavailable")})
    public VBridge getBridge(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName) {
        checkPrivilege(containerName, Privilege.READ);

        IVTNManager mgr = getVTNManager(containerName);
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        try {
            return mgr.getBridge(path);
        } catch (VTNException e) {
            throw getException(e.getStatus());
        }
    }

    /**
     * Add a new virtual L2 bridge to the tenant.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the virtual tenant.
     * @param bridgeName     The name of the virtual bridge.
     * @param param          Virtual bridge configuration.
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{bridgeName}")
    @POST
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @StatusCodes({
            @ResponseCode(code = 201, condition = "Bridge created successfully"),
            @ResponseCode(code = 401, condition = "Authentication failed"),
            @ResponseCode(code = 404, condition = "The specified resource does not exist"),
            @ResponseCode(code = 406, condition = "Cannot operate on Default Container when other Containers are active"),
            @ResponseCode(code = 409, condition = "Failed to create bridge due to conflicting name"),
            @ResponseCode(code = 415, condition = "Invalid bridge name passed in bridgeName parameter"),
            @ResponseCode(code = 500, condition = "Failed to create bridge. Failure Reason included in HTTP Error response"),
            @ResponseCode(code = 503, condition = "One or more of Controller services are unavailable")})
    public Response addBridge(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @TypeHint(VBridgeConfig.class) JAXBElement<VBridgeConfig> param) {
        checkPrivilege(containerName, Privilege.WRITE);

        IVTNManager mgr = getVTNManager(containerName);
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        Status status = mgr.addBridge(path, param.getValue());
        if (status.isSuccess()) {
            return Response.status(Response.Status.CREATED).build();
        }

        throw getException(status);
    }

    /**
     * Modify configuration of existing virtual L2 bridge.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the virtual tenant.
     * @param bridgeName     The name of the virtual bridge.
     * @param all            If {@code true} is specified, all attributes
     *                       of the bridge are modified. {@code null} in
     *                       request body field is interpreted as default
     *                       value.
     *                       If {@code false} is specified, all fields to
     *                       which is assigned {@code null} are not modified.
     * @param param          Virtual bridge configuration.
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{bridgeName}")
    @PUT
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @StatusCodes({
            @ResponseCode(code = 200, condition = "Bridge modified successfully"),
            @ResponseCode(code = 401, condition = "Authentication failed"),
            @ResponseCode(code = 404, condition = "The specified resource does not exist"),
            @ResponseCode(code = 406, condition = "Cannot operate on Default Container when other Containers are active"),
            @ResponseCode(code = 500, condition = "Failed to modify bridge. Failure Reason included in HTTP Error response"),
            @ResponseCode(code = 503, condition = "One or more of Controller services are unavailable")})
    public Response modifyBridge(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @DefaultValue("false") @QueryParam("all") boolean all,
            @TypeHint(VBridgeConfig.class) JAXBElement<VBridgeConfig> param) {
        checkPrivilege(containerName, Privilege.WRITE);

        IVTNManager mgr = getVTNManager(containerName);
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        Status status = mgr.modifyBridge(path, param.getValue(), all);
        if (status.isSuccess()) {
            return Response.ok().build();
        }

        throw getException(status);
    }

    /**
     * Delete a virtual L2 bridge.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the virtual tenant.
     * @param bridgeName     The name of the virtual bridge.
     * @return Response as dictated by the HTTP Response code.
     */
    @Path("{bridgeName}")
    @DELETE
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @StatusCodes({
            @ResponseCode(code = 200, condition = "Bridge deleted successfully"),
            @ResponseCode(code = 401, condition = "Authentication failed"),
            @ResponseCode(code = 404, condition = "The specified resource does not exist"),
            @ResponseCode(code = 406, condition = "Cannot operate on Default Container when other Containers are active"),
            @ResponseCode(code = 500, condition = "Failed to delete bridge. Failure Reason included in HTTP Error response"),
            @ResponseCode(code = 503, condition = "One or more of Controller service is unavailable") })
    public Response deleteBridge(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName) {
        checkPrivilege(containerName, Privilege.WRITE);

        IVTNManager mgr = getVTNManager(containerName);
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        Status status = mgr.removeBridge(path);
        if (status.isSuccess()) {
            return Response.ok().build();
        }

        throw getException(status);
    }

    /**
     * Returns a list of all VLAN mappings in the virtual L2 bridge.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the tenant.
     * @param bridgeName     The name of the virtual bridge.
     * @return  A list of VLAN mappings in the bridge.
     */
    @Path("{bridgeName}/vlanmaps")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(VlanMapList.class)
    @StatusCodes({
            @ResponseCode(code = 200, condition = "Operation successful"),
            @ResponseCode(code = 401, condition = "Authentication failed"),
            @ResponseCode(code = 404, condition = "The specified resource does not exist"),
            @ResponseCode(code = 500, condition = "Failed due to internal error"),
            @ResponseCode(code = 503, condition = "One or more of Controller Services are unavailable")})
    public VlanMapList getVlanMaps(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName) {
        checkPrivilege(containerName, Privilege.READ);

        IVTNManager mgr = getVTNManager(containerName);
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        try {
            List<VlanMap> list = mgr.getVlanMaps(path);
            return new VlanMapList(list);
        } catch (VTNException e) {
            throw getException(e.getStatus());
        }
    }

    /**
     * Returns a VLAN mapping information specified by the given ID.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the virtual tenant.
     * @param bridgeName     The name of the virtual bridge.
     * @param mapId          The identifier of the VLAN mapping.
     * @return  VLAN mapping information associated with the specified ID.
     */
    @Path("{bridgeName}/vlanmaps/{mapId}")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(VlanMap.class)
    @StatusCodes({
            @ResponseCode(code = 200, condition = "Operation successful"),
            @ResponseCode(code = 401, condition = "Authentication failed"),
            @ResponseCode(code = 404, condition = "The specified resource does not exist"),
            @ResponseCode(code = 500, condition = "Failed due to internal error"),
            @ResponseCode(code = 503, condition = "One or more of Controller Services are unavailable")})
    public VlanMap getVlanMap(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @PathParam("mapId") String mapId) {
        checkPrivilege(containerName, Privilege.READ);

        IVTNManager mgr = getVTNManager(containerName);
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        try {
            return mgr.getVlanMap(path, mapId);
        } catch (VTNException e) {
            throw getException(e.getStatus());
        }
    }

    /**
     * Add a new VLAN mapping to the virtual L2 bridge.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the virtual tenant.
     * @param bridgeName     The name of the virtual bridge.
     * @param param          VLAN mapping configuration.
     * @return Response as dictated by the HTTP Response code.
     */
    @Path("{bridgeName}/vlanmaps")
    @POST
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @StatusCodes({
            @ResponseCode(code = 201, condition = "VLAN mapping created successfully"),
            @ResponseCode(code = 401, condition = "Authentication failed"),
            @ResponseCode(code = 404, condition = "The specified resource does not exist"),
            @ResponseCode(code = 406, condition = "Cannot operate on Default Container when other Containers are active"),
            @ResponseCode(code = 409, condition = "The specified VLAN ID is already mapped"),
            @ResponseCode(code = 415, condition = "Invalid configuration is specified"),
            @ResponseCode(code = 500, condition = "Failed to create mapping. Failure Reason included in HTTP Error response"),
            @ResponseCode(code = 503, condition = "One or more of Controller services are unavailable")})
    public Response addVlanMap(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @TypeHint(VlanMapConfig.class) JAXBElement<VlanMapConfig> param) {
        checkPrivilege(containerName, Privilege.WRITE);

        IVTNManager mgr = getVTNManager(containerName);
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        try {
            VlanMap vmap = mgr.addVlanMap(path, param.getValue());

            // Return CREATED with Location header.
            String id = vmap.getId();
            String uri = uriInfo.getAbsolutePath().toASCIIString();
            StringBuilder builder = new StringBuilder(uri);
            if (uri.charAt(uri.length() - 1) != '/') {
                builder.append('/');
            }
            builder.append(id);
            Response resp = Response.status(Response.Status.CREATED).
                header("Location", builder.toString()).build();
            return resp;
        } catch (VTNException e) {
            throw getException(e.getStatus());
        }
    }

    /**
     * Delete a VLAN mapping.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the virtual tenant.
     * @param bridgeName     The name of the virtual bridge.
     * @param mapId          The identifier of the VLAN mapping.
     * @return Response as dictated by the HTTP Response code.
     */
    @Path("{bridgeName}/vlanmaps/{mapId}")
    @DELETE
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @StatusCodes({
            @ResponseCode(code = 200, condition = "VLAN mapping deleted successfully"),
            @ResponseCode(code = 401, condition = "Authentication failed"),
            @ResponseCode(code = 404, condition = "The specified resource does not exist"),
            @ResponseCode(code = 406, condition = "Cannot operate on Default Container when other Containers are active"),
            @ResponseCode(code = 500, condition = "Failed to delete mapping. Failure Reason included in HTTP Error response"),
            @ResponseCode(code = 503, condition = "One or more of Controller service is unavailable") })
    public Response deleteBridge(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @PathParam("mapId") String mapId) {
        checkPrivilege(containerName, Privilege.WRITE);

        IVTNManager mgr = getVTNManager(containerName);
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        Status status = mgr.removeVlanMap(path, mapId);
        if (status.isSuccess()) {
            return Response.ok().build();
        }

        throw getException(status);
    }

    /**
     * Returns a list of MAC address table entries learned by the specified
     * virtual L2 bridge.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the tenant.
     * @param bridgeName     The name of the virtual bridge.
     * @return  A list of MAC address table entries.
     */
    @Path("{bridgeName}/mac")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(MacEntryList.class)
    @StatusCodes({
            @ResponseCode(code = 200, condition = "Operation successful"),
            @ResponseCode(code = 401, condition = "Authentication failed"),
            @ResponseCode(code = 404, condition = "The specified resource does not exist"),
            @ResponseCode(code = 500, condition = "Failed due to internal error"),
            @ResponseCode(code = 503, condition = "One or more of Controller Services are unavailable")})
    public MacEntryList getMacEntries(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName) {
        checkPrivilege(containerName, Privilege.READ);

        IVTNManager mgr = getVTNManager(containerName);
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        try {
            List<MacAddressEntry> list = mgr.getMacEntries(path);
            return new MacEntryList(list);
        } catch (VTNException e) {
            throw getException(e.getStatus());
        }
    }

    /**
     * Flush all MAC address table entries in the specified virtual L2 bridge.
     * virtual L2 bridge.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the tenant.
     * @param bridgeName     The name of the virtual bridge.
     * @return Response as dictated by the HTTP Response code.
     */
    @Path("{bridgeName}/mac")
    @DELETE
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @StatusCodes({
            @ResponseCode(code = 200, condition = "Operation successful"),
            @ResponseCode(code = 401, condition = "Authentication failed"),
            @ResponseCode(code = 404, condition = "The specified resource does not exist"),
            @ResponseCode(code = 500, condition = "Failed due to internal error"),
            @ResponseCode(code = 503, condition = "One or more of Controller Services are unavailable")})
    public Response flushMacEntries(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName) {
        checkPrivilege(containerName, Privilege.WRITE);

        IVTNManager mgr = getVTNManager(containerName);
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        Status status = mgr.flushMacEntries(path);
        if (status.isSuccess()) {
            return Response.ok().build();
        }

        throw getException(status);
    }

    /**
     * Returns a MAC address table entry specified by the given MAC address.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the tenant.
     * @param bridgeName     The name of the virtual bridge.
     * @param macAddr        String representation of MAC address.
     * @return  MAC address entry associated with the specified MAC address.
     */
    @Path("{bridgeName}/mac/{macAddr}")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(MacEntry.class)
    @StatusCodes({
            @ResponseCode(code = 200, condition = "Operation successful"),
            @ResponseCode(code = 401, condition = "Authentication failed"),
            @ResponseCode(code = 404, condition = "The specified resource does not exist"),
            @ResponseCode(code = 500, condition = "Failed due to internal error"),
            @ResponseCode(code = 503, condition = "One or more of Controller Services are unavailable")})
    public MacEntry getMacEntry(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @PathParam("macAddr") String macAddr) {
        checkPrivilege(containerName, Privilege.READ);

        // Parse the given MAC address as Ethernet address.
        EthernetAddress dladdr = parseEthernetAddress(macAddr);
        IVTNManager mgr = getVTNManager(containerName);
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        try {
            MacAddressEntry entry = mgr.getMacEntry(path, dladdr);
            if (entry == null) {
                throw new ResourceNotFoundException("MAC address not found");
            }
            return new MacEntry(entry);
        } catch (VTNException e) {
            throw getException(e.getStatus());
        }
    }

    /**
     * Remove a MAC address entry from the MAC address table in the virtual L2
     * bridge.
     *
     * @param containerName  The name of the container.
     * @param tenantName     The name of the tenant.
     * @param bridgeName     The name of the virtual bridge.
     * @param macAddr        String representation of MAC address.
     * @return Response as dictated by the HTTP Response code.
     */
    @Path("{bridgeName}/mac/{macAddr}")
    @DELETE
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @StatusCodes({
            @ResponseCode(code = 200, condition = "Operation successful"),
            @ResponseCode(code = 401, condition = "Authentication failed"),
            @ResponseCode(code = 404, condition = "The specified resource does not exist"),
            @ResponseCode(code = 500, condition = "Failed due to internal error"),
            @ResponseCode(code = 503, condition = "One or more of Controller Services are unavailable")})
    public Response removeMacEntry(
            @PathParam("containerName") String containerName,
            @PathParam("tenantName") String tenantName,
            @PathParam("bridgeName") String bridgeName,
            @PathParam("macAddr") String macAddr) {
        checkPrivilege(containerName, Privilege.WRITE);

        // Parse the given MAC address as Ethernet address.
        EthernetAddress dladdr = parseEthernetAddress(macAddr);
        IVTNManager mgr = getVTNManager(containerName);
        VBridgePath path = new VBridgePath(tenantName, bridgeName);
        try {
            if (mgr.removeMacEntry(path, dladdr) == null) {
                throw new ResourceNotFoundException("MAC address not found");
            }
            return Response.ok().build();
        } catch (VTNException e) {
            throw getException(e.getStatus());
        }
    }

    /**
     * Parse a string representation of Ethernet address.
     *
     * @param str  A string representation of Ethernet address.
     * @return  An {@code EthernetAddress} object.
     * @throws UnsupportedMediaTypeException
     *    Invalid string is passed to {@code str}.
     */
    private EthernetAddress parseEthernetAddress(String str) {
        try {
            byte[] b = HexEncode.bytesFromHexString(str);
            return new EthernetAddress(b);
        } catch (Exception e) {
            throw new UnsupportedMediaTypeException("Invalid MAC address");
        }
    }
}
