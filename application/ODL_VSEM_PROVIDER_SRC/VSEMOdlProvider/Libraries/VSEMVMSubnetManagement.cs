//     Copyright (c) 2013-2014 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Management.Automation;
using System.Reflection;
using System.Text;
using System.Text.RegularExpressions;
using System.Web.Script.Serialization;
using Microsoft.SystemCenter.NetworkService;
using ODL.VSEMProvider.Libraries.Common;
using ODL.VSEMProvider.Libraries.Entity;
using ODL.VSEMProvider.CTRLibraries;
using ODL.VSEMProvider.CTRLibraries.Common;
using ODL.VSEMProvider.CTRLibraries.Entity;
using ODL.VSEMProvider.VSEMEvents;

namespace ODL.VSEMProvider.Libraries {
    /// <summary>
    /// The classes manages VmNetwork Definition.
    /// </summary>
    public class VSEMVMSubnetManagement {
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
        /// Create a VM Subnet for the specified vBridge.
        /// </summary>
        /// <param name="vmNetworkId">Id of VM network this definition belongs to.</param>
        /// <param name="maxNumberOfPorts">Max number of ports.</param>
        /// <param name="ipSubnet">IP address pool to use.</param>
        /// <param name="name">Name for VM Subnet. If null, use the vBridge name.
        /// </param>
        /// <param name="logicalNetworkDefinitionId">Logical network definition ID to use.</param>
        /// <param name="vbrName">Vbridge name to use.</param>
        /// <param name="vlanId">Vlan ID to use.</param>
        /// <returns>VM Subnet.</returns>
        public static VMSubnet CreateVbrVmNetworkDefinition(Guid vmNetworkId,
            long? maxNumberOfPorts,
            IPSubnet[] ipSubnet,
            string name,
            Guid? logicalNetworkDefinitionId,
            string vbrName,
            long vlanId) {
            if (string.IsNullOrEmpty(name)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'name' is null or invalid.");
                throw new ArgumentException("The parameter 'name' is null or invalid.");
            }
            if (vmNetworkId == Guid.Empty) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'vmNetworkId' is null or invalid.");
                throw new ArgumentException("The parameter 'vmNetworkId' is null or invalid.");
            }
            if (logicalNetworkDefinitionId == Guid.Empty) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'logicalNetworkDefinitionId' is null or invalid.");
                throw new ArgumentException(
                    "The parameter 'logicalNetworkDefinitionId' is null or invalid.");
            }
            if (string.IsNullOrEmpty(vbrName)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'vbrName' is null or invalid.");
                throw new ArgumentException("The parameter 'vbrName' is null or invalid.");
            }
            VMSubnet def = new VMSubnet();
            def.Id = Guid.NewGuid();

            def.Name = name;
            ODLVSEMETW.EventWriteValidateCreateVmNetworkDefinitionForvBr(
                MethodBase.GetCurrentMethod().Name,
                "VM Subnet for vBridge.");
            def.Description = string.Format(CultureInfo.CurrentCulture,
                "VM Subnet for vBridge {0}",
                vbrName);
            def.LogicalNetworkDefinitionId = logicalNetworkDefinitionId;
            def.VMNetworkId = vmNetworkId;
            def.AllowsIntraPortCommunication = true;
            def.IsolationType = NetworkDefinitionIsolationType.Vlan;
            if (ipSubnet != null) {
                for (int cntr = 0; cntr < ipSubnet.Count(); cntr++) {
                    if (ipSubnet[cntr].Id == Guid.Empty && !string.IsNullOrEmpty(ipSubnet[cntr].Subnet)) {
                        ipSubnet[cntr].Id = Guid.NewGuid();
                        ipSubnet[cntr].LastModifiedTimeStamp = DateTime.Now;
                    }
                    if (ipSubnet[cntr].IPAddressPools == null) {
                        ipSubnet[cntr].IPAddressPools = new IPAddressPool[0];
                    }
                }

                def.IPSubnets = ipSubnet;

                string error = Validations.IsIPSubnetListValid(
                    def.IPSubnets.ToList(), Guid.Empty);
                if (!string.IsNullOrEmpty(error)) {
                    ODLVSEMETW.EventWriteValidateVMNetDefinitionError(
                        MethodBase.GetCurrentMethod().Name,
                        error);
                    throw new ArgumentException(error);
                }
            }
            def.SegmentId = new NetworkSegmentIdentifier();
            def.SegmentId.PrimarySegmentIdentifier = Convert.ToUInt32(vlanId);
            def.LastModifiedTimeStamp = DateTime.Now;
            def.MaxNumberOfPorts = maxNumberOfPorts;
            return def;
        }

        /// <summary>
        /// Initializes the fileds.
        /// </summary>
        /// <param name="connectionString">Connection string of ODL.</param>
        /// <param name="credential">Creadentials of ODL.</param>
        public VSEMVMSubnetManagement(string connectionString,
            PSCredential credential) {
            // Verify arguments
            if (string.IsNullOrEmpty(connectionString)) {
                throw new ArgumentException(
                    "The parameter 'connectionString' is null or invalid.");
            }

            if (credential == null) {
                throw new ArgumentException("The parameter 'credential' is null or invalid.");
            }

            this.ConnectionString = connectionString;
            this.Credential = credential;
        }

        /// <summary>
        /// Create a new VM subnet with specified parameters using VTN on demand.
        /// </summary>
        /// <param name="txnMng">Transaction manager.</param>
        /// <param name="name">Name of VM Subnet to create.</param>
        /// <param name="vMNetworkId">Id of VM network to create.</param>
        /// <param name="maxNumberOfPorts">Maximum number of ports.</param>
        /// <param name="ipSubnets">IP pools to use.</param>
        /// <param name="logicalNetworkDefinitionId">ID of logical network definition to associate with.</param>
        /// <param name="connection">VSEM connection.</param>
        /// <param name="vtnName">VTN name.</param>
        /// <param name="vbrName">Vbridge name.</param>
        /// <returns>VM network if successful, else null.</returns>
        public VMSubnet CreateVMNetworkDefinition(TransactionManager txnMng,
            string name,
            Guid vMNetworkId,
            long? maxNumberOfPorts,
            IPSubnet[] ipSubnets,
            Guid logicalNetworkDefinitionId,
            VSEMConnection connection,
            out string vtnName,
            out string vbrName) {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"txnMng\":" + JavaScriptSerializer.Serialize(txnMng));
            json.Append(" \"ipSubnets\":" + JavaScriptSerializer.Serialize(ipSubnets));
            json.Append("\"maxNumberOfPorts\":\"" + maxNumberOfPorts + "\"");
            json.Append("\"name\":\"" + name + "\"");
            json.Append("\"logicalNetworkDefinitionId\":\"" + logicalNetworkDefinitionId + "\"");
            ODLVSEMETW.EventWriteStartLibrary(MethodBase.GetCurrentMethod().Name,
                json.ToString());
            if (txnMng == null) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'txnMng' is null or invalid.");

                throw new ArgumentException("The parameter 'txnMng' is null or invalid.");
            }

            if (string.IsNullOrEmpty(name)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'name' is null or invalid.");

                throw new ArgumentException("The parameter 'name' is null or invalid.");
            }

            if (vMNetworkId == Guid.Empty) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'vMNetworkId' is null or invalid.");

                throw new ArgumentException("The parameter 'vMNetworkId' is null or invalid.");
            }
            if (logicalNetworkDefinitionId == Guid.Empty) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'logicalNetworkDefinitionId' is null or invalid.");

                throw new ArgumentException(
                    "The parameter 'logicalNetworkDefinitionId' is null or invalid.");
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
            if (vMNetworkConfig.VMNetwork.VMNetworkMappingInformation.VMNetworkInfo.Count == 0) {
                ODLVSEMETW.EventWriteConfigManagerFileIOError(MethodBase.GetCurrentMethod().Name,
                    string.Format(CultureInfo.CurrentCulture,
                    "VMNetwork.config {0}",
                    configFileIOErrorValidationMessage));
                throw new InvalidOperationException(
                    string.Format(CultureInfo.CurrentCulture,
                    "VMNetwork.config {0}",
                    configFileIOErrorValidationMessage));
            }

            var vmNw = vMNetworkConfig.VMNetwork.VmNetworks.FirstOrDefault(
                nwk => nwk.Id == vMNetworkId);

            if (vmNw == null) {
                ODLVSEMETW.EventWriteGetFabricNetworkDefinitionError(
                    MethodBase.GetCurrentMethod().Name,
                    string.Format(CultureInfo.CurrentCulture,
                    "VM network '{0}' not found.",
                    vMNetworkId.ToString("B")));
                throw new ArgumentException(string.Format(CultureInfo.CurrentCulture,
                    "VM network '{0}' not found. VTN corresponding to this VM network may have been deleted from ODL.\nRefer Network Object Mapping screen.",
                    vMNetworkId.ToString("B")));
            }

            var LogicalNetworkConfig = new LogicalNetworkConfig(VtnHostName);
            txnMng.SetConfigManager(LogicalNetworkConfig, TransactionManager.OpenMode.ReadMode);
            if (LogicalNetworkConfig.LogicalNetworks.Count == 0) {
                ODLVSEMETW.EventWriteConfigManagerFileIOError(
                    MethodBase.GetCurrentMethod().Name,
                    string.Format(CultureInfo.CurrentCulture,
                    "LogicalNetwork.config {0}",
                    configFileIOErrorValidationMessage));
                throw new InvalidOperationException(
                    string.Format(CultureInfo.CurrentCulture,
                    "LogicalNetwork.config {0}",
                    configFileIOErrorValidationMessage));
            }

            var logicalNwDef = LogicalNetworkConfig.LogicalNetworks.First().LogicalNetworkDefinitions.FirstOrDefault(
                fabnw => fabnw.Id == logicalNetworkDefinitionId);

            if (logicalNwDef == null) {
                ODLVSEMETW.EventWriteGetFabricNetworkDefinitionError(
                    MethodBase.GetCurrentMethod().Name,
                    string.Format(CultureInfo.CurrentCulture,
                    "Logical network definition '{0}' not found.",
                    logicalNetworkDefinitionId.ToString("B")));
                throw new ArgumentException(string.Format(CultureInfo.CurrentCulture,
                     "Logical network definition '{0}' not found.",
                logicalNetworkDefinitionId.ToString("B")));
            }

            string output = "\"logicalNwDef\":" + JavaScriptSerializer.Serialize(logicalNwDef);
            if (!logicalNwDef.SupportsVMNetworkProvisioning) {
                ODLVSEMETW.EventWriteSupportsVMNetworkProvisioningError(
                    "Logical does not support VM network creation.",
                    output);
                throw new InvalidOperationException(
                    "Logical does not support VM network creation.");
            }
            VMNetworkInfo vmnetworkInfo = vMNetworkConfig.VMNetwork.VMNetworkMappingInformation.VMNetworkInfo.First(vmn => vmn.VMNetworkID.CompareTo(vmNw.Id) == 0);
            VSEMSynchronization vSEMSynchronization =
                new VSEMSynchronization(this.ConnectionString, this.Credential);
            Controller odl = new Controller(this.ConnectionString, this.Credential);
            List<Vtn> vtns = odl.ReadVTNObjects(vmnetworkInfo.VTNName);
            if (vtns.Count == 0) {
                ODLVSEMETW.EventWriteValidateVMNetDefinitionError(
                    MethodBase.GetCurrentMethod().Name,
                        "Corresponding VTN is not found on ODL.");
                throw new DataMisalignedException("Corresponding VTN is not found on ODL.\nRefresh the Odl configuration and then retry.");
            }

            VMSubnet nw = this.CreateVMNetworkDefinitionforVtn(txnMng,
                LogicalNetworkConfig,
                vMNetworkConfig,
                name,
                vMNetworkId,
                maxNumberOfPorts,
                ipSubnets,
                logicalNetworkDefinitionId,
                connection,
                out vtnName,
                out vbrName);

            if (nw == null) {
                ODLVSEMETW.EventWriteProcessFailedVMSubNetworkError(
                    MethodBase.GetCurrentMethod().Name,
                    "Failed to create VM Subnet.");
                throw new InvalidOperationException("Failed to create VM Subnet.");
            }

            odl = new Controller(this.ConnectionString, this.Credential);
            odl.UpdateStartupConfiguration();
            ODLVSEMETW.EventWriteReturnODLLibrary("Return from ODL Library.", string.Empty);

            string outputLib = "\"nw\":" + JavaScriptSerializer.Serialize(nw);
            ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name, outputLib);
            return nw;
        }

        /// <summary>
        /// Create a new VM network with specified parameters using VTN on demand.
        /// </summary>
        /// <param name="txnMng">Transaction manager.</param>
        /// <param name="LogicalNetworkConfig">Config file of logical network.</param>
        /// <param name="vMNetworkConfig">Config file of vm network.</param>
        /// <param name="name">Name of VM Subnet to create.</param>
        /// <param name="vMNetworkId">Id of VM network to create.</param>
        /// <param name="maxNumberOfPorts">Maximum number of ports.</param>
        /// <param name="ipSubnet">IP pools to use.</param>
        /// <param name="logicalNetworkDefinitionId">ID of logical network definition 
        /// to associate with.</param>
        /// <param name="conn">VSEM connection.</param>
        /// <param name="vtnName">VTN name.</param>
        /// <param name="vbrName">Vbridge name.</param>
        /// <returns>VM network if successful, else null.</returns>
        public VMSubnet CreateVMNetworkDefinitionforVtn(
            TransactionManager txnMng,
            LogicalNetworkConfig LogicalNetworkConfig,
            VMNetworkConfig vMNetworkConfig,
            string name,
            Guid vMNetworkId,
            long? maxNumberOfPorts,
            IPSubnet[] ipSubnet,
            Guid logicalNetworkDefinitionId,
            VSEMConnection conn,
            out string vtnName,
            out string vbrName) {
            ODLVSEMETW.EventWriteCreateVMsubNetwork(MethodBase.GetCurrentMethod().Name,
                "Creating VM SubNetwork.");
            if (txnMng == null) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'txnMng' is null or invalid.");
                throw new ArgumentException("The parameter 'txnMng' is null or invalid.");
            }
            if (LogicalNetworkConfig == null) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'LogicalNetworkConfig' is null or invalid.");
                throw new ArgumentException(
                    "The parameter 'LogicalNetworkConfig' is null or invalid.");
            }
            if (vMNetworkConfig == null) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'vMNetworkConfig' is null or invalid.");
                throw new ArgumentException(
                    "The parameter 'vMNetworkConfig' is null or invalid.");
            }
            if (string.IsNullOrEmpty(name)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'name' is null or invalid.");
                throw new ArgumentException("The parameter 'name' is null or invalid.");
            }
            if (vMNetworkId == Guid.Empty) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'vMNetworkId' is null or invalid.");
                throw new ArgumentException("The parameter 'vMNetworkId' is null or invalid.");
            }
            if (logicalNetworkDefinitionId == Guid.Empty) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'logicalNetworkDefinitionId' is null or invalid.");
                throw new ArgumentException(
                    "The parameter 'logicalNetworkDefinitionId' is null or invalid.");
            }
            if (conn == null) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'conn' is null or invalid.");
                throw new ArgumentException("The parameter 'conn' is null or invalid.");
            }

            string VtnHostName = this.ConnectionString;

            if (VtnHostName.StartsWith(@"https://", StringComparison.Ordinal)) {
                VtnHostName = VtnHostName.Substring(8);
            }

            VtnHostName = VtnHostName.Split('.', ':').First();

            if (vMNetworkConfig.VMNetwork.VMNetworkMappingInformation.VMNetworkInfo == null) {
                ODLVSEMETW.EventWriteProcessLibraryError(MethodBase.GetCurrentMethod().Name,
                    string.Format(CultureInfo.CurrentCulture,
                    "VM network '{0}' not found.",
                    vMNetworkId.ToString("B")));
                throw new ArgumentException(string.Format(CultureInfo.CurrentCulture,
                    "VM network '{0}' not found.",
                    vMNetworkId.ToString("B")));
            }
            var vmNetInfo =
                vMNetworkConfig.VMNetwork.VMNetworkMappingInformation.VMNetworkInfo.Where(
                vmNw => vmNw.VMNetworkID == vMNetworkId).FirstOrDefault();
            if (vmNetInfo == null) {
                ODLVSEMETW.EventWriteProcessLibraryError(MethodBase.GetCurrentMethod().Name,
                    string.Format(CultureInfo.CurrentCulture,
                    "VM network {0} not found.",
                    vMNetworkId.ToString("B")));
                throw new ArgumentException(string.Format(CultureInfo.CurrentCulture,
                    "VM network '{0}' not found.",
                    vMNetworkId.ToString("B")));
            }
            vtnName = vmNetInfo.VTNName;

            vbrName = this.CreateUniqueNameForVBridge(name, vmNetInfo.VTNName);

            VLANIDMap vLANIDMap = new VLANIDMap(txnMng,
                VtnHostName,
                TransactionManager.OpenMode.ReadMode);
            string vlanIdRange = vLANIDMap.GetVlanId(vmNetInfo.VMNetworkOriginalName, name);
            if (string.IsNullOrEmpty(vlanIdRange)) {
                ODLVSEMETW.EventWriteFoundNoVlanError(MethodBase.GetCurrentMethod().Name,
                    "No VLAN ID found.");
                throw new ItemNotFoundException("No VLAN ID found.");
            }
            long vlanId = this.SelectVlanId(vlanIdRange,
                vMNetworkConfig.VMNetwork.VMNetworkMappingInformation.VMNetworkInfo);
            if (vlanId == 0) {
                ODLVSEMETW.EventWriteFoundNoUnusedVlanError(MethodBase.GetCurrentMethod().Name,
                    "No unused VLAN ID found.");
                throw new ItemNotFoundException("No unused VLAN ID found.");
            }

            VMSubnet vmNetworkDef = CreateVbrVmNetworkDefinition(
                vMNetworkId,
                maxNumberOfPorts,
                ipSubnet,
                name,
                logicalNetworkDefinitionId,
                vbrName,
                vlanId);

            if (vmNetworkDef == null) {
                ODLVSEMETW.EventWriteProcessFailedVMNetworkError(
                    MethodBase.GetCurrentMethod().Name,
                    "Failed to create VM Subnet.");
                throw new InvalidOperationException("Failed to create VM Subnet.");
            }
            Vbridge vBridge = new Vbridge(this.ConnectionString, this.Credential);
            ODLVSEMETW.EventWriteReturnODLLibrary("Return from ODL Library.", string.Empty);
            Controller odl_ctr = new Controller(this.ConnectionString, this.Credential);
            if (odl_ctr.Get_status(Constants.CTR_NAME)) {
            vBridge.AddVbridge(vbrName, vmNetInfo.VTNName, vlanId);
            ODLVSEMETW.EventWriteReturnODLLibrary("Return from ODL Library.", string.Empty);
            } else {
                ODLVSEMETW.EventWriteProcessFailedVMNetworkError(
                    MethodBase.GetCurrentMethod().Name,
                    "Failed to create VBR controller status is down.");
                throw new InvalidOperationException("Failed to create VBR.");
            }

            var vmNet = vMNetworkConfig.VMNetwork.VmNetworks.Where(nwk =>
                nwk.Id == vMNetworkId).FirstOrDefault();

            Array.Resize(ref vmNet.VMSubnets, vmNet.VMSubnets.Length + 1);
            vmNet.VMSubnets[vmNet.VMSubnets.Length - 1] = vmNetworkDef;
            vmNetInfo.VMSubnetInfo.Add(new VMSubnetInfo {
                VBridgeName = vbrName,
                VMSubnetVlanId = vlanId,
                VMSubnetID = vmNetworkDef.Id,
                VMSubnetName = vmNetworkDef.Name,
                CreatedFrom = "SCVMM",
                VBridgeVlanId = vlanId
            });

            return vmNetworkDef;
        }

        /// <summary>
        /// Error message for the file IO exception.
        /// </summary>
        private static string configFileIOErrorValidationMessage =
            string.Format(CultureInfo.CurrentCulture,
            "{0}\n{1}\n1. {2}\n2. {3}\n3. {4}",
            "file could not be accessed.",
            "Possible reasons could be:",
            "File is deleted.",
            "File is renamed.",
            "File is tampered.");

        /// <summary>
        /// Remove the specified VM Subnet.
        /// </summary>
        /// <param name="txnMng">Transaction manager.</param>
        /// <param name="id">ID of VM Subnet to remove.</param>
        /// <param name="connection">VSEM connection.</param>
        /// <param name="vbrName">Name of the vBridge.</param>
        /// <returns>Indicated the need to refresh the network service..</returns>
        public bool RemoveVmNetworkDefinition(TransactionManager txnMng,
            Guid id,
            VSEMConnection connection,
            out string vbrName) {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"txnMng\":" + JavaScriptSerializer.Serialize(txnMng));
            json.Append("\"id\":\"" + id + "\"");
            ODLVSEMETW.EventWriteStartLibrary(MethodBase.GetCurrentMethod().Name,
                json.ToString());
            if (txnMng == null) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'txnMng' is null or invalid.");
                throw new ArgumentException("The parameter 'txnMng' is null or invalid.");
            }
            if (id == Guid.Empty) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'id' is null or invalid.");
                throw new ArgumentException("The parameter 'id' is null or invalid.");
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
            if (vMNetworkConfig.VMNetwork.VMNetworkMappingInformation.VMNetworkInfo.Count == 0) {
                ODLVSEMETW.EventWriteConfigManagerFileIOError(
                    MethodBase.GetCurrentMethod().Name,
                    string.Format(CultureInfo.CurrentCulture,
                    "VMNetwork.config {0}",
                    configFileIOErrorValidationMessage));
                throw new InvalidOperationException(
                    string.Format(CultureInfo.CurrentCulture,
                    "VMNetwork.config {0}",
                    configFileIOErrorValidationMessage));
            }

            string vtnName = string.Empty;
            vbrName = string.Empty;
            Guid vmNetId = Guid.Empty;
            VMNetworkInfo vmnetworkMappigInfoFound = null;
            long vlanId = 0;
            foreach (var vmNet in
                vMNetworkConfig.VMNetwork.VMNetworkMappingInformation.VMNetworkInfo) {
                foreach (var vmSubNet in vmNet.VMSubnetInfo) {
                    if (vmSubNet.VMSubnetID == id) {
                        vtnName = vmNet.VTNName;
                        vbrName = vmSubNet.VBridgeName;
                        vmNetId = vmNet.VMNetworkID;
                        vlanId = vmSubNet.VBridgeVlanId;
                        vmnetworkMappigInfoFound = vmNet;
                        break;
                    }
                }
                if (!string.IsNullOrEmpty(vtnName)) {
                    break;
                }
            }
            if (string.IsNullOrEmpty(vbrName)) {
                ODLVSEMETW.EventWriteGetVSEMVMNetworkError(
                    MethodBase.GetCurrentMethod().Name,
                    string.Format(CultureInfo.CurrentCulture,
                    "VMSubnet '{0}' not found.",
                    id.ToString("B")));
                throw new ArgumentException(string.Format(CultureInfo.CurrentCulture,
                    "VMSubnet '{0}' not found.",
                    id.ToString("B")));
            }
            VMNetwork network = null;
            foreach (var vmNet in
                 vMNetworkConfig.VMNetwork.VmNetworks) {
                if (vmNet.Id == vmNetId) {
                    network = vmNet;
                    break;
                }
            }
            if (network == null || !network.VMSubnets.Any(vms => vms.Id.CompareTo(id) == 0)) {
                ODLVSEMETW.EventWriteGetVSEMVMNetworkError(
                    MethodBase.GetCurrentMethod().Name,
                    string.Format(CultureInfo.CurrentCulture,
                    "VMSubnet '{0}' not found.",
                    id.ToString("B")));
                throw new ArgumentException(string.Format(CultureInfo.CurrentCulture,
                    "VMSubnet '{0}' not found.",
                    id.ToString("B")));
            }
            var logicalNetworkDefinitionId = vMNetworkConfig.VMNetwork.VmNetworks.FirstOrDefault(
                vmn => vmn.Id == vmNetId).VMSubnets.FirstOrDefault(
                subNetwork => subNetwork.Id == id).LogicalNetworkDefinitionId;

            VSEMSynchronization vSEMSynchronization =
                new VSEMSynchronization(this.ConnectionString, this.Credential);
            Controller odl = new Controller(this.ConnectionString, this.Credential);
            List<Vtn> vtns = odl.ReadVTNObjects(vtnName);
            if (vtns.Count == 0 && network.VMSubnets.Count() > 1) {
                ODLVSEMETW.EventWriteValidateVMNetDefinitionError(
                    MethodBase.GetCurrentMethod().Name,
                    "Corresponding VTN is not found on ODL.");
                throw new DataMisalignedException(
                    "Corresponding VTN is not found on ODL.\nRefresh the Odl configuration and then retry.");
            }
            string vbridgeName = vbrName;
            if (vtns[0].Vbridges.Any(vbr => vbr.Name.CompareTo(vbridgeName) != 0) && network.VMSubnets.Count() == 1) {
                ODLVSEMETW.EventWriteValidateVMNetDefinitionError(
                    MethodBase.GetCurrentMethod().Name,
                    "On synchronization, additional vBridge(s) are found in the VTN corresponding to this VM Network on ODL.\nSo, this VM Network cannot be removed and the operation has been abandoned.\nRefresh the Odl configuration and then retry.");
                throw new DataMisalignedException(
                    "On synchronization, additional vBridge(s) are found in the VTN corresponding to this VM Network on ODL.\nSo, this VM Network cannot be removed and the operation has been abandoned.\nRefresh the Odl configuration and then retry.");
            }
            Vbridge vbridge = vtns[0].Vbridges.FirstOrDefault(vbr => vbr.Name.CompareTo(vbridgeName) == 0);
            if (vbridge != null && vlanId != vbridge.VlanId) {
                ODLVSEMETW.EventWriteValidateVMNetDefinitionError(
                    MethodBase.GetCurrentMethod().Name,
                        "VLAN ID of corresponding vBridge is modified on ODL.");
                throw new DataMisalignedException(
                    "VLAN ID of corresponding vBridge is modified on ODL.\nRefresh the Odl configuration and then retry.");
            }

            var vmsubnet = vmnetworkMappigInfoFound.VMSubnetInfo.Where(vms =>
                vms.VMSubnetID.CompareTo(id) == 0).First();
            if (vmsubnet.CreatedFrom.Contains("ODL") &&
                vbridge != null) {
                ODLVSEMETW.EventWriteGetVSEMVMNetworkError(
                   MethodBase.GetCurrentMethod().Name,
                   "Corresponding vBridge is created or modified on ODL");
                throw new DataMisalignedException("Corresponding vBridge is created or modified on ODL.\nRefresh the Odl configuration and then retry.");
            }
            if (vtns.Count != 0) {
                Vbridge vBridge = new Vbridge(this.ConnectionString, this.Credential);
                vBridge.RemoveVbridge(vtnName, vbrName);
                ODLVSEMETW.EventWriteReturnODLLibrary("Return from ODL Library.", string.Empty);
            }

            // Checking if VM Network removal is required.
            network.VMSubnets = network.VMSubnets.Where(
                netDef => netDef.Id != id).ToArray();

            vmnetworkMappigInfoFound.VMSubnetInfo.RemoveAll(subNet => subNet.VMSubnetID == id);
            if (network.VMSubnets.Count() == 0) {
                VSEMVMNetworkManagement vmNetworkManagement =
                    new VSEMVMNetworkManagement(this.ConnectionString,
                        this.Credential);
                vmNetworkManagement.RemoveVmNetwork(vMNetworkConfig,
                    vmNetId);
            }

            odl.UpdateStartupConfiguration();
            ODLVSEMETW.EventWriteReturnODLLibrary("Return from ODL Library.", string.Empty);

            ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name, string.Empty);
            return false;
        }

        /// <summary>
        /// Remove the specified VM Subnet.
        /// </summary>
        /// <param name="vtnName">VTN name.</param>
        /// <param name="vbrName">Vbridge name.</param>
        public void RemoveVmNetworkDefinition(string vtnName, string vbrName) {
            StringBuilder json = new StringBuilder("\"vtnName\":\"" + vtnName + "\"");
            json.Append("\"vbrName\":\"" + vbrName + "\"");
            ODLVSEMETW.EventWriteStartLibrary(MethodBase.GetCurrentMethod().Name,
                json.ToString());
            if (string.IsNullOrWhiteSpace(vtnName)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'vtnName' is null or invalid.");
                throw new ArgumentException("The parameter 'vtnName' is null or invalid.");
            }

            if (string.IsNullOrWhiteSpace(vbrName)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'vbrName' is null or invalid.");
                throw new ArgumentException("The parameter 'vbrName' is null or invalid.");
            }

            Vbridge vBridge = new Vbridge(this.ConnectionString, this.Credential);
            vBridge.RemoveVbridge(vtnName, vbrName);
            ODLVSEMETW.EventWriteReturnODLLibrary("Return from ODL Library.", string.Empty);

            ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name, string.Empty);
        }

        /// <summary>
        /// Gets the list of vBridges for the specified VTN from ODL.
        /// </summary>
        /// <param name="vtnName">Name of the VTN.</param>
        /// <returns>List of vBridges.</returns>
        public List<string> GetVbridgesListforVtn(string vtnName) {
            Controller odl = new Controller(this.ConnectionString, this.Credential);
            ODLVSEMETW.EventWriteReturnODLLibrary("Return from ODL Library.", string.Empty);
            List<string> existingVtns = odl.GetVbridgesListforVtn(vtnName);
            return existingVtns;
        }

        /// <summary>
        /// Generate a new GUID and encode the VLAN for
        /// the vBridge in the GUID.
        /// </summary>
        /// <param name="vlanId">Vlan ID.</param>
        /// <returns>Encoded guid.</returns>
        private static Guid EncodeVlanInGuid(long vlanId) {
            var newGuid = Guid.NewGuid();
            byte[] uid = newGuid.ToByteArray();

            // Use the last 24 bits to store the vlan
            int vlan = (int)vlanId;

            uid[14] = Convert.ToByte((uid[14] & 0xf0) | ((vlan & 0x0f00) >> 8));
            uid[15] = Convert.ToByte(vlan & 0xff);

            var encodedguid = new Guid(uid);

            return encodedguid;
        }

        /// <summary>
        /// Create name for vBridge.
        /// </summary>
        /// <param name="vmNetworkDefinitionName">Subnet name.</param>
        /// <returns>Name for vBridge.</returns>
        private string CreateName(string vmNetworkDefinitionName) {
            if (string.IsNullOrEmpty(vmNetworkDefinitionName)) {
                throw new ArgumentException("No Subnet name is specified.");
            }
            StringBuilder name = new StringBuilder("ODL_");
            string machinename = System.Net.Dns.GetHostName();

            machinename =
                 Regex.Replace(machinename,
                 RegularExpressions.VM_NETWORK_NAME_PATTERN.Replace("^", string.Empty).Replace(
                 "$", string.Empty).Replace("[", "[^"),
                 string.Empty);

            name.Append(machinename);
            name.Append(vmNetworkDefinitionName);
            name.Append("_001");

            if (!Regex.IsMatch(name.ToString(), RegularExpressions.VM_NETWORK_NAME_PATTERN)) {
                ODLVSEMETW.EventWriteValidatevBridgeNameError(MethodBase.GetCurrentMethod().Name,
                    "VMNetworkDefinitionName is invalid"
                + "\n It can only contain alphanumeric characters and underbar.");
                throw new ArgumentException("VMNetworkDefinitionName is invalid"
                + "\n It can only contain alphanumeric characters and underbar.");
            }
            if (name.Length > Constants.MAX_WEBAPI_NAME_LENGTH) {
                ODLVSEMETW.EventWriteValidatevBridgeNameLengthError(
                    MethodBase.GetCurrentMethod().Name,
                    "VMNetworkName or MachineName is too long"
                  + "\n Maximum combined length of VMNetworkDefinitionName and SCVMM host name is 24.");
                throw new ArgumentException(
                    "Either VMNetworkDefinitionName or MachineName is too long"
                  + "\n Maximum combined length of VMNetworkDefinitionName and SCVMM host name is 24.");
            }
            return name.ToString();
        }

        /// <summary>
        /// Create unique name for the vBridge.
        /// </summary>
        /// <param name="vbrName">Name of vBridge.</param>
        /// <returns>Vbridge name.</returns>
        private string RecreateName(string vbrName) {
            if (string.IsNullOrEmpty(vbrName)) {
                throw new ArgumentException("vBridge name is not specified.");
            }
            string name = vbrName.Remove(vbrName.LastIndexOf('_'));
            string number = vbrName.Split('_').Last();
            int num = Convert.ToInt16(number, CultureInfo.CurrentCulture);
            num++;
            name = name + "_" + num.ToString(CultureInfo.CurrentCulture).PadLeft(3, '0');

            if (!Regex.IsMatch(name.ToString(), RegularExpressions.VM_NETWORK_NAME_PATTERN)) {
                ODLVSEMETW.EventWriteValidatevBridgeNameError(MethodBase.GetCurrentMethod().Name,
                    "VMSubnetName is invalid."
                + "\n It can only contain alphanumeric characters and underbar.");
                throw new ArgumentException("VMSubnetName is invalid."
                + "\n It can only contain alphanumeric characters and underbar.");
            }
            if (name.Length > Constants.MAX_WEBAPI_NAME_LENGTH) {
                ODLVSEMETW.EventWriteValidatevBridgeNameLengthError(
                    MethodBase.GetCurrentMethod().Name,
                    "VMSubnetName or SCVMM host name is too long."
                  + "\nMaximum combined length of VMSubnetName and SCVMM host name is 24.");
                throw new ArgumentException(
                    "Either VMSubnetName or SCVMM host name is too long."
                  + "\nMaximum combined length of VMSubnetName and SCVMM host name is 24.");
            }
            return name.ToString();
        }

        /// <summary>
        /// Choose the unused vlan id.
        /// </summary>
        /// <param name="vlanIdRange">Range of available vlan IDs.</param>
        /// <param name="vmNertworkList">List of used IDs.</param>
        /// <returns>Vlan Id to deploy.</returns>
        private long SelectVlanId(string vlanIdRange, List<VMNetworkInfo> vmNertworkList) {
            if (string.IsNullOrEmpty(vlanIdRange)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'vlanIdRange' is null or invalid.");
                throw new ArgumentException("The parameter 'vlanIdRange' is null or invalid.");
            }

            if (vmNertworkList == null) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'vmNertworkList' is null or invalid.");
                throw new ArgumentException("The parameter 'vmNertworkList' is null or invalid.");
            }

            long vlanId = 0;
            List<long> usedVlanIds = new List<long>();
            vmNertworkList.ForEach(vmNw => usedVlanIds.AddRange(vmNw.VMSubnetInfo.Select(
                subNw => subNw.VBridgeVlanId)));
            var vlanIds = vlanIdRange.Split(',');
            foreach (var id in vlanIds) {
                if (id.Contains("-")) {
                    for (long rId = Convert.ToInt64(id.Split('-').First(), CultureInfo.CurrentCulture);
                        rId <= Convert.ToInt64(id.Split('-').ElementAt(1), CultureInfo.CurrentCulture);
                        rId++) {
                        if (!usedVlanIds.Contains(rId)) {
                            vlanId = Convert.ToInt64(rId);
                            break;
                        }
                    }
                } else {
                    if (!usedVlanIds.Contains(Convert.ToInt64(id, CultureInfo.CurrentCulture))) {
                        vlanId = Convert.ToInt64(id, CultureInfo.CurrentCulture);
                        break;
                    }
                }
                if (vlanId != 0) {
                    break;
                }
            }

            return vlanId;
        }

        /// <summary>
        /// Create unique name for vBridge.
        /// </summary>
        /// <param name="vmNSubetworkName">Subnet name.</param>
        /// <param name="vtnName">Vtn name.</param>
        /// <returns>Unique name for vBridge.</returns>
        private string CreateUniqueNameForVBridge(string vmNSubetworkName, string vtnName) {
            ODLVSEMETW.EventWriteCreateUniqueVbridgeName(MethodBase.GetCurrentMethod().Name,
                    "Creating unique name for vBridge.");
            if (string.IsNullOrEmpty(vmNSubetworkName)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'vmNSubetworkName' is null or invalid.");
                throw new ArgumentException(
                    "The parameter 'vmNSubetworkName' is null or invalid.");
            }
            if (string.IsNullOrEmpty(vtnName)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'vtnName' is null or invalid.");
                throw new ArgumentException("The parameter 'vtnName' is null or invalid.");
            }
            List<string> existingVtns = this.GetVbridgesListforVtn(vtnName);
            ODLVSEMETW.EventWriteReturnODLLibrary("Return from ODL Library.", string.Empty);
            string name = this.CreateName(vmNSubetworkName);
            while (existingVtns.Contains(name)) {
                name = this.RecreateName(name);
            }
            return name;
        }
    }
}
