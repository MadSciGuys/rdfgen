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
	fprintf(outputfile,"<?xml version=\"1.0\"?>\n<!DOCTYPE rdf:RDF [<!ENTITY xsd \"http://www.w3.org/2001/XMLSchema#\">]>\n<rdf:RDF\n  xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n  xmlns:rdfs=\"http://www.w3.org/2000/01/rdf-schema#\"\n  xml:base=\"http://www.jnj.com/dwh/tables#\"\n  xmlns:dwh=\"http://www.jnj.com/dwh#\">\n\n\n<rdfs:Class rdf:ID=\"");

	fprintf(outputfile,"%s",tablename);
	fprintf(outputfile,"\"/>\n\n");

	for(unsigned register int i=1;i<colnum;i++) //Skip the first name if the ID is present.
	{
		fprintf(outputfile,"<rdf:Property rdf:ID=\"%s_%s\">\n  <rdfs:domain rdf:resource=\"#%s\"/>\n</rdf:Property>\n\n",tablename,columnames+i*31,tablename);
	}

	return;
}

void outputFooter(FILE *outputfile)
{
	fprintf(outputfile,"</rdf:RDF>");
}

int outputTriples(FILE *outputfile, FILE *inputfile, char *tablename, int colnum, char *columnames)
{
	int colmax = colnum-1; //Minimize use of arithmetic in the main loop.
	char cursor;
	char *current_colname = NULL;
	do
	{
		//Print start tag for row:
		fprintf(outputfile,"<dwh:%s rdf:ID=\"%s",tablename,tablename);
		do
		{
			cursor = fgetc(inputfile);
			//Check to see that the line doesn't end here,
			//this happens if you forget to use csvgen first:
			switch(cursor)
			{
			case '\n':
			case EOF:
				goto malformedFile;
				break;
			default:
				break;
			}
			if(cursor == ',')
			{
				break;
			}
			else
			{
				fprintf(outputfile,"%c",cursor);
			}
		} while(cursor != EOF);
		fprintf(outputfile,"\">\n"); //Close the row start tag.
		//Now we're going to write the tags for each column:
		for(unsigned register int i=1;i<colnum;i++)
		{
			current_colname = columnames+(i*31); //Update the column name pointer.
			//Start the property tag:
			fprintf(outputfile,"  <dwh:%s_%s>",tablename,current_colname);
			//Now we're going to get the data:
			do
			{
				cursor = fgetc(inputfile);
				//Make sure the line doesn't end prematurely,
				//usually happens if csvgen wasn't run properly:
				switch(cursor)
				{
				case '\n':
				case EOF:
					if(i != (colnum-1))
					{
						goto malformedFile;
					}
					break;
				default:
					break;
				}
				if(cursor == ',' || cursor == '\n')
				{
					break;
				}
				else
				{
					fprintf(outputfile,"%c",cursor);
				}
			} while(cursor != EOF);
			fprintf(outputfile,"</dwh:%s_%s>\n",tablename,current_colname);
		}
		fprintf(outputfile,"</dwh:%s>\n\n",tablename);
		cursor = fgetc(inputfile);
		if(cursor == EOF)
		{
			break;
		}
		else
		{
			ungetc(cursor,inputfile);
		}
	} while(cursor != EOF);
	return 0;

	//We jump here if anything terrible happens:
	malformedFile:
		printf("File is malformed, check output file.\n");
		fclose(inputfile);
		fclose(outputfile);
		free(tablename);
		free(columnames);
		return 1;
}
