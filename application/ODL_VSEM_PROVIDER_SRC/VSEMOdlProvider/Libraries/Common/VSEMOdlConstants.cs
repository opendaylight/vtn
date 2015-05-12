//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html


using System;
using Microsoft.SystemCenter.NetworkService;
using Microsoft.SystemCenter.NetworkService.VSEM;

namespace ODL.VSEMProvider.Libraries.Common {
    /// <summary>
    /// VSEMOdlConstants class.
    /// This class is the constants class used by the ConfigManager.
    /// </summary>
    public static class VSEMODLConstants {
        /// <summary>
        /// Manufacture name.
        /// </summary>
        public const string MANUFACTURE_NAME = "ODL";

        /// <summary>
        /// Config folder name.
        /// </summary>
        public const string CONFIG_FOLDER_NAME = "VSEMProvider";

        /// <summary>
        ///  VSEMInfo MinVTNCoordinatorVersion for vtncoordinator WebAPI version smallest version number.
        /// </summary>
        public const string MIN_VTNCO_VERSION = "1.0";

        /// <summary>
        /// SystemInfo name.
        /// </summary>
        public const string SYSTEM_INFO_NAME = "PF1000 VSEM";

        /// <summary>
        /// SystemInfo description.
        /// </summary>
        public const string SYSTEM_INFO_DESCRIPTION =
            "Virtual Switch Extension Manager for PF1000 virtual switch extension.";

        /// <summary>
        /// SystemInfo manufacturer.
        /// </summary>
        public const string SYSTEM_INFO_MANUFACTURER = "ODL";

        /// <summary>
        /// SystemInfo model.
        /// </summary>
        public const string SYSTEM_INFO_MODEL = "PF1000 VSEM";

        /// <summary>
        /// SwitchExtensionInfo name.
        /// </summary>
        public const string SWITCH_EXTENSION_INFO_NAME = "PF1000 virtual switch extension";

        /// <summary>
        /// VSEMSwitchExtensionInfo switch teaming supported.
        /// </summary>
        public const bool SWITCH_EXTENSION_INFO_SUPPORT_SWITCH_TEAMING = true;

        /// <summary>
        /// Name for switch data VSEMSwitchExtSwitchFeatureConfig.
        /// </summary>
        public const string CONTROLLER_SWITCH_FEATURE_NAME = "Switch Data";

        /// <summary>
        /// WMI class required for switch data VSEMSwitchExtSwitchFeatureConfig.
        /// </summary>
        public const string CONTROLLER_SWITCH_FEATURE_WMI_CLASS_NAME = "PF1000_SwitchData";

        /// <summary>
        /// Property name for switch data VSEMSwitchExtSwitchFeatureConfig.
        /// </summary>
        public const string CONTROLLER_SWITCH_FEATURE_CONTROLLER_PROPERTY = "ControllerAddress";

        /// <summary>
        /// IsArray property of VSEMSwitchExtSwitchFeatureConfig.
        /// </summary>
        public const bool CONTROLLER_SWITCH_FEATURE_CONTROLLER_PROPERTY_IS_ARRAY = false;

        /// <summary>
        /// Name property of the LogicalNetwork for non HNV OpenFlow network.
        /// </summary>
        public const string LOGICAL_NETWORK_NAME = "Odl_";

        /// <summary>
        /// Description property of the LogicalNetwork for non HNV OpenFlow network.
        /// </summary>
        public const string LOGICAL_NETWORK_DESCRIPTION = "Logical Network for Odl network";

        /// <summary>
        /// Name property of the LogicalNetworkDefinitions for non HNV OpenFlow network.
        /// </summary>
        public const string LOGICAL_NETWORK_DEFINITION_NAME = "Odl";

        /// <summary>
        /// Key for the Tag property of VSEMUplinkPortProfile for non HNV OpenFlow network.
        /// </summary>
        public const string UPLINK_PORT_PROFILE_TAG_KEY = "Network";

        /// <summary>
        /// Value for the Tag property of VSEMUplinkPortProfile for non HNV OpenFlow network.
        /// </summary>
        public const string UPLINK_PORT_PROFILE_TAG_VALUE = "Odl";

        /// <summary>
        /// Key for the Tag property of VSEMVirtualPortProfile.
        /// </summary>
        public const string VIRTUAL_PORT_PROFILE_TAG_KEY = "Type";

        /// <summary>
        /// Value for the Tag property of VSEMVirtualPortProfile.
        /// </summary>
        public const string VIRTUAL_PORT_PROFILE_TAG_VALUE = "Odl";

        /// <summary>
        /// Maximum VM Subnets per VM network for OpenFLow lOGICAL network definition.
        /// </summary>
        public const int LOGICAL_NETWORK_DEFINITION_MAX_VM_NETWORK_DEFINITIONS_PER_VM_NETWORK =
            4094;

        /// <summary>
        /// Name property of VSEMUplinkPortProfile for non HNV OpenFlow network.
        /// </summary>
        public const string UPLINK_PORT_PROFILE_NAME = "Odl-Uplink-Port-Profile";

        /// <summary>
        /// ProfileData property of VSEMVirtualPortProfile.
        /// </summary>
        public const uint VIRTUAL_PORT_PROFILE_PROFILE_DATA = 0;

        /// <summary>
        /// Name property of VSEMVirtualPortProfile.
        /// </summary>
        public const string VIRTUAL_PORT_PROFILE_NAME =
            "Odl-Virtual-Port-Profile";

        /// <summary>
        /// MaxNumberOfPorts property of VSEMVirtualPortProfile.
        /// </summary>
        public static long? VIRTUAL_PORT_PROFILE_MAX_NUMBER_OF_PORTS = null;

        /// <summary>
        /// MaxNumberOfPortsPerHost property of VSEMVirtualPortProfile.
        /// </summary>
        public static long? VIRTUAL_PORT_PROFILE_MAX_NUMBER_OF_PORTS_PER_HOST = null;

        /// <summary>
        /// PropertyType of the Property of VSEMSwitchExtSwitchFeatureConfig.
        /// </summary>
        public static PropertyType PROPERTY_TYPE = PropertyType.String;

        /// <summary>
        /// AreLogicalNetworkDefinitionsIsolated property of the LogicalNetwork for non HNV OpenFlow network.
        /// </summary>
        public static bool LOGICAL_NETWORK_ARE_LOGICAL_NETWORK_DEFINITIONS_ISOLATED = true;

        /// <summary>
        /// AllowsIntraPortCommunication property of the LogicalNetworkDefinitions for non HNV OpenFlow network.
        /// </summary>
        public static bool LOGICAL_NETWORK_DEFINITION_ALLOWS_INTRAPORT_COMMUNICATION = false;

        /// <summary>
        /// EditableByNonOwners property of the LogicalNetworkDefinitions for non HNV OpenFlow network.
        /// </summary>
        public static bool LOGICAL_NETWORK_DEFINITION_EDITABLE_BY_NON_OWNERS = false;

        /// <summary>
        /// MarkedForDeletion property of the LogicalNetworkDefinitions for non HNV OpenFlow network.
        /// </summary>
        public static bool LOGICAL_NETWORK_DEFINITION_MARKED_FOR_DELETION = false;

        /// <summary>
        /// SupportsVMNetworkProvisioning property of the LogicalNetworkDefinitions for non HNV OpenFlow network.
        /// </summary>
        public static bool LOGICAL_NETWORK_DEFINITION_SUPPORTS_VM_NETWORK_PROVISIONING = true;

        /// <summary>
        /// SupportsIPSubnetConfigurationOnVMSubnets property of the LogicalNetworkDefinitions for non HNV OpenFlow network.
        /// </summary>
        public static bool LOGICAL_NETWORK_DEFINITION_SUPPORTS_IP_SUBNET_CONFIGURATION_ON_VMSUBNETS = true;

        /// <summary>
        /// Id property of the LogicalNetworkDefinitions for non HNV OpenFlow network.
        /// </summary>
        public static Guid LOGICAL_NETWORK_DEFINITION_ID = new Guid("{c7f3fea8-1a6e-415e-b605-cf4d1d79ffc0}");

        /// <summary>
        /// SegmentIds property of the LogicalNetworkDefinitions for non HNV OpenFlow network.
        /// </summary>
        public static NetworkSegmentIdentifier[] LOGICAL_NETWORK_DEFINITION_SEGMENT_IDS =
            new NetworkSegmentIdentifier[] { new NetworkSegmentIdentifier() };

        /// <summary>
        /// SynchronizationErrors property of the LogicalNetworkDefinitions for non HNV OpenFlow network.
        /// </summary>
        public static string[] LOGICAL_NETWORK_DEFINITION_SYNCHRONIZATION_ERRORS = null;

        /// <summary>
        /// ManagedByNetworkServiceId property of the LogicalNetworkDefinitions for non HNV OpenFlow network.
        /// </summary>
        public static Guid? LOGICAL_NETWORK_DEFINITION_LAST_MODIIED_BY_SYSTEM_ID = null;

        /// <summary>
        /// IPSubnets property of the LogicalNetworkDefinitions for non HNV OpenFlow network.
        /// </summary>
        public static IPSubnet[] LOGICAL_NETWORK_DEFINITION_IP_SUBNETS =
        new[]
                    {
                        new IPSubnet()
                            {
                                AddressFamily = AddressFamily.IPv4,
                                Id = new Guid("{c69ae007-c1b2-440b-b3e0-4cb998ea6ce5}"),
                                Subnet = null,
                                IPAddressPools = new IPAddressPool[0],
                                LastModifiedTimeStamp = DateTime.Now,
                                SupportsDHCP = true
                            }
                    };

        /// <summary>
        /// SegmentType property of the LogicalNetworkDefinitions for non HNV OpenFlow network.
        /// </summary>
        public static NetworkSegmentType LOGICAL_NETWORK_DEFINITION_SEGMENT_TYPE = NetworkSegmentType.Custom;

        /// <summary>
        /// Id property of the NetworkServiceSystemInformation.
        /// </summary>
        public static Guid SYSTEM_INFO_ID = new Guid("{c816cad7-c147-4da4-907e-32b0583d346b}");

        /// <summary>
        /// ReceiveFabricNetworkEntitiesOnRefresh property of NetworkManagerCapabilities in NetworkServiceSystemInformation.
        /// </summary>
        public static bool RECEIVE_FABRIC_NETWORK_ENTITIES_ON_REFRESH = false;

        /// <summary>
        /// ReceiveFabricNetworkEntitiesOnUpdate property of NetworkManagerCapabilities in NetworkServiceSystemInformation.
        /// </summary>
        public static bool RECEIVE_FABRIC_NETWORK_ENTITIES_ON_UPDATE = true;

        /// <summary>
        /// ReceiveVMNetworkEntitiesOnRefresh property of NetworkManagerCapabilities in NetworkServiceSystemInformation.
        /// </summary>
        public static bool RECEIVE_VM_NETWORK_ENTITIES_ON_REFRESH = false;

        /// <summary>
        /// ReceiveVMNetworkEntitiesOnUpdate property of NetworkManagerCapabilities in NetworkServiceSystemInformation.
        /// </summary>
        public static bool RECEIVE_VM_NETWORK_ENTITIES_ON_UPDATE = true;

        /// <summary>
        /// SupportsIPAddressUsageInformation property of NetworkManagerCapabilities in NetworkServiceSystemInformation.
        /// </summary>
        public static bool SUPPORTS_IP_ADDRESS_USAGE_INFORMATION = false;

        /// <summary>
        /// DeviceId property of NetworkServiceSystemInformation.
        /// </summary>
        public static string SYSTEM_INFO_DEVICE_ID = "ODL";

        /// <summary>
        /// SupportsVMNetworkOperations property of VSEMCapabilities in NetworkServiceSystemInformation.
        /// </summary>
        public static bool SYSTEM_INFO_SUPPORTS_VM_NETWORK_OPERATIONS = true;

        /// <summary>
        /// VSEMSystemInfo vendor.
        /// </summary>
        public static string SYSTEM_INFO_VENDOR_ID =
            "{0F4FC573-8A68-40C0-ADDA-1A36268E68DF}";

        /// <summary>
        /// VSEMSystemInfo version.
        /// </summary>
        public static string SYSTEM_INFO_VERSION = "2.0";

        /// <summary>
        /// Unique VSEMSwitchExtensionInfo ID.
        /// </summary>
        public static Guid SWITCH_EXTENSION_INFO_ID =
            new Guid("{35e86f93-9f19-4a75-a6ca-bc1765222451}");

        /// <summary>
        /// VSEMSwitchExtensionInfo minimum supported version.
        /// </summary>
        public static Version SWITCH_EXTENSION_INFO_MIN_VERSION = new Version(1, 0);

        /// <summary>
        /// VSEMSwitchExtensionInfo type.
        /// </summary>
        public static VSEMExtensionType SWITCH_EXTENSION_TYPE = VSEMExtensionType.Forwarding;

        /// <summary>
        /// VSEMSwitchExtensionInfo maximum supported version.
        /// </summary>
        public static Version SWITCH_EXTENSION_INFO_MAX_VERSION = new Version(999, 0);

        /// <summary>
        /// NetCfg instance ID for PF1000.
        /// </summary>
        public static Guid PF1000_NETCFG_INSTANCE_ID =
            new Guid("{E103D967-95FC-42C0-8C2E-2FE5F29E56B7}");

        /// <summary>
        /// Is child of WFP switch extension.
        /// </summary>
        public static bool SWITCH_EXTENSION_INFO_IS_CHILD_OF_WFP_SWITCH_EXTENSION = false;

        /// <summary>
        /// Mendatory feature Id.
        /// </summary>
        public static Guid? SWITCH_EXTENSION_INFO_MANDATORY_FEATURE_ID = null;

        /// <summary>
        /// VSEMSwitchExtensionInfo max no. of ports.
        /// </summary>
        public static long? SWITCH_EXTENSION_INFO_MAX_NUMBER_OF_PORTS = 1280;

        /// <summary>
        /// VSEMSwitchExtensionInfo max no. of ports per host.
        /// </summary>
        public static long? SWITCH_EXTENSION_INFO_MAX_NUMBER_OF_PORTS_PER_HOST = null;

        /// <summary>
        /// Unique ID for switch data VSEMSwitchExtSwitchFeatureConfig.
        /// </summary>
        public static Guid CONTROLLER_SWITCH_FEATURE_ID =
            new Guid("{68caac43-b17a-4ead-9567-294c6c3d7060}");

        /// <summary>
        /// Unique ID for OpenFlow logical network.
        /// </summary>
        public static Guid LOGICAL_NETWORK_ID =
            new Guid("{50033e56-6090-4091-9149-af5c7cb481a7}");

        /// <summary>
        /// Unique ID for OpenFlow uplink port profile.
        /// </summary>
        public static Guid UPLINK_PORT_PROFILE_ID =
            new Guid("{6c05e1ad-9021-40e4-b4b3-c05d2b9b6b58}");

        /// <summary>
        /// Unique ID for OpenFlow virtual port profile.
        /// </summary>
        public static Guid VIRTUAL_PORT_PROFILE_ID =
            new Guid("{351f84b1-a146-4db5-8522-1c290d1691e0}");

        /// <summary>
        /// AllowedVNicType property of the VSEMVirtualPortProfile.
        /// </summary>
        public static VirtualPortProfileAllowedVNicType VIRTUAL_PORT_PROFILE_ALLOWED_VNIC_TYPE =
        VirtualPortProfileAllowedVNicType.Both;
    }
}
