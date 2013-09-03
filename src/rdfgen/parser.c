// Travis Whitaker 2013
// twhitak@its.jnj.com

// Standard I/O functions:
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <rdfgen/limits.h>
#include <rdfgen/parser.h>



// This function seeks to the target table in the schema file, leaving the
// cursor on the tab after the newline. Return 1 upon error, 0 otherwise:
int schemaSeek(char *schemafile_map, char *tableName, int *cursor)
{
	int _cursor = 0; // Cursor for use in this scope.
	while(*(schemafile_map + _cursor) != '\0')
	{
		// Skip any lines that are comments or part of another table's spec:
		if(*(schemafile_map + _cursor) == '~' || *(schemafile_map + _cursor) == '\t')
		{
			while(*(schemafile_map + _cursor) != '\n')
			{
				if(*(schemafile_map + _cursor) == '\0')
				{
					printf("Schema file error!\nUnexpected line feed at position 0x%x\n",_cursor);
					return 1;
				}
				_cursor++;
			}
		}
		else if(*(schemafile_map + _cursor) == '#')
		{
			_cursor++;
			for(int i = 0; i < MAX_TABLE_NAME_LEN + 1; i++)
			{
				if(*(schemafile_map + _cursor) == tableName[i])
				{
					_cursor++;
				}
				else if(*(schemafile_map + _cursor) == '\n' && tableName[i] == '\0')
				{
					_cursor++;
					*cursor = _cursor;
					if(*(schemafile_map + *cursor) != '\t')
					{
						printf("Schema file error!\nMalformed line at position 0x%x\n",*cursor);
						return 1;
					}
					return 0;
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			printf("Schema file error!\nMalformed line at position 0x%x\n",_cursor);
			return 1;
		}
	}
	printf("Schema file error!\nMalformed line at position 0x%x\n",_cursor);
	return 1;
}
