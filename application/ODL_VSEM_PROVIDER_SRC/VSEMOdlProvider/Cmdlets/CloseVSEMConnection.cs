//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html

using System.Globalization;
using System.Management.Automation;
using System.Management.Instrumentation;
using System.Text;
using System.Web.Script.Serialization;
using Microsoft.SystemCenter.NetworkService;
using ODL.VSEMProvider.Cmdlets.Common;
using ODL.VSEMProvider.Libraries;
using ODL.VSEMProvider.VSEMEvents;
using VSEM.Cmdlets.Common;

namespace ODL.VSEMProvider.Cmdlets {
    /// <summary>
    /// This class represents a cmdlet. This cmdlet is used to close the connection opened via 
    /// OpenVSEMConnection call.
    /// </summary>
    [Cmdlet(VerbsCommon.Close, "Odl.VSEMConnection")]
    [ManagedName("Microsoft.SystemCenter.NetworkService.CloseDeviceConnection")]
    public sealed class CloseVSEMConnection : VSEMODLCmdletBase {
        /// <summary>
        /// The Connection object that needs to be closed. This parameter is Mandatory.
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
        /// This function is responsible for validating the parameters.
        /// </summary>
        protected override void BeginODLVSEMCmdlet() {
            this.conn = VSEMODLCmdletUtility.ValidateConnectionObject(this.Connection, this.CmdletName);
        }

        /// <summary>
        /// This function is responsible for closing the VSEM connection.
        /// </summary>
        protected override void DoODLVSEMCmdlet() {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"ConnectionString\":" + JavaScriptSerializer.Serialize(this.conn.ConnectionString));
            ODLVSEMETW.EventWriteDoCmdlet(this.CmdletName,
                string.Format(CultureInfo.CurrentCulture,
                "Closing connection to {0}.",
                this.conn.ConnectionString),
                json.ToString());
            string connectionString = this.conn.ConnectionString;
            this.conn.CloseConnection();
            ODLVSEMETW.EventWriteCloseConnectionInformation(this.CmdletName,
                string.Format(CultureInfo.CurrentCulture,
                "Connection to {0} closed.",
                connectionString));
            ODLVSEMETW.EventWriteEndCmdlet(this.CmdletName, string.Empty);
        }
    }
}
