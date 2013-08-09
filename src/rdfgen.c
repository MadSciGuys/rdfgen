// Travis Whitaker 2013
// twhitak@its.jnj.com

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rdfgen.h>

int getColNames(char *firstline, char *columnames)
{
	int colnum = 1;
	unsigned register int j=0;
	int jmp=30;
	for(unsigned register int i=0;i<31000;i++)
	{
		if(*(firstline+j) == '\0')
		{
			break;
		}
		else if(*(firstline+j) == ',' || *(firstline+j) == '\n')
		{
			i = i+jmp;
			jmp = 31;
			colnum++;
		}
		else
		{
			*(columnames+i) = *(firstline+j);
		}
		j++;
		if(jmp == 0)
		{
			printf("ERROR: Name of column %d greater than 30 chars.\n",colnum);
			free(columnames);
			columnames = NULL;
			return -1;
		}
		jmp--;
	}
	return colnum;
}

void outputHeader(FILE *outputfile, char *tablename, int colnum, char *columnames)
{
	fprintf(outputfile,"<?xml version=\"1.0\"?>\n<!DOCTYPE rdf:RDF [<!ENTITY xsd \"http://www.w3.org/2001/XMLSchema#\">]>\n<rdf:RDF\n  xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n  xmlns:rdfs=\"http://www.w3.org/2000/01/rdf-schema#\"\n  xml:base=\"http://example.org/schemas/vehicles\"\n  xmlns:dwh=\"http://www.jnj.com/dwh#\">\n<rdfs:Datatype rdf:about=\"&xsd;string\">\n\n<rdfs:Class rdf:ID=\"");

	fprintf(outputfile,"%s",tablename);
	fprintf(outputfile,"\"/>\n\n");

	for(unsigned register int i=0;i<colnum;i++)
	{
		fprintf(outputfile,"<rdf:Property rdf:ID=\"%s~%s\">\n  <rdfs:domain rdf:resource=\"#%s\"/>\n  <rdfs:range rdf:resource=\"&xsd;string\"/>\n</rdf:Property>\n\n",tablename,columnames+i*31,tablename);
	}

	return;
}
