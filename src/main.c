// Travis Whitaker 2013
// twhitak@its.jnj.com

// Standard IO and string manipulation:
#include <stdio.h>
#include <string.h>

// Memory management, kernel features and types:
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

// Kernel level IO:
#include <fcntl.h>

// rdfgen structures and functions:
#include <rdfgen.h>



int main(int argc,char *argv[])
{
	// First make sure there are at least two arguments, fail otherwise:
	if(argc < 3)
	{
		printf("TOO FEW ARGUMENTS!\nUsage:\nrdfgen schemafile csvfile1 csvfile2 csvfile3 ...\n");
		return 1;
	}

	// Declare file descriptors:
	int schemafile_fd;
	int inputfile_fd;

	// The output file is not memory mapped, so it gets a file pointer:
	FILE *outputfile;
	// Save a little memory for the output filename:
	char outputfilename[MAX_TABLE_NAME_LEN+RDF_EXT_LEN+1];

	// Pointers for memory maps:
	char *schemafile_map;
	char *inputfile_map;

	// fstat() writes it's result to these structs:
	struct stat schemafile_stat;
	struct stat inputfile_stat;


	// The schema file's map persists across the processing of input files,
	// so we'll handle it outside of the main loop.
	// First ask the kernel to create the fd:
	schemafile_fd = open(argv[1],O_READ_FLAGS);
	// Make sure that worked:
	if(schemafile_fd == -1)
	{
		printf("File I/O error!\nUnable to open schema file %s\nFATAL ERROR\n",argv[1]);
		return 1;
	}
	// Ask the kernel to stat the file:
	if(fstat(schemafile_fd,&schemafile_stat) == -1)
	{
		printf("File I/O error!\nUnable to stat schema file %s\nFATAL ERROR\n",argv[1]);
		return 1;
	}
	// Ask the kernel to map the memory:
	schemafile_map = mmap(NULL,schemafile_stat.st_size,PROT_READ,MAP_PRIVATE,schemafile_fd,0);
	if(schemafile_map == MAP_FAILED)
	{
		printf("Memory I/O error!\nUnable to create map for schema file %s\nFATAL ERROR\n",argv[1]);
		return 1;
	}
	// Got this far, get ready for the main loop.
	// Allocate memory for the table metadata:
	table_t *table = malloc(sizeof(table_t));
	if(table == NULL)
	{
		printf("Memory I/O error!\nUnable to allocate memory for table metadata.\nFATAL ERROR\n");
		return 1;
	}
	// Allocate memory for a row buffer:
	field_t *row_buffer = malloc(MAX_COLUMNS*sizeof(field_t));
	if(row_buffer == NULL)
	{
		printf("Memory I/O error!\nUnable to allocate memory for row buffer.\nFATAL ERROR\n");
		return 1;
	}
	// Ready for the main loop. We're going to iterate over the remaining
	// arguments:
	for(int currentArg = 2;currentArg < argc;currentArg++)
	{
		// Get the table name from the input file name:
		if(getTableName(argv[currentArg],outputfilename,table) == -1)
		{
			printf("FATAL ERROR\n");
			return 1;
		}
		printf("Table name: %s\nOutput file name: %s\n",table->tableName,outputfilename);
	}
	
	
	return 0;
}

/*
int oldmain(int argc,char *argv[])
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
*/
