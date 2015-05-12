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
using System.Web.Script.Serialization;
using Microsoft.SystemCenter.NetworkService;
using Microsoft.SystemCenter.NetworkService.NetworkManager;
using Microsoft.SystemCenter.NetworkService.VSEM;
using ODL.VSEMProvider.Libraries.Common;
using ODL.VSEMProvider.Libraries.Entity;
using ODL.VSEMProvider.CTRLibraries;
using ODL.VSEMProvider.CTRLibraries.Common;
using ODL.VSEMProvider.VSEMEvents;

namespace ODL.VSEMProvider.Libraries {
    /// <summary>
    /// Connection to VSEM Provider.
    /// </summary>
    public class VSEMConnection : IConnection, ITriggerRefresh {
        /// <summary>
        /// Performs application-defined tasks associated with freeing, releasing, or resetting unmanaged resources.
        /// </summary>
        public void Dispose() {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Setting the handler for the ConnectionLost event.
        /// </summary>
        /// <param name="e">When Virtual Machine Manager Plugin's connection fails to the device,
        /// the Virtual Machine Manager Plugins can generate an event IConnection.ConnectionLost
        /// to indicate to Virtual Machine Manager that the connection needs to be reestablished.</param>
        public void OnConnectionLost(ConnectionRetryInfo e) {
            EventHandler<ConnectionRetryInfo> handler = this.ConnectionLost;
            if (handler != null) {
                handler(this, e);
            }
        }

        /// <summary>
        /// If the plugin maintains an active connection
        /// with the managed device and can determine loss of connection out of band,
        /// then device can raise the exception to Virtual Machine Manager service using ConnectionLost event.
        /// </summary>
        public event EventHandler<ConnectionRetryInfo> ConnectionLost;

        /// <summary>
        /// Connection string to use.
        /// </summary>
        public string ConnectionString { get; private set; }

        /// <summary>
        /// Connection string to use.
        /// </summary>
        private string controllers;

        /// <summary>
        /// Connection string to use.
        /// </summary>
        private string VtnHostName;

        /// <summary>
        /// Credential to use.
        /// </summary>
        public PSCredential Credential {
            get;
            private set;
        }

        /// <summary>
        /// Constructor initializing a new object of VSEMConnection class.
        /// </summary>
        /// <param name="connectionString">Connection string of ODL.</param>
        /// <param name="credential">Credentials of ODL.</param>
        /// <param name="controllers">Ips of controllers along with port numbers.</param>
        public VSEMConnection(string connectionString,
            PSCredential credential,
            string controllers) {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"connectionString\":\"" + connectionString + "\"");
            json.Append(" \"controllers\":\"" + controllers + "\"");
            ODLVSEMETW.EventWriteStartODLLibrary(
                MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                json.ToString());

            if (string.IsNullOrEmpty(connectionString)) {
                throw new ArgumentException(
                    "The parameter 'connectionString' is null or invalid.");
            }

            if (credential == null) {
                throw new ArgumentException("The parameter 'credential' is null or invalid.");
            }

            if (string.IsNullOrEmpty(controllers)) {
                throw new ArgumentException("The parameter 'controllers' is null or invalid.");
            }

            this.ConnectionString = connectionString;
            this.Credential = credential;
            this.controllers = controllers;

            this.VtnHostName = this.ConnectionString;

            if (this.VtnHostName.StartsWith(@"https://", StringComparison.Ordinal)) {
                this.VtnHostName = this.VtnHostName.Substring(8);
            }
            this.VtnHostName = this.VtnHostName.Split('.', ':').First();
            ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name, string.Empty);
        }

        /// <summary>
        /// Retrieve the webapi version of VTNCoordinator and check whether it is suppored of not.
        /// </summary>
        /// <param name="minVesion">Minimum weapi version supported.</param>
        /// <returns>True if Webapi version of VTNCoordinator is suppoted, else false.</returns>
        public bool VerifyConnection(Version minVesion) {
            Controller odl = new Controller(this.ConnectionString, this.Credential);
            ODLVSEMETW.EventWriteVerifyConnection("Verifying connection.", string.Empty);
            string webapiVersionFound =
                odl.RequestWebApiVersion();
            ODLVSEMETW.EventWriteReturnODLLibrary("Return from VTNCoordinator Library.", string.Empty);

            Version vtncoWebapiVersion = new Version(webapiVersionFound.Substring(1));
            return minVesion <= vtncoWebapiVersion;
        }

        /// <summary>
        /// Update the VSEM repository.
        /// </summary>
        /// <param name="txnMng">Transaction manager.</param>
        public void UpdateConnection(TransactionManager txnMng) {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"TransactionManager\":" + JavaScriptSerializer.Serialize(txnMng));
            ODLVSEMETW.EventWriteStartLibrary(MethodBase.GetCurrentMethod().Name, json.ToString());
            if (txnMng == null) {
                throw new ArgumentException(
                    "txnMng' is null or invalid.");
            }

            string VtnHostName = this.ConnectionString;

            if (VtnHostName.StartsWith(@"https://", StringComparison.Ordinal)) {
                VtnHostName = VtnHostName.Substring(8);
            }

            VtnHostName = VtnHostName.Split('.', ':').First();
            var vseminformation = this.CreateVSEMInfo();

            // Make sure we can connect to the ODL
            if (!this.VerifyConnection(vseminformation.MinVTNCoVersion)) {
                ODLVSEMETW.EventWriteProcesOdlLibraryError(MethodBase.GetCurrentMethod().Name,
                    "WebAPI version is not supported.");
                throw new ArgumentException("WebAPI version is not supported.");
            }
            Controller odl_ctr = new Controller(this.ConnectionString, this.controllers, this.Credential);
            odl_ctr.Create_ctr(Constants.CTR_NAME);

            ODLVSEMETW.EventWriteCreateODLData(
                MethodBase.GetCurrentMethod().Name, "Creating VSEM repository.");

            VSEMConfig vsemConfig = new VSEMConfig();
            txnMng.SetConfigManager(vsemConfig, TransactionManager.OpenMode.WriteMode);

            // Update Configuration Data;
            vsemConfig.Info = vseminformation;
            vsemConfig.Controller = new VSEMController();
            vsemConfig.Controller.ControllerInfo = this.controllers;
            if (vsemConfig.NetworkServiceSystemInformation.Id.CompareTo(Guid.Empty) == 0) {
                vsemConfig.NetworkServiceSystemInformation = VSEMConnection.CreateSystemInfo();
            } else {
                VSEMODLConstants.SYSTEM_INFO_ID = vsemConfig.NetworkServiceSystemInformation.Id;
            }
            if (vsemConfig.SwitchExtensionInfo.Count == 0) {
                vsemConfig.SwitchExtensionInfo = VSEMConnection.CreateSwitchExtensionInfos();
            } else {
                VSEMODLConstants.SWITCH_EXTENSION_INFO_ID = vsemConfig.SwitchExtensionInfo[0].Id;
            }
            vsemConfig.SwitchExtensionInfo.ForEach((i) =>
                    VSEMConnection.AddSwitchFeatureToSwitchExtInfos(i, this.controllers));

            LogicalNetworkConfig logicalNetworkConfig = new LogicalNetworkConfig(VtnHostName);
            txnMng.SetConfigManager(logicalNetworkConfig, TransactionManager.OpenMode.WriteMode);
            if (logicalNetworkConfig.LogicalNetworks.Count == 0) {
                logicalNetworkConfig.LogicalNetworks =
                    this.CreateLogicalNetworks();
            }

            PortProfileConfig portProfileConfig = new PortProfileConfig();
            txnMng.SetConfigManager(portProfileConfig, TransactionManager.OpenMode.WriteMode);

            if (portProfileConfig.UplinkPortProfiles.Count == 0) {
                portProfileConfig.UplinkPortProfiles =
                    VSEMConnection.CreateUplinkPortProfiles();
            } else {
                VSEMODLConstants.UPLINK_PORT_PROFILE_ID = portProfileConfig.UplinkPortProfiles[0].Id;
            }

            if (portProfileConfig.VirtualPortProfiles.Count == 0) {
                portProfileConfig.VirtualPortProfiles =
                    VSEMConnection.CreateVirtualPortProfiles();
            }

            // Create HNV default resource.
            HNVLogicalNetworkManagement hnvMgmt = new HNVLogicalNetworkManagement(this.VtnHostName);
            hnvMgmt.CreateHNVLogicalNetwork(logicalNetworkConfig);
            hnvMgmt.AssociatePortProfileWithHNV(portProfileConfig);

            this.SaveVtncoInfo(txnMng, VtnHostName);

            ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name, string.Empty);
        }

        /// <summary>
        /// Closes the connection.
        /// </summary>
        public void CloseConnection() {
            ODLVSEMETW.EventWriteStartLibrary(MethodBase.GetCurrentMethod().Name, string.Empty);
            Controller odl_ctr = new Controller(this.ConnectionString, this.controllers, this.Credential);
            odl_ctr.Delete_ctr(Constants.CTR_NAME);
            odl_ctr.UpdateStartupConfiguration();
            this.ConnectionString = null;
            this.controllers = null;
            this.Credential = null;
            ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name, string.Empty);
        }

        /// <summary>
        /// This method is responsible for extracting the VSEMSystemInfo
        /// from the VSEM repository for the corresponding connection.
        /// </summary>
        /// <param name="txnMng">Transaction manager.</param>
        /// <returns>VSEMSystemInfo instance attached with the connection.</returns>
        public NetworkServiceSystemInformation GetVSEMSystemInfo(TransactionManager txnMng) {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"TransactionManager\":" + JavaScriptSerializer.Serialize(txnMng));
            ODLVSEMETW.EventWriteStartLibrary(MethodBase.GetCurrentMethod().Name, json.ToString());
            ODLVSEMETW.EventWriteExtractsystemInfo("Extracting System Info.", string.Empty);
            var vsemConfig = new VSEMConfig();
            try {
                txnMng.SetConfigManager(vsemConfig, TransactionManager.OpenMode.ReadMode);
            } catch (Exception ex) {
                ODLVSEMETW.EventWriteConfigManagerFileIOError(
                    MethodBase.GetCurrentMethod().Name,
                    string.Format(CultureInfo.CurrentCulture,
                    "VSEM.config {0}\n{1}",
                    configFileIOErrorValidationMessage,
                    ex.Message));
                throw new InvalidOperationException(
                    string.Format(CultureInfo.CurrentCulture,
                    "VSEM.config {0}",
                    configFileIOErrorValidationMessage));
            }
            string output = "\"NetworkServiceSystemInformation\":" + JavaScriptSerializer.Serialize(vsemConfig.NetworkServiceSystemInformation);
            ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name, output);
            return vsemConfig.NetworkServiceSystemInformation;
        }

        /// <summary>
        /// This method is responsible for extracting the VSEMSwitchExtensionInfo
        /// from the VSEM repository for the corresponding connection for the specified id.
        /// </summary>
        /// <param name="txnMng">Transaction manager.</param>
        /// <param name="id"> Id of the VSEMSwitchExtensionInfo.</param>
        /// <returns>VSEMSwitchExtensionInfo instance attached with the connection.</returns>
        public VSEMSwitchExtensionInfo GetVSEMSwitchExtensionInfoById(
            TransactionManager txnMng,
            Guid id) {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"TransactionManager\":" + JavaScriptSerializer.Serialize(txnMng));
            json.Append(" \"id\":" + id.ToString("B"));
            ODLVSEMETW.EventWriteStartLibrary(
                    MethodBase.GetCurrentMethod().Name,
                    json.ToString());
            ODLVSEMETW.EventWriteExtractSwitchExtensionInfo(
                "Extracting VSEMSwitchExtension Info.",
                string.Empty);
            if (id == Guid.Empty) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'id' is null or invalid.");
                throw new ArgumentException("'id' is null or invalid.");
            }
            var vsemConfig = new VSEMConfig();
            try {
                txnMng.SetConfigManager(vsemConfig, TransactionManager.OpenMode.ReadMode);
                var ret = vsemConfig.SwitchExtensionInfo.FirstOrDefault(
                     (x) => x.Id == id);
                string output = "\"VSEMSwitchExtensionInfo\":" + JavaScriptSerializer.Serialize(ret);
                ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name, output);
                return ret;
            } catch (Exception ex) {
                ODLVSEMETW.EventWriteConfigManagerFileIOError(
                    MethodBase.GetCurrentMethod().Name,
                    string.Format(CultureInfo.CurrentCulture,
                    "VSEM.config {0}\n{1}",
                    configFileIOErrorValidationMessage,
                    ex.Message));
                ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name,
                    string.Empty);
                throw new InvalidOperationException(
                    string.Format(CultureInfo.CurrentCulture,
                    "VSEM.config {0}",
                    configFileIOErrorValidationMessage));
            }
        }

        /// <summary>
        /// This method is responsible for extracting the list of VSEMSwitchExtensionInfo
        /// from the VSEM repository for the corresponding connection.
        /// </summary>
        /// <param name="txnMng">Transaction manager.</param>
        /// <returns>List of VSEMSwitchExtensionInfo instances
        /// attached with the connection.</returns>
        public List<VSEMSwitchExtensionInfo> GetVSEMSwitchExtensionInfo(
            TransactionManager txnMng) {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"TransactionManager\":" + JavaScriptSerializer.Serialize(txnMng));
            ODLVSEMETW.EventWriteStartLibrary(
                    MethodBase.GetCurrentMethod().Name,
                    json.ToString());
            ODLVSEMETW.EventWriteExtractSwitchExtensionInfolist(
                "Extracting list of VSEMSwitchExtension Info.",
                string.Empty);
            var vsemConfig = new VSEMConfig();
            try {
                txnMng.SetConfigManager(vsemConfig, TransactionManager.OpenMode.ReadMode);
            } catch (Exception ex) {
                ODLVSEMETW.EventWriteConfigManagerFileIOError(
                    MethodBase.GetCurrentMethod().Name,
                    string.Format(CultureInfo.CurrentCulture,
                    "VSEM.config {0}\n{1}",
                    configFileIOErrorValidationMessage,
                    ex.Message));
                throw new InvalidOperationException(
                    string.Format(CultureInfo.CurrentCulture,
                    "VSEM.config {0}",
                    configFileIOErrorValidationMessage));
            }
            string output = "\"VSEMSwitchExtensionInfo\":" + JavaScriptSerializer.Serialize(vsemConfig.SwitchExtensionInfo);
            ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name, output);
            return vsemConfig.SwitchExtensionInfo;
        }

        /// <summary>
        /// This method is responsible for extracting the LogicalNetwork
        /// from the VSEM repository for the corresponding connection for the specified id.
        /// </summary>
        /// <param name="txnMng">Transaction manager.</param>
        /// <param name="id">Id of the LogicalNetwork.</param>
        /// <returns>LogicalNetwork instance attached with the connection.</returns>
        public LogicalNetwork GetVSEMLogicalNetworkById(
            TransactionManager txnMng,
            Guid id) {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"TransactionManager\":" + JavaScriptSerializer.Serialize(txnMng));
            json.Append(" \"id\":" + id.ToString("B"));
            ODLVSEMETW.EventWriteStartLibrary(
                    MethodBase.GetCurrentMethod().Name,
                    json.ToString());
            ODLVSEMETW.EventWriteExtractFabricNetworkDefinition(
                "Extracting LogicalNetwork Info.",
                string.Empty);

            if (id == Guid.Empty) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'id' is null or invalid.");
                throw new ArgumentException("The parameter 'id' is null or invalid.");
            }
            string VtnHostName = this.ConnectionString;

            if (VtnHostName.StartsWith(@"https://", StringComparison.Ordinal)) {
                VtnHostName = VtnHostName.Substring(8);
            }

            VtnHostName = VtnHostName.Split('.', ':').First();
            var logicalNetworkConfig = new LogicalNetworkConfig(VtnHostName);
            try {
                txnMng.SetConfigManager(logicalNetworkConfig, TransactionManager.OpenMode.ReadMode);
                var ret = logicalNetworkConfig.LogicalNetworks.FirstOrDefault(
                (x) => x.Id == id);
                string output = "\"LogicalNetwork\":" + JavaScriptSerializer.Serialize(ret);
                ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name, output);
                return ret;
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
            }
        }

        /// <summary>
        /// This method is responsible for extracting the list of VSEMLogicalNetworkDefinition
        /// from the VSEM repository for the corresponding connection.
        /// </summary>
        /// <param name="txnMng">Transaction manager.</param>
        /// <returns>List of VSEMLogicalNetworkDefinition instances
        /// attached with the connection.</returns>
        public List<LogicalNetwork> GetVSEMLogicalNetwork(
            TransactionManager txnMng) {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"TransactionManager\":" + JavaScriptSerializer.Serialize(txnMng));
            ODLVSEMETW.EventWriteStartLibrary(
                    MethodBase.GetCurrentMethod().Name,
                    json.ToString());
            ODLVSEMETW.EventWriteExtractFabricNetworkDefinitionlist(
                "Extracting list of Logical Network Info.",
                string.Empty);
            string VtnHostName = this.ConnectionString;

            if (VtnHostName.StartsWith(@"https://", StringComparison.Ordinal)) {
                VtnHostName = VtnHostName.Substring(8);
            }

            VtnHostName = VtnHostName.Split('.', ':').First();
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
                throw new InvalidOperationException(
                    string.Format(CultureInfo.CurrentCulture,
                    "LogicalNetwork.config {0}",
                    configFileIOErrorValidationMessage));
            }
            string output = "\"LogicalNetwork\":" + JavaScriptSerializer.Serialize(logicalNetworkConfig.LogicalNetworks);
            ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name, output);
            return logicalNetworkConfig.LogicalNetworks;
        }

        /// <summary>
        /// This method is responsible for extracting the VSEMUplinkPortProfile
        /// from the VSEM repository for the corresponding connection for the specified id.
        /// </summary>
        /// <param name="txnMng">Transaction manager.</param>
        /// <param name="id">Id of the VSEMUplinkPortProfile.</param>
        /// <returns>VSEMUplinkPortProfile instance attached with the connection.</returns>
        public VSEMUplinkPortProfile GetVSEMUplinkPortProfileById(
            TransactionManager txnMng,
            Guid id) {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"TransactionManager\":" + JavaScriptSerializer.Serialize(txnMng));
            json.Append(" \"id\":" + id.ToString("B"));
            ODLVSEMETW.EventWriteStartLibrary(
                    MethodBase.GetCurrentMethod().Name,
                    json.ToString());
            ODLVSEMETW.EventWriteExtractUplinkPortProfile(
                "Extracting UplinkPortProfile Info.", string.Empty);
            ODLVSEMETW.EventWriteArgumentError(
                MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                MethodBase.GetCurrentMethod().Name,
                "The parameter 'id' is null or invalid.");
            if (id == Guid.Empty) {
                throw new ArgumentException("The parameter 'id' is null or invalid.");
            }

            var portProfileConfig = new PortProfileConfig();
            try {
                txnMng.SetConfigManager(portProfileConfig, TransactionManager.OpenMode.ReadMode);
                var ret = portProfileConfig.UplinkPortProfiles.FirstOrDefault(
                port => port.Id == id);
                string output = "\"LogicalNetwork\":" + JavaScriptSerializer.Serialize(ret);
                ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name, output);
                return ret;
            } catch (Exception ex) {
                ODLVSEMETW.EventWriteConfigManagerFileIOError(
                    MethodBase.GetCurrentMethod().Name,
                    string.Format(CultureInfo.CurrentCulture,
                    "PortProfile.config {0}\n{1}",
                    configFileIOErrorValidationMessage,
                    ex.Message));
                ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name, string.Empty);
                throw new InvalidOperationException(
                    string.Format(CultureInfo.CurrentCulture,
                    "PortProfile.config {0}",
                    configFileIOErrorValidationMessage));
            }
        }

        /// <summary>
        /// This method is responsible for extracting the list of VSEMUplinkPortProfile
        /// from the VSEM repository for the corresponding connection.
        /// </summary>
        /// <param name="txnMng">Transaction manager.</param>
        /// <returns>Lis of VSEMUplinkPortProfile instances
        /// attached with the connection.</returns>
        public List<VSEMUplinkPortProfile> GetVSEMUplinkPortProfile(
            TransactionManager txnMng) {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"TransactionManager\":" + JavaScriptSerializer.Serialize(txnMng));
            ODLVSEMETW.EventWriteStartLibrary(
                    MethodBase.GetCurrentMethod().Name,
                    json.ToString());
            ODLVSEMETW.EventWriteExtractUplinkPortProfilelist(
                "Extracting list of UplinkPortProfile Info.", string.Empty);
            var portProfileConfig = new PortProfileConfig();
            try {
                txnMng.SetConfigManager(portProfileConfig, TransactionManager.OpenMode.ReadMode);
            } catch (Exception ex) {
                ODLVSEMETW.EventWriteConfigManagerFileIOError(
                    MethodBase.GetCurrentMethod().Name,
                    string.Format(CultureInfo.CurrentCulture,
                    "PortProfile.config {0}\n{1}",
                    configFileIOErrorValidationMessage,
                    ex.Message));
                throw new InvalidOperationException(
                    string.Format(CultureInfo.CurrentCulture,
                    "PortProfile.config {0}",
                    configFileIOErrorValidationMessage));
            }
            string output = "\"VSEMUplinkPortProfile\":" + JavaScriptSerializer.Serialize(portProfileConfig.UplinkPortProfiles);
            ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name, output);
            return portProfileConfig.UplinkPortProfiles;
        }

        /// <summary>
        /// This method is responsible for extracting the VSEMVirtualPortProfile
        /// from the VSEM repository for the corresponding connection for the specified id.
        /// </summary>
        /// <param name="txnMng">Transaction manager.</param>
        /// <param name="id">Id of the VSEMVirtualPortProfile.</param>
        /// <returns>VSEMVirtualPortProfile instance attached with the connection.</returns>
        public VSEMVirtualPortProfile GetVSEMVirtualPortProfileById(
            TransactionManager txnMng,
            Guid id) {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"TransactionManager\":" + JavaScriptSerializer.Serialize(txnMng));
            json.Append(" \"id\":" + id.ToString("B"));
            ODLVSEMETW.EventWriteStartLibrary(
                    MethodBase.GetCurrentMethod().Name,
                    json.ToString());
            if (id == Guid.Empty) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'id' is null or invalid.");
                throw new ArgumentException(
                    "The parameter 'id' is null or invalid.");
            }

            ODLVSEMETW.EventWriteExtractVirtualPortProfile(
                "Extracting VirtualPortProfile Info.", string.Empty);
            var portProfileConfig = new PortProfileConfig();
            try {
                txnMng.SetConfigManager(portProfileConfig, TransactionManager.OpenMode.ReadMode);
                var ret = portProfileConfig.VirtualPortProfiles.FirstOrDefault(
                    port => port.Id == id);
                string output = "\"VSEMVirtualPortProfile\":" + JavaScriptSerializer.Serialize(ret);
                ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name, output);
                return ret;
            } catch (Exception ex) {
                ODLVSEMETW.EventWriteConfigManagerFileIOError(
                    MethodBase.GetCurrentMethod().Name,
                    string.Format(CultureInfo.CurrentCulture,
                    "PortProfile.config {0}\n{1}",
                    configFileIOErrorValidationMessage,
                    ex.Message));
                ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name, string.Empty);
                throw new InvalidOperationException(
                    string.Format(CultureInfo.CurrentCulture,
                    "PortProfile.config {0}",
                    configFileIOErrorValidationMessage));
            }
        }

        /// <summary>
        /// This method is responsible for extracting the list of VSEMVirtualPortProfile
        /// from the VSEM repository for the corresponding connection.
        /// </summary>
        /// <param name="txnMng">Transaction manager.</param>
        /// <returns>List of VSEMVirtualPortProfile instances
        /// attached with the connection.</returns>
        public List<VSEMVirtualPortProfile> GetVSEMVirtualPortProfile(
            TransactionManager txnMng) {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"TransactionManager\":" + JavaScriptSerializer.Serialize(txnMng));
            ODLVSEMETW.EventWriteStartLibrary(
                    MethodBase.GetCurrentMethod().Name,
                    json.ToString());
            ODLVSEMETW.EventWriteExtractVirtualPortProfilelist(
                "Extracting list of VirtualPortProfile Info.", string.Empty);
            var portProfileConfig = new PortProfileConfig();
            try {
                txnMng.SetConfigManager(portProfileConfig, TransactionManager.OpenMode.ReadMode);
            } catch (Exception ex) {
                ODLVSEMETW.EventWriteConfigManagerFileIOError(
                    MethodBase.GetCurrentMethod().Name,
                    string.Format(CultureInfo.CurrentCulture,
                    "PortProfile.config {0}\n{1}",
                    configFileIOErrorValidationMessage,
                    ex.Message));
                throw new InvalidOperationException(
                    string.Format(CultureInfo.CurrentCulture,
                    "PortProfile.config {0}",
                    configFileIOErrorValidationMessage));
            }
            string output = "\"VSEMVirtualPortProfile\":" + JavaScriptSerializer.Serialize(portProfileConfig.VirtualPortProfiles);
            ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name, output);
            return portProfileConfig.VirtualPortProfiles;
        }

        /// <summary>
        /// Get VTNCoordinatorhost name from private field.
        /// </summary>
        /// <returns>VTNCoordinator  host name string.</returns>
        public string GetVtnHostName() {
            return this.VtnHostName;
        }

        /// <summary>
        /// This method is responsible for extracting the VSEMVmNetwork
        /// from the VSEM repository for the corresponding connection for the specified id.
        /// </summary>
        /// <param name="txnMng">Transaction manager.</param>
        /// <param name="id">Id of the VSEMVmNetwork.</param>
        /// <returns>VSEMVmNetwork instance attached with the connection.</returns>
        public VMNetwork GetVSEMVMNetworkById(TransactionManager txnMng, Guid id) {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"TransactionManager\":" + JavaScriptSerializer.Serialize(txnMng));
            json.Append(" \"id\":" + id.ToString("B"));
            ODLVSEMETW.EventWriteStartLibrary(
                    MethodBase.GetCurrentMethod().Name,
                    json.ToString());
            if (id == Guid.Empty) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'id' is null or invalid.");
                throw new ArgumentException(
                    "The parameter 'id' is null or invalid.");
            }

            ODLVSEMETW.EventWriteExtractVMNetworkInfo(
                "Extracting VMNetwork Info.",
            string.Empty);
            string VtnHostName = this.ConnectionString;

            if (VtnHostName.StartsWith(@"https://", StringComparison.Ordinal)) {
                VtnHostName = VtnHostName.Substring(8);
            }

            VtnHostName = VtnHostName.Split('.', ':').First();
            var vMNetworkConfig = new VMNetworkConfig(VtnHostName);
            try {
                txnMng.SetConfigManager(vMNetworkConfig, TransactionManager.OpenMode.ReadMode);

                var ret = vMNetworkConfig.VMNetwork.VmNetworks.FirstOrDefault(
                    nw => nw.Id == id);
                string output = "\"VMNetwork\":" + JavaScriptSerializer.Serialize(ret);
                ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name,
                    output);
                return ret;
            } catch (System.IO.FileNotFoundException) {
                // Ignore if the file is not yet created and return empty list.
                return new VMNetwork();
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
        }

        /// <summary>
        /// Method to refresh the network service in VMM.
        /// </summary>
        public void RefreshConnection() {
            if (this.NeedRefresh != null) {
                this.NeedRefresh(this, null);
            } else {
                throw new InvalidOperationException("NeedRefresh method is not handled on SCVMM");
            }
        }

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
            "File is not yet created.");

        /// <summary>
        /// This method is responsible for extracting the list of VSEMVmNetwork
        /// from the VSEM repository for the corresponding connection.
        /// </summary>
        /// <param name="txnMng">Transaction manager.</param>
        /// <returns>List of VSEMVmNetwork instances attached with the connection.</returns>
        public List<VMNetwork> GetVSEMVMNetwork(TransactionManager txnMng) {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"TransactionManager\":" + JavaScriptSerializer.Serialize(txnMng));
            ODLVSEMETW.EventWriteStartLibrary(
                    MethodBase.GetCurrentMethod().Name,
                    json.ToString());
            ODLVSEMETW.EventWriteExtractVMNetworkInfolist(
                "Extracting list of VMNetwork Info.",
                string.Empty);
            string VtnHostName = this.ConnectionString;

            if (VtnHostName.StartsWith(@"https://", StringComparison.Ordinal)) {
                VtnHostName = VtnHostName.Substring(8);
            }

            VtnHostName = VtnHostName.Split('.', ':').First();
            var vMNetworkConfig = new VMNetworkConfig(VtnHostName);
            try {
                txnMng.SetConfigManager(vMNetworkConfig, TransactionManager.OpenMode.ReadMode);
                var ret = vMNetworkConfig.VMNetwork.VmNetworks;
                string output = "\"VMNetwork\":" + JavaScriptSerializer.Serialize(ret);
                ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name,
                    output);
                return ret;
            } catch (System.IO.FileNotFoundException) {
                // Ignore if the file is not yet created and return empty list.
                return new List<VMNetwork>();
            } catch (Exception ex) {
                ODLVSEMETW.EventWriteConfigManagerFileIOError(
                    MethodBase.GetCurrentMethod().Name,
                    string.Format(CultureInfo.CurrentCulture,
                    "VMNetwork.config {0}\n{1}",
                    configFileIOErrorValidationMessage,
                    ex.Message));
                throw new InvalidOperationException(
                    string.Format(CultureInfo.CurrentCulture,
                    "VMNetwork.config {0}",
                    configFileIOErrorValidationMessage));
            }
        }

        /// <summary>
        /// Performs application-defined tasks associated with freeing, releasing, or resetting unmanaged resources.
        /// </summary>
        /// <param name="disposing">Indicates whether to dispose or not.</param>
        protected virtual void Dispose(bool disposing) {
            if (disposing) {
                this.ConnectionString = null;
                this.controllers = null;
                this.Credential = null;
            }
        }

        /// <summary>
        /// Create VSEM system info object.
        /// </summary>
        /// <returns>VSEMSystemInfo instance.</returns>
        private static NetworkServiceSystemInformation CreateSystemInfo() {
            NetworkServiceSystemInformation sysInfo = new NetworkServiceSystemInformation();
            sysInfo.Id = VSEMODLConstants.SYSTEM_INFO_ID;
            sysInfo.Name = VSEMODLConstants.SYSTEM_INFO_NAME;
            sysInfo.Description = VSEMODLConstants.SYSTEM_INFO_DESCRIPTION;
            sysInfo.Manufacturer = VSEMODLConstants.SYSTEM_INFO_MANUFACTURER;
            sysInfo.Model = VSEMODLConstants.SYSTEM_INFO_MODEL;
            sysInfo.Version = VSEMODLConstants.SYSTEM_INFO_VERSION;
            sysInfo.NetworkDataInformation = new NetworkManagerCapabilities {
                ReceiveFabricNetworkEntitiesOnRefresh = VSEMODLConstants.RECEIVE_FABRIC_NETWORK_ENTITIES_ON_REFRESH,
                ReceiveFabricNetworkEntitiesOnUpdate = VSEMODLConstants.RECEIVE_FABRIC_NETWORK_ENTITIES_ON_UPDATE,
                ReceiveVMNetworkEntitiesOnRefresh = VSEMODLConstants.RECEIVE_VM_NETWORK_ENTITIES_ON_REFRESH,
                ReceiveVMNetworkEntitiesOnUpdate = VSEMODLConstants.RECEIVE_VM_NETWORK_ENTITIES_ON_UPDATE,
                SupportsIPAddressUsageInformation = VSEMODLConstants.SUPPORTS_IP_ADDRESS_USAGE_INFORMATION
            };

            sysInfo.DeviceId = VSEMODLConstants.SYSTEM_INFO_DEVICE_ID;
            sysInfo.SwitchExtensionManagementInformation = new VSEMCapabilities {
                SupportsVMNetworkOperations = VSEMODLConstants.SYSTEM_INFO_SUPPORTS_VM_NETWORK_OPERATIONS,
                VendorId = VSEMODLConstants.SYSTEM_INFO_VENDOR_ID,
                SwitchExtensionInfoIds = new Guid[1] { VSEMODLConstants.SWITCH_EXTENSION_INFO_ID }
            };

            return sysInfo;
        }

        /// <summary>
        /// Create VSEM switch extension infos.
        /// </summary>
        /// <returns>List of VSEMSwitchExtensionInfo instances.</returns>
        private static List<VSEMSwitchExtensionInfo> CreateSwitchExtensionInfos() {
            ODLVSEMETW.EventWriteCreateSwitchExtensionInfo(
                "Creating switch extension information.", string.Empty);

            List<VSEMSwitchExtensionInfo> info = new List<VSEMSwitchExtensionInfo>();
            VSEMSwitchExtensionInfo pf1000 = new VSEMSwitchExtensionInfo();
            pf1000.Id = VSEMODLConstants.SWITCH_EXTENSION_INFO_ID;
            pf1000.DriverNetCfgInstanceId = VSEMODLConstants.PF1000_NETCFG_INSTANCE_ID;
            pf1000.Name = VSEMODLConstants.SWITCH_EXTENSION_INFO_NAME;
            pf1000.MinVersion = VSEMODLConstants.SWITCH_EXTENSION_INFO_MIN_VERSION;
            pf1000.MaxVersion = VSEMODLConstants.SWITCH_EXTENSION_INFO_MAX_VERSION;
            pf1000.ExtensionType = VSEMODLConstants.SWITCH_EXTENSION_TYPE;
            pf1000.IsSwitchTeamSupported =
                VSEMODLConstants.SWITCH_EXTENSION_INFO_SUPPORT_SWITCH_TEAMING;
            pf1000.DefaultVirtualPortProfileId = VSEMODLConstants.VIRTUAL_PORT_PROFILE_ID;
            pf1000.MandatoryFeatureId =
                VSEMODLConstants.SWITCH_EXTENSION_INFO_MANDATORY_FEATURE_ID;
            pf1000.MaxNumberOfPorts = VSEMODLConstants.SWITCH_EXTENSION_INFO_MAX_NUMBER_OF_PORTS;
            pf1000.MaxNumberOfPortsPerHost = VSEMODLConstants.SWITCH_EXTENSION_INFO_MAX_NUMBER_OF_PORTS_PER_HOST;
            pf1000.SwitchExtSwitchFeatureConfigs = new VSEMSwitchExtSwitchFeatureConfig[0];
            pf1000.IsChildOfWFPSwitchExtension = VSEMODLConstants.SWITCH_EXTENSION_INFO_IS_CHILD_OF_WFP_SWITCH_EXTENSION;
            pf1000.LastModifiedTimeStamp = DateTime.Now;

            // Add this to the list to be returned
            info.Add(pf1000);

            return info;
        }

        /// <summary>
        /// Add switch extension switch feature config to
        /// the specified switch extension info.
        /// </summary>
        /// <param name="info">Switch extension info to add switch feature config.</param>
        /// <param name="CoordinatorUri">URI to VTNCoordinator.</param>
        private static void AddSwitchFeatureToSwitchExtInfos(VSEMSwitchExtensionInfo info,
            string CoordinatorUri) {
            if (info == null) {
                throw new ArgumentException("The parameter 'info' is null or invalid.");
            }

            if (string.IsNullOrEmpty(CoordinatorUri)) {
                throw new ArgumentException("The parameter 'VTNCoordinatorUri' is null or invalid.");
            }

            info.SwitchExtSwitchFeatureConfigs = new VSEMSwitchExtSwitchFeatureConfig[1];
            info.SwitchExtSwitchFeatureConfigs[0] = CreateSwitchExtSwitchFeatureConfig(CoordinatorUri);
        }

        /// <summary>
        /// Create VSEM switch extension switch feature config.
        /// </summary>
        /// <param name="ipAddr">IP address for ODL.</param>
        /// <returns>VSEMSwitchExtSwitchFeatureConfig instance.</returns>
        private static VSEMSwitchExtSwitchFeatureConfig CreateSwitchExtSwitchFeatureConfig(
            string ipAddr) {
            ODLVSEMETW.EventWriteCreateSwitchExtensionSwitchFeature(
                "Creating VSEM switch extension switch feature configuration.", string.Empty);

            if (string.IsNullOrEmpty(ipAddr)) {
                throw new ArgumentException("The parameter 'ipAddr' is null or invalid.");
            }

            VSEMSwitchExtSwitchFeatureConfig config = new VSEMSwitchExtSwitchFeatureConfig();

            config.Id = VSEMODLConstants.CONTROLLER_SWITCH_FEATURE_ID;
            config.Name = VSEMODLConstants.CONTROLLER_SWITCH_FEATURE_NAME;
            config.WmiClassName = VSEMODLConstants.CONTROLLER_SWITCH_FEATURE_WMI_CLASS_NAME;
            config.Properties = new PropertyNameValuePair[1] {
                new PropertyNameValuePair() {
                    PropertyName =
                        VSEMODLConstants.CONTROLLER_SWITCH_FEATURE_CONTROLLER_PROPERTY,
                    PropertyType = VSEMODLConstants.PROPERTY_TYPE,
                    IsArray =
                        VSEMODLConstants.CONTROLLER_SWITCH_FEATURE_CONTROLLER_PROPERTY_IS_ARRAY,
                    PropertyValue = ipAddr,
                    LastModifiedTimeStamp = DateTime.Now
                }
            };

            config.LastModifiedTimeStamp = DateTime.Now;

            return config;
        }

        /// <summary>
        /// Create VSEM uplink port profiles.
        /// </summary>
        /// <returns>List of VSEMUplinkPortProfile instances.</returns>
        private static List<VSEMUplinkPortProfile> CreateUplinkPortProfiles() {
            ODLVSEMETW.EventWriteCreateUplinkPortProfile(
                    "Creating uplink port profile for OpenFlow network.", string.Empty);

            List<VSEMUplinkPortProfile> profiles = new List<VSEMUplinkPortProfile>();

            // Create one uplink port profile for non HNV OpenFlow network
            VSEMUplinkPortProfile uplink = new VSEMUplinkPortProfile();
            uplink.Id = VSEMODLConstants.UPLINK_PORT_PROFILE_ID;
            uplink.Name = VSEMODLConstants.UPLINK_PORT_PROFILE_NAME;
            uplink.LastModifiedTimeStamp = DateTime.Now;
            uplink.SwitchExtensionInfoId = VSEMODLConstants.SWITCH_EXTENSION_INFO_ID;
            uplink.LogicalNetworkDefinitionIds =
                new Guid[1] { VSEMODLConstants.LOGICAL_NETWORK_DEFINITION_ID };
            uplink.Tags = new KeyValuePair<string, string>[] {
                new KeyValuePair<string, string>(
                    VSEMODLConstants.UPLINK_PORT_PROFILE_TAG_KEY, VSEMODLConstants.UPLINK_PORT_PROFILE_TAG_VALUE)
            };

            profiles.Add(uplink);

            return profiles;
        }

        /// <summary>
        /// Create VSEM virtual port profiles.
        /// </summary>
        /// <returns>List of VSEMVirtualPortProfile instances.</returns>
        private static List<VSEMVirtualPortProfile> CreateVirtualPortProfiles() {
            ODLVSEMETW.EventWriteCreateVirtualPortProfile(
                    "Creating virtual port profile for OpenFlow network.", string.Empty);

            List<VSEMVirtualPortProfile> profiles = new List<VSEMVirtualPortProfile>();

            // Create a virtual port profile
            VSEMVirtualPortProfile vp = new VSEMVirtualPortProfile();
            vp.Id = VSEMODLConstants.VIRTUAL_PORT_PROFILE_ID;
            vp.Name = VSEMODLConstants.VIRTUAL_PORT_PROFILE_NAME;
            vp.SwitchExtensionInfoId = VSEMODLConstants.SWITCH_EXTENSION_INFO_ID;
            vp.AllowedVNicType = VSEMODLConstants.VIRTUAL_PORT_PROFILE_ALLOWED_VNIC_TYPE;
            vp.MaxNumberOfPorts =
                VSEMODLConstants.VIRTUAL_PORT_PROFILE_MAX_NUMBER_OF_PORTS;
            vp.MaxNumberOfPortsPerHost =
                VSEMODLConstants.VIRTUAL_PORT_PROFILE_MAX_NUMBER_OF_PORTS_PER_HOST;
            vp.Tags = new KeyValuePair<string, string>[] {
                new KeyValuePair<string, string>(
                    VSEMODLConstants.VIRTUAL_PORT_PROFILE_TAG_KEY, VSEMODLConstants.VIRTUAL_PORT_PROFILE_TAG_VALUE)
            };

            vp.ProfileData = VSEMODLConstants.VIRTUAL_PORT_PROFILE_PROFILE_DATA;
            vp.LastModifiedTimeStamp = DateTime.Now;

            profiles.Add(vp);

            return profiles;
        }

        /// <summary>
        /// Saves the VTNCordinator information.
        /// </summary>
        /// <param name="txnMng">Transaction Manager.</param>
        /// <param name="VtnHostName">Host name of the VTNCoordinator.</param>
        private void SaveVtncoInfo(TransactionManager txnMng, string VtnHostName) {
            OdlInformation odlInfo = new OdlInformation(VtnHostName);
            txnMng.SetConfigManager(odlInfo, TransactionManager.OpenMode.WriteMode);
            odlInfo.SetCredentials(this.Credential, this.ConnectionString);
        }

        /// <summary>
        /// Create VSEM system information.
        /// </summary>
        /// <returns>VSEMInfo instance.</returns>
        private VSEMInfo CreateVSEMInfo() {
            VSEMInfo info = new VSEMInfo();
            string VtncoHost = this.ConnectionString;

            if (VtncoHost.StartsWith(@"https://", StringComparison.Ordinal)) {
                VtncoHost = VtncoHost.Substring(8);
            }

            info.ServerName = this.VtnHostName;
            info.Port = Convert.ToInt64(VtncoHost.Split(':').ElementAt(1), CultureInfo.CurrentCulture);
            info.MinVTNCoVersion = new Version(VSEMODLConstants.MIN_VTNCO_VERSION);
            info.LastModifiedTimeStamp = DateTime.Now;

            return info;
        }

        /// <summary>
        /// Create logical networks for HNV and non HNV openflow networks.
        /// </summary>
        /// <returns>List of LogicalNetwork instances.</returns>
        private List<LogicalNetwork> CreateLogicalNetworks() {
            List<LogicalNetwork> logicalNetworks = new List<LogicalNetwork>();

            string VtnHostName = this.ConnectionString;

            if (VtnHostName.StartsWith(@"https://", StringComparison.Ordinal)) {
                VtnHostName = VtnHostName.Substring(8);
            }

            VtnHostName = VtnHostName.Split('.', ':').First();

            // Create a logical network for the non HNV OpenFlow network
            LogicalNetwork logicalNet = new LogicalNetwork();
            logicalNet.Id = VSEMODLConstants.LOGICAL_NETWORK_ID;
            logicalNet.LastModifiedTimeStamp = DateTime.Now;
            logicalNet.Name = VSEMODLConstants.LOGICAL_NETWORK_NAME;
            logicalNet.Name += VtnHostName;
            logicalNet.AreLogicalNetworkDefinitionsIsolated =
                VSEMODLConstants.LOGICAL_NETWORK_ARE_LOGICAL_NETWORK_DEFINITIONS_ISOLATED;
            logicalNet.Description = VSEMODLConstants.LOGICAL_NETWORK_DESCRIPTION;
            logicalNet.LogicalNetworkDefinitions = new LogicalNetworkDefinition[1];
            logicalNet.LogicalNetworkDefinitions[0] = new LogicalNetworkDefinition {
                AllowsIntraPortCommunication =
                    VSEMODLConstants.LOGICAL_NETWORK_DEFINITION_ALLOWS_INTRAPORT_COMMUNICATION,
                EditableByNonOwners =
                    VSEMODLConstants.LOGICAL_NETWORK_DEFINITION_EDITABLE_BY_NON_OWNERS,
                Id = VSEMODLConstants.LOGICAL_NETWORK_DEFINITION_ID,
                LastModifiedTimeStamp = DateTime.Now,
                LogicalNetworkId = VSEMODLConstants.LOGICAL_NETWORK_ID,
                ManagedByNetworkServiceId = VSEMODLConstants.SYSTEM_INFO_ID,
                MarkedForDeletion =
                    VSEMODLConstants.LOGICAL_NETWORK_DEFINITION_MARKED_FOR_DELETION,
                MaximumVMSubnetsPerVMNetwork =
                    VSEMODLConstants.LOGICAL_NETWORK_DEFINITION_MAX_VM_NETWORK_DEFINITIONS_PER_VM_NETWORK,
                Name = VSEMODLConstants.LOGICAL_NETWORK_DEFINITION_NAME,
                SegmentIds = VSEMODLConstants.LOGICAL_NETWORK_DEFINITION_SEGMENT_IDS,
                SegmentType = VSEMODLConstants.LOGICAL_NETWORK_DEFINITION_SEGMENT_TYPE,
                SupportsIPSubnetConfigurationOnVMSubnets =
                    VSEMODLConstants.LOGICAL_NETWORK_DEFINITION_SUPPORTS_IP_SUBNET_CONFIGURATION_ON_VMSUBNETS,
                SupportsVMNetworkProvisioning =
                    VSEMODLConstants.LOGICAL_NETWORK_DEFINITION_SUPPORTS_VM_NETWORK_PROVISIONING,
                SynchronizationErrors = VSEMODLConstants.LOGICAL_NETWORK_DEFINITION_SYNCHRONIZATION_ERRORS,
                IPSubnets = VSEMODLConstants.LOGICAL_NETWORK_DEFINITION_IP_SUBNETS,
                LastModifiedBySystemId = VSEMODLConstants.LOGICAL_NETWORK_DEFINITION_LAST_MODIIED_BY_SYSTEM_ID
            };
            logicalNet.LogicalNetworkDefinitions[0].Name += VtnHostName;

            logicalNetworks.Add(logicalNet);

            return logicalNetworks;
        }

        /// <summary>
        /// If the plugin requires vmm refresher to be triggered,
        /// then plugin can indicate this event to VMM and VMM will initiate refresher for the plugin.
        /// </summary>
        public event EventHandler<NetworkServiceRefreshEventArgs> NeedRefresh;
    }
}
