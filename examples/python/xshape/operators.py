from SofaImplicitField import ScalarField
import numpy

class Union(ScalarField):
    """Union of two scalar fields"""
    def __init__(self, *args, **kwargs):
        ScalarField.__init__(self, *args, **kwargs)

        self.childA = kwargs.get("childA", None)
        self.childB = kwargs.get("childB", None)

    def getValue(self, position):
        return min(self.childA.getValue(position), self.childB.getValue(position))

    def getValues(self, positions, results):
        results[:] = numpy.minimum(self.childA.getValues(positions, numpy.empty(len(positions))), self.childB.getValues(positions, numpy.empty(len(positions))))
        return results

class Difference(ScalarField):
    """Difference of two scalar fields"""
    def __init__(self, *args, **kwargs):
        ScalarField.__init__(self, *args, **kwargs)

        self.childA = kwargs.get("childA", None)
        self.childB = kwargs.get("childB", None)

    def getValue(self, position):
        return max(-self.childA.getValue(position), self.childB.getValue(position))

    def getValues(self, positions, results):
        results[:] = numpy.maximum(-self.childA.getValues(positions, numpy.empty(len(positions))), self.childB.getValues(positions, numpy.empty(len(positions))))
        return results

class Intersection(ScalarField):
    """Intersection of two scalar fields"""
    def __init__(self, *args, **kwargs):
        ScalarField.__init__(self, *args, **kwargs)

        self.childA = kwargs.get("childA", None)
        self.childB = kwargs.get("childB", None)

    def getValue(self, position):
        return max(self.childA.getValue(position), self.childB.getValue(position))

    def getValues(self, positions, results):
        results[:] = numpy.maximum(self.childA.getValues(positions, numpy.empty(len(positions))), self.childB.getValues(positions, numpy.empty(len(positions))))
        return results  