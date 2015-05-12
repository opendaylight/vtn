//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html


using System;
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
    /// This class represents a cmdlet. This cmdlet is used to publish VM Network to VSEM Provider.
    /// </summary>
    [Cmdlet("Publish", "Odl.VSEMVMNetwork")]
    [OutputType(typeof(VMNetwork))]
    [System.Management.Instrumentation.ManagedName("Microsoft.SystemCenter.NetworkService.PublishVMNetwork")]
    public sealed class PublishVSEMVMNetwork : VSEMODLCmdletBase {
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
        /// The parameter indicates whether the entity is being created/updated or deleted.
        /// </summary>
        private NetworkEntityPublishType operationType;

        /// <summary>
        /// The parameter indicates whether the entity is being created/updated or deleted.
        /// </summary>
        [Parameter(ValueFromPipeline = true)]
        public NetworkEntityPublishType OperationType {
            get {
                return this.operationType;
            }

            set {
                this.operationType = value;
            }
        }

        /// <summary>
        /// This parameter specifies VM network that is being indicated to the provider/network service.
        /// </summary>
        private VMNetwork vMNetwork;

        /// <summary>
        /// This parameter specifies VM network that is being indicated to the provider/network service.
        /// </summary>
        [Parameter(ValueFromPipeline = true)]
        public VMNetwork VMNetwork {
            get {
                return this.vMNetwork;
            }

            set {
                this.vMNetwork = value;
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
                || this.VMNetwork == null) {
                ODLVSEMETW.EventWriteValidateCmdletParameter(this.CmdletName,
                    "Mandatory parameter(s) not provided.");
                throw new NSPluginArgumentException("Mandatory parameter(s) not provided.");
            }
            this.conn = VSEMODLCmdletUtility.ValidateConnectionObject(this.Connection, this.CmdletName);
        }

        /// <summary>
        /// This function is responsible for creating a VM Network
        /// with one VM Subnet on the VSEM.
        /// </summary>
        protected override void DoODLVSEMCmdlet() {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder(" \"VMNetwork\":" + JavaScriptSerializer.Serialize(this.VMNetwork));
            json.Append(" \"OperationType\":" + JavaScriptSerializer.Serialize(this.OperationType));
            ODLVSEMETW.EventWriteDoCmdlet(this.CmdletName,
                   "VM Network creation process started.",
                   json.ToString());

            // If VMNetwork is HNV resource, return immediately.
            var mgmt = new HNVVMNetworkManagement(this.conn.GetVtnHostName());
            if (mgmt.IsHNVVMNetwork(this.VMNetwork) == true) {
                return;
            }

            TransactionManager txnMng = new TransactionManager();
            txnMng.StartTransaction();
            var ope = TransactionManager.Operation.None;
            VMNetwork nw = null;
            string connectionString =
                this.conn.ConnectionString.Split(',').FirstOrDefault();
            VSEMSynchronization vSEMSynchronization =
                new VSEMSynchronization(connectionString, this.conn.Credential);
            try {
                bool needRefresh = vSEMSynchronization.PublishVMNetwork(txnMng,
                    this.VMNetwork,
                    this.OperationType,
                    this.conn);
                ODLVSEMETW.EventWriteReturnLibrary("VM network is updated.",
                    string.Empty);
                if (needRefresh) {
                    this.conn.RefreshConnection();
                }
                ope = TransactionManager.Operation.Commit;
            } catch (Exception ex) {
                Exception exception = VSEMODLExceptionUtil.ConvertExceptionToVSEMException(ex);
                ODLVSEMETW.EventWriteFailedCmdlet(this.CmdletName, exception.GetType() + " : " + ex.Message);
                ope = TransactionManager.Operation.Rollback;
                throw exception;
            } finally {
                txnMng.EndTransaction(ope);

                string output = "\"VM Network\":" + JavaScriptSerializer.Serialize(nw);
                ODLVSEMETW.EventWriteEndCmdlet(this.CmdletName, output);
                this.WriteObject(this.VMNetwork);
            }
        }
    }
}
