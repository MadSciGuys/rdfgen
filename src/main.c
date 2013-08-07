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
		printf("Couldn't allocate memory for *inputfilename.\n");
		return 0;
	}
	char *outputfilename = malloc(35);
	if(outputfilename == NULL)
	{
		printf("Couldn't allocate memory for *outputfilename.\n");
		return 0;
	}
	char *tablename = malloc(31);
	if(tablename == NULL)
	{
		printf("Couldn't allocate memory for *tablename.\n");
		return 0;
	}

	//Make sure there is only one argument:
	if(argv[1] == NULL)
	{
		printf("Too few arguments.\n");
		return 0;
	}
	else if(argv[2] != NULL)
	{
		printf("Too many arguments.\n");
		return 0;
	}

	//Copy the arg and build the output filename:
	strncpy(inputfilename,argv[1],99);
	*(inputfilename+99) = '\0';
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

	//Allocate memory for the 2D array of column names:
	char *columnames = malloc(31000); //30 chars * 1000 max cols
	if(columnames == NULL)
	{
		printf("Couldn't allocate memory for *columnames.\n");
	}

	char *firstline = malloc(512000);
	if(columnames == NULL)
	{
		printf("Couldn't allocate memory for *firstline.\n");
	}
	getLine(inputfile,firstline,512000);

	getColNames(firstline,columnames);
	return 0;
}
