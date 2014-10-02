%module pyQClassify
%include <stl.i>
%include <typemaps.i>
%include <exception.i>
%include <std_string.i>
%include <std_wstring.i>

%{ 
    #define SWIG_FILE_WITH_INIT
    #include "pyQClassify.hpp"    
%}

%exception {
    try {
        $action
    } 
    catch (QClassifyError &e) {
        SWIG_Python_Raise(SWIG_NewPointerObj(
                (new QClassifyError(static_cast<const QClassifyError& >(e))),  
                SWIGTYPE_p_QClassifyError,SWIG_POINTER_OWN),
            "QClassifyError", SWIGTYPE_p_QClassifyError); 
        SWIG_fail;
    } 
}

%include "pyQClassify.hpp"
