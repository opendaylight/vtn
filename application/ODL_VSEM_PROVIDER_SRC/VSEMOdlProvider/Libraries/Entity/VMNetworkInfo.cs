//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//
//     This program and the accompanying materials are made available under the
//     terms of the Eclipse Public License v1.0 which accompanies this
//     distribution, and is available at http://www.eclipse.org/legal/epl-v10.html

using System;
using System.Collections.Generic;
using System.Runtime.Serialization;

namespace ODL.VSEMProvider.Libraries.Entity {
    /// <summary>
    /// This class is responsible for the storing the information of VM Network.
    /// </summary>
    [Serializable, DataContract]
    public class VMNetworkInfo {
        /// <summary>
        /// This parameter specifies  the VM Network name.
        /// </summary>
        [DataMember]
        public string VMNetworkName {
            get;
            set;
        }

        /// <summary>
        /// This parameter specifies  the VTN name .
        /// </summary>
        [DataMember]
        public string VTNName {
            get;
            set;
        }

        /// <summary>
        /// This parameter is an object of VMSubNetworkInfo
        /// which contains the information of VM Sub-network.
        /// </summary>
        [DataMember]
        public List<VMSubnetInfo> VMSubnetInfo {
            get;
            set;
        }

        /// <summary>
        /// This parameter specifies the VM network ID generated.
        /// </summary>
        [DataMember]
        public Guid VMNetworkID {
            get;
            set;
        }

        /// <summary>
        /// This parameter specifies the original name of VM Network.
        /// </summary>
        [DataMember]
        public string VMNetworkOriginalName {
            get;
            set;
        }

        /// <summary>
        /// This parameter specifies the type of server on which this VM network is created originally.
        /// </summary>
        [DataMember]
        public string CreatedFrom {
            get;
            set;
        }

        /// <summary>
        /// This parameter specifies the reason for inconsistency.
        /// </summary>
        [DataMember]
        public string Description {
            get;
            set;
        }
    }
}
