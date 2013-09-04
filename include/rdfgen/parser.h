// Travis Whitaker 2013
// twhitak@its.jnj.com

#ifndef PARSER_H
#define FARSER_H

int schemaSeek(char *schemafile_map, char *tableName, int *cursor);
int schemaPI(char *schemafile_map, table_t table*, int *cursor);
int schemaFetchLine(char *schemafile_map, char *op, char *arg1, char *arg2, int *cursor);
int renameColumn(char *arg1, char *arg2, table_t *table);
int defineDV(char *arg1, char *arg2, table_t *table);
int requireColumn(char *arg1, char *arg2, table_t *table);
int defineFK(char *arg1, char *arg2, table_t *table);
int defineVC(char *arg1, char *arg2, table_t *table);

#endif
