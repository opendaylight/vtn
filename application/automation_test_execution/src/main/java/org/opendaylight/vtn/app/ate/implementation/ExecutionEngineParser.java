/**
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.ate.implementation;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;

import javax.xml.bind.JAXBContext;
import javax.xml.bind.JAXBException;
import javax.xml.bind.Marshaller;
import javax.xml.bind.Unmarshaller;

import org.opendaylight.vtn.app.ate.beans.executionengine.TestCase;
import org.opendaylight.vtn.app.ate.beans.executionengine.TestMappingClass;
import org.opendaylight.vtn.app.ate.beans.executionengine.TestSuite;
import org.opendaylight.vtn.app.ate.beans.executionengine.TestSuiteList;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Creating a list of Testsuite and Testcase based on Testmapping.xml
 * and Testcase defined in the Testsuite .yml input folder
 */
@SuppressWarnings("restriction")
public class ExecutionEngineParser {

    /**
     * Logger instance.
     */
    static final Logger LOG = LoggerFactory.getLogger(ExecutionEngineParser.class);

    public ExecutionEngineParser() {
    }

    public TestSuiteList getTestSuiteList(String folderName) {
        try {
            ExecutionEngineParser parser = new ExecutionEngineParser();
            TestMappingClass testMappingClasses = parser.readTestMappingClasses();

            TestSuiteList testSuiteList = parser.createTestSuiteList(testMappingClasses, folderName);

            parser.writeTestExecuterFile(testSuiteList);
            return testSuiteList;
        } catch (Exception e) {
            LOG.error("Exception in creating List of TestSuites and TestCases for ATE - {}", e.getMessage());
        }
        return null;
    }

    private void writeTestExecuterFile(TestSuiteList testSuiteList) throws JAXBException, IOException {
        JAXBContext context = JAXBContext.newInstance(TestSuiteList.class);

        Marshaller marshaller = context.createMarshaller();
        marshaller.setProperty(Marshaller.JAXB_FORMATTED_OUTPUT, true);

        marshaller.marshal(testSuiteList, new FileWriter(new File("TestExecution.xml")));
    }

    public TestMappingClass readTestMappingClasses() throws JAXBException, FileNotFoundException {
        JAXBContext context = JAXBContext.newInstance(TestMappingClass.class);

        Unmarshaller unmarshaller = context.createUnmarshaller();
        return (TestMappingClass)unmarshaller.unmarshal(new FileReader(new File(TestMappingClass.class.getSimpleName() + ".xml")));
    }

    private TestSuiteList createTestSuiteList(TestMappingClass testMappingClasses, String fileName) {
        File rootFile = new File(fileName);

        if (!rootFile.isDirectory()) {
            System.out.println("The specified path is not a directory..");
            return null;
        }

        TestSuiteList testSuiteList = new TestSuiteList();
        for (File file:rootFile.listFiles()) {

            if (file.isDirectory()) {
                TestSuite testSuite = new TestSuite();
                testSuite.setName(file.getName());

                for (File subFile:file.listFiles()) {
                    if (subFile.isFile() && (subFile.getName().lastIndexOf(".yml") >= 0)) {
                        String testCaseName = subFile.getName().substring(0, subFile.getName().lastIndexOf("."));
                        if (validateTestCase(testMappingClasses, testSuite.getName(), testCaseName)) {
                            TestCase testCase = new TestCase();
                            testCase.setName(testCaseName);

                            testSuite.getTestCaseList().add(testCase);
                        }
                    }
                }

                if (testSuite.getTestCaseList().size() > 0) {
                    testSuiteList.getTestSuiteList().add(testSuite);
                }
            }
        }
        return testSuiteList;
    }

    private boolean validateTestCase(TestMappingClass testMappingClasses, String testSuiteName, String testCaseName) {
        for (TestSuite testSuite : testMappingClasses.getTestSuiteList()) {
            if (testSuite.getName().equals(testSuiteName)) {
                for (TestCase testCase : testSuite.getTestCaseList()) {
                    if (testCase.getName().equals(testCaseName)) {
                        return true;
                    }
                }
                break;
            }
        }
        return false;
    }
}
