#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <rdfgen.h>
#include <fileIO.h>

int main(int argc,char *argv[])
{
	//File pointers:
	FILE *inputfile;
	FILE *outputfile;

	//Allocate memory for filenames:
	char *inputfilename = malloc(1000);
	if(inputfilename == NULL)
	{
		printf("Couldn't allocate memory for *inputfilename.\n");
		return 0;
	}
	char *outputfilename = malloc(1000);
	if(outputfilename == NULL)
	{
		printf("Couldn't allocate memory for *outputfilename.\n");
		return 0;
	}
	char *tablename = malloc(1000);
	if(tablename == NULL)
	{
		printf("Couldn't allocate memory for *tablename.\n");
		return 0;
	}

	if(argv[1] == NULL)
	{
		printf("Too few arguments.\n");
		return 0;
	}

	//Copy the arg and build the output filename:
	strncpy(inputfilename,argv[1],999);
	*(inputfilename+999) = '\0';
	OutputFilename(inputfilename,outputfilename,tablename);

	//Open the files:
	inputfile = fopen(inputfilename,"r");
	if(inputfile == NULL)
	{
		printf("Couldn't open %s\n",inputfilename);
		return 0;
	}
	free(inputfilename);
	outputfile = fopen(outputfilename,"w");
	if(outputfile == NULL)
	{
		printf("Couldn't open %s\n",outputfilename);
		return 0;
	}
	free(outputfilename);

	fprintf(outputfile,"It works!\nThis is the output RDF file.");
	fclose(inputfile);
	fclose(outputfile);

	return 0;
}
