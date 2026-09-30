#include "ModelicaUtilities.h"
#include <stdio.h>
#include <stdarg.h>

//void ModelicaMessage(const char *string) {}
//
//void ModelicaFormatMessage(const char *format, ...) {}
//
//void ModelicaVFormatMessage(const char *format, va_list args) {}

void ModelicaError(const char *string) {
	printf(string);
}

//void ModelicaWarning(const char *string) {}
//
//void ModelicaFormatWarning(const char *format, ...) MODELICA_FORMATATTR_PRINTF;
//
//void ModelicaVFormatWarning(const char *format, va_list args) MODELICA_FORMATATTR_VPRINTF;

void ModelicaFormatError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

//MODELICA_NORETURN void ModelicaVFormatError(const char *format, va_list args) MODELICA_FORMATATTR_VPRINTF;
//
//char* ModelicaAllocateString(size_t len);
//
//char* ModelicaAllocateStringWithErrorReturn(size_t len);
//
//char* ModelicaDuplicateString(const char* str);
//
//char* ModelicaDuplicateStringWithErrorReturn(const char* str);
