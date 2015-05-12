//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//
//     This program and the accompanying materials are made available under the
//     terms of the Eclipse Public License v1.0 which accompanies this
//     distribution, and is available at http://www.eclipse.org/legal/epl-v10.html


using System;
using System.Management.Automation;
using System.Management.Automation.Runspaces;
using System.Management.Instrumentation;
using Microsoft.SystemCenter.NetworkService;
using ODL.VSEMProvider.Cmdlets.Common;
using ODL.VSEMProvider.Libraries;
using ODL.VSEMProvider.Libraries.Common;
using ODL.VSEMProvider.Libraries.Entity;
using ODL.VSEMProvider.VSEMEvents;

namespace ODL.VSEM.Cmdlets { /// <summary>
    /// This class represents a cmdlet. This cmdlet is used to synchronize the VTN objects between ODL and SCVMM. 
    /// </summary>
    [Cmdlet(VerbsData.Sync, "Odl.VSEMVTNObjects")]
    [OutputType(typeof(VSEMVLANIDMapping))]
    [ManagedName("Microsoft.SystemCenter.NetworkService.SyncVSEMVTNObjects")]
    public sealed class SyncVSEMVTNObjects : VSEMODLCmdletBase {
        /// <summary>
        /// Name of the VM network.
        /// </summary>
        private string scvmSubnets;

        /// <summary>
        /// Name of the VM network.
        /// </summary>
        [Parameter(Mandatory = false)]
        public string SCVMSubnets {
            get {
                return this.scvmSubnets;
            }

            set {
                this.scvmSubnets = value;
            }
        }

        /// <summary>
        /// This method is responsible to add specified vlan entry into the local variable.
        /// </summary>
        protected override void DoODLVSEMCmdlet() {
            ODLVSEMETW.EventWriteDoCmdlet(this.CmdletName,
                "Synchronizing SCVMM and ODL.",
                string.Empty);
            VSEMSynchronization vSEMSynchronization = null;
            if (string.IsNullOrEmpty(this.SCVMSubnets)) {
                TransactionManager txnMng = new TransactionManager();
                txnMng.StartTransaction();
                var operation = TransactionManager.Operation.None;
                try {
                    vSEMSynchronization = new VSEMSynchronization(txnMng);

                    vSEMSynchronization.SynchronizeVTNObjects(txnMng);
                    operation = TransactionManager.Operation.Commit;
                } catch (Exception ex) {
                    Exception exception = VSEMODLExceptionUtil.ConvertExceptionToVSEMException(ex);
                    ODLVSEMETW.EventWriteFailedCmdlet(this.CmdletName, exception.GetType() + " : synchronization is failed. \n" + ex.Message);
                    operation = TransactionManager.Operation.Rollback;
                    ODLVSEMETW.EventWriteEndCmdlet(this.CmdletName, string.Empty);
                    throw exception;
                } finally {
                    txnMng.EndTransaction(operation);
                }
            } else {
                TransactionManager txnMng = new TransactionManager();
                txnMng.StartTransaction();
                var operation = TransactionManager.Operation.Commit;
                try {
                    vSEMSynchronization = new VSEMSynchronization(txnMng);
                    vSEMSynchronization.StatusAfterRefreshNetworkService(txnMng, this.SCVMSubnets);
                } catch (Exception ex) {
                    operation = TransactionManager.Operation.Rollback;
                    if (ex.GetType() != typeof(NSPluginSynchronizationException)) {
                        ODLVSEMETW.EventWriteFailedCmdlet("NetworkService could not be refreshed in SCVMM. " + ex.Message,
                           string.Empty);

                        throw new NSPluginSynchronizationException("NetworkService could not be refreshed in SCVMM. " + ex.Message);
                    } else {
                        throw;
                    }
                } finally {
                    txnMng.EndTransaction(operation);
                    ODLVSEMETW.EventWriteEndCmdlet(this.CmdletName, string.Empty);
                }
            }
        }

        /// <summary>
        /// Method to refresh the network service on SCVMM.
        /// </summary>
        private static void RefreshNetworkService() {
            Runspace runspace = RunspaceFactory.CreateRunspace();
            runspace.Open();
            try {
                Pipeline pipeline = null;
                Command command = null;
                pipeline = runspace.CreatePipeline();
                pipeline.Commands.Clear();
                command = new Command("Get-SCNetworkService");
                pipeline.Commands.Add(command);
                var nwservice = pipeline.Invoke();

                if (nwservice != null && nwservice.Count != 0 && nwservice[0].BaseObject != null) {
                    var service = nwservice[0].BaseObject;
                    pipeline = runspace.CreatePipeline();
                    pipeline.Commands.Clear();
                    command = new Command("Read-SCNetworkService");
                    pipeline.Commands.Add(command);
                    command.Parameters.Add("NetworkService", service);
                    pipeline.Invoke();
                } else {
                    ODLVSEMETW.EventWriteFailedCmdlet("NetworkService is not added in SCVMM.",
                            string.Empty);
                    throw new NSPluginSynchronizationException("NetworkService is not added in SCVMM.");
                }
            } finally {
                runspace.Close();
            }
        }
    }
}
