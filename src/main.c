// Travis Whitaker 2013
// twhitak@its.jnj.com

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <rdfgen.h>

int main(int argc,char *argv[])
{
	FILE *inputfile;
	FILE *outputfile;
	FILE *schemafile;

	char *inputfilename = malloc(MAXFILENAME);
	if(inputfilename == NULL)
	{
		printf("Couldn't allocate memory for *inputfilename.\nFATAL ERROR\n");
		return 0;
	}
	char *outputfilename = malloc(MAXTABLENAME+RDFEXTLEN);
	if(outputfilename == NULL)
	{
		printf("Couldn't allocate memory for *outputfilename.\nFATAL ERROR\n");
		return 0;
	}
	char *tablename = malloc(MAXTABLENAME+1);
	if(tablename == NULL)
	{
		printf("Couldn't allocate memory for *tablename.\nFATAL ERROR\n");
		return 0;
	}
	char anon = 'n';

	if(argv[1] == NULL || argv[3] != NULL)
	{
		printf("Argument error.\nUsage: rdfgen schema_file_name csv_file_name\n");
		return 0;
	}

	strncpy(inputfilename,argv[2],MAXFILENAME-1);
	*(inputfilename+(MAXFILENAME-1)) = '\0';
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
	schemafile = fopen(argv[1],"r");
	if(schemafile == NULL)
	{
		printf("Couldn't open %s\nFATAL ERROR\n",argv[1]);
		return 0;
	}

	colname_t *columnames = malloc(MAXCOLS*sizeof(colname_t));
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

	int isLeaf = resolveFK(&anon,schemafile,tablename,columnames);
	if(isLeaf == -1)
	{
		printf("FATAL ERROR\n");
		return 0;
	}
	else if(isLeaf == 1)
	{
		if(anon == 'n')
		{
			outputHeader_leaf(outputfile,tablename,colnum,columnames);
			fprintf(outputfile,"\n\n");
			if(outputTriples_leaf(outputfile,inputfile,tablename,colnum,columnames) == 1)
			{
				printf("FATAL ERROR\n");
				return 0;
			}
		}
		else
		{
			outputHeader_leaf_anon(outputfile,tablename,colnum,columnames);
			fprintf(outputfile,"\n\n");
			if(outputTriples_leaf_anon(outputfile,inputfile,tablename,colnum,columnames) == 1)
			{
				printf("FATAL ERROR\n");
				return 0;
			}
		}
	}
	else
	{
		if(anon == 'n')
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
	}

	fprintf(outputfile,"\n\n");

	outputFooter(outputfile);

	fclose(inputfile);
	fclose(outputfile);
	return 0;
}
