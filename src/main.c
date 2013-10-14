// Travis Whitaker 2013
// twhitak@its.jnj.com

// Standard I/O and string manipulation:
#include <stdio.h>
#include <string.h>

// Memory management, kernel API and types:
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

// Timekeeping:
#ifndef NO_TIME
#include <time.h>
#endif

// rdfgen headers:
#include <rdfgen/limits.h>
#include <rdfgen/interface.h>
#include <rdfgen/structure.h>

#include <rdfgen/color.h>

int main(int argc, char *argv[])
{
	if(argc < 3)
	{
		printf(BOLD RED "Too few arguments!\n" RESET "Usage:\nrdfgen schemafile csvfile1 csvfile2 csvfile3 ...\n");
		return 1;
	}

	int schemafile_fd;
	int inputfile_fd;

	FILE *outputfile;
	char outputfilename[MAX_TABLE_NAME_LEN + RDF_EXT_LEN + 1];

	char *schemafile_map;
	char *inputfile_map;

	struct stat schemafile_stat;
	struct stat inputfile_stat;


	// The schema file's map persists across the processing of input files.
	schemafile_fd = open(argv[1], O_READ_FLAGS);
	if(schemafile_fd == -1)
	{
		printf(BOLD RED "File I/O error!\nUnable to open schema file %s\n" REVERSE BLINK "FATAL ERROR\n" RESET, argv[1]);
		return 1;
	}
	if(fstat(schemafile_fd, &schemafile_stat) == -1)
	{
		printf(BOLD RED "File I/O error!\nUnable to stat schema file %s\n" REVERSE "FATAL ERROR\n" RESET, argv[1]);
		return 1;
	}
	schemafile_map = mmap(NULL, schemafile_stat.st_size, PROT_READ, MAP_PRIVATE, schemafile_fd, 0);
	if(schemafile_map == MAP_FAILED)
	{
		printf(BOLD RED "Memory map error!\nUnable to create map for schema file %s\n" REVERSE "FATAL ERROR\n" RESET, argv[1]);
		return 1;
	}
	// Attempt to set page cache mode, this is POSIX specific:
	if(madvise(schemafile_map, schemafile_stat.st_size, MADV_SEQUENTIAL) == -1)
	{
		printf(BOLD RED "Page cache mode set error!\nUnable to set MADV_SEQUENTIAL page cache optimization mode.\n" REVERSE "FATAL ERROR\n" RESET);
		return 1;
	}
	table_t *table = malloc(sizeof(*table));
	if(table == NULL)
	{
		printf(BOLD RED "Memory I/O error!\nUnable to allocate memory for table metadata.\n" REVERSE "FATAL ERROR\n" RESET);
		return 1;
	}
	memset(table, '\0', sizeof(*table));
	field_t *row_buffer = malloc(MAX_COLUMNS * sizeof(*row_buffer));
	if(row_buffer == NULL)
	{
		printf(BOLD RED "Memory I/O error!\nUnable to allocate memory for row buffer.\n" REVERSE "FATAL ERROR\n" RESET);
		return 1;
	}
	memset(row_buffer, '\0', sizeof(*row_buffer) * MAX_COLUMNS);
	unsigned long int triples = 0;
#ifndef NO_TIME
	unsigned long int start_time;
	unsigned long int end_time;
	struct timespec start_time_spec;
	struct timespec end_time_spec;
	long double total_time;
	long double triples_per_sec;
#endif
	for(int currentArg = 2; currentArg < argc; currentArg++)
	{
		if(getTableName(argv[currentArg], outputfilename, table) == -1)
		{
			printf(BOLD RED REVERSE BLINK "FATAL ERROR\n" RESET);
			return 1;
		}
		inputfile_fd = open(argv[currentArg], O_READ_FLAGS);
		if(inputfile_fd == -1)
		{
			printf(BOLD RED "File I/O error!\nUnable to open input file %s\n" REVERSE BLINK "FATAL ERROR\n" RESET, argv[currentArg]);
			return 1;
		}
		if(fstat(inputfile_fd, &inputfile_stat) == -1)
		{
			printf(BOLD RED "File I/O error!\nUnable to stat input file %s\n" REVERSE BLINK "FATAL ERROR\n" RESET, argv[currentArg]);
			return 1;
		}
		inputfile_map = mmap(NULL, inputfile_stat.st_size, PROT_READ, MAP_PRIVATE, inputfile_fd, 0);
		if(inputfile_map == MAP_FAILED)
		{
			printf(BOLD RED "Memory map error!\nUnable to create map for input file %s\n" REVERSE BLINK "FATAL ERROR\n" RESET, argv[currentArg]);
			return 1;
		}
		if(madvise(inputfile_map, inputfile_stat.st_size, MADV_SEQUENTIAL) == -1)
		{
			printf(BOLD RED "Page cache mode set error!\nUnable to set MADV_SEQUENTIAL page cache optimization mode.\n" REVERSE BLINK "FATAL ERROR\n" RESET);
			return 1;
		}
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
		if(getColumnNames(inputfile_map, table) == 1)
		{
			printf(BOLD RED REVERSE BLINK "FATAL ERROR\n" RESET);
			return 1;
		}
		if(getTableMetadata(schemafile_map, table) == 1)
		{
			printf(BOLD RED REVERSE BLINK "FATAL ERROR\n" RESET);
			return 1;
		}
		outputfile = fopen(outputfilename,"w");
		if(outputfile == NULL)
		{
			printf(BOLD RED "File I/O error!\nUnable to open output file %s\n" REVERSE BLINK "FATAL ERROR\n" RESET, argv[currentArg]);
			return 1;
		}
		printTableMetadata(table);
#ifndef NO_TIME
		clock_gettime(CLOCK_MONOTONIC, &start_time_spec);
#endif
		outputHeader(outputfile, table, &triples);
		outputTriples(outputfile, inputfile_map, table, row_buffer, &triples);
#ifndef NO_TIME
		clock_gettime(CLOCK_MONOTONIC, &end_time_spec);
		start_time = (start_time_spec.tv_sec * 1000000) + (start_time_spec.tv_nsec/1000);
		end_time = (end_time_spec.tv_sec * 1000000) + (end_time_spec.tv_nsec/1000);
		total_time = (end_time - start_time)/1000000.0;
		triples_per_sec = triples / total_time;
#endif
		// Clean up after this iteration:
#ifndef NO_TIME
		printf(BOLD GREEN "Finished %s, " BLUE "%lu" GREEN " total triples, " BLUE "%Lf" GREEN " seconds, " BLUE "%Lf" GREEN " triples/second.\n" RESET,outputfilename, triples, total_time, triples_per_sec);
#else
		printf(BOLD GREEN "Finished %s, " BLUE "%lu" GREEN " total triples.\n" RESET,outputfilename, triples);
#endif
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
#ifndef NO_TIME
		start_time = 0;
		end_time = 0;
		memset(&start_time_spec,'\0',sizeof(start_time_spec));
		memset(&end_time_spec,'\0',sizeof(end_time_spec));
#endif
	}
	free(table);
	free(row_buffer);
	return 0;
}
