// Travis Whitaker 2013
// twhitak@its.jnj.com

#ifndef INTERFACE_H
#define INTERFACE_H

// Define structure to hold one field's worth of data for buffering:
typedef struct
{
	char data[MAX_FIELD_LEN + 1]; // One field or cell's worth of data.
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
	column_type_t type; // Column type.
	char columnName[MAX_COLUMN_NAME_LEN + 1]; // Name of the column.
	char FKtarget[MAX_TABLE_NAME_LEN + 1]; // If the column is an FK, this contains the name of the target. Null otherwise.
	field_t defaultValue; // Default column value if applicable.
} column_t;

// Define structure to hold table-specific information:
typedef struct
{
	char tableName[MAX_TABLE_NAME_LEN + 1]; // Name of the table.
	int totalColumns; // Total number of columns in the table.
	column_t columns[MAX_COLUMNS]; // List of columns in the table.
	int primaryIdentifier; // Index in columns[] of the table's primary identifier. If negative, table has no primary identifier.
} table_t;

int getTableName(char *inputfilename, char *outputfilename, table_t *table);
int checkEmpty(char *inputfile_map);
int getColumnNames(char *inputfile_map, table_t *table);
int getTableMetadata(char *schemafile_map, table_t *table);
void outputHeader(FILE *outputfile, table_t *table);
void outputTriples( FILE* outputfile, char* inputfile_map, table_t* table, char* row_buffer );

#endif
