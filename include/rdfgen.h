// Travis Whitaker 2013
// twhitak@its.jnj.com

#ifndef RDFGEN_H
#define RDFGEN_H

int getColNames(char *firstline, char*columnames);
void outputHeader(FILE *outputfile, char *tablename, int colnum, char *columnames);

#endif
