// Travis Whitaker 2013
// twhitak@its.jnj.com

// Standar I/O functions:
#include <stdio.h>

// rdfgen structures and functions:
#include <rdfgen.h>



// This function gets the table's name from the input file name, puts it into
// the table metadata struct, then computes the output file name. Returns -1
// on error, 0 on success.
int getTableName(char *inputfilename, char *outputfilename, table_t *table)
{
	// Iterate over the file name to get the table name:
	for(int i=0;i<MAX_TABLE_NAME_LEN+1;i++)
	{
		if(*(inputfilename+i) == '.') // Beginning of extension.
		{
			table->tablename[i] = '\0'
			break;
		}
		else
		{
			table->tablename[i] = *(inputfilename+i);
		}
	}
	// If the last char isn't null, then the table name was too long:
	if(*(inputfilename+MAX_TABLE_NAME_LEN) != '\0')
	{
		printf("File name error!\nThe table's name in file %s is too long.\n",inputfilename);
		return -1;
	}
