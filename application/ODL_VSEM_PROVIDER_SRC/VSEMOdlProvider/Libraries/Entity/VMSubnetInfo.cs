//     Copyright (c) 2013-2014 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html


using System;
using System.Runtime.Serialization;

namespace ODL.VSEMProvider.Libraries.Entity {
    /// <summary>
    /// This class is responsible for the storing the information of VM Sub-Network. 
    /// </summary>
    [Serializable, DataContract]
    public class VMSubnetInfo {
        /// <summary>
        /// This parameter specifies  the VM Sub-Network name.
        /// </summary>
        [DataMember]
        public string VMSubnetName {
            get;
            set;
        }

        /// <summary>
        /// This parameter specifies  the vBridge name.
        /// </summary>
        [DataMember]
        public string VBridgeName {
            get;
            set;
        }

        /// <summary>
        /// This parameter contains the VLAN ID fo VM Subnet.
        /// </summary>
        [DataMember]
        public long VMSubnetVlanId {
            get;
            set;
        }

        /// <summary>
        /// This parameter contains the VLAN ID of VBridge.
        /// </summary>
        [DataMember]
        public long VBridgeVlanId {
            get;
            set;
        }

        /// <summary>
        /// This parameter contains the unique VM Sub-Network ID.
        /// </summary>
        [DataMember]
        public Guid VMSubnetID {
            get;
            set;
        }

        /// <summary>
        /// This parameter specifies the type of server on which this VM subnet is created originally.
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
