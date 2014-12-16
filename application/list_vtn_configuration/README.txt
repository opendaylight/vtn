VTN RUNNING CONFIGURATION (VTN Client - Application)
====================================================

## Overview

This application acts as a client to get all the information about the
VTN Manager present under a single container. The output is saved as a User
readable json file. This application is designed as a user friendly
application, with simple commands we can get the whole VTN data, which saves
lot of time for the users on giving individual curl commands for each data.
Testing is made easier in this tool as the output is generated as user
friendly json file.

This application is developed purely as a Java application. The Rest
Response(s) from VTN Manager will be processed by JSON parser in VTN Client
to an output file runConfig_output.json.

Please check wiki pages of VTN running configuration for more details:
https://wiki.opendaylight.org/view/OpenDaylight_Virtual_Tenant_Network_(VTN):VTN_Client_Application

##Prerequisites:
A controller must be UP and running.

To set up a Controller please visit
https://wiki.opendaylight.org/view/Release/Helium/VTN/User_Guide/OpenStack_Support

##How To Use This Application

1. Download the VTN Source code from git.

2. Navigate to application:
    cd vtn/application/

3. Build the source code under the application folder with:
    mvn clean install

4. After Build Success, please Navigate to the target folder with following
command:
    cd list_vtn_configuration/target/

5. List the files in the target directory:
    ls

6. VTNRESTClient.jar will be present.

7. Execute the jar file:
    java -jar VTNRESTClient.jar

8. It will prompt for controller IP Address, where we need to give the
IP Address and press enter.

9. On a valid IP Address it will prompt for a port address of Controller,
provide a valid port name and press enter.

10. On a valid port number it will prompt for a username and password of the
controller.

11. Finally it will prompt for the container name, in current implementation
"default" is the only container name VTN product support and press enter.

12. This will generate a user readable and well formatted JSON file
"runConfig_output.json", in the current directory itself.

Please find the below example on how to run this Application.

##How to run this Application.

[root@u12 target]# java -jar VTNRESTClient.jar
Please Enter the VTN Server IP Address:
10.106.138.192
Please Enter the Port to communicate:
8080
Please enter the username of the Controller:
admin
Please enter the password of the Controller:
admin
Please Enter the Container Name:
default
Loading, please wait...
Process completed...
Creating output file - runConfig_output.json
Finished

##Sample output format.

{
    "version": {
        "api": 2,
        "bundle": {
            "major": 0,
            "minor": 2,
            "micro": 1,
            "qualifier": "Helium-SR1"
        }
    },
    "flowconditions": {
        "flowcondition": [
            {
                "name": "COND_2",
                "match": []
            }
        ]
    },
    "pathPolicies": [
        {
            "id": 1,
            "defaultInt": 100000,
            "costList": []
        }
    ],
    "pathmap": {
        "pathMap": [
            {
                "index": 1,
                "condition": "flowcond_1",
                "policy": 1,
                "idleTimeout": 300,
                "hardTimeout": 0
            }
        ]
    },
    "vtn": [
        {
            "name": "Tenant1",
            "description": "My First Virtual Tenant Network",
            "idleTimeout": 300,
            "hardTimeout": 0,
            "vbridge": [],
            "dataflow": {
                "dataFlowlist": []
            }
        }
    ]
}
