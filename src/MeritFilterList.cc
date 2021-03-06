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

#include "latbuilder/MeritFilterList.h"

namespace LatBuilder {

namespace {
   void setValue(Real& merit, Real value)
   { merit = value; }
   void setValue(RealVector& merit, Real value)
   { merit = boost::numeric::ublas::scalar_vector<Real>(merit.size(), value); }
}

template <LatType LAT>
auto BasicMeritFilterList<LAT>::apply(
      MeritValue merit,
      const LatDef& lat
      ) const -> MeritValue
{
   try {
      for (const auto& filter : filters())
         merit = (*filter)(merit, lat);
   }
   catch (LatticeRejectedException&) {
      onReject()(lat);
      setValue(merit, std::numeric_limits<Real>::infinity());
   }
   return merit;
}

template class BasicMeritFilterList<LatType::ORDINARY>;
template class BasicMeritFilterList<LatType::EMBEDDED>;


Real MeritFilterListPolicy<LatType::ORDINARY>::applyFilters(Real merit, const LatDef& lat) const
{ return OBase::apply(merit, lat); }

Real MeritFilterListPolicy<LatType::EMBEDDED>::applyFilters(const RealVector& merit, const typename EBase::LatDef& lat) const
{ return OBase::apply(combiner()(EBase::apply(merit, lat), lat), lat); }

namespace {
   template <typename LIST>
   void filtersFormatHelper(std::ostream& os, const LIST& list, const std::string& label)
   {
      os << label << " = (";
      auto it = list.begin();
      if (it != list.end())
         os << (*it)->name();
      while (++it != list.end())
         os << ", " << (*it)->name();
      os << ")";
   }
}

std::ostream& operator<<(std::ostream& os, const BasicMeritFilterList<LatType::ORDINARY>& filters)
{ filtersFormatHelper(os, filters.filters(), "scalar filters"); return os; }

std::ostream& operator<<(std::ostream& os, const BasicMeritFilterList<LatType::EMBEDDED>& filters)
{ filtersFormatHelper(os, filters.filters(), "multilevel filters"); return os; }

std::ostream& operator<<(std::ostream& os, const MeritFilterList<LatType::ORDINARY>& filters)
{ return os << dynamic_cast<const BasicMeritFilterList<LatType::ORDINARY>&>(filters); return os; }

std::ostream& operator<<(std::ostream& os, const MeritFilterList<LatType::EMBEDDED>& filters)
{
   os << dynamic_cast<const BasicMeritFilterList<LatType::ORDINARY>&>(filters);
   os << ", ";
   os << "combiner=" << filters.combiner().name();
   os << ", ";
   os << dynamic_cast<const BasicMeritFilterList<LatType::EMBEDDED>&>(filters);
   return os;
}

}
