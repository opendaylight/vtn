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
    /// This class is an entity class to store the information of VLANIDMapping.config file.
    /// </summary>
    [Serializable, DataContract]
    public class VSEMVLANIDMapping {
        /// <summary>
        /// Name of VM network to associate.
        /// </summary>
        [DataMember]
        public string VMNetworkName { get; set; }

        /// <summary>
        /// Name of VM sub network to associate.
        /// </summary>
        [DataMember]
        public string VMSubNetworkName { get; set; }

        /// <summary>
        /// VLAN ID.
        /// </summary>
        [DataMember]
        public string VlanId { get; set; }

        /// <summary>
        /// Date and time of the last modification.
        /// </summary>
        [DataMember]
        public DateTime LastModifiedTimeStamp { get; set; }
    }
}
