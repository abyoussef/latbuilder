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

#include "latbuilder/MeritSeq/ConcreteCoordUniformState-PD.h"

namespace LatBuilder { namespace MeritSeq {

//========================================================================
// ProjectionDependentWeights
//========================================================================

template <LatType LAT, Compress COMPRESS>
void
ConcreteCoordUniformState<LAT, COMPRESS, LatCommon::ProjectionDependentWeights>::
reset()
{
   CoordUniformState<LAT, COMPRESS>::reset();
   m_state.clear();
   // empty set
   m_state[LatCommon::Coordinates()] =
      boost::numeric::ublas::scalar_vector<Real>(this->storage().size(), 1.0);
   m_gen.clear();
}

//===========================================================================

template <LatType LAT, Compress COMPRESS>
const RealVector&
ConcreteCoordUniformState<LAT, COMPRESS, LatCommon::ProjectionDependentWeights>::
createStateVector(const LatCommon::Coordinates& projection, const RealVector& kernelValues)
{
   auto it = m_state.find(projection);
   if (it != m_state.end())
      return it->second;

   // select largest index
   const LatCommon::Coordinates::value_type largestCoord = *projection.rbegin();

   // remove largest coordinate index
   LatCommon::Coordinates baseProjection = projection;
   baseProjection.erase(largestCoord);
   // create base state vector
   const RealVector& baseState = createStateVector(baseProjection, kernelValues);

   // compute merit value for new projection
   auto stridedKernelValues = this->storage().strided(
         kernelValues,
         m_gen[largestCoord]
         );
   return m_state[projection] = boost::numeric::ublas::element_prod(
         stridedKernelValues,
         baseState
         );
}

//===========================================================================

template <LatType LAT, Compress COMPRESS>
void
ConcreteCoordUniformState<LAT, COMPRESS, LatCommon::ProjectionDependentWeights>::
update(const RealVector& kernelValues, Modulus gen)
{
   CoordUniformState<LAT, COMPRESS>::update(kernelValues, gen);
   m_gen.push_back(gen);

   // Create a new state vector for each projection $\mathfrak u$ such that
   // $\max \mathfrak u$ < $j$ and that $\mathfrak u \cup \{j\}$ has a nonzero
   // weight.
   const auto nextCoordinate = this->dimension();
   for (const auto& pw : m_weights.getWeightsForLargestIndex(nextCoordinate)) {
      // remove largest coordinate index
      LatCommon::Coordinates proj = pw.first;
      proj.erase(*proj.rbegin());
      // create state vector
      createStateVector(proj, kernelValues);
   }
}

//===========================================================================

template <LatType LAT, Compress COMPRESS>
RealVector
ConcreteCoordUniformState<LAT, COMPRESS, LatCommon::ProjectionDependentWeights>::
weightedState() const
{
   using LatCommon::Coordinates;

   const auto nextCoordinate = this->dimension();

   RealVector weightedState =
      boost::numeric::ublas::scalar_vector<Real>(this->storage().size(), 0.0);

   for (const auto& pw : m_weights.getWeightsForLargestIndex(nextCoordinate)) {
      // remove largest coordinate index
      LatCommon::Coordinates proj = pw.first;
      proj.erase(*proj.rbegin());
      // find state vector
      auto it = m_state.find(proj);
      if (it == m_state.end())
         throw std::runtime_error("projection-dependent state was not created");
      // contribute to weighted state
      weightedState += pw.second * it->second;
   }

   return weightedState;
}

//===========================================================================

template class ConcreteCoordUniformState<LatType::ORDINARY, Compress::NONE,      LatCommon::ProjectionDependentWeights>;
template class ConcreteCoordUniformState<LatType::ORDINARY, Compress::SYMMETRIC, LatCommon::ProjectionDependentWeights>;
template class ConcreteCoordUniformState<LatType::EMBEDDED, Compress::NONE,      LatCommon::ProjectionDependentWeights>;
template class ConcreteCoordUniformState<LatType::EMBEDDED, Compress::SYMMETRIC, LatCommon::ProjectionDependentWeights>;

}}
