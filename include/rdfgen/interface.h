// Travis Whitaker 2013
// twhitak@its.jnj.com

#ifndef INTERFACE_H
#define INTERFACE_H

#include <rdfgen/structure.h>



// Functions used in main() to interface with the modules:
int getTableName(char *inputfilename, char *outputfilename, table_t *table);
int checkEmpty(char *inputfile_map);
int getColumnNames(char *inputfile_map, table_t *table);
int getTableMetadata(char *schemafile_map, table_t *table);
void outputHeader(FILE *outputfile, table_t *table);
void outputTriples(FILE *outputfile, char *inputfile_map, table_t *table, field_t *row_buffer);

#endif
