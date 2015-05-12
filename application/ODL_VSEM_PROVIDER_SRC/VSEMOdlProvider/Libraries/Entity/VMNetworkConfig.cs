//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Runtime.Serialization;
using Microsoft.SystemCenter.NetworkService;
using ODL.VSEMProvider.Libraries.Common;

namespace ODL.VSEMProvider.Libraries.Entity {
    /// <summary>
    /// VMNetworkConfig class.
    /// This class has the function to manage the VMNetwork.config file.
    /// </summary>
    [Serializable, DataContract]
    public class VMNetworkConfig : ConfigManagerBase {
        /// <summary>
        /// Config file name.
        /// </summary>
        private const string CONFIG_FILE_NAME = "VMNetwork.config";

        /// <summary>
        /// Entity(VSEMVMNetwork).
        /// </summary>
        [DataMember]
        public VSEMVMNetwork VMNetwork {
            get;
            set;
        }

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="parentFolder">Name of ODL host name as parent folder.</param>
        public VMNetworkConfig(string parentFolder) {
            // Initialize.
            this.SetPath(string.Format(CultureInfo.CurrentCulture,
                @"{0}\{1}",
                parentFolder,
                VMNetworkConfig.CONFIG_FILE_NAME));
        }

        /// <summary>
        /// Store the config info to field member(Entity).
        /// </summary>
        /// <param name="config">ConfigManager object.</param>
        protected override void SetEntity(ConfigManagerBase config) {
            if (config == null) {
                this.VMNetwork = new VSEMVMNetwork();
                this.VMNetwork.VmNetworks = new List<VMNetwork>();
                this.VMNetwork.VMNetworkMappingInformation = new VMNetworkMappingInfo();
                this.VMNetwork.VMNetworkMappingInformation.VMNetworkInfo = new List<VMNetworkInfo>();
            } else {
                this.VMNetwork = ((VMNetworkConfig)config).VMNetwork;
            }
        }
    }
}
