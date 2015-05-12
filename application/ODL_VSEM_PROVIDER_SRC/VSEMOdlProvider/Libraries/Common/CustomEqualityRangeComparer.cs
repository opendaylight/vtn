//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html

using System.Collections.Generic;

namespace ODL.VSEMProvider.Libraries.Common {
    /// <summary>
    /// Custom equality compararer for range.
    /// </summary>
    public class CustomEqualityRangeComparer : IEqualityComparer<Range> {
        /// <summary>
        /// Overrides Equals method of IEqualityComparer.
        /// </summary>
        /// <param name="x">The first object of type Range to compare.</param>
        /// <param name="y">The second object of type Range to compare.</param>
        /// <returns>true if both object are equal.</returns>
        public bool Equals(Range x, Range y) {
            if (x.Start.CompareTo(y.Start) == 0 && x.End.CompareTo(y.End) == 0) {
                return true;
            } else {
                return false;
            }
        }

        /// <summary>
        /// Overrides GetHashCode method of IEqualityComparer.
        /// </summary>
        /// <param name="bx">Range object.</param>
        /// <returns>A hash code for the specified object.</returns>
        public int GetHashCode(Range bx) {
            int code = bx.Start ^ bx.End;
            return code.GetHashCode();
        }
    }
}
