//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//
//     This program and the accompanying materials are made available under the
//     terms of the Eclipse Public License v1.0 which accompanies this
//     distribution, and is available at http://www.eclipse.org/legal/epl-v10.html


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
    /// This class represents a cmdlet.
    /// This cmdlet adds a Static IP Address Pool to an existing VM Subnet.
    /// </summary>
    [Cmdlet("Add", "Odl.VSEMIPAddressPool")]
    [System.Management.Instrumentation.ManagedName("Microsoft.SystemCenter.NetworkService.AddIPAddressPool")]
    [OutputType(typeof(IPAddressPool))]
    public class AddVSEMIPAddressPool : VSEMODLCmdletBase {
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
        /// ID of VM Subnet that adds this static IP address pool.
        /// </summary>
        private Guid vMSubnetId;

        /// <summary>
        /// ID of VM Subnet that adds this static IP address pool.
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
        /// Name of the static IP address pool.
        /// </summary>
        private string name;

        /// <summary>
        /// Name of the static IP address pool.
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
                || this.VMSubnetId == null
                || this.VMSubnetId == Guid.Empty
                || string.IsNullOrWhiteSpace(this.Name)
                || string.IsNullOrWhiteSpace(this.IPAddressSubnet)
                || string.IsNullOrWhiteSpace(this.AddressRangeStart)
                || string.IsNullOrWhiteSpace(this.AddressRangeEnd)) {
                ODLVSEMETW.EventWriteValidateCmdletParameter(this.CmdletName,
                    "Mandatory parameter(s) not provided.");
                throw new NSPluginArgumentException("Mandatory parameter(s) not provided.");
            }
            this.conn = VSEMODLCmdletUtility.ValidateConnectionObject(this.Connection, this.CmdletName);
        }

        /// <summary>
        /// This function is responsible for creating IP address pool.
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
            json.Append("\"VMSubnetId\":\"" + this.VMSubnetId + "\"");
            ODLVSEMETW.EventWriteDoCmdlet(this.CmdletName,
                    "Creating IP Address Pool.",
                    json.ToString());
            TransactionManager txnMng = new TransactionManager();
            txnMng.StartTransaction();
            var ope = TransactionManager.Operation.None;
            IPAddressPool ipPool = null;
            VSEMIPAddressPoolManagement vSEMIPAddressPoolManagement = null;
            try {
                string connectionString =
                this.conn.ConnectionString.Split(',').FirstOrDefault();
                vSEMIPAddressPoolManagement =
                    new VSEMIPAddressPoolManagement(connectionString,
                this.conn.Credential);
                ipPool = vSEMIPAddressPoolManagement.CreateIpAddressPool(
                    txnMng,
                    this.Name,
                    this.Description,
                    this.IPAddressSubnet,
                    this.AddressRangeStart,
                    this.AddressRangeEnd,
                    this.NetworkGatewayInfo,
                    this.VMSubnetId);

                ODLVSEMETW.EventWriteReturnLibrary("IP address pool is created.",
                    string.Empty);

                ope = TransactionManager.Operation.Commit;
            } catch (Exception ex) {
                Exception exception = VSEMODLExceptionUtil.ConvertExceptionToVSEMException(ex);
                ODLVSEMETW.EventWriteFailedCmdlet(this.CmdletName, exception.GetType() + " : " + ex.Message);
                ope = TransactionManager.Operation.Rollback;
                throw exception;
            } finally {
                txnMng.EndTransaction(ope);
                string output = "\"IPAddressPool\":" + JavaScriptSerializer.Serialize(ipPool);
                ODLVSEMETW.EventWriteEndCmdlet(this.CmdletName, output);
                this.WriteObject(ipPool);
            }
        }
    }
}
