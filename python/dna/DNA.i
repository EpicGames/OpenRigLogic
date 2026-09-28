%pythonbegin
%{
import os
if hasattr(os, 'add_dll_directory'):
    for path in os.environ.get('PATH', '').split(';'):
        try:
            if path:
                os.add_dll_directory(path)
        except Exception:
            pass
%}

%define MODULEIMPORT
"
import os
import sys

module_loader_cls = None
for cls in sys.meta_path:
    if hasattr(cls, '__name__') and cls.__name__ == 'IsolatedModuleLoader':
        module_loader_cls = cls
        break

if module_loader_cls is None:
    import $module
else:
    $module = module_loader_cls.load_module('$module', rootdir=os.path.dirname(os.path.abspath(__file__)))
"
%enddef

%module(moduleimport=MODULEIMPORT) dna

%{
#include <pma/Defs.h>
#include <pma/MemoryResource.h>
#include <pma/resources/AlignedMemoryResource.h>
#include <pma/resources/ArenaMemoryResource.h>
#include <pma/resources/DefaultMemoryResource.h>
#include <status/Defs.h>
#include <status/StatusCode.h>
#include <status/Status.h>
#include <tdm/Types.h>
#include <tdm/CoordSys.h>
#include <tdm/TDM.h>
#include <trio/Defs.h>
#include <trio/Concepts.h>
#include <trio/Stream.h>
#include <trio/types/Aliases.h>
#include <trio/types/Parameters.h>
#include <trio/streams/FileStream.h>
#include <trio/streams/MemoryMappedFileStream.h>
#include <trio/streams/MemoryStream.h>

#include <arrayview/ArrayView.h>
#include <arrayview/StringView.h>

#include "dna/Defs.h"
#include "dna/Configuration.h"
#include "dna/types/Aliases.h"

#include "dna/layers/Descriptor.h"
#include "dna/layers/JointBehaviorMetadata.h"
#include "dna/layers/MachineLearnedBehavior.h"
#include "dna/layers/MachineLearnedBehaviorExt.h"
#include "dna/layers/Twist.h"
#include "dna/layers/RBFBehavior.h"
#include "dna/layers/Geometry.h"

#include "dna/layers/HeaderReader.h"
#include "dna/layers/DescriptorReader.h"
#include "dna/layers/DefinitionReader.h"
#include "dna/layers/BehaviorReader.h"
#include "dna/layers/JointBehaviorMetadataReader.h"
#include "dna/layers/MachineLearnedBehaviorReader.h"
#include "dna/layers/MachineLearnedBehaviorExtReader.h"
#include "dna/layers/RBFBehaviorReader.h"
#include "dna/layers/TwistSwingBehaviorReader.h"
#include "dna/layers/GeometryReader.h"
#include "dna/Reader.h"
#include "dna/StreamReader.h"
#include "dna/BinaryStreamReader.h"
#ifdef DNA_BUILD_WITH_JSON_SUPPORT
    #include "dna/JSONStreamReader.h"
#endif  // DNA_BUILD_WITH_JSON_SUPPORT

#include "dna/layers/HeaderWriter.h"
#include "dna/layers/DescriptorWriter.h"
#include "dna/layers/DefinitionWriter.h"
#include "dna/layers/BehaviorWriter.h"
#include "dna/layers/JointBehaviorMetadataWriter.h"
#include "dna/layers/MachineLearnedBehaviorWriter.h"
#include "dna/layers/MachineLearnedBehaviorExtWriter.h"
#include "dna/layers/RBFBehaviorWriter.h"
#include "dna/layers/TwistSwingBehaviorWriter.h"
#include "dna/layers/GeometryWriter.h"
#include "dna/Writer.h"
#include "dna/StreamWriter.h"
#include "dna/BinaryStreamWriter.h"
#ifdef DNA_BUILD_WITH_JSON_SUPPORT
    #include "dna/JSONStreamWriter.h"
#endif  // DNA_BUILD_WITH_JSON_SUPPORT

#include "dna/version/VersionInfo.h"
%}

%include <spyus/ExceptionHandling.i>
%include <spyus/CharPtr.i>
// setMetaData(key, nullptr) deletes the key (DescriptorWriter.h contract): None stays legal for value.
%typemap(check) const char* value "";

%include "stdint.i"
%include <spyus/Caster.i>

%ignore sc::operator==;
%ignore sc::operator!=;

%ignore tdm::operator==;
%ignore tdm::operator!=;
%ignore tdm::chirality;
%rename(Direction) tdm::axis_dir;
%rename(RotationDirection) tdm::rot_dir;
%rename(RotationSequence) tdm::rot_seq;
%rename(RotationSign) tdm::rot_sign;
%rename(CoordinateSystem) tdm::coord_sys;

%include <pma/Defs.h>
%include <pma/MemoryResource.h>
%include <pma/resources/AlignedMemoryResource.h>
%include <pma/resources/ArenaMemoryResource.h>
%include <pma/resources/DefaultMemoryResource.h>
%include <status/Defs.h>
%include <status/StatusCode.h>
%include <status/Status.h>
%include <tdm/Types.h>
%include <tdm/CoordSys.h>
%include <tdm/TDM.h>
%include <trio/Defs.h>
%include <trio/Concepts.h>
%include <trio/Stream.h>
%include <trio/types/Aliases.h>
%include <trio/types/Parameters.h>
%include <trio/streams/FileStream.h>
%include <trio/streams/MemoryMappedFileStream.h>
%include <trio/streams/MemoryStream.h>
pythonize_unmanaged_type(FileStream, create, destroy)
pythonize_unmanaged_type(MemoryMappedFileStream, create, destroy)
pythonize_unmanaged_type(MemoryStream, create, destroy)

%pythoncode %{
FileStream.AccessMode_Read = AccessMode_Read
FileStream.AccessMode_Write = AccessMode_Write
FileStream.AccessMode_ReadWrite = AccessMode_ReadWrite

FileStream.OpenMode_Binary = OpenMode_Binary
FileStream.OpenMode_Text = OpenMode_Text

MemoryMappedFileStream.AccessMode_Read = AccessMode_Read
MemoryMappedFileStream.AccessMode_Write = AccessMode_Write
MemoryMappedFileStream.AccessMode_ReadWrite = AccessMode_ReadWrite
%}

%include <spyus/ArrayView.i>
%include <spyus/Vector3.i>

%include "EnumTypeMap.i"
%include "Geometry.i"

%ignore av::operator==;
%ignore av::operator!=;

array_view_to_py_list(av::ArrayView);
%apply av::ArrayView {
    av::ConstArrayView,
    dna::ArrayView,
    dna::ConstArrayView
};

string_view_to_py_string(av::StringView);
%apply av::StringView {
    dna::StringView
};

vector3_typemap(dna::Vector3);
texture_coordinate_typemap(dna::TextureCoordinate);
vertex_layout_typemap(dna::VertexLayout);
enum_typemap(dna::MachineLearnedBehaviorOperationType);

py_list_to_c_array(const std::uint16_t* jointIndices, std::uint16_t count);
%apply (const std::uint16_t* jointIndices, std::uint16_t count) {
    (const std::uint16_t* animatedMapIndices, std::uint16_t count),
    (const std::uint16_t* meshIndices, std::uint16_t count),
    (const std::uint16_t* rowIndices, std::uint16_t count),
    (const std::uint16_t* columnIndices, std::uint16_t count),
    (const std::uint16_t* outputIndices, std::uint16_t count),
    (const std::uint16_t* inputIndices, std::uint16_t count),
    (const std::uint16_t* lods, std::uint16_t count),
    (const std::uint16_t* blendShapeChannelIndices, std::uint16_t count),
    (const std::uint16_t* netIndices, std::uint16_t count),
    (const std::uint16_t* solverIndices, std::uint16_t count),
    (const std::uint16_t* rawControlIndices, std::uint16_t count),
    (const std::uint16_t* poseIndices, std::uint16_t count),
    (const std::uint16_t* jointIndices, std::uint16_t jointIndexCount),
    (const std::uint16_t* controlIndices, std::uint16_t controlIndexCount),
    (const std::uint16_t* parameterKeys, std::uint16_t count),
    (const std::uint16_t* parameterValues, std::uint16_t count),
    (const std::uint16_t* mlOperationIndices, std::uint16_t count),
    (const std::uint16_t* indices, std::uint16_t count)
};

py_list_to_c_array(const std::uint32_t* parameters, std::uint16_t count);

py_list_to_c_array(const std::uint32_t* vertexIndices, std::uint32_t count);
%apply (const std::uint32_t* vertexIndices, std::uint32_t count) {
    (const std::uint32_t* layoutIndices, std::uint32_t count)
};

py_list_to_c_array(const float* fromValues, std::uint16_t count);
%apply (const float* fromValues, std::uint16_t count) {
    (const float* toValues, std::uint16_t count),
    (const float* slopeValues, std::uint16_t count),
    (const float* cutValues, std::uint16_t count),
    (const float* weights, std::uint16_t count),
    (const float* activationFunctionParameters, std::uint16_t count),
    (const float* values, std::uint16_t count),
    (const float* blendWeights, std::uint16_t blendWeightCount),
    (const float* controlWeights, std::uint16_t controlWeightCount)
};

py_list_to_c_array(const float* values, std::uint32_t count);
%apply(const float* values, std::uint32_t count) {
    (const float* biases, std::uint32_t count),
    (const float* weights, std::uint32_t count)
};

py_list_to_c_array(const dna::VertexLayout* layouts, std::uint32_t count);

py_list_to_c_array(const dna::TextureCoordinate* textureCoordinates, std::uint32_t count);

py_list_to_c_array(const dna::Normal* normals, std::uint32_t count);
%apply (const dna::Normal* normals, std::uint32_t count) {
    (const dna::Vector3* translations, std::uint16_t count),
    (const dna::Vector3* rotations, std::uint16_t count),
    (const dna::Vector3* scales, std::uint16_t count),
    (const dna::Position* positions, std::uint32_t count),
    (const dna::Delta* deltas, std::uint32_t count)
};

%include <arrayview/ArrayView.h>
%include <arrayview/StringView.h>

%include "dna/Defs.h"
%include "dna/types/Aliases.h"
%include "dna/types/Vector3.h"
%include "dna/Configuration.h"
%include "dna/layers/Descriptor.h"
%include "dna/layers/MachineLearnedBehavior.h"
%include "dna/layers/MachineLearnedBehaviorExt.h"
%include "dna/layers/Twist.h"
%include "dna/layers/RBFBehavior.h"
%include "dna/layers/JointBehaviorMetadata.h"
%include "dna/layers/Geometry.h"
%include "dna/layers/HeaderReader.h"
%include "dna/layers/DescriptorReader.h"
%include "dna/layers/DefinitionReader.h"
%include "dna/layers/BehaviorReader.h"
%include "dna/layers/JointBehaviorMetadataReader.h"
%include "dna/layers/MachineLearnedBehaviorReader.h"
%include "dna/layers/MachineLearnedBehaviorExtReader.h"
%include "dna/layers/RBFBehaviorReader.h"
%include "dna/layers/TwistSwingBehaviorReader.h"
%include "dna/layers/GeometryReader.h"
%include "dna/Reader.h"
%include "dna/StreamReader.h"
%include "dna/BinaryStreamReader.h"
pythonize_unmanaged_type(BinaryStreamReader, create, destroy)
#ifdef DNA_BUILD_WITH_JSON_SUPPORT
    %include "dna/JSONStreamReader.h"
    pythonize_unmanaged_type(JSONStreamReader, create, destroy)
#endif  // DNA_BUILD_WITH_JSON_SUPPORT

%include "dna/layers/HeaderWriter.h"
%include "dna/layers/DescriptorWriter.h"
%include "dna/layers/DefinitionWriter.h"
%include "dna/layers/BehaviorWriter.h"
%include "dna/layers/JointBehaviorMetadataWriter.h"
%include "dna/layers/MachineLearnedBehaviorWriter.h"
%include "dna/layers/MachineLearnedBehaviorExtWriter.h"
%include "dna/layers/RBFBehaviorWriter.h"
%include "dna/layers/TwistSwingBehaviorWriter.h"
%include "dna/layers/GeometryWriter.h"
%include "dna/Writer.h"
%include "dna/StreamWriter.h"
%include "dna/BinaryStreamWriter.h"
pythonize_unmanaged_type(BinaryStreamWriter, create, destroy)
#ifdef DNA_BUILD_WITH_JSON_SUPPORT
    %include "dna/JSONStreamWriter.h"
    pythonize_unmanaged_type(JSONStreamWriter, create, destroy)
#endif  // DNA_BUILD_WITH_JSON_SUPPORT

%include "dna/version/VersionInfo.h"
