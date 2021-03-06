// This file is part of Lattice Builder.
//
// Copyright (C) 2012-2016  Pierre L'Ecuyer and Universite de Montreal
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef LATBUILDER__SIZE_PARAM_EMBEDDED_H
#define LATBUILDER__SIZE_PARAM_EMBEDDED_H

#include "latbuilder/SizeParam.h"

namespace LatBuilder {

/**
 * Lattice size type for embedded lattices.
 * Defining parameters:
 * - prime base for the number of points;
 * - maximum embedding level (power of the base for the last lattice in the sequence).
 */
template <>
class SizeParam<LatType::EMBEDDED> :
   public BasicSizeParam<SizeParam<LatType::EMBEDDED>> {
public:

   /**
    * Constructor.
    *
    * \param primeBase     Prime base for the number of points.
    * \param maxLevel      The last lattice in the sequence (the one with the
    *                      largest number of points) has
    *                      \f$\mathtt{primeBase}^{\mathtt{maxLevel}}\f$ points.
    */
   SizeParam(Modulus primeBase, Level maxLevel);

   /**
    * Constructor.
    *
    * \param numPoints  Number of points factorizable as an integer power of
    *                   a prime base.
    */
   SizeParam(Modulus numPoints = 0);

   template <class D>
   SizeParam(const BasicSizeParam<D>& other): SizeParam(other.numPoints())
   {}

   /**
    * Returns the prime base for the number of points.
    */
   Modulus base() const
   { return m_base; }

   /**
    * Returns the maximum embedding level of the base for the last lattice in the sequence.
    */
   Level maxLevel() const
   { return m_maxLevel; }

   /**
    * Returns the number of points for the lattice at embedding level \c level
    * in the sequence.
    */
   Modulus numPointsOnLevel(Level level) const;

   size_t totient() const;

   void normalize(Real& merit) const;

   /**
    * Divides the merit value at each level in \c merit by the number of
    * points at that level.
    */
   void normalize(RealVector& merit) const;

   std::ostream& format(std::ostream& os) const;

private:
   Modulus m_base;
   Level m_maxLevel;
};

}

#endif
