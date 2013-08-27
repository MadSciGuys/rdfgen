// Travis Whitaker 2013
// twhitak@its.jnj.com

#ifndef RDFGEN_H
#define RDFGEN_H

// Here we define program-wide parameters that vary according to the environment
// and application. We've chosen default parameters for processing CSV files
// sourced from Oracle tables on a UNIX-like machine.Note that these are
// compiler macros, NOT variables. They are simply found and replaced in the
// source code text at compile time. These can be easily identified in the
// source by their capitalization and use of underscores. All field lengths
// are in bytes and disregarding the terminating null byte.

// Define booleans:

#define TRUE 0xff
#define FALSE 0x00

// Define parameters of the output extension:

#define RDF_EXT ".rdf"
#define RDF_EXT_LEN 4

// Define the max file name length, this will be OS-specific:

#define MAX_FILE_NAME_LEN 256

// Define the max lengths of table names and column names:

#define MAX_TABLE_NAME_LEN 30
#define MAX_COLUMN_NAME_LEN 30

// Define the max number of columns and max length of data fields

#define MAX_COLUMNS 1000
#define MAX_FIELD_LEN 4000

// Define the flags used with the open() system call:
#define O_READ_FLAGS O_RDONLY



// Here we define the data structures that are specific to rdfgen.

// Define structure to hold column-specific information:

typedef struct
{
	char columnName[MAX_COLUMN_NAME_LEN+1]; // Name of the column.
	char FKtarget[MAX_TABLE_NAME_LEN+1]; // If the column is an FK, this contains the name of the target. Null otherwise.
} column_t;

// Define structure to hold table-specific information:

typedef struct
{
	char tableName[MAX_TABLE_NAME_LEN+1]; // Name of the table.
	int totalColumns; // Total number of columns in the table.
	column_t columns[MAX_COLUMNS]; // List of columns in the table.
	int primaryIdentifier; // Index in columns[] of the table's primary identifier. If negative, table has no primary identifier.
	char foreignIdentifier[MAX_TABLE_NAME_LEN+1]; // If the primary identifier is a foreign key, this contains the name of the target. Null otherwise.
	char isLeaf; // True if the table has no foreign keys.
} table_t;

// Define structure to hold one field's worth of data for buffering:

typedef struct
{
	char data[MAX_FIELD_LEN+1]; // One field or cell's worth of data.
} field_t;

// Function declarations:
int getTableName(char *inputfilename, char *outputfilename, table_t *table);


#endif
