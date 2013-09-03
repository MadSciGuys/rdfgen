// Travis Whitaker 2013
// twhitak@its.jnj.com

// Standard I/O functions:
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <rdfgen/limits.h>
#include <rdfgen/interface.h>
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
					printf("Schema file error!\nUnexpected EOF at position 0x%x\n",_cursor);
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
					while(*(schemafile_map + _cursor) != '\n')
					{
						if(*(schemafile_map + _cursor) == '\0')
						{
							printf("Schema file error!\nUnexpected EOF at position 0x%x\n",_cursor);
					break;
				}
			}
		}
		else
		{
			printf("Schema file error!\nFound line that does not begin with ~, !, or tab\n",_cursor);
			return 1;
		}
	}
	printf("Schema file error!\nTable %s not found in schema file.\n",tableName);
	return 1;
}

// This function gets the primary key identifier. Cursor should be on the tab
// at the beginning of the line. Return 1 on error, 0 otherwise:
int schemaPI(char *schemafile_map, table_t *table, int *cursor)
{
	int _cursor = *cursor;
	char PIname[MAX_COLUMN_NAME_LEN + 1];
	char FKname[MAX_COLUMN_NAME_LEN + 1];

	_cursor++;
	if(*(schemafile_map + _cursor) != '!')
	{
		printf("Schema file error!\nNo Primary Identifier defined in schema file for table %s\n",table->tableName);
		return 1;
	}
	_cursor++;
	if(*(schemafile_map + _cursor) == '$')
	{
		table->primaryIdentifier = -1;
		_cursor++;
		if(*(schemafile_map + _cursor) != '\n')
		{
			printf("Schema file error!\nUnexpected char at position 0x%x\n",_cursor);
			return 1;
		}
		_cursor++;
		*cursor = _cursor;
		return 0;
	}
	for(int i = 0; i < MAX_COLUMN_NAME_LEN + 1; i++)
	{
		if(*(schemefile_map + _cursor) == '@' || *(schemafile_map + _cursor) == '\n')
		{
			PIname[i] = '\0';
			break;
		}
		else
		{
			PIname[i] = *(schemafile_map + _cursor);
		}
		_cursor++;
	}
}
