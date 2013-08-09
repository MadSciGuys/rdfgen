// Travis Whitaker 2013
// twhitak@its.jnj.com

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
	char *inputfilename = malloc(100);
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

	//Make sure there is only one argument:
	if(argv[1] == NULL)
	{
		printf("Too few arguments.\nUsage: rdfgen filename");
		return 0;
	}
	else if(argv[2] != NULL)
	{
		printf("Too many arguments.\nUsage: rdfgen filename");
		return 0;
	}

	//Copy the arg and build the output filename:
	strncpy(inputfilename,argv[1],99);
	*(inputfilename+99) = '\0';
	printf("Input filename: %s\n",inputfilename);
	outputFilename(inputfilename,outputfilename,tablename);
	printf("Table name: %s\n",tablename);
	printf("Output filename: %s\n",outputfilename);

	//Open the files:
	printf("Opening input file...\n");
	inputfile = fopen(inputfilename,"r");
	if(inputfile == NULL)
	{
		printf("Couldn't open %s\nFATAL ERROR\n",inputfilename);
		return 0;
	}
	free(inputfilename);
	printf("Opening output file...\n");
	outputfile = fopen(outputfilename,"w");
	if(outputfile == NULL)
	{
		printf("Couldn't open %s\nFATAL ERROR\n",outputfilename);
		return 0;
	}
	free(outputfilename);

	//Allocate memory for the 2D array of column names:
	printf("Allocating memory for *columnames...\n");
	char *columnames = malloc(31000); //(30 chars + \0) * 1000 max cols
	if(columnames == NULL)
	{
		printf("Couldn't allocate memory for *columnames.\nFATAL ERROR\n");
		return 0;
	}

	//Allocate memory for one line:
	printf("Allocating memory for *line...\n");
	char *line = malloc(4001000); //(4000 chars + comma)*1000 max cols
	printf("malloc returned.\n");
	if(line == NULL)
	{
		printf("Couldn't allocate memory for *line.\nFATAL ERROR\n");
		return 0;
	}
	printf("About to get first line.\n");
	getLine(inputfile,line,4001000);
	printf("Got first line.\n");

	//Get the names and number of columns:
	int colnum = getColNames(line,columnames);
	if(colnum == -1)
	{
		printf("FATAL ERROR.\n");
		return 0;
	}
	printf("Got column names.\n");

	//Print the RDF header to the output file:
	outputHeader(outputfile,tablename,columnames);

	//Close the files:
	fclose(inputfile);
	fclose(outputfile);
	return 0;
}
