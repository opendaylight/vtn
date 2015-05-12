//     Copyright (c) 2013-2014 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html


using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using Microsoft.SystemCenter.NetworkService;
using Microsoft.SystemCenter.NetworkService.VSEM;
using ODL.VSEMProvider.Libraries.Common;
using ODL.VSEMProvider.Libraries.Entity;
using ODL.VSEMProvider.CTRLibraries;
using ODL.VSEMProvider.VSEMEvents;

namespace ODL.VSEMProvider.Libraries
{
    /// <summary>
    /// LogicalNetworkConfig class for HNV.
    /// This class has the function to manage the LogicalNetwork.config file.
    /// </summary>
    public class HNVLogicalNetworkManagement {
        /// <summary>
        /// VTNCoordinatorHostName.
        /// </summary>
        private string VtnHostName { get; set; }

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="VtnHostName">Name of VTNCoordinator host name.</param>
        public HNVLogicalNetworkManagement(string VtnHostName) {
            // Verify arguments.
            if (string.IsNullOrEmpty(VtnHostName)) {
                throw new ArgumentNullException("The parameter 'VTNCoordinatorHostName' is invalid.");
            }

            // Set field.
            this.VtnHostName = VtnHostName;
        }

        /// <summary>
        /// Create default LogicalNetwork and add to configuration.
        /// </summary>
        /// <param name="logicalNetworkConfig">FabricNetworkConfig instance.</param>
        public void CreateHNVLogicalNetwork(LogicalNetworkConfig logicalNetworkConfig) {
            // Verify arguments.
            if (logicalNetworkConfig == null) {
                return;
            }

            if (logicalNetworkConfig.GetLogicalNetworkById(HNVODLConstants.LOGICAL_NETWORK.LOGICALNETWORK_ID) != null) {
                // If exists, nothing to do.
                return;
            }

            ODLVSEMETW.EventWriteGetHNVLogicalNetworkNotFound(MethodBase.GetCurrentMethod().Name,
                string.Format("Create LogicalNetwork that Guid is {0}", HNVODLConstants.LOGICAL_NETWORK.LOGICALNETWORK_ID));

            // Create LogicalNetwork instance.
            var logicalNetwork = new LogicalNetwork() {
                AreLogicalNetworkDefinitionsIsolated =
                    HNVODLConstants.LOGICAL_NETWORK.ARE_LOGICALNETWORK_DEFINITIONS_ISOLATED,
                Description = HNVODLConstants.LOGICAL_NETWORK.DESCRIPTION,
                Id = HNVODLConstants.LOGICAL_NETWORK.LOGICALNETWORK_ID,
                LastModifiedTimeStamp = DateTime.Now,
                LogicalNetworkDefinitions = new LogicalNetworkDefinition[1],
                Name = string.Format(HNVODLConstants.LOGICAL_NETWORK.NAME, this.VtnHostName)
            };
            logicalNetwork.LogicalNetworkDefinitions[0] = this.CreateDefaultLogicalNetworkDefinition();

            // Add created logical network to the configuration.
            logicalNetworkConfig.LogicalNetworks.Add(logicalNetwork);
        }

        /// <summary>
        /// Associate default portprofile with HNV LogicalNetwork.
        /// </summary>
        /// <param name="portProfileConfig">PortProfileConfig instance.</param>
        public void AssociatePortProfileWithHNV(PortProfileConfig portProfileConfig) {
            // Verify arguments.
            if (portProfileConfig == null) {
                return;
            }

            var vsemUplinkPortProfile = portProfileConfig.GetUplinkPortProfileById(VSEMODLConstants.UPLINK_PORT_PROFILE_ID);
            if (vsemUplinkPortProfile == null) {
                // If not exists default UplinkPortProfile, output ETW and return.
                ODLVSEMETW.EventWriteGetHNVUplinkPortProfileNotFound(MethodBase.GetCurrentMethod().Name,
                    string.Format("UplinkPortProfile Guid is {0}", VSEMODLConstants.UPLINK_PORT_PROFILE_ID.ToString()));
                return;
            }

            var tmpGuid = vsemUplinkPortProfile.LogicalNetworkDefinitionIds.FirstOrDefault(l => l.CompareTo(HNVODLConstants.LOGICAL_NETWORK_DEFINITION.ID) == 0);
            if (tmpGuid != Guid.Empty) {
                // If default PortProfile has been associated with HNV LogicalNetwork, return this function.
                return;
            }

            List<Guid> mergedGuidList = new List<Guid>();
            mergedGuidList.AddRange(vsemUplinkPortProfile.LogicalNetworkDefinitionIds);
            mergedGuidList.Add(HNVODLConstants.LOGICAL_NETWORK_DEFINITION.ID);
            vsemUplinkPortProfile.LogicalNetworkDefinitionIds = mergedGuidList.ToArray();

            List<KeyValuePair<string, string>> mergedTagList = new List<KeyValuePair<string, string>>();
            mergedTagList.AddRange(vsemUplinkPortProfile.Tags);
            mergedTagList.Add(new KeyValuePair<string, string>("Network", "HNV"));
            vsemUplinkPortProfile.Tags = mergedTagList.ToArray();

            vsemUplinkPortProfile.LastModifiedTimeStamp = System.DateTime.Now;
        }

        /// <summary>
        /// Verify whether the specified LogicalNetwork is HNV resource or not.
        /// </summary>
        /// <param name="logicalNetwork">LogicalNetwork instance.</param>
        /// <returns>True if HNV resource, else false.</returns>
        public bool IsHNVLogicalNetwork(LogicalNetwork logicalNetwork) {
            if (logicalNetwork == null) {
                return false;
            }

            LogicalNetworkDefinition tmpLogicalNetworkDefinition =
                logicalNetwork.LogicalNetworkDefinitions.FirstOrDefault(l => l.Id == HNVODLConstants.LOGICAL_NETWORK_DEFINITION.ID);
            if (tmpLogicalNetworkDefinition != null
                && tmpLogicalNetworkDefinition.ManagedByNetworkServiceId == VSEMODLConstants.SYSTEM_INFO_ID
                && logicalNetwork.AreLogicalNetworkDefinitionsIsolated == false) {
                return true;
            }
            ODLVSEMETW.EventWriteNotHNVLogicalNetwork(MethodBase.GetCurrentMethod().Name,
                string.Format("LogicalNetwork name is {0}", logicalNetwork.Name));

            return false;
        }

        /// <summary>
        /// Validate whether the specified LogicalNetwork is correct configuration or not.
        /// </summary>
        /// <param name="logicalNetwork">LogicalNetwork instance.</param>
        /// <returns>True if correct configuraton or not HNV resource, else false.</returns>
        public bool IsLogicalNetworkValid(LogicalNetwork logicalNetwork) {
            if (logicalNetwork == null) {
                return false;
            }

            // IsHNV check.
            if (this.IsHNVLogicalNetwork(logicalNetwork) == false) {
                // Not HNV resource.
                return true;
            }

            // Validate NetworkSites.
            bool ret;
            ret = this.IsLogicalNetworkDefinitionsValid(logicalNetwork);

            if (ret == false) {
                ODLVSEMETW.EventWriteValidateLogicalNetworkError(
                    MethodBase.GetCurrentMethod().Name,
                    string.Format("LogicalNetwork name is {0}", logicalNetwork.Name));
            }

            return ret;
        }

        /// <summary>
        /// Update the existing logical network.
        /// </summary>
        /// <param name="txnMng">Transaction manager.</param>
        /// <param name="logicalNetwork">Updated logical network.</param>
        public void UpdateLogicalNetwork(TransactionManager txnMng, LogicalNetwork logicalNetwork) {
            var logicalNetworkConfig = new LogicalNetworkConfig(this.VtnHostName);
            txnMng.SetConfigManager(logicalNetworkConfig, TransactionManager.OpenMode.WriteMode);

            int target;
            target = logicalNetworkConfig.ReplaceLogicalNetwork(logicalNetwork);
            if (target == -1) {
                ODLVSEMETW.EventWriteGetHNVLogicalNetworkNotFound(MethodBase.GetCurrentMethod().Name,
                    string.Format("LogicalNetwork name is {0}", logicalNetwork.Name));
            }
        } 

        /// <summary>
        /// Validate whether the specified LogicalNetwork.LogicalNetworkDefinitions are correct configuration or not.
        /// </summary>
        /// <param name="logicalNetwork">LogicalNetwork instance.</param>
        /// <returns>True if correct configuraton, else false.</returns>
        private bool IsLogicalNetworkDefinitionsValid(LogicalNetwork logicalNetwork) {
            bool ret = true;
            for (int i = 0; i < logicalNetwork.LogicalNetworkDefinitions.Length; i++) {
                LogicalNetworkDefinition tmpLogicalNetworkDefinition = logicalNetwork.LogicalNetworkDefinitions[i];
                if (tmpLogicalNetworkDefinition.Id == HNVODLConstants.LOGICAL_NETWORK_DEFINITION.ID) {
                    continue;
                } else {
                    // Invalid NetworkSite.
                    string ErrorString =
                        string.Format(HNVODLConstants.ERROR_STRING.INVALID_NETWORKSITE, tmpLogicalNetworkDefinition.Name);
                    List<string> msg = new List<string>();
                    msg.Add(ErrorString);
                    tmpLogicalNetworkDefinition.SynchronizationErrors = msg.ToArray();
                    ret = false;
                }
            }
            return ret;
        }
 
        /// <summary>
        /// Create LogicalNetworkDefinition instance.
        /// </summary>
        /// <returns>LogicalNetworkDefinition instance.</returns>
        private LogicalNetworkDefinition CreateDefaultLogicalNetworkDefinition() {
            var logicalNetworkDefinition = new LogicalNetworkDefinition() {
                AllowsIntraPortCommunication = true, EditableByNonOwners = true,
                Id = HNVODLConstants.LOGICAL_NETWORK_DEFINITION.ID, IPSubnets = new IPSubnet[1],
                LastModifiedBySystemId = HNVODLConstants.LOGICAL_NETWORK_DEFINITION.MANAGED_BY_NETWORKSERVICE_ID,
                LastModifiedTimeStamp = DateTime.Now, LogicalNetworkId = HNVODLConstants.LOGICAL_NETWORK.LOGICALNETWORK_ID,
                ManagedByNetworkServiceId = HNVODLConstants.LOGICAL_NETWORK_DEFINITION.MANAGED_BY_NETWORKSERVICE_ID,
                MarkedForDeletion = HNVODLConstants.LOGICAL_NETWORK_DEFINITION.MARKED_FOR_DELETION,
                MaximumVMSubnetsPerVMNetwork = HNVODLConstants.LOGICAL_NETWORK_DEFINITION.MAXIMUM_VMSUBNETS_PER_VMNETWORK,
                Name = HNVODLConstants.LOGICAL_NETWORK_DEFINITION.NAME, SegmentIds = new NetworkSegmentIdentifier[1],
                SegmentType = HNVODLConstants.LOGICAL_NETWORK_DEFINITION.SEGMENT_TYPE,
                SupportsIPSubnetConfigurationOnVMSubnets = HNVODLConstants.LOGICAL_NETWORK_DEFINITION.SUPPORTS_IPSUBNET_CONFIGURATIONON_VMSUBNETS,
                SupportsVMNetworkProvisioning = HNVODLConstants.LOGICAL_NETWORK_DEFINITION.SUPPORTS_VMNETWORK_PROVISIONING
            };
            logicalNetworkDefinition.IPSubnets[0] = this.CreateDefaultIPSubnet();
            logicalNetworkDefinition.SegmentIds[0] = this.CreateDefaultNetworkSegmentIdentifier();
            return logicalNetworkDefinition;
        }

        /// <summary>
        /// Create IPSubnet instance.
        /// </summary>
        /// <returns>IPSubnet instance.</returns>
        private IPSubnet CreateDefaultIPSubnet() {
            var ipSubnet = new IPSubnet {
                AddressFamily = HNVODLConstants.IP_SUBNET.ADDRESS_FAMILY, Id = HNVODLConstants.IP_SUBNET.ID,
                IPAddressPools = null, LastModifiedTimeStamp = DateTime.Now, Subnet = HNVODLConstants.IP_SUBNET.SUBNET,
                SupportsDHCP = HNVODLConstants.IP_SUBNET.SUPPORTS_DHCP
            };
            return ipSubnet;
        }

        /// <summary>
        /// Create NetworkSegmentIdentifier instance.
        /// </summary>
        /// <returns>NetworkSegmentIdentifier instance.</returns>
        private NetworkSegmentIdentifier CreateDefaultNetworkSegmentIdentifier() {
            var networkSegmentIdentifier = new NetworkSegmentIdentifier() {
                PrimarySegmentIdentifier = HNVODLConstants.NETWORK_SEGMENT_IDENTIFIER.PRIMARY_SEGMENT_IDENTIFIER,
                SecondarySegmentIdentifier = HNVODLConstants.NETWORK_SEGMENT_IDENTIFIER.SECONDARY_SEGMENT_IDENTIFIER
            };
            return networkSegmentIdentifier;
        }
    }
}
