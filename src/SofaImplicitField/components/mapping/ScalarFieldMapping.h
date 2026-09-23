/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, development version     *
*                (c) 2006-2025 INRIA, USTL, UJF, CNRS, MGH                    *
*                                                                             *
* This program is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This program is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this program. If not, see <http://www.gnu.org/licenses/>.        *
*******************************************************************************
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#pragma once
#include <SofaImplicitField/config.h>
#include <SofaImplicitField/components/geometry/ScalarField.h>
#include <sofa/core/Mapping.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
namespace sofaimplicitfield::mapping
{

namespace{
    using namespace sofa;
    using sofa::core::objectmodel::BaseComponent;
    using sofa::core::visual::VisualParams;
    using sofa::core::ConstraintParams;
    using sofa::type::Vec3d;
    using sofa::type::Vec3u;
    using sofa::component::geometry::ScalarField;
    using sofa::core::Mapping;
    using sofa::core::BaseMapping;
    using sofa::core::MechanicalParams;
    using sofa::core::MultiVecDerivId;
    using sofa::core::ConstMultiVecDerivId;
}

class ScalarFieldMapping : public Mapping<sofa::defaulttype::Vec3Types, sofa::defaulttype::Vec1Types>
{
public:
    SOFA_CLASS(ScalarFieldMapping,
               SOFA_TEMPLATE2(Mapping, sofa::defaulttype::Vec3Types, sofa::defaulttype::Vec1Types));

    void init() override;

    ScalarFieldMapping();
    ~ScalarFieldMapping();

    void apply( const MechanicalParams* mparams, OutDataVecCoord& out, const InDataVecCoord& in) override;
    void applyJ( const MechanicalParams* mparams, OutDataVecDeriv& out, const InDataVecDeriv& in) override;
    void applyJT( const MechanicalParams* mparams, InDataVecDeriv& out, const OutDataVecDeriv& in) override;
    void applyJT( const ConstraintParams* /* mparams */, InDataMatrixDeriv& /* out */, const OutDataMatrixDeriv& /* in */) override;

    void buildGeometricStiffnessMatrix(sofa::core::GeometricStiffnessMatrix* matrices) override;

    SingleLink<ScalarFieldMapping, ScalarField,
               BaseLink::FLAG_STOREPATH|BaseLink::FLAG_STRONGLINK> l_field;
};

}

