// Travis Whitaker 2013
// twhitak@its.jnj.com

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <rdfgen.h>
#include <fileIO.h>

int main(int argc,char *argv[])
{
	FILE *inputfile;
	FILE *outputfile;

	char *inputfilename = malloc(256);
	if(inputfilename == NULL)
	{
		printf("Couldn't allocate memory for *inputfilename.\nFATAL ERROR\n");
		return 0;
	}
	char *outputfilename = malloc(35);
	if(outputfilename == NULL)
	{
		printf("Couldn't allocate memory for *outputfilename.\nFATAL ERROR\n");
		return 0;
	}
	char *tablename = malloc(31);
	if(tablename == NULL)
	{
		printf("Couldn't allocate memory for *tablename.\nFATAL ERROR\n");
		return 0;
	}

	if(argv[1] == NULL)
	{
		printf("Too few arguments.\nUsage: rdfgen filename\n");
		return 0;
	}
	else if(argv[2] != NULL)
	{
		printf("Too many arguments.\nUsage: rdfgen filename\n");
		return 0;
	}

	strncpy(inputfilename,argv[1],255);
	*(inputfilename+255) = '\0';
	outputFilename(inputfilename,outputfilename,tablename);

	inputfile = fopen(inputfilename,"r");
	if(inputfile == NULL)
	{
		printf("Couldn't open %s\nFATAL ERROR\n",inputfilename);
		return 0;
	}
	free(inputfilename);
	outputfile = fopen(outputfilename,"w");
	if(outputfile == NULL)
	{
		printf("Couldn't open %s\nFATAL ERROR\n",outputfilename);
		return 0;
	}
	free(outputfilename);

	char *columnames = malloc(31000);
	if(columnames == NULL)
	{
		printf("Couldn't allocate memory for *columnames.\nFATAL ERROR\n");
		return 0;
	}

	int colnum = getColNames(inputfile,columnames);
	if(colnum == -1)
	{
		printf("FATAL ERROR.\n");
		return 0;
	}

	if(columnames[0] == 'I' && columnames [1] == 'D' && columnames[2] == '\0')
	{
		outputHeader(outputfile,tablename,colnum,columnames);
		fprintf(outputfile,"\n\n");
		if(outputTriples(outputfile,inputfile,tablename,colnum,columnames) == 1)
		{
			printf("FATAL ERROR\n");
			return 0;
		}
	}
	else
	{
		outputHeader_anon(outputfile,tablename,colnum,columnames);
		fprintf(outputfile,"\n\n");
		if(outputTriples_anon(outputfile,inputfile,tablename,colnum,columnames) == 1)
		{
			printf("FATAL ERROR\n");
			return 0;
		}
	}
	fprintf(outputfile,"\n\n");

	outputFooter(outputfile);

	fclose(inputfile);
	fclose(outputfile);
	return 0;
}
