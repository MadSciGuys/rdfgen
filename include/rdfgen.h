// Travis Whitaker 2013
// twhitak@its.jnj.com

#ifndef RDFGEN_H
#define RDFGEN_H

typedef struct
{
	char name[31];
	char FK[31];
} colname_t;


int getColNames(FILE *inputfile, colname_t *columnames);
int resolveFK(FILE *schmafile, char *tablename, colname_t *columnames);

void outputHeader_leaf(FILE *outputfile, char *tablename, int colnum, colname_t *columnames);
void outputFooter(FILE *outputfile);
int outputTriples_leaf(FILE *outputfile, FILE *inputfile, char *tablename, int colnum, colname_t *columnames);
void outputHeader_leaf_anon(FILE *outputfile, char *tablename, int colnum, colname_t *columnames);
int outputTriples_leaf_anon(FILE *outputfile, FILE *inputfile, char *tablename, int colnum, colname_t *columnames);

void outputHeader(FILE *outputfile, char *tablename, int colnum, colname_t *columnames);
int outputTriples(FILE *outputfile, FILE *inputfile, char *tablename, int colnum, colname_t *columnames);
void outputHeader_anon(FILE *outputfile, char *tablename, int colnum, colname_t *columnames);
int outputTriples_anon(FILE *outputfile, FILE *inputfile, char *tablename, int colnum, colname_t *columnames);


#endif
