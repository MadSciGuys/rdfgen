// Travis Whitaker 2013
// twhitak@its.jnj.com

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rdfgen.h>

int getColNames(FILE *inputfile, char *columnames)
{
	int colnum = 1;
	char cursor;
	char EOL = 0x00;
	int jmp=30;
	for(unsigned register int i=0;i<30999;i++)
	{
		cursor = fgetc(inputfile);
		if(cursor == '\n' || cursor == EOF)
		{
			EOL = 0xFF;
			break;
		}
		else if(cursor == ',')
		{
			i = i+jmp;
			jmp = 31;
			colnum++;
		}
		else
		{
			*(columnames+i) = cursor;
		}
		if(jmp == 0)
		{
			printf("Name of column %d greater than 30 characters.\n",colnum);
			free(columnames);
			columnames = NULL;
			return -1;
		}
		jmp--;
	}
	if(EOL == 0x00)
	{
		cursor = fgetc(inputfile);
		if(!(cursor == '\n' || cursor == EOF))
		{
			printf("First line must have no more than 31000 characters.\n");
			free(columnames);
			columnames = NULL;
			return -1;
		}
		ungetc(cursor,inputfile);
	}
	*(columnames+30999)='\0';
	return colnum;
}

void outputHeader(FILE *outputfile, char *tablename, int colnum, char *columnames)
{
	fprintf(outputfile,"<?xml version=\"1.0\"?>\n<!DOCTYPE rdf:RDF [<!ENTITY xsd \"http://www.w3.org/2001/XMLSchema#\">]>\n<rdf:RDF\n  xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n  xmlns:rdfs=\"http://www.w3.org/2000/01/rdf-schema#\"\n  xml:base=\"http://www.jnj.com/dwh/tables#\"\n  xmlns:dwh=\"http://www.jnj.com/dwh#\">\n<rdfs:Datatype rdf:about=\"&xsd;string\">\n\n<rdfs:Class rdf:ID=\"");

	fprintf(outputfile,"%s",tablename);
	fprintf(outputfile,"\"/>\n\n");

	for(unsigned register int i=0;i<colnum;i++)
	{
		fprintf(outputfile,"<rdf:Property rdf:ID=\"%s~%s\">\n  <rdfs:domain rdf:resource=\"#%s\"/>\n  <rdfs:range rdf:resource=\"&xsd;string\"/>\n</rdf:Property>\n\n",tablename,columnames+i*31,tablename);
	}

	return;
}

void outputFooter(FILE *outputfile)
{
	fprintf(outputfile,"</rdf:RDF>");
}

void outputTriples(FILE *outputfile, FILE *inputfile, char *line, int maxlinelength, char *tablename, char *colnum, char *columnames);
