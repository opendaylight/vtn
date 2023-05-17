OpenDaylight Virtual Tenant Network (VTN)
=========================================

## Overview

OpenDaylight VTN provides multi-tenant virtual network functions on
OpenDaylight controller. OpenDaylight VTN consists of two parts:
VTN Coordinator and VTN Manager.

Please check wiki pages of OpenDaylight VTN project for more details:
https://wiki.opendaylight.org/view/OpenDaylight_Virtual_Tenant_Network_(VTN):Main

The requirements for installing these two are different. Therefore, we
recommend that you install VTN Manager and VTN Coordinator in different
machines.

## VTN Manager

VTN Manager is a set of OSGi bundles running in OpenDaylight Controller.
Current VTN Manager supports only OpenFlow switches. It handles PACKET_IN
messages, sends PACKET_OUT messages, manages host information, and installs
flow entries into OpenFlow switches to provide VTN Coordinator with virtual
network functions. An OpenDaylight Controller(ODC) Plugin that interacts with
other modules to implement the components of the VTN model. It also provides a REST
interface to configure VTN components in ODL controller. VTN Manager is implemented
as one plugin to the OpenDaylight controller.This provides a REST interface to
create/update/delete VTN components.The user command in VTN Coordinator is
translated as REST CONF to VTN Manager by the ODC Driver component.


 - How to build and run VTN Manager
   [https://wiki.opendaylight.org/view/OpenDaylight_Virtual_Tenant_Network_(VTN):Manager:Hacking](https://github.com/kirazero17/odl_neo_vtn/blob/legacy/post-oxygen/hydrogen_install)
 - To trouble shoot any issues while building
   (Link is dead ://)


## VTN Coordinator

VTN Coordinator orchestrates multiple OpenDaylight Controllers, and provides
applications with VTN API. It is an external application that provides a REST
interface for a user to use the VTN Virtualization.
It interacts with VTN Manager Plugin to implement the user configuration.
It realizes Virtual Tenant Network (VTN) provisioning in OpenDaylight Controller.
In the OpenDaylight architecture VTN Coordinator is part of the network application,
orchestration and services layer.VTN Coordinator will use the REST interface exposed by
the VTN Manger to realize the virtual network using the OpenDaylight controller.
It provides REST APIs for northbound VTN applications and supports virtual networks
spanning across multiple ODC by coordinating across ODC.

 - How to build and run VTN Coordinator
   [https://wiki.opendaylight.org/view/OpenDaylight_Virtual_Tenant_Network_(VTN):Installation:VTN_Coordinator](https://github.com/kirazero17/odl_neo_vtn/blob/legacy/post-oxygen/hydrogen_install)
 - For Coordinator RESTAPI please refer
   https://docs.opendaylight.org/en/stable-oxygen/user-guide/virtual-tenant-network-(vtn).html#rest-api
