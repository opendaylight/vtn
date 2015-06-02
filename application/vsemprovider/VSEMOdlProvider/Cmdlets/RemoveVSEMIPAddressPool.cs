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
    /// This cmdlet removes a Static IP Address Pool from an existing VM Subnet.
    /// </summary>
    [Cmdlet(VerbsCommon.Remove, "Odl.VSEMIPAddressPool")]
    [System.Management.Instrumentation.ManagedName("Microsoft.SystemCenter.NetworkService.RemoveIPAddressPool")]
    public sealed class RemoveVSEMIPAddressPool : VSEMODLCmdletBase {
        /// <summary>
        /// Connection object returned in Cmdlet during connection start
        /// by target extension manager.
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
        /// Static IP address pool ID .
        /// </summary>
        private Guid ipAddressPoolId;

        /// <summary>
        /// Static IP address pool ID .
        /// </summary>
        [Parameter(ValueFromPipeline = true)]
        public Guid IPAddressPoolId {
            get {
                return this.ipAddressPoolId;
            }

            set {
                this.ipAddressPoolId = value;
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
                || this.IPAddressPoolId == Guid.Empty
                || this.IPAddressPoolId == null) {
                ODLVSEMETW.EventWriteValidateCmdletParameter(this.CmdletName,
                    "Mandatory parameter(s) not provided.");
                throw new NSPluginArgumentException("Mandatory parameter(s) not provided.");
            }
            this.conn = VSEMODLCmdletUtility.ValidateConnectionObject(this.Connection, this.CmdletName);
        }

        /// <summary>
        /// This method is responsible to remove static Ip address pool.
        /// </summary>
        protected override void DoODLVSEMCmdlet() {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            TransactionManager txnMng = new TransactionManager();
            txnMng.StartTransaction();
            var ope = TransactionManager.Operation.None;
            VSEMIPAddressPoolManagement vSEMIPAddressPoolManagement = null;
            try {
                StringBuilder json = new StringBuilder("\"IPAddressPoolId\":\"" + this.IPAddressPoolId + "\"");
                ODLVSEMETW.EventWriteDoCmdlet(this.CmdletName,
                    "Removing IP Address Pool.",
                    json.ToString());
                string connectionString =
                this.conn.ConnectionString.Split(',').FirstOrDefault();
                vSEMIPAddressPoolManagement =
                    new VSEMIPAddressPoolManagement(connectionString,
                this.conn.Credential);
                if (!vSEMIPAddressPoolManagement.RemoveIpAddressPool(
                                   txnMng,
                                   this.IPAddressPoolId,
                                   this.conn)) {
                    ODLVSEMETW.EventWriteRemoveIPPoolWarning(this.CmdletName,
                    string.Format(CultureInfo.CurrentCulture,
                    "IP Address Pool '{0}' not found.",
                    this.IPAddressPoolId.ToString("B")));

                    // if IP address pool was not found in Remove-Odl.VSEMRemoveIpAddressPool,
                    // ignore it
                }

                ODLVSEMETW.EventWriteReturnLibrary("VSEMIpAddressPool is removed.",
                    string.Empty);
                ope = TransactionManager.Operation.Commit;
            } catch (Exception ex) {
                Exception exception = VSEMODLExceptionUtil.ConvertExceptionToVSEMException(ex);
                ODLVSEMETW.EventWriteFailedCmdlet(this.CmdletName, exception.GetType() + " : " + ex.Message);
                ope = TransactionManager.Operation.Rollback;
                throw exception;
            } finally {
                txnMng.EndTransaction(ope);
                ODLVSEMETW.EventWriteEndCmdlet(this.CmdletName, string.Empty);
            }
        }
    }
}
