//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//
//     This program and the accompanying materials are made available under the
//     terms of the Eclipse Public License v1.0 which accompanies this
//     distribution, and is available at http://www.eclipse.org/legal/epl-v10.html

using System.Collections.Generic;
using System.Text.RegularExpressions;
using ODL.VSEMProvider.Libraries.Entity;

namespace ODL.VSEMProvider.Libraries.Common {
    /// <summary>
    /// Custom comparer class for VSEMVLANIDMapping type.
    /// </summary>
    public class PartialMatchComparer : IComparer<VSEMVLANIDMapping> {
        /// <summary>
        /// Initializes the name as searching criteria.
        /// </summary>
        /// <param name="name">Name, searching crietria.</param>
        public PartialMatchComparer(string name) {
            this.inputVmNetworkName = name;
        }

        /// <summary>
        /// Name, searching crietria.
        /// </summary>
        private string inputVmNetworkName;

        /// <summary>
        /// Compares two objects and returns a value indicating whether one is less than,
        /// equal to, or greater than the other.
        /// </summary>
        /// <param name="x"> The first object to compare.</param>
        /// <param name="y"> The second object to compare.</param>
        /// <returns> A signed integer that indicates the relative values of x and y,
        /// as shown in the following table.
        /// Value Meaning Less than zerox is less than y.Zerox equals
        /// y.Greater than zerox is greater than y.</returns>
        public int Compare(VSEMVLANIDMapping x, VSEMVLANIDMapping y) {
            return Regex.Match(this.inputVmNetworkName,
                "^" + y.VMNetworkName.Replace("*", string.Empty)).Length.CompareTo(
                Regex.Match(this.inputVmNetworkName,
                "^" + x.VMNetworkName.Replace("*", string.Empty)).Length);
        }
    }
}
