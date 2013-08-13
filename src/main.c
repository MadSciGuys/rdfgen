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

	//Make sure there is only one argument:
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

	//Copy the arg and build the output filename:
	strncpy(inputfilename,argv[1],255);
	*(inputfilename+255) = '\0';
	outputFilename(inputfilename,outputfilename,tablename);

	//Open the files:
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

	//Allocate memory for the 2D array of column names:
	char *columnames = malloc(31000); //(30 chars + \0) * 1000 max cols
	if(columnames == NULL)
	{
		printf("Couldn't allocate memory for *columnames.\nFATAL ERROR\n");
		return 0;
	}

	//Get the names and number of columns:
	int colnum = getColNames(inputfile,columnames);
	if(colnum == -1)
	{
		printf("FATAL ERROR.\n");
		return 0;
	}

	//Print the RDF header to the output file:
	if(columnames[0] == 'I' && columnames [1] == 'D' && columnames[2] == '\0')
	{
		outputHeader(outputfile,tablename,colnum,columnames);
	}
	else
	{
		printf("Tables without ID not yet implemented...\n");
		return 0;
	}
	fprintf(outputfile,"\n\n");

	//THIS IS THE IMPORTANT PART!
	//Print the triples for each column:
	if(outputTriples(outputfile,inputfile,tablename,colnum,columnames) == 1)
	{
		printf("FATAL ERROR\n");
		return 0;
	}
	fprintf(outputfile,"\n\n");

	//Output the footer:
	outputFooter(outputfile);

	//Close the files:
	fclose(inputfile);
	fclose(outputfile);
	return 0;
}
