Virtualization Edition Guide for VTN
-------------------------------------

Supported Platforms
-------------------

  * RHEL 6.1 (x86_64)
    Download Oracle JDK 7 and install it.

      http://www.oracle.com/technetwork/java/javase/downloads/index.html

  * RHEL 6.4 (x86_64)
    Install OpenJDK 7 from installation media.

      # yum install java-1.7.0-openjdk-devel

Installation
------------
Installing ODL Controller and Execution
***************************************

1.Unzip the file as follows.
  This will create a directory with name "opendaylight".

    # unzip distributions-virtualization-0.1.0.osgipackage.zip

2. Please ensure the environment variable JAVA_HOME is set to the location
   of the JDK.

3. Execute Controller for VTN using the below command.

    # cd opendaylight
    # ./run.sh -virt vtn

4. The Controller will be up and running with the components required for
   VTN virtualization.

Installing the VTN Coordinator
******************************

1. The VTN Coordinator is available in the "externalapps" directory of the
   virtualization edition as the tarball named
   org.opendaylight.vtn.distribution.vtn-coordinator-5.0.0.0-bin.tar.bz2.

   If you want to run the VTN Coordinator on a different machine, copy the
   tarball to the target machine.

2. Extract the VTN Coordinator tarball under the root directory.
   This will install the VTN Coordinator to /usr/local/vtn directory.

     # tar -C / -xvjf \
       org.opendaylight.vtn.distribution.vtn-coordinator-5.0.0.0-bin.tar.bz2

Launch the VTN Coordinator
--------------------------
Prior Preparation
*****************

Install additional applications required for VTN Coordinator

   # yum install perl-Digest-SHA uuid libxslt libcurl unixODBC
   # wget http://dl.fedoraproject.org/pub/epel/6/i386/epel-release-6-8.noarch.rpm
   # rpm -Uvh epel-release-6-8.noarch.rpm
   # yum install json-c

Installing Postgres Database
****************************

Download the following PostgreSQL 9.1 files from
http://yum.postgresql.org/9.1/redhat/rhel-6-x86_64/ and install them.

    postgresql91-libs-9.1.9-1PGDG.rhel6.x86_64.rpm
    postgresql91-9.1.9-1PGDG.rhel6.x86_64.rpm
    postgresql91-server-9.1.9-1PGDG.rhel6.x86_64.rpm
    postgresql91-contrib-9.1.9-1PGDG.rhel6.x86_64.rpm
    postgresql91-odbc-09.00.0310-1PGDG.rhel6.x86_64.rpm

Install and Configure tomcat
***************************

1. Install Tomcat.

   * Download the Tomcat from the following URI.

     http://archive.apache.org/dist/tomcat/tomcat-7/v7.0.39/bin/apache-tomcat-7.0.39.tar.gz

   * Extract the Tomcat tarball under /usr/share/java.

     # tar -C /usr/share/java -xvzf apache-tomcat-7.0.39.tar.gz 

2. Carry out Tomcat settings.

   * Create the following symbolic link.

     # ln -s /usr/local/vtn/tomcat/webapps/vtn-webapi \
       /usr/share/java/apache-tomcat-7.0.39/webapps/vtn-webapi

   * Add the following to common.loader of
     /usr/share/java/apache-tomcat-7.0.39/conf/catalina.properties.

      /usr/local/vtn/tomcat/lib,/usr/local/vtn/tomcat/lib/*.jar

   * Add the following to shared.loader of
     /usr/share/java/apache-tomcat-7.0.39/conf/catalina.properties.

      /usr/local/vtn/tomcat/shared/lib/*.jar

   * Add the following line into <Server> element in
     /usr/share/java/apache-tomcat-7.0.39/conf/server.xml.

      <Listener className="org.opendaylight.vtn.tomcat.server.StateListener" />

3. If the VTN Coordinator and the controller are deployed in the same server,
   then change the apache port number from 8080 to some other number as
   available in the server. 8080 is the port used by the ODL. The ports
   need to be modified in the server.xml of the tomcat installation.

Configure Database for VTN Coordinator
--------------------------------------

  # /usr/local/vtn/sbin/db_setup

Launch VTN Coordinator to accept requests
-----------------------------------------

  # /usr/local/vtn/bin/vtn_start

Launch tomcat to accept requests
--------------------------------

  # /usr/share/java/apache-tomcat-7.0.39/bin/catalina.sh start

Test and use VTN Coordinator
----------------------------

1. The below command should yield such a response to ensure successful
   installation.

   # curl -X GET -H 'content-type: application/json' -H 'username: admin' \
     -H 'password: adminpass' -H 'ipaddr:127.0.0.1' \
     http://<VTN_COORDINATOR_IP_ADDRESS>:8080/vtn-webapi/api_version.json

     Response
     {"api_version":{"version":"V1.0"}}

2. Create and use VTN
   Please refer to the below URI for all the API details to create VTN and
   all its sub components.

     https://wiki.opendaylight.org/view/OpenDaylight_Virtual_Tenant_Network_%28VTN%29:VTN_Coordinator:RestApi
