//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html

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
    /// This cmdlet is used to remove VSEM VM Subnet.
    /// </summary>
    [Cmdlet(VerbsCommon.Remove, "Odl.VSEMVMSubnet")]
    [System.Management.Instrumentation.ManagedName("Microsoft.SystemCenter.NetworkService.RemoveVMSubnet")]
    public sealed class RemoveVSEMVMSubnet : VSEMODLCmdletBase {
        /// <summary>
        /// Id of vmnetwork definition that is managed by this Extension Manager.
        /// </summary>
        private Guid vMSubnetId;

        /// <summary>
        /// Id of vmnetwork definition that is managed by this Extension Manager.
        /// </summary>
        [Parameter(ValueFromPipeline = true)]
        public Guid VMSubnetId {
            get {
                return this.vMSubnetId;
            }

            set {
                this.vMSubnetId = value;
            }
        }

        /// <summary>
        /// Connection object returned by the Open Connection cmdlet with the target extension
        /// manager. This is a mandatory parameter.
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
        /// The connection object parsed to VSEMConnection.
        /// </summary>
        private VSEMConnection conn;

        /// <summary>
        /// Name of the VM Subnet.
        /// </summary>
        private string name;

        /// <summary>
        /// This function is responsible for validating the parameters.
        /// </summary>
        protected override void BeginODLVSEMCmdlet() {
            if (this.Connection == null
                || this.VMSubnetId == Guid.Empty
                || this.VMSubnetId == null) {
                ODLVSEMETW.EventWriteValidateCmdletParameter(this.CmdletName,
                    "Mandatory parameter(s) not provided.");
                throw new NSPluginArgumentException("Mandatory parameter(s) not provided.");
            }
            this.conn = VSEMODLCmdletUtility.ValidateConnectionObject(this.Connection, this.CmdletName);
        }

        /// <summary>
        /// This function is responsible to remove the information about the
        /// vm network(s) for the connection.
        /// </summary>
        protected override void DoODLVSEMCmdlet() {
            TransactionManager txnMng = new TransactionManager();
            txnMng.StartTransaction();
            var ope = TransactionManager.Operation.None;
            bool needRefresh = false;
            VSEMVMSubnetManagement vSEMVmNetworkDefinitionManagement = null;
            try {
                var JavaScriptSerializer = new JavaScriptSerializer();
                JavaScriptSerializer.MaxJsonLength = int.MaxValue;
                StringBuilder json = new StringBuilder("\"VMSubnetId\":\"" + this.VMSubnetId + "\"");
                ODLVSEMETW.EventWriteDoCmdlet(this.CmdletName,
                    "Removing VMSubnet.",
                    json.ToString());
                string connectionString =
                this.conn.ConnectionString.Split(',').FirstOrDefault();
                vSEMVmNetworkDefinitionManagement =
                    new VSEMVMSubnetManagement(connectionString,
                this.conn.Credential);
                needRefresh = vSEMVmNetworkDefinitionManagement.RemoveVmNetworkDefinition(
                    txnMng,
                    this.VMSubnetId,
                    this.conn,
                    out this.name);
                ODLVSEMETW.EventWriteReturnLibrary(string.Format(CultureInfo.CurrentCulture,
                "VM Subnet with ID: {0} is removed",
                this.VMSubnetId.ToString("B")),
                string.Empty);

                ope = TransactionManager.Operation.Commit;
            } catch (Exception ex) {
                ope = TransactionManager.Operation.Rollback;
                if (!string.IsNullOrEmpty(this.name)
                    && string.Compare(ex.GetType().ToString(), "System.Net.WebException", StringComparison.Ordinal) == 0) {
                    ODLVSEMETW.EventWriteFailedCmdlet(string.Format(CultureInfo.CurrentCulture,
                        "Due to some problem in connection with ODL, VSEM VM Subnet '{0}' removal process is terminated in the middle of the request. vBridge may have been deleted from ODL, Please check. Please retry and delete VM Subnet if inconsistency is created.",
                    this.name),
                    string.Empty);
                    Exception exception = VSEMODLExceptionUtil.ConvertExceptionToVSEMException(
                        new System.Net.WebException(string.Format(CultureInfo.CurrentCulture,
                            "Due to some problem in connection with ODL, VSEM VM Subnet '{0}' removal process is terminated in the middle of the request. vBridge may have been deleted from ODL, Please check. Please retry and delete VM Subnet if inconsistency is created.",
                            this.name),
                            ex));
                    ODLVSEMETW.EventWriteFailedCmdlet(this.CmdletName, exception.GetType() + " : " + ex.Message);
                    ope = TransactionManager.Operation.Rollback;
                    throw exception;
                } else {
                    ODLVSEMETW.EventWriteFailedCmdlet(
                        "VSEM VM Subnet removal is failed.",
                        string.Empty);
                    Exception exception = VSEMODLExceptionUtil.ConvertExceptionToVSEMException(ex);
                    ODLVSEMETW.EventWriteFailedCmdlet(this.CmdletName, exception.GetType() + " : " + exception.Message);
                    ope = TransactionManager.Operation.Rollback;
                    throw exception;
                }
            } finally {
                txnMng.EndTransaction(ope);
                ODLVSEMETW.EventWriteEndCmdlet(this.CmdletName, string.Empty);
            }
        }
    }
}
