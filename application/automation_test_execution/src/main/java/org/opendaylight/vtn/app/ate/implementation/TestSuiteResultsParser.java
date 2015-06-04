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
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.StringWriter;

import javax.xml.bind.JAXBContext;
import javax.xml.bind.JAXBException;
import javax.xml.bind.Marshaller;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.stream.StreamSource;

import org.opendaylight.vtn.app.ate.beans.testsuitresults.TestCase;
import org.opendaylight.vtn.app.ate.beans.testsuitresults.TestResult;
import org.opendaylight.vtn.app.ate.beans.testsuitresults.TestSuite;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * ATE Engine Result - Result is created based on the Test suite and test case in .html format.
 */
public class TestSuiteResultsParser {

    /**
     * Logger instance.
     */
    static final Logger LOG = LoggerFactory.getLogger(TestSuiteResultsParser.class);

    public TestSuiteResultsParser() {
    }

    @SuppressWarnings("restriction")
    public void creatTestSuiteResultInHtml(TestResult testResult) {
        updateTestSuitesStatus(testResult);
        try {
            JAXBContext context = JAXBContext.newInstance(TestResult.class);

            Marshaller marshaller = context.createMarshaller();
            StringWriter writer = new StringWriter();
            writer.append("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>");
            writer.append("\n<?xml-stylesheet type='text/xsl' href=\"" + TestResult.class.getSimpleName() + ".xsl\" ?>");
            marshaller.setProperty(Marshaller.JAXB_FRAGMENT, true);
            marshaller.marshal(testResult, writer);

            FileWriter source = new FileWriter(new File(TestResult.class.getSimpleName() + ".xml"));
            source.write(writer.toString());
            source.close();

            TransformerFactory tFactory = TransformerFactory.newInstance();

            Transformer transformer = tFactory.newTransformer(new StreamSource(TestResult.class.getSimpleName() + ".xsl"));

            transformer.transform(new StreamSource(TestResult.class.getSimpleName() + ".xml"), new StreamResult(new FileOutputStream(TestResult.class.getSimpleName() + ".html")));
        } catch (JAXBException e) {
            LOG.error("Exception at Test suite result parser - {}", e.getMessage());
        } catch (IOException e) {
            LOG.error("IOException at Test suite result parser - {}", e.getMessage());
        } catch (Exception e) {
            LOG.error("Exception at Test suite result parser - {}", e.getMessage());
        }
    }

    private void updateTestSuitesStatus(TestResult testResult) {
        int totalTestCases = 0;
        int totalSucceededTestCases = 0;

        for (TestSuite testSuite : testResult.getTestSuite()) {
            if (testSuite.getTestCases().size() > 0) {
                testSuite.setTestSuiteStatus(testSuite.testSuccess);
            }

            for (TestCase testCase : testSuite.getTestCases()) {
                totalTestCases++;
                if ((testCase.getStatus().equalsIgnoreCase(testCase.testFailed))
                        ||  (testCase.getStatus().equalsIgnoreCase(testCase.testNotExecuted))) {
                    testSuite.setTestSuiteStatus(testCase.testFailed);
                } else if (testCase.getStatus().equalsIgnoreCase(testCase.testSuccess)) {
                    totalSucceededTestCases++;
                }
            }
        }

        testResult.setTotalTestCases(totalTestCases);
        testResult.setTotalSucceededTestCases(totalSucceededTestCases);
    }
}
