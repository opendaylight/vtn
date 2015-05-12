//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html


using System;
using System.Linq;
using System.Reflection;
using Microsoft.SystemCenter.NetworkService;
using ODL.VSEMProvider.Libraries.Common;
using ODL.VSEMProvider.Libraries.Entity;
using ODL.VSEMProvider.CTRLibraries;
using ODL.VSEMProvider.VSEMEvents;

namespace ODL.VSEMProvider.Libraries {
    /// <summary>
    /// VMNetworkConfig class for HNV.
    /// This class has the function to manage the VMNetwork.config file.
    /// </summary>
    public class HNVVMNetworkManagement {
        /// <summary>
        /// VTNCoordinator HostName.
        /// </summary>
        private string VtnHostName { get; set; }

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="VtnHostName">Name of VTNCoordinator host name.</param>
        public HNVVMNetworkManagement(string VtnHostName) {
            // Verify arguments
            if (string.IsNullOrEmpty(VtnHostName)) {
                throw new ArgumentNullException("The parameter 'VTNCoordinator HostName' is invalid.");
            }

            // Set field.
            this.VtnHostName = VtnHostName;
        }

        /// <summary>
        /// Verify whether the specified VMNetwork is HNV resource or not.
        /// </summary>
        /// <param name="vmnetwork">VMNetwork instance.</param>
        /// <returns>True if HNV resource, else false.</returns>
        public bool IsHNVVMNetwork(VMNetwork vmnetwork) {
            if (vmnetwork == null) {
                return false;
            }

            // Create txn and read configuration.
            var txnMng = new TransactionManager();
            txnMng.StartTransaction();
            var logicalNetworkConfig = new LogicalNetworkConfig(this.VtnHostName);
            txnMng.SetConfigManager(logicalNetworkConfig, TransactionManager.OpenMode.ReadMode);

            bool ret;

            // Get LogicalNetowork that Guid equals vmnetwork.LogicalNetwork.
            LogicalNetwork logicalnetwork = logicalNetworkConfig.GetLogicalNetworkById(vmnetwork.LogicalNetwork);
            if (logicalnetwork == null) {
                ret = false;
                goto txnEnd;
            }

            var hnvLogicalNetworkManagement = new HNVLogicalNetworkManagement(this.VtnHostName);
            ret = hnvLogicalNetworkManagement.IsHNVLogicalNetwork(logicalnetwork);

            if (ret == false) {
                ODLVSEMETW.EventWriteNotHNVVMNetwork(
                    MethodBase.GetCurrentMethod().Name,
                    string.Format("VMNetwork name is {0}", vmnetwork.Name));
            }
txnEnd:
            txnMng.EndTransaction(TransactionManager.Operation.None);
            return ret;
        }
    }
}
