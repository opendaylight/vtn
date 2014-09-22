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

  * Fedora 20
     yum install java-1.7.0-openjdk-devel

Installation
------------

Installing ODL Controller and Execution
***************************************

1.Unzip the karaf distro as follows.
    # unzip distributions-virtualization-%ODL_VIRT_VERSION%.zip

2. Please ensure the environment variable JAVA_HOME is set to the location
   of the JDK.

3. Execute Controller for VTN using the below command.

    # bin/karaf

    Once the prompt appears, Please type the below commands to use VTN Manager
    feature:install odl-vtn-manager-all odl-adsal-compatibility-all odl-openflowplugin-all


4. The Controller will be up and running with the components required for
   VTN virtualization.

Installing the VTN Coordinator
******************************

1. The VTN Coordinator is available in the "externalapps" directory of the
   karaf edition as the tarball named
   org.opendaylight.vtn.distribution.vtn-coordinator-%VTN_COORDINATOR_VERSION%-bin.tar.bz2.

   If you want to run the VTN Coordinator on a different machine, copy the
   tarball to the target machine.

2. Extract the VTN Coordinator tarball under the root directory.
   This will install the VTN Coordinator to /usr/local/vtn directory.

     # tar -C / -xvjf \
       org.opendaylight.vtn.distribution.vtn-coordinator-%VTN_COORDINATOR_VERSION%-bin.tar.bz2

Installing prerequisites
************************

Install additional applications required for VTN Coordinator

   # yum install perl-Digest-SHA uuid libxslt libcurl unixODBC
   # wget http://dl.fedoraproject.org/pub/epel/6/i386/epel-release-6-8.noarch.rpm
   # rpm -Uvh epel-release-6-8.noarch.rpm
   # yum install json-c

Installing PostgreSQL Database
******************************

Download the following PostgreSQL 9.1 files from
http://yum.postgresql.org/9.1/redhat/rhel-6-x86_64/ and install them.

    postgresql91-libs-9.1.9-1PGDG.rhel6.x86_64.rpm
    postgresql91-9.1.9-1PGDG.rhel6.x86_64.rpm
    postgresql91-server-9.1.9-1PGDG.rhel6.x86_64.rpm
    postgresql91-contrib-9.1.9-1PGDG.rhel6.x86_64.rpm
    postgresql91-odbc-09.00.0310-1PGDG.rhel6.x86_64.rpm

Configure Tomcat server
***********************

By default, Tomcat server will listen on 8083/tcp. If you want to change
the listening port, modify the TOMCAT_PORT defined in below file:

    /usr/local/vtn/tomcat/conf/tomcat-env.sh

Configure Database for VTN Coordinator
**************************************

  # /usr/local/vtn/sbin/db_setup

Launch VTN Coordinator
----------------------

  # /usr/local/vtn/bin/vtn_start

Test and use VTN Coordinator
----------------------------

1. The below command should yield such a response to ensure successful
   installation.

   # curl -X GET -H 'content-type: application/json' -u admin:adminpass \
     http://<VTN_COORDINATOR_IP_ADDRESS>:8083/vtn-webapi/api_version.json

     Response
     {"api_version":{"version":"VX.X"}} // The recent version will be returned!!

2. Create and use VTN
   Please refer to the below URI for all the API details to create VTN and
   all its sub components.

     https://wiki.opendaylight.org/view/OpenDaylight_Virtual_Tenant_Network_%28VTN%29:VTN_Coordinator:RestApi

Terminate VTN Coordinator
----------------------

  # /usr/local/vtn/bin/vtn_stop
