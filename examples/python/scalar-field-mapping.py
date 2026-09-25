import Sofa
import Sofa.Core
from SofaImplicitField import ScalarField
import numpy

def Model():
    particules = Sofa.Core.Node("Particles")
    particules.addObject("MechanicalObject", name="state", template="Vec3", position=[[-1,0,0],[0,0,0],[1,0,0]])
    particules.addObject("UniformMass", name="mass", totalMass=1.0)
    
    child = particules.addChild("Child")
    child.addObject("MechanicalObject", name="state", template="Vec1", position=[0,0,0])
    child.addObject("SphericalField", name="field", center=[0,0,0], radius=1.0)
    child.addObject("ScalarFieldMapping", name="mapping", 
                         input=particules.state.linkpath, 
                         output=child.state.linkpath, field=child.field.linkpath)

    return particules

def createScene(root):
    """In this scene we create a ScalarField of spherical shape and a mechanical object of 3D particles. 
       The field and the particules are used as input of a ScalarFieldMapping, mapping the particules to a 1D child space.
       This child space is then used in a constraints. 
    """
    root.addObject('RequiredPlugin', pluginName='Sofa.Component.AnimationLoop') # Needed to use components [FreeMotionAnimationLoop]  
    root.addObject('RequiredPlugin', pluginName='Sofa.Component.Constraint.Lagrangian.Model') # Needed to use components [StopperLagrangianConstraint]  
    root.addObject('RequiredPlugin', pluginName='Sofa.Component.Constraint.Lagrangian.Solver') # Needed to use components [BlockGaussSeidelConstraintSolver]  
    root.addObject('RequiredPlugin', pluginName='Sofa.Component.StateContainer') # Needed to use components [MechanicalObject]  
    root.addObject("RequiredPlugin", pluginName="SofaImplicitField")
    root.addObject('RequiredPlugin', pluginName='Sofa.Component.Mass') # Needed to use components [UniformMass]  

    root.addObject("FreeMotionAnimationLoop")
    root.addObject("BlockGaussSeidelConstraintSolver", name="solver", tolerance=1e-6, maxIterations=1000)

    root.addObject("EulerImplicitSolver", name="odesolver", rayleighStiffness=0.1, rayleighMass=0.1)
    root.addObject("SparseLDLSolver", name="linearSolver", template="CompressedRowSparseMatrixd")

    model = root.addChild(Model())
    model.state.showObject = True
    model.state.showObjectScale = 5.0

    model.Child.addObject("StopperLagrangianConstraint", name="constraint", min=0.0, max=1.0, index=3)
    model.Child.addObject("GenericConstraintCorrection", name="correction", linearSolver=root.linearSolver.linkpath, ODESolver=root.odesolver.linkpath)
