//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//
//     This program and the accompanying materials are made available under the
//     terms of the Eclipse Public License v1.0 which accompanies this
//     distribution, and is available at http://www.eclipse.org/legal/epl-v10.html


using System;
using System.Globalization;
using System.Linq;
using System.Management.Automation;
using System.Text;
using System.Web.Script.Serialization;
using Microsoft.SystemCenter.NetworkService;
using ODL.VSEMProvider.Cmdlets.Common;
using ODL.VSEMProvider.Libraries;
using ODL.VSEMProvider.Libraries.Common;
using ODL.VSEMProvider.VSEMEvents;
using VSEM.Cmdlets.Common;

namespace ODL.VSEMProvider.Cmdlets {
    /// <summary>
    /// This class represents a cmdlet.
    /// This cmdlet creates the VM Subnet on an existing VM Network on the VSEM.
    /// </summary>
    [Cmdlet("Add", "Odl.VSEMVMSubnet")]
    [System.Management.Instrumentation.ManagedName("Microsoft.SystemCenter.NetworkService.AddVMSubnet")]
    [OutputType(typeof(VMSubnet))]
    public class AddVSEMVMSubnet : VSEMODLCmdletBase {
        /// <summary>
        /// This parameter specifies Connection object.
        /// </summary>
        private IConnection connection;

        /// <summary>
        /// This parameter specifies Connection object.
        /// </summary>
        [Parameter(ValueFromPipeline = true)]
        public IConnection Connection {
            get {
                return this.connection;
            }

            set {
                this.connection = value;
            }
        }

        /// <summary>
        /// Nme of the VSEMVmNetworkDefinition.
        /// </summary>
        private string name;

        /// <summary>
        /// Nme of the VSEMVmNetworkDefinition.
        /// </summary>
        [Parameter(ValueFromPipeline = true)]
        public string Name {
            get {
                return this.name;
            }

            set {
                this.name = value;
            }
        }

        /// <summary>
        /// Id of the VM network on which VSEMVmNetworkDefinition is to be created.
        /// </summary>
        private Guid vMNetworkId;

        /// <summary>
        /// Id of the VM network on which VSEMVmNetworkDefinition is to be created.
        /// </summary>
        [Parameter(ValueFromPipeline = true)]
        public Guid VMNetworkId {
            get {
                return this.vMNetworkId;
            }

            set {
                this.vMNetworkId = value;
            }
        }

        /// <summary>
        /// This parameter specifies Logical network definition ID
        /// where the VM network is created .
        /// </summary>
        private Guid logicalNetworkDefinitionId;

        /// <summary>
        /// This parameter specifies Logical network definition ID
        /// where the VM network is created .
        /// </summary>
        [Parameter(ValueFromPipeline = true)]
        public Guid LogicalNetworkDefinitionId {
            get {
                return this.logicalNetworkDefinitionId;
            }

            set {
                this.logicalNetworkDefinitionId = value;
            }
        }

        /// <summary>
        /// This parameter specifies  Maximum value of the number of ports supported
        /// in VM Subnet.If it is not specified,
        /// the port count prescribed by VSEM gets configured.
        /// </summary>
        private long? maxNumberOfPorts;

        /// <summary>
        /// This parameter specifies  Maximum value of the number of ports supported
        /// in VM Subnet.If it is not specified,
        /// the port count prescribed by VSEM gets configured.
        /// </summary>
        [Parameter(Mandatory = false)]
        public long? MaxNumberOfPorts {
            get {
                return this.maxNumberOfPorts;
            }

            set {
                this.maxNumberOfPorts = value;
            }
        }

        /// <summary>
        /// This parameter specifies If the value of the property
        /// SupportsIPSubnetConfigurationOnVMNetworkDefinitions is “True” in logical network
        /// definition, this parameter is specified.
        /// </summary>
        private IPSubnet[] ipSubnets;

        /// <summary>
        /// This parameter specifies If the value of the property
        /// SupportsIPSubnetConfigurationOnVMNetworkDefinitions is “True” in logical network
        /// definition, this parameter is specified.
        /// </summary>
        [Parameter(Mandatory = false)]
        public IPSubnet[] IPSubnets {
            get {
                return this.ipSubnets;
            }

            set {
                this.ipSubnets = value;
            }
        }

        /// <summary>
        /// The connection object parsed to VSEMConnection.
        /// </summary>
        private VSEMConnection conn;

        /// <summary>
        /// This function is responsible for validating the parameters.
        /// </summary>
        protected override void BeginODLVSEMCmdlet() {
            if (this.Connection == null
                || this.IPSubnets == null
                || this.VMNetworkId == null
                || this.VMNetworkId == Guid.Empty
                || this.LogicalNetworkDefinitionId == null
                || this.LogicalNetworkDefinitionId == Guid.Empty
                || string.IsNullOrWhiteSpace(this.Name)) {
                ODLVSEMETW.EventWriteValidateCmdletParameter(this.CmdletName,
                    "Mandatory parameter(s) not provided.");
                throw new NSPluginArgumentException("Mandatory parameter(s) not provided.");
            }
            this.conn = VSEMODLCmdletUtility.ValidateConnectionObject(this.Connection, this.CmdletName);
            if (this.IPSubnets.Any(subnet => string.IsNullOrEmpty(subnet.Subnet)
                || subnet.AddressFamily == null)) {
                ODLVSEMETW.EventWriteValidateCmdletParameter(this.CmdletName,
                    "IPSubnets is invalid. Please provide AddressFamily and  Subnet in IPSubnets.");
                throw new NSPluginArgumentException("IPSubnets is invalid. Please provide AddressFamily and Subnet in IPSubnets.");
            }
        }

        /// <summary>
        /// This function is responsible for creating a VM Subnet.
        /// </summary>
        protected override void DoODLVSEMCmdlet() {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder(" \"IPSubnets\":" + JavaScriptSerializer.Serialize(this.IPSubnets));
            json.Append("\"MaxNumberOfPorts\":\"" + this.MaxNumberOfPorts + "\"");
            json.Append("\"Name\":\"" + this.Name + "\"");
            json.Append("\"VMNetworkId\":\"" + this.VMNetworkId + "\"");
            json.Append("\"LogicalNetworkDefinitionId\":\"" + this.LogicalNetworkDefinitionId + "\"");
            ODLVSEMETW.EventWriteDoCmdlet(this.CmdletName,
                    "Creating VM Subnet.",
                    json.ToString());
            TransactionManager txnMng = new TransactionManager();
            txnMng.StartTransaction();
            var ope = TransactionManager.Operation.None;
            string vtnName = string.Empty;
            string vbrName = string.Empty;
            VMSubnet nw = null;
            string connectionString =
                this.conn.ConnectionString.Split(',').FirstOrDefault();
            VSEMVMSubnetManagement vSEMVMSubnetManagement =
                new VSEMVMSubnetManagement(connectionString,
                    this.conn.Credential);
            try {
                nw = vSEMVMSubnetManagement.CreateVMNetworkDefinition(txnMng,
                    this.Name,
                    this.VMNetworkId,
                    this.MaxNumberOfPorts,
                    this.IPSubnets,
                    this.LogicalNetworkDefinitionId,
                    this.conn,
                    out vtnName,
                    out vbrName);
                ODLVSEMETW.EventWriteReturnLibrary("VM Subnet is created.",
                    string.Empty);

                ope = TransactionManager.Operation.Commit;
            }
            catch (Exception ex) {
                    Exception userException = ex;
                    ODLVSEMETW.EventWriteFailedCmdlet(
                        "VM Subnet creation is failed. ODL changes rollback is started.",
                        string.Empty);
                    ope = TransactionManager.Operation.Rollback;
                    try {
                        if (!string.IsNullOrEmpty(vtnName) && !string.IsNullOrEmpty(vbrName)) {
                            vSEMVMSubnetManagement.RemoveVmNetworkDefinition(
                                vtnName, vbrName);
                            ODLVSEMETW.EventWriteFailedCmdlet(
                                "VM Subnet creation is failed. ODL changes are successfully rolled back.",
                                string.Empty);
                        }
                    } catch (Exception excep) {
                        // VM Network Creation Process is terminated in the middle of the request. VTN 
                        ODLVSEMETW.EventWriteFailedCmdlet(
                            string.Format(
                            CultureInfo.CurrentCulture,
                            "Due to some problem in connection with ODL, VSEM VM Subnet creation process terminated in the middle of the request. vBridge '{0}' may have been created on ODL, Please check. If inconsistency is created, Please delete the vBridge from ODL to maintain the consistency between ODL and SCVMM.\n{1}",
                            vbrName,
                            excep.Message),
                            string.Empty);
                        userException = new System.Net.WebException(string.Format(
                            CultureInfo.CurrentCulture,
                            "Due to some problem in connection with ODL, VSEM VM Subnet creation process terminated in the middle of the request. vBridge '{0}' may have been created on ODL, Please check. If inconsistency is created, Please delete the vBridge from ODL to maintain the consistency between ODL and SCVMM.",
                            vbrName),
                            ex);
                    }
                    Exception exception = VSEMODLExceptionUtil.ConvertExceptionToVSEMException(userException);
                    ODLVSEMETW.EventWriteFailedCmdlet(this.CmdletName, exception.GetType() + " : " + ex.Message);
                    throw exception;
            } finally {
                txnMng.EndTransaction(ope);

                string output = "\"VMSubnet\":" + JavaScriptSerializer.Serialize(nw);
                ODLVSEMETW.EventWriteEndCmdlet(this.CmdletName, output);
                this.WriteObject(nw);
            }
        }
    }
}
