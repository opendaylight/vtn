//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html

using System;
using System.Management.Automation;
using System.Management.Instrumentation;
using System.Web.Script.Serialization;
using Microsoft.SystemCenter.NetworkService;
using Microsoft.SystemCenter.NetworkService.NetworkManager;
using ODL.VSEMProvider.Cmdlets.Common;
using ODL.VSEMProvider.Libraries;
using ODL.VSEMProvider.Libraries.Common;
using ODL.VSEMProvider.VSEMEvents;

namespace ODL.VSEMProvider.Cmdlets {
    /// <summary>
    /// This cmdlet updates the information about the specified logical network.
    /// </summary>
    /// <remarks>
    /// VMM calls this cmdlets when updates to the logical network is made.
    /// </remarks>
    [Cmdlet(VerbsData.Publish, "Odl.LogicalNetwork")]
    [OutputType(typeof(LogicalNetwork))]
    [System.Management.Instrumentation.ManagedName("Microsoft.SystemCenter.NetworkService.PublishLogicalNetwork")]
    public sealed class PublishLogicalNetwork : VSEMODLCmdletBase {
        /// <summary>
        /// Cmdlet parameters.
        /// </summary>
        [Parameter(Mandatory = true, ValueFromPipeline = true)]
        public IConnection Connection;

        /// <summary>
        /// Publish parameters.
        /// </summary>
        [Parameter(Mandatory = true, ValueFromPipeline = false)]
        public NetworkEntityPublishType OperationType;

        /// <summary>
        /// Target LogicalNetwork instance.
        /// </summary>
        [Parameter(Mandatory = true, ValueFromPipeline = false)]
        public LogicalNetwork LogicalNetwork;

        /// <summary>
        /// The connection object parsed to VSEMConnection.
        /// </summary>
        private VSEMConnection conn;

        /// <summary>
        /// Validate the parameters.
        /// </summary>
        protected override void BeginODLVSEMCmdlet() {
            this.conn = this.Connection as VSEMConnection;
            if (this.conn == null) {
                ODLVSEMETW.EventWriteValidateConnectionObjectError(this.CmdletName,
                    "Connection object is NULL.");
                throw new NSPluginArgumentException("Invalid connection object.");
            }

            if (string.IsNullOrEmpty(this.conn.ConnectionString)) {
                ODLVSEMETW.EventWriteValidateVSEMRepositoryError(this.CmdletName,
                    "Invalid connection object.");
                throw new NSPluginInvalidOperationException(
                    "Invalid connection object.");
            }

            if ((this.LogicalNetwork == null) ||
                (this.LogicalNetwork.LogicalNetworkDefinitions == null) ||
                (this.LogicalNetwork.LogicalNetworkDefinitions.Length == 0)) {
                throw new NSPluginArgumentException(
                    "No logical network definitions found.");
            }
        }

        /// <summary>
        /// Process the updated logical network.
        /// </summary>
        /// <remarks>
        /// Ignore logical network that does not belong to us.
        /// </remarks>
        protected override void DoODLVSEMCmdlet() {
            var hnvLogicalNetworkManagement = new HNVLogicalNetworkManagement(this.conn.GetVtnHostName());

            // Make sure we are managing this logical network.
            if (hnvLogicalNetworkManagement.IsHNVLogicalNetwork(this.LogicalNetwork) == true) {
                var txnMng = new TransactionManager();
                txnMng.StartTransaction();
                var ope = TransactionManager.Operation.None;
                bool writeResult = false;

                try {
                    switch (this.OperationType) {
                        case NetworkEntityPublishType.Create:
                            {
                                // Create is not supported through VMM.
                                writeResult = true;
                            }
                            break;

                        case NetworkEntityPublishType.Update:
                            {
                                hnvLogicalNetworkManagement.UpdateLogicalNetwork(txnMng, this.LogicalNetwork);
                                writeResult = hnvLogicalNetworkManagement.IsLogicalNetworkValid(this.LogicalNetwork);
                            }
                            break;

                        case NetworkEntityPublishType.Delete:
                            {
                                // Delete is not supported through VMM.
                                writeResult = true;
                            }
                            break;
                    }

                    ope = TransactionManager.Operation.Commit;
                } catch (Exception ex) {
                    ope = TransactionManager.Operation.Rollback;
                    throw VSEMODLExceptionUtil.ConvertExceptionToVSEMException(ex);
                } finally {
                    txnMng.EndTransaction(ope);
                }

                string output = "\"Logical Network\":" + new JavaScriptSerializer().Serialize(this.LogicalNetwork);
                ODLVSEMETW.EventWriteEndCmdlet(this.CmdletName, output);

                // Indicate errors.
                if (!writeResult) {
                    this.WriteObject(this.LogicalNetwork);
                }
            }
        }
    }
}
