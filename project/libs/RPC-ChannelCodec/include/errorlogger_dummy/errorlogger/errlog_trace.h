/*
 * trace.h
 *
 *  Created on: 08.01.2016
 *      Author: ak
 */

#ifndef MODULES_ERRORLOGGER_INCLUDE_ERRORLOGGER_TRACE_H_
#define MODULES_ERRORLOGGER_INCLUDE_ERRORLOGGER_TRACE_H_

/**
 *  Outputs a formatted string using 'printf' if the log level is high
 *  enough. Can be disabled by defining TRACE_LEVEL=0 during compilation.
 *  \param ...  Additional parameters depending on formatted string.
 */
#include <stdio.h>

#define DYN_TRACES  0

#define TRACE_LEVEL_DEBUG      5
#define TRACE_LEVEL_INFO       4
#define TRACE_LEVEL_WARNING    3
#define TRACE_LEVEL_ERROR      2
#define TRACE_LEVEL_FATAL      1
#define TRACE_LEVEL_NO_TRACE   0


/* Trace output depends on dwTraceLevel value */
#define TRACE_DEBUG(...)      { printf("-D- " __VA_ARGS__); } 
#define TRACE_INFO(...)       { printf("-I- " __VA_ARGS__); } 
#define TRACE_WARNING(...)    { printf("-W- " __VA_ARGS__); } 
#define TRACE_ERROR(...)      { printf("-E- " __VA_ARGS__); } 
#define TRACE_FATAL(...)      { printf("-F- " __VA_ARGS__); while(1); } 




#endif /* MODULES_ERRORLOGGER_INCLUDE_ERRORLOGGER_TRACE_H_ */
