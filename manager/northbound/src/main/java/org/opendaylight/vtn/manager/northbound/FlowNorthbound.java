/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import static java.net.HttpURLConnection.HTTP_BAD_REQUEST;
import static java.net.HttpURLConnection.HTTP_INTERNAL_ERROR;
import static java.net.HttpURLConnection.HTTP_NOT_FOUND;
import static java.net.HttpURLConnection.HTTP_NO_CONTENT;
import static java.net.HttpURLConnection.HTTP_OK;
import static java.net.HttpURLConnection.HTTP_UNAUTHORIZED;
import static java.net.HttpURLConnection.HTTP_UNAVAILABLE;

import javax.ws.rs.DefaultValue;
import javax.ws.rs.GET;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.Produces;
import javax.ws.rs.QueryParam;
import javax.ws.rs.core.MediaType;

import org.codehaus.enunciate.jaxrs.ResponseCode;
import org.codehaus.enunciate.jaxrs.StatusCodes;
import org.codehaus.enunciate.jaxrs.TypeHint;
import org.opendaylight.controller.sal.authorization.Privilege;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.vtn.manager.EthernetHost;
import org.opendaylight.vtn.manager.IVTNManager;
import org.opendaylight.vtn.manager.SwitchPort;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.flow.DataFlow;
import org.opendaylight.vtn.manager.flow.DataFlowFilter;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.DataFlowMode;

/**
 * This class provides Northbound REST APIs to handle data flow in the VTN.
 *
 * @since  Helium
 */
@Path("/default/vtns/{tenantName}/flows")
public class FlowNorthbound extends VTNNorthBoundBase {
    /**
     * Return summarized information about all data flows present in the
     * specified VTN.
     *
     * <p>
     *   The condition to filter out unwanted data flows can be specified
     *   by query parameters.
     *   If more than one contitions are specified, only data flows that meet
     *   all the conditions will be selected.
     * </p>
     *
     * <dl style="margin-left: 1em;">
     *   <dt>Source L2 host
     *   <dd style="margin-left: 1.5em">
     *     Source L2 host can be specified by a pair of
     *     <span style="text-decoration: underline;">srcMac</span> and
     *     <span style="text-decoration: underline;">srcVlan</span> parameters.
     *     If this condition is specified, only data flows that map packets
     *     sent by the specified L2 host will be selected.
     *
     *   <dt>Physical switch or physical switch port.
     *   <dd style="margin-left: 1.5em">
     *     Either a physical switch or a physical switch port can be specified.
     *     <ul>
     *       <li>
     *         A physical switch can be specified by
     *         <span style="text-decoration: underline;">node</span> parameter.
     *         If a physical switch is specified, only data flows that forward
     *         packets via the specified switch will be selected.
     *       </li>
     *       <li>
     *         A physical switch port can be specifeid by a set of
     *         <span style="text-decoration: underline;">node</span>,
     *         <span style="text-decoration: underline;">portType</span>,
     *         <span style="text-decoration: underline;">portId</span>, and
     *         <span style="text-decoration: underline;">portName</span>
     *         parameters. If a physical switch port is specified, only
     *         data flows that forward packets via the specified port will
     *         be selected.
     *       </li>
     *     </ul>
     * </dl>
     *
     * @param tenantName The name of the VTN.
     * @param srcMac
     *    The MAC address which specifies the source L2 host.
     *    <ul>
     *      <li>
     *        A MAC address should be specified in hexadecimal notation with
     *        {@code ':'} inserted between octets.
     *        (e.g. {@code 11:22:33:aa:bb:cc})
     *      </li>
     *    </ul>
     * @param srcVlan
     *    The VLAN ID which specifies the source L2 host.
     *    <ul>
     *      <li><strong>0</strong> implies untagged network.</li>
     *      <li>
     *        This parameter is ignored unless
     *        <span style="text-decoration: underline;">srcMac</span>
     *        parameter is specified.
     *      </li>
     *    </ul>
     * @param nodeStr
     *    A string representation of a {@link Node} instance corresponding
     *    to a physical switch.
     *    <p>
     *       For example, <strong>OF|00:11:22:33:44:55:66:77</strong> means
     *       an OpenFlow switch with the DPID 00:11:22:33:44:55:66:77.
     *    </p>
     * @param portType
     *    Type of the node connector corresponding to the physical switch port.
     *    <ul>
     *      <li>
     *        Specify <strong>"OF"</strong> for physical port of OpenFlow
     *        switch.
     *      </li>
     *      <li>
     *        This parameter is ignored unless both
     *        <span style="text-decoration: underline;">node</span> and
     *        <span style="text-decoration: underline;">portId</span>
     *        parameters are specified.
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
     *      <li>
     *        This parameter is ignored unless both
     *        <span style="text-decoration: underline;">node</span> and
     *        <span style="text-decoration: underline;">portType</span>
     *        parameters are specified.
     *      </li>
     *    </ul>
     * @param portName
     *    The name of the physical switch port.
     *    <ul>
     *      <li>
     *        This parameter is ignored unless
     *        <span style="text-decoration: underline;">node</span> parameter
     *        is specified.
     *      </li>
     *    </ul>
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
     */
    @Path("summary")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(DataFlowList.class)
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
                      condition = "The specified VTN does not exist."),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public DataFlowList getDataFlows(
            @PathParam("tenantName") String tenantName,
            @QueryParam("srcMac") String srcMac,
            @QueryParam("srcVlan") @DefaultValue("0") String srcVlan,
            @QueryParam("node") String nodeStr,
            @QueryParam("portType") String portType,
            @QueryParam("portId") String portId,
            @QueryParam("portName") String portName) {
        checkPrivilege(Privilege.READ);

        IVTNManager mgr = getVTNManager();
        VTenantPath path = new VTenantPath(tenantName);
        DataFlowFilter filter = createFilter(srcMac, srcVlan, nodeStr,
                                             portType, portId, portName);
        try {
            return new DataFlowList(
                mgr.getDataFlows(path, DataFlowMode.SUMMARY, filter, -1));
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Return summarized information about the data flow specified by the
     * flow identifier.
     *
     * @param tenantName  The name of the VTN.
     * @param flowId
     *    The identifier of the data flow.
     *    A string representation of a long integer value must be
     *    specified.
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
                      "<li>The specified VTN does not exist.</li>" +
                      "<li>A string passed to <u>{flowId}</u> can not be " +
                      "converted into a long integer.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public DataFlow getDataFlow(
            @PathParam("tenantName") String tenantName,
            @PathParam("flowId") long flowId) {
        checkPrivilege(Privilege.READ);

        IVTNManager mgr = getVTNManager();
        VTenantPath path = new VTenantPath(tenantName);
        try {
            return mgr.getDataFlow(path, flowId, DataFlowMode.SUMMARY, -1);
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Return detailed information about all data flows present in the
     * specified VTN.
     *
     * <p>
     *   The condition to filter out unwanted data flows can be specified
     *   by query parameters.
     *   If more than one contitions are specified, only data flows that meet
     *   all the conditions will be selected.
     * </p>
     *
     * <dl style="margin-left: 1em;">
     *   <dt>Source L2 host
     *   <dd style="margin-left: 1.5em">
     *     Source L2 host can be specified by a pair of
     *     <span style="text-decoration: underline;">srcMac</span> and
     *     <span style="text-decoration: underline;">srcVlan</span> parameters.
     *     If this condition is specified, only data flows that map packets
     *     sent by the specified L2 host will be selected.
     *
     *   <dt>Physical switch or physical switch port.
     *   <dd style="margin-left: 1.5em">
     *     Either a physical switch or a physical switch port can be specified.
     *     <ul>
     *       <li>
     *         A physical switch can be specified by
     *         <span style="text-decoration: underline;">node</span> parameter.
     *         If a physical switch is specified, only data flows that forward
     *         packets via the specified switch will be selected.
     *       </li>
     *       <li>
     *         A physical switch port can be specifeid by a set of
     *         <span style="text-decoration: underline;">node</span>,
     *         <span style="text-decoration: underline;">portType</span>,
     *         <span style="text-decoration: underline;">portId</span>, and
     *         <span style="text-decoration: underline;">portName</span>
     *         parameters. If a physical switch port is specified, only
     *         data flows that forward packets via the specified port will
     *         be selected.
     *       </li>
     *     </ul>
     * </dl>
     *
     * @param tenantName  The name of the VTN.
     * @param srcMac
     *    The MAC address which specifies the source L2 host.
     *    <ul>
     *      <li>
     *        A MAC address should be specified in hexadecimal notation with
     *        {@code ':'} inserted between octets.
     *        (e.g. {@code 11:22:33:aa:bb:cc})
     *      </li>
     *    </ul>
     * @param srcVlan
     *    The VLAN ID which specifies the source L2 host.
     *    <ul>
     *      <li><strong>0</strong> implies untagged network.</li>
     *      <li>
     *        This parameter is ignored unless
     *        <span style="text-decoration: underline;">srcMac</span>
     *        parameter is specified.
     *      </li>
     *    </ul>
     * @param nodeStr
     *    A string representation of a {@link Node} instance corresponding
     *    to a physical switch.
     *    <p>
     *       For example, <strong>OF|00:11:22:33:44:55:66:77</strong> means
     *       an OpenFlow switch with the DPID 00:11:22:33:44:55:66:77.
     *    </p>
     * @param portType
     *    Type of the node connector corresponding to the physical switch port.
     *    <ul>
     *      <li>
     *        Specify <strong>"OF"</strong> for physical port of OpenFlow
     *        switch.
     *      </li>
     *      <li>
     *        This parameter is ignored unless both
     *        <span style="text-decoration: underline;">node</span> and
     *        <span style="text-decoration: underline;">portId</span>
     *        parameters are specified.
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
     *      <li>
     *        This parameter is ignored unless both
     *        <span style="text-decoration: underline;">node</span> and
     *        <span style="text-decoration: underline;">portType</span>
     *        parameters are specified.
     *      </li>
     *    </ul>
     * @param portName
     *    The name of the physical switch port.
     *    <ul>
     *      <li>
     *        This parameter is ignored unless
     *        <span style="text-decoration: underline;">node</span> parameter
     *        is specified.
     *      </li>
     *    </ul>
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
     * @param interval
     *   Time interval in seconds for retrieving the average statistics.
     *   <ul>
     *     <li>The default value is <strong>10</strong> seconds.</li>
     *     <li>
     *       The default value is used if zero or a negative value is
     *       specified.
     *     </li>
     *     <li>
     *       Note that this value is just a hint for determining the
     *       measurement period. The actual measurement period can be derived
     *       from <strong>averagedStats</strong> element in the returned
     *       <strong>dataflow</strong> element.
     *     </li>
     *   </ul>
     * @return
     *    <strong>dataflows</strong> element contains detailed information
     *    about all data flows present in the VTN specified by the requested
     *    URI.
     */
    @Path("detail")
    @GET
    @Produces({MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML})
    @TypeHint(DataFlowList.class)
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
                      condition = "The specified VTN does not exist."),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public DataFlowList getDetailedDataFlows(
            @PathParam("tenantName") String tenantName,
            @QueryParam("srcMac") String srcMac,
            @QueryParam("srcVlan") @DefaultValue("0") String srcVlan,
            @QueryParam("node") String nodeStr,
            @QueryParam("portType") String portType,
            @QueryParam("portId") String portId,
            @QueryParam("portName") String portName,
            @DefaultValue("false") @QueryParam("update") boolean update,
            @DefaultValue("10") @QueryParam("interval") int interval) {
        checkPrivilege(Privilege.READ);

        IVTNManager mgr = getVTNManager();
        VTenantPath path = new VTenantPath(tenantName);
        DataFlowFilter filter = createFilter(srcMac, srcVlan, nodeStr,
                                             portType, portId, portName);
        DataFlowMode mode = (update)
            ? DataFlowMode.UPDATESTATS
            : DataFlowMode.DETAIL;
        try {
            return new DataFlowList(mgr.getDataFlows(path, mode, filter,
                                                     interval));
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Return detailed information about the data flow specified by the
     * flow identifier.
     *
     * @param tenantName  The name of the VTN.
     * @param flowId
     *    The identifier of the data flow.
     *    A string representation of a long integer value must be
     *    specified.
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
     * @param interval
     *   Time interval in seconds for retrieving the average statistics.
     *   <ul>
     *     <li>The default value is <strong>10</strong> seconds.</li>
     *     <li>
     *       The default value is used if zero or a negative value is
     *       specified.
     *     </li>
     *     <li>
     *       Note that this value is just a hint for determining the
     *       measurement period. The actual measurement period can be derived
     *       from <strong>averagedStats</strong> element in the returned
     *       <strong>dataflow</strong> element.
     *     </li>
     *   </ul>
     * @return
     *    <strong>dataflow</strong> element contains detailed information
     *    about the data flow specified by the requested URI.
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
                      "<li>The specified VTN does not exist.</li>" +
                      "<li>A string passed to <u>{flowId}</u> can not be " +
                      "converted into a long integer.</li>" +
                      "</ul>"),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public DataFlow getDetailedDataFlow(
            @PathParam("tenantName") String tenantName,
            @PathParam("flowId") long flowId,
            @DefaultValue("false") @QueryParam("update") boolean update,
            @DefaultValue("10") @QueryParam("interval") int interval) {
        checkPrivilege(Privilege.READ);

        IVTNManager mgr = getVTNManager();
        VTenantPath path = new VTenantPath(tenantName);
        DataFlowMode mode = (update)
            ? DataFlowMode.UPDATESTATS
            : DataFlowMode.DETAIL;
        try {
            return mgr.getDataFlow(path, flowId, mode, interval);
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Return the number of data flows present in the specified VTN.
     *
     * @param tenantName  The name of the VTN.
     * @return
     *    <strong>integer</strong> element contains the number of data flows
     *    present in the VTN specified by the requested URI.
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
                      condition = "The specified VTN does not exist."),
        @ResponseCode(code = HTTP_INTERNAL_ERROR,
                      condition = "Fatal internal error occurred in the " +
                      "VTN Manager."),
        @ResponseCode(code = HTTP_UNAVAILABLE,
                      condition = "One or more of mandatory controller " +
                      "services, such as the VTN Manager, are unavailable.")})
    public XmlLongInteger getFlowCount(
            @PathParam("tenantName") String tenantName) {
        checkPrivilege(Privilege.READ);

        IVTNManager mgr = getVTNManager();
        VTenantPath path = new VTenantPath(tenantName);
        try {
            return new XmlLongInteger(mgr.getDataFlowCount(path));
        } catch (VTNException e) {
            throw getException(e);
        }
    }

    /**
     * Create a {@link DataFlowFilter} instance as specified.
     *
     * @param srcMac    The MAC address which specifies the source L2 host.
     * @param srcVlan   The VLAN ID which specifies the source L2 host.
     * @param nodeStr   A string representation of a {@link Node} instance
     *                  corresponding to a physical switch.
     * @param portType  Type of the node connector corresponding to the
     *                  physical switch port.
     * @param portId    A string which represents identifier of the node
     *                  connector corresponding to the physical switch port.
     * @param portName  The name of the physical switch port.
     * @return  A {@link DataFlowFilter} instance.
     * @throws org.opendaylight.controller.northbound.commons.exception.BadRequestException
     *    Invalid parameter is specified.
     */
    private DataFlowFilter createFilter(String srcMac, String srcVlan,
                                        String nodeStr, String portType,
                                        String portId, String portName) {
        DataFlowFilter filter = new DataFlowFilter();
        if (srcMac != null) {
            // Specify the source L2 host.
            EthernetAddress eaddr = parseEthernetAddress(srcMac);
            short vlan = parseVlanId(srcVlan);
            EthernetHost src = new EthernetHost(eaddr, vlan);
            filter.setSourceHost(src);
        }

        if (nodeStr != null) {
            // Specify the physical switch.
            Node node = parseNode(nodeStr);
            filter.setNode(node);

            // Determine whether the switch port is specified or not.
            if (portType != null && portId != null) {
                // Specify the location of the physical switch port.
                // The port name is also used if specified.
                SwitchPort port = new SwitchPort(portName, portType, portId);
                filter.setSwitchPort(port);
            } else if (portName != null) {
                // Specify the name of the physical switch port.
                SwitchPort port = new SwitchPort(portName);
                filter.setSwitchPort(port);
            }
        }

        return filter;
    }
}
