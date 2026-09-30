// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// DLL_ATTRIBUTE_IS_VISIBILITY distinguishes the two export mechanisms: a visibility attribute, which
// merely widens the linkage of something the translation unit already emits, and dllexport/dllimport,
// which names an entity in the import/export table. They attach to opposite halves of an explicit
// template instantiation, so the mechanism has to be visible to the macros below.
#if defined(FORCE_DLL_ATTRIBUTE_EXPORT_IMPORT)
    #define DLL_EXPORT __attribute__((dllexport))
    #define DLL_IMPORT __attribute__((dllimport))
    #define DLL_ATTRIBUTE_IS_VISIBILITY 0
    #define DLL_ATTRIBUTE_ON_TYPE 1
    #define DLL_ATTRIBUTE_ON_MEMBER 0
#elif defined(FORCE_DLL_DECLSPEC_EXPORT_IMPORT)
    #define DLL_EXPORT __declspec(dllexport)
    #define DLL_IMPORT __declspec(dllimport)
    #define DLL_ATTRIBUTE_IS_VISIBILITY 0
    #if defined(_MSC_VER)
        #define DLL_ATTRIBUTE_ON_TYPE 0
        #define DLL_ATTRIBUTE_ON_MEMBER 1
    #else
        #define DLL_ATTRIBUTE_ON_TYPE 1
        #define DLL_ATTRIBUTE_ON_MEMBER 0
    #endif
#elif defined(FORCE_DLL_VISIBILITY_EXPORT_IMPORT)
    #define DLL_EXPORT __attribute__((visibility("default")))
    #define DLL_IMPORT DLL_EXPORT
    #define DLL_ATTRIBUTE_IS_VISIBILITY 1
    #define DLL_ATTRIBUTE_ON_TYPE 1
    #define DLL_ATTRIBUTE_ON_MEMBER 1
#else
    #if defined(_WIN32) || defined(__CYGWIN__)
        #if defined(__GNUC__)
            #define DLL_EXPORT __attribute__((dllexport))
            #define DLL_IMPORT __attribute__((dllimport))
            #define DLL_ATTRIBUTE_IS_VISIBILITY 0
            #define DLL_ATTRIBUTE_ON_TYPE 1
            #define DLL_ATTRIBUTE_ON_MEMBER 0
        #else
            #define DLL_EXPORT __declspec(dllexport)
            #define DLL_IMPORT __declspec(dllimport)
            #define DLL_ATTRIBUTE_IS_VISIBILITY 0
            #define DLL_ATTRIBUTE_ON_TYPE 0
            #define DLL_ATTRIBUTE_ON_MEMBER 1
        #endif
    #elif defined(__GNUC__)
        #define DLL_EXPORT __attribute__((visibility("default")))
        #define DLL_IMPORT DLL_EXPORT
        #define DLL_ATTRIBUTE_IS_VISIBILITY 1
        #define DLL_ATTRIBUTE_ON_TYPE 1
        #define DLL_ATTRIBUTE_ON_MEMBER 1
    #else
        #define DLL_EXPORT
        #define DLL_IMPORT
        #define DLL_ATTRIBUTE_IS_VISIBILITY 0
        #define DLL_ATTRIBUTE_ON_TYPE 0
        #define DLL_ATTRIBUTE_ON_MEMBER 0
    #endif
#endif

#if defined(RL_BUILD_SHARED)
    // Build shared library
    #define SCAPI DLL_EXPORT
#elif defined(RL_SHARED)
    // Use shared library
    #define SCAPI DLL_IMPORT
#else
    // Build or use static library
    #define SCAPI
#endif

// A class definition and its members, for a class whose vtable and typeinfo have to cross the
// boundary. Spell both halves; whichever one would conflict on the target ABI expands to nothing.
//   class SCAPI_TYPE TClass {
//       SCAPI_MEMBER void method();
//   };
#if DLL_ATTRIBUTE_ON_TYPE
    #define SCAPI_TYPE SCAPI
#else
    #define SCAPI_TYPE
#endif

#if DLL_ATTRIBUTE_ON_MEMBER
    #define SCAPI_MEMBER SCAPI
#else
    #define SCAPI_MEMBER
#endif

// The two halves of an explicit class-template instantiation. Spell both; the mechanism decides which
// one carries the attribute. A visibility attribute goes on the declaration, since that is what tells
// consumers the instantiation is external and default-visible, and on the definition it would come too
// late. dllexport goes on the definition, and is ill-formed on a declaration. dllimport is legal on the
// declaration and is what a consumer needs, so the declaration carries it when consuming a shared
// library. The instantiated template's members must stay unannotated: their own attributes cannot widen
// an instantiation the arguments' visibility has already made hidden, and under dllexport they collide.
//   extern template class SCAPI_TEMPLATE_DECL TClass<...>;  // header
//   template class SCAPI_TEMPLATE_DEFN TClass<...>;        // one .cpp
#if DLL_ATTRIBUTE_IS_VISIBILITY || (defined(RL_SHARED) && !defined(RL_BUILD_SHARED))
    #define SCAPI_TEMPLATE_DECL SCAPI
#else
    #define SCAPI_TEMPLATE_DECL
#endif

#if DLL_ATTRIBUTE_IS_VISIBILITY
    #define SCAPI_TEMPLATE_DEFN
#else
    #define SCAPI_TEMPLATE_DEFN SCAPI
#endif
