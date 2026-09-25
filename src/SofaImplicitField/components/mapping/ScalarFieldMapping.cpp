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
******************************************************************************
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#include <SofaImplicitField/config.h>
#include <SofaImplicitField/components/mapping/ScalarFieldMapping.h>

#include <sofa/core/MappingHelper.h>
#include <sofa/core/BaseLocalMappingMatrix.h>

#include <sofa/core/ObjectFactory.h>
using sofa::core::RegisterObject;

namespace sofaimplicitfield::mapping
{

ScalarFieldMapping::ScalarFieldMapping() :
    l_field(initLink("field", "The scalar field to sample"))
{
}

ScalarFieldMapping::~ScalarFieldMapping()
{
}

void ScalarFieldMapping::init()
{
    if(l_field.get() == nullptr){
        msg_error() << "The field is missing. Cannot work properly without it.";
        d_componentState = core::objectmodel::ComponentState::Invalid;
        return;
    }
    d_componentState = core::objectmodel::ComponentState::Valid;
}

///
void ScalarFieldMapping::apply( const MechanicalParams* mparams, OutDataVecCoord& out_, const InDataVecCoord& in_)
{
    SOFA_UNUSED(mparams);
    if(!isComponentStateValid())
        return;

    auto in = sofa::helper::getReadAccessor(in_);
    auto out = sofa::helper::getWriteOnlyAccessor(out_);
    auto field = l_field.get();
    int domain{0};

    for(Size i=0; i<out.size(); i++)
    {
        sofa::core::eq(out[i], field->getValue(in[i],domain));
    }
}

/// This method must be reimplemented by all mappings.
void ScalarFieldMapping::applyJ( const MechanicalParams* mparams, OutDataVecDeriv& dy_, const InDataVecDeriv& dx_)
{
    SOFA_UNUSED(mparams);
    if(!isComponentStateValid())
        return;
    auto x = fromModel->readPositions();
    auto dx = sofa::helper::getReadAccessor(dx_);
    auto dy = sofa::helper::getWriteOnlyAccessor(dy_);
    auto field = l_field.get();

    for (Size i = 0; i < x.size(); ++i)
    {
        const Vec3d grad = field->getGradient(x[i]);
        dy[i] = sofa::type::dot(grad, dx[i]);
    }
}

/// This method must be reimplemented by all mappings.
void ScalarFieldMapping::applyJT( const MechanicalParams* mparams, InDataVecDeriv& dx_, const OutDataVecDeriv& dy_)
{
    SOFA_UNUSED(mparams);
    if(!isComponentStateValid())
        return;
    auto x = fromModel->readPositions();
    auto dx = sofa::helper::getWriteOnlyAccessor(dx_);
    auto dy = sofa::helper::getReadAccessor(dy_);
    auto field = l_field.get();

    for (Size i = 0; i < dx.size(); ++i)
    {
        const Vec3d grad = field->getGradient(x[i]);
        dx[i] += grad * dy[i].x();                           //< Because dy is in R so there is only one value
    }
}

void ScalarFieldMapping::applyJT( const ConstraintParams* mparams, InDataMatrixDeriv& dx_, const OutDataMatrixDeriv& dy_)
{
    SOFA_UNUSED(mparams);
    if(!isComponentStateValid())
        return;
    auto x = fromModel->readPositions();
    auto y = toModel->readPositions();
    auto dx = sofa::helper::getWriteOnlyAccessor(dx_);
    auto dy = sofa::helper::getReadAccessor(dy_);
    auto field = l_field.get();

    std::cout << "Constraint InDataMatrixDeriv:" << dx_ << std::endl;
    std::cout << "Constraint OutDataMatrixDeriv:" << dy_ << std::endl;

    // for (Size i = 0; i < dx.size(); ++i)
    // {
    //     const Vec3d grad = field->getGradient(x[i]);
    //     dx[i] += grad * dy[i].x();                           //< Because dy is in R so there is only one value
    // }
}


void ScalarFieldMapping::buildGeometricStiffnessMatrix(sofa::core::GeometricStiffnessMatrix* matrices)
{
    if(!isComponentStateValid())
        return;

    const auto childForces = this->toModel->readTotalForces();
    const auto dJdx = matrices->getMappingDerivativeIn(this->fromModel).withRespectToPositionsIn(this->fromModel);
    const auto x = fromModel->readPositions();

    auto field = l_field.get();

    for (Size i = 0; i < x.size(); ++i)
    {
        const auto f = childForces[i];

        type::Mat3x3d H;
        field->getHessian(x[i], H);

        const sofa::type::Mat3x3d Ki = H * f.x(); // again les forces sont des "forces" 1d.
        for (int a = 0; a < 3; ++a)
        {
            for (int b = 0; b < 3; ++b)
            {
                dJdx(3*i + a, 3*i + b) += Ki[a][b];
            }
        }
    }
}

void registerScalarFieldMapping(sofa::core::ObjectFactory* factory)
{
    factory->registerObjects(sofa::core::ObjectRegistrationData("Maps a positional field to its scalar field values.")
    .add< ScalarFieldMapping >());
}

}
