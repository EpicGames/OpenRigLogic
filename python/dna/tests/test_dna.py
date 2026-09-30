#-*- coding: utf-8 -*-
import inspect
import os
import unittest

if hasattr(os, 'add_dll_directory') and 'LD_LIBRARY_PATH' in os.environ:
    os.add_dll_directory(os.environ['LD_LIBRARY_PATH'])

import dna


ANY_MISSING = "{} missing from wrapper"

METHODS_MISSING = """
methods missing in tests, but found in {className}:
{missing}
methods missing in {className}, but found in tests:
{extraneous}
"""

PARAMS_MISSING = """
{methodName}'s parameter list:
{parameters}
params missing in tests, but found in {methodName}'s params:
{missing}
params missing in {methodName}, but found in tests:
{extraneous}
"""


def merged(source, *args):
    """Return a copy of the `source` dict with all `args` merged into it"""
    result = source.copy()
    for ext in args:
        result.update(ext)
    return result


class TestClass(unittest.TestCase):

    def setUp(self):
        self.allSymbols = inspect.getmembers(dna)

    def findObject(self, source, objectName, **kwargs):
        objects = [item for item in source if item[0] == objectName]
        self.assertTrue(len(objects) != 0,
                        kwargs.get('error', ANY_MISSING.format(objectName)))
        return (objects[0][0], objects[0][1])

    def findClass(self, className):
        return self.findObject(self.allSymbols, className)

    def findMethod(self, obj, methodName, **kwargs):
        return self.findObject(inspect.getmembers(obj), methodName, **kwargs)

    def findAllMethods(self, obj):
        members = inspect.getmembers(obj, predicate=lambda m: inspect.ismethod(m) or inspect.isbuiltin(m) or inspect.isfunction(m))
        return [k for (k, v) in members if "__" not in k and
                k != "_s" and k != "thisown"]

    def findAllParameters(self, obj):
        if inspect.isbuiltin(obj):
            return []

        spec = inspect.getfullargspec(obj)
        return [arg for arg in spec.args if "self" not in arg]

    def formatErrorMessage(self, template, result, expected, **kwargs):
        missing = sorted(set(result).difference(expected))
        extraneous = sorted(set(expected).difference(result))
        return template.format(missing=missing,
                               extraneous=extraneous,
                               **kwargs)

    def assertClassExists(self, className):
        (foundClassName, foundClassObject) = self.findClass(className)
        with self.subTest(className):
            self.assertTrue(inspect.isclass(foundClassObject))
            self.assertTrue(className == foundClassName)

    def assertMethodParametersExist(self, methodName, methodObject, parameters):
        foundParameters = self.findAllParameters(methodObject)
        errorMessage = self.formatErrorMessage(PARAMS_MISSING,
                                               result=foundParameters,
                                               expected=parameters,
                                               parameters=foundParameters,
                                               methodName=methodName)
        self.assertEqual(len(foundParameters), len(parameters), errorMessage)
        for param in parameters:
            self.assertTrue(param in foundParameters, errorMessage)

    def assertClassMethodsExist(self, className, methods):
        (className, classObject) = self.findClass(className)
        foundMethods = self.findAllMethods(classObject)
        errorMessage = self.formatErrorMessage(METHODS_MISSING,
                                               foundMethods,
                                               methods,
                                               className=className)
        self.assertEqual(len(foundMethods), len(methods), errorMessage)
        for (methodName, parameters) in methods.items():
            (methodName, methodObject) = self.findMethod(classObject,
                                                         methodName,
                                                         error=errorMessage)
            with self.subTest(className + "_" + methodName):
                compoundMethodName = '.'.join((className, methodName))
                self.assertMethodParametersExist(compoundMethodName,
                                                 methodObject,
                                                 parameters)


class TestLibrary(TestClass):
    tests = {}

    def testExistence(self):
        for (className, methods) in self.tests.items():
            self.assertClassExists(className)
            self.assertClassMethodsExist(className, methods)


class TestVersionInfo(TestLibrary):
    tests = {
        "VersionInfo": {
            "getMajorVersion": [],
            "getMinorVersion": [],
            "getPatchVersion": [],
            "getVersionString": []
        }
    }


class TestStatus(TestLibrary):
    tests = {
        "Status": {
            "isOk": [],
            "get": [],
            "getHook": [],
            "setHook": ["hook"]
        }
    }


class TestMemoryResource(TestLibrary):
    tests = {
        "MemoryResource": {
            "allocate": ["size", "alignment"],
            "deallocate": ["ptr", "size", "alignment"]
        }
    }


class TestAlignedMemoryResource(TestLibrary):
    tests = {
        "AlignedMemoryResource": {
            "allocate": ["size", "alignment"],
            "deallocate": ["ptr", "size", "alignment"]
        }
    }


class TestDefaultMemoryResource(TestLibrary):
    tests = {
        "DefaultMemoryResource": {
            "allocate": ["size", "alignment"],
            "deallocate": ["ptr", "size", "alignment"]
        }
    }


class TestBoundedIOStream(TestLibrary):
    tests = {
        "BoundedIOStream": {
            "open": [],
            "close": [],
            "tell": [],
            "seek": ["position"],
            "flush": [],
            "size": [],
            "read": [],
            "write": []
        }
    }


class TestFileStream(TestLibrary):
    tests = {
        "FileStream": merged(
            TestBoundedIOStream.tests["BoundedIOStream"], {
            }
        )
    }


class TestHeaderReader(TestLibrary):
    tests = {
        "HeaderReader": {
            "getFileFormatGeneration": [],
            "getFileFormatVersion": []
        }
    }


class TestDescriptorReader(TestLibrary):
    tests = {
        "DescriptorReader": merged(
            TestHeaderReader.tests["HeaderReader"], {
                "getName": [],
                "getArchetype": [],
                "getGender": [],
                "getAge": [],
                "getMetaDataCount": [],
                "getMetaDataKey": ["index"],
                "getMetaDataValue": ["key"],
                "getTranslationUnit": [],
                "getRotationUnit": [],
                "getCoordinateSystem": [],
                "getRotationSequence": [],
                "getRotationSign": [],
                "getFaceWindingOrder": [],
                "getLODCount": [],
                "getDBMaxLOD": [],
                "getDBComplexity": [],
                "getDBName": []
            }
        )
    }


class TestDefinitionReader(TestLibrary):
    tests = {
        "DefinitionReader": merged(
            TestDescriptorReader.tests["DescriptorReader"], {
                "getGUIControlCount": [],
                "getGUIControlName": ["index"],
                "getRawControlCount": [],
                "getRawControlName": ["index"],
                "getJointCount": [],
                "getJointName": ["index"],
                "getJointIndexListCount": [],
                "getJointIndicesForLOD": ["lod"],
                "getJointParentIndex": ["index"],
                "getBlendShapeChannelCount": [],
                "getBlendShapeChannelIndexListCount": [],
                "getBlendShapeChannelName": ["index"],
                "getBlendShapeChannelIndexListCount": [],
                "getBlendShapeChannelIndicesForLOD": ["lod"],
                "getAnimatedMapCount": [],
                "getAnimatedMapIndexListCount": [],
                "getAnimatedMapName": ["index"],
                "getAnimatedMapIndexListCount": [],
                "getAnimatedMapIndicesForLOD": ["lod"],
                "getMeshCount": [],
                "getMeshIndexListCount": [],
                "getMeshName": ["index"],
                "getMeshIndexListCount": [],
                "getMeshIndicesForLOD": ["lod"],
                "getMeshBlendShapeChannelMappingCount": [],
                "getMeshBlendShapeChannelMapping": ["index"],
                "getMeshBlendShapeChannelMappingIndicesForLOD": ["lod"],
                "getNeutralJointTranslation": ["index"],
                "getNeutralJointTranslationXs": [],
                "getNeutralJointTranslationYs": [],
                "getNeutralJointTranslationZs": [],
                "getNeutralJointRotation": ["index"],
                "getNeutralJointRotationXs": [],
                "getNeutralJointRotationYs": [],
                "getNeutralJointRotationZs": []
            }
        )
    }


class TestBehaviorReader(TestLibrary):
    tests = {
        "BehaviorReader": merged(
            TestDefinitionReader.tests["DefinitionReader"], {
                "getGUIToRawInputIndices": [],
                "getGUIToRawOutputIndices": [],
                "getGUIToRawFromValues": [],
                "getGUIToRawToValues": [],
                "getGUIToRawSlopeValues": [],
                "getGUIToRawCutValues": [],
                "getPSDCount": [],
                "getPSDRowIndices": [],
                "getPSDColumnIndices": [],
                "getPSDValues": [],
                "getJointRowCount": [],
                "getJointColumnCount": [],
                "getJointVariableAttributeIndices": ["lod"],
                "getJointGroupCount": [],
                "getJointGroupLODs": ["jointGroupIndex"],
                "getJointGroupInputIndices": ["jointGroupIndex"],
                "getJointGroupOutputIndices": ["jointGroupIndex"],
                "getJointGroupValues": ["jointGroupIndex"],
                "getJointGroupJointIndices": ["jointGroupIndex"],
                "getBlendShapeChannelLODs": [],
                "getBlendShapeChannelInputIndices": [],
                "getBlendShapeChannelOutputIndices": [],
                "getAnimatedMapLODs": [],
                "getAnimatedMapInputIndices": [],
                "getAnimatedMapOutputIndices": [],
                "getAnimatedMapFromValues": [],
                "getAnimatedMapToValues": [],
                "getAnimatedMapSlopeValues": [],
                "getAnimatedMapCutValues": []
            }
        )
    }


class TestJointBehaviorMetadataReader(TestLibrary):
    tests = {
        "JointBehaviorMetadataReader": merged(
            TestDefinitionReader.tests["DefinitionReader"], {
                "getJointTranslationRepresentation": ["jointIndex"],
                "getJointRotationRepresentation": ["jointIndex"],
                "getJointScaleRepresentation": ["jointIndex"]
            }
        )
    }


class TestGeometryReader(TestLibrary):
    tests = {
        "GeometryReader": merged(
            TestDefinitionReader.tests["DefinitionReader"], {
                "getVertexPositionCount": ["meshIndex"],
                "getVertexPosition": ["meshIndex", "vertexIndex"],
                "getVertexPositionXs": ["meshIndex"],
                "getVertexPositionYs": ["meshIndex"],
                "getVertexPositionZs": ["meshIndex"],
                "getVertexTextureCoordinateCount": ["meshIndex"],
                "getVertexTextureCoordinate": ["meshIndex", "textureCoordinateIndex"],
                "getVertexTextureCoordinateUs": ["meshIndex"],
                "getVertexTextureCoordinateVs": ["meshIndex"],
                "getVertexNormalCount": ["meshIndex"],
                "getVertexNormal": ["meshIndex", "normalIndex"],
                "getVertexNormalXs": ["meshIndex"],
                "getVertexNormalYs": ["meshIndex"],
                "getVertexNormalZs": ["meshIndex"],
                "getVertexLayoutCount": ["meshIndex"],
                "getVertexLayout": ["meshIndex", "layoutIndex"],
                "getVertexLayoutPositionIndices": ["meshIndex"],
                "getVertexLayoutTextureCoordinateIndices": ["meshIndex"],
                "getVertexLayoutNormalIndices": ["meshIndex"],
                "getFaceCount": ["meshIndex"],
                "getFaceVertexLayoutIndices": ["meshIndex", "faceIndex"],
                "getMaximumInfluencePerVertex": ["meshIndex"],
                "getSkinWeightsCount": ["meshIndex"],
                "getSkinWeightsValues": ["meshIndex", "vertexIndex"],
                "getSkinWeightsJointIndices": ["meshIndex", "vertexIndex"],
                "getBlendShapeTargetCount": ["meshIndex"],
                "getBlendShapeChannelIndex": ["meshIndex", "blendShapeTargetIndex"],
                "getBlendShapeTargetDeltaCount": ["meshIndex", "blendShapeTargetIndex"],
                "getBlendShapeTargetDelta": ["meshIndex", "blendShapeTargetIndex", "deltaIndex"],
                "getBlendShapeTargetDeltaXs": ["meshIndex", "blendShapeTargetIndex"],
                "getBlendShapeTargetDeltaYs": ["meshIndex", "blendShapeTargetIndex"],
                "getBlendShapeTargetDeltaZs": ["meshIndex", "blendShapeTargetIndex"],
                "getBlendShapeTargetVertexIndices": ["meshIndex", "blendShapeTargetIndex"]
            }
        )
    }


class TestMachineLearnedBehaviorReader(TestLibrary):
    tests = {
        "MachineLearnedBehaviorReader": merged(
            TestDefinitionReader.tests["DefinitionReader"], {
                "getMLControlCount": [],
                "getMLControlName": ["index"],
                "getNeuralNetworkCount": [],
                "getNeuralNetworkIndexListCount": [],
                "getNeuralNetworkIndicesForLOD": ["lod"],
                "getMeshRegionCount": ["meshIndex"],
                "getMeshRegionName": ["meshIndex", "regionIndex"],
                "getNeuralNetworkIndicesForMeshRegion": ["meshIndex", "regionIndex"],
                "getNeuralNetworkInputIndices": ["netIndex"],
                "getNeuralNetworkOutputIndices": ["netIndex"],
                "getNeuralNetworkLayerCount": ["netIndex"],
                "getNeuralNetworkLayerActivationFunction": ["netIndex", "layerIndex"],
                "getNeuralNetworkLayerActivationFunctionParameters": ["netIndex", "layerIndex"],
                "getNeuralNetworkLayerBiases": ["netIndex", "layerIndex"],
                "getNeuralNetworkLayerWeights": ["netIndex", "layerIndex"]
            }
        )
    }


class TestMachineLearnedBehaviorExtReader(TestLibrary):
    tests = {
        "MachineLearnedBehaviorExtReader": merged(
            TestMachineLearnedBehaviorReader.tests["MachineLearnedBehaviorReader"], {
                "getMLTypeCount": [],
                "getMLOperationSetCount": ["mlTypeIndex"],
                "getMLOperationCount": ["mlTypeIndex", "mlOperationSetIndex"],
                "getMLOperationType": ["mlTypeIndex", "mlOperationSetIndex", "mlOperationIndex"],
                "getMLOperationParameters": ["mlTypeIndex", "mlOperationSetIndex", "mlOperationIndex"],
                "getMLOperationDependencyOperationSetIndices": ["mlTypeIndex", "mlOperationSetIndex", "mlOperationIndex"],
                "getMLOperationDependencyOperationIndices": ["mlTypeIndex", "mlOperationSetIndex", "mlOperationIndex"],
                "getMLOperationIndexListCount": ["mlTypeIndex", "mlOperationSetIndex"],
                "getMLOperationIndicesForLOD": ["mlTypeIndex", "mlOperationSetIndex", "lod"],
                "getMLJointsInputIndices": [],
                "getMLJointsOutputIndices": [],
                "getMLJointsParameterKeys": [],
                "getMLJointsParameterValues": []
            }
        )
    }


class TestRBFBehaviorReader(TestLibrary):
    tests = {
        "RBFBehaviorReader": merged(
            TestBehaviorReader.tests["BehaviorReader"], {
                "getRBFPoseCount": [],
                "getRBFPoseName": ["poseIndex"],
                "getRBFPoseJointOutputIndices": ["poseIndex"],
                "getRBFPoseBlendShapeChannelOutputIndices": ["poseIndex"],
                "getRBFPoseAnimatedMapOutputIndices": ["poseIndex"],
                "getRBFPoseJointOutputValues": ["poseIndex"],
                "getRBFPoseScale": ["poseIndex"],
                "getRBFPoseControlCount": [],
                "getRBFPoseControlName": ["poseControlIndex"],
                "getRBFPoseInputControlIndices": ["poseIndex"],
                "getRBFPoseOutputControlIndices": ["poseIndex"],
                "getRBFPoseOutputControlWeights": ["poseIndex"],
                "getRBFSolverCount": [],
                "getRBFSolverIndexListCount": [],
                "getRBFSolverIndicesForLOD": ["lod"],
                "getRBFSolverName": ["solverIndex"],
                "getRBFSolverRawControlIndices": ["solverIndex"],
                "getRBFSolverPoseIndices": ["solverIndex"],
                "getRBFSolverRawControlValues": ["solverIndex"],
                "getRBFSolverType": ["solverIndex"],
                "getRBFSolverRadius": ["solverIndex"],
                "getRBFSolverAutomaticRadius": ["solverIndex"],
                "getRBFSolverWeightThreshold": ["solverIndex"],
                "getRBFSolverDistanceMethod": ["solverIndex"],
                "getRBFSolverNormalizeMethod": ["solverIndex"],
                "getRBFSolverFunctionType": ["solverIndex"],
                "getRBFSolverTwistAxis": ["solverIndex"]
            }
        )
    }


class TestTwistSwingBehaviorReader(TestLibrary):
    tests = {
        "TwistSwingBehaviorReader": merged(
            TestDefinitionReader.tests["DefinitionReader"], {
                "getTwistCount": [],
                "getTwistSetupTwistAxis": ["twistIndex"],
                "getTwistInputControlIndices": ["twistIndex"],
                "getTwistOutputJointIndices": ["twistIndex"],
                "getTwistBlendWeights": ["twistIndex"],
                "getSwingCount": [],
                "getSwingSetupTwistAxis": ["swingIndex"],
                "getSwingInputControlIndices": ["swingIndex"],
                "getSwingOutputJointIndices": ["swingIndex"],
                "getSwingBlendWeights": ["swingIndex"],
            }
        )
    }


class TestReader(TestLibrary):
    tests = {
        "Reader": merged(
            TestHeaderReader.tests["HeaderReader"],
            TestDescriptorReader.tests["DescriptorReader"],
            TestDefinitionReader.tests["DefinitionReader"],
            TestBehaviorReader.tests["BehaviorReader"],
            TestGeometryReader.tests["GeometryReader"],
            TestMachineLearnedBehaviorReader.tests["MachineLearnedBehaviorReader"],
            TestMachineLearnedBehaviorExtReader.tests["MachineLearnedBehaviorExtReader"], {
                "unload": ["layer"]
            },
            TestRBFBehaviorReader.tests["RBFBehaviorReader"],
            TestJointBehaviorMetadataReader.tests["JointBehaviorMetadataReader"],
            TestTwistSwingBehaviorReader.tests["TwistSwingBehaviorReader"]
        )
    }


class TestStreamReader(TestLibrary):
    tests = {
        "StreamReader": merged(
            TestReader.tests["Reader"], {
                "read": []
            }
         )
    }


class TestBinaryStreamReader(TestLibrary):
    tests = {
        "BinaryStreamReader": merged(
            TestStreamReader.tests["StreamReader"], {
            }
         )
    }


class TestHeaderWriter(TestLibrary):
    tests = {
        "HeaderWriter": {
            "setFileFormatGeneration": ["generation"],
            "setFileFormatVersion": ["version"]
        }
    }


class TestDescriptorWriter(TestLibrary):
    tests = {
        "DescriptorWriter": merged(
            TestHeaderWriter.tests["HeaderWriter"], {
                "setName": ["name"],
                "setArchetype": ["archetype"],
                "setGender": ["gender"],
                "setAge": ["age"],
                "clearMetaData": [],
                "setMetaData": ["key", "value"],
                "setTranslationUnit": ["unit"],
                "setRotationUnit": ["unit"],
                "setCoordinateSystem": ["system"],
                "setRotationSequence": ["rotationSequence"],
                "setRotationSign": ["rotationSign"],
                "setFaceWindingOrder": ["faceWindingOrder"],
                "setLODCount": ["lodCount"],
                "setDBMaxLOD": ["lod"],
                "setDBComplexity": ["name"],
                "setDBName": ["name"]
            }
        )
    }


class TestDefinitionWriter(TestLibrary):
    tests = {
        "DefinitionWriter": merged(
            TestDescriptorWriter.tests["DescriptorWriter"], {
                "clearGUIControlNames": [],
                "setGUIControlName": ["index", "name"],
                "clearRawControlNames": [],
                "setRawControlName": ["index", "name"],
                "clearJointNames": [],
                "setJointName": ["index", "name"],
                "clearJointIndices": [],
                "setJointIndices": ["index", "jointIndices"],
                "clearLODJointMappings": [],
                "setLODJointMapping": ["lod", "index"],
                "clearBlendShapeChannelNames": [],
                "setBlendShapeChannelName": ["index", "name"],
                "clearBlendShapeChannelIndices": [],
                "setBlendShapeChannelIndices": ["index", "blendShapeChannelIndices"],
                "clearLODBlendShapeChannelMappings": [],
                "setLODBlendShapeChannelMapping": ["lod", "index"],
                "clearAnimatedMapNames": [],
                "setAnimatedMapName": ["index", "name"],
                "clearAnimatedMapIndices": [],
                "setAnimatedMapIndices": ["index", "animatedMapIndices"],
                "clearLODAnimatedMapMappings": [],
                "setLODAnimatedMapMapping": ["lod", "index"],
                "clearMeshNames": [],
                "setMeshName": ["index", "name"],
                "clearMeshIndices": [],
                "setMeshIndices": ["index", "meshIndices"],
                "clearLODMeshMappings": [],
                "setLODMeshMapping": ["lod", "index"],
                "clearMeshBlendShapeChannelMappings": [],
                "setMeshBlendShapeChannelMapping": ["index", "meshIndex", "blendShapeChannelIndex"],
                "setJointHierarchy": ["jointIndices"],
                "setNeutralJointTranslations": ["translations"],
                "setNeutralJointRotations": ["rotations"],
                "setMeshBlendShapeChannelMapping": ["index", "meshIndex", "blendShapeChannelIndex"]
            }
        )
    }


class TestBehaviorWriter(TestLibrary):
    tests = {
        "BehaviorWriter": merged(
            TestDefinitionWriter.tests["DefinitionWriter"], {
                "clearJointGroups": [],
                "deleteJointGroup": ["jointGroupIndex"],
                "setGUIToRawInputIndices": ["inputIndices"],
                "setGUIToRawOutputIndices": ["outputIndices"],
                "setGUIToRawFromValues": ["fromValues"],
                "setGUIToRawToValues": ["toValues"],
                "setGUIToRawSlopeValues": ["slopeValues"],
                "setGUIToRawCutValues": ["cutValues"],
                "setPSDCount": ["count"],
                "setPSDRowIndices": ["rowIndices"],
                "setPSDColumnIndices": ["columnIndices"],
                "setPSDValues": ["weights"],
                "setJointRowCount": ["rowCount"],
                "setJointColumnCount": ["columnCount"],
                "clearJointGroups": [],
                "deleteJointGroup": ["jointGroupIndex"],
                "setJointGroupLODs": ["jointGroupIndex", "lods"],
                "setJointGroupInputIndices": ["jointGroupIndex", "inputIndices"],
                "setJointGroupOutputIndices": ["jointGroupIndex", "outputIndices"],
                "setJointGroupValues": ["jointGroupIndex", "values"],
                "setJointGroupJointIndices": ["jointGroupIndex", "jointIndices"],
                "setBlendShapeChannelLODs": ["lods"],
                "setBlendShapeChannelInputIndices": ["inputIndices"],
                "setBlendShapeChannelOutputIndices": ["outputIndices"],
                "setAnimatedMapLODs": ["lods"],
                "setAnimatedMapInputIndices": ["inputIndices"],
                "setAnimatedMapOutputIndices": ["outputIndices"],
                "setAnimatedMapFromValues": ["fromValues"],
                "setAnimatedMapToValues": ["toValues"],
                "setAnimatedMapSlopeValues": ["slopeValues"],
                "setAnimatedMapCutValues": ["cutValues"]
            }
        )
    }


class TestJointBehaviorMetadataWriter(TestLibrary):
    tests = {
        "JointBehaviorMetadataWriter": merged(
            TestDefinitionWriter.tests["DefinitionWriter"], {
                "clearJointRepresentations": [],
                "setJointTranslationRepresentation": ["jointIndex", "representation"],
                "setJointRotationRepresentation": ["jointIndex", "representation"],
                "setJointScaleRepresentation": ["jointIndex", "representation"]
            }
        )
    }


class TestGeometryWriter(TestLibrary):
    tests = {
        "GeometryWriter": merged(
            TestDefinitionWriter.tests["DefinitionWriter"], {
                "clearMeshes": [],
                "deleteMesh": ["meshIndex"],
                "setVertexPositions": ["meshIndex", "positions"],
                "setVertexTextureCoordinates": ["meshIndex", "textureCoordinates"],
                "setVertexNormals": ["meshIndex", "normals"],
                "setVertexLayouts": ["meshIndex", "layouts"],
                "clearFaceVertexLayoutIndices": ["meshIndex"],
                "setFaceVertexLayoutIndices": ["meshIndex", "faceIndex", "layoutIndices"],
                "setMaximumInfluencePerVertex": ["meshIndex", "maxInfluenceCount"],
                "clearSkinWeights": ["meshIndex"],
                "setSkinWeightsValues": ["meshIndex", "vertexIndex", "weights"],
                "setSkinWeightsJointIndices": ["meshIndex", "vertexIndex", "jointIndices"],
                "clearBlendShapeTargets": ["meshIndex"],
                "setBlendShapeChannelIndex": ["meshIndex", "blendShapeTargetIndex", "blendShapeChannelIndex"],
                "setBlendShapeTargetDeltas": ["meshIndex", "blendShapeTargetIndex", "deltas"],
                "setBlendShapeTargetVertexIndices": ["meshIndex", "blendShapeTargetIndex", "vertexIndices"]
            }
        )
    }


class TestMachineLearnedBehaviorWriter(TestLibrary):
    tests = {
        "MachineLearnedBehaviorWriter": merged(
            TestDefinitionWriter.tests["DefinitionWriter"], {
                "setMLControlName": ["index", "name"],
                "clearMLControlNames": [],
                "clearNeuralNetworks": [],
                "clearNeuralNetworkIndices": [],
                "setNeuralNetworkIndices": ["index", "netIndices"],
                "clearLODNeuralNetworkMappings": [],
                "setLODNeuralNetworkMapping": ["lod", "index"],
                "clearMeshRegionNames": [],
                "setMeshRegionName": ["meshIndex", "regionIndex", "name"],
                "clearNeuralNetworkIndicesPerMeshRegion": [],
                "setNeuralNetworkIndicesForMeshRegion": ["meshIndex", "regionIndex", "netIndices"],
                "deleteNeuralNetwork": ["netIndex"],
                "setNeuralNetworkInputIndices": ["netIndex", "inputIndices"],
                "setNeuralNetworkOutputIndices": ["netIndex", "outputIndices"],
                "clearNeuralNetworkLayers": ["netIndex"],
                "setNeuralNetworkLayerActivationFunction": ["netIndex", "layerIndex", "function"],
                "setNeuralNetworkLayerActivationFunctionParameters": ["netIndex", "layerIndex", "activationFunctionParameters"],
                "setNeuralNetworkLayerBiases": ["netIndex", "layerIndex", "biases"],
                "setNeuralNetworkLayerWeights": ["netIndex", "layerIndex", "weights"]
            }
        )
    }


class TestMachineLearnedBehaviorExtWriter(TestLibrary):
    tests = {
        "MachineLearnedBehaviorExtWriter": merged(
            TestMachineLearnedBehaviorWriter.tests["MachineLearnedBehaviorWriter"], {
                "clearMLExtData": [],
                "clearMLOperationSets": ["mlTypeIndex"],
                "clearMLOperations": ["mlTypeIndex", "mlOperationSetIndex"],
                "setMLOperationType": ["mlTypeIndex", "mlOperationSetIndex", "mlOperationIndex", "operationType"],
                "setMLOperationParameters": ["mlTypeIndex", "mlOperationSetIndex", "mlOperationIndex", "parameters"],
                "setMLOperationDependencyOperationSetIndices": ["mlTypeIndex", "mlOperationSetIndex", "mlOperationIndex", "indices"],
                "setMLOperationDependencyOperationIndices": ["mlTypeIndex", "mlOperationSetIndex", "mlOperationIndex", "indices"],
                "clearMLOperationIndicesAndLODMappings": [],
                "clearMLOperationIndices": ["mlTypeIndex", "mlOperationSetIndex"],
                "clearLODMLOperationMappings": ["mlTypeIndex", "mlOperationSetIndex"],
                "setMLOperationIndices": ["mlTypeIndex", "mlOperationSetIndex", "index", "mlOperationIndices"],
                "setLODMLOperationMapping": ["mlTypeIndex", "mlOperationSetIndex", "lod", "index"],
                "setMLJointsParameterKeys": ["parameterKeys"],
                "setMLJointsParameterValues": ["parameterValues"],
                "setMLJointsInputIndices": ["inputIndices"],
                "setMLJointsOutputIndices": ["outputIndices"]
            }
        )
    }


class TestRBFBehaviorWriter(TestLibrary):
    tests = {
        "RBFBehaviorWriter": merged(
            TestBehaviorWriter.tests["BehaviorWriter"], {
                "clearRBFPoses": [],
                "setRBFPoseName": ["poseIndex", "name"],
                "setRBFPoseScale": ["poseIndex", "scale"],
                "clearRBFPoseControlNames": [],
                "setRBFPoseControlName": ["poseControlIndex", "name"],
                "setRBFPoseInputControlIndices": ["poseIndex", "controlIndices"],
                "setRBFPoseOutputControlIndices": ["poseIndex", "controlIndices"],
                "setRBFPoseOutputControlWeights": ["poseIndex", "controlWeights"],
                "clearRBFSolvers": [],
                "clearRBFSolverIndices": [],
                "setRBFSolverIndices": ["index", "solverIndices"],
                "clearLODRBFSolverMappings": [],
                "setLODRBFSolverMapping": ["lod", "index"],
                "setRBFSolverName": ["solverIndex", "name"],
                "setRBFSolverRawControlIndices": ["solverIndex", "rawControlIndices"],
                "setRBFSolverPoseIndices": ["solverIndex", "poseIndices"],
                "setRBFSolverRawControlValues": ["solverIndex", "values"],
                "setRBFSolverType": ["solverIndex", "type"],
                "setRBFSolverRadius": ["solverIndex", "radius"],
                "setRBFSolverAutomaticRadius": ["solverIndex", "automaticRadius"],
                "setRBFSolverWeightThreshold": ["solverIndex", "weightThreshold"],
                "setRBFSolverDistanceMethod": ["solverIndex", "distanceMethod"],
                "setRBFSolverNormalizeMethod": ["solverIndex", "normalizeMethod"],
                "setRBFSolverFunctionType": ["solverIndex", "functionType"],
                "setRBFSolverTwistAxis": ["solverIndex", "twistAxis"]
            }
        )
    }


class TestTwistSwingBehaviorWriter(TestLibrary):
    tests = {
        "TwistSwingBehaviorWriter": merged(
            TestDefinitionWriter.tests["DefinitionWriter"], {
                "clearTwists": [],
                "deleteTwist": ["twistIndex"],
                "setTwistSetupTwistAxis": ["twistIndex", "twistAxis"],
                "setTwistInputControlIndices": ["twistIndex", "controlIndices"],
                "setTwistOutputJointIndices": ["twistIndex", "jointIndices"],
                "setTwistBlendWeights": ["twistIndex", "blendWeights"],
                "clearSwings": [],
                "deleteSwing": ["swingIndex"],
                "setSwingSetupTwistAxis": ["swingIndex", "twistAxis"],
                "setSwingInputControlIndices": ["swingIndex", "controlIndices"],
                "setSwingOutputJointIndices": ["swingIndex", "jointIndices"],
                "setSwingBlendWeights": ["swingIndex", "blendWeights"],
            }
        )
    }


class TestWriter(TestLibrary):
    tests = {
        "Writer": merged(
            TestHeaderWriter.tests["HeaderWriter"],
            TestDescriptorWriter.tests["DescriptorWriter"],
            TestDefinitionWriter.tests["DefinitionWriter"],
            TestBehaviorWriter.tests["BehaviorWriter"],
            TestGeometryWriter.tests["GeometryWriter"],
            TestMachineLearnedBehaviorWriter.tests["MachineLearnedBehaviorWriter"],
            TestMachineLearnedBehaviorExtWriter.tests["MachineLearnedBehaviorExtWriter"], {
                "setFrom": []  # has three overloads, but they're hidden by swig's dispatch mechanism
            },
            TestRBFBehaviorWriter.tests["RBFBehaviorWriter"],
            TestJointBehaviorMetadataWriter.tests["JointBehaviorMetadataWriter"],
            TestTwistSwingBehaviorWriter.tests["TwistSwingBehaviorWriter"]
        )
    }


class TestStreamWriter(TestLibrary):
    tests = {
        "StreamWriter": merged(
            TestWriter.tests["Writer"], {
                "write": []
            }
        )
    }


class TestBinaryStreamWriter(TestLibrary):
    tests = {
        "BinaryStreamWriter": merged(
            TestStreamWriter.tests["StreamWriter"], {
            }
        )
    }



class TestCharPtrArguments(TestClass):
    """Anything but str for a const char* parameter must raise TypeError (used to segfault)."""

    def setUp(self):
        self.stream = dna.MemoryStream()
        self.writer = dna.BinaryStreamWriter(self.stream)

    def testNonStringIsRejected(self):
        for value in (0, 1.5, b"bytes", [], None):
            with self.subTest(value=value):
                with self.assertRaises(TypeError):
                    self.writer.setName(value)

    def testNoneIsRejectedForEveryStringParameter(self):
        with self.assertRaises(TypeError):
            self.writer.setJointName(0, None)
        with self.assertRaises(TypeError):
            self.writer.setMetaData(None, "value")
        with self.assertRaises(TypeError):
            dna.FileStream(None, dna.FileStream.AccessMode_Read, dna.FileStream.OpenMode_Binary)

    def testNoneMetaDataValueDeletesKey(self):
        # Documented DescriptorWriter contract; the only const char* parameter where None is legal.
        self.writer.setMetaData("key", "value")
        self.writer.setMetaData("key", None)
        self.writer.write()
        self.assertTrue(dna.Status.isOk())
        self.stream.seek(0)
        reader = dna.BinaryStreamReader(self.stream, dna.DataLayer_All)
        reader.read()
        self.assertTrue(dna.Status.isOk())
        self.assertEqual(reader.getMetaDataCount(), 0)
        with self.assertRaises(TypeError):
            reader.getMetaDataValue(None)


if __name__ == '__main__':
    unittest.main()
