//     Copyright (c) 2013-2014 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html

using System;
using System.Globalization;
using System.Linq;
using System.Management.Automation;
using System.Reflection;
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
    /// This cmdlet is used to update the static IP address pool associated with VMNetwork. 
    /// </summary>
    [Cmdlet(VerbsCommon.Set, "Odl.VSEMIPAddressPool")]
    [OutputType(typeof(IPAddressPool))]
    [System.Management.Instrumentation.ManagedName("Microsoft.SystemCenter.NetworkService.SetIPAddressPool")]
    public sealed class SetVSEMIPAddressPool : VSEMODLCmdletBase {
        /// <summary>
        /// Static IP address pool ID.
        /// </summary>
        private Guid ipAddressPoolId;

        /// <summary>
        /// Static IP address pool ID.
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
        /// Name of the static IP address pool.
        /// </summary>        
        private string name;

        /// <summary>
        /// Name of the static IP address pool.
        /// </summary>        
        [Parameter(Mandatory = false)]
        public string Name {
            get {
                return this.name;
            }

            set {
                this.name = value;
            }
        }

        /// <summary>
        /// Description.
        /// </summary>
        private string description;

        /// <summary>
        /// Description.
        /// </summary>
        [Parameter(Mandatory = false)]
        public string Description {
            get {
                return this.description;
            }

            set {
                this.description = value;
            }
        }

        /// <summary>
        /// IP address VM subnetwork.
        /// </summary>
        private string ipAddressSubnet;

        /// <summary>
        /// IP address VM subnetwork.
        /// </summary>
        [Parameter(ValueFromPipeline = true)]
        public string IPAddressSubnet {
            get {
                return this.ipAddressSubnet;
            }

            set {
                this.ipAddressSubnet = value;
            }
        }

        /// <summary>
        /// Start value of static IP address pool.
        /// </summary>
        private string addressRangeStart;

        /// <summary>
        /// Start value of static IP address pool.
        /// </summary>
        [Parameter(ValueFromPipeline = true)]
        public string AddressRangeStart {
            get {
                return this.addressRangeStart;
            }

            set {
                this.addressRangeStart = value;
            }
        }

        /// <summary>
        /// End value of static IP address pool.
        /// </summary>
        private string addressRangeEnd;

        /// <summary>
        /// End value of static IP address pool.
        /// </summary>
        [Parameter(ValueFromPipeline = true)]
        public string AddressRangeEnd {
            get {
                return this.addressRangeEnd;
            }

            set {
                this.addressRangeEnd = value;
            }
        }

        /// <summary>
        /// Network gateway information.
        /// </summary>
        private NetworkGatewayInfo[] networkGatewayInfo;

        /// <summary>
        /// Network gateway information.
        /// </summary>
        [Parameter(Mandatory = false)]
        public NetworkGatewayInfo[] NetworkGatewayInfo {
            get {
                return this.networkGatewayInfo;
            }

            set {
                this.networkGatewayInfo = value;
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
        /// This function is responsible for updating the IP address pool.
        /// </summary>
        protected override void DoODLVSEMCmdlet() {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder(" \"NetworkGatewayInfo\":" + JavaScriptSerializer.Serialize(this.NetworkGatewayInfo));
            json.Append("\"Name\":\"" + this.Name + "\"");
            json.Append("\"Description\":\"" + this.Description + "\"");
            json.Append("\"IPAddressSubnet\":\"" + this.IPAddressSubnet + "\"");
            json.Append("\"AddressRangeStart\":\"" + this.AddressRangeStart + "\"");
            json.Append("\"AddressRangeEnd\":\"" + this.AddressRangeEnd + "\"");
            json.Append("\"IPAddressPoolId\":\"" + this.IPAddressPoolId + "\"");
            ODLVSEMETW.EventWriteDoCmdlet(MethodBase.GetCurrentMethod().Name,
                    "Updating IP Address Pool.",
                    string.Empty);
            TransactionManager txnMng = new TransactionManager();
            txnMng.StartTransaction();
            var ope = TransactionManager.Operation.None;
            VSEMIPAddressPoolManagement vSEMIPAddressPoolManagement = null;
            try {
                string connectionString =
                this.conn.ConnectionString.Split(',').FirstOrDefault();
                vSEMIPAddressPoolManagement =
                    new VSEMIPAddressPoolManagement(connectionString,
                this.conn.Credential);
                IPAddressPool pool = vSEMIPAddressPoolManagement.UpdateIpAddressPool(txnMng,
                    this.Name,
                    this.Description,
                    this.IPAddressSubnet,
                    this.AddressRangeStart,
                    this.AddressRangeEnd,
                    this.NetworkGatewayInfo,
                    this.conn,
                    this.IPAddressPoolId);
                ODLVSEMETW.EventWriteReturnLibrary(string.Format(CultureInfo.CurrentCulture,
                    "VSEM IP address pool with ID: {0} is updated",
                    this.IPAddressPoolId.ToString("B")),
                    string.Empty);
                string output = "\"pool\":" + JavaScriptSerializer.Serialize(pool);
                ODLVSEMETW.EventWriteEndCmdlet(this.CmdletName, output);
                this.WriteObject(pool);

                ope = TransactionManager.Operation.Commit;
            } catch (Exception ex) {
                Exception exception = VSEMODLExceptionUtil.ConvertExceptionToVSEMException(ex);
                ODLVSEMETW.EventWriteFailedCmdlet(this.CmdletName, exception.GetType() + " : " + ex.Message);
                ope = TransactionManager.Operation.Rollback;
                ODLVSEMETW.EventWriteEndCmdlet(this.CmdletName, string.Empty);
                throw exception;
            } finally {
                txnMng.EndTransaction(ope);
            }
        }
    }
}