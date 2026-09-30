/**
 * Copyright Epic Games, Inc. All Rights Reserved.
 */

// SWIG maps None to a null const char* and the wrapped C++ APIs take non-null strings, so the call would dereference null.
%typemap(check) const char* {
    if ($1 == NULL) {
        SWIG_exception_fail(SWIG_TypeError, "in method '$symname', argument $argnum of type '$1_type' must not be None");
    }
}
