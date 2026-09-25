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
#include <SofaImplicitField/components/engine/GridSampler.h>

#include <sofa/core/ObjectFactory.h>
#include <sofa/core/visual/VisualParams.h>
using sofa::core::RegisterObject;
using sofa::core::visual::VisualParams;

namespace sofaimplicitfield::component::engine
{

GridSampler::GridSampler()
    : d_resolution(initData(&d_resolution, Vec3u(10, 10, 10), "resolution", "Number of samples in each dimension"))
    , d_min(initData(&d_min, Vec3d(-1.0, -1.0, -1.0), "min", "Minimum corner of the sampling grid"))
    , d_max(initData(&d_max, Vec3d(1.0, 1.0, 1.0), "max", "Maximum corner of the sampling grid"))
    , d_buffer(initData(&d_buffer, "buffer", "The data buffer olding the values"))
    , l_field(initLink("field", "The scalar field to sample"))
{
    addUpdateCallback("sample", {&d_resolution, &d_min, &d_max}, [this](const sofa::core::DataTracker&)
    {
        Vec3u resolution = d_resolution.getValue();
        Vec3d min = d_min.getValue();
        Vec3d max = d_max.getValue();

        updateInternalBuffer(resolution, min, max);
        sampleField();
        return core::objectmodel::ComponentState::Valid;
    }, {});
}

GridSampler::~GridSampler()
{
}

void GridSampler::init()
{
    if (!l_field.get())
    {
        msg_error() << "Missing scalar field to sample";
        d_componentState = core::objectmodel::ComponentState::Invalid;
        return;
    }

    updateInternalBuffer(d_resolution.getValue(), d_min.getValue(), d_max.getValue());
    sampleField();
    d_componentState = core::objectmodel::ComponentState::Valid;
}

void GridSampler::updateInternalBuffer(const Vec3u& resolution, const Vec3d& min, const Vec3d& max)
{
    auto buffer = sofa::helper::getWriteOnlyAccessor(d_buffer);
    buffer->resize(min, max, resolution);
}

void GridSampler::sampleField()
{
    auto field = l_field.get();
    if (!field)
    {
        msg_error() << "No scalar field linked";
        return;
    }

    auto buffer = sofa::helper::getWriteAccessor(d_buffer);
    auto resolution = buffer->resolution;
    auto min = buffer->min;
    auto max = buffer->max;

    if (resolution[0] <= 0 || resolution[1] <= 0 || resolution[2] <= 0)
    {
        msg_error() << "Resolution must be greater than 0 in all dimensions";
        return;
    }

    if (min[0] >= max[0] || min[1] >= max[1] || min[2] >= max[2])
    {
        msg_error() << "min must be less than max in all dimensions";
        return;
    }

    std::cout << getPathName() << " sampling the grid " << std::endl;
    auto data = buffer->data;
    auto spacing = buffer->spacing;

    std::cout << "          : " << buffer->data << std::endl;

    int rx = resolution.x();
    int rxy = resolution.y() * rx;

    std::vector<Vec3d> positions;
    std::vector<double> results;
    positions.reserve(rxy);
    results.reserve(rxy);

    for (unsigned int z = 0; z < resolution[2]; z++)
    {
        positions.clear();
        results.clear();
        for (unsigned int y = 0; y < resolution[1]; y++)
        {
            for (unsigned int x = 0; x < resolution[0]; x++)
            {
                positions.emplace_back(min + Vec3d(
                    static_cast<double>(x) * spacing[0],
                    static_cast<double>(y) * spacing[1],
                    static_cast<double>(z) * spacing[2]
                ));
            }
        }
        field->getValues(positions, results);
        for (unsigned int y = 0; y < resolution[1]; y++)
        {
            for (unsigned int x = 0; x < resolution[0]; x++)
            {
                auto index=[rxy,rx](int x,int y, int z){ return z * rxy + y * rx + x; };
                auto rindex=[rx](int x,int y){ return y * rx + x; };

                data[index(x,y,z)] = results[rindex(x,y)];
            }
        }
    }
    std::cout << "SAMPLING DONE "<< std::endl;
}

void GridSampler::computeBBox(const core::ExecParams* params, bool onlyVisible)
{
    if (onlyVisible) return;

    Vec3d min = d_min.getValue();
    Vec3d max = d_max.getValue();

    sofa::core::objectmodel::BaseComponent::computeBBox(params, onlyVisible);

    f_bbox.setValue({min, max});
}

void GridSampler::draw(const VisualParams* vparams)
{
    if (!vparams || !vparams->displayFlags().getShowBehaviorModels())
        return;

    if (isComponentStateInvalid())
        return;
}

void registerGridSampler(sofa::core::ObjectFactory* factory)
{
    factory->registerObjects(sofa::core::ObjectRegistrationData("Samples a ScalarField on a 3D grid and stores the result in a DiscreteGridField.")
    .add< GridSampler >());
}

}
