// Travis Whitaker 2013
// twhitak@its.jnj.com

// Standard I/O and string manipulation:
#include <stdio.h>
#include <string.h>

// Memory management, kernel API and system types:
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

// Kernel level disk IO:
#include <fcntl.h>

#include <rdfgen/limits.h>
#include <rdfgen/interface.h>
#include <rdfgen/structure.h>



int main(int argc, char *argv[])
{
	// First make sure there are at least two arguments, fail otherwise:
	if(argc < 3)
	{
		printf("Too few arguments!\nUsage:\nrdfgen schemafile csvfile1 csvfile2 csvfile3 ...\n");
		return 1;
	}

	// Declare file descriptors:
	int schemafile_fd;
	int inputfile_fd;

	// The output file is not memory mapped, so it gets a file pointer:
	FILE *outputfile;
	// Save a little memory for the output filename:
	char outputfilename[MAX_TABLE_NAME_LEN + RDF_EXT_LEN + 1];

	// Pointers for memory maps:
	char *schemafile_map;
	char *inputfile_map;

	// fstat() writes it's result to these structs:
	struct stat schemafile_stat;
	struct stat inputfile_stat;


	// The schema file's map persists across the processing of input files,
	// so we'll handle it outside of the main loop.
	// First ask the kernel to create the fd:
	schemafile_fd = open(argv[1], O_READ_FLAGS);
	// Make sure that worked:
	if(schemafile_fd == -1)
	{
		printf("File I/O error!\nUnable to open schema file %s\nFATAL ERROR\n", argv[1]);
		return 1;
	}
	// Ask the kernel to stat the file:
	if(fstat(schemafile_fd, &schemafile_stat) == -1)
	{
		printf("File I/O error!\nUnable to stat schema file %s\nFATAL ERROR\n", argv[1]);
		return 1;
	}
	// Ask the kernel to map the memory:
	schemafile_map = mmap(NULL, schemafile_stat.st_size, PROT_READ, MAP_PRIVATE, schemafile_fd, 0);
	if(schemafile_map == MAP_FAILED)
	{
		printf("Memory map error!\nUnable to create map for schema file %s\nFATAL ERROR\n", argv[1]);
		return 1;
	}
	// Set page cache mode:
	if(madvise(schemafile_map, schemafile_stat.st_size, MADV_SEQUENTIAL) == -1)
	{
		printf("Page cache mode set error!\nUnable to set MADV_SEQUENTIAL page cache optimization mode.\nFATAL ERROR\n");
		return 1;
	}
	// Allocate memory for the table metadata:
	table_t *table = malloc(sizeof(*table));
	if(table == NULL)
	{
		printf("Memory I/O error!\nUnable to allocate memory for table metadata.\nFATAL ERROR\n");
		return 1;
	}
	// Zero out:
	memset(table, '\0', sizeof(*table));
	// Allocate memory for a row buffer:
	field_t *row_buffer = malloc(MAX_COLUMNS * sizeof(*row_buffer));
	if(row_buffer == NULL)
	{
		printf("Memory I/O error!\nUnable to allocate memory for row buffer.\nFATAL ERROR\n");
		return 1;
	}
	// Zero out:
	memset(row_buffer, '\0', sizeof(*row_buffer) * MAX_COLUMNS);
	// Ready for the main loop. We're going to iterate over the remaining arguments:
	for(int currentArg = 2; currentArg < argc; currentArg++)
	{
		// Get the table name from the input file name:
		if(getTableName(argv[currentArg], outputfilename, table) == -1)
		{
			printf("FATAL ERROR\n");
			return 1;
		}
		// Open the input file:
		inputfile_fd = open(argv[currentArg], O_READ_FLAGS);
		// Make sure that worked:
		if(schemafile_fd == -1)
		{
			printf("File I/O error!\nUnable to open input file %s\nFATAL ERROR\n", argv[currentArg]);
			return 1;
		}
		// Stat the file:
		if(fstat(inputfile_fd, &inputfile_stat) == -1)
		{
			printf("File I/O error!\nUnable to stat input file %s\nFATAL ERROR\n", argv[currentArg]);
			return 1;
		}
		// Map the memory:
		inputfile_map = mmap(NULL, inputfile_stat.st_size, PROT_READ, MAP_PRIVATE, inputfile_fd, 0);
		if(inputfile_map == MAP_FAILED)
		{
			printf("Memory map error!\nUnable to create map for input file %s\nFATAL ERROR\n", argv[currentArg]);
			return 1;
		}
		// Set page cache mode:
		if(madvise(inputfile_map, inputfile_stat.st_size, MADV_SEQUENTIAL) == -1)
		{
			printf("Page cache mode set error!\nUnable to set MADV_SEQUENTIAL page cache optimization mode.\nFATAL ERROR\n");
			return 1;
		}
		// Check to make sure the input file has data in it:
		if(checkEmpty(inputfile_map) == -1)
		{
			printf("FATAL ERROR\n");
			return 1;
		}
		else if(checkEmpty(inputfile_map) == 1)
		{
			printf("Input file error!\nFile %s contains no data.\nContinuing...\n", argv[currentArg]);
			char *emptyfilename = malloc(MAX_TABLE_NAME_LEN + RDF_EXT_LEN + 1 + 7);
			strcat(emptyfilename, outputfilename);
			strcat(emptyfilename, ".empty");
			rename(outputfilename, emptyfilename);
			free(emptyfilename);
			// Clean up after this iteration:
			memset(outputfilename, '\0', MAX_TABLE_NAME_LEN + RDF_EXT_LEN + 1);
			memset(table, '\0', sizeof(*table));
			if(munmap(inputfile_map, inputfile_stat.st_size) == -1)
			{
				printf("Memory map error!\nUnable to unmap input file %s\nFATAL ERROR\n", argv[currentArg]);
				return 1;
			}
			memset(&inputfile_stat, '\0', sizeof(inputfile_stat));
			if(close(inputfile_fd) == -1)
			{
				printf("File I/O error!\nUnable to close input file %s\nFATAL ERROR\n", argv[currentArg]);
				return 1;
			}
			continue;
		}
		// Get the column names:
		if(getColumnNames(inputfile_map, table) == 1)
		{
			printf("FATAL ERROR\n");
			return 1;
		}
		// Get table metadata:
		if(getTableMetadata(schemafile_map, table) == 1)
		{
			printf("FATAL ERROR\n");
			return 1;
		}
		// Open the output file:
		outputfile = fopen(outputfilename,"w");
		if(outputfile == NULL)
		{
			printf("File I/O error!\nUnable to open output file %s\nFATAL ERROR\n", argv[currentArg]);
			return 1;
		}
		// Print some table details for debugging purposes:
		printf("Table name: %s\n", table->tableName);
		printf("Number of columns: %d\n", table->totalColumns);
		if(table->primaryIdentifier == -1)
		{
			printf("No Primary Identifier\n");
		}
		else
		{
			printf("Primary Identifier: %s\n", table->columns[table->primaryIdentifier].columnName);
		}
		printf("Columns:\n");
		for(int i = 0; i < table->totalColumns; i++)
		{
			printf("%4d:%-32s", i + 1, table->columns[i].columnName);
			switch(table->columns[i].type)
			{
			case real:
				printf("%-6s","real");
				break;
			case req:
				printf("%-6s","req");
				break;
			case virt:
				printf("%-6s","virt");
				break;
			default:
				printf("%-6s","!!!!");
				break;
			}
			switch(table->columns[i].FKtarget[0])
			{
			case '\0':
				printf("%-36s","Independent");
				break;
			default:
				printf("->%-34s",table->columns[i].FKtarget);
				break;
			}
			switch(table->columns[i].defaultValue.data[0])
			{
			case '\0':
				printf("no default value");
				break;
			default:
				printf("default value: %s", table->columns[i].defaultValue.data);
				break;
			}
			printf("\n");
		}
		// Output the RDF header:
		printf("Writing header...\n");
		outputHeader(outputfile, table);
		// Output the RDF triples:
		printf("Generating triples...\n");
		outputTriples(outputfile, inputfile_map, table, row_buffer);
		// Clean up after this iteration:
		printf("Finished %s.\n",outputfilename);
		fclose(outputfile);
		memset(outputfilename, '\0', MAX_TABLE_NAME_LEN + RDF_EXT_LEN + 1);
		memset(table, '\0', sizeof(table));
		if(munmap(inputfile_map, inputfile_stat.st_size) == -1)
		{
			printf("Memory map error!\nUnable to unmap input file %s\nFATAL ERROR\n",argv[currentArg]);
			return 1;
		}
		memset(&inputfile_stat, '\0', sizeof(inputfile_stat));
		if(close(inputfile_fd) == -1)
		{
			printf("File I/O error!\nUnable to close input file%s\nFATAL ERROR\n", argv[currentArg]);
			return 1;
		}
	}
	return 0;
}
