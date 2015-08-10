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

import org.opendaylight.vtn.manager.IVTNManager;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.PathMap;

import org.opendaylight.controller.sal.authorization.Privilege;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.utils.Status;

/**
 * This class provides Northbound REST APIs to handle container path map.
 *
 * <p>
 *   Path maps configured in the container path map list affect flows in all
 *   VTNs in the container, but the VTN path maps are always evaluated prior
 *   to the container path maps. Container path maps will be evaluated only
 *   if a packet does not match any VTN path map.
 * </p>
 *
 * @since Helium
 */
@Path("/default/pathmaps")
public class ContainerPathMapNorthbound extends VTNNorthBoundBase {
    /**
     * Return information about container path map configured in the specified
     * container.
     *
     * @return  <strong>pathmaps</strong> element contains information
     *          about container path map list specified by the requested URI.
     */
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(PathMapList.class)
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
    public PathMapList getPathMaps() {
        checkPrivilege(Privilege.READ);

        IVTNManager mgr = getVTNManager();
        try {
            return new PathMapList(mgr.getPathMaps());
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Delete all the container path maps in the default container.
     *
     * @return Response as dictated by the HTTP Response Status code.
     * @since  Lithium
     */
    @DELETE
    @TypeHint(TypeHint.NO_CONTENT.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "At least one container path map was " +
                      "deleted successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "No container path map is present."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public Response clearPathMap() {
        checkPrivilege(Privilege.WRITE);

        IVTNManager mgr = getVTNManager();
        Status status = mgr.clearPathMap();
        if (status == null) {
            return Response.noContent().build();
        }
        if (status.isSuccess()) {
            return Response.ok().build();
        }

        throw getException(status);
    }

    /**
     * Return information about the container path map specified by the
     * path map index inside the default container.
     *
     * @param index
     *   The index value which specifies the path map in the container path
     *   map list.
     *   A string representation of an integer value must be specified.
     * @return  <strong>pathmap</strong> element contains information
     *          about the path map specified by the requested URI.
     */
    @Path("{index}")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(PathMap.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "The specified path map does not exist " +
                      "in the default container."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "A string passed to <u>{index}</u> can " +
                      "not be converted into an integer."),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public PathMap getPathMap(@PathParam("index") int index) {
        checkPrivilege(Privilege.READ);

        IVTNManager mgr = getVTNManager();
        try {
            return mgr.getPathMap(index);
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Create or modify the container path map specified by the index number
     * inside the default container.
     *
     * <ul>
     *   <li>
     *     If the container path map specified by
     *     <span style="text-decoration: underline;">{index}</span> does not
     *     exist, a new container path map will be associated with
     *     <span style="text-decoration: underline;">{index}</span> in the
     *     container path map list.
     *   </li>
     *   <li>
     *     If the container path map specified by
     *     <span style="text-decoration: underline;">{index}</span> already
     *     exists, it will be modified as specified by <strong>pathmap</strong>
     *     element.
     *   </li>
     * </ul>
     *
     * @param uriInfo  Requested URI information.
     * @param index
     *   The index value which specifies the path map in the container path
     *   map list.
     *   <ul>
     *     <li>
     *       A string representation of an integer value must be specified.
     *     </li>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>1</strong> to <strong>65535</strong>.
     *     </li>
     *   </ul>
     * @param pmap
     *   <strong>pathmap</strong> element specifies the configuration of the
     *   path map.
     *   <ul>
     *     <li>
     *       <strong>index</strong> attribute in the <strong>pathmap</strong>
     *       element is always ignored. The index number is determined by the
     *       <span style="text-decoration: underline;">{index}</span>
     *       parameter.
     *     </li>
     *     <li>
     *       Note that this API does not check whether the flow condition
     *       specified by <strong>condition</strong> attribute in
     *       <strong>pathmap</strong> element actually exists or not.
     *       The path map will be invalidated if the specified flow condition
     *       does not exist.
     *     </li>
     *     <li>
     *       Note that this API does not check whether the path policy
     *       specified by <strong>policy</strong> attribute in
     *       <strong>pathmap</strong> element actually exists or not.
     *       The path map will be invalidated if the specified path policy
     *       does not exist.
     *     </li>
     *   </ul>
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{index}")
    @PUT
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(TypeHint.NO_CONTENT.class)
    @ResponseHeaders({
        @ResponseHeader(name = "Location",
                        description = "URI corresponding to the newly " +
                        "created path map, which is the same URI specified " +
                        "in request. This header is set only if " +
                        "CREATED(201) is returned as response code.")})
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Existing path map was modified " +
                      "successfully."),
        @ResponseCode(code = HTTP_CREATED,
                      condition = "Path map was newly created " +
                      "successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "Path map was not changed."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "<ul>" +
                      "<li>Incorrect XML or JSON data is specified " +
                      "in Request body.</li>" +
                      "<li>Index number specified by <u>{index}</u> " +
                      "parameter is out of valid range.</li>" +
                      "<li>Incorrect value is configured in " +
                      "<strong>pathmap</strong>.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "A string passed to <u>{index}</u> can " +
                      "not be converted into an integer."),
        @ResponseCode(code = HTTP_UNSUPPORTED_TYPE,
                      condition = "Unsupported data type is specified in " +
                      "<strong>Content-Type</strong> header."),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public Response putPathMap(
            @Context UriInfo uriInfo,
            @PathParam("index") int index,
            @TypeHint(PathMap.class) PathMap pmap) {
        checkPrivilege(Privilege.WRITE);

        IVTNManager mgr = getVTNManager();
        try {
            UpdateType result = mgr.setPathMap(index, pmap);
            if (result == null) {
                return Response.noContent().build();
            }
            if (result == UpdateType.ADDED) {
                // Return CREATED with Location header.
                return Response.created(uriInfo.getAbsolutePath()).build();
            }

            return Response.ok().build();
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Delete the container path map specified by the index number inside the
     * default container.
     *
     * @param index
     *   The index value which specifies the path map in the container path
     *   map list.
     *   A string representation of an integer value must be specified.
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{index}")
    @DELETE
    @TypeHint(TypeHint.NO_CONTENT.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Path map was deleted successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "The specified path map does not exist " +
                      "in the default container."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "A string passed to <u>{index}</u> can " +
                      "not be converted into an integer."),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public Response deletePathMap(
            @PathParam("index") int index) {
        checkPrivilege(Privilege.WRITE);

        IVTNManager mgr = getVTNManager();
        Status status = mgr.removePathMap(index);
        if (status == null) {
            return Response.noContent().build();
        }
        if (status.isSuccess()) {
            return Response.ok().build();
        }

        throw getException(status);
    }
}
