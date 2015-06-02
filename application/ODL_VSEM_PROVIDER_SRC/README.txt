#These source files are used to build the VSEM Provider that can be used with SCVMM to enable OpenDaylight as a Network Service

Prerequisites to build VSEM Provider in your machine
====================================================

1. Platforms
   * CentOS/CentOS 6
   * Ubuntu 14.04
   * Fedora 20

2. Installing build tools in Fedora/CentOS
   * rpm --import "http://keyserver.ubuntu.com/pks/lookup?op=get&search=0x3FA7E0328081BFF6A14DA29AA6A19B38D3D831EF"
   * yum-config-manager --add-repo http://download.mono-project.com/repo/centos/
   * Alternatively, We can skip the import of key and make gpgcheck as disabled in the repo file.

   Installing build tools in CentOS 6
   * rpm --import "http://keyserver.ubuntu.com/pks/lookup?op=get&search=0x3FA7E0328081BFF6A14DA29AA6A19B38D3D831EF"
   * yum-config-manager --add-repo http://download.mono-project.com/repo/centos6

   Installing build tools in Ubuntu
   * echo "deb http://download.mono-project.com/repo/debian wheezy-apache24-compat main" | sudo tee -a /etc/apt/sources.list.d/mono-xamarin.list

3. Installing zip in Fedora/CentOS/CentOS 6
   * yum install zip

   Installing zip in ubuntu
   * sudo apt-get install zip

   Installing MONO and NUGET in Fedora/CentOS
   * yum install mono-complete nuget

   Installing MONO and NUGET in CentOS 6
   * yum install epel-release
   * yum install mono-complete
   * rpm -ivh http://download.mono-project.com/repo/centos/RPMS/noarch/nuget-2.8.3+md58+dhx1-0.noarch.rpm

   Installing MONO and NUGET in Ubuntu
   * sudo apt-get install mono-complete nuget

4. Installing maven in Fedora/CentOS/RHEL.
   * Download maven from the following page "http://maven.apache.org/download.cgi" and follow the instructions in the page to install maven in your machine.

5. To build VSEM Provider in your machine, Please run the "mvn install" command on top of the vtn.git directory.

6. The below generated zip file needs to be copied to SCVMM machine
   * ODL_SCVMM_PROVIDER.zip(This zip file is generated inside "/root/vtn/application/ODL_VSEM_PROVIDER_SRC/dist/odl_vsem_provider/" directory.)
