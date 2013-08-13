// Travis Whitaker 2013
// twhitak@its.jnj.com

#ifndef RDFGEN_H
#define RDFGEN_H

int getColNames(FILE *inputfile, char *columnames);
void outputHeader(FILE *outputfile, char *tablename, int colnum, char *columnames);
void outputFooter(FILE *outputfile);
int outputTriples(FILE *outputfile, FILE *inputfile, char *tablename, int colnum, char *columnames);

#endif
