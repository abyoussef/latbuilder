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

#include "latbuilder/CoordUniformFigureOfMerit.h"
#include "latcommon/ProductWeights.h"
#include "latbuilder/Kernel/PAlpha.h"
#include "latbuilder/Accumulator.h"
#include "latbuilder/Functor/binary.h"
#include "latbuilder/Storage.h"

#include "latbuilder/MeritFilterList.h"
#include "latbuilder/MeritCombiner.h"
#include "latbuilder/MeritFilter.h"
#include "latbuilder/Norm/Normalizer.h"
#include "latbuilder/Norm/PAlphaSL10.h"
#include "latbuilder/Functor/LowPass.h"

#include "latbuilder/MeritSeq/CoordUniformCBC.h"
#include "latbuilder/MeritSeq/CoordUniformInnerProd.h"
#include "latbuilder/GenSeq/CoprimeIntegers.h"
#include "latbuilder/GenSeq/Creator.h"

#include "latbuilder/TextStream.h"

#include <iostream>
#include <limits>

using namespace LatBuilder;
using TextStream::operator<<;

template <typename T, typename... ARGS>
std::unique_ptr<T> unique(ARGS&&... args)
{ return std::unique_ptr<T>(new T(std::forward<ARGS>(args)...)); }

template <class NORMALIZER>
void setLevelWeights(NORMALIZER&, const SizeParam<LatType::ORDINARY>&)
{}

template <class NORMALIZER>
void setLevelWeights(NORMALIZER& normalizer, const SizeParam<LatType::EMBEDDED>& sizeParam)
{
   //! [per-level weights]
   normalizer.setWeights(RealVector(
            sizeParam.maxLevel() + 1,
            1.0 / sizeParam.maxLevel()
            ));
   //! [per-level weights]
}

void setCombiner(MeritFilterList<LatType::ORDINARY>&) {}

//! [combiner]
void setCombiner(MeritFilterList<LatType::EMBEDDED>& filters)
{ filters.add(unique<MeritCombiner::Accumulator<Functor::Sum>>()); }
//! [combiner]

template <LatType L, Compress C>
void test(const Storage<L, C>& storage, Dimension dimension)
{
   //! [figure]
   auto weights = unique<LatCommon::ProductWeights>();
   weights->setDefaultWeight(0.7);

   CoordUniformFigureOfMerit<Kernel::PAlpha> figure(std::move(weights), 2);
   std::cout << "figure of merit: " << figure << std::endl;
   //! [figure]

   typedef GenSeq::CoprimeIntegers<decltype(figure)::suggestedCompression()> Coprime;
   auto genSeq  = GenSeq::Creator<Coprime>::create(storage.sizeParam());
   auto genSeq0 = GenSeq::Creator<Coprime>::create(SizeParam<L>(2));

   //! [cbc]
   auto cbc = MeritSeq::cbc<MeritSeq::CoordUniformInnerProd>(storage, figure);
   //! [cbc]

   //! [filters]
   MeritFilterList<L> filters;

   setCombiner(filters);

   //! [normalizer]
   auto normalizer = unique<Norm::Normalizer<L, Norm::PAlphaSL10>>(
         Norm::PAlphaSL10(figure.kernel().alpha(), figure.weights())
         );
   setLevelWeights(*normalizer, storage.sizeParam());
   filters.add(std::move(normalizer));
   //! [normalizer]

   //! [low-pass]
   auto lowPass = unique<MeritFilter<L>>(Functor::LowPass<Real>(1.0), "low-pass");
   filters.add(std::move(lowPass));
   //! [low-pass]
   std::cout << "filters: " << filters << std::endl;
   //! [filters]

   //! [CBC loop]
   while (cbc.baseLat().dimension() < dimension) {

      Dimension baseDim = cbc.baseLat().dimension();

      std::cout << "CBC search for dimension: " << (baseDim + 1) << std::endl;
      std::cout << "  base lattice: " << cbc.baseLat() << std::endl;
      std::cout << "  base merit value: " << cbc.baseMerit() << std::endl;

      //! [meritSeq]
      auto meritSeq = cbc.meritSeq(baseDim == 0 ? genSeq0 : genSeq);
      //! [meritSeq]

      //! [filteredSeq]
      auto filteredSeq = filters.apply(meritSeq);
      //! [filteredSeq]

      //! [min_element]
      auto best = std::min_element(filteredSeq.begin(), filteredSeq.end());
      //! [min_element]
      //! [select]
      cbc.select(best.base());
      //! [select]

      //! [output]
      std::cout << "BEST LATTICE: " << cbc.baseLat() << " with merit value " << *best << std::endl;
      //! [output]
   }
   //! [CBC loop]
}

int main()
{
   Dimension dim = 3;

   //! [Storage]
   test(Storage<LatType::ORDINARY, Compress::SYMMETRIC>(256), dim);
   test(Storage<LatType::EMBEDDED, Compress::SYMMETRIC>(256), dim);
   //! [Storage]

   return 0;
}
