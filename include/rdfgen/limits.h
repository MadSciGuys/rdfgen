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

/*
 * DON'T try to be clever and large powers of two here with the hope of
 * optimizing MMU instructions. The allocator will do this for you if it makes
 * sense. These limits are intended to be used for data-curation purposes.
 */

// Define parameters of the output extension:
#define RDF_EXT ".rdf"
#define RDF_EXT_LEN 4

// Define the max file name length:
#define MAX_FILE_NAME_LEN 256

// Define the max lengths of table names and column names:
#define MAX_TABLE_NAME_LEN 30
#define MAX_COLUMN_NAME_LEN 30

// Define the max number of columns and max length of data fields:
#define MAX_COLUMNS 1000
#define MAX_FIELD_LEN 20000

// Define the flags used with the open() system call:
#define O_READ_FLAGS O_RDONLY

#endif
