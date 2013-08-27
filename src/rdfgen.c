// Travis Whitaker 2013
// twhitak@its.jnj.com

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rdfgen.h>

void outputFilename(char *inputfilename, char *outputfilename, char *tablename)
{
	for(unsigned register int i=0; i<MAXTABLENAME; i++)
	{
		if(*(inputfilename+i) == '.' || *(inputfilename+i) == '\0')
		{
			*(tablename+i) = '\0';
			break;
		}
		*(tablename+i) = *(inputfilename+i);
	}
	*(tablename+MAXTABLENAME) = '\0';
	for(unsigned register int i=0;i<MAXTABLENAME+1;i++)
	{
		if(*(tablename+i) == '\0')
		{
			strcpy(outputfilename+i,RDFEXT);
			break;
		}
		*(outputfilename+i) = *(tablename+i);
	}
	*(outputfilename+MAXTABLENAME+RDFEXTLEN-1) = '\0';
	return;
}

int getColNames(FILE *inputfile, colname_t *columnames)
{
	int colnum = 1;
	char cursor;
	colname_t *colname = columnames;
	int j = 0;
	char EOL = 0x00;
	int len=MAXTABLENAME;
	for(unsigned register int i=0;i<((MAXTABLENAME+1)*MAXCOLS);i++)
	{
		cursor = fgetc(inputfile);
		if(cursor == '\n' || cursor == EOF)
		{
			EOL = 0xFF;
			break;
		}
		else if(cursor == ',')
		{
			j = 0;
			len = MAXTABLENAME+1;
			colname++;
			colnum++;
		}
		else
		{
			colname->name[j] = cursor;
			j++;
		}
		if(len == 0)
		{
			printf("Name of column %d greater than 30 characters.\n",colnum);
			free(columnames);
			columnames = NULL;
			return -1;
		}
		len--;
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
	return colnum;
}

int resolveFK(char *anon, FILE *schemafile, char *tablename, colname_t *columnames)
{
	char cursor = fgetc(schemafile);
	char seek_tablename = 'n';
	while(seek_tablename != 'y')
	{
		switch(cursor)
		{
		case EOF:
			printf("Tablename %s not found in schema.\nAssuming %s is leaf table.\n",tablename,tablename);
			return 1;
		case '\n':
			printf("Syntax error in schema file, found unexpected newline character.\n");
			return -1;
		default:
			break;
		}
		if(cursor == '~' || cursor == '\t')
		{
			while(cursor != '\n')
			{
				cursor = fgetc(schemafile);
			}
			cursor = fgetc(schemafile);
			continue;
		}
		else if(cursor == '#' || cursor == '&')
		{
			if(cursor == '&')
			{
				*anon = 'y';
			}
			cursor = fgetc(schemafile);
			for(unsigned register int i=0; i<(MAXTABLENAME+1);i++)
			{
				if(tablename[i] == '\0')
				{
					if(cursor == '\n')
					{
						seek_tablename = 'y';
					}
					break;
				}
				else
				{
					if(tablename[i] == cursor)
					{
						cursor = fgetc(schemafile);
						continue;
					}
					else
					{
						while(cursor != '\n')
						{
							cursor = fgetc(schemafile);
						}
						cursor = fgetc(schemafile);
						break;
					}
				}
			}
		}
		else
		{
			printf("Syntax error in schema file, found line that does not start with ~, #, or \\t.\n");
			return -1;
		}
	}

	char colname_buffer[MAXTABLENAME+1];
	char tablename_buffer[MAXTABLENAME+1];
	char falloff = 'y';

	while(cursor != EOF)
	{
		cursor = fgetc(schemafile);
		if(cursor == '\t')
		{
			for(unsigned register int i=0;i<30;i++)
			{
				cursor = fgetc(schemafile);
				if(cursor == '@')
				{
					colname_buffer[i] = '\0';
					falloff = 'n';
					break;
				}
				else if(cursor == '&')
				{
					return 1;
				}
				else if(cursor != '\n' && cursor != '\t' && cursor != '#' && cursor != EOF)
				{
					colname_buffer[i] = cursor;
				}
				else
				{
					printf("Line found under table %s without @.\n",tablename);
					return -1;
				}
			}
			if(falloff == 'y')
			{
				cursor = fgetc(schemafile);
				if(cursor != '@')
				{
					printf("Columname too long or missing @ under table %s.\n",tablename);
					return -1;
				}
			}
			for(unsigned int i=0;i<MAXTABLENAME;i++)
			{
				cursor = fgetc(schemafile);
				if(cursor == '\n')
				{
					tablename_buffer[i] = '\0';
					break;
				}
				else
				{
					tablename_buffer[i] = cursor;
				}
			}
			for(unsigned register int i=0;i<MAXCOLS;i++)
			{
				if(columnames[i].name[0] == '\0')
				{
					printf("Column %s not found in table %s.\n",colname_buffer,tablename);
					return -1;
				}
				if(strncmp(colname_buffer,columnames[i].name,(MAXCOLS+1)) == 0)
				{
					memcpy(columnames[i].FK,tablename_buffer,(MAXTABLENAME+1));
					colname_buffer[0] = '\0';
					tablename_buffer[0] = '\0';
					break;
				}
			}
			if(colname_buffer[0] != '\0')
			{
				printf("column %s not found in table %s.\n",colname_buffer,tablename);
				return -1;
			}
		}
		else if(cursor == '~')
		{
			while(cursor != '\n')
			{
				cursor = fgetc(schemafile);
			}
		}
		else if(cursor == '#' || cursor == EOF)
		{
			break;
		}
		else
		{
			printf("Syntax error in schema file under table %s.\n",tablename);
			return -1;
		}
	}
	return 0;
}

void outputHeader_leaf(FILE *outputfile, char *tablename, int colnum, colname_t *columnames)
{
	fprintf(outputfile,"<?xml version=\"1.0\"?>\n<!DOCTYPE rdf:RDF [<!ENTITY xsd \"http://www.w3.org/2001/XMLSchema#\">]>\n<rdf:RDF\n  xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n  xmlns:rdfs=\"http://www.w3.org/2000/01/rdf-schema#\"\n  xml:base=\"http://www.jnj.com/dwh/tables#\"\n  xmlns:dwh=\"http://www.jnj.com/dwh#\">\n\n\n<rdfs:Class rdf:ID=\"");

	fprintf(outputfile,"%s",tablename);
	fprintf(outputfile,"\"/>\n\n");

	for(unsigned register int i=1;i<colnum;i++)
	{
		fprintf(outputfile,"<rdf:Property rdf:ID=\"%s_%s\">\n  <rdfs:domain rdf:resource=\"#%s\"/>\n</rdf:Property>\n\n",tablename,columnames[i].name,tablename);
	}

	return;
}

void outputFooter(FILE *outputfile)
{
	fprintf(outputfile,"</rdf:RDF>");
}

int outputTriples_leaf(FILE *outputfile, FILE *inputfile, char *tablename, int colnum, colname_t *columnames)
{
	int colmax = colnum-1;
	char cursor;
	char *current_colname = NULL;
	do
	{
		fprintf(outputfile,"<dwh:%s rdf:ID=\"%s",tablename,tablename);
		do
		{
			cursor = fgetc(inputfile);
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
		fprintf(outputfile,"\">\n");
		for(unsigned register int i=1;i<colnum;i++)
		{
			current_colname = columnames[i].name;
			fprintf(outputfile,"  <dwh:%s_%s>",tablename,current_colname);
			do
			{
				cursor = fgetc(inputfile);
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

	malformedFile:
		printf("File is malformed, check output file.\n");
		fclose(inputfile);
		fclose(outputfile);
		free(tablename);
		free(columnames);
		return 1;
}

void outputHeader_leaf_anon(FILE *outputfile, char *tablename, int colnum, colname_t *columnames)
{
	fprintf(outputfile,"<?xml version=\"1.0\"?>\n<!DOCTYPE rdf:RDF [<!ENTITY xsd \"http://www.w3.org/2001/XMLSchema#\">]>\n<rdf:RDF\n  xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n  xmlns:rdfs=\"http://www.w3.org/2000/01/rdf-schema#\"\n  xml:base=\"http://www.jnj.com/dwh/tables#\"\n  xmlns:dwh=\"http://www.jnj.com/dwh#\">\n\n\n<rdfs:Class rdf:ID=\"");

	fprintf(outputfile,"%s",tablename);
	fprintf(outputfile,"\"/>\n\n");

	for(unsigned register int i=0;i<colnum;i++)
	{
		fprintf(outputfile,"<rdf:Property rdf:ID=\"%s_%s\">\n  <rdfs:domain rdf:resource=\"#%s\"/>\n</rdf:Property>\n\n",tablename,columnames[i].name,tablename);
	}

	return;
}

int outputTriples_leaf_anon(FILE *outputfile, FILE *inputfile, char *tablename, int colnum, colname_t *columnames)
{
	int colmax = colnum-1;
	char cursor;
	char *current_colname = NULL;
	do
	{
		fprintf(outputfile,"<dwh:%s>\n",tablename);
		for(unsigned register int i=0;i<colnum;i++)
		{
			current_colname = columnames[i].name;
			fprintf(outputfile,"  <dwh:%s_%s>",tablename,current_colname);
			do
			{
				cursor = fgetc(inputfile);
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

	malformedFile:
		printf("File is malformed, check output file.\n");
		fclose(inputfile);
		fclose(outputfile);
		free(tablename);
		free(columnames);
		return 1;
}

void outputHeader(FILE *outputfile, char *tablename, int colnum, colname_t *columnames)
{
	fprintf(outputfile,"<?xml version=\"1.0\"?>\n<!DOCTYPE rdf:RDF [<!ENTITY xsd \"http://www.w3.org/2001/XMLSchema#\">]>\n<rdf:RDF\n  xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n  xmlns:rdfs=\"http://www.w3.org/2000/01/rdf-schema#\"\n  xml:base=\"http://www.jnj.com/dwh/tables#\"\n  xmlns:dwh=\"http://www.jnj.com/dwh#\">\n\n\n<rdfs:Class rdf:ID=\"");

	fprintf(outputfile,"%s",tablename);
	fprintf(outputfile,"\"/>\n\n");

	for(unsigned register int i=1;i<colnum;i++)
	{
		if(columnames[i].FK[0] == '\0')
		{
			fprintf(outputfile,"<rdf:Property rdf:ID=\"%s_%s\">\n  <rdfs:domain rdf:resource=\"#%s\"/>\n</rdf:Property>\n\n",tablename,columnames[i].name,tablename);
		}
		else
		{
			fprintf(outputfile,"<rdf:Property rdf:ID=\"%s_%s\">\n  <rdfs:domain rdf:resource=\"#%s\"/>\n  <rdfs:range rdf:resource=\"#%s\"/>\n</rdf:Property>\n\n",tablename,columnames[i].name,tablename,columnames[i].FK);
		}
	}

	return;
}

void outputHeader_anon(FILE *outputfile, char *tablename, int colnum, colname_t *columnames)
{
	fprintf(outputfile,"<?xml version=\"1.0\"?>\n<!DOCTYPE rdf:RDF [<!ENTITY xsd \"http://www.w3.org/2001/XMLSchema#\">]>\n<rdf:RDF\n  xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n  xmlns:rdfs=\"http://www.w3.org/2000/01/rdf-schema#\"\n  xml:base=\"http://www.jnj.com/dwh/tables#\"\n  xmlns:dwh=\"http://www.jnj.com/dwh#\">\n\n\n<rdfs:Class rdf:ID=\"");

	fprintf(outputfile,"%s",tablename);
	fprintf(outputfile,"\"/>\n\n");

	for(unsigned register int i=0;i<colnum;i++)
	{
		if(columnames[i].FK[0] == '\0')
		{
			fprintf(outputfile,"<rdf:Property rdf:ID=\"%s_%s\">\n  <rdfs:domain rdf:resource=\"#%s\"/>\n</rdf:Property>\n\n",tablename,columnames[i].name,tablename);
		}
		else
		{
			fprintf(outputfile,"<rdf:Property rdf:ID=\"%s_%s\">\n  <rdfs:domain rdf:resource=\"#%s\"/>\n  <rdfs:range rdf:resource=\"#%s\"/>\n</rdf:Property>\n\n",tablename,columnames[i].name,tablename,columnames[i].FK);
		}
	}

	return;
}

int outputTriples(FILE *outputfile, FILE *inputfile, char *tablename, int colnum, colname_t *columnames)
{
	int colmax = colnum-1;
	char cursor;
	char *current_colname = NULL;
	char *current_FK = NULL;
	do
	{
		fprintf(outputfile,"<dwh:%s rdf:ID=\"%s",tablename,tablename);
		do
		{
			cursor = fgetc(inputfile);
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
		fprintf(outputfile,"\">\n");
		for(unsigned register int i=1;i<colnum;i++)
		{
			current_colname = columnames[i].name;
			current_FK = columnames[i].FK;
			if(current_FK[0] == '\0')
			{
				fprintf(outputfile,"  <dwh:%s_%s>",tablename,current_colname);
				do
				{
					cursor = fgetc(inputfile);
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
			else
			{
				fprintf(outputfile,"  <dwh:%s_%s rdf:resource=\"dwh:%s",tablename,current_colname,current_FK);
				do
				{
					cursor = fgetc(inputfile);
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
				fprintf(outputfile,"\"/>\n");
			}
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

	malformedFile:
		printf("File is malformed, check output file.\n");
		fclose(inputfile);
		fclose(outputfile);
		free(tablename);
		free(columnames);
		return 1;
}

int outputTriples_anon(FILE *outputfile, FILE *inputfile, char *tablename, int colnum, colname_t *columnames)
{
	int colmax = colnum-1;
	char cursor;
	char *current_colname = NULL;
	char *current_FK = NULL;
	do
	{
		fprintf(outputfile,"<dwh:%s>\n",tablename);
		for(unsigned register int i=0;i<colnum;i++)
		{
			current_colname = columnames[i].name;
			current_FK = columnames[i].FK;
			fprintf(outputfile,"  <dwh:%s_%s>",tablename,current_colname);
			if(current_FK[0] == '\0')
			{
				do
				{
					cursor = fgetc(inputfile);
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
			}
			else
			{
				fprintf(outputfile,"<dwh:%s rdf:ID=\"%s",current_FK,current_FK);
				do
				{
					cursor = fgetc(inputfile);
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
				fprintf(outputfile,"\"/>");
			}
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

	malformedFile:
		printf("File is malformed, check output file.\n");
		fclose(inputfile);
		fclose(outputfile);
		free(tablename);
		free(columnames);
		return 1;
}
