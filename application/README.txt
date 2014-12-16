VTN RUNNING CONFIGURATION (VTN Client - Application)
=========================================

## Overview

This application acts as a client to get all the information about the VTN present under a single container. The output obtained will be saved as a User readable json file.  This application is designed as a user friendly application ,with simple commands we can get the whole VTN  data, which save lot of time for the users on giving individual curl commands for each data. Testing will also be easy in this tool as the output is generated as user friendly json file.

This application is designed purely as a Java/J2EE application, to process the Rest Responses from VTN Manager. The Rest responses is processed by the JSON Parser and then pass the output to third party JSON Formater which writes it to an output file runConfig_output.json .

##Prerequisites:
A controller must be UP and running.

Please visit https://wiki.opendaylight.org/view/Release/Helium/VTN/User_Guide/OpenStack_Support to set up a Controller.

##How To Use This Application

1. Download the VTN Source code from git.

2. Navigate to Application Folder: 
     cd vtn/application/
     
3. Build the source code under the application folder with:
    mvn install 
    
4. After Build Success, Please Navigate to the target folder with following command:
 cd vtn_running_app/target/

5. List the files in the target directory:
   ls 
   
6. VTNRESTClient.jar will be present.

7. Execute the jar file :
  java -jar VTNRESTClient.jar
  
8. It will prompt for controller IP Address, where we need to give the IP Address and press enter .
  
9. On a valid IP Address it will prompt for a port address of Controller, provide a valid port name and press enter.

10. Finally it will prompt for the container name, In current implementation "default" is the only container name VTN product support and press enter.

11. This will generate a user readable and well formatted JSON file "runConfig_output.json" ,in the current directory itself.

Please find the below example on how to run this Application.

##How to run this Application.

[root@u12 target]# java -jar VTNRESTClient.jar
Please Enter the VTN Server IP Address:
10.106.138.192
Please Enter the Port to communicate:
8080
Please Enter the Container Name:
default

