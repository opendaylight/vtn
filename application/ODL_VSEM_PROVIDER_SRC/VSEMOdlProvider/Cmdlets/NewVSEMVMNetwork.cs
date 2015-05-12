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
    /// This class represents a cmdlet. This cmdlet is used to create a VM Network with one
    /// VM Subnet on VSEM.
    /// </summary>
    [Cmdlet(VerbsCommon.New, "Odl.VSEMVMNetwork")]
    [OutputType(typeof(VMNetwork))]
    [System.Management.Instrumentation.ManagedName("Microsoft.SystemCenter.NetworkService.NewVMNetwork")]
    public sealed class NewVSEMVMNetwork : VSEMODLCmdletBase {
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
        /// This parameter specifies VM Subnet name that is to be newly created.
        /// </summary>
        private string vMSubnetName;

        /// <summary>
        /// This parameter specifies VM Subnet name that is to be newly created.
        /// </summary>
        [Parameter(Mandatory = false)]
        public string VMSubnetName {
            get {
                return this.vMSubnetName;
            }

            set {
                this.vMSubnetName = value;
            }
        }

        /// <summary>
        /// This parameter specifies VM network name that is to be created.
        /// </summary>
        private string vMNetworkName;

        /// <summary>
        /// This parameter specifies VM network name that is to be created.
        /// </summary>
        [Parameter(ValueFromPipeline = true)]
        public string VMNetworkName {
            get {
                return this.vMNetworkName;
            }

            set {
                this.vMNetworkName = value;
            }
        }

        /// <summary>
        /// This parameter specifies logical network definition ID
        /// where the VM network is created .
        /// </summary>
        private Guid logicalNetworkDefinitionId;

        /// <summary>
        /// This parameter specifies logical network definition ID
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
                || string.IsNullOrWhiteSpace(this.VMSubnetName)
                || this.IPSubnets == null
                || this.LogicalNetworkDefinitionId == null
                || this.LogicalNetworkDefinitionId == Guid.Empty
                || string.IsNullOrWhiteSpace(this.VMNetworkName)) {
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
        /// This function is responsible for creating a VM Network
        /// with one VM Subnet on the VSEM.
        /// </summary>
        protected override void DoODLVSEMCmdlet() {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder(" \"IPSubnets\":" + JavaScriptSerializer.Serialize(this.IPSubnets));
            json.Append("\"MaxNumberOfPorts\":\"" + this.MaxNumberOfPorts + "\"");
            json.Append("\"VMSubnetName\":\"" + this.VMSubnetName + "\"");
            json.Append("\"VMNetworkName\":\"" + this.VMNetworkName + "\"");
            json.Append("\"LogicalNetworkDefinitionId\":\"" + this.LogicalNetworkDefinitionId + "\"");
            ODLVSEMETW.EventWriteDoCmdlet(this.CmdletName,
                   "VM Network creation process started.",
                   json.ToString());
            TransactionManager txnMng = new TransactionManager();
            txnMng.StartTransaction();
            var ope = TransactionManager.Operation.None;
            VMNetwork nw = null;
            string vtnName = string.Empty;
            string connectionString =
                this.conn.ConnectionString.Split(',').FirstOrDefault();
            VSEMVMNetworkManagement vSEMVmNetworkManagement =
                new VSEMVMNetworkManagement(connectionString, this.conn.Credential);
            try {
                nw = vSEMVmNetworkManagement.CreateVMNetwork(txnMng,
                    this.VMSubnetName,
                    this.VMNetworkName,
                    this.MaxNumberOfPorts,
                    this.IPSubnets,
                    this.LogicalNetworkDefinitionId,
                    this.conn,
                    out vtnName);
                ODLVSEMETW.EventWriteReturnLibrary("VM network is created.",
                    string.Empty);

                ope = TransactionManager.Operation.Commit;
            } catch (Exception ex) {
                Exception userException = ex;
                ODLVSEMETW.EventWriteFailedCmdlet(
                    "VM network creation is failed. ODL changes rollback is started.",
                    string.Empty);
                ope = TransactionManager.Operation.Rollback;
                try {
                    if (!string.IsNullOrEmpty(vtnName)) {
                        vSEMVmNetworkManagement.RemoveVmNetwork(vtnName);
                        ODLVSEMETW.EventWriteFailedCmdlet(
                            "VM network creation is failed. ODL changes are successfully rolled back.",
                            string.Empty);
                    }
                } catch (Exception except) {
                    ODLVSEMETW.EventWriteFailedCmdlet(
                        string.Format(CultureInfo.CurrentCulture,
                        "Due to some problem in connection with ODL, VSEM VM Network creation process is terminated in the middle of the request. VTN '{0}' may have been created on ODL, Please check. Please delete the VTN from ODL to maintain the consistency between ODL and SCVMM.\n{1}",
                        vtnName,
                        except.Message),
                        string.Empty);
                    userException =
                        new System.Net.WebException(string.Format(CultureInfo.CurrentCulture,
                            "Due to some problem in connection with ODL, VSEM VM Network creation process is terminated in the middle of the request. VTN '{0}' may have been created on ODL, Please check. Please delete the VTN from ODL to maintain the consistency between ODL and SCVMM.",
                            vtnName),
                            ex);
                }

                Exception exception = VSEMODLExceptionUtil.ConvertExceptionToVSEMException(userException);
                ODLVSEMETW.EventWriteFailedCmdlet(this.CmdletName, exception.GetType() + " : " + ex.Message);
                ope = TransactionManager.Operation.Rollback;
                throw exception;
            } finally {
                txnMng.EndTransaction(ope);

                string output = "\"VM Network\":" + JavaScriptSerializer.Serialize(nw);
                ODLVSEMETW.EventWriteEndCmdlet(this.CmdletName, output);
                this.WriteObject(nw);
            }
        }
    }
}
