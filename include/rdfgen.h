// Travis Whitaker 2013
// twhitak@its.jnj.com

#ifndef RDFGEN_H
#define RDFGEN_H

#define MAXFILENAME 256
#define MAXTABLENAME 30

#define RDFEXT ".rdf"
#define RDFEXTLEN 5

#define MAXCOLS 1000

typedef struct
{
	char name[MAXTABLENAME+1];
	char FK[MAXTABLENAME+1];
} colname_t;

void outputFilename(char *inputfilename, char *outputfilename, char *tablename);
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
