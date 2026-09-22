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
#include <SofaImplicitField/components/geometry/DiscreteGridField.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
namespace sofaimplicitfield::component::engine
{

namespace{
    using namespace sofa;
    using sofa::core::objectmodel::BaseComponent;
    using sofa::core::visual::VisualParams;
    using sofa::type::Vec3d;
    using sofa::type::Vec3u;
    using sofa::component::geometry::ScalarField;
    using sofa::component::geometry::DiscreteGridField;
}

class GridSampler : public BaseComponent
{
public:
    SOFA_CLASS(GridSampler, BaseComponent);

    void init() override;
    void draw(const VisualParams* params) override;

    Data<Vec3u> d_resolution;
    Data<Vec3d> d_min;
    Data<Vec3d> d_max;

    Data<sofa::component::geometry::MemoryBuffer> d_buffer;

    void notifyLinkSet(BaseLink*, Base*) override;

protected:
    SingleLink<GridSampler, ScalarField,
               BaseLink::FLAG_STOREPATH|BaseLink::FLAG_STRONGLINK> l_field;

protected:
    GridSampler();
    virtual ~GridSampler();

private:
    void computeBBox(const core::ExecParams* params, bool onlyVisible = false) override;
    void updateInternalBuffer(const Vec3u& resolution, const Vec3d& min, const Vec3d& max);
    void sampleField();
};

}

