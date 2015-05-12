//     Copyright (c) 2013-2014 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html


using System.Collections.Generic;

namespace ODL.VSEMProvider.Libraries.Common {
    /// <summary>
    /// Custom comparer class for Range type.
    /// </summary>
    public class CustomSortRangeComparer : IComparer<Range> {
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
        public int Compare(Range x, Range y) {
            return x.Start.CompareTo(y.Start);
        }
    }
}
