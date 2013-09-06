// Travis Whitaker 2013
// twhitak@its.jnj.com

// Standard I/O functions:
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <rdfgen/limits.h>
#include <rdfgen/parser.h>
#include <rdfgen/interface.h>



// Return 1 if table has no FKs, 0 otherwise.
int checkLeaf(table_t *table)
{
	for(int i = 0; i < table->totalColumns; i++)
	{
		if(table->columns[i].FKtarget[0] != '\0')
		{
			return 1;
		}
	}
	return 0;
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
	for(unsigned int i = *cursor; i < (((MAX_FIELD_LEN + 1) * MAX_COLUMNS) + cursor); i++)
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
		}
	}
}

// This function generates triples for an anonymous leaf table.
void genTriples_anon_leaf(char *inputfile_map, int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table)
{
	while(readRow(inputfile_map, cursor, row_buffer) != 1)
	{
		fprintf(outputfile,"<%s:%s>\n", PREFIX, table->tableName);
		for(int i = 0; i < table.totalColumns; i++)
		{
			// Is column virtual?
			if(table.columns[i].type == virt)
			{
				fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table.tableName, table.columns[i].columnName, table.columns[i].defaultValue, PREFIX, table.tableName, table.columns[i].columnname);
			}
			// If not, does it have a value?
			else if((row_buffer + i)->data[0] != '\0')
			{
				fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table.tableName, table.columns[i].columnName, (row_buffer + i)->data, PREFIX, table.tableName, table.columns[i].columnName);
			}
			// If not, must it?
			else if(table.columns[i].type == req)
			{
				// If it must, is there a default value?
				if(table.columns[i].defaultValue[0] != '\0')
				{
					fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table.tableName, table.columns[i].columnName, table.columns[i].defaultValue, PREFIX, table.tableName, table.columns[i].columnname);
				}
				// If not, warn the user.
				else
				{
					printf("Input file error!\nColumn %s in table %s is specified as required, but an empty cell was found.\nContinuing...\n");
				}
			}
			// If not, write nil:
			else
			{
				fprintf(outputfile, "  <%s:%s_%s><rdf:nil/></%s:%s_%s>\n", PREFIX, table.tableName, table.columns[i].columnName, PREFIX, table.tableName, table.columns[i].columnname);
			}
		}
		fprintf(outputfile, "</%s:%s>\n", PREFIX, table->tableName);
	}
	fprintf(outputfile, "\n\n<rdf:RDF>");
	return 0;
}

// This function generates triples for an anonymous table.
void genTriples_anon(char *inputfile_map, int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table)
{
	while(readRow(inputfile_map, cursor, row_buffer) != 1)
	{
		fprintf(outputfile,"<%s:%s>\n", PREFIX, table->tableName);
		for(int i = 0; i < table.totalColumns; i++)
		{
			// Is column virtual?
			if(table.columns[i].type == virt)
			{
				// Is it a foreign key?
				if(table.columns[i].FKtarget[0] != '\0')
				{
					fprintf(outputfile, "  <%s:%s_%s rdf:resource=%s:%s_%s/>", PREFIX, table.tableName, table.columns[i].columnName, PREFIX, table.columns[i].FKtarget, table.columns[i].defaultValue);
				}
				else
				{
					fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table.tableName, table.columns[i].columnName, table.columns[i].defaultValue, PREFIX, table.tableName, table.columns[i].columnname);
				}
			}
			// If not, does it have a value?
			else if((row_buffer + i)->data[0] != '\0')
			{
				// Is it a foreign key?
				if(table.columns[i].FKtarget[0] != '\0')
				{
					fprintf(outputfile, "  <%s:%s_%s rdf:resource=%s:%s_%s/>", PREFIX, table.tableName, table.columns[i].columnName, PREFIX, table.columns[i].FKtarget, (row_buffer + i)->data);
				}
				else
				{
					fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table.tableName, table.columns[i].columnName, (row_buffer + i)->data, PREFIX, table.tableName, table.columns[i].columnName);
				}
			}
			// If not, must it?
			else if(table.columns[i].type == req)
			{
				// If it must, is there a default value?
				if(table.columns[i].defaultValue[0] != '\0')
				{
					// Is it a foreign key?
					if(table.columns[i].FKtarget[0] != '\0')
					{
						fprintf(outputfile, "  <%s:%s_%s rdf:resource=%s:%s_%s/>", PREFIX, table.tableName, table.columns[i].columnName, PREFIX, table.columns[i].FKtarget, table.columns[i].defaultValue);
					}
					else
					{
						fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table.tableName, table.columns[i].columnName, table.columns[i].defaultValue, PREFIX, table.tableName, table.columns[i].columnname);
					}
				}
				// If not, warn the user.
				else
				{
					printf("Input file error!\nColumn %s in table %s is specified as required, but an empty cell was found.\nContinuing...\n");
				}
			}
			// If not, write nil:
			else
			{
				fprintf(outputfile, "  <%s:%s_%s><rdf:nil/></%s:%s_%s>\n", PREFIX, table.tableName, table.columns[i].columnName, PREFIX, table.tableName, table.columns[i].columnname);
			}
		}
		fprintf(outputfile, "</%s:%s\n", PREFIX, table->tableName);
	}
	fprintf(outputfile, "\n\n<rdf:RDF>");
	return 0;
}

// This function generates triples for a leaf table.
void genTriples_leaf(char *inputfile_map, int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table)
{
	while(readRow(inputfile_map, cursor, row_buffer) != 1)
	{
		fprintf(outputfile,"<%s:%s rdf:ID=\"%s_%s\">\n", PREFIX, table->tableName, table->tableName, (row_buffer + (table->primaryIdentifier))->data);
		for(int i = 0; i < table.totalColumns; i++)
		{
			// Is this the PI?
			if(i == table.primaryIdentifier)
			{
				continue;
			}
			// Is column virtual?
			else if(table.columns[i].type == virt)
			{
				fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table.tableName, table.columns[i].columnName, table.columns[i].defaultValue, PREFIX, table.tableName, table.columns[i].columnname);
			}
			// If not, does it have a value?
			else if((row_buffer + i)->data[0] != '\0')
			{
				fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table.tableName, table.columns[i].columnName, (row_buffer + i)->data, PREFIX, table.tableName, table.columns[i].columnName);
			}
			// If not, must it?
			else if(table.columns[i].type == req)
			{
				// If it must, is there a default value?
				if(table.columns[i].defaultValue[0] != '\0')
				{
					fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table.tableName, table.columns[i].columnName, table.columns[i].defaultValue, PREFIX, table.tableName, table.columns[i].columnname);
				}
				// If not, warn the user.
				else
				{
					printf("Input file error!\nColumn %s in table %s is specified as required, but an empty cell was found.\nContinuing...\n");
				}
			}
			// If not, write nil:
			else
			{
				fprintf(outputfile, "  <%s:%s_%s><rdf:nil/></%s:%s_%s>\n", PREFIX, table.tableName, table.columns[i].columnName, PREFIX, table.tableName, table.columns[i].columnname);
			}
		}
		fprintf(outputfile, "</%s:%s>\n", PREFIX, table->tableName);
	}
	fprintf(outputfile, "\n\n<rdf:RDF>");
	return 0;
}

// This function generates triples for a table whose PI is independent.
void genTriples(char *inputfile_map, int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table)
{
	while(readRow(inputfile_map, cursor, row_buffer) != 1)
	{
		fprintf(outputfile,"<%s:%s rdf:ID=\"%s_%s\">\n", PREFIX, table->tableName, table->tableName, (row_buffer + (table->primaryIdentifier))->data);
		for(int i = 0; i < table.totalColumns; i++)
		{
			// Is this the PI?
			if(i == table.primaryIdentifier)
			{
				continue;
			}
			// Is column virtual?
			else if(table.columns[i].type == virt)
			{
				// Is it a foreign key?
				if(table.columns[i].FKtarget[0] != '\0')
				{
					fprintf(outputfile, "  <%s:%s_%s rdf:resource=%s:%s_%s/>", PREFIX, table.tableName, table.columns[i].columnName, PREFIX, table.columns[i].FKtarget, table.columns[i].defaultValue);
				}
				else
				{
					fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table.tableName, table.columns[i].columnName, table.columns[i].defaultValue, PREFIX, table.tableName, table.columns[i].columnname);
				}
			}
			// If not, does it have a value?
			else if((row_buffer + i)->data[0] != '\0')
			{
				// Is it a foreign key?
				if(table.columns[i].FKtarget[0] != '\0')
				{
					fprintf(outputfile, "  <%s:%s_%s rdf:resource=%s:%s_%s/>", PREFIX, table.tableName, table.columns[i].columnName, PREFIX, table.columns[i].FKtarget, (row_buffer + i)->data);
				}
				else
				{
					fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table.tableName, table.columns[i].columnName, (row_buffer + i)->data, PREFIX, table.tableName, table.columns[i].columnName);
				}
			}
			// If not, must it?
			else if(table.columns[i].type == req)
			{
				// If it must, is there a default value?
				if(table.columns[i].defaultValue[0] != '\0')
				{
					// Is it a foreign key?
					if(table.columns[i].FKtarget[0] != '\0')
					{
						fprintf(outputfile, "  <%s:%s_%s rdf:resource=%s:%s_%s/>", PREFIX, table.tableName, table.columns[i].columnName, PREFIX, table.columns[i].FKtarget, table.columns[i].defaultValue);
					}
					else
					{
						fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table.tableName, table.columns[i].columnName, table.columns[i].defaultValue, PREFIX, table.tableName, table.columns[i].columnname);
					}
				}
				// If not, warn the user.
				else
				{
					printf("Input file error!\nColumn %s in table %s is specified as required, but an empty cell was found.\nContinuing...\n");
				}
			}
			// If not, write nil:
			else
			{
				fprintf(outputfile, "  <%s:%s_%s><rdf:nil/></%s:%s_%s>\n", PREFIX, table.tableName, table.columns[i].columnName, PREFIX, table.tableName, table.columns[i].columnname);
			}
		}
		fprintf(outputfile, "</%s:%s\n", PREFIX, table->tableName);
	}
	fprintf(outputfile, "\n\n<rdf:RDF>");
	return 0;
}

// This function generates triples for a table whose PI is an FK.
void genTriples_pifk(char *inputfile_map, int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table)
{
	while(readRow(inputfile_map, cursor, row_buffer) != 1)
	{
		fprintf(outputfile,"<%s:%s rdf:ID=\"%s_%s\">\n", PREFIX, table->tableName, table->tableName, (row_buffer + (table->primaryIdentifier))->data);
		for(int i = 0; i < table.totalColumns; i++)
		{
			// Is this the PI?
			if(i == table.primaryIdentifier)
			{
				continue;
			}
			// Is column virtual?
			else if(table.columns[i].type == virt)
			{
				// Is it a foreign key?
				if(table.columns[i].FKtarget[0] != '\0')
				{
					fprintf(outputfile, "  <%s:%s_%s rdf:resource=%s:%s_%s/>", PREFIX, table.tableName, table.columns[i].columnName, PREFIX, table.columns[i].FKtarget, table.columns[i].defaultValue);
				}
				else
				{
					fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table.tableName, table.columns[i].columnName, table.columns[i].defaultValue, PREFIX, table.tableName, table.columns[i].columnname);
				}
			}
			// If not, does it have a value?
			else if((row_buffer + i)->data[0] != '\0')
			{
				// Is it a foreign key?
				if(table.columns[i].FKtarget[0] != '\0')
				{
					fprintf(outputfile, "  <%s:%s_%s rdf:resource=%s:%s_%s/>", PREFIX, table.tableName, table.columns[i].columnName, PREFIX, table.columns[i].FKtarget, (row_buffer + i)->data);
				}
				else
				{
					fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table.tableName, table.columns[i].columnName, (row_buffer + i)->data, PREFIX, table.tableName, table.columns[i].columnName);
				}
			}
			// If not, must it?
			else if(table.columns[i].type == req)
			{
				// If it must, is there a default value?
				if(table.columns[i].defaultValue[0] != '\0')
				{
					// Is it a foreign key?
					if(table.columns[i].FKtarget[0] != '\0')
					{
						fprintf(outputfile, "  <%s:%s_%s rdf:resource=%s:%s_%s/>", PREFIX, table.tableName, table.columns[i].columnName, PREFIX, table.columns[i].FKtarget, table.columns[i].defaultValue);
					}
					else
					{
						fprintf(outputfile, "  <%s:%s_%s>%s</%s:%s_%s>\n", PREFIX, table.tableName, table.columns[i].columnName, table.columns[i].defaultValue, PREFIX, table.tableName, table.columns[i].columnname);
					}
				}
				// If not, warn the user.
				else
				{
					printf("Input file error!\nColumn %s in table %s is specified as required, but an empty cell was found.\nContinuing...\n");
				}
			}
			// If not, write nil:
			else
			{
				fprintf(outputfile, "  <%s:%s_%s><rdf:nil/></%s:%s_%s>\n", PREFIX, table.tableName, table.columns[i].columnName, PREFIX, table.tableName, table.columns[i].columnname);
			}
		}
		fprintf(outputfile, "</%s:%s>\n", PREFIX, table->tableName);
		fprintf(outputfile, "<%s:%s rdf:ID=\"%s_%s\">\n", PREFIX, table->columns[table->primaryIdentifier].FKtarget, table->columns[table->primaryIdentifier].FKtarget, (row_buffer + (table->primaryIdentifier))->data);
		fprintf(outputfile, "  <%s:%s rdf:resource=%s:%s_%s/>\n", PREFIX, table->tableName, PREFIX, table->tableName, (row_buffer + (table->primaryIdentifier)->data));
		fprintf(outputfile, "</%s:%s>\n\n", PREFIX, table->columns[table->primaryIdentifier].FKtarget);
	}
	fprintf(outputfile, "\n\n<rdf:RDF>");
	return 0;
}
