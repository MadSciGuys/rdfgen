// Travis Whitaker 2013
// twhitak@its.jnj.com

#ifndef STRUCTURE_H
#define STRUCTURE_H

#include <rdfgen/limits.h>



// Define structure to hold one field's worth of data:
typedef struct
{
	char data[MAX_FIELD_LEN + 1];
} field_t;

// Define enum for column type:
typedef enum
{
	real,
	req,
	virt
} column_type_t;

// Define structure to hold column-specific information:
typedef struct
{
	column_type_t type;
	field_t defaultValue;
	char columnName[MAX_COLUMN_NAME_LEN + 1];
	char FKtarget[MAX_TABLE_NAME_LEN + 1];
} column_t;

// Define structure to hold table-specific information:
typedef struct
{
	column_t columns[MAX_COLUMNS];
	int totalColumns;
	int primaryIdentifier;
	char tableName[MAX_TABLE_NAME_LEN + 1];
} table_t;

#endif
