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
import org.opendaylight.vtn.manager.PathPolicy;
import org.opendaylight.vtn.manager.PortLocation;
import org.opendaylight.vtn.manager.SwitchPort;

import org.opendaylight.controller.northbound.commons.exception.
    BadRequestException;
import org.opendaylight.controller.sal.authorization.Privilege;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.utils.Status;

/**
 * This class provides Northbound REST APIs to handle path policy in the
 * container.
 *
 * <p>
 *   The default policy of packet routing in the OpenDaylight controller is
 *   "shortest-path". When the VTN Manager configures a unicast flow with
 *   default path policy, it chooses the packet route which minimizes the
 *   number of hops. The cost of using each link between switches is not
 *   considered.
 * </p>
 * <p>
 *   "Path Policy" feature implements cost-based packet routing.
 *   A path policy is a set of user-defined cost of using the link for
 *   transmission. It can specify the cost of using specific switch link.
 *   If a path policy is applied for packet routing, the VTN Manager chooses
 *   the packet route which minimizes the total cost of switch links.
 *   Flow condition feature is used to determine path policy to be applied
 *   for packet routing.
 * </p>
 *
 * @since Helium
 */
@Path("/default/pathpolicies")
public class PathPolicyNorthbound extends VTNNorthBoundBase {
    /**
     * Return a list of path policy identifiers configured in the default
     * container.
     *
     * @return  <strong>integers</strong> element contains integer values
     *          which identify path policies in the default container.
     */
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(XmlLongIntegerList.class)
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
    public XmlLongIntegerList getPathPolicyIds() {
        checkPrivilege(Privilege.READ);

        IVTNManager mgr = getVTNManager();
        try {
            return new XmlLongIntegerList(mgr.getPathPolicyIds());
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Delete all the path policies configured in the default container.
     *
     * @return Response as dictated by the HTTP Response Status code.
     * @since  Lithium
     */
    @DELETE
    @TypeHint(TypeHint.NO_CONTENT.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "At least one path policy was deleted " +
                      "successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "No path policy is present."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public Response clearPathPolicy() {
        checkPrivilege(Privilege.WRITE);

        IVTNManager mgr = getVTNManager();
        Status status = mgr.clearPathPolicy();
        if (status == null) {
            return Response.noContent().build();
        }
        if (status.isSuccess()) {
            return Response.ok().build();
        }

        throw getException(status);
    }

    /**
     * Return information about the path policy specified by the path policy
     * ID inside the default container.
     *
     * @param policyId  The identifier of the path policy.
     * @return  <strong>pathpolicy</strong> element contains information
     *          about the path policy specified by the requested URI.
     */
    @Path("{policyId}")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(PathPolicy.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified path policy does not exist.</li>" +
                      "<li>A string passed to <u>{policyId}</u> can not be " +
                      "converted into an integer.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public PathPolicy getPathPolicy(@PathParam("policyId") int policyId) {
        checkPrivilege(Privilege.READ);

        IVTNManager mgr = getVTNManager();
        try {
            return mgr.getPathPolicy(policyId);
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Create or modify the path policy specified by the path policy ID
     * inside the default container.
     *
     * <ul>
     *   <li>
     *     If the path policy specified by
     *     <span style="text-decoration: underline;">{policyId}</span> does
     *     not exist, a new path policy will be associated with
     *     <span style="text-decoration: underline;">{policyId}</span> in the
     *     default container.
     *   </li>
     *   <li>
     *     If the path policy specified by
     *     <span style="text-decoration: underline;">{policyId}</span> already
     *     exists, it will be modified as specified by
     *     <strong>pathpolicy</strong> element.
     *   </li>
     * </ul>
     *
     * @param uriInfo   Requested URI information.
     * @param policyId
     *   The identifier of the path policy.
     *   <p>
     *     The range of value that can be specified is from
     *     <strong>1</strong> to <strong>3</strong>.
     *   </p>
     * @param policy
     *   <strong>pathpolicy</strong> element specifies the configuration of
     *   the path policy.
     *   <ul>
     *     <li>
     *       <strong>id</strong> attribute in the <strong>pathpolicy</strong>
     *       element is always ignored.
     *       The identifier of the path policy is determined by the
     *       <span style="text-decoration: underline;">{policyId}</span>
     *       parameter.
     *     </li>
     *     <li>
     *       Each <strong>cost</strong> element in the <strong>costs</strong>
     *       element can not omit <strong>location</strong> element which
     *       specifies the location of the switch port, and it must be unique
     *       in the <strong>costs</strong> element.
     *     </li>
     *   </ul>
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{policyId}")
    @PUT
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(TypeHint.NO_CONTENT.class)
    @ResponseHeaders({
        @ResponseHeader(name = "Location",
                        description = "URI corresponding to the newly " +
                        "created path policy, which is the same URI " +
                        "specified in request. This header is set only " +
                        "if CREATED(201) is returned as response code.")})
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Existing path policy was modified " +
                      "successfully."),
        @ResponseCode(code = HTTP_CREATED,
                      condition = "Path policy was newly created " +
                      "successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "Path policy was not changed."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "<ul>" +
                      "<li>Incorrect XML or JSON data is specified " +
                      "in Request body.</li>" +
                      "<li>Invalid path policy identifier is specified to " +
                      "<u>{policyId}</u>.</li>" +
                      "<li>Incorrect value is configured in " +
                      "<strong>pathpolicy</strong> element.</li>" +
                      "<li>Duplicate location of switch port is configured " +
                      "in <strong>cost</strong> element in " +
                      "<strong>pathpolicy</strong> element.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "A string passed to <u>{policyId}</u> can " +
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
    public Response putPathPolicy(
            @Context UriInfo uriInfo,
            @PathParam("policyId") int policyId,
            @TypeHint(PathPolicy.class) PathPolicy policy) {
        checkPrivilege(Privilege.WRITE);

        IVTNManager mgr = getVTNManager();
        try {
            UpdateType result = mgr.setPathPolicy(policyId, policy);
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
     * Delete the path policy specified by the path policy identifier inside
     * the default container.
     *
     * @param policyId  The identifier of the path policy.
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{policyId}")
    @DELETE
    @TypeHint(TypeHint.NO_CONTENT.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Path policy was deleted successfully."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified flow condition does not exist.</li>" +
                      "<li>A string passed to <u>{policyId}</u> can not be " +
                      "converted into an integer.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public Response deletePathPolicy(@PathParam("policyId") int policyId) {
        checkPrivilege(Privilege.WRITE);

        IVTNManager mgr = getVTNManager();
        Status status = mgr.removePathPolicy(policyId);
        if (status.isSuccess()) {
            return Response.ok().build();
        }

        throw getException(status);
    }

    /**
     * Return the default link cost configured in the specified path policy.
     *
     * @param policyId  The identifier of the path policy.
     * @return
     *   <strong>integer</strong> element contains the default link cost
     *   configured in the path policy specified by the requested URI.
     *   <ul>
     *     <li>
     *       A long integer value more than <strong>0</strong> represents
     *       the static default link cost configured in the path policy.
     *     </li>
     *     <li>
     *       <strong>0</strong> means that the default link cost for the
     *       specified path policy is determined individually by the link speed
     *       configured to a switch port.
     *       See documents of <strong>pathpolicy</strong> element for
     *       more defails.
     *     </li>
     *   </ul>
     */
    @Path("{policyId}/default")
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
                      "<li>The specified path policy does not exist.</li>" +
                      "<li>A string passed to <u>{policyId}</u> can not be " +
                      "converted into an integer.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public XmlLongInteger getPathPolicyDefaultCost(
            @PathParam("policyId") int policyId) {
        checkPrivilege(Privilege.READ);

        IVTNManager mgr = getVTNManager();
        try {
            return new XmlLongInteger(mgr.getPathPolicyDefaultCost(policyId));
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Change the default link cost configured in the specified path policy.
     *
     * @param policyId  The identifier of the path policy.
     * @param cost
     *   <strong>integer</strong> element specifies the default link cost
     *   to be configured.
     *   <ul>
     *     <li>
     *       If a long integer value more than <strong>0</strong> is specified,
     *       it will be configured as the static default link cost.
     *     </li>
     *     <li>
     *       If <strong>0</strong> is specified, the default link cost of
     *       using a switch port will be defined individually by the link speed
     *       configured to a switch port.
     *       See documents of <strong>pathpolicy</strong> element for
     *       more defails.
     *     </li>
     *   </ul>
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{policyId}/default")
    @PUT
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(TypeHint.NO_CONTENT.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "The default link cost of the path policy " +
                      "was changed successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "The default link cost of the path policy " +
                      "was not changed."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "<ul>" +
                      "<li>Incorrect XML or JSON data is specified " +
                      "in Request body.</li>" +
                      "<li>Invalid link cost is configured in " +
                      "<strong>integer</strong> element.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified path policy does not exist.</li>" +
                      "<li>A string passed to <u>{policyId}</u> can not be " +
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
    public Response putPathPolicyDefaultCost(
            @PathParam("policyId") int policyId,
            @TypeHint(XmlLongInteger.class) XmlLongInteger cost) {
        checkPrivilege(Privilege.WRITE);

        IVTNManager mgr = getVTNManager();
        try {
            Long c = cost.getValue();
            if (c == null) {
                throw new BadRequestException("Link cost must be specified.");
            }

            if (mgr.setPathPolicyDefaultCost(policyId, c.longValue())) {
                return Response.ok().build();
            }

            return Response.noContent().build();
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Return the link cost associated with the specified switch port location
     * in the specified path policy.
     *
     * <p>
     *   This API retrieves the link cost associated with the switch port
     *   location which specifies only a physical switch.
     *   Returned link cost will be applied to all ports within the specified
     *   switch unless it is overridden by more exact switch port location.
     * </p>
     * <p>
     *   See document of <strong>pathpolicy</strong> element for more details.
     * </p>
     *
     * @param policyId  The identifier of the path policy.
     * @param nodeType
     *   Type of the node corresponding to the physical switch.
     *    <ul>
     *      <li>
     *        Specify <strong>"OF"</strong> for OpenFlow switch.
     *      </li>
     *    </ul>
     * @param nodeId
     *   A string which represents identifier of the node corresponding to
     *   the physical switch.
     *    <ul>
     *      <li>
     *        Specify a string representation of the DPID for OpenFlow switch.
     *        (e.g. <strong>00:11:22:33:44:55:66:77</strong>)
     *      </li>
     *    </ul>
     * @return
     *   <strong>integer</strong> element contains the link cost associated
     *   with the specified switch port location in the path policy.
     */
    @Path("{policyId}/costs/{nodeType}/{nodeId}")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(XmlLongInteger.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "The link cost is not associated with the " +
                      "physical switch."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "An invalid pair of node type and ID is " +
                      "passed to <u>{nodeType}</u> and <u>{nodeId}</u> " +
                      "respectively."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified path policy does not exist.</li>" +
                      "<li>A string passed to <u>{policyId}</u> can not be " +
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
    public XmlLongInteger getCost(
            @PathParam("policyId") int policyId,
            @PathParam("nodeType") String nodeType,
            @PathParam("nodeId") String nodeId) {
        return getCostImpl(policyId, nodeType, nodeId, null, null, null);
    }

    /**
     * Associate the link cost with the specified switch port location
     * in the specified path policy.
     *
     * <p>
     *   This API associates the link cost with the switch port location
     *   which specifies only a physical switch. The specified link cost
     *   will be applied when a packet is forwarded from a switch port within
     *   the specified switch, unless it is overridden by more exact switch
     *   location.
     * </p>
     * <p>
     *   See document of <strong>pathpolicy</strong> element for more details.
     * </p>
     *
     * @param uriInfo   Requested URI information.
     * @param policyId  The identifier of the path policy.
     * @param nodeType
     *   Type of the node corresponding to the physical switch.
     *    <ul>
     *      <li>
     *        Specify <strong>"OF"</strong> for OpenFlow switch.
     *      </li>
     *    </ul>
     * @param nodeId
     *   A string which represents identifier of the node corresponding to
     *   the physical switch.
     *    <ul>
     *      <li>
     *        Specify a string representation of the DPID for OpenFlow switch.
     *        (e.g. <strong>00:11:22:33:44:55:66:77</strong>)
     *      </li>
     *    </ul>
     * @param cost
     *   <strong>integer</strong> element specifies the link cost.
     *   <ul>
     *     <li>
     *       A long integer value more than <strong>0</strong> must be
     *       specified.
     *     </li>
     *   </ul>
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{policyId}/costs/{nodeType}/{nodeId}")
    @PUT
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(TypeHint.NO_CONTENT.class)
    @ResponseHeaders({
        @ResponseHeader(name = "Location",
                        description = "URI corresponding to the newly " +
                        "added the link cost configuration for the switch " +
                        "port location, which is the same URI specified " +
                        "in request. This header is set only if " +
                        "CREATED(201) is returned as response code.")})
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "The link cost associated with existing " +
                      "switch port location in the path policy was changed " +
                      "successfully."),
        @ResponseCode(code = HTTP_CREATED,
                      condition = "The link cost was newly associated with " +
                      "the specified switch port location in the path " +
                      "policy."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "The link cost for the specified switch " +
                      "port location was not changed."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "<ul>" +
                      "<li>Incorrect XML or JSON data is specified " +
                      "in Request body.</li>" +
                      "<li>An invalid pair of node type and ID is passed to " +
                      "<u>{nodeType}</u> and <u>{nodeId}</u> " +
                      "respectively.</li>" +
                      "<li>Invalid link cost is configured in " +
                      "<strong>integer</strong> element.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified path policy does not exist.</li>" +
                      "<li>A string passed to <u>{policyId}</u> can not be " +
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
    public Response putCost(
            @Context UriInfo uriInfo,
            @PathParam("policyId") int policyId,
            @PathParam("nodeType") String nodeType,
            @PathParam("nodeId") String nodeId,
            @TypeHint(XmlLongInteger.class) XmlLongInteger cost) {
        return putCostImpl(uriInfo, policyId, nodeType, nodeId, null, null,
                           null, cost);
    }

    /**
     * Detete the link cost for the specified switch port location
     * in the specified path policy.
     *
     * <p>
     *   This API delete the link cost associated with the switch port location
     *   which specifies only a physical switch.
     * </p>
     * <p>
     *   See document of <strong>pathpolicy</strong> element for more details.
     * </p>
     *
     * @param policyId  The identifier of the path policy.
     * @param nodeType
     *   Type of the node corresponding to the physical switch.
     *    <ul>
     *      <li>
     *        Specify <strong>"OF"</strong> for OpenFlow switch.
     *      </li>
     *    </ul>
     * @param nodeId
     *   A string which represents identifier of the node corresponding to
     *   the physical switch.
     *    <ul>
     *      <li>
     *        Specify a string representation of the DPID for OpenFlow switch.
     *        (e.g. <strong>00:11:22:33:44:55:66:77</strong>)
     *      </li>
     *    </ul>
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{policyId}/costs/{nodeType}/{nodeId}")
    @DELETE
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(TypeHint.NO_CONTENT.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "The link cost associated with the " +
                      "specifeid switch port location in the path policy " +
                      "was deleted successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "The link cost for the specified switch " +
                      "port location is not configured in the path policy."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "An invalid pair of node type and ID is " +
                      "passed to <u>{nodeType}</u> and <u>{nodeId}</u> " +
                      "respectively."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified path policy does not exist.</li>" +
                      "<li>A string passed to <u>{policyId}</u> can not be " +
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
    public Response deleteCost(
            @PathParam("policyId") int policyId,
            @PathParam("nodeType") String nodeType,
            @PathParam("nodeId") String nodeId) {
        return deleteCostImpl(policyId, nodeType, nodeId, null, null, null);
    }

    /**
     * Return the link cost associated with the specified switch port location
     * in the specified path policy.
     *
     * <p>
     *   This API retrieves the link cost associated with the switch port
     *   location which specifies a physical switch and the name of the
     *   switch port. Returned link cost will be applied to the specified
     *   switch port unless it is overridden by more exact switch port
     *   location.
     * </p>
     * <p>
     *   See document of <strong>pathpolicy</strong> element for more details.
     * </p>
     *
     * @param policyId  The identifier of the path policy.
     * @param nodeType
     *   Type of the node corresponding to the physical switch.
     *    <ul>
     *      <li>
     *        Specify <strong>"OF"</strong> for OpenFlow switch.
     *      </li>
     *    </ul>
     * @param nodeId
     *   A string which represents identifier of the node corresponding to
     *   the physical switch.
     *    <ul>
     *      <li>
     *        Specify a string representation of the DPID for OpenFlow switch.
     *        (e.g. <strong>00:11:22:33:44:55:66:77</strong>)
     *      </li>
     *    </ul>
     * @param portName
     *   The name of the switch port within the physical switch specified by
     *   <span style="text-decoration: underline;">{nodeType}</span> and
     *   <span style="text-decoration: underline;">{nodeId}</span>.
     * @return
     *   <strong>integer</strong> element contains the link cost associated
     *   with the specified switch port location in the path policy.
     */
    @Path("{policyId}/costs/{nodeType}/{nodeId}/name/{portName}")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(XmlLongInteger.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "The link cost is not associated with the " +
                      "specified switch port."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "An invalid pair of node type and ID is " +
                      "passed to <u>{nodeType}</u> and <u>{nodeId}</u> " +
                      "respectively."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified path policy does not exist.</li>" +
                      "<li>A string passed to <u>{policyId}</u> can not be " +
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
    public XmlLongInteger getCost(
            @PathParam("policyId") int policyId,
            @PathParam("nodeType") String nodeType,
            @PathParam("nodeId") String nodeId,
            @PathParam("portName") String portName) {
        return getCostImpl(policyId, nodeType, nodeId, portName, null, null);
    }

    /**
     * Associate the link cost with the specified switch port location
     * in the specified path policy.
     *
     * <p>
     *   This API associates the link cost with the switch port location
     *   which specifies a physical switch and the name of the switch port.
     *   The specified link cost will be applied when a packet is forwarded
     *   from the specified switch port unless it is overridden by more exact
     *   switch location.
     * </p>
     * <p>
     *   See document of <strong>pathpolicy</strong> element for more details.
     * </p>
     *
     * @param uriInfo   Requested URI information.
     * @param policyId  The identifier of the path policy.
     * @param nodeType
     *   Type of the node corresponding to the physical switch.
     *    <ul>
     *      <li>
     *        Specify <strong>"OF"</strong> for OpenFlow switch.
     *      </li>
     *    </ul>
     * @param nodeId
     *   A string which represents identifier of the node corresponding to
     *   the physical switch.
     *    <ul>
     *      <li>
     *        Specify a string representation of the DPID for OpenFlow switch.
     *        (e.g. <strong>00:11:22:33:44:55:66:77</strong>)
     *      </li>
     *    </ul>
     * @param portName
     *   The name of the switch port within the physical switch specified by
     *   <span style="text-decoration: underline;">{nodeType}</span> and
     *   <span style="text-decoration: underline;">{nodeId}</span>.
     * @param cost
     *   <strong>integer</strong> element specifies the link cost.
     *   <ul>
     *     <li>
     *       A long integer value more than <strong>0</strong> must be
     *       specified.
     *     </li>
     *   </ul>
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{policyId}/costs/{nodeType}/{nodeId}/name/{portName}")
    @PUT
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(TypeHint.NO_CONTENT.class)
    @ResponseHeaders({
        @ResponseHeader(name = "Location",
                        description = "URI corresponding to the newly " +
                        "added the link cost configuration for the switch " +
                        "port location, which is the same URI specified " +
                        "in request. This header is set only if " +
                        "CREATED(201) is returned as response code.")})
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "The link cost associated with existing " +
                      "switch port location in the path policy was changed " +
                      "successfully."),
        @ResponseCode(code = HTTP_CREATED,
                      condition = "The link cost was newly associated with " +
                      "the specified switch port location in the path " +
                      "policy."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "The link cost for the specified switch " +
                      "port location was not changed."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "<ul>" +
                      "<li>Incorrect XML or JSON data is specified " +
                      "in Request body.</li>" +
                      "<li>Invalid path policy identifier is specified to " +
                      "<u>{policyId}</u>.</li>" +
                      "<li>An invalid pair of node type and ID is passed to " +
                      "<u>{nodeType}</u> and <u>{nodeId}</u> " +
                      "respectively.</li>" +
                      "<li>Invalid link cost is configured in " +
                      "<strong>integer</strong> element.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified path policy does not exist.</li>" +
                      "<li>A string passed to <u>{policyId}</u> can not be " +
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
    public Response putCost(
            @Context UriInfo uriInfo,
            @PathParam("policyId") int policyId,
            @PathParam("nodeType") String nodeType,
            @PathParam("nodeId") String nodeId,
            @PathParam("portName") String portName,
            @TypeHint(XmlLongInteger.class) XmlLongInteger cost) {
        return putCostImpl(uriInfo, policyId, nodeType, nodeId, portName,
                           null, null, cost);
    }

    /**
     * Detete the link cost for the specified switch port location
     * in the specified path policy.
     *
     * <p>
     *   This API delete the link cost associated with the switch port location
     *   which specifies a physical switch and the name of the switch port.
     * </p>
     * <p>
     *   See document of <strong>pathpolicy</strong> element for more details.
     * </p>
     *
     * @param policyId  The identifier of the path policy.
     * @param nodeType
     *   Type of the node corresponding to the physical switch.
     *    <ul>
     *      <li>
     *        Specify <strong>"OF"</strong> for OpenFlow switch.
     *      </li>
     *    </ul>
     * @param nodeId
     *   A string which represents identifier of the node corresponding to
     *   the physical switch.
     *    <ul>
     *      <li>
     *        Specify a string representation of the DPID for OpenFlow switch.
     *        (e.g. <strong>00:11:22:33:44:55:66:77</strong>)
     *      </li>
     *    </ul>
     * @param portName
     *   The name of the switch port within the physical switch specified by
     *   <span style="text-decoration: underline;">{nodeType}</span> and
     *   <span style="text-decoration: underline;">{nodeId}</span>.
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{policyId}/costs/{nodeType}/{nodeId}/name/{portName}")
    @DELETE
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(TypeHint.NO_CONTENT.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "The link cost associated with the " +
                      "specifeid switch port location in the path policy " +
                      "was deleted successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "The link cost for the specified switch " +
                      "port location is not configured in the path policy."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "An invalid pair of node type and ID is " +
                      "passed to <u>{nodeType}</u> and <u>{nodeId}</u> " +
                      "respectively."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified path policy does not exist.</li>" +
                      "<li>A string passed to <u>{policyId}</u> can not be " +
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
    public Response deleteCost(
            @PathParam("policyId") int policyId,
            @PathParam("nodeType") String nodeType,
            @PathParam("nodeId") String nodeId,
            @PathParam("portName") String portName) {
        return deleteCostImpl(policyId, nodeType, nodeId, portName, null,
                              null);
    }

    /**
     * Return the link cost associated with the specified switch port location
     * in the specified path policy.
     *
     * <p>
     *   This API retrieves the link cost associated with the switch port
     *   location which specifies a physical switch and a pair of node
     *   connector type and ID. Returned link cost will be applied to the
     *   specified switch port unless it is overridden by more exact switch
     *   port location.
     * </p>
     * <p>
     *   See document of <strong>pathpolicy</strong> element for more details.
     * </p>
     *
     * @param policyId  The identifier of the path policy.
     * @param nodeType
     *   Type of the node corresponding to the physical switch.
     *    <ul>
     *      <li>
     *        Specify <strong>"OF"</strong> for OpenFlow switch.
     *      </li>
     *    </ul>
     * @param nodeId
     *   A string which represents identifier of the node corresponding to
     *   the physical switch.
     *    <ul>
     *      <li>
     *        Specify a string representation of the DPID for OpenFlow switch.
     *        (e.g. <strong>00:11:22:33:44:55:66:77</strong>)
     *      </li>
     *    </ul>
     * @param portType
     *    Type of the node connector corresponding to the physical switch port.
     *    <ul>
     *      <li>
     *        Specify <strong>"OF"</strong> for physical port of OpenFlow
     *        switch.
     *      </li>
     *    </ul>
     * @param portId
     *    A string which represents identifier of the node connector
     *    corresponding to the physical switch port.
     *    <ul>
     *      <li>
     *        Specify a string representation of the port number for physical
     *        port of OpenFlow switch.
     *      </li>
     *    </ul>
     * @return
     *   <strong>integer</strong> element contains the link cost associated
     *   with the specified switch port location in the path policy.
     */
    @Path("{policyId}/costs/{nodeType}/{nodeId}/type/{portType}/{portId}")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(XmlLongInteger.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "The link cost is not associated with the " +
                      "specified switch port."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "An invalid pair of node type and ID is " +
                      "passed to <u>{nodeType}</u> and <u>{nodeId}</u> " +
                      "respectively."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified path policy does not exist.</li>" +
                      "<li>A string passed to <u>{policyId}</u> can not be " +
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
    public XmlLongInteger getCost(
            @PathParam("policyId") int policyId,
            @PathParam("nodeType") String nodeType,
            @PathParam("nodeId") String nodeId,
            @PathParam("portType") String portType,
            @PathParam("portId") String portId) {
        return getCostImpl(policyId, nodeType, nodeId, null, portType, portId);
    }

    /**
     * Associate the link cost with the specified switch port location
     * in the specified path policy.
     *
     * <p>
     *   This API associates the link cost with the switch port location
     *   which specifies a physical switch and a pair of node connector type
     *   and ID. The specified link cost will be applied when a packet is
     *   forwarded from the specified switch port unless it is overridden by
     *   more exact switch location.
     * </p>
     * <p>
     *   See document of <strong>pathpolicy</strong> element for more details.
     * </p>
     *
     * @param uriInfo   Requested URI information.
     * @param policyId  The identifier of the path policy.
     * @param nodeType
     *   Type of the node corresponding to the physical switch.
     *    <ul>
     *      <li>
     *        Specify <strong>"OF"</strong> for OpenFlow switch.
     *      </li>
     *    </ul>
     * @param nodeId
     *   A string which represents identifier of the node corresponding to
     *   the physical switch.
     *    <ul>
     *      <li>
     *        Specify a string representation of the DPID for OpenFlow switch.
     *        (e.g. <strong>00:11:22:33:44:55:66:77</strong>)
     *      </li>
     *    </ul>
     * @param portType
     *    Type of the node connector corresponding to the physical switch port.
     *    <ul>
     *      <li>
     *        Specify <strong>"OF"</strong> for physical port of OpenFlow
     *        switch.
     *      </li>
     *    </ul>
     * @param portId
     *    A string which represents identifier of the node connector
     *    corresponding to the physical switch port.
     *    <ul>
     *      <li>
     *        Specify a string representation of the port number for physical
     *        port of OpenFlow switch.
     *      </li>
     *    </ul>
     * @param cost
     *   <strong>integer</strong> element specifies the link cost.
     *   <ul>
     *     <li>
     *       A long integer value more than <strong>0</strong> must be
     *       specified.
     *     </li>
     *   </ul>
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{policyId}/costs/{nodeType}/{nodeId}/type/{portType}/{portId}")
    @PUT
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(TypeHint.NO_CONTENT.class)
    @ResponseHeaders({
        @ResponseHeader(name = "Location",
                        description = "URI corresponding to the newly " +
                        "added the link cost configuration for the switch " +
                        "port location, which is the same URI specified " +
                        "in request. This header is set only if " +
                        "CREATED(201) is returned as response code.")})
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "The link cost associated with existing " +
                      "switch port location in the path policy was changed " +
                      "successfully."),
        @ResponseCode(code = HTTP_CREATED,
                      condition = "The link cost was newly associated with " +
                      "the specified switch port location in the path " +
                      "policy."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "The link cost for the specified switch " +
                      "port location was not changed."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "<ul>" +
                      "<li>Incorrect XML or JSON data is specified " +
                      "in Request body.</li>" +
                      "<li>Invalid path policy identifier is specified to " +
                      "<u>{policyId}</u>.</li>" +
                      "<li>An invalid pair of node type and ID is passed to " +
                      "<u>{nodeType}</u> and <u>{nodeId}</u> " +
                      "respectively.</li>" +
                      "<li>An invalid pair of node connector type and ID is " +
                      "passed to <u>{portType}</u> and <u>{portId}</u> " +
                      "respectively.</li>" +
                      "<li>Invalid link cost is configured in " +
                      "<strong>integer</strong> element.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified path policy does not exist.</li>" +
                      "<li>A string passed to <u>{policyId}</u> can not be " +
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
    public Response putCost(
            @Context UriInfo uriInfo,
            @PathParam("policyId") int policyId,
            @PathParam("nodeType") String nodeType,
            @PathParam("nodeId") String nodeId,
            @PathParam("portType") String portType,
            @PathParam("portId") String portId,
            @TypeHint(XmlLongInteger.class) XmlLongInteger cost) {
        return putCostImpl(uriInfo, policyId, nodeType, nodeId, null,
                           portType, portId, cost);
    }

    /**
     * Detete the link cost for the specified switch port location
     * in the specified path policy.
     *
     * <p>
     *   This API delete the link cost associated with the switch port location
     *   which specifies a physical switch and a pair of node connector type
     *   and ID.
     * </p>
     * <p>
     *   See document of <strong>pathpolicy</strong> element for more details.
     * </p>
     *
     * @param policyId  The identifier of the path policy.
     * @param nodeType
     *   Type of the node corresponding to the physical switch.
     *    <ul>
     *      <li>
     *        Specify <strong>"OF"</strong> for OpenFlow switch.
     *      </li>
     *    </ul>
     * @param nodeId
     *   A string which represents identifier of the node corresponding to
     *   the physical switch.
     *    <ul>
     *      <li>
     *        Specify a string representation of the DPID for OpenFlow switch.
     *        (e.g. <strong>00:11:22:33:44:55:66:77</strong>)
     *      </li>
     *    </ul>
     * @param portType
     *    Type of the node connector corresponding to the physical switch port.
     *    <ul>
     *      <li>
     *        Specify <strong>"OF"</strong> for physical port of OpenFlow
     *        switch.
     *      </li>
     *    </ul>
     * @param portId
     *    A string which represents identifier of the node connector
     *    corresponding to the physical switch port.
     *    <ul>
     *      <li>
     *        Specify a string representation of the port number for physical
     *        port of OpenFlow switch.
     *      </li>
     *    </ul>
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{policyId}/costs/{nodeType}/{nodeId}/type/{portType}/{portId}")
    @DELETE
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(TypeHint.NO_CONTENT.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "The link cost associated with the " +
                      "specifeid switch port location in the path policy " +
                      "was deleted successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "The link cost for the specified switch " +
                      "port location is not configured in the path policy."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "An invalid pair of node type and ID is " +
                      "passed to <u>{nodeType}</u> and <u>{nodeId}</u> " +
                      "respectively."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified path policy does not exist.</li>" +
                      "<li>A string passed to <u>{policyId}</u> can not be " +
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
    public Response deleteCost(
            @PathParam("policyId") int policyId,
            @PathParam("nodeType") String nodeType,
            @PathParam("nodeId") String nodeId,
            @PathParam("portType") String portType,
            @PathParam("portId") String portId) {
        return deleteCostImpl(policyId, nodeType, nodeId, null, portType,
                              portId);
    }

    /**
     * Return the link cost associated with the specified switch port location
     * in the specified path policy.
     *
     * <p>
     *   This API retrieves the link cost associated with the switch port
     *   location which specifies a physical switch, a pair of node
     *   connector type and ID, and the name of the switch port.
     *   Returned link cost will be applied to the specified switch port.
     * </p>
     * <p>
     *   See document of <strong>pathpolicy</strong> element for more details.
     * </p>
     *
     * @param policyId  The identifier of the path policy.
     * @param nodeType
     *   Type of the node corresponding to the physical switch.
     *    <ul>
     *      <li>
     *        Specify <strong>"OF"</strong> for OpenFlow switch.
     *      </li>
     *    </ul>
     * @param nodeId
     *   A string which represents identifier of the node corresponding to
     *   the physical switch.
     *    <ul>
     *      <li>
     *        Specify a string representation of the DPID for OpenFlow switch.
     *        (e.g. <strong>00:11:22:33:44:55:66:77</strong>)
     *      </li>
     *    </ul>
     * @param portType
     *    Type of the node connector corresponding to the physical switch port.
     *    <ul>
     *      <li>
     *        Specify <strong>"OF"</strong> for physical port of OpenFlow
     *        switch.
     *      </li>
     *    </ul>
     * @param portId
     *    A string which represents identifier of the node connector
     *    corresponding to the physical switch port.
     *    <ul>
     *      <li>
     *        Specify a string representation of the port number for physical
     *        port of OpenFlow switch.
     *      </li>
     *    </ul>
     * @param portName
     *   The name of the switch port within the physical switch specified by
     *   <span style="text-decoration: underline;">{nodeType}</span> and
     *   <span style="text-decoration: underline;">{nodeId}</span>.
     * @return
     *   <strong>integer</strong> element contains the link cost associated
     *   with the specified switch port location in the path policy.
     */
    @Path("{policyId}/costs/{nodeType}/{nodeId}/type/{portType}/{portId}/{portName}")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(XmlLongInteger.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "Operation completed successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "The link cost is not associated with the " +
                      "specified switch port."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "An invalid pair of node type and ID is " +
                      "passed to <u>{nodeType}</u> and <u>{nodeId}</u> " +
                      "respectively."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified path policy does not exist.</li>" +
                      "<li>A string passed to <u>{policyId}</u> can not be " +
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
    public XmlLongInteger getCost(
            @PathParam("policyId") int policyId,
            @PathParam("nodeType") String nodeType,
            @PathParam("nodeId") String nodeId,
            @PathParam("portType") String portType,
            @PathParam("portId") String portId,
            @PathParam("portName") String portName) {
        return getCostImpl(policyId, nodeType, nodeId, portName, portType,
                           portId);
    }

    /**
     * Associate the link cost with the specified switch port location
     * in the specified path policy.
     *
     * <p>
     *   This API associates the link cost with the switch port location
     *   which specifies a physical switch, a pair of node connector type
     *   and ID, and the name of the switch port. The specified link cost
     *   will be applied when a packet is forwarded from the specified switch
     *   port.
     * </p>
     * <p>
     *   See document of <strong>pathpolicy</strong> element for more details.
     * </p>
     *
     * @param uriInfo   Requested URI information.
     * @param policyId  The identifier of the path policy.
     * @param nodeType
     *   Type of the node corresponding to the physical switch.
     *    <ul>
     *      <li>
     *        Specify <strong>"OF"</strong> for OpenFlow switch.
     *      </li>
     *    </ul>
     * @param nodeId
     *   A string which represents identifier of the node corresponding to
     *   the physical switch.
     *    <ul>
     *      <li>
     *        Specify a string representation of the DPID for OpenFlow switch.
     *        (e.g. <strong>00:11:22:33:44:55:66:77</strong>)
     *      </li>
     *    </ul>
     * @param portType
     *    Type of the node connector corresponding to the physical switch port.
     *    <ul>
     *      <li>
     *        Specify <strong>"OF"</strong> for physical port of OpenFlow
     *        switch.
     *      </li>
     *    </ul>
     * @param portId
     *    A string which represents identifier of the node connector
     *    corresponding to the physical switch port.
     *    <ul>
     *      <li>
     *        Specify a string representation of the port number for physical
     *        port of OpenFlow switch.
     *      </li>
     *    </ul>
     * @param portName
     *   The name of the switch port within the physical switch specified by
     *   <span style="text-decoration: underline;">{nodeType}</span> and
     *   <span style="text-decoration: underline;">{nodeId}</span>.
     * @param cost
     *   <strong>integer</strong> element specifies the link cost.
     *   <ul>
     *     <li>
     *       A long integer value more than <strong>0</strong> must be
     *       specified.
     *     </li>
     *   </ul>
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{policyId}/costs/{nodeType}/{nodeId}/type/{portType}/{portId}/{portName}")
    @PUT
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(TypeHint.NO_CONTENT.class)
    @ResponseHeaders({
        @ResponseHeader(name = "Location",
                        description = "URI corresponding to the newly " +
                        "added the link cost configuration for the switch " +
                        "port location, which is the same URI specified " +
                        "in request. This header is set only if " +
                        "CREATED(201) is returned as response code.")})
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "The link cost associated with existing " +
                      "switch port location in the path policy was changed " +
                      "successfully."),
        @ResponseCode(code = HTTP_CREATED,
                      condition = "The link cost was newly associated with " +
                      "the specified switch port location in the path " +
                      "policy."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "The link cost for the specified switch " +
                      "port location was not changed."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "<ul>" +
                      "<li>Incorrect XML or JSON data is specified " +
                      "in Request body.</li>" +
                      "<li>Invalid path policy identifier is specified to " +
                      "<u>{policyId}</u>.</li>" +
                      "<li>An invalid pair of node type and ID is passed to " +
                      "<u>{nodeType}</u> and <u>{nodeId}</u> " +
                      "respectively.</li>" +
                      "<li>An invalid pair of node connector type and ID is " +
                      "passed to <u>{portType}</u> and <u>{portId}</u> " +
                      "respectively.</li>" +
                      "<li>Invalid link cost is configured in " +
                      "<strong>integer</strong> element.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified path policy does not exist.</li>" +
                      "<li>A string passed to <u>{policyId}</u> can not be " +
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
    public Response putCost(
            @Context UriInfo uriInfo,
            @PathParam("policyId") int policyId,
            @PathParam("nodeType") String nodeType,
            @PathParam("nodeId") String nodeId,
            @PathParam("portType") String portType,
            @PathParam("portId") String portId,
            @PathParam("portName") String portName,
            @TypeHint(XmlLongInteger.class) XmlLongInteger cost) {
        return putCostImpl(uriInfo, policyId, nodeType, nodeId, portName,
                           portType, portId, cost);
    }

    /**
     * Detete the link cost for the specified switch port location
     * in the specified path policy.
     *
     * <p>
     *   This API delete the link cost associated with the switch port location
     *   which specifies a physical switch, a pair of node connector type and
     *   ID, and the name of the switch port.
     * </p>
     * <p>
     *   See document of <strong>pathpolicy</strong> element for more details.
     * </p>
     *
     * @param policyId  The identifier of the path policy.
     * @param nodeType
     *   Type of the node corresponding to the physical switch.
     *    <ul>
     *      <li>
     *        Specify <strong>"OF"</strong> for OpenFlow switch.
     *      </li>
     *    </ul>
     * @param nodeId
     *   A string which represents identifier of the node corresponding to
     *   the physical switch.
     *    <ul>
     *      <li>
     *        Specify a string representation of the DPID for OpenFlow switch.
     *        (e.g. <strong>00:11:22:33:44:55:66:77</strong>)
     *      </li>
     *    </ul>
     * @param portType
     *    Type of the node connector corresponding to the physical switch port.
     *    <ul>
     *      <li>
     *        Specify <strong>"OF"</strong> for physical port of OpenFlow
     *        switch.
     *      </li>
     *    </ul>
     * @param portId
     *    A string which represents identifier of the node connector
     *    corresponding to the physical switch port.
     *    <ul>
     *      <li>
     *        Specify a string representation of the port number for physical
     *        port of OpenFlow switch.
     *      </li>
     *    </ul>
     * @param portName
     *   The name of the switch port within the physical switch specified by
     *   <span style="text-decoration: underline;">{nodeType}</span> and
     *   <span style="text-decoration: underline;">{nodeId}</span>.
     * @return Response as dictated by the HTTP Response Status code.
     */
    @Path("{policyId}/costs/{nodeType}/{nodeId}/type/{portType}/{portId}/{portName}")
    @DELETE
    @Consumes({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(TypeHint.NO_CONTENT.class)
    @StatusCodes({
        @ResponseCode(code = HTTP_OK,
                      condition = "The link cost associated with the " +
                      "specifeid switch port location in the path policy " +
                      "was deleted successfully."),
        @ResponseCode(code = HTTP_NO_CONTENT,
                      condition = "The link cost for the specified switch " +
                      "port location is not configured in the path policy."),
        @ResponseCode(code = HTTP_BAD_REQUEST,
                      condition = "An invalid pair of node type and ID is " +
                      "passed to <u>{nodeType}</u> and <u>{nodeId}</u> " +
                      "respectively."),
        @ResponseCode(code = HTTP_UNAUTHORIZED,
                      condition = "User is not authorized to perform this " +
                      "operation."),
        @ResponseCode(code = HTTP_NOT_FOUND,
                      condition = "<ul>" +
                      "<li>The specified path policy does not exist.</li>" +
                      "<li>A string passed to <u>{policyId}</u> can not be " +
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
    public Response deleteCost(
            @PathParam("policyId") int policyId,
            @PathParam("nodeType") String nodeType,
            @PathParam("nodeId") String nodeId,
            @PathParam("portType") String portType,
            @PathParam("portId") String portId,
            @PathParam("portName") String portName) {
        return deleteCostImpl(policyId, nodeType, nodeId, portName, portType,
                              portId);
    }

    /**
     * Return the link cost associated with the specified switch port location.
     *
     * @param policyId  The identifier of the path policy.
     * @param nodeType  Type of the node corresponding to the physical switch.
     * @param nodeId    A string which represents identifier of the node
     *                  corresponding to the physical switch.
     * @param portName  The name of the physical switch port.
     * @param portType  Type of the node connector corresponding to a
     *                  physical switch port.
     * @param portId    A string which represents identifier of a node
     *                  connector corresponding to a physical switch port.
     * @return  A {@link XmlLongInteger} instance which contains the link cost.
     * @throws BadRequestException
     *    Invalid parameter is specified.
     */
    private XmlLongInteger getCostImpl(int policyId, String nodeType,
                                       String nodeId, String portName,
                                       String portType, String portId) {
        checkPrivilege(Privilege.READ);

        IVTNManager mgr = getVTNManager();
        PortLocation ploc =
            createLocation(nodeType, nodeId, portName, portType, portId);
        try {
            long cost = mgr.getPathPolicyCost(policyId, ploc);
            return (cost == PathPolicy.COST_UNDEF)
                ? null
                : new XmlLongInteger(cost);
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Associate the link cost with the specified switch port location.
     *
     * @param uriInfo   Requested URI information.
     * @param policyId  The identifier of the path policy.
     * @param nodeType  Type of the node corresponding to the physical switch.
     * @param nodeId    A string which represents identifier of the node
     *                  corresponding to the physical switch.
     * @param portName  The name of the physical switch port.
     * @param portType  Type of the node connector corresponding to a
     *                  physical switch port.
     * @param portId    A string which represents identifier of a node
     *                  connector corresponding to a physical switch port.
     * @param cost      A {@link XmlLongInteger} instance which contains
     *                  the link cost to be configured.
     * @return  A {@link Response} instance which represents the result of
     *          REST API.
     * @throws BadRequestException
     *    Invalid parameter is specified.
     */
    private Response putCostImpl(UriInfo uriInfo, int policyId,
                                 String nodeType, String nodeId,
                                 String portName, String portType,
                                 String portId, XmlLongInteger cost) {
        checkPrivilege(Privilege.WRITE);

        IVTNManager mgr = getVTNManager();
        Long c = cost.getValue();
        if (c == null) {
            throw new BadRequestException("Link cost must be specified.");
        }
        PortLocation ploc =
            createLocation(nodeType, nodeId, portName, portType, portId);
        try {
            UpdateType result =
                mgr.setPathPolicyCost(policyId, ploc, c.longValue());
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
     * Detete the link cost configuration for the specified switch port
     * location.
     *
     * @param policyId  The identifier of the path policy.
     * @param nodeType  Type of the node corresponding to the physical switch.
     * @param nodeId    A string which represents identifier of the node
     *                  corresponding to the physical switch.
     * @param portName  The name of the physical switch port.
     * @param portType  Type of the node connector corresponding to a
     *                  physical switch port.
     * @param portId    A string which represents identifier of a node
     *                  connector corresponding to a physical switch port.
     * @return  A {@link Response} instance which represents the result of
     *          REST API.
     * @throws BadRequestException
     *    Invalid parameter is specified.
     */
    private Response deleteCostImpl(int policyId, String nodeType,
                                    String nodeId, String portName,
                                    String portType, String portId) {
        checkPrivilege(Privilege.WRITE);

        IVTNManager mgr = getVTNManager();
        PortLocation ploc =
            createLocation(nodeType, nodeId, portName, portType, portId);
        Status status = mgr.removePathPolicyCost(policyId, ploc);
        if (status == null) {
            return Response.noContent().build();
        }
        if (status.isSuccess()) {
            return Response.ok().build();
        }

        throw getException(status);
    }

    /**
     * Construct a new {@link PortLocation} instance.
     *
     * @param nodeType  Type of the node corresponding to the physical switch.
     * @param nodeId    A string which represents identifier of the node
     *                  corresponding to the physical switch.
     * @param portType  Type of the node connector corresponding to a
     *                  physical switch port.
     * @param portId    A string which represents identifier of a node
     *                  connector corresponding to a physical switch port.
     * @param portName  The name of the physical switch port.
     * @return  A {@link PortLocation} instance.
     * @throws BadRequestException
     *    Invalid parameter is specified.
     */
    private PortLocation createLocation(String nodeType, String nodeId,
                                        String portName, String portType,
                                        String portId) {
        Node node = parseNode(nodeType, nodeId);
        SwitchPort swport;
        if (portType == null && portId == null && portName == null) {
            swport = null;
        } else {
            swport = new SwitchPort(portName, portType, portId);
        }

        return new PortLocation(node, swport);
    }
}
