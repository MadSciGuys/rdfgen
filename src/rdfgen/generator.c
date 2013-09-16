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



// Return 1 if table has no FKs, 0 otherwise.
int checkLeaf(table_t *table)
{
	for(int i = 0; i < table->totalColumns; i++)
	{
		if(table->columns[i].FKtarget[0] != '\0')
		{
			return 0;
		}
	}
	return 1;
}

// Return 1 if table has no virtual columns, 0 othersize.
int checkVirt(table_t *table)
{
	for(int i = 0; i < table->totalColumns; i++)
	{
		if(table->columns[i].type == virt)
		{
			return 0;
		}
	}
	return 1;
}

// This function reads one row of a CSV file into a buffer. Return 1 if EOF is
// found, 0 otherwise. Undefined behavior if a cell if bigger than MAX_FIELD_LEN!
int readRow(char *inputfile_map, int *cursor, field_t *row_buffer)
{
	if(*(inputfile_map + *cursor) == '\0')
	{
		return 1;
	}
	int cell = 0;
	int _cursor = 0;
	for(unsigned int i = *cursor; i < (((MAX_FIELD_LEN + 1) * MAX_COLUMNS) + *cursor); i++)
	{
		if(*(inputfile_map + i) == '\n')
		{
			(row_buffer + cell)->data[_cursor] = '\0';
			i++;
			*cursor = i;
			return 0;
		}
		else if(*(inputfile_map + i) == '\0')
		{
			(row_buffer + cell)->data[_cursor] = '\0';
			*cursor = i;
			return 0;
		}
		else if(*(inputfile_map + i) == ',')
		{
			(row_buffer + cell)->data[_cursor] = '\0';
			_cursor = 0;
			cell++;
		}
		else
		{
			(row_buffer + cell)->data[_cursor] = *(inputfile_map + i);
			_cursor++;
		}
	}
	return 1;
}

// This function generates triples for an anonymous leaf table.
void genTriples_anon_leaf(char *inputfile_map, int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table)
{
	while(readRow(inputfile_map, cursor, row_buffer) != 1)
	{
		fprintf(outputfile,"<%s:%s>\n", PREFIX, table->tableName);
		for(int i = 0; i < table->totalColumns; i++)
		{
			// Is column virtual?
			if(table->columns[i].type == virt)
			{
				fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, table->columns[i].defaultValue.data, PREFIX, table->tableName, table->columns[i].columnName);
			}
			// If not, does it have a value?
			else if((row_buffer + i)->data[0] != '\0')
			{
				fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, (row_buffer + i)->data, PREFIX, table->tableName, table->columns[i].columnName);
			}
			// If not, must it?
			else if(table->columns[i].type == req)
			{
				// If it must, is there a default value?
				if(table->columns[i].defaultValue.data[0] != '\0')
				{
					fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, table->columns[i].defaultValue.data, PREFIX, table->tableName, table->columns[i].columnName);
				}
				// If not, warn the user.
				else
				{
					printf("Input file error!\nColumn %s in table %s is specified as required, but an empty cell was found.\nContinuing...\n", table->columns[i].columnName, table->tableName);
				}
			}
			// If not, write nil:
			else
			{
				fprintf(outputfile, "  <%s:%s_%s><rdf:nil/></%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, PREFIX, table->tableName, table->columns[i].columnName);
			}
		}
		fprintf(outputfile, "</%s:%s>\n", PREFIX, table->tableName);
		memset(row_buffer, '\0', MAX_COLUMNS * sizeof(*row_buffer));
	}
}

// This function generates triples for an anonymous table.
void genTriples_anon(char *inputfile_map, int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table)
{
	while(readRow(inputfile_map, cursor, row_buffer) != 1)
	{
		fprintf(outputfile,"<%s:%s>\n", PREFIX, table->tableName);
		for(int i = 0; i < table->totalColumns; i++)
		{
			// Is column virtual?
			if(table->columns[i].type == virt)
			{
				// Is it a foreign key?
				if(table->columns[i].FKtarget[0] != '\0')
				{
					fprintf(outputfile, "  <%s:%s_%s rdf:resource=\"%s:%s_%s\"/>\n", PREFIX, table->tableName, table->columns[i].columnName, PREFIX, table->columns[i].FKtarget, table->columns[i].defaultValue.data);
				}
				else
				{
					fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, table->columns[i].defaultValue.data, PREFIX, table->tableName, table->columns[i].columnName);
				}
			}
			// If not, does it have a value?
			else if((row_buffer + i)->data[0] != '\0')
			{
				// Is it a foreign key?
				if(table->columns[i].FKtarget[0] != '\0')
				{
					fprintf(outputfile, "  <%s:%s_%s rdf:resource=\"%s:%s_%s\"/>\n", PREFIX, table->tableName, table->columns[i].columnName, PREFIX, table->columns[i].FKtarget, (row_buffer + i)->data);
				}
				else
				{
					fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, (row_buffer + i)->data, PREFIX, table->tableName, table->columns[i].columnName);
				}
			}
			// If not, must it?
			else if(table->columns[i].type == req)
			{
				// If it must, is there a default value?
				if(table->columns[i].defaultValue.data[0] != '\0')
				{
					// Is it a foreign key?
					if(table->columns[i].FKtarget[0] != '\0')
					{
						fprintf(outputfile, "  <%s:%s_%s rdf:resource=\"%s:%s_%s\"/>\n", PREFIX, table->tableName, table->columns[i].columnName, PREFIX, table->columns[i].FKtarget, table->columns[i].defaultValue.data);
					}
					else
					{
						fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, table->columns[i].defaultValue.data, PREFIX, table->tableName, table->columns[i].columnName);
					}
				}
				// If not, warn the user.
				else
				{
					printf("Input file error!\nColumn %s in table %s is specified as required, but an empty cell was found.\nContinuing...\n", table->columns[i].columnName, table->tableName);
				}
			}
			// If not, write nil:
			else
			{
				fprintf(outputfile, "  <%s:%s_%s><rdf:nil/></%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, PREFIX, table->tableName, table->columns[i].columnName);
			}
		}
		fprintf(outputfile, "</%s:%s>\n", PREFIX, table->tableName);
		memset(row_buffer, '\0', MAX_COLUMNS * sizeof(*row_buffer));
	}
}

// This function generates triples for a leaf table.
void genTriples_leaf(char *inputfile_map, int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table)
{
	while(readRow(inputfile_map, cursor, row_buffer) != 1)
	{
		if((row_buffer + (table->primaryIdentifier))->data[0] == '\0')
		{
			printf("Input file error!\nTable %s has row missing Primary Identifier %s\nContinuing...\n", table->tableName, table->columns[table->primaryIdentifier].columnName);
			memset(row_buffer, '\0', MAX_COLUMNS * sizeof(*row_buffer));
			continue;
		}
		fprintf(outputfile,"<%s:%s rdf:ID=\"%s_%s\">\n", PREFIX, table->tableName, table->tableName, (row_buffer + (table->primaryIdentifier))->data);
		for(int i = 0; i < table->totalColumns; i++)
		{
			// Is this the PI?
			if(i == table->primaryIdentifier)
			{
				continue;
			}
			// Is column virtual?
			else if(table->columns[i].type == virt)
			{
				fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, table->columns[i].defaultValue.data, PREFIX, table->tableName, table->columns[i].columnName);
			}
			// If not, does it have a value?
			else if((row_buffer + i)->data[0] != '\0')
			{
				fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, (row_buffer + i)->data, PREFIX, table->tableName, table->columns[i].columnName);
			}
			// If not, must it?
			else if(table->columns[i].type == req)
			{
				// If it must, is there a default value?
				if(table->columns[i].defaultValue.data[0] != '\0')
				{
					fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, table->columns[i].defaultValue.data, PREFIX, table->tableName, table->columns[i].columnName);
				}
				// If not, warn the user.
				else
				{
					printf("Input file error!\nColumn %s in table %s is specified as required, but an empty cell was found.\nContinuing...\n", table->columns[i].columnName, table->tableName);
				}
			}
			// If not, write nil:
			else
			{
				fprintf(outputfile, "  <%s:%s_%s><rdf:nil/></%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, PREFIX, table->tableName, table->columns[i].columnName);
			}
		}
		fprintf(outputfile, "</%s:%s>\n", PREFIX, table->tableName);
		memset(row_buffer, '\0', MAX_COLUMNS * sizeof(*row_buffer));
	}
}

// This function generates triples for a table whose PI is independent.
void genTriples(char *inputfile_map, int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table)
{
	while(readRow(inputfile_map, cursor, row_buffer) != 1)
	{
		if((row_buffer + (table->primaryIdentifier))->data[0] == '\0')
		{
			printf("Input file error!\nTable %s has row missing Primary Identifier %s\nContinuing...\n", table->tableName, table->columns[table->primaryIdentifier].columnName);
			memset(row_buffer, '\0', MAX_COLUMNS * sizeof(*row_buffer));
			continue;
		}
		fprintf(outputfile,"<%s:%s rdf:ID=\"%s_%s\">\n", PREFIX, table->tableName, table->tableName, (row_buffer + (table->primaryIdentifier))->data);
		for(int i = 0; i < table->totalColumns; i++)
		{
			// Is this the PI?
			if(i == table->primaryIdentifier)
			{
				continue;
			}
			// Is column virtual?
			else if(table->columns[i].type == virt)
			{
				// Is it a foreign key?
				if(table->columns[i].FKtarget[0] != '\0')
				{
					fprintf(outputfile, "  <%s:%s_%s rdf:resource=\"%s:%s_%s\"/>\n", PREFIX, table->tableName, table->columns[i].columnName, PREFIX, table->columns[i].FKtarget, table->columns[i].defaultValue.data);
				}
				else
				{
					fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, table->columns[i].defaultValue.data, PREFIX, table->tableName, table->columns[i].columnName);
				}
			}
			// If not, does it have a value?
			else if((row_buffer + i)->data[0] != '\0')
			{
				// Is it a foreign key?
				if(table->columns[i].FKtarget[0] != '\0')
				{
					fprintf(outputfile, "  <%s:%s_%s rdf:resource=\"%s:%s_%s\"/>\n", PREFIX, table->tableName, table->columns[i].columnName, PREFIX, table->columns[i].FKtarget, (row_buffer + i)->data);
				}
				else
				{
					fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, (row_buffer + i)->data, PREFIX, table->tableName, table->columns[i].columnName);
				}
			}
			// If not, must it?
			else if(table->columns[i].type == req)
			{
				// If it must, is there a default value?
				if(table->columns[i].defaultValue.data[0] != '\0')
				{
					// Is it a foreign key?
					if(table->columns[i].FKtarget[0] != '\0')
					{
						fprintf(outputfile, "  <%s:%s_%s rdf:resource=\"%s:%s_%s\"/>\n", PREFIX, table->tableName, table->columns[i].columnName, PREFIX, table->columns[i].FKtarget, table->columns[i].defaultValue.data);
					}
					else
					{
						fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, table->columns[i].defaultValue.data, PREFIX, table->tableName, table->columns[i].columnName);
					}
				}
				// If not, warn the user.
				else
				{
					printf("Input file error!\nColumn %s in table %s is specified as required, but an empty cell was found.\nContinuing...\n", table->columns[i].columnName, table->tableName);
				}
			}
			// If not, write nil:
			else
			{
				fprintf(outputfile, "  <%s:%s_%s><rdf:nil/></%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, PREFIX, table->tableName, table->columns[i].columnName);
			}
		}
		fprintf(outputfile, "</%s:%s>\n", PREFIX, table->tableName);
		memset(row_buffer, '\0', MAX_COLUMNS * sizeof(*row_buffer));
	}
}

// This function generates triples for a table whose PI is an FK.
void genTriples_pifk(char *inputfile_map, int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table)
{
	while(readRow(inputfile_map, cursor, row_buffer) != 1)
	{
		if((row_buffer + (table->primaryIdentifier))->data[0] == '\0')
		{
			printf("Input file error!\nTable %s has row missing Primary Identifier %s\nContinuing...\n", table->tableName, table->columns[table->primaryIdentifier].columnName);
			memset(row_buffer, '\0', MAX_COLUMNS * sizeof(*row_buffer));
			continue;
		}
		fprintf(outputfile,"<%s:%s rdf:ID=\"%s_%s\">\n", PREFIX, table->tableName, table->tableName, (row_buffer + (table->primaryIdentifier))->data);
		for(int i = 0; i < table->totalColumns; i++)
		{
			// Is this the PI?
			if(i == table->primaryIdentifier)
			{
				continue;
			}
			// Is column virtual?
			else if(table->columns[i].type == virt)
			{
				// Is it a foreign key?
				if(table->columns[i].FKtarget[0] != '\0')
				{
					fprintf(outputfile, "  <%s:%s_%s rdf:resource=\"%s:%s_%s\"/>\n", PREFIX, table->tableName, table->columns[i].columnName, PREFIX, table->columns[i].FKtarget, table->columns[i].defaultValue.data);
				}
				else
				{
					fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, table->columns[i].defaultValue.data, PREFIX, table->tableName, table->columns[i].columnName);
				}
			}
			// If not, does it have a value?
			else if((row_buffer + i)->data[0] != '\0')
			{
				// Is it a foreign key?
				if(table->columns[i].FKtarget[0] != '\0')
				{
					fprintf(outputfile, "  <%s:%s_%s rdf:resource=\"%s:%s_%s\"/>\n", PREFIX, table->tableName, table->columns[i].columnName, PREFIX, table->columns[i].FKtarget, (row_buffer + i)->data);
				}
				else
				{
					fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, (row_buffer + i)->data, PREFIX, table->tableName, table->columns[i].columnName);
				}
			}
			// If not, must it?
			else if(table->columns[i].type == req)
			{
				// If it must, is there a default value?
				if(table->columns[i].defaultValue.data[0] != '\0')
				{
					// Is it a foreign key?
					if(table->columns[i].FKtarget[0] != '\0')
					{
						fprintf(outputfile, "  <%s:%s_%s rdf:resource=\"%s:%s_%s\"/>\n", PREFIX, table->tableName, table->columns[i].columnName, PREFIX, table->columns[i].FKtarget, table->columns[i].defaultValue.data);
					}
					else
					{
						fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, table->columns[i].defaultValue.data, PREFIX, table->tableName, table->columns[i].columnName);
					}
				}
				// If not, warn the user.
				else
				{
					printf("Input file error!\nColumn %s in table %s is specified as required, but an empty cell was found.\nContinuing...\n", table->columns[i].columnName, table->tableName);
				}
			}
			// If not, write nil:
			else
			{
				fprintf(outputfile, "  <%s:%s_%s><rdf:nil/></%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, PREFIX, table->tableName, table->columns[i].columnName);
			}
		}
		fprintf(outputfile, "</%s:%s>\n", PREFIX, table->tableName);
		fprintf(outputfile, "<%s:%s rdf:ID=\"%s_%s\">\n", PREFIX, table->columns[table->primaryIdentifier].FKtarget, table->columns[table->primaryIdentifier].FKtarget, (row_buffer + (table->primaryIdentifier))->data);
		fprintf(outputfile, "  <%s:%s rdf:resource=\"%s:%s_%s\"/>\n", PREFIX, table->tableName, PREFIX, table->tableName, (row_buffer + (table->primaryIdentifier))->data);
		fprintf(outputfile, "</%s:%s>\n\n", PREFIX, table->columns[table->primaryIdentifier].FKtarget);
		memset(row_buffer, '\0', MAX_COLUMNS * sizeof(*row_buffer));
	}
}


//These functions assume there are no virtual columns:


// This function generates triples for an anonymous leaf table.
void genTriples_anon_leaf_no_virt(char *inputfile_map, int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table)
{
	while(readRow(inputfile_map, cursor, row_buffer) != 1)
	{
		fprintf(outputfile,"<%s:%s>\n", PREFIX, table->tableName);
		for(int i = 0; i < table->totalColumns; i++)
		{
			// Does it have a value?
			if((row_buffer + i)->data[0] != '\0')
			{
				fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, (row_buffer + i)->data, PREFIX, table->tableName, table->columns[i].columnName);
			}
			// If not, must it?
			else if(table->columns[i].type == req)
			{
				// If it must, is there a default value?
				if(table->columns[i].defaultValue.data[0] != '\0')
				{
					fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, table->columns[i].defaultValue.data, PREFIX, table->tableName, table->columns[i].columnName);
				}
				// If not, warn the user.
				else
				{
					printf("Input file error!\nColumn %s in table %s is specified as required, but an empty cell was found.\nContinuing...\n", table->columns[i].columnName, table->tableName);
				}
			}
			// If not, write nil:
			else
			{
				fprintf(outputfile, "  <%s:%s_%s><rdf:nil/></%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, PREFIX, table->tableName, table->columns[i].columnName);
			}
		}
		fprintf(outputfile, "</%s:%s>\n", PREFIX, table->tableName);
		memset(row_buffer, '\0', MAX_COLUMNS * sizeof(*row_buffer));
	}
}

// This function generates triples for an anonymous table.
void genTriples_anon_no_virt(char *inputfile_map, int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table)
{
	while(readRow(inputfile_map, cursor, row_buffer) != 1)
	{
		fprintf(outputfile,"<%s:%s>\n", PREFIX, table->tableName);
		for(int i = 0; i < table->totalColumns; i++)
		{
			// Does it have a value?
			if((row_buffer + i)->data[0] != '\0')
			{
				// Is it a foreign key?
				if(table->columns[i].FKtarget[0] != '\0')
				{
					fprintf(outputfile, "  <%s:%s_%s rdf:resource=\"%s:%s_%s\"/>\n", PREFIX, table->tableName, table->columns[i].columnName, PREFIX, table->columns[i].FKtarget, (row_buffer + i)->data);
				}
				else
				{
					fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, (row_buffer + i)->data, PREFIX, table->tableName, table->columns[i].columnName);
				}
			}
			// If not, must it?
			else if(table->columns[i].type == req)
			{
				// If it must, is there a default value?
				if(table->columns[i].defaultValue.data[0] != '\0')
				{
					// Is it a foreign key?
					if(table->columns[i].FKtarget[0] != '\0')
					{
						fprintf(outputfile, "  <%s:%s_%s rdf:resource=\"%s:%s_%s\"/>\n", PREFIX, table->tableName, table->columns[i].columnName, PREFIX, table->columns[i].FKtarget, table->columns[i].defaultValue.data);
					}
					else
					{
						fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, table->columns[i].defaultValue.data, PREFIX, table->tableName, table->columns[i].columnName);
					}
				}
				// If not, warn the user.
				else
				{
					printf("Input file error!\nColumn %s in table %s is specified as required, but an empty cell was found.\nContinuing...\n", table->columns[i].columnName, table->tableName);
				}
			}
			// If not, write nil:
			else
			{
				fprintf(outputfile, "  <%s:%s_%s><rdf:nil/></%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, PREFIX, table->tableName, table->columns[i].columnName);
			}
		}
		fprintf(outputfile, "</%s:%s>\n", PREFIX, table->tableName);
		memset(row_buffer, '\0', MAX_COLUMNS * sizeof(*row_buffer));
	}
}

// This function generates triples for a leaf table.
void genTriples_leaf_no_virt(char *inputfile_map, int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table)
{
	while(readRow(inputfile_map, cursor, row_buffer) != 1)
	{
		if((row_buffer + (table->primaryIdentifier))->data[0] == '\0')
		{
			printf("Input file error!\nTable %s has row missing Primary Identifier %s\nContinuing...\n", table->tableName, table->columns[table->primaryIdentifier].columnName);
			memset(row_buffer, '\0', MAX_COLUMNS * sizeof(*row_buffer));
			continue;
		}
		fprintf(outputfile,"<%s:%s rdf:ID=\"%s_%s\">\n", PREFIX, table->tableName, table->tableName, (row_buffer + (table->primaryIdentifier))->data);
		for(int i = 0; i < table->totalColumns; i++)
		{
			// Is this the PI?
			if(i == table->primaryIdentifier)
			{
				continue;
			}
			// If not, does it have a value?
			else if((row_buffer + i)->data[0] != '\0')
			{
				fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, (row_buffer + i)->data, PREFIX, table->tableName, table->columns[i].columnName);
			}
			// If not, must it?
			else if(table->columns[i].type == req)
			{
				// If it must, is there a default value?
				if(table->columns[i].defaultValue.data[0] != '\0')
				{
					fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, table->columns[i].defaultValue.data, PREFIX, table->tableName, table->columns[i].columnName);
				}
				// If not, warn the user.
				else
				{
					printf("Input file error!\nColumn %s in table %s is specified as required, but an empty cell was found.\nContinuing...\n", table->columns[i].columnName, table->tableName);
				}
			}
			// If not, write nil:
			else
			{
				fprintf(outputfile, "  <%s:%s_%s><rdf:nil/></%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, PREFIX, table->tableName, table->columns[i].columnName);
			}
		}
		fprintf(outputfile, "</%s:%s>\n", PREFIX, table->tableName);
		memset(row_buffer, '\0', MAX_COLUMNS * sizeof(*row_buffer));
	}
}

// This function generates triples for a table whose PI is independent.
void genTriples_no_virt(char *inputfile_map, int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table)
{
	while(readRow(inputfile_map, cursor, row_buffer) != 1)
	{
		if((row_buffer + (table->primaryIdentifier))->data[0] == '\0')
		{
			printf("Input file error!\nTable %s has row missing Primary Identifier %s\nContinuing...\n", table->tableName, table->columns[table->primaryIdentifier].columnName);
			memset(row_buffer, '\0', MAX_COLUMNS * sizeof(*row_buffer));
			continue;
		}
		fprintf(outputfile,"<%s:%s rdf:ID=\"%s_%s\">\n", PREFIX, table->tableName, table->tableName, (row_buffer + (table->primaryIdentifier))->data);
		for(int i = 0; i < table->totalColumns; i++)
		{
			// Is this the PI?
			if(i == table->primaryIdentifier)
			{
				continue;
			}
			// If not, does it have a value?
			else if((row_buffer + i)->data[0] != '\0')
			{
				// Is it a foreign key?
				if(table->columns[i].FKtarget[0] != '\0')
				{
					fprintf(outputfile, "  <%s:%s_%s rdf:resource=\"%s:%s_%s\"/>\n", PREFIX, table->tableName, table->columns[i].columnName, PREFIX, table->columns[i].FKtarget, (row_buffer + i)->data);
				}
				else
				{
					fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, (row_buffer + i)->data, PREFIX, table->tableName, table->columns[i].columnName);
				}
			}
			// If not, must it?
			else if(table->columns[i].type == req)
			{
				// If it must, is there a default value?
				if(table->columns[i].defaultValue.data[0] != '\0')
				{
					// Is it a foreign key?
					if(table->columns[i].FKtarget[0] != '\0')
					{
						fprintf(outputfile, "  <%s:%s_%s rdf:resource=\"%s:%s_%s\"/>\n", PREFIX, table->tableName, table->columns[i].columnName, PREFIX, table->columns[i].FKtarget, table->columns[i].defaultValue.data);
					}
					else
					{
						fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, table->columns[i].defaultValue.data, PREFIX, table->tableName, table->columns[i].columnName);
					}
				}
				// If not, warn the user.
				else
				{
					printf("Input file error!\nColumn %s in table %s is specified as required, but an empty cell was found.\nContinuing...\n", table->columns[i].columnName, table->tableName);
				}
			}
			// If not, write nil:
			else
			{
				fprintf(outputfile, "  <%s:%s_%s><rdf:nil/></%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, PREFIX, table->tableName, table->columns[i].columnName);
			}
		}
		fprintf(outputfile, "</%s:%s>\n", PREFIX, table->tableName);
		memset(row_buffer, '\0', MAX_COLUMNS * sizeof(*row_buffer));
	}
}

// This function generates triples for a table whose PI is an FK.
void genTriples_pifk_no_virt(char *inputfile_map, int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table)
{
	while(readRow(inputfile_map, cursor, row_buffer) != 1)
	{
		if((row_buffer + (table->primaryIdentifier))->data[0] == '\0')
		{
			printf("Input file error!\nTable %s has row missing Primary Identifier %s\nContinuing...\n", table->tableName, table->columns[table->primaryIdentifier].columnName);
			memset(row_buffer, '\0', MAX_COLUMNS * sizeof(*row_buffer));
			continue;
		}
		fprintf(outputfile,"<%s:%s rdf:ID=\"%s_%s\">\n", PREFIX, table->tableName, table->tableName, (row_buffer + (table->primaryIdentifier))->data);
		for(int i = 0; i < table->totalColumns; i++)
		{
			// Is this the PI?
			if(i == table->primaryIdentifier)
			{
				continue;
			}
			// If not, does it have a value?
			else if((row_buffer + i)->data[0] != '\0')
			{
				// Is it a foreign key?
				if(table->columns[i].FKtarget[0] != '\0')
				{
					fprintf(outputfile, "  <%s:%s_%s rdf:resource=\"%s:%s_%s\"/>\n", PREFIX, table->tableName, table->columns[i].columnName, PREFIX, table->columns[i].FKtarget, (row_buffer + i)->data);
				}
				else
				{
					fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, (row_buffer + i)->data, PREFIX, table->tableName, table->columns[i].columnName);
				}
			}
			// If not, must it?
			else if(table->columns[i].type == req)
			{
				// If it must, is there a default value?
				if(table->columns[i].defaultValue.data[0] != '\0')
				{
					// Is it a foreign key?
					if(table->columns[i].FKtarget[0] != '\0')
					{
						fprintf(outputfile, "  <%s:%s_%s rdf:resource=\"%s:%s_%s\"/>\n", PREFIX, table->tableName, table->columns[i].columnName, PREFIX, table->columns[i].FKtarget, table->columns[i].defaultValue.data);
					}
					else
					{
						fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, table->columns[i].defaultValue.data, PREFIX, table->tableName, table->columns[i].columnName);
					}
				}
				// If not, warn the user.
				else
				{
					printf("Input file error!\nColumn %s in table %s is specified as required, but an empty cell was found.\nContinuing...\n", table->columns[i].columnName, table->tableName);
				}
			}
			// If not, write nil:
			else
			{
				fprintf(outputfile, "  <%s:%s_%s><rdf:nil/></%s:%s_%s>\n", PREFIX, table->tableName, table->columns[i].columnName, PREFIX, table->tableName, table->columns[i].columnName);
			}
		}
		fprintf(outputfile, "</%s:%s>\n", PREFIX, table->tableName);
		fprintf(outputfile, "<%s:%s rdf:ID=\"%s_%s\">\n", PREFIX, table->columns[table->primaryIdentifier].FKtarget, table->columns[table->primaryIdentifier].FKtarget, (row_buffer + (table->primaryIdentifier))->data);
		fprintf(outputfile, "  <%s:%s rdf:resource=\"%s:%s_%s\"/>\n", PREFIX, table->tableName, PREFIX, table->tableName, (row_buffer + (table->primaryIdentifier))->data);
		fprintf(outputfile, "</%s:%s>\n\n", PREFIX, table->columns[table->primaryIdentifier].FKtarget);
		memset(row_buffer, '\0', MAX_COLUMNS * sizeof(*row_buffer));
	}
}
