VTN AUTOMATION TEST ENGINE (VTN Client - Application)
=====================================================
FUNCTIONAL TEST Scripts for VTN MANAGER

NOTE : Contains full core implementation and supports only one FT case.

Purpose
======
    * Intented to test the VTN Manager through NorthBound APIs.
    * This script will test all the create/read/update/delete of the components
      in the VTN Model.
    * The script will issue requests to the VTN Manager and verify if the same
      VTN Component is created at the controller.


Pre-Requisites
==============
    * A controller must be UP and running.

    To set up a Controller please visit
    https://wiki.opendaylight.org/view/Release/Helium/VTN/User_Guide/OpenStack_Support


Configuration
=============
    * The data files are names with extension 'yml'.
    * Seperate file will be defined for each testcase.
    * Each test case will be aligned and added in there respective testsuite
      folder.
    * Each test suite folder contains more than one test case.
    * The controller related details and other required attributes can be
      edited in there respective testcase file.


How To Use This Application
===========================

1. Download the VTN Source code from git.

2. Navigate to application:
    cd vtn/application/

3. Build the source code under the application folder with:
    mvn clean install

4. After Build Success, please Navigate to the target folder with following
command:
    cd automation_test_execution/target/

5. List the files in the target directory:
    ls

6. VTNAutomationTestExecution.jar will be present.

7. Execute the jar file:
    java -jar VTNAutomationTestExecution.jar

8. It will prompt for controller IP Address, where we need to give the
IP Address and press enter.

9. On a valid IP Address it will prompt for a port address of Controller,
provide a valid port name and press enter.

10. On a valid port number it will prompt for a username and password of the
controller.

11. This will generate a list of TestSuite and TestCase results in a
TestResult.html, in the current directory itself.

Please find the below example on how to run this Application.

##How to run this Application.

[root@u12 target]# java -jar VTNAutomationTestExecution.jar
Please enter the ODL Controller IP address:
10.106.138.192
Please enter the ODL Controller Port to communicate:
8080
Please enter the username of the Controller:
admin
Please enter the password of the Controller:
admin

Loading, please wait...

-------------------------------------------------------------------
                AUTOMATION TEST ENGINE - VTN MANAGER
-------------------------------------------------------------------


TestSuite1
        CreateVTenant......................................SUCCESS

-------------------------------------------------------------------


Result is generated...