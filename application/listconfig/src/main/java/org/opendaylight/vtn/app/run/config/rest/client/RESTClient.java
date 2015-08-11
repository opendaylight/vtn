/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.client;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Scanner;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.json.JSONArray;
import org.json.JSONObject;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import org.opendaylight.vtn.app.run.config.rest.enumgroups.ApplicationType;
import org.opendaylight.vtn.app.run.config.rest.enumgroups.RestURL;
import org.opendaylight.vtn.app.run.config.rest.output.format.beans.VBridgeBean;
import org.opendaylight.vtn.app.run.config.rest.output.format.beans.VBridgeInterfaceBean;
import org.opendaylight.vtn.app.run.config.rest.parser.CRUDImpl;
import org.opendaylight.vtn.app.run.config.rest.parser.Parser;
import org.opendaylight.vtn.app.run.config.rest.parser.RestRequest;
import org.opendaylight.vtn.app.run.config.rest.response.beans.ContainerPathMapNorthBoundList;
import org.opendaylight.vtn.app.run.config.rest.response.beans.DataFlowList;
import org.opendaylight.vtn.app.run.config.rest.response.beans.FlowconditionList;
import org.opendaylight.vtn.app.run.config.rest.response.beans.Index;
import org.opendaylight.vtn.app.run.config.rest.response.beans.MacEntryList;
import org.opendaylight.vtn.app.run.config.rest.response.beans.MacMapNorthBound;
import org.opendaylight.vtn.app.run.config.rest.response.beans.PathPoliciesIndex;
import org.opendaylight.vtn.app.run.config.rest.response.beans.PathPolicy;
import org.opendaylight.vtn.app.run.config.rest.response.beans.VBridgeInterface;
import org.opendaylight.vtn.app.run.config.rest.response.beans.VBridgeInterfaceList;
import org.opendaylight.vtn.app.run.config.rest.response.beans.VBridgeIfFlowFilterNorthboundList;
import org.opendaylight.vtn.app.run.config.rest.response.beans.VBridgeNorthBound;
import org.opendaylight.vtn.app.run.config.rest.response.beans.VBridgeNorthBoundList;
import org.opendaylight.vtn.app.run.config.rest.response.beans.VBridgePortMap;
import org.opendaylight.vtn.app.run.config.rest.response.beans.VTNConfiguration;
import org.opendaylight.vtn.app.run.config.rest.response.beans.VTNConfigurationBean;
import org.opendaylight.vtn.app.run.config.rest.response.beans.VTNManagerVersion;
import org.opendaylight.vtn.app.run.config.rest.response.beans.VTenantNorthBound;
import org.opendaylight.vtn.app.run.config.rest.response.beans.VTenantNorthBoundList;
import org.opendaylight.vtn.app.run.config.rest.response.beans.VlanNorthBoundList;
import org.opendaylight.vtn.app.run.config.rest.response.beans.VTerminalIfFlowFilterNorthboundList;
import org.opendaylight.vtn.app.run.config.rest.response.beans.VTerminalInterfaceNorthboundList;
import org.opendaylight.vtn.app.run.config.rest.response.beans.VTerminalNorthBound;
import org.opendaylight.vtn.app.run.config.rest.response.beans.VTerminalNorthBoundList;
import org.opendaylight.vtn.app.run.config.rest.response.beans.VTerminalInterfaceNorthbound;
import org.opendaylight.vtn.app.run.config.rest.response.beans.VTerminalPortMap;
import org.opendaylight.vtn.app.run.config.rest.response.beans.VBridgeFlowFilterList;
import org.opendaylight.vtn.app.run.config.rest.response.beans.VTenantFlowFilterList;
import org.opendaylight.vtn.app.run.config.rest.output.format.beans.VTerminalBean;
import org.opendaylight.vtn.app.run.config.rest.output.format.beans.VTerminalInterfaceBean;

/**
 * RESTClient - Client main class helps to perform all the CRUD Operations.
 *
 */
public class RESTClient {

    /**
     * Logger instance.
     */
    static final Logger LOG = LoggerFactory.getLogger(RESTClient.class);

    /**
     * CRUDOperation interface reference.
     */
    private CRUDOperation client = null;

    /**
     * Json Parser class Parser reference.
     */
    private Parser parser = null;

    /**
     * REST Service server URL address.
     */
    private String serverUrl = null;

    /**
     * REST Service runnig server ip address.
     */
    private String serverIp = null;

    /**
     * REST SErvice running server port number.
     */
    private String serverPort = null;

    /**
     * Response string value received from the request.
     */
    private String response = null;


    /**
     * RESTClient - Constructor with arguments.
     * @param serverIp - IP of the server where the service is running.
     * @param serverPort - port number through the client can connect server.
     * @param userName - username of the controller to connect to the server.
     * @param password - password of the controller to connect to the server.
     * @throws VTNClientException
     */
    public RESTClient(String serverIp, String serverPort, String userName, String password) throws VTNClientException {

        this.serverIp = serverIp;
        this.serverPort = serverPort;
        this.serverUrl = "http://" + serverIp + ":" + serverPort;
        if (serverUrl == null || serverUrl.equals("")) {
            LOG.warn("Invalid server URL : {}", serverUrl);
            throw new VTNClientException("Invalid server URL");
        }
        client = new CRUDImpl(ApplicationType.JSON, userName, password);
        parser = new Parser();
    }

    /**
     * getClient - function to get the CRUDOperation reference..
     * @return {@link CRUDOperation}
     */
    public final CRUDOperation getClient() {
        return client;
    }

    /**
     * GET - get operation method to communicate the server to fetch the result for requested URI.
     * @param restObject
     * @return {@link Object}
     * @throws IllegalAccessException
     * @throws InstantiationException
     * @throws VTNClientException
     */
    public Object get(Object restObject)
        throws IllegalAccessException, InstantiationException,
               VTNClientException {
        Map<String, String> paramMap = new HashMap<String, String>();
        Map<String, Object> headers = new HashMap<String, Object>();
        return this.get(restObject, paramMap, headers);
    }

    /**
     * GET - get operation method to communicate the server to fetch the result for requested URI.
     * @param restObject
     * @param headers
     * @return {@link Object}
     * @throws IllegalAccessException
     * @throws InstantiationException
     * @throws VTNClientException
     */
    public Object get(Object restObject, Map<String, Object> headers)
        throws IllegalAccessException, InstantiationException,
               VTNClientException {
        Map<String, String> paramMap = new HashMap<String, String>();
        return this.get(restObject, paramMap, headers);
    }

    /**
     * GET - get operation method to communicate the server to fetch the result for requested URI.
     * @param restObject
     * @param headers
     * @param paramName
     * @param paramValue
     * @return {@link Object}
     * @throws IllegalAccessException
     * @throws InstantiationException
     * @throws VTNClientException
     */
    public Object get(Object restObject, Map<String, Object> headers,
            String paramName, String paramValue)
        throws IllegalAccessException, InstantiationException,
               VTNClientException {
        Map<String, String> paramMap = new HashMap<String, String>();
        paramMap.put(paramName, paramValue);
        return this.get(restObject, paramMap, headers);
    }

    /**
     * GET - get operation method to communicate the server to fetch the result for requested URI.
     * @param restObject {@link Object}
     * @param paramName {@link String}
     * @param paramValue {@link String}
     * @return {@link Object}
     * @throws IllegalAccessException
     * @throws InstantiationException
     * @throws VTNClientException
     */
    public Object get(Object restObject, String paramName, String paramValue)
        throws IllegalAccessException, InstantiationException,
               VTNClientException {
        Map<String, String> paramMap = new HashMap<String, String>();
        paramMap.put(paramName, paramValue);
        return this.get(restObject, paramMap, null);
    }

    /**
     * GET - get operation method to communicate the server to fetch the result for requested URI.
     * @param restObject
     * @param paramMap
     * @return The value of the requested URI.
     * @throws IllegalAccessException
     * @throws InstantiationException
     * @throws VTNClientException
     */
    public Object get(Object restObject, HashMap<String, String> paramMap)
        throws IllegalAccessException, InstantiationException,
               VTNClientException {
        return this.get(restObject, paramMap, null);
    }

    /**
     * GET - get operation method to communicate the server to fetch the result for requested URI.
     * @param restObject
     * @param param1
     * @param value1
     * @param param2
     * @param value2
     * @return The value of the requested URI.
     * @throws IllegalAccessException
     * @throws InstantiationException
     * @throws VTNClientException
     */
    public Object get(Object restObject, String param1, String value1,
            String param2, String value2)
        throws IllegalAccessException, InstantiationException,
               VTNClientException {
        Map<String, String> paramMap = new HashMap<String, String>();
        paramMap.put(param1, value1);
        paramMap.put(param2, value2);
        return this.get(restObject, paramMap, null);
    }

    /**
     * GET - get operation method to communicate the server to fetch the result for requested URI.
     * @param restObject
     * @param headers
     * @param param1
     * @param value1
     * @param param2
     * @param value2
     * @return The value of the requested URI.
     * @throws IllegalAccessException
     * @throws InstantiationException
     * @throws VTNClientException
     */
    public Object get(Object restObject, Map<String, Object> headers,
        String param1, String value1, String param2, String value2)
        throws IllegalAccessException, InstantiationException,
               VTNClientException {
        Map<String, String> paramMap = new HashMap<String, String>();
        paramMap.put(param1, value1);
        paramMap.put(param2, value2);
        return this.get(restObject, paramMap, headers);
    }

    /**
     * GET - get operation method to communicate the server to fetch the result for requested URI.
     * @param restObject
     * @param paramMap
     * @param headers
     * @return The value of the requested URI.
     * @throws IllegalAccessException
     * @throws InstantiationException
     * @throws VTNClientException
     */
    public Object get(Object restObject, Map<String, String> paramMap,
            Map<String, Object> headers)
        throws IllegalAccessException, InstantiationException,
               VTNClientException {
        RestRequest request = getRestRequest(restObject, paramMap);
        if (request != null) {
            request.setResponse(client.doGET(request.getUrl(), headers));
            setResponse(request.getResponse());
        }
        return parseResponse(restObject, request);
    }

    /**
     * getResponse - response for the request.
     * @return {@link String}
     */
    public String getResponse() {
        return response;
    }

    /**
     * setResponse - sets the response of the request.
     * @param response
     */
    private void setResponse(String response) {
        this.response = response;
    }

    /**
     * Get the request object for an object annotated with 'RestURL'
     *
     * @param obj
     *        Rest object
     * @param paramMap
     *        Parameter map
     * @return Rest REquest object
     *
     * @throws VTNClientException
     *         Throws VTNClientException if unable to construct the Rest
     *         Request
     */
    @SuppressWarnings("unused")
    public RestRequest getRestRequest(Object obj, Map<String, String> paramMap)
        throws VTNClientException {
        RestRequest request = null;
        RestURL restUrl = obj.getClass().getAnnotation(RestURL.class);
        if (restUrl != null) {
            if (!restUrl.vtnMgrUrl().equals("")) {
                request = new RestRequest();

                request.setUrl(getServerUrl() + "/" + restUrl.vtnMgrUrl());

                if (paramMap != null) {
                    request.setParams(paramMap);
                    request.setURLParams();
                }
                return request;
            } else {
                LOG.warn("Invalid rest URL : {}", restUrl);
                throw new VTNClientException("Invalid URL");
            }
        } else if (restUrl == null) {
            request = new RestRequest();
            LOG.debug(getServerUrl());
            request.setUrl(getServerUrl() + "/" + "controller/nb/v2/vtn/default/vtns");

            return request;
        } else {
            LOG.warn("Incompatible Object - Unable to find '{}' annotation", RestURL.class.getName());
            throw new VTNClientException("Incompatible Object (Unable to find '" + RestURL.class.getName() + "' annotation)");
        }
    }

    /**
     * parseResponse - parse response which received from the get opertaion.
     * @param target
     * @param request
     * @return The parsed response.
     * @throws IllegalAccessException
     * @throws InstantiationException
     * @throws VTNClientException
     */
    private Object parseResponse(Object target, RestRequest request)
        throws IllegalAccessException, InstantiationException,
               VTNClientException {
        if (request.getResponse() != null && !request.getResponse().equals("")
                && !request.getResponse().equals("null")) {
            LOG.debug(request.getResponse());
            if (request.getResponse().startsWith("[")) {
                JSONArray jsonArray = new JSONArray(request.getResponse());
                return parser.parseJsonArray(target.getClass(), jsonArray);
            } else {
                String str = request.getResponse();
                JSONObject jsonObject = new JSONObject(str);
                Parser parse = new Parser();
                return parse.parseJsonObject(target, jsonObject);
            }
        } else {
            return target;
        }
    }

    /**
     * getserverUrl - to get the server URL.
     * @return  The server URL.
     */
    public String getServerUrl() {
        return serverUrl;
    }

    /**
     * setServerUrl  to set the server URL.
     * @param serverUrl
     */
    public void setServerUrl(String serverUrl) {
        this.serverUrl = serverUrl;
    }

    /**
     * get the serverIP address.
     * @return {@link String}
     */
    public String getServerIp() {
        return serverIp;
    }

    /**
     * get the serverPort number.
     * @return {@link String}
     */
    public String getServerPort() {
        return serverPort;
    }

    /**
     * validateIP - function to validate the given serverIP address.
     * @param ip
     * @return {@link Boolean}
     */
    private static boolean validateIP(final String ip) {
        String ipAddressPattern =
                "^([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\." +
                        "([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\." +
                        "([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\." +
                        "([01]?\\d\\d?|2[0-4]\\d|25[0-5])$";

        Pattern pattern = Pattern.compile(ipAddressPattern);
        Matcher matcher = pattern.matcher(ip);
        return matcher.matches();
    }

    /**
     * validatePort - function to validate the given serverPort.
     * @param port
     * @return {@link Boolean}
     */
    private static boolean validatePort(final String port) {
        String portPattern =
                "^(\\d\\d?\\d?\\d?)$";

        Pattern pattern = Pattern.compile(portPattern);
        Matcher matcher = pattern.matcher(port);
        return matcher.matches();
    }

    /**
     * Main  - to intiatiate the RESTclient class.
     * @param args
     */
    @SuppressWarnings("resource")
    public static void main(String[] args) {

        try {

            LOG.error("===============================================================");
            LOG.error("\tThis is the starting tag for the current running scenario");
            LOG.error("===============================================================");
            Scanner scanner = new Scanner(new InputStreamReader(System.in));
            System.out.println("Please enter the ODL Controller IP address:");
            String ipAddress = scanner.nextLine();

            if (!(validateIP(ipAddress))) {
                LOG.warn("Invalid IP address - {}", ipAddress);
                throw new VTNClientException("Invalid IPAddress...");
            }

            System.out.println("Please enter the ODL Controller Port to communicate:");
            String port = scanner.nextLine();

            if (!(validatePort(port))) {
                LOG.warn("Invalid port number - {}", port);
                throw new VTNClientException("Invalid port number...");
            }

            System.out.println("Please enter the username of the Controller:");
            String userName = scanner.nextLine();

            System.out.println("Please enter the password of the Controller:");
            String password = scanner.nextLine();

            System.out.println("Please enter the Container name:");
            String container = scanner.nextLine();

            System.out.println("Loading, please wait...");
            RESTClient handler = new RESTClient(ipAddress, port, userName, password);

            List<PathPolicy> pathPolicy = new ArrayList<PathPolicy>();

            VTNManagerVersion ver = (VTNManagerVersion)handler.get(new VTNManagerVersion());

            ContainerPathMapNorthBoundList pathmap = (ContainerPathMapNorthBoundList)handler.get(new ContainerPathMapNorthBoundList(), "containerName", container);

            FlowconditionList flowcondition = (FlowconditionList)handler.get(new FlowconditionList(), "containerName", container);

            VTenantNorthBoundList vtnList = (VTenantNorthBoundList)handler.get(new VTenantNorthBoundList(), "containerName", container);
            List<VTNConfigurationBean> bean = new ArrayList<VTNConfigurationBean>();
            MacEntryList mc = null;
            VlanNorthBoundList vlan = null;
            VBridgeInterfaceList vbrIf = null;
            VBridgeIfFlowFilterNorthboundList vbrIfFlowfilterListIn = null, vbrIfFlowfilterListOut = null;
            VBridgePortMap  vbrPort = null;
            VTerminalPortMap vtPort = null;
            VTerminalIfFlowFilterNorthboundList vtFlowfilterList = null, vtFlowfilter = null;
            VTerminalInterfaceNorthboundList vtInterface = null;
            VBridgeFlowFilterList vbrFlowFilterOut = null, vbrFlowFilterIn = null;
            if (vtnList.getVtns().size() > 0) {
                for (VTenantNorthBound vtn : vtnList.getVtns()) {
                    String vtnName = vtn.getVtnName();

                    LOG.debug(" == == == == == == == == == == == == == == = RUNNING CONFIGURATION STARTED == == == == == == == == == == == == == == == == == == == == == == == == =  = ");
                    VTNConfigurationBean runConfig = new VTNConfigurationBean();
                    List<VBridgeBean> vbrBean = new ArrayList<VBridgeBean>();
                    List<VTerminalBean> vterminalBean = new ArrayList<VTerminalBean>();

                    DataFlowList dflow = (DataFlowList)handler.get(new DataFlowList(), "containerName", container, "tenantName", vtnName);

                    VTenantFlowFilterList vtnFlowFilter = (VTenantFlowFilterList)handler.get(new VTenantFlowFilterList(), "containerName", container, "tenantName", vtnName);
                    VBridgeNorthBoundList vb = (VBridgeNorthBoundList)handler.get(new VBridgeNorthBoundList(), "containerName", container, "tenantName", vtnName);

                    if (vb.getVbridge().size() > 0) {
                        for (VBridgeNorthBound vbn : vb.getVbridge()) {
                            VBridgeBean vbean = new VBridgeBean();

                            vbean.setName(vbn.getName());
                            vbean.setAgeInterval(vbn.getAgeInterval());
                            vbean.setDescription(vbn.getDescription());
                            vbean.setFaults(vbn.getFaults());
                            vbean.setState(vbn.getState());

                            Map<String, String> vbridgeMap = new HashMap<String, String>();
                            vbridgeMap.put("containerName", "default");
                            vbridgeMap.put("tenantName", vtnName);
                            vbridgeMap.put("bridgeName", vbn.getName());

                            MacMapNorthBound macMap = (MacMapNorthBound)handler.get(new MacMapNorthBound(), vbridgeMap, null);
                            vbean.setMacMap(macMap);

                            mc = (MacEntryList)handler.get(new MacEntryList(), vbridgeMap, null);
                            vbean.setMacentry(mc);

                            vlan = (VlanNorthBoundList)handler.get(new VlanNorthBoundList(), vbridgeMap, null);
                            vbean.setBrvlan(vlan);

                            Map<String, String> vbrFlowfilterInMap = new HashMap<String, String>();
                            vbrFlowfilterInMap.put("containerName", "default");
                            vbrFlowfilterInMap.put("tenantName", vtnName);
                            vbrFlowfilterInMap.put("bridgeName", vbn.getName());
                            vbrFlowfilterInMap.put("listType", "in");

                            vbrFlowFilterIn = (VBridgeFlowFilterList)handler.get(new VBridgeFlowFilterList(), vbrFlowfilterInMap, null);

                            Map<String, String> vbrFlowfilterOutMap = new HashMap<String, String>();
                            vbrFlowfilterOutMap.put("containerName", "default");
                            vbrFlowfilterOutMap.put("tenantName", vtnName);
                            vbrFlowfilterOutMap.put("bridgeName", vbn.getName());
                            vbrFlowfilterOutMap.put("listType", "out");
                            vbrFlowFilterOut = (VBridgeFlowFilterList)handler.get(new VBridgeFlowFilterList(), vbrFlowfilterOutMap, null);

                            if (vbrFlowFilterIn.getFlowfilter().size() > 0) {
                                if (vbrFlowFilterOut.getFlowfilter().size() > 0) {
                                    vbrFlowFilterIn.getFlowfilter().addAll(vbrFlowFilterOut.getFlowfilter());
                                }
                                vbean.setVbrFlowFilter(vbrFlowFilterIn);
                            } else if (vbrFlowFilterOut.getFlowfilter().size() > 0) {
                                vbean.setVbrFlowFilter(vbrFlowFilterOut);
                            }

                            vbrIf = (VBridgeInterfaceList)handler.get(new VBridgeInterfaceList(), vbridgeMap, null);

                            if (vbrIf.getInterfaces().size() > 0) {
                                List<VBridgeInterfaceBean> interfaces = new ArrayList<VBridgeInterfaceBean>();
                                for (VBridgeInterface vbrInter : vbrIf.getInterfaces()) {

                                    VBridgeInterfaceBean iBean = new VBridgeInterfaceBean();
                                    iBean.setDescription(vbrInter.getDescription());
                                    iBean.setName(vbrInter.getName());
                                    iBean.setEnabled(vbrInter.getEnabled());
                                    iBean.setEntityState(vbrInter.getEntityState());
                                    iBean.setState(vbrInter.getState());

                                    Map<String, String> vbrInterfaceMap = new HashMap<String, String>();
                                    vbrInterfaceMap.put("containerName", "default");
                                    vbrInterfaceMap.put("tenantName", vtnName);
                                    vbrInterfaceMap.put("bridgeName", vbn.getName());
                                    vbrInterfaceMap.put("ifName", vbrInter.getName());

                                    vbrPort = (VBridgePortMap)handler.get(new VBridgePortMap(), vbrInterfaceMap, null);
                                    iBean.setPortmap(vbrPort);

                                    Map<String, String> vbrInterflowfilterInMap = new HashMap<String, String>();
                                    vbrInterflowfilterInMap.put("containerName", "default");
                                    vbrInterflowfilterInMap.put("tenantName", vtnName);
                                    vbrInterflowfilterInMap.put("bridgeName", vbn.getName());
                                    vbrInterflowfilterInMap.put("ifName", vbrInter.getName());
                                    vbrInterflowfilterInMap.put("listType", "in");
                                    vbrIfFlowfilterListIn = (VBridgeIfFlowFilterNorthboundList)handler.get(new VBridgeIfFlowFilterNorthboundList(), vbrInterflowfilterInMap , null);

                                    Map<String, String> vbrInterflowfilterOutMap = new HashMap<String, String>();
                                    vbrInterflowfilterOutMap.put("containerName", "default");
                                    vbrInterflowfilterOutMap.put("tenantName", vtnName);
                                    vbrInterflowfilterOutMap.put("bridgeName", vbn.getName());
                                    vbrInterflowfilterOutMap.put("ifName", vbrInter.getName());
                                    vbrInterflowfilterOutMap.put("listType", "out");
                                    vbrIfFlowfilterListOut = (VBridgeIfFlowFilterNorthboundList)handler.get(new VBridgeIfFlowFilterNorthboundList(), vbrInterflowfilterOutMap, null);

                                    if (vbrIfFlowfilterListIn.getFlowfilters().size() > 0) {
                                        if (vbrIfFlowfilterListOut.getFlowfilters().size() > 0) {
                                            vbrIfFlowfilterListIn.getFlowfilters().addAll(vbrIfFlowfilterListOut.getFlowfilters());
                                        }
                                        iBean.setFlowfilters(vbrIfFlowfilterListIn);
                                    } else if (vbrIfFlowfilterListOut.getFlowfilters().size() > 0) {
                                        iBean.setFlowfilters(vbrIfFlowfilterListOut);
                                    }
                                    interfaces.add(iBean);
                                }
                                vbean.setVbrInterface(interfaces);
                            }
                            vbrBean.add(vbean);
                        }
                    }
                    VTerminalNorthBoundList vTerminal = (VTerminalNorthBoundList)handler.get(new VTerminalNorthBoundList(), "containerName", container, "tenantName", vtnName);
                    if (vTerminal.getVterm().size() > 0) {
                        for (VTerminalNorthBound vTerminalNorthBound : vTerminal.getVterm()) {
                            VTerminalBean vtbean = new VTerminalBean();
                            vtbean.setName(vTerminalNorthBound.getName());
                            vtbean.setFaults(vTerminalNorthBound.getFaults());
                            vtbean.setDescription(vTerminalNorthBound.getDescription());
                            vtbean.setState(vTerminalNorthBound.getState());

                            Map<String, String> vterminalMap = new HashMap<String, String>();
                            vterminalMap.put("containerName", "default");
                            vterminalMap.put("tenantName", vtnName);
                            vterminalMap.put("terminalName", vTerminalNorthBound.getName());
                            vtInterface = (VTerminalInterfaceNorthboundList)handler.get(new VTerminalInterfaceNorthboundList(), vterminalMap, null);
                            if (vtInterface.getInterfaces().size() > 0) {
                                List<VTerminalInterfaceBean> vtinterfaces = new ArrayList<VTerminalInterfaceBean>();
                                for (VTerminalInterfaceNorthbound vtInter : vtInterface.getInterfaces()) {

                                    VTerminalInterfaceBean vtInterfaceBean = new VTerminalInterfaceBean();
                                    vtInterfaceBean.setDescription(vtInter.getDescription());
                                    vtInterfaceBean.setName(vtInter.getName());
                                    vtInterfaceBean.setEnabled(vtInter.getEnabled());
                                    vtInterfaceBean.setEntityState(vtInter.getEntityState());
                                    vtInterfaceBean.setState(vtInter.getState());

                                    Map<String, String> vInterfaceMap = new HashMap<String, String>();
                                    vInterfaceMap.put("containerName", "default");
                                    vInterfaceMap.put("tenantName", vtnName);
                                    vInterfaceMap.put("terminalName", vTerminalNorthBound.getName());
                                    vInterfaceMap.put("ifName", vtInter.getName());
                                    vtPort = (VTerminalPortMap)handler.get(new VTerminalPortMap(), vInterfaceMap, null);
                                    vtInterfaceBean.setPortmap(vtPort);

                                    Map<String, String> vInterflowfilterInMap = new HashMap<String, String>();
                                    vInterflowfilterInMap.put("containerName", "default");
                                    vInterflowfilterInMap.put("tenantName", vtnName);
                                    vInterflowfilterInMap.put("terminalName", vTerminalNorthBound.getName());
                                    vInterflowfilterInMap.put("ifName", vtInter.getName());
                                    vInterflowfilterInMap.put("listType", "in");
                                    vtFlowfilterList = (VTerminalIfFlowFilterNorthboundList)handler.get(new VTerminalIfFlowFilterNorthboundList(), vInterflowfilterInMap , null);

                                    Map<String, String> vInterflowfilterOutMap = new HashMap<String, String>();
                                    vInterflowfilterOutMap.put("containerName", "default");
                                    vInterflowfilterOutMap.put("tenantName", vtnName);
                                    vInterflowfilterOutMap.put("terminalName", vTerminalNorthBound.getName());
                                    vInterflowfilterOutMap.put("ifName", vtInter.getName());
                                    vInterflowfilterOutMap.put("listType", "out");
                                    vtFlowfilter = (VTerminalIfFlowFilterNorthboundList)handler.get(new VTerminalIfFlowFilterNorthboundList(), vInterflowfilterOutMap, null);

                                    if (vtFlowfilterList.getFlowfilters().size() > 0) {
                                        if (vtFlowfilter.getFlowfilters().size() > 0) {
                                            vtFlowfilterList.getFlowfilters().addAll(vtFlowfilter.getFlowfilters());
                                        }
                                        vtInterfaceBean.setVtFlowfilter(vtFlowfilterList);
                                    } else if (vtFlowfilter.getFlowfilters().size() > 0) {
                                        vtInterfaceBean.setVtFlowfilter(vtFlowfilter);
                                    }
                                    vtinterfaces.add(vtInterfaceBean);
                                }
                                vtbean.setVInterface(vtinterfaces);
                            }
                            vterminalBean.add(vtbean);
                        }
                    }
                    runConfig.setName(vtn.getVtnName());
                    runConfig.setDescription(vtn.getVtnDesc());
                    runConfig.setIdleTimeout(vtn.getIdleTimeout());
                    runConfig.setHardTimeout(vtn.getHardTimeout());
                    runConfig.setDataflow(dflow);
                    runConfig.setVtnFlowfilter(vtnFlowFilter);
                    runConfig.setVbridge(vbrBean);
                    runConfig.setVTerminal(vterminalBean);
                    bean.add(runConfig);
                    LOG.debug(" == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == = ");
                }
            }

            PathPolicy policy = null;
            PathPoliciesIndex policyIndex = (PathPoliciesIndex)handler.get(new PathPoliciesIndex(), "containerName", container);
            if (policyIndex.getIntegers().size() > 0) {
                for (Index index  : policyIndex.getIntegers()) {
                    index.getValue();
                    policy = (PathPolicy)handler.get(new PathPolicy(), "containerName", container, "policyId", "" + index.getValue());
                    pathPolicy.add(policy);
                    LOG.debug("PATHPOLICY:" + pathPolicy);
                }
            }

            VTNConfiguration conf = new VTNConfiguration();
            conf.setVersion(ver);
            conf.setFlowconditions(flowcondition);
            conf.setPathmap(pathmap);
            conf.setPathPolicies(pathPolicy);
            conf.setVtn(bean);
            if (handler.jsonFormating(conf)) {
                System.out.println("Finished");
            }
        } catch (VTNClientException e) {
            System.out.println(e.getStatus());
            System.out.println("\nFor more information, please see the logfile");
        } catch (Exception e) {
            System.out.println("\n\nAn error has occured...\n\nFor more information, please see the logfile");
            LOG.error("An exception occured - ", e);
        }
    }

    /**
     * jsonFormatting - to format the output file as a JSON format.
     * @param jsonVal
     */
    private boolean jsonFormating(Object jsonVal) throws VTNClientException {
        Object json  = jsonVal;
        if (json != null) {
            try {
                System.out.println("Process completed...");
                System.out.println("Creating output file - runConfig_output.json");
                Gson gson = new GsonBuilder().setPrettyPrinting().create();
                String prettyJson = gson.toJson(json);
                FileWriter fw = new FileWriter(new File("runConfig_output.json"));
                fw.write(prettyJson);
                fw.close();
            } catch (IOException ex) {
                LOG.error("An exception occured - ", ex);
                throw new VTNClientException("\n\nAn error has occured in writing output file...");
            }
        }
        return true;
    }
}
