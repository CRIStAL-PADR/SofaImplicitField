/******************************************************************************
*                 SOFA, Simulation Open-Framework Architecture                *
*                    (c) 2006 INRIA, USTL, UJF, CNRS, MGH                     *
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
#include <SofaImplicitField/config.h>
#include <SofaImplicitField/components/geometry/DiscreteGridField.h>
#include <sofa/core/visual/VisualParams.h>
#include <SofaImplicitField/MHD.h>

#include <bits/stdc++.h>
#include <algorithm>

#include <sofa/core/ObjectFactory.h>
using sofa::core::RegisterObject ;

#include <cmath>

namespace sofa::component::geometry
{

DiscreteGridField::DiscreteGridField()
    : ScalarField(),
      d_distanceMapHeader( initData( &d_distanceMapHeader, "file", "MHD file for the distance map" ) ),
      d_min(initData( &d_min, {-0.5,-0.5,-0.5}, "min", "The min positions in world space" ) ),
      d_max(initData( &d_max, {0.5 ,0.5 ,0.5}, "max", "The max positions in world space" ) ),
      d_resolution(initData( &d_resolution, {10,10,10}, "resolution", "The resolution of each axis of the grid" ) ),
      d_buffer(initData(&d_buffer, "buffer", "The data buffer olding the values")),
      d_debugDraw(initData(&d_debugDraw, false, "debugDraw", "show the values on the grid"))
{
    addUpdateCallback("updateFromRMM",{&d_resolution, &d_min, &d_max},[this](const sofa::core::DataTracker&){

        /// Update the internal buffers when d_resolution change
        std::cout << "WE ARE GOING TO UPDATE FROM DATA " << d_resolution.getCounter() << std::endl;

        auto buffer = sofa::helper::getWriteOnlyAccessor(d_buffer);
        buffer->resize(d_min.getValue(), d_max.getValue(), d_resolution.getValue());
        internalUpdate(buffer);
        refreshTrackers();

        return sofa::core::objectmodel::ComponentState::Valid;
    }, {});

    addUpdateCallback("updateFromData",{&d_buffer},[this](const sofa::core::DataTracker&){

        /// Update the internal buffers when d_resolution change
        std::cout << "WE ARE GOING TO UPDATE FROM DATA BUFFER: " << d_buffer.getCounter() << std::endl;

        /// Update the internal state
        auto buffer = sofa::helper::getReadAccessor(d_buffer);
        internalUpdate(buffer);

        /// Propagate the change to the other inputs.
        d_resolution.setValue(buffer->resolution);
        d_min.setValue(buffer->min);
        d_max.setValue(buffer->max);

        refreshTrackers();

        return sofa::core::objectmodel::ComponentState::Valid;
    }, {});
}

DiscreteGridField::~DiscreteGridField()
{
}

// Clean the tracker so we don't call the update mechanisme twice
void DiscreteGridField::refreshTrackers()
{
    for(auto& tracker : m_internalEngine){
        tracker.second.cleanDirty();
    }
}

void DiscreteGridField::init()
{
    if(d_distanceMapHeader.isSet())
    {
        bool ok = loadGridFromMHD( d_distanceMapHeader.getFullPath().c_str() );
        if (ok){
            msg_info() << "Successfully loaded a distance map from file.";
            d_componentState = sofa::core::objectmodel::ComponentState::Valid;
        }
        else{
            d_componentState = sofa::core::objectmodel::ComponentState::Invalid;
        }
        return;
    }
    std::cout << getPathName() << " init" << std::endl;

    if(!d_buffer.isSet()){
        internalResize(d_resolution.getValue(), d_min.getValue(), d_max.getValue());
    }else{
        const auto& buffer = sofa::helper::getReadAccessor(d_buffer);
        std::cout << getPathName() << "   buffer " << buffer->min << ", " << buffer->max << " and " << buffer->data << std::endl;
        internalUpdate(buffer);
    }

    std::cout << getPathName() << " init done" << std::endl;
    d_componentState = sofa::core::objectmodel::ComponentState::Valid;
}

bool DiscreteGridField::loadGridFromMHD( const char *filename )
{
    auto buffer = sofa::helper::getWriteOnlyAccessor(d_buffer);

    bool loadSucceeded = sofaimplicitfield::loader::loadGridFromMHD(filename,
                                                                    buffer->min, buffer->spacing,
                                                                    buffer->resolution, buffer->data);

    if(!loadSucceeded)
        return false;

    // init remaining variables
    // for (int d=0; d<3; d++)
    // {
    //     scaling[d] = 1.0/spacing[d];
    //     max[d] = min[d] + (double)(resolution[d]-1)*spacing[d];
    // }
    // m_deltaOfs[0] = 0;
    // m_deltaOfs[1] = 1;
    // m_deltaOfs[2] = resolution[0];
    // m_deltaOfs[3] = resolution[0]+1;

    // unsigned int sliceSize = resolution[0]*resolution[1];
    // m_deltaOfs[4] = m_deltaOfs[0] + sliceSize;
    // m_deltaOfs[5] = m_deltaOfs[1] + sliceSize;
    // m_deltaOfs[6] = m_deltaOfs[2] + sliceSize;
    // m_deltaOfs[7] = m_deltaOfs[3] + sliceSize;

    return true;
}

void MemoryBuffer::resize(const Vec3d& min_, const Vec3d& max_, const Vec3u& resolution_)
{
    resolution = resolution_;
    min = min_;
    max = max_;

    auto newImgSize = resolution.x() * resolution.y() * resolution.z();
    if(size != newImgSize){
        std::cout << "Resizeing memory buffer " << data << " to " << newImgSize << std::endl;
        if (data)
        {
            delete[] data;
        }
        data = new float[newImgSize];
        size = newImgSize;
    }
    std::cout << "Resizeing done... " << data << std::endl;

    spacing = (max - min).linearDivision(Vec3d{
        static_cast<double>(resolution[0] - 1 > 0 ? resolution[0] - 1 : 1),
        static_cast<double>(resolution[1] - 1 > 0 ? resolution[1] - 1 : 1),
        static_cast<double>(resolution[2] - 1 > 0 ? resolution[2] - 1 : 1)
    });

    scaling = Vec3d{1.0,1.0,1.0}.linearDivision(spacing);
}

void DiscreteGridField::internalUpdate(const MemoryBuffer& buffer)
{
    const auto& resolution = buffer.resolution;

    unsigned int sliceSize = resolution[0] * resolution[1];
    m_deltaOfs[0] = 0;
    m_deltaOfs[1] = 1;
    m_deltaOfs[2] = resolution[0];
    m_deltaOfs[3] = resolution[0] + 1;
    m_deltaOfs[4] = sliceSize;
    m_deltaOfs[5] = sliceSize + 1;
    m_deltaOfs[6] = sliceSize + resolution[0];
    m_deltaOfs[7] = sliceSize + resolution[0] + 1;

    d_resolution.setValue(resolution);
    d_min.setValue(buffer.min);
    d_max.setValue(buffer.max);
}

void DiscreteGridField::internalResize(const Vec3u& resolution, const Vec3d& min, const Vec3d& max)
{
    auto buffer = sofa::helper::getWriteOnlyAccessor(d_buffer);
    buffer->resize(min,max,resolution);
    internalUpdate(buffer);
}

bool DiscreteGridField::empty()
{
    auto buffer = sofa::helper::getReadAccessor(d_buffer);
    return buffer->data == nullptr;
}

void DiscreteGridField::draw(const sofa::core::visual::VisualParams* params)
{
    if(!d_debugDraw.getValue())
        return;

    auto dt = params->drawTool();
    const auto& buffer = d_buffer.getValue();
    auto& resolution = buffer.resolution;
    auto& min = buffer.min;
    auto& data = buffer.data;
    auto& spacing = buffer.spacing;

    auto index = [resolution](unsigned int x, unsigned int y, unsigned int z) { return x + resolution.x() * y + (resolution.x() * resolution.y() * z); };

    for(unsigned int x=0;x<resolution.x();++x)
    {
        for(unsigned int y=0;y<resolution.y();++y)
        {
            for(unsigned int z=0;z<resolution.z();++z)
            {
                Vec3d pos {x,y,z};
                pos = min+(pos.linearProduct(spacing));
                int i =0;
                double r = getValue(pos,i);
                std::stringstream s;
                s << std::setprecision(1) << data[index(x,y,z)]  << " vs " << r << "(" << x << "," << y << "," << z << ")";
                dt->draw3DText(pos, 0.1, type::RGBAColor::cyan(), s.str().c_str());
            }
        }
    }
}

void DiscreteGridField::getValues(const std::vector<Vec3d>& positions, std::vector<double>& results)
{
    auto buffer = sofa::helper::getReadAccessor(d_buffer);
    if(buffer->data==nullptr)
        return;

    const auto& resolution = buffer->resolution;
    const auto& min = buffer->min;
    const auto& scaling = buffer->scaling;
    const auto& spacing = buffer->spacing;
    const auto& data = buffer->data;

    auto getValue = [&min, &scaling, &resolution, &data](const Vec3d position){
        Vec3d localPosition = (position-min);

        Vec3d t = localPosition.linearProduct(scaling);

        // Compute the indices of voxels surrounding the position
        unsigned int x0 = std::min((unsigned int)std::floor(t.x()), resolution.x() - 2 );
        unsigned int y0 = std::min((unsigned int)std::floor(t.y()), resolution.y() - 2 );
        unsigned int z0 = std::min((unsigned int)std::floor(t.z()), resolution.z() - 2 );

        unsigned int x1 = x0 + 1;
        unsigned int y1 = y0 + 1;
        unsigned int z1 = z0 + 1;

        t = t-Vec3d{x0,y0,z0};

        // Helper function to access the index
        auto index = [resolution](unsigned int x, unsigned int y, unsigned int z) { return x + resolution.x() * y + (resolution.x() * resolution.y() * z); };

        // Get the height raw values surrounding the position
        const double c000 = data[index(x0, y0, z0)];
        const double c100 = data[index(x1, y0, z0)];
        const double c010 = data[index(x0, y1, z0)];
        const double c110 = data[index(x1, y1, z0)];
        const double c001 = data[index(x0, y0, z1)];
        const double c101 = data[index(x1, y0, z1)];
        const double c011 = data[index(x0, y1, z1)];
        const double c111 = data[index(x1, y1, z1)];

        // X
        const double c00 = c000 * (1.0 - t.x()) + c100 * t.x();
        const double c10 = c010 * (1.0 - t.x()) + c110 * t.x();
        const double c01 = c001 * (1.0 - t.x()) + c101 * t.x();
        const double c11 = c011 * (1.0 - t.x()) + c111 * t.x();

        // Y
        const double c0 = c00 * (1.0 - t.y()) + c10 * t.y();
        const double c1 = c01 * (1.0 - t.y()) + c11 * t.y();

        // Z
        const double value = c0 * (1.0 - t.z()) + c1 * t.z();

        return value;
    };

    for(unsigned int i=0;i<positions.size();++i){
        results[i] = getValue(positions[i]);
    }

}

double DiscreteGridField::getValue(Vec3d &position, int &)
{
    auto buffer = sofa::helper::getReadAccessor(d_buffer);
    if(buffer->data==nullptr)
        return -1.0;

    const auto& resolution = buffer->resolution;
    const auto& min = buffer->min;
    const auto& scaling = buffer->scaling;
    const auto& spacing = buffer->spacing;
    const auto& data = buffer->data;

    Vec3d localPosition = (position-min);

    // use trilinear interpolation to get the value at any location
    Vec3d t = localPosition.linearProduct(scaling);

    // Compute the indices of voxels surrounding the position
    unsigned int x0 = std::min((unsigned int)std::floor(t.x()), resolution.x() - 2 );
    unsigned int y0 = std::min((unsigned int)std::floor(t.y()), resolution.y() - 2 );
    unsigned int z0 = std::min((unsigned int)std::floor(t.z()), resolution.z() - 2 );

    unsigned int x1 = x0 + 1;
    unsigned int y1 = y0 + 1;
    unsigned int z1 = z0 + 1;

    t = t-Vec3d{x0,y0,z0};

    // Helper function to access the index
    auto index = [resolution](unsigned int x, unsigned int y, unsigned int z) { return x + resolution.x() * y + (resolution.x() * resolution.y() * z); };

    // Get the height raw values surrounding the position
    const double c000 = data[index(x0, y0, z0)];
    const double c100 = data[index(x1, y0, z0)];
    const double c010 = data[index(x0, y1, z0)];
    const double c110 = data[index(x1, y1, z0)];
    const double c001 = data[index(x0, y0, z1)];
    const double c101 = data[index(x1, y0, z1)];
    const double c011 = data[index(x0, y1, z1)];
    const double c111 = data[index(x1, y1, z1)];

    // X
    const double c00 = c000 * (1.0 - t.x()) + c100 * t.x();
    const double c10 = c010 * (1.0 - t.x()) + c110 * t.x();
    const double c01 = c001 * (1.0 - t.x()) + c101 * t.x();
    const double c11 = c011 * (1.0 - t.x()) + c111 * t.x();

    // Y
    const double c0 = c00 * (1.0 - t.y()) + c10 * t.y();
    const double c1 = c01 * (1.0 - t.y()) + c11 * t.y();

    // Z
    const double value = c0 * (1.0 - t.z()) + c1 * t.z();

    //std::cout << "POSITION " << position << "("<< x0 << "," << y0 << "," << z0 << " and "<< t << ") value " << c000 << std::endl;
    return value;
}

// Register in the Factory
void registerDiscreteGridField(sofa::core::ObjectFactory* factory)
{
    factory->registerObjects(sofa::core::ObjectRegistrationData("A discrete scalar field from a regular grid storing field value with interpolation.")
    .add< DiscreteGridField >());
}

}

namespace sofa::core::objectmodel
{
/// Specialization for MemoryBuffer
template<> bool Data<component::geometry::MemoryBuffer>::read( const std::string&) { return false; }
template<> void Data<component::geometry::MemoryBuffer>::printValue( std::ostream& ) const {}
template<> std::string Data<component::geometry::MemoryBuffer>::getValueString() const {
    std::stringstream tmp;
    auto& buffer = getValue();
    tmp << "Grid<" << buffer.resolution.x() << "," << buffer.resolution.y() << "," << buffer.resolution.z() << "> @"<<buffer.data;
    return tmp.str() ; }
template<> std::string Data<component::geometry::MemoryBuffer>::getDefaultValueString() const { return ""; }
}