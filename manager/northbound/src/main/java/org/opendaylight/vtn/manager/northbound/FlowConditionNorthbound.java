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
import org.opendaylight.vtn.manager.flow.cond.FlowCondition;
import org.opendaylight.vtn.manager.flow.cond.FlowMatch;

import org.opendaylight.controller.sal.authorization.Privilege;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.utils.Status;

/**
 * This class provides Northbound REST APIs to handle flow condition in the
 * container.
 *
 * @since Helium
 */
@Path("/default/flowconditions")
public class FlowConditionNorthbound extends VTNNorthBoundBase {
    /**
     * Return information about flow conditions configured in the specified
     * container.
     *
     * @return  <strong>flowconditions</strong> element contains information
     *          about flow conditions specified by the requested URI.
     */
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(FlowConditionList.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public FlowConditionList getFlowConditions() {
        checkPrivilege(Privilege.READ);

        IVTNManager mgr = getVTNManager();
        try {
            return new FlowConditionList(mgr.getFlowConditions());
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Delete all the flow conditions configured in the default container.
     *
     * @return Response as dictated by the HTTP Response Status code.
     * @since  Lithium
     */
    @DELETE
    @TypeHint(TypeHint.NO_CONTENT.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "At least one flow condition was deleted " +
                      "successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "No flow condition is present."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public Response clearFlowCondition() {
        checkPrivilege(Privilege.WRITE);

        IVTNManager mgr = getVTNManager();
        Status status = mgr.clearFlowCondition();
        if (status == null) {
            return Response.noContent().build();
        }
        if (status.isSuccess()) {
            return Response.ok().build();
        }

        throw getException(status);
    }

    /**
     * Return information about the flow condition specified by the name
     * inside the default container.
     *
     * @param condName  The name of the flow condition.
     * @return  <strong>flowcondition</strong> element contains information
     *          about the flow condition specified by the requested URI.
     */
    @Path("{condName}")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(FlowCondition.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "The specified flow condition does not " +
                      "exist."),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public FlowCondition getFlowCondition(
            @PathParam("condName") String condName) {
        checkPrivilege(Privilege.READ);

        IVTNManager mgr = getVTNManager();
        try {
            return mgr.getFlowCondition(condName);
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Create or modify the flow condition specified by the name inside the
     * default container.
     *
     * <ul>
     *   <li>
     *     If the flow condition specified by
     *     <span style="text-decoration: underline;">{condName}</span> does
     *     not exist, a new flow condition will be associated with
     *     <span style="text-decoration: underline;">{condName}</span> in the
     *     default container.
     *   </li>
     *   <li>
     *     If the flow condition specified by
     *     <span style="text-decoration: underline;">{condName}</span> already
     *     exists, it will be modified as specified by
     *     <strong>flowcondition</strong> element.
     *   </li>
     * </ul>
     *
     * @param uriInfo  Requested URI information.
     * @param condName
     *   The name of the flow condition.
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
     * @param fcond
     *   <strong>flowcondition</strong> element specifies the contents of the
     *   flow condition.
     *   <ul>
     *     <li>
     *       <strong>name</strong> attribute in the
     *       <strong>flowcondition</strong> element is always ignored.
     *       The name of the flow condition is determined by the
     *       <span style="text-decoration: underline;">{condName}</span>
     *       parameter.
     *     </li>
     *     <li>
     *       Each <strong>match</strong> element in <strong>matches</strong>
     *       must have a unique match index in the <strong>index</strong>
     *       attribute.
     *     </li>
     *     <li>
     *       The flow condition will match every packet if no
     *       <strong>match</strong> element is configured in
     *       <strong>flowcondition</strong> element.
     *     </li>
     *   </ul>
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{condName}")
    @PUT
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(TypeHint.NO_CONTENT.class)
    @ResponseHeaders({
        @ResponseHeader(name = "Location",
                        description = "URI corresponding to the newly " +
                        "created flow condition, which is the same URI " +
                        "specified in request. This header is set only " +
                        "if CREATED(201) is returned as response code.")})
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Existing flow condition was modified " +
                      "successfully."),
        @ResponseCode(code = HTTP_CREATED,
                      condition = "Flow condition was newly created " +
                      "successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "Flow condition was not changed."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "<ul>" +
                      "<li>Incorrect XML or JSON data is specified " +
                      "in Request body.</li>" +
                      "<li>Incorrect flow condition name is specified to " +
                      "<u>{condName}</u>.</li>" +
                      "<li>Incorrect value is configured in " +
                      "<strong>flowcondition</strong>.</li>" +
                      "<li>Duplicate match index is configured in " +
                      "<strong>match</strong> element in " +
                      "<strong>flowcondition</strong> element.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_UNSUPPORTED_TYPE,
                      condition = "Unsupported data type is specified in " +
                      "<strong>Content-Type</strong> header."),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public Response putFlowCondition(
            @Context UriInfo uriInfo,
            @PathParam("condName") String condName,
            @TypeHint(FlowCondition.class) FlowCondition fcond) {
        checkPrivilege(Privilege.WRITE);

        IVTNManager mgr = getVTNManager();
        try {
            UpdateType result = mgr.setFlowCondition(condName, fcond);
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
     * Delete the flow condition specified by the name inside the default
     * container.
     *
     * @param condName  The name of the flow condition.
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{condName}")
    @DELETE
    @TypeHint(TypeHint.NO_CONTENT.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Flow condition was deleted successfully."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "The specified flow condition does not " +
                      "exist."),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public Response deleteFlowCondition(
            @PathParam("condName") String condName) {
        checkPrivilege(Privilege.WRITE);

        IVTNManager mgr = getVTNManager();
        Status status = mgr.removeFlowCondition(condName);
        if (status.isSuccess()) {
            return Response.ok().build();
        }

        throw getException(status);
    }

    /**
     * Return information about the specified flow match condition configured
     * in the specified flow condition.
     *
     * @param condName  The name of the flow condition.
     * @param index
     *    The match index that specifies the flow match condition.
     *    A string representation of an integer value must be specified.
     * @return  <strong>flowcondition</strong> element contains information
     *          about the flow condition specified by the requested URI.
     */
    @Path("{condName}/{index}")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(FlowMatch.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "No flow match condition is associated " +
                      "with the match index specified by <u>{index}</u>."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified flow condition does not exist.</li>" +
                      "<li>A string passed to <u>{index}</u> can not be " +
                      "converted into an integer.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public FlowMatch getFlowConditionMatch(
            @PathParam("condName") String condName,
            @PathParam("index") int index) {
        checkPrivilege(Privilege.READ);

        IVTNManager mgr = getVTNManager();
        try {
            return mgr.getFlowConditionMatch(condName, index);
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Configure a flow match condition into the specified flow condition
     * inside the default container.
     *
     * <ul>
     *   <li>
     *     If no flow match condition is associated with the match index
     *     specified by
     *     <span style="text-decoration: underline;">{index}</span> parameter
     *     in the flow condition, a new flow match condition will be
     *     associated with
     *    <span style="text-decoration: underline;">{index}</span> in the
     *    flow condition.
     *   </li>
     *   <li>
     *     If the flow match condition is already associated with the match
     *     index specified by
     *     <span style="text-decoration: underline;">{index}</span> parameter
     *     in the flow condition, the contents of the specified flow match
     *     condition will be modified as specified by
     *     <strong>flowmatch</strong> element.
     *   </li>
     * </ul>
     *
     * @param uriInfo   Requested URI information.
     * @param condName  The name of the flow condition.
     * @param index
     *   The match index that specifies flow match condition in the flow
     *   condition.
     *   <ul>
     *     <li>
     *       A string representation of an integer value must be specified.
     *     </li>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>1</strong> to <strong>65535</strong>.
     *     </li>
     *   </ul>
     * @param match
     *   <strong>flowmatch</strong> element specifies the contents of the
     *   flow match condition.
     *   <ul>
     *     <li>
     *       <strong>index</strong> attribute in the <strong>flowmatch</strong>
     *       element is always ignored.
     *       The match index is determined by the
     *       <span style="text-decoration: underline;">{index}</span> parameter.
     *     </li>
     *     <li>
     *       If no condition to match packets is configured in
     *       <strong>flowmatch</strong> element, it will match every packet.
     *     </li>
     *   </ul>
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{condName}/{index}")
    @PUT
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(TypeHint.NO_CONTENT.class)
    @ResponseHeaders({
        @ResponseHeader(name = "Location",
                        description = "URI corresponding to the newly " +
                        "created flow match condition, which is the same " +
                        "URI specified in request. This header is set only " +
                        "if CREATED(201) is returned as response code.")})
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Existing flow match condition was " +
                      "modified successfully."),
        @ResponseCode(code = HTTP_CREATED,
                      condition = "Flow match condition was newly created " +
                      "in the flow condition successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "Flow match condition was not changed."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "<ul>" +
                      "<li>Incorrect XML or JSON data is specified " +
                      "in Request body.</li>" +
                      "<li>Incorrect flow condition name is specified to " +
                      "<u>{condName}</u>.</li>" +
                      "<li>Match index specified by <u>{index}</u> " +
                      "parameter is out of valid range.</li>" +
                      "<li>Incorrect value is configured in " +
                      "<strong>flowmatch</strong>.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified flow condition does not exist.</li>" +
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
    public Response putFlowConditionMatch(
            @Context UriInfo uriInfo,
            @PathParam("condName") String condName,
            @PathParam("index") int index,
            @TypeHint(FlowMatch.class) FlowMatch match) {
        checkPrivilege(Privilege.WRITE);

        IVTNManager mgr = getVTNManager();
        try {
            UpdateType result =
                mgr.setFlowConditionMatch(condName, index, match);
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
     * Delete the specified flow match condition in the flow condition.
     *
     * @param condName  The name of the flow condition.
     * @param index
     *    The match index that specifies the flow match condition.
     *    A string representation of an integer value must be specified.
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{condName}/{index}")
    @DELETE
    @TypeHint(TypeHint.NO_CONTENT.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Flow match condition was deleted " +
                      "successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "No flow match condition is associated " +
                      "with the match index specified by <u>{index}</u>."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified flow condition does not exist.</li>" +
                      "<li>A string passed to <u>{index}</u> can not be " +
                      "converted into an integer.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public Response deleteFlowConditionMatch(
            @PathParam("condName") String condName,
            @PathParam("index") int index) {
        checkPrivilege(Privilege.WRITE);

        IVTNManager mgr = getVTNManager();
        Status status = mgr.removeFlowConditionMatch(condName, index);
        if (status == null) {
            return Response.noContent().build();
        }
        if (status.isSuccess()) {
            return Response.ok().build();
        }

        throw getException(status);
    }
}
