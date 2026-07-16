cmake_minimum_required(VERSION 3.22)
project(SofaImplicitField VERSION 1.0)


if(NOT TARGET Sofa.Framework)
    find_package(Sofa.Framework REQUIRED) #For macros
endif()
sofa_find_package(Sofa.Component.Topology.Container.Constant REQUIRED)

set(SOFAIMPLICITFIELD_SRC "src/${PROJECT_NAME}")

set(HEADER_FILES
    ${SOFAIMPLICITFIELD_SRC}/config.h.in
    ${SOFAIMPLICITFIELD_SRC}/initSofaImplicitField.h
    ${SOFAIMPLICITFIELD_SRC}/MarchingCube.h

    # This is backward compatibility
    ${SOFAIMPLICITFIELD_SRC}/deprecated/SphereSurface.h
    ${SOFAIMPLICITFIELD_SRC}/deprecated/ImplicitSurfaceContainer.h    # This is a backward compatibility file toward ScalarField
    ${SOFAIMPLICITFIELD_SRC}/deprecated/InterpolatedImplicitSurface.h # This is a backward compatibility file toward DiscreteGridField

    ${SOFAIMPLICITFIELD_SRC}/components/engine/FieldToSurfaceMesh.h
    ${SOFAIMPLICITFIELD_SRC}/components/geometry/BottleField.h
    ${SOFAIMPLICITFIELD_SRC}/components/geometry/DiscreteGridField.h
    ${SOFAIMPLICITFIELD_SRC}/components/geometry/SphericalField.h
    ${SOFAIMPLICITFIELD_SRC}/components/geometry/ScalarField.h
    ${SOFAIMPLICITFIELD_SRC}/components/geometry/StarShapedField.h
    ${SOFAIMPLICITFIELD_SRC}/components/mapping/ImplicitSurfaceMapping.h
    ${SOFAIMPLICITFIELD_SRC}/components/mapping/ImplicitSurfaceMapping.inl
)

set(SOURCE_FILES
    ${SOFAIMPLICITFIELD_SRC}/initSofaImplicitField.cpp
    ${SOFAIMPLICITFIELD_SRC}/MarchingCube.cpp

    ## This is a backward compatibility..
    ${SOFAIMPLICITFIELD_SRC}/deprecated/SphereSurface.cpp
    ${SOFAIMPLICITFIELD_SRC}/deprecated/InterpolatedImplicitSurface.cpp
    
    ${SOFAIMPLICITFIELD_SRC}/components/engine/FieldToSurfaceMesh.cpp
    ${SOFAIMPLICITFIELD_SRC}/components/geometry/BottleField.cpp
    ${SOFAIMPLICITFIELD_SRC}/components/geometry/ScalarField.cpp
    ${SOFAIMPLICITFIELD_SRC}/components/geometry/DiscreteGridField.cpp
    ${SOFAIMPLICITFIELD_SRC}/components/geometry/SphericalField.cpp
    ${SOFAIMPLICITFIELD_SRC}/components/geometry/StarShapedField.cpp
    ${SOFAIMPLICITFIELD_SRC}/components/mapping/ImplicitSurfaceMapping.cpp
)

set(EXTRA_FILES
    README.md
    )

if(SOFA_BUILD_TESTS)
    add_subdirectory(SofaImplicitField_test)
endif()

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES} ${EXTRA_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE Sofa.Component.Topology.Container.Constant)

find_package(SofaPython3 REQUIRED)
if (SofaPython3_FOUND)
  add_subdirectory(python)
endif()

## Install rules for the library and headers; CMake package configurations files
sofa_create_package_with_targets(
    PACKAGE_NAME ${PROJECT_NAME}
    PACKAGE_VERSION ${PROJECT_VERSION}
    TARGETS ${PROJECT_NAME} AUTO_SET_TARGET_PROPERTIES
    INCLUDE_SOURCE_DIR "src"
    INCLUDE_INSTALL_DIR "${PROJECT_NAME}"
    RELOCATABLE "plugins"
    )
