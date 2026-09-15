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
#include <SofaImplicitField/config.h>
#include <SofaImplicitField/MHD.h>
#include <sofa/type/Vec.h>
#include <fstream>
#include <cstring>
#include <string>

namespace sofaimplicitfield::loader
{
using sofa::type::Vec3d;
using sofa::type::Vec3u;

bool loadGridFromMHD( const std::string& filename_, Vec3d& imgMin, Vec3d& spacing, Vec3u& imgSize, float*& imgData)
{
    const char* filename = filename_.c_str();
    imgMin={0.0, 0.0, 0.0};
    imgSize={0, 0, 0};
    spacing={0, 0, 0};

    char buffer[1024];
    char *value;
    bool dataFileSpecified = false;
    float f0, f1, f2;
    int i0, i1, i2;
    char dataFile[1024];

    // read header file
    std::ifstream header( filename );
    if (!header.is_open()) return false;
    while (!header.eof())
    {
        header.getline( buffer, 1024 );
        if (strncmp( buffer, "ObjectType", 10 ) == 0)
        {
            value = strchr( buffer, '=' )+1;  while (*value==' ') value++;
            if (strncmp( value, "Image", 5 ) != 0)
            {
                printf( "ERROR: Object is no image.\n" );
                return false;
            }
        }
        else if (strncmp( buffer, "NDims", 5 ) == 0)
        {
            value = strchr( buffer, '=' )+1;  while (*value==' ') value++;
            if (*value != '3')
            {
                printf( "ERROR: Wrong number of dimensions.\n" );
                return false;
            }
        }
        else if (strncmp( buffer, "BinaryData ", 11 ) == 0 || strncmp( buffer, "BinaryData=", 11 ) == 0)
        {
            value = strchr( buffer, '=' )+1;  while (*value==' ') value++;
            if (strncmp( value, "True", 4 ) != 0)
            {
                printf( "ERROR: Data is not binary.\n" );
                return false;
            }
        }
        else if (strncmp( buffer, "CompressedData", 14 ) == 0)
        {
            value = strchr( buffer, '=' )+1;  while (*value==' ') value++;
            if (strncmp( value, "False", 5 ) != 0)
            {
                printf( "ERROR: Data is compressed.\n" );
                return false;
            }
        }
        else if (strncmp( buffer, "TransformMatrix", 15 ) == 0)
        {
            value = strchr( buffer, '=' )+1;  while (*value==' ') value++;
            if (strncmp( value, "1 0 0 0 1 0 0 0 1", 17 ) != 0)
            {
                printf( "ERROR: Unsupported transform matrix.\n" );
                return false;
            }
        }
        else if (strncmp( buffer, "Offset", 6 ) == 0)
        {
            value = strchr( buffer, '=' )+1;  while (*value==' ') value++;
            sscanf( value, "%f %f %f", &f0, &f1, &f2 );
            imgMin[0]=f0;  imgMin[1]=f1;  imgMin[2]=f2;
            printf( "Image offset = %f %f %f\n", imgMin[0], imgMin[1], imgMin[2] );
        }
        else if (strncmp( buffer, "ElementSpacing", 14 ) == 0)
        {
            value = strchr( buffer, '=' )+1;  while (*value==' ') value++;
            sscanf( value, "%f %f %f", &f0, &f1, &f2 );
            spacing[0]=f0;  spacing[1]=f1;  spacing[2]=f2;
            printf( "Image spacing = %f %f %f\n", spacing[0], spacing[1], spacing[2] );
        }
        else if (strncmp( buffer, "DimSize", 7 ) == 0)
        {
            value = strchr( buffer, '=' )+1;  while (*value==' ') value++;
            sscanf( value, "%d %d %d", &i0, &i1, &i2 );
             imgSize[0]=i0;   imgSize[1]=i1;   imgSize[2]=i2;
            printf( "Image size = %i %i %i\n",  imgSize[0],  imgSize[1],  imgSize[2] );
        }
        else if (strncmp( buffer, "ElementType", 11 ) == 0)
        {
            value = strchr( buffer, '=' )+1;  while (*value==' ') value++;
            if (strncmp( value, "MET_FLOAT", 9) != 0)
            {
                printf( "ERROR: Datatype is not supported.\n" );
                return false;
            }
        }
    }
    header.close();

    // read data file
    if (!dataFileSpecified)
    {
        // change extension to .raw
        strncpy( dataFile, filename, sizeof(dataFile) - 1 );
        dataFile[sizeof(dataFile) - 1] = '\0';
        size_t lenWithoutExt = strlen( filename );
        if (lenWithoutExt >= 3)
            lenWithoutExt -= 3;
        if (lenWithoutExt < sizeof(dataFile) - 4)
        {
            dataFile[lenWithoutExt] = '\0';
            strncat( dataFile, "raw", sizeof(dataFile) - lenWithoutExt - 1 );
        }
        else
        {
            printf( "Warning: filename too long to replace extension, keeping '%s'\n", dataFile );
        }
    }
    std::ifstream data( dataFile, std::ios_base::binary|std::ios_base::in );
    if (!data.is_open()) return false;
    unsigned int numVoxels =  imgSize[0]* imgSize[1]* imgSize[2];
    imgData = new float[numVoxels];
    data.read( (char*)imgData, numVoxels*sizeof(float) );
    if (data.bad()) return false;
    data.close();
    return false;
}


}
