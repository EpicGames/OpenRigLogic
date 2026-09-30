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

%define RL_MODULEIMPORT
"
import os
import sys

module_loader_cls = None
for cls in sys.meta_path:
    if hasattr(cls, '__name__') and cls.__name__ == 'IsolatedModuleLoader':
        module_loader_cls = cls
        break

if module_loader_cls is None:
    import dna
    import $module
else:
    dna_rootdir = module_loader_cls.get_module_rootdir('dna') or os.path.dirname(os.path.abspath(__file__))
    dna = module_loader_cls.load_module('dna', rootdir=dna_rootdir)
    $module = module_loader_cls.load_module('$module', rootdir=os.path.dirname(os.path.abspath(__file__)))
"
%enddef

%module(moduleimport=RL_MODULEIMPORT) riglogic

%include <exception.i>
%include <stdint.i>

%include <spyus/Caster.i>
%include <spyus/ArrayView.i>
%include <spyus/Vector3.i>

%include <spyus/ExceptionHandling.i>

%pythoncode %{
if module_loader_cls is not None:
    global_dna = sys.modules.pop('dna', None)
    global_pydna = sys.modules.pop('_py3dna', None)
    local_dna = dna
%}
%import "DNA.i"
%pythoncode %{
if module_loader_cls is not None:
    if '_py3dna' in sys.modules:
        del sys.modules['_py3dna']
    if global_pydna is not None:
        sys.modules['_py3dna'] = global_pydna

    if 'dna' in sys.modules:
        del sys.modules['dna']
    if global_dna is not None:
        sys.modules['dna'] = global_dna

    dna = local_dna
%}

%{
#include "riglogic/Defs.h"
#include "riglogic/types/Aliases.h"
#include "riglogic/version/VersionInfo.h"
#include "riglogic/riglogic/Configuration.h"
#include "riglogic/riglogic/Stats.h"
#include "riglogic/riglogic/RigInstance.h"
#include "riglogic/riglogic/RigLogic.h"
%}

%ignore rl4::RigLogic::restore;
%ignore rl4::RigLogic::dump;
%rename(calculateJointGroup) calculateJoints(RigInstance*, std::uint16_t) const;
%rename(calculateMLOperation) calculateMLControls(RigInstance*, std::uint16_t, std::uint16_t, std::uint16_t) const;
%rename(calculateRBFSolver) calculateRBFControls(RigInstance*, std::uint16_t) const;

py_list_to_array_view(rl4::ConstArrayView<float>, SWIG_TYPECHECK_FLOAT_ARRAY)
py_list_to_array_view(rl4::ConstArrayView<std::uint16_t>, SWIG_TYPECHECK_INT16_ARRAY)
py_list_to_array_view(rl4::ConstArrayView<std::uint32_t>, SWIG_TYPECHECK_INT32_ARRAY)

%ignore av::operator==;
%ignore av::operator!=;

array_view_to_py_list(av::ArrayView);
%apply av::ArrayView {
    av::ConstArrayView,
    rl4::ArrayView,
    rl4::ConstArrayView
};

string_view_to_py_string(av::StringView);
%apply av::StringView {
    rl4::StringView
};

vector3_typemap(rl4::Vector3);

py_list_to_c_pointer(const float* values);

%include "riglogic/Defs.h"
%include "riglogic/types/Aliases.h"
%include "riglogic/version/VersionInfo.h"
%include "riglogic/riglogic/Configuration.h"
%include "riglogic/riglogic/Stats.h"
%include "riglogic/riglogic/RigInstance.h"
pythonize_unmanaged_type(RigInstance, create, destroy)
%include "riglogic/riglogic/RigLogic.h"
pythonize_unmanaged_type(RigLogic, create, destroy)

%runtime %{
#include <cstring>
#include <string>

#define SWIGPY_MODULE_NAME ("swig_runtime_data" SWIG_RUNTIME_VERSION)
#define SWIGPY_MODULE_ATTR_PREFIX "type_pointer_capsule"

static swig_module_info* GetSwigModule(const char* type_table_name) {
    std::string name;
    name.insert(0, SWIGPY_MODULE_ATTR_PREFIX, sizeof(SWIGPY_MODULE_ATTR_PREFIX) - 1);
    name.insert(name.size(), type_table_name, std::strlen(type_table_name));

    PyObject* mod = PyImport_ImportModule(SWIGPY_MODULE_NAME);
    if (mod == nullptr) {
        PyErr_Clear();
        return nullptr;
    }

    PyObject* capsule = PyObject_GetAttrString(mod, name.data());
    if (capsule == nullptr) {
        PyErr_Clear();
        return nullptr;
    }

    name.insert(0, ".", 1);
    name.insert(0, SWIGPY_MODULE_NAME, sizeof(SWIGPY_MODULE_NAME) - 1);

    void* type_pointer = PyCapsule_GetPointer(capsule, name.data());
    if (PyErr_Occurred()) {
        PyErr_Clear();
        type_pointer = nullptr;
    }
    return static_cast<swig_module_info*>(type_pointer);
}

SWIGRUNTIME swig_module_info* SWIG_Python_GetModule_Patched(void *SWIGUNUSEDPARM(clientdata)) {
    swig_module_info* module = GetSwigModule(SWIG_EXPAND_AND_QUOTE_STRING(PYDNA_SWIG_TYPE_TABLE));
    return module;
}

#undef SWIG_GetModule
#define SWIG_GetModule(clientdata) SWIG_Python_GetModule_Patched(clientdata)
%}

%init %{
    // SWIG_InitializeModule publishes this module's own type-table capsule only when it is the first
    // SWIG module in the interpreter; the patched SWIG_GetModule always finds py3dna first, so the
    // stock SWIG_Python_TypeQuery (compiled before the patch) would find no capsule and dereference
    // null, e.g. on any wrong-type argument to a const char* parameter.
    SWIG_Python_SetModule(&swig_module);
%}
