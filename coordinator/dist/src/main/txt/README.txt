VTN COORDINATOR GUIDE
=====================

SUPPORTED PLATFORMS
-------------------

 * RHEL 6 / 7 (x86_64)
 * CentOS 6 / 7 (x86_64)
 * Fedora 20 (x86_64)

INSTALLATION
------------

### INSTALLING VTN COORDINATOR

1. The VTN Coordinator is available in the "externalapps" directory of the
   Karaf distribution as the tarball named:

   distribution.vtn-coordinator-%VTN_COORDINATOR_VERSION%-bin.tar.bz2

   If you want to run the VTN Coordinator on a different machine, copy the
   tarball to the target machine.

2. Extract the VTN Coordinator tarball under the root directory.
   This will install the VTN Coordinator to /usr/local/vtn directory.

    tar -C / -xvjf \
    distribution.vtn-coordinator-%VTN_COORDINATOR_VERSION%-bin.tar.bz2

### INSTALLING JAVA

 * For RHEL/CentOS 6.1 (x86_64)
     Download Oracle JDK 7 and install it.
     http://www.oracle.com/technetwork/java/javase/downloads/index.html

 * For RHEL/CentOS 6.4 (x86_64)
     Install OpenJDK 7.
     yum install java-1.7.0-openjdk-devel

 * Fedora 19/20
     Install OpenJDK 7.
     yum install java-1.7.0-openjdk-devel

### INSTALLING PREREQUISITES

Install additional applications required for VTN Coordinator.

    yum install perl-Digest-SHA uuid libxslt libcurl unixODBC
    wget http://dl.fedoraproject.org/pub/epel/6/i386/epel-release-6-8.noarch.rpm
    rpm -Uvh epel-release-6-8.noarch.rpm
    yum install json-c

### INSTALLING POSTGRESQL DATABASE

Configure Yum repository to download the latest rpms for PostgreSQL 9.1

    rpm -ivh http://yum.postgresql.org/9.1/redhat/rhel-6-x86_64/pgdg-redhat91-9.1-5.noarch.rpm

Install the required PostgreSQL packages.

    yum install postgresql91-libs postgresql91 postgresql91-server \
    postgresql91-contrib postgresql91-odbc

Note:

 1. VTN Coordinator supports postgres version greater than 9.1 only and currently
  tested with 9.1 and 9.3. Please ensure the postgres version>=9.1 is installed.
 2. Please install the rpm for the particular platform from
   http://yum.postgresql.org/9.1/ or http://yum.postgresql.org/9.3/

If you are facing any problems while installing postgreSQL rpm,
please refer to openssl_problems query in troubleshooting FAQ.

https://wiki.opendaylight.org/view/OpenDaylight_Virtual_Tenant_Network_(VTN):Installation:Troubleshooting#Problems_while_Installing_PostgreSQL_due_to_openssl

### Configure Database for VTN Coordinator

    /usr/local/vtn/sbin/db_setup

LAUNCH VTN COORDINATOR
----------------------

    /usr/local/vtn/bin/vtn_start

TEST AND USE VTN COORDINATOR
----------------------------

### RUN OPENDAYLIGHT CONTROLLER
VTN Coordinator depends on virtual network function of OpenDaylight controllers.
Therefore, you need to run OpenDaylight controllers, and enable VTN features.

1. Run an OpenDaylight controller.

    bin/karaf

2. Install VTN Manager.

    feature:install odl-vtn-manager-rest

### TEST VTN COORDINATOR

1. The below command should yield such a response to ensure successful
   installation.

    curl -X GET -H 'content-type: application/json' -u admin:adminpass \
    http://<VTN_COORDINATOR_IP_ADDRESS>:8083/vtn-webapi/api_version.json

   The response should be:
     {"api_version":{"version":"VX.X"}}
   The API version will be returned.

2. Create and use virtual networks.
   Please refer to the below URI for all the API details to create VTN and
   all its sub components.

     https://wiki.opendaylight.org/view/OpenDaylight_Virtual_Tenant_Network_%28VTN%29:VTN_Coordinator:RestApi

TERMINATE VTN COORDINATOR
-------------------------

    /usr/local/vtn/bin/vtn_stop
