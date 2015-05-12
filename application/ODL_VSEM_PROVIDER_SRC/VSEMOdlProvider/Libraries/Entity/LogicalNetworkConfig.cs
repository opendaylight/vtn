//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Runtime.Serialization;
using Microsoft.SystemCenter.NetworkService;
using ODL.VSEMProvider.Libraries.Common;

namespace ODL.VSEMProvider.Libraries.Entity {
    /// <summary>
    /// LogicalNetworkConfig class.
    /// This class has the function to manage the LogicalNetwork.config file.
    /// </summary>
    [Serializable, DataContract]
    public class LogicalNetworkConfig : ConfigManagerBase {
        /// <summary>
        /// Config file name.
        /// </summary>
        private const string CONFIG_FILE_NAME = "LogicalNetwork.config";

        /// <summary>
        /// Entity(List of VSEMLogicalNetworkDefinition).
        /// </summary>
        [DataMember]
        public List<LogicalNetwork> LogicalNetworks {
            get;
            set;
        }

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="parentFolder">Name of VTNCoordinator host name as parent folder.</param>
        public LogicalNetworkConfig(string parentFolder) {
            // Initialize.
            this.SetPath(string.Format(CultureInfo.CurrentCulture,
                @"{0}\{1}",
                parentFolder,
                LogicalNetworkConfig.CONFIG_FILE_NAME));
        }

        /// <summary>
        /// Get logical network with the specified ID.
        /// </summary>
        /// <param name="logicalNwId">Logical network ID to find.</param>
        /// <returns>Logical network if found, else null.</returns>
        public LogicalNetwork GetLogicalNetworkById(Guid logicalNwId) {
            if (this.LogicalNetworks != null) {
                var logNw = this.LogicalNetworks.FirstOrDefault(nw => nw.Id == logicalNwId);
                return logNw;
            } else {
                return null;
            }
        }

        /// <summary>
        /// Replace logical network.
        /// </summary>
        /// <param name="src">Logical network instance.</param>
        /// <returns>List index if replace successfully, else -1.</returns>
        public int ReplaceLogicalNetwork(LogicalNetwork src) {
            if (this.LogicalNetworks != null) {
                int i = this.LogicalNetworks.FindIndex(nw => nw.Id == src.Id);
                if (i >= 0) {
                    this.LogicalNetworks[i] = src;
                }
                return i;
            } else {
                return -1;
            }
        }

        /// <summary>
        /// Store the config info to field member(Entity).
        /// </summary>
        /// <param name="config">ConfigManager object.</param>
        protected override void SetEntity(ConfigManagerBase config) {
            if (config == null) {
                this.LogicalNetworks = new List<LogicalNetwork>();
            } else {
                this.LogicalNetworks = ((LogicalNetworkConfig)config).LogicalNetworks;
            }
        }
    }
}
