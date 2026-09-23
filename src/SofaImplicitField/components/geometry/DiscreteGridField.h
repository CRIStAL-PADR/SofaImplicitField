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
#pragma once
#include <SofaImplicitField/config.h>

#include <sofa/core/objectmodel/DataFileName.h>
#include <SofaImplicitField/components/geometry/ScalarField.h>


namespace sofa::component::geometry
{

namespace
{
using sofa::type::Vec3;
using sofa::type::Vec3u;
using sofa::type::Vec3d;
}

class MemoryBuffer
{
public:
    Vec3d spacing;
    Vec3d scaling;
    Vec3d min;
    Vec3d max;
    Vec3u resolution;
    unsigned int size{0};
    float*       data{nullptr};

    void resize(const Vec3d& min_, const Vec3d& max_, const Vec3u& resolution_);
};


class SOFA_SOFAIMPLICITFIELD_API DiscreteGridField : public virtual ScalarField
{
public:
    SOFA_CLASS(DiscreteGridField, ScalarField);

    DiscreteGridField();
    ~DiscreteGridField() override;

    void init() override;
    void draw(const sofa::core::visual::VisualParams*) override;

    double getValue(const Vec3d& position, int& domain) override;
    void getValues(const std::vector<Vec3d>& positions, std::vector<double>& results) override;

    bool loadGridFromMHD( const char *filename ) ;

    sofa::core::objectmodel::DataFileName d_distanceMapHeader;

    Data<Vec3d> d_min;                // bounding box (min)
    Data<Vec3d> d_max;                // bounding box (max)
    Data<Vec3u> d_resolution;         // resolution of the grid along each axis

    Data<MemoryBuffer> d_buffer;

    Data<bool> d_debugDraw;

    unsigned int m_deltaOfs[8];     // offsets to define 8 corners of cube for interpolation
    bool empty();

    class Modifier
    {
    public:
        Modifier(DiscreteGridField* field){self=field;}
        ~Modifier(){ self->getComponentState(); }
        Modifier& resize(const Vec3u& resolution,
                         const Vec3d& gridMin, const Vec3d& gridMax){

            self->d_min.setValue(gridMin);
            self->d_max.setValue(gridMax);
            self->d_resolution.setValue(resolution);
            return *this; } //< resize the grid and re-allocate the buffers
    private:
        DiscreteGridField* self;
    };
    Modifier modify(){ return Modifier(this); }
    void refreshTrackers();

private:
    void internalUpdate(const MemoryBuffer& buffer);
    void internalResize(const Vec3u& resolution, const Vec3d& min, const Vec3d& max);

};

}

namespace sofa::core::objectmodel
{

/// Specialization for MemoryBuffer
template<> bool Data<component::geometry::MemoryBuffer>::read( const std::string&);
template<> void Data<component::geometry::MemoryBuffer>::printValue( std::ostream& ) const;
template<> std::string Data<component::geometry::MemoryBuffer>::getValueString() const;
template<> std::string Data<component::geometry::MemoryBuffer>::getDefaultValueString() const;

}
