## SofaImplicitField

The SofaImplicitField plugin adds support for implicit surfaces (shapes defined by a scalar field f(x,y,z), where the surface is the isovalue/level set) to SOFA.

It provides a ScalarField base class along with ready-made analytical fields (SphericalField, StarShapedField, BottleField) and a DiscreteGridField that samples a field from a grid, plus gradient/Hessian utilities via finite differences. A FieldToSurfaceMesh engine and Marching Cubes implementation convert these fields into visualizable/collidable triangle meshes, while ImplicitSurfaceMapping maps particle positions onto the implicit surface.

Python bindings expose ScalarField so scenes and custom fields can be defined and queried from Python.
