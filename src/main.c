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

//Timekeeping:
#include <time.h>

#include <rdfgen/limits.h>
#include <rdfgen/interface.h>
#include <rdfgen/structure.h>
#include <rdfgen/color.h>



int main(int argc, char *argv[])
{
	// First make sure there are at least two arguments, fail otherwise:
	if(argc < 3)
	{
		printf(BOLD RED "Too few arguments!\n" RESET "Usage:\nrdfgen schemafile csvfile1 csvfile2 csvfile3 ...\n");
		return 1;
	}

	// Declare file descriptors:
	int schemafile_fd;
	int inputfile_fd;

	// The output file is not memory mapped, so it gets a file pointer:
	FILE *outputfile;
	// Stack space for the output filename:
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
		printf(BOLD RED "File I/O error!\nUnable to open schema file %s\n" REVERSE BLINK "FATAL ERROR\n" RESET, argv[1]);
		return 1;
	}
	// Get file stats:
	if(fstat(schemafile_fd, &schemafile_stat) == -1)
	{
		printf(BOLD RED "File I/O error!\nUnable to stat schema file %s\n" REVERSE "FATAL ERROR\n" RESET, argv[1]);
		return 1;
	}
	// Create memory map:
	schemafile_map = mmap(NULL, schemafile_stat.st_size, PROT_READ, MAP_PRIVATE, schemafile_fd, 0);
	if(schemafile_map == MAP_FAILED)
	{
		printf(BOLD RED "Memory map error!\nUnable to create map for schema file %s\n" REVERSE "FATAL ERROR\n" RESET, argv[1]);
		return 1;
	}
	// Attempt to set page cache mode:
	if(madvise(schemafile_map, schemafile_stat.st_size, MADV_SEQUENTIAL) == -1)
	{
		printf(BOLD RED "Page cache mode set error!\nUnable to set MADV_SEQUENTIAL page cache optimization mode.\n" REVERSE "FATAL ERROR\n" RESET);
		return 1;
	}
	// Allocate memory for the table metadata:
	table_t *table = malloc(sizeof(*table));
	if(table == NULL)
	{
		printf(BOLD RED "Memory I/O error!\nUnable to allocate memory for table metadata.\n" REVERSE "FATAL ERROR\n" RESET);
		return 1;
	}
	// Zero out:
	memset(table, '\0', sizeof(*table));
	// Allocate memory for a row buffer:
	field_t *row_buffer = malloc(MAX_COLUMNS * sizeof(*row_buffer));
	if(row_buffer == NULL)
	{
		printf(BOLD RED "Memory I/O error!\nUnable to allocate memory for row buffer.\n" REVERSE "FATAL ERROR\n" RESET);
		return 1;
	}
	// Zero out:
	memset(row_buffer, '\0', sizeof(*row_buffer) * MAX_COLUMNS);
	// Keep track of the total number of triples generated(includes implied triples):
	unsigned long int triples = 0;
	//Keep track of generation start and end times:
	unsigned long int start_time;
	unsigned long int end_time;
	struct timespec start_time_spec;
	struct timespec end_time_spec;
	long double total_time;
	long double triples_per_sec;
	// Ready for the main loop. We're going to iterate over the remaining arguments:
	for(int currentArg = 2; currentArg < argc; currentArg++)
	{
		// Get the table name from the input file name:
		if(getTableName(argv[currentArg], outputfilename, table) == -1)
		{
			printf(BOLD RED REVERSE BLINK "FATAL ERROR\n" RESET);
			return 1;
		}
		// Open the input file:
		inputfile_fd = open(argv[currentArg], O_READ_FLAGS);
		// Make sure that worked:
		if(inputfile_fd == -1)
		{
			printf(BOLD RED "File I/O error!\nUnable to open input file %s\n" REVERSE BLINK "FATAL ERROR\n" RESET, argv[currentArg]);
			return 1;
		}
		// Stat the file:
		if(fstat(inputfile_fd, &inputfile_stat) == -1)
		{
			printf(BOLD RED "File I/O error!\nUnable to stat input file %s\n" REVERSE BLINK "FATAL ERROR\n" RESET, argv[currentArg]);
			return 1;
		}
		// Map the memory:
		inputfile_map = mmap(NULL, inputfile_stat.st_size, PROT_READ, MAP_PRIVATE, inputfile_fd, 0);
		if(inputfile_map == MAP_FAILED)
		{
			printf(BOLD RED "Memory map error!\nUnable to create map for input file %s\n" REVERSE BLINK "FATAL ERROR\n" RESET, argv[currentArg]);
			return 1;
		}
		// Set page cache mode:
		if(madvise(inputfile_map, inputfile_stat.st_size, MADV_SEQUENTIAL) == -1)
		{
			printf(BOLD RED "Page cache mode set error!\nUnable to set MADV_SEQUENTIAL page cache optimization mode.\n" REVERSE BLINK "FATAL ERROR\n" RESET);
			return 1;
		}
		// Check to make sure the input file has data in it:
		if(checkEmpty(inputfile_map) == -1)
		{
			printf(BOLD RED REVERSE BLINK "FATAL ERROR\n" RESET);
			return 1;
		}
		else if(checkEmpty(inputfile_map) == 1)
		{
			printf(BOLD RED "Input file error!\nFile %s contains no data.\n" GREEN REVERSE "Continuing...\n" RESET, argv[currentArg]);
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
				printf(BOLD RED "Memory map error!\nUnable to unmap input file %s\n" REVERSE BLINK "FATAL ERROR\n" RESET, argv[currentArg]);
				return 1;
			}
			memset(&inputfile_stat, '\0', sizeof(inputfile_stat));
			if(close(inputfile_fd) == -1)
			{
				printf(BOLD RED "File I/O error!\nUnable to close input file %s\n" REVERSE BLINK "FATAL ERROR\n" RESET, argv[currentArg]);
				return 1;
			}
			triples = 0;
			continue;
		}
		// Get the column names:
		if(getColumnNames(inputfile_map, table) == 1)
		{
			printf(BOLD RED REVERSE BLINK "FATAL ERROR\n" RESET);
			return 1;
		}
		// Get table metadata:
		if(getTableMetadata(schemafile_map, table) == 1)
		{
			printf(BOLD RED REVERSE BLINK "FATAL ERROR\n" RESET);
			return 1;
		}
		// Open the output file:
		outputfile = fopen(outputfilename,"w");
		if(outputfile == NULL)
		{
			printf(BOLD RED "File I/O error!\nUnable to open output file %s\n" REVERSE BLINK "FATAL ERROR\n" RESET, argv[currentArg]);
			return 1;
		}
		// Print some table details for debugging purposes:
		printf("Table name: %s\n", table->tableName);
		printf("Number of columns: %d\n", table->totalColumns);
		if(table->primaryIdentifier == -1)
		{
			printf(BOLD CYAN "No Primary Identifier\n" RESET);
		}
		else
		{
			printf("Primary Identifier: " BOLD BLUE "%s\n" RESET, table->columns[table->primaryIdentifier].columnName);
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
				printf(BOLD RED "%-6s" RESET,"req");
				break;
			case virt:
				printf(BOLD MAGENTA "%-6s" RESET,"virt");
				break;
			default:
				printf(BOLD RED REVERSE BLINK "%-6s" RESET,"!!!!");
				break;
			}
			switch(table->columns[i].FKtarget[0])
			{
			case '\0':
				printf(BOLD GREEN "%-36s" RESET,"Independent");
				break;
			default:
				printf(BOLD YELLOW "->%-34s" RESET,table->columns[i].FKtarget);
				break;
			}
			switch(table->columns[i].defaultValue.data[0])
			{
			case '\0':
				printf(BOLD GREEN "No default value" RESET);
				break;
			default:
				printf(BOLD MAGENTA "default value: %s" RESET, table->columns[i].defaultValue.data);
				break;
			}
			printf("\n");
		}
		// Output the RDF header:
		printf("Writing header...\n");
		outputHeader(outputfile, table, &triples);
		// Output the RDF triples:
		printf("Generating triples...\n");
		clock_gettime(CLOCK_MONOTONIC, &start_time_spec);
		outputTriples(outputfile, inputfile_map, table, row_buffer, &triples);
		clock_gettime(CLOCK_MONOTONIC, &end_time_spec);
		start_time = (start_time_spec.tv_sec * 1000000) + (start_time_spec.tv_nsec/1000);
		end_time = (end_time_spec.tv_sec * 1000000) + (end_time_spec.tv_nsec/1000);
		total_time = (end_time - start_time)/1000000.0;
		triples_per_sec = triples / total_time;
		// Clean up after this iteration:
		printf(BOLD GREEN "Finished %s, " BLUE "%lu" GREEN " total triples, " BLUE "%Lf" GREEN " seconds, " BLUE "%Lf" GREEN " triples/second.\n" RESET,outputfilename, triples, total_time, triples_per_sec);
		fclose(outputfile);
		memset(outputfilename, '\0', MAX_TABLE_NAME_LEN + RDF_EXT_LEN + 1);
		memset(table, '\0', sizeof(*table));
		if(munmap(inputfile_map, inputfile_stat.st_size) == -1)
		{
			printf(BOLD RED "Memory map error!\nUnable to unmap input file %s\n" REVERSE BLINK "FATAL ERROR\n" RESET,argv[currentArg]);
			return 1;
		}
		memset(&inputfile_stat, '\0', sizeof(inputfile_stat));
		if(close(inputfile_fd) == -1)
		{
			printf(BOLD RED "File I/O error!\nUnable to close input file%s\n" REVERSE BLINK "FATAL ERROR\n" RESET, argv[currentArg]);
			return 1;
		}
		triples = 0;
		start_time = 0;
		end_time = 0;
		memset(&start_time_spec,'\0',sizeof(start_time_spec));
		memset(&end_time_spec,'\0',sizeof(end_time_spec));
	}
	free(table);
	free(row_buffer);
	return 0;
}
