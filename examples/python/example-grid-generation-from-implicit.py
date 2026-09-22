import Sofa
import SofaImplicitField

from Sofa.Types import RGBAColor
from SofaTypes.SofaTypes import Vec3d
from xshape.primitives import *
from xshape.transforms import *
from xshape.operators import *  

class DrawController(Sofa.Core.Controller):
   def __init__(self, *args, **kwargs):
      Sofa.Core.Controller.__init__(self, *args, **kwargs)

   def draw(self, visual_context):
      dt = visual_context.getDrawTool()
      dt.drawText([-1.0, 1.0, 0.5], 0.2, "Union(Sphere, Box)", RGBAColor(1.0,1.0,1.0,1.0))
      dt.drawText([ 1.0, 1.0, 0.5], 0.2, "Difference(Sphere, Box)", RGBAColor(1.0,1.0,1.0,1.0))
      
def createScene(root : Sofa.Core.Node):
    """Creates two different grids from two different scalar field and visualize them as mesh 
       using mesh extraction from implicit field. 
    """
    root.addObject("RequiredPlugin", pluginName="SofaImplicitField")

    root.addObject(DrawController())

    ########################### The same field is used for both grids ##################
    if True:
       f1 = root.addObject(
         Difference(name="field1",
               childA=Sphere(name="sphere", center=[0,0,0],radius=0.7),
               childB=RoundedBox(center=[0.0,0.0,0.0],dimensions=[0.95,0.5,0.5], rounding_radius=0.1))
      ) 
    else:
      #f1 = root.addObject(SpatialField(name="sphere", axis=0)) 
      f1 = root.addObject(Sphere(name="sphere", center=[0,0,0],radius=1.0)) 
          
    ########################### GridGeneration ##################
    g = root.addChild("Grid_10x10x10")
    m1 = g.addObject("GridSampler", name="sampler", min=[-2,-2,-2], max=[2,2,2], resolution=[255,255,255])
    m1.field.setLinkedBase(f1)

    e1 = g.addObject("DiscreteGridField", name="grid1", buffer=m1.buffer.linkpath)
        
    m2 = g.addObject("FieldToSurfaceMesh", name="polygonizer1",
                          field=e1.linkpath, min=[-2,-2,-2], max=[2,2,2], step=0.01)
    r = g.addObject("OglModel", name="renderer",
                        position=g.polygonizer1.points.linkpath,
                        triangles=g.polygonizer1.triangles.linkpath)

