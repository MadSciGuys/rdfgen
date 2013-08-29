// Travis Whitaker 2013
// twhitak@its.jnj.com

// Standar I/O functions:
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// rdfgen structures and functions:
#include <rdfgen.h>



// This function gets the table's name from the input file name, puts it into
// the table metadata struct, then computes the output file name. Returns -1
// on error, 0 on success.
int getTableName( char* inputfilename, char* outputfilename, table_t* table )
{
	// Iterate over the file name to get the table name:
	for( int i = 0; i < MAX_TABLE_NAME_LEN + 1; i++ )
	{
		if( *( inputfilename + i ) == '.' ) // Beginning of extension.
		{
			table->tableName[i] = '\0';
			break;
		}
		else // Otherwise copy the character.
		{
			table->tableName[i] = *( inputfilename + i );
		}
	}
	// If the last char isn't null, then the table name was too long:
	if( table->tableName[MAX_TABLE_NAME_LEN] != '\0' )
	{
		printf( "File name error!\nThe table name in file %s is too long.\n", inputfilename );
		return -1;
	}
	// Now make the output file name:
	for( int i = 0; i < MAX_TABLE_NAME_LEN + 1; i++ )
	{
		if( table->tableName[i] == '\0' ) // Now write the extension.
		{
			snprintf( ( outputfilename + i ), RDF_EXT_LEN + 1, RDF_EXT );
			break;
		}
		else // Otherwise copy the character.
		{
			*( outputfilename + i ) = table->tableName[i];
		}
	}
	return 0;
}
