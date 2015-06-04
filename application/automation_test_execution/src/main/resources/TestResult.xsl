<!--
#
# Copyright (c) 2015 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:template match="/">
        <html>
            <head>
                <style>
                    table {
                        font-family: "Trebuchet MS", Arial, Helvetica, sans-serif;
                        width: 80%;
                        border-collapse: collapse;
                    }

                    #TestBedInformation {
                        width: 40%;
                        align: center;
                    }

                    td, th {
                        font-size: 1em;
                        border: 2px solid #FFFFFF;
                        background-color: #E6E6E6;
                        padding: 2px 2px 2px 2px;
                    }

                    th {
                        font-size: 1.3em;
                        text-align: center;
                        padding-top: 5px;
                        padding-bottom: 5px;
                        background-color: #8E8E87;
                        color: #FFFFFF;
                    }

                    #SUCCESS {
                        color: #297A29;
                    }

                    #FAILED {
                        color: #CC3300;
                    }

                    #NOT_EXECUTED {
                        color : #FF9900;
                    }

                    #TestSuiteRow tr {
                        border: 1px solid #FF3311;
                    }

                    <!-- Legend-->
                    .legend {
                        list-style: none;
                    }

                    .legend li {
                        float: left;
                        margin-right: 40px;
                    }

                    .legend span {
                        border: 0px solid #FFFFFF;
                        float: left;
                        width: 62px;
                        height: 25px;
                        margin: 2px;
                    }

                    .legend .pass {
                        background-color: #297A29;
                    }

                    .legend .fail {
                        background-color: #CC3300;
                    }

                    .legend .notexecuted {
                       background-color: #FF9900;
                    }
                </style>
            </head>
            <body>
                <h1 style="text-align:center; color:#FF9900;">VTN Automation Test Result(s)</h1>
                <div style="text-align:center;">
                    <table id="TestBedInformation">
                        <tr>
                            <th colspan="2">VTN Testbed Information</th>
                        </tr>
                        <tr>
                            <td><b>Controller IP</b></td>
                            <td>
                                <xsl:value-of select="TestResults/ControllerIP"/>
                            </td>
                        </tr>
                        <tr>
                            <td><b>Container name</b></td>
                            <td>
                                <xsl:value-of select="TestResults/ContainerName"/>
                            </td>
                        </tr>

                        <tr>
                            <td>
                                <b>Number Of Selected Testsuites</b>
                            </td>
                            <td>
                                <xsl:value-of select="count(TestResults/TestSuites/TestSuite)"/>
                            </td>
                        </tr>

                        <tr>
                            <td>
                                <b>Number Of Selected Testcases</b>
                            </td>
                            <td>
                                <xsl:value-of select="TestResults/TotalTestCases"/>
                            </td>
                        </tr>

                        <tr>
                            <td>
                                <b>Number Of Succeeded Testsuites</b>
                            </td>
                            <td>
                                <xsl:value-of select="count(TestResults/TestSuites/TestSuite[Status='SUCCESS'])"/>
                            </td>
                        </tr>

                        <tr>
                            <td>
                                <b>Number Of Succeeded Testcases</b>
                            </td>
                            <td>
                                <xsl:value-of select="TestResults/TotalSucceededTestCases"/>
                            </td>
                        </tr>
                    </table>

                    <br/>
                    <br/>

                    <fieldset style="width:40%;">
                        <legend><b>Legends:</b></legend>
                        <ul class="legend">
                            <li><span class="pass"></span> Pass</li>
                            <li><span class="fail"></span> Fail</li>
                            <li><span class="notexecuted"></span> Not executed</li>
                        </ul>
                    </fieldset>

                    <br/>
                    <br/>

                    <table>
                        <tr>
                            <th>S.No.</th>
                            <th>Test suite name</th>
                            <th>Test cases name</th>
                            <th>Test case status</th>
                            <th>Test case error</th>
                            <th>Test suite status</th>
                        </tr>
                        <xsl:for-each select="TestResults/TestSuites/TestSuite">
                            <xsl:variable name="BackGroundColor" select="Status"/>
                            <tr id="TestSuiteRow">
                                <xsl:variable name="count" select="count(TestCases/TestCase)"/>
                                <td rowspan="{$count + 1}" style="text-align:center">
                                    <xsl:value-of select="position()"/>
                                </td>
                                <td rowspan="{$count + 1}" style="text-align:center">
                                    <b><xsl:value-of select="@Name"/></b>
                                </td>
                                <xsl:choose>
                                    <xsl:when test="$count &gt; 0">
                                        <xsl:for-each select = "TestCases/TestCase">
                                        <tr>
                                            <td>
                                                <xsl:value-of select = "@Name"/>
                                            </td>

                                            <xsl:variable name="status" select="Status"/>
                                            <td id="{$status}">
                                                <xsl:value-of select = "Status"/>
                                            </td>
                                            <td>
                                                <xsl:choose>
                                                    <xsl:when test = "Status = 'NOT_EXECUTED'">
                                                    </xsl:when>
                                                    <xsl:when test = "Error = ''">
                                                        Pass
                                                    </xsl:when>
                                                    <xsl:otherwise>
                                                        <xsl:value-of select = "Error"/>
                                                    </xsl:otherwise>
                                                </xsl:choose>
                                            </td>
                                            <xsl:if test="position() = '1'">
                                                <td rowspan="{$count + 1}" style="text-align:center">
                                                    <b><xsl:value-of select="$BackGroundColor"/></b>
                                                </td>
                                            </xsl:if>
                                        </tr>
                                        </xsl:for-each>

                                    </xsl:when>
                                    <xsl:otherwise>
                                        <td> No test cases</td>
                                        <td/>
                                        <td/>
                                        <td/>
                                    </xsl:otherwise>
                                </xsl:choose>
                            </tr>
                            <tr/>
                            <tr/>
                            <tr/>
                            <tr/>
                        </xsl:for-each>
                    </table>
                </div>
            </body>
        </html>
    </xsl:template>
</xsl:stylesheet>
