//     Copyright (c) 2015 NEC Corporation
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
    /// The classes manages VmNetwork.
    /// </summary>
    public class VSEMVMNetworkManagement {
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
        /// Create vm network.
        /// </summary>
        /// <param name="name">Vm network name.</param>
        /// <returns>Vm network.</returns>
        public static VMNetwork CreateVtnVmNetwork(string name) {
            ODLVSEMETW.EventWriteCreateVtnVMNetwork(MethodBase.GetCurrentMethod().Name,
                    "Creating VMNetwork.");
            if (string.IsNullOrEmpty(name)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'name' is null or invalid.");
                throw new ArgumentException("The parameter 'name' is null or invalid.");
            }

            VMNetwork vmNet = new VMNetwork();
            vmNet.Id = Guid.NewGuid();
            vmNet.Name = name;
            vmNet.VMSubnets = new VMSubnet[0];
            vmNet.LastModifiedTimeStamp = DateTime.Now;
            vmNet.ManagedByNetworkServiceId = VSEMODLConstants.SYSTEM_INFO_ID;
            return vmNet;
        }

        /// <summary>
        /// Initializes the fileds.
        /// </summary>
        /// <param name="connectionString">Connection string of ODL.</param>
        /// <param name="credential">Creadentials of ODL.</param>
        public VSEMVMNetworkManagement(string connectionString,
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
        /// Create a new VM network with specified parameters using VTN on demand.
        /// </summary>
        /// <param name="txnMng">Transaction manager.</param>
        /// <param name="vMSubnetName">Name of VM Subnet to create.</param>
        /// <param name="vMNetworkName">Name of VM network to create.</param>
        /// <param name="maxNumberOfPorts">Maximum number of ports.</param>
        /// <param name="ipSubnets">IP pools to use.</param>
        /// <param name="logicalNetworkDefinitionId">ID of logical network definition
        /// to associate with.</param>
        /// <param name="connection">VSEM connection.</param>
        /// <param name="vtnName">VTN name.</param>
        /// <returns>VM network if successful, else null.</returns>
        public VMNetwork CreateVMNetwork(TransactionManager txnMng,
            string vMSubnetName,
            string vMNetworkName,
            long? maxNumberOfPorts,
            IPSubnet[] ipSubnets,
            Guid logicalNetworkDefinitionId,
            VSEMConnection connection,
            out string vtnName) {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"txnMng\":" + JavaScriptSerializer.Serialize(txnMng));
            json.Append(" \"ipSubnets\":" + JavaScriptSerializer.Serialize(ipSubnets));
            json.Append("\"maxNumberOfPorts\":\"" + maxNumberOfPorts + "\"");
            json.Append("\"vMNetworkName\":\"" + vMNetworkName + "\"");
            json.Append("\"vMSubnetName\":\"" + vMSubnetName + "\"");
            json.Append("\"logicalNetworkDefinitionId\":\"" + logicalNetworkDefinitionId + "\"");
            ODLVSEMETW.EventWriteStartLibrary(MethodBase.GetCurrentMethod().Name,
                json.ToString());
            if (txnMng == null) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'transaction' is null or invalid.");
                throw new ArgumentException("The parameter 'transaction' is null or invalid.");
            }

            if (string.IsNullOrEmpty(vMNetworkName)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'vMNetworkName' is null or invalid.");
                throw new ArgumentException("The parameter 'vMNetworkName' is null or invalid.");
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

            var logicalNetName = VSEMODLConstants.LOGICAL_NETWORK_NAME;
            logicalNetName += VtnHostName;
            var fabricNwDef = LogicalNetworkConfig.LogicalNetworks.First(
                log => log.Name.Equals(logicalNetName)).LogicalNetworkDefinitions.FirstOrDefault(
                fabnw => fabnw.Id == logicalNetworkDefinitionId);
            if (fabricNwDef == null) {
                ODLVSEMETW.EventWriteGetFabricNetworkDefinitionError(
                    MethodBase.GetCurrentMethod().Name,
                    string.Format(CultureInfo.CurrentCulture,
                    "Logical network definition '{0}' not found.",
                    logicalNetworkDefinitionId.ToString("B")));
                throw new ArgumentException(string.Format(CultureInfo.CurrentCulture,
                     "Logical network definition '{0}' not found.",
                logicalNetworkDefinitionId.ToString("B")));
            }

            ODLVSEMETW.EventWriteReturnLibrary(string.Format(CultureInfo.CurrentCulture,
                "VSEMLogicalNetworkDefinition is retrieved by ID: {0}",
                logicalNetworkDefinitionId.ToString("B")),
                string.Empty);

            if (!fabricNwDef.SupportsVMNetworkProvisioning) {
                ODLVSEMETW.EventWriteSupportsVMNetworkProvisioningError(
                    "Logical network does not support VM network creation.",
                    string.Empty);
                throw new InvalidOperationException(
                    "Logical network does not support VM network creation.");
            }

            vtnName = this.CreateUniqueNameForVTN(vMNetworkName);

            Vtn vtn = new Vtn(this.ConnectionString, this.Credential);
            vtn.AddVtn(vtnName);

            ODLVSEMETW.EventWriteReturnODLLibrary(string.Format(CultureInfo.CurrentCulture,
                "VTN '{0}' is created",
                vtnName),
                string.Empty);

            VMNetwork vmNet = CreateVtnVmNetwork(vMNetworkName);
            vmNet.LogicalNetwork = fabricNwDef.LogicalNetworkId;
            if (vmNet == null) {
                ODLVSEMETW.EventWriteProcessFailedVMNetworkError(
                    MethodBase.GetCurrentMethod().Name,
                    "Failed to create VM Network.");
                throw new InvalidOperationException("Failed to create VM Network.");
            }
            ODLVSEMETW.EventWriteSuccessVmNetwork(MethodBase.GetCurrentMethod().Name,
                    "VM Network Successfully Created.");

            // Create the VM Subnet
            vMNetworkConfig.VMNetwork.VmNetworks.Add(vmNet);
            if (vMNetworkConfig.VMNetwork.VMNetworkMappingInformation.VMNetworkInfo == null) {
                vMNetworkConfig.VMNetwork.VMNetworkMappingInformation.VMNetworkInfo =
                    new List<VMNetworkInfo>();
            }

            vMNetworkConfig.VMNetwork.VMNetworkMappingInformation.VMNetworkInfo.Add(
                new VMNetworkInfo {
                    VMNetworkID = vmNet.Id,
                    VMNetworkName = vmNet.Name,
                    VTNName = vtnName,
                    VMSubnetInfo = new List<VMSubnetInfo>(),
                    CreatedFrom = "SCVMM",
                    VMNetworkOriginalName = vmNet.Name
                });

            var vsemvmnetworkDefinition = new VSEMVMSubnetManagement(
                this.ConnectionString, this.Credential);
            ODLVSEMETW.EventWriteCreateVMsubNetwork(MethodBase.GetCurrentMethod().Name,
                    "VM Sub Network creation process started.");
            string vbrName = string.Empty;
            VMSubnet vmNetworkDef =
                vsemvmnetworkDefinition.CreateVMNetworkDefinitionforVtn(
                txnMng,
                LogicalNetworkConfig,
                vMNetworkConfig,
                vMSubnetName,
                vmNet.Id,
                maxNumberOfPorts,
                ipSubnets,
                logicalNetworkDefinitionId,
                connection,
                out vtnName,
                out vbrName);

            Controller odl = new Controller(this.ConnectionString, this.Credential);
            odl.UpdateStartupConfiguration();
            string output = "\"vmNet\":" + JavaScriptSerializer.Serialize(vmNet);
            ODLVSEMETW.EventWriteReturnODLLibrary("Return from ODL Library.", output);

            ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name,
                output);
            return vmNet;
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
        /// Remove the specified VM network.
        /// </summary>
        /// <param name="vMNetworkConfig">Vm network config.</param>
        /// <param name="vmNetId">ID of VM network to remove.</param>
        public void RemoveVmNetwork(VMNetworkConfig vMNetworkConfig, Guid vmNetId) {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder(" \"vMNetworkConfig\":" + JavaScriptSerializer.Serialize(vMNetworkConfig));
            json.Append("\"vmNetId\":\"" + vmNetId + "\"");
            ODLVSEMETW.EventWriteStartLibrary(MethodBase.GetCurrentMethod().Name,
                json.ToString());
            ODLVSEMETW.EventWriteRemoveVMNetwork(MethodBase.GetCurrentMethod().Name,
                    "Removing VMNetwork.");
            if (vMNetworkConfig == null) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'vMNetworkConfig' is null or invalid.");
                throw new ArgumentException(
                    "The parameter 'vMNetworkConfig' is null or invalid.");
            }

            if (vmNetId == Guid.Empty) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'vmNetId' is null or invalid.");
                throw new ArgumentException("The parameter 'vmNetId' is null or invalid.");
            }
            string vtnName = string.Empty;
            foreach (var vmNet in
                vMNetworkConfig.VMNetwork.VMNetworkMappingInformation.VMNetworkInfo) {
                if (vmNet.VMNetworkID == vmNetId) {
                    vtnName = vmNet.VTNName;
                    break;
                }
            }

            Vtn vtn = new Vtn(this.ConnectionString, this.Credential);
            try {
                vtn.RemoveVtn(vtnName);
            } catch (System.Net.WebException) {
                //// If VSEM succeeded in vBridge deletion,
                //// there will be no problem in further operations.
                //// Garbage VTN will be abandoned.
            }
            ODLVSEMETW.EventWriteReturnODLLibrary("Return from ODL Library.", string.Empty);
            var vmNwkInfo =
                vMNetworkConfig.VMNetwork.VMNetworkMappingInformation.VMNetworkInfo;

            if (vmNwkInfo.First(vmNet => vmNet.VMNetworkID == vmNetId).VMSubnetInfo.Count == 0) {
                vmNwkInfo.RemoveAll(
                    vmNet => vmNet.VMNetworkID == vmNetId);
            } else {
                vmNwkInfo.First(vmNet => vmNet.VMNetworkID == vmNetId).Description =
                    "Corresponding VTN is deleted on ODL";
            }

            vMNetworkConfig.VMNetwork.VmNetworks.RemoveAll(
                vmNet => vmNet.Id == vmNetId);
            ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name, string.Empty);
        }

        /// <summary>
        /// Remove the specified vtn.
        /// </summary>
        /// <param name="vtnName">VTN name.</param>
        public void RemoveVmNetwork(string vtnName) {
            StringBuilder json = new StringBuilder("\"vtnName\":\"" + vtnName + "\"");
            ODLVSEMETW.EventWriteStartLibrary(MethodBase.GetCurrentMethod().Name,
                json.ToString());
            ODLVSEMETW.EventWriteRemoveVMNetwork(MethodBase.GetCurrentMethod().Name,
                    "Removing VTN.");
            if (string.IsNullOrWhiteSpace(vtnName)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'vtnName' is null or invalid.");
                throw new ArgumentException("The parameter 'vtnName' is null or invalid.");
            }

            Vtn vtn = new Vtn(this.ConnectionString, this.Credential);
            vtn.RemoveVtn(vtnName);
            ODLVSEMETW.EventWriteReturnODLLibrary("Return from ODL Library.",
                string.Empty);
        }

        /// <summary>
        /// Create vtn name.
        /// </summary>
        /// <param name="vmNetworkName">Vm network name.</param>
        /// <returns>Vtn name.</returns>
        private static string CreateName(string vmNetworkName) {
            ODLVSEMETW.EventWriteCreateVTNName(MethodBase.GetCurrentMethod().Name,
                    "Creating name for VTN.");
            if (string.IsNullOrEmpty(vmNetworkName)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'vmNetworkName' is null or invalid.");
                throw new ArgumentException("The parameter 'vmNetworkName' is null or invalid.");
            }

            StringBuilder name = new StringBuilder("ODL_");
            string machinename = System.Net.Dns.GetHostName();

            machinename =
                Regex.Replace(machinename,
                RegularExpressions.VM_NETWORK_NAME_PATTERN.Replace("^", string.Empty).Replace(
                "$", string.Empty).Replace("[", "[^"),
                string.Empty);

            name.Append(machinename);
            name.Append(vmNetworkName);
            name.Append("_001");
            if (!Regex.IsMatch(name.ToString(), RegularExpressions.VM_NETWORK_NAME_PATTERN)) {
                ODLVSEMETW.EventWriteValidateNameError(MethodBase.GetCurrentMethod().Name,
                    "VMNetworkName is invalid."
                + "\nIt can only contain alphanumeric characters and underbar.");
                throw new ArgumentException("VMNetworkName is invalid."
                + "\nIt can only contain alphanumeric characters and underbar.");
            }
            if (name.Length > Constants.MAX_WEBAPI_NAME_LENGTH) {
                ODLVSEMETW.EventWriteValidateNameLengthError(MethodBase.GetCurrentMethod().Name,
                    "VMNetworkName or SCVMM host name is too long."
                  + "\nMaximum combined length of VMNetworkName and SCVMM host name is 24.");
                throw new ArgumentException("Either VMNetworkName or SCVMM host name is too long."
                  + "\nMaximum combined length of VMNetworkName and SCVMM host name is 24.");
            }
            return name.ToString();
        }

        /// <summary>
        /// Create unique vtn name.
        /// </summary>
        /// <param name="vtnName">Vtn name.</param>
        /// <returns>Unique vtn name.</returns>
        private static string RecreateName(string vtnName) {
            ODLVSEMETW.EventWriteRecreateUniqueVTNName(
                MethodBase.GetCurrentMethod().Name,
                "Recreating unique name for VTN.");
            if (string.IsNullOrEmpty(vtnName)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'vtnName' is null or invalid.");
                throw new ArgumentException("The parameter 'vtnName' is null or invalid.");
            }

            string name = vtnName.Remove(vtnName.LastIndexOf('_'));
            string number = vtnName.Split('_').Last();
            int num = Convert.ToInt16(number, CultureInfo.CurrentCulture);
            num++;
            name = name + "_" + num.ToString(CultureInfo.CurrentCulture).PadLeft(3, '0');

            if (!Regex.IsMatch(name.ToString(), RegularExpressions.VM_NETWORK_NAME_PATTERN)) {
                ODLVSEMETW.EventWriteValidateNameError(MethodBase.GetCurrentMethod().Name,
                    "VMNetworkName is invalid."
                + "\nIt can only contain alphanumeric characters and underbar.");
                throw new ArgumentException("VMNetworkName is invalid."
                + "\nIt can only contain alphanumeric characters and underbar.");
            }
            if (name.Length > Constants.MAX_WEBAPI_NAME_LENGTH) {
                ODLVSEMETW.EventWriteValidateNameLengthError(MethodBase.GetCurrentMethod().Name,
                    "VMNetworkName or SCVMM host name is too long."
                  + "\n Maximum combined length of VMNetworkName and SCVMM host name is 24.");
                throw new ArgumentException("Either VMNetworkName or SCVMM host name is too long."
                  + "\n Maximum combined length of VMNetworkName and SCVMM host name is 24.");
            }
            return name.ToString();
        }

        /// <summary>
        /// Create unique name for vtn.
        /// </summary>
        /// <param name="vmNetworkName">VmNetwork name.</param>
        /// <returns>Unique name of vtn.</returns>
        private string CreateUniqueNameForVTN(string vmNetworkName) {
            ODLVSEMETW.EventWriteCreateUniqueVTNName(MethodBase.GetCurrentMethod().Name,
                    "Creating unique name for VTN.");
            if (string.IsNullOrEmpty(vmNetworkName)) {
                ODLVSEMETW.EventWriteArgumentError(
                MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                MethodBase.GetCurrentMethod().Name,
                "The parameter 'vmNetworkName' is null or invalid.");
                throw new ArgumentException("The parameter 'vmNetworkName' is null or invalid.");
            }

            Controller Odl = new Controller(this.ConnectionString, this.Credential);
            List<string> existingVtns = Odl.GetVtnList();
            ODLVSEMETW.EventWriteReturnODLLibrary("Return from ODL Library.", string.Empty);
            string name = CreateName(vmNetworkName);
            while (existingVtns.Contains(name)) {
                name = RecreateName(name);
            }
            return name;
        }
    }
}
