//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//
//     This program and the accompanying materials are made available under the
//     terms of the Eclipse Public License v1.0 which accompanies this
//     distribution, and is available at http://www.eclipse.org/legal/epl-v10.html

using System;
using System.Runtime.Serialization;

namespace ODL.VSEMProvider.Libraries.Entity {
    /// <summary>
    /// This class describes the information about the ODL which is managed by Virtual Switch Extension Manager.
    /// </summary>
    [Serializable, DataContract]
    public class VSEMInfo {
        /// <summary>
        /// VTNCoordinatoor Host name.
        /// </summary>
        [DataMember]
        public string ServerName { get; set; }

        /// <summary>
        /// ODL port no.
        /// </summary>
        [DataMember]
        public long Port { get; set; }

        /// <summary>
        /// Minimum version of VTNCoordinator allowed for connection with VSEMProvider.
        /// </summary>
        [DataMember]
        public Version MinVTNCoVersion { get; set; }

        /// <summary>
        /// Time and Date of the last modification.
        /// </summary>
        [DataMember]
        public DateTime LastModifiedTimeStamp { get; set; }
    }
}
