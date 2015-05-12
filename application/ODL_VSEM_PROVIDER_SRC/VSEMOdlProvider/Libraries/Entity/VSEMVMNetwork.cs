//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html

using System;
using System.Collections.Generic;
using System.Runtime.Serialization;
using Microsoft.SystemCenter.NetworkService;

namespace ODL.VSEMProvider.Libraries.Entity {
    /// <summary>
    /// This class is an entity class to store the information of VMNetwork.config file.
    /// </summary>
    [Serializable, DataContract]
    public class VSEMVMNetwork {
        /// <summary>
        /// Name of VM network to associate.
        /// </summary>
        [DataMember]
        public VMNetworkMappingInfo VMNetworkMappingInformation {
            get;
            set;
        }

        /// <summary>
        /// Name of VM sub network to associate.
        /// </summary>
        [DataMember]
        public List<VMNetwork> VmNetworks {
            get;
            set;
        }
    }
}
