//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html


using System;
using System.Collections.Generic;
using System.Runtime.Serialization;

namespace ODL.VSEMProvider.Libraries.Entity {
    /// <summary>
    /// This class is responsible for the mapping of VM network mapping information.
    /// </summary>
    [Serializable, DataContract]
    public class VMNetworkMappingInfo {
        /// <summary>
        /// This parameter specifies VM network mapping information.
        /// </summary>
        [DataMember]
        public List<VMNetworkInfo> VMNetworkInfo {
            get;
            set;
        }

        /// <summary>
        /// This parameter specifies  the time this mapping of VM network  was last modified.
        /// </summary>
        [DataMember]
        public DateTime LastModifiedTimeStamp {
            get;
            set;
        }
    }
}
