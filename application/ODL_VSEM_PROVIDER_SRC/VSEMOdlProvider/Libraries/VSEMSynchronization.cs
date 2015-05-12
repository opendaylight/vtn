//     Copyright (c) 2013-2014 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html


using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Globalization;
using System.Linq;
using System.Management.Automation;
using System.Management.Automation.Runspaces;
using System.Reflection;
using System.Text;
using System.Web.Script.Serialization;
using Microsoft.SystemCenter.NetworkService;
using ODL.VSEMProvider.Libraries.Common;
using ODL.VSEMProvider.Libraries.Entity;
using ODL.VSEMProvider.CTRLibraries;
using ODL.VSEMProvider.CTRLibraries.Entity;
using ODL.VSEMProvider.VSEMEvents;

namespace ODL.VSEMProvider.Libraries {
    /// <summary>
    /// This class is responsible to synchronize the network configuration between ODL and SCVMM.
    /// </summary>
    public class VSEMSynchronization {
        /// <summary>
        /// Error message for the file IO exception.
        /// </summary>
        private static string configFileIOErrorValidationMessage =
            string.Format(CultureInfo.CurrentCulture,
            "{0}\n{1}\n1. {2}\n2. {3}\n3. {4}\n4. {5}",
            "file could not be accessed.",
            "Possible reasons could be:",
            "File is deleted.",
            "File is renamed.",
            "File is tampered.",
            "File is being used by another process.");

        /// <summary>
        /// Connection string to use.
        /// </summary>
        public string ConnectionString {
            get;
            private set;
        }

        /// <summary>
        /// Credential to use.
        /// </summary>
        public PSCredential Credential {
            get;
            private set;
        }

        /// <summary>
        /// Initializes a new instance of the VSEMSynchronization class.
        /// </summary>
        /// <param name="connectionString">Connection string of ODL.</param>
        /// <param name="credential">Creadentials of ODL.</param>
        public VSEMSynchronization(string connectionString,
            PSCredential credential) {
            // Verify arguments
            if (string.IsNullOrEmpty(connectionString)) {
                throw new ArgumentException("The parameter 'connectionString' is invalid.");
            }

            if (credential == null) {
                throw new ArgumentException("The parameter 'credential' is invalid.");
            }

            this.ConnectionString = connectionString;
            this.Credential = credential;
        }

        /// <summary>
        /// Initializes a new instance of the VSEMSynchronization class.
        /// </summary>
        /// <param name="txnMng">Transaction Manager.</param>
        public VSEMSynchronization(TransactionManager txnMng) {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"txnMng\":" + JavaScriptSerializer.Serialize(txnMng));
            ODLVSEMETW.EventWriteStartLibrary(MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                   json.ToString());
            if (txnMng == null) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'transaction' is null or invalid.");
                throw new ArgumentException("The parameter 'transaction' is null or invalid.");
            }
            var vSEMConfig = new VSEMConfig();
            try {
                txnMng.SetConfigManager(vSEMConfig, TransactionManager.OpenMode.ReadMode);
            } catch (Exception ex) {
                ODLVSEMETW.EventWriteConfigManagerFileIOError(
                    MethodBase.GetCurrentMethod().Name,
                    string.Format(CultureInfo.CurrentCulture,
                    "VSEM.config {0}\n",
                    configFileIOErrorValidationMessage) +
                    ex.Message);
                ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name, string.Empty);
                throw new InvalidOperationException(
                    string.Format(CultureInfo.CurrentCulture,
                    "Either the NetworkService is not added in SCVMM or VSEM.config {0}",
                    configFileIOErrorValidationMessage));
            }
            try {
                var odlInformation = new OdlInformation(vSEMConfig.Info.ServerName);
                txnMng.SetConfigManager(odlInformation, TransactionManager.OpenMode.ReadMode);
                this.ConnectionString = odlInformation.GetConnectionString();
                this.Credential = odlInformation.GetCredentials();
            } catch (Exception ex) {
                ODLVSEMETW.EventWriteConfigManagerFileIOError(
                    MethodBase.GetCurrentMethod().Name,
                    string.Format(CultureInfo.CurrentCulture,
                    "ODLInformation.config {0}\n",
                    configFileIOErrorValidationMessage) +
                    ex.Message);
                ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name, string.Empty);
                throw new InvalidOperationException(
                    string.Format(CultureInfo.CurrentCulture,
                    "ODLInformation.config {0}",
                    configFileIOErrorValidationMessage));
            }
        }

        /// <summary>
        /// Remove the VMSubnet corresponding to the removed vBridge from ODL.
        /// </summary>
        /// <param name="vMNetworkConfig">VMNetwork.config file instance.</param>
        /// <param name="vmSubnetId">Id of the VMSubnet.</param>
        /// <param name="vmnetwork">Parent VM Network.</param>
        /// <param name="vmnetworkInfo">Corresponding VM netrok info.</param>
        public static void RemoveVMSubnetSync(VMNetworkConfig vMNetworkConfig, Guid vmSubnetId, VMNetwork vmnetwork, VMNetworkInfo vmnetworkInfo) {
            if (vMNetworkConfig == null && (vmnetwork == null || vmnetworkInfo == null)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'vMNetworkConfig' is null or invalid.");
                throw new ArgumentException("The parameter 'vMNetworkConfig' is null or invalid.");
            }
            if (vmnetwork == null) {
                vmnetwork = vMNetworkConfig.VMNetwork.VmNetworks.FirstOrDefault(net => net.VMSubnets.Any(vms => vms.Id.CompareTo(vmSubnetId) == 0));
            }
            if (vmnetwork != null) {
                var temp = vmnetwork.VMSubnets.ToList();
                temp.RemoveAll(net => net.Id.CompareTo(vmSubnetId) == 0);
                vmnetwork.VMSubnets = temp.ToArray();
                if (vmnetworkInfo == null) {
                    vmnetworkInfo = vMNetworkConfig.VMNetwork.VMNetworkMappingInformation.VMNetworkInfo.FirstOrDefault(vmn => vmn.VMNetworkID.CompareTo(vmnetwork.Id) == 0);
                }
                vmnetworkInfo.VMSubnetInfo.ForEach(vms => {
                    if (vms.VMSubnetID.CompareTo(vmSubnetId) == 0) {
                        vms.Description = "Corresponding vBridge is deleted on ODL.";
                        vms.VBridgeName = string.Empty;
                        vms.VBridgeVlanId = 0;
                    }
                });
            }
        }

        /// <summary>
        /// Remove the VMSubnet corresponding to the removed vBridge from ODL.
        /// </summary>
        /// <param name="vMNetworkConfig">VMNetwork.config file instance.</param>
        /// <param name="networkInfo">Corresponding VM netrok info.</param>
        public static void RemoveVMNetworkSync(VMNetworkConfig vMNetworkConfig, VMNetworkInfo networkInfo) {
            vMNetworkConfig.VMNetwork.VmNetworks.RemoveAll(net => net.Id.CompareTo(networkInfo.VMNetworkID) == 0);
            networkInfo.VTNName = string.Empty;
            networkInfo.Description = "Corresponding VTN is deleted on ODL.";
            networkInfo.VMSubnetInfo.ForEach(net => {
                net.VBridgeName = string.Empty; net.VBridgeVlanId = 0;
                net.Description = "Corresponding vBridge is deleted on ODL.";
            });
        }

        /// <summary>
        /// Completes the sync operation for the removed removed entities.
        /// </summary>
        /// <param name="txnMng">Transaction Manager instance.</param>
        /// <param name="SCVMSubnets">Names of VMSubnets present in SCVMM.</param>
        public void StatusAfterRefreshNetworkService(TransactionManager txnMng, string SCVMSubnets) {
            this.HandleVMSubnetsDependencies(SCVMSubnets.Split(',').ToList(), txnMng);
        }

        /// <summary>
        /// Publish VM Network.
        /// </summary>
        /// <param name="txnMng">TransactionManager instance.</param>
        /// <param name="vMNetwork">VM Network.</param>
        /// <param name="operationType">Operation type performed on SCVMM.</param>
        /// <param name="connection">Connection object.</param>
        /// <returns>Boolean indicating whether network service refresh is needed..</returns>
        public bool PublishVMNetwork(TransactionManager txnMng,
            VMNetwork vMNetwork,
            NetworkEntityPublishType operationType,
            VSEMConnection connection) {
            bool needRefresh = false;
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"txnMng\":" + JavaScriptSerializer.Serialize(txnMng));
            json.Append(" \"vMNetwork\":" + JavaScriptSerializer.Serialize(vMNetwork));
            json.Append(" \"operationType\":" + JavaScriptSerializer.Serialize(operationType));
            ODLVSEMETW.EventWriteStartLibrary(MethodBase.GetCurrentMethod().Name,
                json.ToString());
            if (txnMng == null) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'transaction' is null or invalid.");
                throw new ArgumentException("The parameter 'transaction' is null or invalid.");
            }

            if (vMNetwork == null) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'vMNetwork' is null or invalid.");
                throw new ArgumentException("The parameter 'vMNetworkName' is null or invalid.");
            }
            if (connection == null) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'connection' is null or invalid.");
                throw new ArgumentException("The parameter 'connection' is null or invalid.");
            }

            string VtnHostName = this.ConnectionString;

            if (VtnHostName.StartsWith(@"https://", StringComparison.Ordinal)) {
                VtnHostName = VtnHostName.Substring(8);
            }

            VtnHostName = VtnHostName.Split('.', ':').First();

            var vMNetworkConfig = new VMNetworkConfig(VtnHostName);
            txnMng.SetConfigManager(vMNetworkConfig, TransactionManager.OpenMode.WriteMode);

            var vmnetworkFound = vMNetworkConfig.VMNetwork.VmNetworks.FirstOrDefault(
                nw => nw.Id == vMNetwork.Id);

            var vmnetworkMappigInfoFound = vMNetworkConfig.VMNetwork.VMNetworkMappingInformation.VMNetworkInfo.FirstOrDefault(
                        nw => nw.VMNetworkID == vMNetwork.Id);
            if (vmnetworkMappigInfoFound == null) {
                return needRefresh;
            }

            if (operationType != NetworkEntityPublishType.Delete) {
                vmnetworkMappigInfoFound.VMNetworkName = vMNetwork.Name;
                if (vmnetworkMappigInfoFound.VTNName.CompareTo(string.Empty) == 0) {
                    return needRefresh;
                }
                vmnetworkFound.OwnerName = vMNetwork.OwnerName;
                vmnetworkFound.RoutingDomainId = vMNetwork.RoutingDomainId;
                if (operationType == NetworkEntityPublishType.Create) {
                    foreach (var subnetinfo in vmnetworkMappigInfoFound.VMSubnetInfo) {
                        subnetinfo.VMSubnetName = subnetinfo.VBridgeName;
                        subnetinfo.VMSubnetVlanId = subnetinfo.VBridgeVlanId;
                    }
                    Controller odl = new Controller(this.ConnectionString, this.Credential);
                    List<Vtn> vtns = odl.ReadVTNObjects(vmnetworkMappigInfoFound.VTNName);
                    if (vtns.Count == 0) {
                        needRefresh = true;
                        vMNetworkConfig.VMNetwork.VmNetworks.Remove(vmnetworkFound);
                        vMNetworkConfig.VMNetwork.VMNetworkMappingInformation.VMNetworkInfo.Remove(vmnetworkMappigInfoFound);
                    } if (vtns.Count != 0) {
                        needRefresh = this.SyncVTN(vmnetworkFound.VMSubnets.First().LogicalNetworkDefinitionId,
                    vmnetworkFound,
                    vtns.First(),
                    vmnetworkMappigInfoFound,
                    txnMng,
                    VtnHostName);
                    }
                }
                vmnetworkMappigInfoFound.Description = string.Empty;
            } else {
                if (vmnetworkMappigInfoFound.VTNName.CompareTo(string.Empty) == 0) {
                    vMNetworkConfig.VMNetwork.VMNetworkMappingInformation.VMNetworkInfo.Remove(vmnetworkMappigInfoFound);
                    return needRefresh;
                }
                vmnetworkMappigInfoFound.VMNetworkName = string.Empty;
                vmnetworkMappigInfoFound.VMSubnetInfo.ForEach(subnet => { subnet.VMSubnetName = string.Empty; subnet.VMSubnetVlanId = 0; });
                vmnetworkMappigInfoFound.Description = "VM Network corresponding to this VTN is not created on SCVMM";
                vmnetworkFound.Name = vmnetworkMappigInfoFound.VTNName;
                vmnetworkFound.RoutingDomainId = null;
                vmnetworkFound.OwnerName = string.Empty;
            }

            string output = "\"VMNetwork\":" + JavaScriptSerializer.Serialize(vmnetworkFound);
            string outputMappingInfo = "\"VMNetworkInfo\":" + JavaScriptSerializer.Serialize(vmnetworkMappigInfoFound);
            ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name,
                output);
            return needRefresh;
        }

        /// <summary>
        /// This method is responsible to synchronize the network configuration between ODL and SCVMM.
        /// </summary>
        /// <param name="txnMng">TransactionManager instance.</param>
        public void SynchronizeVTNObjects(TransactionManager txnMng) {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"txnMng\":" + JavaScriptSerializer.Serialize(txnMng));
            ODLVSEMETW.EventWriteStartLibrary(MethodBase.GetCurrentMethod().Name,
                   json.ToString());
            if (txnMng == null) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'transaction' is null or invalid.");
                throw new ArgumentException("The parameter 'transaction' is null or invalid.");
            }
            Controller odl = new Controller(this.ConnectionString, this.Credential);
            List<Vtn> vtns = odl.ReadVTNObjects(string.Empty);

            ODLVSEMETW.EventWriteReturnODLLibrary("Return from ODL Library.", string.Empty);

            string VtnHostName = this.ConnectionString;

            if (VtnHostName.StartsWith(@"https://", StringComparison.Ordinal)) {
                VtnHostName = VtnHostName.Substring(8);
            }

            VtnHostName = VtnHostName.Split('.', ':').First();
            var vMNetworkConfig = new VMNetworkConfig(VtnHostName);
            try {
                txnMng.SetConfigManager(vMNetworkConfig, TransactionManager.OpenMode.WriteMode);
            } catch (Exception ex) {
                ODLVSEMETW.EventWriteConfigManagerFileIOError(
                    MethodBase.GetCurrentMethod().Name,
                    string.Format(CultureInfo.CurrentCulture,
                    "VMNetwork.config {0}\n{1}",
                    configFileIOErrorValidationMessage,
                    ex.Message));
                ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name, string.Empty);
                throw new InvalidOperationException(
                    string.Format(CultureInfo.CurrentCulture,
                    "VMNetwork.config {0}",
                    configFileIOErrorValidationMessage));
            }
            var logicalNetworkConfig = new LogicalNetworkConfig(VtnHostName);
            try {
                txnMng.SetConfigManager(logicalNetworkConfig, TransactionManager.OpenMode.ReadMode);
            } catch (Exception ex) {
                ODLVSEMETW.EventWriteConfigManagerFileIOError(
                    MethodBase.GetCurrentMethod().Name,
                    string.Format(CultureInfo.CurrentCulture,
                    "LogicalNetwork.config {0}\n{1}",
                    configFileIOErrorValidationMessage,
                    ex.Message));
                ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name, string.Empty);
                throw new InvalidOperationException(
                    string.Format(CultureInfo.CurrentCulture,
                    "LogicalNetwork.config {0}",
                    configFileIOErrorValidationMessage));
            } string logicalNetworkName = VSEMODLConstants.LOGICAL_NETWORK_NAME;
            logicalNetworkName += VtnHostName;
            var logicalNetwork = logicalNetworkConfig.LogicalNetworks.FirstOrDefault(logicalnw => logicalnw.Name.Equals(logicalNetworkName));
            if (logicalNetwork == null) {
                ODLVSEMETW.EventWriteGetFabricNetworkDefinitionError(
                    MethodBase.GetCurrentMethod().Name,
                    string.Format(CultureInfo.CurrentCulture,
                    "Logical network '{0}' not found.",
                    logicalNetworkName));
                throw new InvalidOperationException(string.Format(CultureInfo.CurrentCulture,
                     "Logical network definition '{0}' not found.",
                     logicalNetworkName));
            }
            Guid logicalNetworkId = logicalNetwork.Id;
            Guid logicalNetworkDefinitionId = logicalNetwork.LogicalNetworkDefinitions.First().Id;
            ////for creation.
            this.CompareVTNObjects(vtns, vMNetworkConfig, logicalNetworkId, logicalNetworkDefinitionId, txnMng, VtnHostName);
            ////for removal.
            this.CompareVMNetworkObjects(vtns, vMNetworkConfig);
            vMNetworkConfig.VMNetwork.VMNetworkMappingInformation.LastModifiedTimeStamp =
                        DateTime.Now;
        }

        /// <summary>
        /// Compare the VTN.
        /// </summary>
        /// <param name="vtn">VTN to compare.</param>
        /// <param name="vmnetworkInfo">Corresponding VM network info.</param>
        /// <returns>Indicates whether VTN is synchronized or not.</returns>
        public bool CompareVTN(Vtn vtn, VMNetworkInfo vmnetworkInfo) {
            bool needRefresh = false;
            List<Vbridge> vbridges = new List<Vbridge>();
            foreach (Vbridge vbr in vtn.Vbridges) {
                var vmsubnetInfo = vmnetworkInfo.VMSubnetInfo.FirstOrDefault(vmsubnet => vmsubnet.VBridgeName.Equals(vbr.Name));
                if (vmsubnetInfo != null) {
                    if (vmsubnetInfo.VBridgeVlanId != vbr.VlanId) {
                        needRefresh = true;
                    }
                } else {
                    needRefresh = true;
                }
            }
            foreach (var subnetInfo in vmnetworkInfo.VMSubnetInfo) {
                Vbridge vbr = vtn.Vbridges.FirstOrDefault(odlItem => odlItem.Name.CompareTo(subnetInfo.VBridgeName) == 0);
                if (vbr == null) {
                    needRefresh = true;
                }
            }
            return needRefresh;
        }

        /// <summary>
        /// Compare the VTN.
        /// </summary>
        /// <param name="logicalNetworkDefinitionId">Logfical network definition ID.</param>
        /// <param name="vmnetwork">Corresponding VM network.</param>
        /// <param name="vtn">VTN to compare.</param>
        /// <param name="vmnetworkInfo">Corresponding VM network info.</param>
        /// <param name="txnMng">Transaction manager instance.</param>
        /// <param name="VtnHostName">Host name of the VTNCoordinator.</param>
        /// <returns>Indicateds the need to refresh the network service.</returns>
        public bool SyncVTN(Guid? logicalNetworkDefinitionId, VMNetwork vmnetwork, Vtn vtn, VMNetworkInfo vmnetworkInfo, TransactionManager txnMng, string VtnHostName) {
            bool needRefresh = false;
            foreach (Vbridge vbr in vtn.Vbridges) {
                var vmsubnetInfo = vmnetworkInfo.VMSubnetInfo.FirstOrDefault(vmsubnet => vmsubnet.VBridgeName.Equals(vbr.Name));
                if (vmsubnetInfo != null) {
                    if (vmsubnetInfo.VBridgeVlanId != vbr.VlanId && !string.IsNullOrEmpty(vmsubnetInfo.VMSubnetName)) {
                        string vlanRange = CheckVlanRange(vmnetworkInfo, txnMng, VtnHostName, vmsubnetInfo);
                        if (vlanRange.CompareTo(string.Empty) == 0) {
                            vmsubnetInfo.Description = string.Format("VLAN ID of this VM Subnet is changed from {0} to {1} on ODL. VM attached to this VM Subnet prior to this change (i.e, with VLAN ID {0}), will not be operable now.", vmsubnetInfo.VMSubnetVlanId, vbr.VlanId);
                            vmsubnetInfo.VMSubnetVlanId = vbr.VlanId;
                            var vmsubnet = vmnetwork.VMSubnets.FirstOrDefault(subnet => subnet.Id.Equals(vmsubnetInfo.VMSubnetID));
                            vmsubnet.SegmentId.PrimarySegmentIdentifier = Convert.ToUInt32(vbr.VlanId);
                            needRefresh = true;
                        } else {
                            vmsubnetInfo.Description = string.Format("VLAN ID of this VM Subnet is changed from {0} to {1} on ODL. But {1} does not lie in the specified range '{2}'. VM attached to this VM Subnet will not be operable now.", vmsubnetInfo.VMSubnetVlanId, vbr.VlanId, vlanRange);
                        }
                    }
                    vmsubnetInfo.VBridgeVlanId = vbr.VlanId;
                } else {
                    this.AddVMSubnet(vmnetworkInfo, vbr, vmnetwork, logicalNetworkDefinitionId);
                    needRefresh = true;
                }
            }
            foreach (var subnetInfo in vmnetworkInfo.VMSubnetInfo) {
                Vbridge vbr = vtn.Vbridges.FirstOrDefault(odlItem => odlItem.Name.CompareTo(subnetInfo.VBridgeName) == 0);
                if (vbr == null) {
                    RemoveVMSubnetSync(null, subnetInfo.VMSubnetID, vmnetwork, vmnetworkInfo);
                    needRefresh = true;
                }
            }
            return needRefresh;
        }

        /// <summary>
        /// Checks whether vlanId lie in the vlanIdRange.
        /// </summary>
        /// <param name="vlanIdRange">Range of vlan IDs.</param>
        /// <param name="vlanId">VLAN ID.</param>
        /// <returns>Ture if vlanId lie in the vlanIdRange.</returns>
        private static bool CheckVlanId(string vlanIdRange, long vlanId) {
            bool isInRange = false;
            if (string.IsNullOrEmpty(vlanIdRange)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'vlanIdRange' is null or invalid.");
                throw new ArgumentException("The parameter 'vlanIdRange' is null or invalid.");
            }

            if (vlanId == 0) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'vlanId' is null or invalid.");
                throw new ArgumentException("The parameter 'vlanId' is null or invalid.");
            }

            var vlanIds = vlanIdRange.Split(',');
            foreach (var id in vlanIds) {
                if (id.Contains("-")) {
                    if (vlanId >= Convert.ToInt64(id.Split('-').First(), CultureInfo.CurrentCulture) &&
                        vlanId <= Convert.ToInt64(id.Split('-').ElementAt(1), CultureInfo.CurrentCulture)) {
                        isInRange = true;
                        break;
                    }
                } else {
                    if (vlanId == Convert.ToInt64(id, CultureInfo.CurrentCulture)) {
                        isInRange = true;
                        break;
                    }
                }
            }
            return isInRange;
        }

        /// <summary>
        /// Checks whether the change is VLAN ID is consistent or not.
        /// </summary>
        /// <param name="vmnetworkInfo">Corresponding VM network info.</param>
        /// <param name="txnMng">Transaction manager instance.</param>
        /// <param name="VtnHostName">Host name of the VTNCoordinator.</param>
        /// <param name="vmsubnetInfo">Corresponding VM subnet info.</param>
        /// <returns>Empty id the modification is consistent.</returns>
        private static string CheckVlanRange(VMNetworkInfo vmnetworkInfo, TransactionManager txnMng, string VtnHostName, VMSubnetInfo vmsubnetInfo) {
            string vlanIdRange = string.Empty;
            if (vmsubnetInfo.CreatedFrom.CompareTo("ODL") == 0) {
                return vlanIdRange;
            }
            vmsubnetInfo.CreatedFrom = "ODLModified";
            VLANIDMap vLANIDMap = new VLANIDMap(txnMng,
                   VtnHostName,
                   TransactionManager.OpenMode.ReadMode);
            vlanIdRange = vLANIDMap.GetVlanId(vmnetworkInfo.VMNetworkOriginalName, vmsubnetInfo.VMSubnetName);
            if (string.IsNullOrEmpty(vlanIdRange)) {
                return vlanIdRange;
            }
            bool isinRange = CheckVlanId(vlanIdRange, vmsubnetInfo.VBridgeVlanId);
            if (isinRange) {
                return string.Empty;
            } else {
                return vlanIdRange;
            }
        }

        /// <summary>
        /// This function is responsible to synchronize the network configuration between ODL and SCVMM.
        /// </summary>
        /// <param name="vtns">List of VTNs on ODL.</param>
        /// <param name="vMNetworkConfig">VMNetwork.config file instance.</param>
        /// <param name="logicalNetworkId">Logical network ID.</param>
        /// <param name="logicalNetworkDefinitionId">Logical network definition ID.</param>
        /// <param name="txnMng">Transaction manager instance.</param>
        /// <param name="VtnHostName">Host name of the VTNCoordinator.</param>
        private void CompareVTNObjects(List<Vtn> vtns, VMNetworkConfig vMNetworkConfig, Guid logicalNetworkId, Guid? logicalNetworkDefinitionId, TransactionManager txnMng, string VtnHostName) {
            var vmnetworks = vMNetworkConfig.VMNetwork;
            foreach (Vtn vtn in vtns) {
                var vmnetworkInfo = vmnetworks.VMNetworkMappingInformation.VMNetworkInfo.FirstOrDefault(vmnet => vmnet.VTNName.Equals(vtn.Name));
                if (vmnetworkInfo != null) {
                    VMNetwork vmnetwork = vmnetworks.VmNetworks.FirstOrDefault(vmn => vmn.Id == vmnetworkInfo.VMNetworkID);
                    this.SyncVTN(logicalNetworkDefinitionId, vmnetwork, vtn, vmnetworkInfo, txnMng, VtnHostName);
                } else {
                    VMNetwork vMNetwork = VSEMVMNetworkManagement.CreateVtnVmNetwork(vtn.Name);
                    vMNetwork.LogicalNetwork = logicalNetworkId;

                    vMNetworkConfig.VMNetwork.VmNetworks.Add(vMNetwork);
                    if (vMNetworkConfig.VMNetwork.VMNetworkMappingInformation.VMNetworkInfo == null) {
                        vMNetworkConfig.VMNetwork.VMNetworkMappingInformation.VMNetworkInfo =
                            new List<VMNetworkInfo>();
                    }
                    var vmnetInfo = new VMNetworkInfo {
                        VMNetworkID = vMNetwork.Id,
                        VMNetworkName = string.Empty,
                        VTNName = vtn.Name,
                        VMSubnetInfo = new List<VMSubnetInfo>(),
                        CreatedFrom = "ODL",
                        Description = "VM Network corresponding to this VTN is not created on SCVMM",
                        VMNetworkOriginalName = vMNetwork.Name
                    };
                    vMNetworkConfig.VMNetwork.VMNetworkMappingInformation.VMNetworkInfo.Add(
                        vmnetInfo);
                    foreach (Vbridge vbr in vtn.Vbridges) {
                        this.AddVMSubnet(vmnetInfo, vbr, vMNetwork, logicalNetworkDefinitionId);
                    }
                    ////--------VTN is added on ODL
                }
            }
        }

        /// <summary>
        /// This function is responsible to synchronize the network configuration between ODL and SCVMM.
        /// </summary>
        /// <param name="vtns">List of VTNs on ODL.</param>
        /// <param name="vMNetworkConfig">VMNetwork.config file instance.</param>
        private void CompareVMNetworkObjects(List<Vtn> vtns, VMNetworkConfig vMNetworkConfig) {
            foreach (var networkInfo in vMNetworkConfig.VMNetwork.VMNetworkMappingInformation.VMNetworkInfo) {
                Vtn vtn = vtns.FirstOrDefault(odlItem => odlItem.Name.CompareTo(networkInfo.VTNName) == 0);
                if (vtn == null) {
                    RemoveVMNetworkSync(vMNetworkConfig, networkInfo);
                } else {
                    foreach (var subnetInfo in networkInfo.VMSubnetInfo) {
                        Vbridge vbr = vtn.Vbridges.FirstOrDefault(odlItem => odlItem.Name.CompareTo(subnetInfo.VBridgeName) == 0);
                        if (vbr == null) {
                            RemoveVMSubnetSync(vMNetworkConfig, subnetInfo.VMSubnetID, null, null);
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Add the VMSubnet that corresponds the newly created vBridge on ODL.
        /// </summary>
        /// <param name="vmnetworkInfo">VM Network info.</param>
        /// <param name="vbr">Newly created vBridge.</param>
        /// <param name="vmnetwork">VM network that correspond the VTN in which vBridge is created.</param>
        /// <param name="logicalNetworkDefinitionId">Logical network definition Id.</param>
        private void AddVMSubnet(VMNetworkInfo vmnetworkInfo, Vbridge vbr, VMNetwork vmnetwork, Guid? logicalNetworkDefinitionId) {
            List<string> ipsubnets = new List<string>();

            vmnetwork.VMSubnets.ToList().Where(item => item.Description.Contains("/")).ToList().ForEach(sub
                    => sub.IPSubnets.ToList().ForEach(ipsub
                        => ipsubnets.Add(ipsub.Subnet)));
            string ipsubnet = this.CreateUniqueIPSubnet(ipsubnets);
            VMSubnet vmsubnet = VSEMVMSubnetManagement.CreateVbrVmNetworkDefinition(vmnetworkInfo.VMNetworkID,
                null,
                new IPSubnet[] { new IPSubnet{
                                    AddressFamily = AddressFamily.IPv4, 
                                    Subnet = ipsubnet, 
                                    Id = Guid.NewGuid(), 
                                    LastModifiedTimeStamp = DateTime.Now, 
                                    SupportsDHCP = true
                                }},
                vbr.Name,
                logicalNetworkDefinitionId,
                vbr.Name,
                vbr.VlanId);
            vmsubnet.Description = vmsubnet.Description + " (" + ipsubnet + ")";
            Array.Resize(ref vmnetwork.VMSubnets, vmnetwork.VMSubnets.Length + 1);
            vmnetwork.VMSubnets[vmnetwork.VMSubnets.Length - 1] = vmsubnet;
            vmnetworkInfo.VMSubnetInfo.Add(new VMSubnetInfo {
                VBridgeName = vbr.Name,
                VMSubnetVlanId = string.IsNullOrEmpty(vmnetworkInfo.VMNetworkName) ? 0 : vbr.VlanId,
                VMSubnetID = vmsubnet.Id,
                VMSubnetName = string.IsNullOrEmpty(vmnetworkInfo.VMNetworkName) ? string.Empty : vbr.Name,
                CreatedFrom = "ODL",
                VBridgeVlanId = vbr.VlanId
            });
        }

        /// <summary>
        /// Create unique IP subnet.
        /// </summary>
        /// <param name="ipsubnets">List of existing IP subnets.</param>
        /// <returns>Unique IP subnet.</returns>
        private string CreateUniqueIPSubnet(List<string> ipsubnets) {
            string ipsubnet = "1.0.0.1/24";
            for (int i = 0, j = 1; ipsubnets.Contains(ipsubnet) && (i < 20); j++) {
                ipsubnet = "1." + i + "." + (j % 256) + ".1/24";
                i = j / 256;
            }
            return ipsubnet;
        }

        /// <summary>
        /// Remove completely the entities removed on SCVMM after refresh network service.
        /// </summary>
        /// <param name="scvmsubnets">Collection of VMSubnets on SCVMM.</param>
        /// <param name="txnMng">Transaction manager instance.</param>
        private void HandleVMSubnetsDependencies(List<string> scvmsubnets, TransactionManager txnMng) {
            string VtnHostName = this.ConnectionString;

            if (VtnHostName.StartsWith(@"https://", StringComparison.Ordinal)) {
                VtnHostName = VtnHostName.Substring(8);
            }

            VtnHostName = VtnHostName.Split('.', ':').First();
            var vMNetworkConfig = new VMNetworkConfig(VtnHostName);
            try {
                txnMng.SetConfigManager(vMNetworkConfig, TransactionManager.OpenMode.WriteMode);
            } catch (Exception ex) {
                ODLVSEMETW.EventWriteConfigManagerFileIOError(
                    MethodBase.GetCurrentMethod().Name,
                    string.Format(CultureInfo.CurrentCulture,
                    "VMNetwork.config {0}\n{1}",
                    configFileIOErrorValidationMessage,
                    ex.Message));
                ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name, string.Empty);
                throw new InvalidOperationException(
                    string.Format(CultureInfo.CurrentCulture,
                    "VMNetwork.config {0}",
                    configFileIOErrorValidationMessage));
            }
            List<string> vsemvmsubnets = new List<string>();
            var vmnetworkInfo = vMNetworkConfig.VMNetwork.VMNetworkMappingInformation.VMNetworkInfo;
            vmnetworkInfo.ForEach(vmn => vmn.VMSubnetInfo.ForEach(vms => {
                if (vms.VBridgeName.CompareTo(string.Empty) == 0) {
                    vsemvmsubnets.Add(vms.VMSubnetID.ToString());
                }
            }));

            List<string> vmsubnetsDeleted = vsemvmsubnets.Except(scvmsubnets).ToList();
            vmnetworkInfo.ForEach(vmn => vmn.VMSubnetInfo.RemoveAll(vms => vmsubnetsDeleted.Contains(vms.VMSubnetID.ToString())));
            vmnetworkInfo.RemoveAll(vmn => vmn.VMSubnetInfo.Count == 0);
            vmnetworkInfo.ForEach(vmn => {
                vmn.VMSubnetInfo.ForEach(vms => {
                    if (vms.VBridgeName.CompareTo(string.Empty) == 0) {
                        vms.Description = "VM Subnet could not be deleted on SCVMM due to dependent resources.\nEither one or more VM Subnets associated with the corresponding VM Network have dependent resources such as VM templates, virtual network adapters etc. Or corresponding VM Network has dependent resources such as VM templates, virtual network adapters etc.";
                    }
                });
                if (vmn.VTNName.CompareTo(string.Empty) == 0) {
                    vmn.Description = "VM Network could not be deleted on SCVMM due to dependent resources.\nIt has dependent resources such as VM templates, virtual network adapters etc.";
                }
            });
        }
    }
}