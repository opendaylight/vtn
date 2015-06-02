//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//
//     This program and the accompanying materials are made available under the
//     terms of the Eclipse Public License v1.0 which accompanies this
//     distribution, and is available at http://www.eclipse.org/legal/epl-v10.html


using System;
using Microsoft.SystemCenter.NetworkService;

namespace ODL.VSEMProvider.Libraries.Common {
    /// <summary>
    /// VSEMOdlConstants class.
    /// This class is the constants class used by the HNV co-existence.
    /// </summary>
    public static class HNVODLConstants {
        /// <summary>
        /// LOGICAL_NETWORK definitions.
        /// </summary>
        public struct LOGICAL_NETWORK {
            /// <summary>
            /// AreLogicalNetworkDefinitionsIsolated property of the LogicalNetwork for HNV OpenFlow network.
            /// </summary>
            public const bool ARE_LOGICALNETWORK_DEFINITIONS_ISOLATED = false;

            /// <summary>
            /// Description property of the LogicalNetwork for HNV OpenFlow network.
            /// </summary>
            public const string DESCRIPTION = "Logical network for HNV networks on OpenFlow networks.";

            /// <summary>
            /// Name property of the LogicalNetwork for HNV OpenFlow network.
            /// </summary>
            public const string NAME = "Odl_HNV_{0}";

            /// <summary>
            /// Id property of the LogicalNetwork for HNV OpenFlow network.
            /// </summary>
            public static Guid LOGICALNETWORK_ID = new Guid("{2CED83ED-B6E4-462A-89E4-2266606A1D0F}");
        }

        /// <summary>
        /// LOGICAL_NETWORK_DEFINITION definitions.
        /// </summary>
        public struct LOGICAL_NETWORK_DEFINITION {
            /// <summary>
            /// AllowsIntraPortCommunication property of the LogicalNetwork for HNV OpenFlow network.
            /// </summary>
            public const bool ALLOWS_INTRAPORT_COMMUNICATION = true;

            /// <summary>
            /// EditableByNonOwners property of the LogicalNetwork for HNV OpenFlow network.
            /// </summary>
            public const bool EDITABLE_BY_NON_OWNERS = true;

            /// <summary>
            /// MarkedForDeletion property of the LogicalNetwork for HNV OpenFlow network.
            /// </summary>
            public const bool MARKED_FOR_DELETION = false;

            /// <summary>
            /// MaximumVMSubnetsPerVMNetwork property of the LogicalNetwork for HNV OpenFlow network.
            /// </summary>
            public const long MAXIMUM_VMSUBNETS_PER_VMNETWORK = 4094;

            /// <summary>
            /// Name property of the LogicalNetwork for HNV OpenFlow network.
            /// </summary>
            public const string NAME = "Odl_HNV_NetworkSite";

            /// <summary>
            /// SegmentType property of the LogicalNetwork for HNV OpenFlow network.
            /// </summary>
            public const NetworkSegmentType SEGMENT_TYPE = NetworkSegmentType.Vlan;

            /// <summary>
            /// SupportsIPSubnetConfigurationOnVMSubnets property of the LogicalNetwork for HNV OpenFlow network.
            /// </summary>
            public const bool SUPPORTS_IPSUBNET_CONFIGURATIONON_VMSUBNETS = true;

            /// <summary>
            /// SupportsVMNetworkProvisioning property of the LogicalNetwork for HNV OpenFlow network.
            /// </summary>
            public const bool SUPPORTS_VMNETWORK_PROVISIONING = true;

            /// <summary>
            /// Id property of the LogicalNetwork for HNV OpenFlow network.
            /// </summary>
            public static Guid ID = new Guid("{EEAC9361-1E2E-4471-987E-141AAFF6410E}");

            /// <summary>
            /// ManagedByNetworkServiceId property of the LogicalNetwork for HNV OpenFlow network.
            /// </summary>
            public static Guid MANAGED_BY_NETWORKSERVICE_ID = VSEMODLConstants.SYSTEM_INFO_ID;
        }

        /// <summary>
        /// IP_SUBNET definitions.
        /// </summary>
        public struct IP_SUBNET {
            /// <summary>
            /// Subnet property of the LogicalNetwork for HNV OpenFlow network.
            /// </summary>
            public const string SUBNET = null;

            /// <summary>
            /// SupportsDHCP property of the LogicalNetwork for HNV OpenFlow network.
            /// </summary>
            public const bool SUPPORTS_DHCP = true;

            /// <summary>
            /// AddressFamily property of the LogicalNetwork for HNV OpenFlow network.
            /// </summary>
            public static readonly AddressFamily? ADDRESS_FAMILY = null;

            /// <summary>
            /// IPSubnet Id property of the LogicalNetwork for HNV OpenFlow network.
            /// </summary>
            public static Guid ID = new Guid("{391F22A7-CCAF-4944-8D5E-640D7DD83CF2}");
        }

        /// <summary>
        /// NETWORK_SEGMENT_IDENTIFIER definitions.
        /// </summary>
        public struct NETWORK_SEGMENT_IDENTIFIER {
            /// <summary>
            /// PrimarySegmentIdentifier property of the LogicalNetwork for HNV OpenFlow network.
            /// </summary>
            public static readonly uint? PRIMARY_SEGMENT_IDENTIFIER = 0;

            /// <summary>
            /// SecondarySegmentIdentifier property of the LogicalNetwork for HNV OpenFlow network.
            /// </summary>
            public static readonly uint? SECONDARY_SEGMENT_IDENTIFIER = null;
        }

        /// <summary>
        /// ERROR_STRING definitions.
        /// </summary>
        public struct ERROR_STRING {
            /// <summary>
            /// Error String that indicates the LogicalNetwork has invalid Network Site.
            /// </summary>
            public const string INVALID_NETWORKSITE =
                "This Logical Network does not support additional network site. Remove the network site ({0}).";
        }
    }
}
