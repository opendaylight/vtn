/**
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.ate.testengine;

import java.io.InputStreamReader;
import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Scanner;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.opendaylight.vtn.app.ate.api.MapParser;
import org.opendaylight.vtn.app.ate.api.YmlReader;
import org.opendaylight.vtn.app.ate.beans.executionengine.TestCase;
import org.opendaylight.vtn.app.ate.beans.executionengine.TestMappingClass;
import org.opendaylight.vtn.app.ate.beans.executionengine.TestSuite;
import org.opendaylight.vtn.app.ate.beans.executionengine.TestSuiteList;
import org.opendaylight.vtn.app.ate.beans.testsuitresults.TestResult;
import org.opendaylight.vtn.app.ate.implementation.ExecutionEngineParser;
import org.opendaylight.vtn.app.ate.implementation.ExtendedRestClient;
import org.opendaylight.vtn.app.ate.implementation.MapParserImpl;
import org.opendaylight.vtn.app.ate.implementation.TestSuiteResultsParser;
import org.opendaylight.vtn.app.ate.implementation.YmlReaderImpl;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonArray;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonElement;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObjectRef;
import org.opendaylight.vtn.app.run.config.rest.client.VTNClientException;
import org.opendaylight.vtn.app.run.config.rest.response.beans.VTNManagerVersion;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Automated test execution engine.
 */
public final class AutomatedTestExecutor {

    /**
     * Logger instance.
     */
    static final Logger LOG = LoggerFactory.getLogger(AutomatedTestExecutor.class);

    /**
     * RestClient handler.
     */
    private ExtendedRestClient handler;

    /**
     * MapParser - Converts Map to objects.
     */
    private MapParser mapParser;

    /**
     * Test suite executing engine.
     */
    private ExecutionEngineParser executionEngineParser;

    /**
     * List of Test suite container.
     */
    private TestSuiteList testSuiteList;

    /**
     * TestMappingClass - Contains list of objects for the specifice testcase and test suite.
     */
    private TestMappingClass testMappingClass;

    /**
     * Test result container.
     */
    private TestResult testResult;

    private AutomatedTestExecutor(String serverIp, String serverPort,
            String userName, String password, String ymlInputParentDirectory) throws Exception {
        mapParser = new MapParserImpl();
        executionEngineParser = new ExecutionEngineParser();
        testMappingClass = executionEngineParser.readTestMappingClasses();
        testResult = new TestResult();
        testSuiteList = executionEngineParser.getTestSuiteList(ymlInputParentDirectory);
        testResult.setControllerIP(serverIp);
        handler = new ExtendedRestClient(serverIp, serverPort, userName, password);
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

    public static void main(String[] args) {
        try {
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

            String ymlInputParentDirectory = "Test";
            System.out.println("Loading, please wait...");

            AutomatedTestExecutor ate = new AutomatedTestExecutor(ipAddress, port, userName, password, ymlInputParentDirectory);

            if (ate.validateControllerCredentials()) {
                ate.executeAutomationTest(ymlInputParentDirectory);
            }
        } catch (VTNClientException e) {
            System.out.println(e.getMessage());
            LOG.error("VTNClientException at ATE engine - {}", e.getMessage());
        } catch (Exception e) {
            System.out.println("Error occured, Please see the log file...");
            LOG.error("Exception at ATE engine - {}", e.getMessage());
        }
    }

    private boolean validateControllerCredentials() throws VTNClientException, Exception {
        handler.get(new VTNManagerVersion(), (String)null, null);
        return true;
    }

    private void executeAutomationTest(String ymlInputParentDirectory) {
        String indentationGapWithUnderscore = "-------------------------------------------------------------------";
        String indentationGapWithDot = "..........................................................";
        System.out.println("\n\n" + indentationGapWithUnderscore);
        System.out.println("\t\tAUTOMATION TEST ENGINE - VTN MANAGER");
        System.out.println(indentationGapWithUnderscore + "\n");
        try {
            for (TestSuite testSuite : testSuiteList.getTestSuiteList()) {
                org.opendaylight.vtn.app.ate.beans.testsuitresults.TestSuite testSuiteResult = new org.opendaylight.vtn.app.ate.beans.testsuitresults.TestSuite();
                testSuiteResult.setTestSuiteName(testSuite.getName());
                System.out.println("\n" + testSuite.getName());

                for (TestCase testCase : testSuite.getTestCaseList()) {
                    org.opendaylight.vtn.app.ate.beans.testsuitresults.TestCase testCaseResult = new org.opendaylight.vtn.app.ate.beans.testsuitresults.TestCase();
                    testCaseResult.setName(testCase.getName());

                    List<Object> objects = getListOfRestRequestObjects(testMappingClass, testSuite.getName(), testCase.getName());

                    if (objects != null) {
                        try {
                            YmlReader ymlReader = new YmlReaderImpl(ymlInputParentDirectory + "//" + testSuite.getName() + "//" + testCase.getName() + ".yml");
                            Map<String, Object> ymlDatasInMap = ymlReader.getYmlDatas();
                            if (ymlDatasInMap != null) {
                                if (handlePutOrPostRequest(mapParser, ymlDatasInMap, objects, handler, testCaseResult)) {
                                    if (!verifyTestCase(mapParser, ymlDatasInMap, objects, handler, testCaseResult)) {
                                        testCaseResult.setStatus(testCaseResult.testFailed);
                                    }
                                }
                                deleteCreatedObjects(mapParser, ymlDatasInMap, objects, handler, testCaseResult);
                            } else {
                                testCaseResult.setStatus(testCaseResult.testFailed);
                                testCaseResult.setError("Error - Input format is incorrect in the file");
                            }
                            testSuiteResult.getTestCases().add(testCaseResult);
                        } catch (Exception e) {
                            testCaseResult.setStatus(testCaseResult.testFailed);
                            testCaseResult.setError(e.getMessage());
                            testSuiteResult.getTestCases().add(testCaseResult);
                            LOG.error("Exception - {}", e.getMessage());
                        }
                    }

                    int indentationSize = testCaseResult.getName().length() + testCaseResult.getStatus().length();
                    if (indentationGapWithDot.length() > indentationSize) {
                        System.out.println("\t" + testCaseResult.getName()
                                            + indentationGapWithDot.substring(indentationSize)
                                            + testCaseResult.getStatus());
                    } else {
                        System.out.println("\t" + testCaseResult.getName() + ".." + testCaseResult.getStatus());
                    }
                }

                if (testSuiteResult.getTestCases().size() > 0) {
                    testResult.getTestSuite().add(testSuiteResult);
                }
            }
            new TestSuiteResultsParser().creatTestSuiteResultInHtml(testResult);
            System.out.println("\n" + indentationGapWithUnderscore);
            System.out.println("\n\nResult is generated...");
        } catch (Exception e) {
            System.out.println("\n" + indentationGapWithUnderscore);
            System.out.println("\n\nError occured, Please see the log file...");
            LOG.error("Exception - {}", e.getMessage());
        }
    }

    @SuppressWarnings("unchecked")
    private boolean handlePutOrPostRequest(MapParser mapParser, Map<String, Object> ymlDatasInMap,
            List<Object> objects, ExtendedRestClient handler,
            org.opendaylight.vtn.app.ate.beans.testsuitresults.TestCase testCase) {

        try {
            for (Map<String, Object> datas : (List<Map<String, Object>>)mapParser.getInputDatas(ymlDatasInMap)) {
                Object object = getClassReference(objects, datas);
                if (object != null) {
                    Map<String, Object> datasWithParam = (Map<String, Object>)mapParser.setAnnotatedFields(datas, object);

                    object = datasWithParam.get((Object)object.getClass().getSimpleName());
                    datasWithParam.remove(object.getClass().getSimpleName());

                    Boolean bool = (Boolean)datasWithParam.get(MapParser.POST_COMMAND);
                    boolean isPutCommand = (bool == null) ? false : bool.booleanValue();

                    Map<String, String> paramMap = new LinkedHashMap<String, String>();
                    for (String key : datasWithParam.keySet()) {
                        paramMap.put(key, (String)datasWithParam.get(key));
                    }

                    // Setting default VTN container name from ODL controller
                    if ((testResult.getContainerName() != null)
                            && (paramMap.containsKey(MapParser.CONTAINER_NAME))) {
                        testResult.setContainerName(paramMap.get(MapParser.CONTAINER_NAME));
                    }

                    if (isPutCommand) {
                        handler.put(object, paramMap, null);
                    } else {
                        handler.post(object, paramMap, null);
                    }
                } else {
                    testCase.setStatus(testCase.testFailed);
                    testCase.setError("Error - Invalid Class name or Input not set or Class name not defined in the TestMappingClass.xml");
                    LOG.error("Error - Invalid Class name or Input not set or Class name not defined in the TestMappingClass.xml- {}", testCase);
                    return false;
                }
            }
        } catch (Exception e) {
            testCase.setStatus(testCase.testFailed);
            testCase.setError("Error - " + e.getMessage());
            LOG.error("Exception in Put/Get method - {}", e.getMessage());
            return false;
        }
        return true;
    }

    @SuppressWarnings("unchecked")
    private boolean verifyTestCase(MapParser mapParser, Map<String, Object> ymlDatasInMap,
            List<Object> objects, ExtendedRestClient handler,
            org.opendaylight.vtn.app.ate.beans.testsuitresults.TestCase testCase) {

        try {
            for (Map<String, Object> datas : (List<Map<String, Object>>)mapParser.getVerifyingDatas(ymlDatasInMap)) {
                Object srcObject = getClassReference(objects, datas);
                if (srcObject != null) {
                    Map<String, Object> datasWithParam = (Map<String, Object>)mapParser.setAnnotatedFields(datas, srcObject);

                    srcObject = datasWithParam.get((Object)srcObject.getClass().getSimpleName());
                    datasWithParam.remove(srcObject.getClass().getSimpleName());

                    Map<String, String> paramMap = new LinkedHashMap<String, String>();
                    for (String key : datasWithParam.keySet()) {
                        paramMap.put(key, (String)datasWithParam.get(key));
                    }

                    Object odlObject = handler.get(srcObject.getClass().newInstance(), paramMap, null);
                    if (!compareObjects(srcObject, odlObject)) {
                        testCase.setStatus(testCase.testFailed);
                        testCase.setError("Error - Object is not mapped with the ODL - " + odlObject.getClass().getSimpleName());
                        return false;
                    }
                } else {
                    testCase.setStatus(testCase.testFailed);
                    testCase.setError("Error - Invalid Class name or Input not set or Class name not defined in the TestMappingClass.xml");
                    return false;
                }
            }
        } catch (Exception e) {
            testCase.setStatus(testCase.testFailed);
            testCase.setError("Error - " + e.getMessage());
            LOG.error("Exception in Get method or Verify method - {}", e.getMessage());
            return false;
        }
        testCase.setStatus(testCase.testSuccess);
        return true;
    }

    @SuppressWarnings("unchecked")
    private void deleteCreatedObjects(MapParser mapParser, Map<String, Object> ymlDatasInMap,
            List<Object> objects, ExtendedRestClient handler,
            org.opendaylight.vtn.app.ate.beans.testsuitresults.TestCase testCase) {

        String className = "";
        try {
            for (Map<String, Object> datas : (List<Map<String, Object>>)mapParser.getInputDatas(ymlDatasInMap)) {
                Object object = getClassReference(objects, datas);
                if (object != null) {
                    Map<String, Object> datasWithParam = (Map<String, Object>)mapParser.setAnnotatedFields(datas, object);

                    object = datasWithParam.get((Object)object.getClass().getSimpleName());
                    datasWithParam.remove(object.getClass().getSimpleName());

                    Map<String, String> paramMap = new LinkedHashMap<String, String>();
                    for (String key : datasWithParam.keySet()) {
                        paramMap.put(key, (String)datasWithParam.get(key));
                    }
                    if(paramMap.size() < 3) {
                        handler.delete(object, paramMap, null);
                    }

                } else {
                    if ((testCase.getError() == null) || (testCase.getError().length() == 0)) {
                        testCase.setError("Warning - Object shouldnt be empty in yml file");
                    }
                    return;
                }
            }
        } catch (Exception e) {
            if ((testCase.getError() == null) || (testCase.getError().length() == 0)) {
                testCase.setError("Warning - Object not deleted in the controller - " + className);
            }
            LOG.error("Exception in deleting the object from controller- {}", e.getMessage());
        }
    }

    private Object getClassReference(List<Object> objects, Map<String, Object> datas) {
        for (Object object : objects) {
            if (datas.containsKey(object.getClass().getSimpleName())) {
                return object;
            }
        }

        return null;
    }

    private List<Object> getListOfRestRequestObjects(TestMappingClass testMappingClass, String testSuiteName, String testCaseName) {
        List<Object> objects = new ArrayList<Object>(1);
        try {
            for (TestSuite testSuite : testMappingClass.getTestSuiteList()) {
                if (testSuite.getName().equals(testSuiteName)) {
                    for (TestCase testCase : testSuite.getTestCaseList()) {
                        if (testCase.getName().equals(testCaseName)) {
                            if (testCase.getClasses() != null) {
                                for (String className : testCase.getClasses()) {
                                    objects.add(Class.forName(className).newInstance());
                                }
                            }
                        }
                    }
                }
            }
        } catch (InstantiationException e) {
            LOG.error("InstantiationException in getting list of class from TestMapping class - {}", e.getMessage());
        } catch (IllegalAccessException e) {
            LOG.error("IllegalAccessException in getting list of class from TestMapping class - {}", e.getMessage());
        } catch (ClassNotFoundException e) {
            LOG.error("ClassNotFoundException in getting list of class from TestMapping class - {}", e.getMessage());
        }
        return objects;
    }

    @SuppressWarnings("unchecked")
    private boolean compareObjects(Object object1, Object object2) throws IllegalArgumentException, IllegalAccessException {

        if (object1 == object2) {
            return true;
        } else if ((object1 == null) || (object2 == null)) {
            return false;
        } else if (object1.getClass().getSimpleName().equals(object2.getClass().getSimpleName())) {
            for (Field field : object1.getClass().getDeclaredFields()) {
                field.setAccessible(true);

                JsonElement jsonElement = field.getAnnotation(JsonElement.class);
                JsonObjectRef jsonObjectRef = field.getAnnotation(JsonObjectRef.class);
                JsonArray jsonList = field.getAnnotation(JsonArray.class);

                if (jsonElement != null) {
                    if (!field.get(object1).equals(field.get(object2))) {
                        return false;
                    }
                } else if (jsonObjectRef != null) {
                    if (!compareObjects(field.get(object1), field.get(object2))) {
                        return false;
                    }
                } else if (jsonList != null) {
                    if (field.get(object1) == field.get(object2)) {
                        continue;
                    } else if ((field.get(object1) == null) || (field.get(object2) == null)) {
                        return false;
                    } else if ((field.get(object1) != null) && (field.get(object2) != null)) {
                        List<Object> list1 = ((List<Object>)field.get(object1));
                        List<Object> list2 = ((List<Object>)field.get(object2));

                        if (((list1 != null) && (list2 != null))
                                && (list1.size() == list2.size())) {
                            for (Object listObject1 : list1) {
                                int identicalIndex = -1;
                                boolean isListObject1AvailableInListObject2 = false;
                                for (Object listObject2 : list2) {
                                    identicalIndex++;
                                    if ((compareTwoObjectsInTwoDifferentArrays(listObject1, listObject2))) {
                                        isListObject1AvailableInListObject2 = true;
                                        break;
                                    }
                                }

                                if (isListObject1AvailableInListObject2) {
                                    list2.remove(identicalIndex);
                                } else {
                                    // If no two objects are same
                                    return false;
                                }
                            }
                        } else {
                            // If two lists are unequal in size or any one list is null
                            return false;
                        }
                    }
                }
            }
        } else {
            return false;
        }
        return true;
    }

    private boolean compareTwoObjectsInTwoDifferentArrays(Object object1, Object object2) throws IllegalArgumentException, IllegalAccessException {

        if (object1 == object2) {
            return true;
        } else if ((object1 == null) || (object2 == null)) {
            return false;
        } else  if ((object1 instanceof Object)
                && (object2 instanceof Object)) {
            return compareObjects(object1, object2);
        } else {
            return object1.equals(object2);
        }
    }
}
