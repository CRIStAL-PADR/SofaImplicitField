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
#include <SofaImplicitField/MHD.h>

#include <sofa/core/ObjectFactory.h>
using sofa::core::RegisterObject ;

namespace sofa::component::geometry
{

DiscreteGridField::DiscreteGridField()
    : ScalarField(),
      d_distanceMapHeader( initData( &d_distanceMapHeader, "file", "MHD file for the distance map" ) ),
      d_maxDomains( initData( &d_maxDomains, 1, "maxDomains", "Number of domains available for caching" ) ),
      d_position(initData( &d_position, {0.0,0.0,0.0}, "position", "The position in world space of the grid" ) )
{
    m_usedDomains = 0;
    m_imgData = nullptr;
}

DiscreteGridField::~DiscreteGridField()
{
    if (m_imgData)
    {
        delete[] m_imgData;
        m_imgData = nullptr;
    }
}

///used to set a name in tests
void DiscreteGridField::setFilename(const std::string& name)
{
    d_distanceMapHeader.setValue(name);
}

void DiscreteGridField::init()
{
    m_domainCache.resize( d_maxDomains.getValue() );
    bool ok = loadGridFromMHD( d_distanceMapHeader.getFullPath().c_str() );
    if (ok) printf( "Successfully loaded distance map.\n" );
}

bool DiscreteGridField::loadGridFromMHD( const char *filename )
{
    bool loadSucceeded = sofaimplicitfield::loader::loadGridFromMHD(filename, m_imgMin, m_spacing, m_imgSize, m_imgData);

    if(!loadSucceeded)
        return false;

    // init remaining variables
    for (int d=0; d<3; d++)
    {
        m_scale[d] = 1.0/m_spacing[d];
        m_imgMax[d] = m_imgMin[d] + (double)(m_imgSize[d]-1)*m_spacing[d];
    }
    m_deltaOfs[0] = 0;
    m_deltaOfs[1] = 1;
    m_deltaOfs[2] = m_imgSize[0];
    m_deltaOfs[3] = m_imgSize[0]+1;
    unsigned int sliceSize = m_imgSize[0]*m_imgSize[1];
    m_deltaOfs[4] = m_deltaOfs[0] + sliceSize;
    m_deltaOfs[5] = m_deltaOfs[1] + sliceSize;
    m_deltaOfs[6] = m_deltaOfs[2] + sliceSize;
    m_deltaOfs[7] = m_deltaOfs[3] + sliceSize;

    return true;
}

void DiscreteGridField::updateCache( DomainCache *cache, Vec3d& pos )
{
    cache->insideImg = true;
    for (int d=0; d<3; d++)
    {
        if (pos[d]<m_imgMin[d] || pos[d]>=m_imgMax[d])
        {
            cache->insideImg = false;
            break;
        }
    }
    if (cache->insideImg)
    {
        int voxMinPos[3];
        for (int d=0; d<3; d++)
        {
            voxMinPos[d] = (int)(m_scale[d] * (pos[d]-m_imgMin[d]));
            cache->bbMin[d] = m_spacing[d]*(double)voxMinPos[d] + m_imgMin[d];
            cache->bbMax[d] = cache->bbMin[d] + m_spacing[d];
        }
        unsigned int ofs = voxMinPos[0] + m_imgSize[0]*(voxMinPos[1] + m_imgSize[1]*voxMinPos[2]);
        cache->val[0] = m_imgData[ofs];
        for (int i=1; i<8; i++) cache->val[i] = m_imgData[ofs+m_deltaOfs[i]];
    }
    else
    {
        // init bounding box to be as large as possible to prevent unnecessary cache updates while outside image
        const double MIN=-10e6, MAX=10e6;
        int voxMappedPos[3];
        for (int d=0; d<3; d++)
        {
            if (pos[d] < m_imgMin[d])
            {
                cache->bbMin[d] = MIN;
                cache->bbMax[d] = m_imgMin[d];
                voxMappedPos[d] = 0;
            }
            else if (pos[d] >= m_imgMax[d])
            {
                cache->bbMin[d] = m_imgMax[d];
                cache->bbMax[d] = MAX;
                voxMappedPos[d] = m_imgSize[d]-1;
            }
            else
            {
                cache->bbMin[d] = MIN;
                cache->bbMax[d] = MAX;
                voxMappedPos[d] = (int)(m_scale[d] * (pos[d]-m_imgMin[d]));
            }
        }
        unsigned int ofs = voxMappedPos[0] + m_imgSize[0]*(voxMappedPos[1] + m_imgSize[1]*voxMappedPos[2]);
        // if cache lies outside image, the returned distance is not updated anymore, instead this boundary value is returned
        cache->val[0] = m_imgData[ofs] + m_spacing[0]+m_spacing[1]+m_spacing[2];
    }
}

int DiscreteGridField::getNextDomain()
{
    // while we have free domains always return the next one, afterwards always use the last one
    if (m_usedDomains < (int)m_domainCache.size()) m_usedDomains++;
    return m_usedDomains-1;
}

double DiscreteGridField::getValue( Vec3d &transformedPos, int &domain )
{
    // use translation
    Vec3d pos = d_position.getValue();

    // find cache domain and check if it needs an update
    DomainCache *cache;
    if (domain < 0)
    {
        domain = getNextDomain();
        cache = &(m_domainCache[domain]);
        updateCache( cache, pos );
    }
    else
    {
        cache = &(m_domainCache[domain]);
        for (int d=0; d<3; d++)
        {
            if (pos[d]<cache->bbMin[d] || pos[d]>cache->bbMax[d])
            {
                updateCache( cache, pos );
                break;
            }
        }
    }

    // if cache lies outside image, the returned distance is not updated anymore, instead this boundary value is returned
    if (!cache->insideImg) return cache->val[0];

    // use trilinear interpolation on cached cube
    double weight[3];
    for (int d=0; d<3; d++)
    {
        weight[d] = m_scale[d] * (pos[d]-cache->bbMin[d]);
    }
    double d = weight[0]*weight[1];
    double c = weight[1] - d;
    double b = weight[0] - d;
    double a = (1.0-weight[1]) - b;
    double res = ( cache->val[0]*a + cache->val[1]*b + cache->val[2]*c + cache->val[3]*d ) * (1.0-weight[2])
            + ( cache->val[4]*a + cache->val[5]*b + cache->val[6]*c + cache->val[7]*d ) * weight[2];

    return res;
}

double DiscreteGridField::getValue( Vec3d &transformedPos )
{
    static int domain=-1;
    return getValue( transformedPos, domain );
}

// Register in the Factory
void registerDiscreteGridField(sofa::core::ObjectFactory* factory)
{
    factory->registerObjects(sofa::core::ObjectRegistrationData("A discrete scalar field from a regular grid storing field value with interpolation.")
    .add< DiscreteGridField >());
}

}