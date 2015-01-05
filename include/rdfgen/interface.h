// Travis Whitaker 2013-2015
// twhitak@its.jnj.com

#ifndef INTERFACE_H
#define INTERFACE_H

#include <rdfgen/structure.h>

int getTableName(char *inputfilename, char *outputfilename, table_t *table);
int checkEmpty(char *inputfile_map);
int getColumnNames(char *inputfile_map, table_t *table);
int getTableMetadata(char *schemafile_map, table_t *table);
void printTableMetadata(table_t *table);
void outputHeader(FILE *outputfile, table_t *table, unsigned long int *triples);
void outputTriples(FILE *outputfile, char *inputfile_map, table_t *table, field_t *row_buffer, unsigned long int *triples);

#endif
