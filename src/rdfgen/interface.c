// Travis Whitaker 2013
// twhitak@its.jnj.com

// Standard I/O functions:
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <rdfgen/limits.h>
#include <rdfgen/generator.h>
#include <rdfgen/parser.h>
#include <rdfgen/interface.h>



// This function gets the table's name from the input file name, puts it into
// the table metadata struct, then computes the output file name. Returns -1
// on error, 0 on success.
int getTableName(char *inputfilename, char *outputfilename, table_t *table)
{
	// Iterate over the file name to get the table name:
	for(int i = 0; i < MAX_TABLE_NAME_LEN + 1; i++)
	{
		if(*(inputfilename + i) == '.') // Beginning of extension.
		{
			table->tableName[i] = '\0';
			break;
		}
		else // Otherwise copy the character.
		{
			table->tableName[i] = *(inputfilename + i);
		}
	}
	// If the last char isn't null, then the table name was too long:
	if(table->tableName[MAX_TABLE_NAME_LEN] != '\0')
	{
		printf("File name error!\nThe table name in file %s is too long.\n", inputfilename);
		return -1;
	}
	// Now make the output file name:
	for(int i = 0; i < MAX_TABLE_NAME_LEN + 1; i++)
	{
		if(table->tableName[i] == '\0') // Now write the extension.
		{
			snprintf((outputfilename + i), RDF_EXT_LEN + 1, RDF_EXT);
			break;
		}
		else // Otherwise copy the character.
		{
			*(outputfilename + i) = table->tableName[i];
		}
	}
	return 0;
}

// This function checks to make sure the input file isn't empty. Return 0 if
// the file is not empty, return 1 if it is, return -1 on fatal error.
int checkEmpty(char *inputfile_map)
{
	int cursor;
	// First seek to the end of the first line:
	for(cursor = 0; cursor < ((MAX_COLUMN_NAME_LEN + 1) * MAX_COLUMNS); cursor++)
	{
		if(*(inputfile_map + cursor) == '\n')
		{
			break;
		}
	}
	// Make sure we didn't fall out of the loop:
	if(*(inputfile_map + cursor) != '\n')
	{
		printf("Input file error!\nFirst line is too long!\n");
		return -1;
	}
	cursor++;
	if(*(inputfile_map + cursor) == '\0')
	{
		return 1;
	}
	return 0;
}

// This function gets the names of the columns from the first line of the input
// file and populates the table metadata. Returns 1 on error, 0 on success.
int getColumnNames(char *inputfile_map, table_t *table)
{
	int cursor = 0;
	int column;
	for(column = 0; column < MAX_COLUMNS; column++)
	{
		if(*(inputfile_map + cursor) == ',')
		{
			printf("Input file error!\nName of column %d in table %s is blank.\n", (column +1), table->tableName);
			return 1;
		}
		else
		{
			for(int i = 0; i < MAX_COLUMN_NAME_LEN + 1; i++)
			{
				if(*(inputfile_map + cursor) == ',' || *(inputfile_map + cursor) == '\n')
				{
					table->columns[column].columnName[i] = '\0';
					table->columns[column].type = real;
					break;
				}
				else
				{
					table->columns[column].columnName[i] = *(inputfile_map + cursor);
					cursor++;
				}
			}
			if(*(inputfile_map + cursor) == '\n')
			{
				if(table->columns[column].columnName[0] == '\0')
				{
					printf("Input file error!\nName of column %d in table %s is blank.\n", (column +1), table->tableName);
					return 1;
				}
				break;
			}
			if(table->columns[column].columnName[MAX_COLUMNS] != '\0')
			{
				printf("Input file error!\nName of column %d in table %s is too long.\n", (column + 1), table->tableName);
				return 1;
			}
			cursor++;
		}
	}
	column++;
	table->totalColumns = column;
	return 0;
}

// This function gets the table metadata from the schema file. Return 1 on
// error, relying on other functions to report the specific error:
int getTableMetadata(char *schemafile_map, table_t *table)
{
	int cursor; // Keep track of our place between function calls.
	char op; // Keep track of the operator for this line.
	char arg1[MAX_COLUMN_NAME_LEN + 1]; // First argument is always a column name.
	char arg2[MAX_FIELD_LEN + 1]; // Second argument may be a column name or default field value.

	// First, seek to the correct spot in the file:
	if(schemaSeek(schemafile_map, table->tableName, &cursor) == 1)
	{
		return 1;
	}
	// The primary identifier must come next:
	if(schemaPI(schemafile_map, table, &cursor) == 1)
	{
		printf("Parsing error under table %s\n", table->tableName);
		return 1;
	}
	// Process each following line in turn:
	while(op != '#')
	{
		// Clean up after the previous iteration:
		memset(arg1, '\0', MAX_COLUMN_NAME_LEN + 1);
		memset(arg2, '\0', MAX_FIELD_LEN + 1);
		op = '\0';
		// Fetch the next line:
		if(schemaFetchLine(schemafile_map, &op, arg1, arg2, &cursor) == 1)
		{
			return 1;
		}
		// Handle the op:
		switch(op)
		{
		case '>':
			if(renameColumn(arg1, arg2, table) == 1)
			{
				return 1;
			}
			break;
		case '?':
			if(defineDV(arg1, arg2, table) == 1)
			{
				return 1;
			}
			break;
		case '*':
			if(requireColumn(arg1, table) == 1)
			{
				return 1;
			}
			break;
		case '@':
			if(defineFK(arg1, arg2, table) == 1)
			{
				return 1;
			}
			break;
		case '&':
			if(defineVC(arg1, arg2, table) == 1)
			{
				return 1;
			}
			break;
		case '#':
			break;
		default:
			printf("INTERNAL EXECUTION STATE ERROR!\nSomething really bad happened in the function getTableMetadata()!\nInvalid value of switch char operator at location 0x%x!\n", (unsigned int)&op);
			return 1;
			break;
		}
	}
	return 0;
}

// This function outputs the RDF header. If we've gotten this far, the only
// possible errors will occur at the kernel level, or so we hope...
void outputHeader(FILE *outputfile, table_t *table)
{
	fprintf(outputfile, XML_VERSION);
	fprintf(outputfile, "\n");
	fprintf(outputfile, DOCTYPE);
	fprintf(outputfile, "\n");
	fprintf(outputfile, RDF_NAMESPACES);
	fprintf(outputfile, "\n\n");

	fprintf(outputfile, "<rdfs:Class rdf:ID=\"%s\"/>\n\n", table->tableName);

	for(int i = 0; i < table->totalColumns; i++)
	{
		if(i == table->primaryIdentifier)
		{
			// If the PI is not an FK, it is a component of the node name and is therefore NOT a property.
			if(table->columns[i].FKtarget[0] == '\0')
			{
				continue;
			}
			// Otherwise, it is a protery of the FK target referring to the current table.
			else
			{
				fprintf(outputfile, "<rdf:Property rdf:ID=\"%s_%s\">\n  <rdfs:domain rdf:resource=\"#%s\"/>\n  <rdfs:range rdf:resource=\"#%s\"/>\n</rdf:Property>\n\n", table->tableName, table->columns[i].columnName, table->columns[i].FKtarget, table->tableName);
			}
		}
		else
		{
			fprintf(outputfile, "<rdf:Property rdf:ID=\"%s_%s\">\n  <rdfs:domain rdf:resource=\"%s\"/>\n</rdf:Property>\n\n", table->tableName, table->columns[i].columnName, table->tableName);
		}
	}
	fprintf(outputfile, "\n\n");
	return;
}
