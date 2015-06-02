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
   * ODL_SCVMM_PROVIDER.zip(This zip file is generated inside "application/ODL_VSEM_PROVIDER_SRC/target/" directory.)
   * ODL_SCVMM_PROVIDER.zip file needs to be copied to SCVMM machine.
   * Unzip the ODL_SCVMM_PROVIDER.zip file anywhere in your SCVMM machine.
   * Stop SCVMM service from "service manager->tools->servers->select system center virtual machine manager" and click stop.
   * Go to "C:/Program Files" in your SCVMM machine. Inside "C:/Program Files", create a folder named as "ODLProvider".
   * Inside "C:/Program Files/ODLProvider", create a folder named as "Module" in your SCVMM machine.
   * Inside "C:/Program Files/ODLProvider/Module", Create two folders named as "Odl.VSEMProvider" and "VSEMOdlUI" in your SCVMM machine.
   * Copy the "VSEMOdl.dll" file from "ODL_SCVMM_Provider/ODL_VSEM_PROVIDER" to "C:/Program Files/ODLProvider/Module/Odl.VSEMProvider" in your SCVMM machine.
   * Copy the "VSEMOdlUI.dll" file from "ODL_SCVMM_Provider/ODL_VSEM_PROVIDER_UI" to "C:/Program Files/ODLProvider/Module/VSEMOdlUI" in your SCVMM machine.
   * Copy the "reg_entry.reg" file from "ODL_SCVMM_Provider/Register_settings" to your SCVMM desktop and double click the "reg_entry.reg" file to install registry entry in your SCVMM machine.
   * Download "PF1000.msi" from this link "https://www.pf-info.com/License/en/index.php?url=index/index_non_buyer" and place into "C:/Program Files/Switch Extension Drivers" in your SCVMM machine.
   * Start SCVMM service from "service manager->tools->servers->select system center virtual machine manager" and click start.
