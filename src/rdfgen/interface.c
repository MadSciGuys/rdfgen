// Travis Whitaker 2013
// twhitak@its.jnj.com

// Standard I/O functions:
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <rdfgen/limits.h>
#include <rdfgen/parser.h>
#include <rdfgen/interface.h>
#include <rdfgen/structure.h>
#include <rdfgen/generator.h>

#ifndef NO_COLOR
#include <rdfgen/color.h>
#endif

// This function gets the table's name from the input file name, puts it into
// the table metadata struct, then computes the output file name. Returns -1
// on error, 0 on success:
int getTableName(char *inputfilename, char *outputfilename, table_t *table)
{
	for(unsigned int i = 0; i < MAX_TABLE_NAME_LEN + 1; i++)
	{
		if(*(inputfilename + i) == '.' || *(inputfilename + i) == '\0')
		{
			table->tableName[i] = '\0';
			break;
		}
		else
		{
			table->tableName[i] = *(inputfilename + i);
		}
	}
	if(table->tableName[MAX_TABLE_NAME_LEN] != '\0')
	{
#ifdef NO_COLOR
		printf("File name error!\nThe table name in file %s is too long.\n", inputfilename);
#else
		printf(BOLD RED "File name error!\nThe table name in file %s is too long.\n" RESET, inputfilename);
#endif
		return -1;
	}
	for(unsigned int i = 0; i < MAX_TABLE_NAME_LEN + 1; i++)
	{
		if(table->tableName[i] == '\0')
		{
			snprintf((outputfilename + i), RDF_EXT_LEN + 1, RDF_EXT);
			break;
		}
		else
		{
			*(outputfilename + i) = table->tableName[i];
		}
	}
	return 0;
}

// This function checks to make sure the input file isn't empty. Return 0 if
// the file is not empty, return 1 if it is, return -1 on fatal error:
int checkEmpty(char *inputfile_map)
{
	unsigned long int cursor;
	// First seek to the end of the first line:
	for(cursor = 0; cursor < ((MAX_COLUMN_NAME_LEN + 1) * MAX_COLUMNS); cursor++)
	{
		if(*(inputfile_map + cursor) == '\n')
		{
			break;
		}
	}
	if(*(inputfile_map + cursor) != '\n')
	{
#ifdef NO_COLOR
		printf("Input file error!\nFirst line is too long!\n");
#else
		printf(BOLD RED "Input file error!\nFirst line is too long!\n" RESET);
#endif
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
// file and populates the table metadata. Returns 1 on error, 0 on success:
int getColumnNames(char *inputfile_map, table_t *table)
{
	unsigned long int cursor = 0;
	unsigned int column;
	for(column = 0; column < MAX_COLUMNS; column++)
	{
		if(*(inputfile_map + cursor) == ',')
		{
#ifdef NO_COLOR
			printf("Input file error!\nName of column %d in table %s is blank.\n", (column +1), table->tableName);
#else
			printf(BOLD RED "Input file error!\nName of column %d in table %s is blank.\n" RESET, (column +1), table->tableName);
#endif
			return 1;
		}
		else
		{
			for(unsigned int i = 0; i < MAX_COLUMN_NAME_LEN + 1; i++)
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
#ifdef NO_COLOR
					printf("Input file error!\nName of column %d in table %s is blank.\n", (column +1), table->tableName);
#else
					printf(BOLD RED "Input file error!\nName of column %d in table %s is blank.\n" RESET, (column +1), table->tableName);
#endif
					return 1;
				}
				break;
			}
			if(table->columns[column].columnName[MAX_COLUMN_NAME_LEN + 1] != '\0')
			{
#ifdef NO_COLOR
				printf("Input file error!\nName of column %d in table %s is too long.\n", (column + 1), table->tableName);
#else
				printf(BOLD RED "Input file error!\nName of column %d in table %s is too long.\n" RESET, (column + 1), table->tableName);
#endif
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
	unsigned long int cursor = 0; // Keep track of our place between function calls.
	char op = '\0'; // Keep track of the operator for this line.
	char arg1[MAX_COLUMN_NAME_LEN + 1]; // First argument is always a column name.
	char arg2[MAX_FIELD_LEN + 1]; // Second argument may be a column name or default field value.

	if(schemaSeek(schemafile_map, table->tableName, &cursor) == 1)
	{
		return 1;
	}
	if(schemaPI(schemafile_map, table, &cursor) == 1)
	{
#ifdef NO_COLOR
		printf("Parsing error under table %s\n", table->tableName);
#else
		printf(BOLD RED "Parsing error under table %s\n" RESET, table->tableName);
#endif
		return 1;
	}
	while(op != '#')
	{
		memset(arg1, '\0', MAX_COLUMN_NAME_LEN + 1);
		memset(arg2, '\0', MAX_FIELD_LEN + 1);
		op = '\0';
		if(schemaFetchLine(schemafile_map, &op, arg1, arg2, &cursor) == 1)
		{
			return 1;
		}
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
#ifdef NO_COLOR
			printf("INTERNAL EXECUTION STATE ERROR!\nSomething really bad happened in the function getTableMetadata()!\nInvalid value of switch char operator at location 0x%x!\n", (unsigned int)&op);
#else
			printf(BOLD MAGENTA REVERSE BLINK "INTERNAL EXECUTION STATE ERROR!\n" RED "Something really bad happened in the function getTableMetadata()!" YELLOW "\nInvalid value of switch char operator at location 0x%x!\n" RESET, (unsigned int)&op);
#endif
			return 1;
			break;
		}
	}
	return 0;
}

void printTableMetadata(table_t *table)
{
	printf("%s, %d columns\n", table->tableName, table->totalColumns);
	if(table->primaryIdentifier == -1)
	{
#ifdef NO_COLOR
		printf("No Primary Identifier\n");
#else
		printf(BOLD CYAN "No Primary Identifier\n" RESET);
#endif
	}
	else
	{
#ifdef NO_COLOR
		printf("Primary Identifier: %s\n", table->columns[table->primaryIdentifier].columnName);
#else
		printf("Primary Identifier: " BOLD BLUE "%s\n" RESET, table->columns[table->primaryIdentifier].columnName);
#endif
	}
	printf("Columns:\n");
	for(int i = 0; i < table->totalColumns; i++)
	{
		printf("%4d: %-32s", i + 1, table->columns[i].columnName);
		switch(table->columns[i].type)
		{
		case real:
			printf("%-12s", "real");
			break;
		case req:
#ifdef NO_COLOR
			printf("%-12s", "required");
#else
			printf(BOLD RED "%-12s" RESET, "required");
#endif
			break;
		case virt:
#ifdef NO_COLOR
			printf("%-12s", "virtual");
#else
			printf(BOLD MAGENTA "%-12s" RESET, "virtual");
#endif
			break;
		default:
#ifdef NO_COLOR
			printf("%-12s", "!!!!!!");
#else
			printf(BOLD RED REVERSE BLINK "%-12s" RESET, "!!!!!!");
#endif
			break;
		}
		switch(table->columns[i].FKtarget[0])
		{
		case '\0':
#ifdef NO_COLOR
			printf("%-38s", "independent");
#else
			printf(BOLD GREEN "%-38s" RESET, "independent");
#endif
			break;
		default:
#ifdef NO_COLOR
			printf("->%-36s", table->columns[i].FKtarget);
#else
			printf(BOLD YELLOW "->%-36s" RESET, table->columns[i].FKtarget);
#endif
			break;
		}
		switch(table->columns[i].defaultValue.data[0])
		{
		case '\0':
#ifdef NO_COLOR
			printf("no default value");
#else
			printf(BOLD GREEN "no default value" RESET);
#endif
			break;
		default:
#ifdef NO_COLOR
			printf("default value: %s", table->columns[i].defaultValue.data);
#else
			printf(BOLD MAGENTA "default value: %s" RESET, table->columns[i].defaultValue.data);
#endif
			break;
		}
		printf("\n");
	}
	return;
}

// This function outputs the RDF header. If we've gotten this far, the only
// possible errors will occur at the kernel level, or so we hope...
void outputHeader(FILE *outputfile, table_t *table, unsigned long int *triples)
{
	fprintf(outputfile, XML_VERSION);
	fprintf(outputfile, "\n");
	fprintf(outputfile, DOCTYPE);
	fprintf(outputfile, "\n");
	fprintf(outputfile, RDF_NAMESPACES);
	fprintf(outputfile, "\n\n");

	fprintf(outputfile, "<rdfs:Class rdf:ID=\"%s\"/>\n\n", table->tableName);
	(*triples)++;

	for(int i = 0; i < table->totalColumns; i++)
	{
		if(i == table->primaryIdentifier)
		{
			// If the PI is not an FK, it is a component of the node name and is therefore NOT a property:
			if(table->columns[i].FKtarget[0] == '\0')
			{
				continue;
			}
			// Otherwise, it is a protery of the FK target referring to the current table:
			else
			{
				fprintf(outputfile, "<rdf:Property rdf:ID=\"%s\">\n  <rdfs:domain rdf:resource=\"#%s\"/>\n  <rdfs:range rdf:resource=\"#%s\"/>\n</rdf:Property>\n\n", table->columns[i].columnName, table->columns[i].FKtarget, table->tableName);
				(*triples) += 3;
			}
		}
		else
		{
			if(table->columns[i].FKtarget[0] == '\0')
			{
				fprintf(outputfile, "<rdf:Property rdf:ID=\"%s\">\n  <rdfs:domain rdf:resource=\"%s\"/>\n</rdf:Property>\n\n", table->columns[i].columnName, table->tableName);
				(*triples) += 2;
			}
			else
			{
				fprintf(outputfile, "<rdf:Property rdf:ID=\"%s\">\n  <rdfs:domain rdf:resource=\"#%s\"/>\n  <rdfs:range rdf:resource=\"#%s\"/>\n</rdf:Property>\n\n", table->columns[i].columnName, table->tableName, table->columns[i].FKtarget);
				(*triples) += 3;
			}
		}
	}
	return;
}

// This function outputs the RDF triples. There should be no userspace errors...
void outputTriples(FILE *outputfile, char *inputfile_map, table_t *table, field_t *row_buffer, unsigned long int *triples)
{
	unsigned long int cursor;
	// Seek to the beginning of the first data line:
	for(cursor = 0; cursor < ((MAX_COLUMN_NAME_LEN + 1) * MAX_COLUMNS); cursor++)
	{
		if(*(inputfile_map + cursor) == '\n')
		{
			cursor++;
			break;
		}
	}
	// Huge nasty decision tree to determine which generating function to use:
	if(checkVirt(table) == 1)
	{
		if(checkReq(table) == 0)
		{
			if(table->primaryIdentifier == -1)
			{
				if(checkLeaf(table) == 1)
				{
					printf("Using genTriples_anon_leaf_no_virt()\n");
					genTriples_anon_leaf_no_virt(inputfile_map, &cursor, outputfile, row_buffer, table, triples);
				}
				else
				{
					printf("Using genTriples_anon_no_virt()\n");
					genTriples_anon_no_virt(inputfile_map, &cursor, outputfile, row_buffer, table, triples);
				}
			}
			else if(table->columns[table->primaryIdentifier].FKtarget[0] != '\0')
			{
				printf("Using genTriples_pifk_no_virt()\n");
				genTriples_pifk_no_virt(inputfile_map, &cursor, outputfile, row_buffer, table, triples);
			}
			else
			{
				if(checkLeaf(table) == 1)
				{
					printf("Using genTriples_leaf_no_virt()\n");
					genTriples_leaf_no_virt(inputfile_map, &cursor, outputfile, row_buffer, table, triples);
				}
				else
				{
					printf("Using genTriples_no_virt()\n");
					genTriples_no_virt(inputfile_map, &cursor, outputfile, row_buffer, table, triples);
				}
			}
		}
		else
		{
			if(table->primaryIdentifier == -1)
			{
				if(checkLeaf(table) == 1)
				{
					printf("Using genTriples_anon_leaf_no_virt_no_req()\n");
					genTriples_anon_leaf_no_virt_no_req(inputfile_map, &cursor, outputfile, row_buffer, table, triples);
				}
				else
				{
					printf("Using genTriples_anon_no_virt_no_req()\n");
					genTriples_anon_no_virt_no_req(inputfile_map, &cursor, outputfile, row_buffer, table, triples);
				}
			}
			else if(table->columns[table->primaryIdentifier].FKtarget[0] != '\0')
			{
				printf("Using genTriples_pifk_no_virt_no_req()\n");
				genTriples_pifk_no_virt_no_req(inputfile_map, &cursor, outputfile, row_buffer, table, triples);
			}
			else
			{
				if(checkLeaf(table) == 1)
				{
					printf("Using genTriples_leaf_no_virt_no_req()\n");
					genTriples_leaf_no_virt_no_req(inputfile_map, &cursor, outputfile, row_buffer, table, triples);
				}
				else
				{
					printf("Using genTriples_no_virt_no_req()\n");
					genTriples_no_virt_no_req(inputfile_map, &cursor, outputfile, row_buffer, table, triples);
				}
			}
		}
	}
	else
	{
		if(checkReq(table) == 0)
		{
			if(table->primaryIdentifier == -1)
			{
				if(checkLeaf(table) == 1)
				{
					printf("Using genTriples_anon_leaf()\n");
					genTriples_anon_leaf(inputfile_map, &cursor, outputfile, row_buffer, table, triples);
				}
				else
				{
					printf("Using genTriples_anon()\n");
					genTriples_anon(inputfile_map, &cursor, outputfile, row_buffer, table, triples);
				}
			}
			else if(table->columns[table->primaryIdentifier].FKtarget[0] != '\0')
			{
				printf("Using genTriples_pifk()\n");
				genTriples_pifk(inputfile_map, &cursor, outputfile, row_buffer, table, triples);
			}
			else
			{
				if(checkLeaf(table) == 1)
				{
					printf("Using genTriples_leaf()\n");
					genTriples_leaf(inputfile_map, &cursor, outputfile, row_buffer, table, triples);
				}
				else
				{
					printf("Using genTriples()\n");
					genTriples(inputfile_map, &cursor, outputfile, row_buffer, table, triples);
				}
			}
		}
		else
		{
			if(table->primaryIdentifier == -1)
			{
				if(checkLeaf(table) == 1)
				{
					printf("Using genTriples_anon_leaf_no_req()\n");
					genTriples_anon_leaf_no_req(inputfile_map, &cursor, outputfile, row_buffer, table, triples);
				}
				else
				{
					printf("Using genTriples_anon_no_req()\n");
					genTriples_anon_no_req(inputfile_map, &cursor, outputfile, row_buffer, table, triples);
				}
			}
			else if(table->columns[table->primaryIdentifier].FKtarget[0] != '\0')
			{
				printf("Using genTriples_pifk_no_req()\n");
				genTriples_pifk_no_req(inputfile_map, &cursor, outputfile, row_buffer, table, triples);
			}
			else
			{
				if(checkLeaf(table) == 1)
				{
					printf("Using genTriples_leaf_no_req()\n");
					genTriples_leaf_no_req(inputfile_map, &cursor, outputfile, row_buffer, table, triples);
				}
				else
				{
					printf("Using genTriples_no_req()\n");
					genTriples_no_req(inputfile_map, &cursor, outputfile, row_buffer, table, triples);
				}
			}
		}
	}
	fprintf(outputfile, "\n</rdf:RDF>");
	return;
}
