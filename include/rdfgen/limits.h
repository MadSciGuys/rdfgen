// Travis Whitaker 2013
// twhitak@its.jnj.com

#ifndef LIMITS_H
#define LIMITS_H

/*
 * Here we define program-wide limits that vary according to the environment
 * and application. We've chosen default parameters for processing CSV files
 * sourced from Oracle tables on a UNIX-like machine. Note that these are
 * compiler macros, NOT variables. They are simply found and replaced in the
 * source code text at compile time. These can be easily identified in the
 * source by their capitalization and use of underscores. All field lengths
 * are in bytes and disregarding the terminating null byte(where applicable).
 */

// Define parameters of the output extension:
#define RDF_EXT ".rdf"
#define RDF_EXT_LEN 4

// Define the max file name length:
#define MAX_FILE_NAME_LEN 256

// Define the max lengths of table names and column names:
#define MAX_TABLE_NAME_LEN 32
#define MAX_COLUMN_NAME_LEN 32

// Define the max number of columns and max length of data fields:
#define MAX_COLUMNS 1024
#define MAX_FIELD_LEN 32768

// Define the flags used with the open() system call:
#define O_READ_FLAGS O_RDONLY

#endif
