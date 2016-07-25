OpenDaylight Virtual Tenant Network (VTN)
=========================================

## VTN Coordinator Module

The VTN Coordinator is an external application that provides a REST
interface for a user to use the VTN Virtualization.
It interacts with VTN Manager Plugin to implement the user configuration.
It realizes Virtual Tenant Network (VTN) provisioning in OpenDaylight Controller.
In the OpenDaylight architecture VTN Coordinator is part of the network application,
orchestration and services layer.VTN Coordinator will use the REST interface exposed by
the VTN Manger to realize the virtual network using the OpenDaylight controller.
It provides REST APIs for northbound VTN applications and supports virtual networks
spanning across multiple ODC by coordinating across ODC.

Please check wiki pages of OpenDaylight VTN project for more details:
https://wiki.opendaylight.org/view/OpenDaylight_Virtual_Tenant_Network_(VTN):Main


 - How to build and run VTN Coordinator
   https://wiki.opendaylight.org/view/OpenDaylight_Virtual_Tenant_Network_(VTN):Installation:VTN_Coordinator
 - For Coordinator RESTAPI please refer
   https://wiki.opendaylight.org/view/OpenDaylight_Virtual_Tenant_Network_%28VTN%29:VTN_Coordinator:RestApi

