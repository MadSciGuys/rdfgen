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
	char FKname[MAX_TABLE_NAME_LEN + 1];

	table->primaryIdentifier = -2; // Set value OOR for error detection.

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
		if(*(schemafile_map + _cursor) == '\0')
		{
			*cursor = _cursor;
			return 0;
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
		else if(*(schemafile_map + _cursor) == '\0')
		{
			printf("Schema file error!\nUnexpected EOF at position 0x%x\n",_cursor);
			return 1;
		}
		else
		{
			PIname[i] = *(schemafile_map + _cursor);
		}
		_cursor++;
	}
	if(PIname[MAX_COLUMN_NAME_LEN] != '\0')
	{
		printf("Schema file error!\nTable %s: Primary Identifier too long.\n",table->tableName);
		return 1;
	}
	for(int i = 0; i < MAX_COLUMNS; i++)
	{
		if(strncmp(PIname,table->columns[i],MAX_COLUMN_NAME_LEN + 1) == 0)
		{
			table->primaryIdentifier = i;
			break;
		}
	}
	if(table->primaryIdentifier == -2)
	{
		printf("Schema file error!\nTable %s: Primary Identifier %s not found in table file.\n"table->tableName,PIname);
		return 1;
	}
	if(*(schemafile_map + _cursor) == '\n')
	{
		_cursor++;
		*cursor = _cursor;
		return 0;
	}
	else if(*(schemafile_map + _cursor) == '@')
	{
		_cursor++;
		for(int i = 0; i < MAX_TABLE_NAME_LEN; i++)
		{
			if(*(schemafile_map + _cursor) == '\n')
			{
				FKname[i] = '\0';
				break;
			}
			else if(*(schemafile_map + _cursor) == '\0')
			{
				printf("Schema file error!\nUnexpected EOF at position 0x%x\n",_cursor);
				return 1;
			}
			else
			{
				FKname[i] = *(schemafile_map + _cursor);
			}
			_cursor++;
		}
		if(FKname[MAX_TABLE_NAME_LEN] != '\0')
		{
			printf("Schema file error!\nTable %s: Foreign Key for Primary Identifier %s is too long.\n", table->tableName, table->columns[table->primaryIdentifier].columnName);
			return 1;
		}
		strncpy(table->columns[table->primaryIdentifier].FKtarget,FKname,MAX_TABLE_NAME_LEN + 1);
		_cursor++;
		*cursor = _cursor;
		return 0;
	}
	else
	{
	printf("Schema file error!\nUnexpected char at position 0x%x\n",_cursor);
	return 1
	}
}

// This function fetches the operator and arguments from one line of the schema
// file. Cursor should be on the tab preceeding the declaration. Return 1 on
// error, return 0 otherwise.
int schemaFetchLine(char *schemafile_map, char *op, char *arg1, char *arg2, int *cursor)
{
	int _cursor = *cursor;
	if(*(schemafile_map + _cursor) != '\t')
	{
		if(*(schemafile_map + _cursor) == '#' || *(schemafile_map + _cusor) == '\0')
		{
			*op = '#';
			arg1[0] = '\0';
			arg2[0] = '\0';
			return 0
		}
		else
		{
			printf("Schema file error!\nMalformed line at position 0x%x\n",_cursor);
			return 1;
		}
	}
	_cursor++;
	for(int i = 0; i < MAX_COLUMN_NAME_LEN; i++)
	{
		if(*(schemafile_map + _cursor) == '!')
		{
			printf("Schema file error!\nUnexpected '!' at position 0x%x\n",_cursor);
			return 1;
		}
		else if(*(schemafile_map + _cursor) == '\n')
		{
			printf("Schema file error!\nUnexpected line feed at position 0x%x\n",_cursor);
			return 1;
		}
		else if(*(schemafile_map + _cursor) == '>' || *(schemafile_map + _cursor) == '?' || *(schemafile_map + _cursor) == '*' || *(schemafile_map + _cursor) == '@' || *(schemafile_map + _cursor) == '&')
		{
			*op = *(schemafile_map + _cursor);
			arg1[i] = '\0';
			break;
		}
		else
		{
			arg1[i] = *(schemafile_map + _cursor);
		}
		_cursor++;
	}
	if(*op == '*')
	{
		arg2[0] = '\0';
		if(arg1[MAX_COLUMN_NAME_LEN] == '\0')
		{
			if(*(schemafile_map + _cursor) == '\0')
			{
				*cursor = _cursor;
				return 0;
			}
			_cursor = _cursor + 2;
			*cursor = _cursor;
			return 0;
		}
		else
		{
			printf("Schema file error!\nInitial argument too long.\n");
			return 1;
		}
	}
	_cursor++;
	for(int i = 0; i < MAX_FIELD_LEN; i++)
	{
		if(*(schemafile_map + _cursor) == '\n' || *(schemafile_map + _cursor) == '\0')
		{
			arg2[i] = '\0';
			break;
		}
		else if(*(schemafile_map + _cursor) == '!' || *(schemafile_map + _cursor) == '>' || *(schemafile_map + _cursor) == '?' || *(schemafile_map + _cursor) == '*' || *(schemafile_map + _cursor) == '@' || *(schemafile_map + _cursor) == '&')
		{
			printf("Schema file error!\nToo many arguments!\n");
			return 1;
		}
		else
		{
			arg2[i] = *(schemafile_map + _cursor);
		}
		_cursor++;
	}
	if(*op == '>')
	{
		if(arg2[MAX_COLUMN_NAME_LEN] != '\0')
		{
			printf("Schema file error!\nNew name for column %s is too long.\n",arg1);
			return 1;
		}
	}
	else if(*op == '?' || *op == '&')
	{
		if(arg2[MAX_FIELD_LEN] != '\0')
		{
			printf("Schema file error!\nDefault value for column %s is too long.\n",arg1);
			return 1;
		}
	}
	else if(*op == '@')
	{
		if(arg2[MAX_TABLE_NAME_LEN] != '\0')
		{
			printf("Schema file error!\nForeign Key target name for column %s is too long.\n",arg1);
			return 1;
		}
	}
	if(*(schemafile_map + _cursor) == '\0')
	{
		*cursor = _cursor;
		return 0;
	}
	else
	{
		_cursor++;
		*cursor = _cursor;
		return 0;
	}
}

// This function renames a column. Return 1 on error, 0 otherwise.
int renameColumn(char *arg1, char *arg2, table_t *table)
{
	int colnum = -1;
	for(int i = 0; i < table->totalColumns; i++)
	{
		if(strncmp(arg1,table->columns[i].columnName,MAX_COLUMN_NAME_LEN + 1) == 0)
		{
			colnum = i;
			break;
		}
	}
	if(colnum == -1)
	{
		printf("Column rename error!\nColumn name %s not found in table %s\n",arg1,table->tableName);
		return 1;
	}
	else
	{
		memset(table->columns[colnum].columnName,'\0',MAX_COLUMN_NAME_LEN + 1);
		strncpy(table->columns[colnum].columnName,arg1,MAX_COLUMN_NAME_LEN + 1);
		return 0;
	}
}

// This function defines a default value for a column. Return 1 on error,
// return 0 otherwise.
int defineDV(char *arg1, char *arg2, table_t *table)
{
	int colnum = -1;
	for(int i = 0; i < table->totalColumns; i++)
	{
		if(strncmp(arg1,table->columns[i].columnName,MAX_COLUMN_NAME_LEN + 1) == 0)
		{
			colnum = i;
			break;
		}
	}
	if(colnum == -1)
	{
		printf("Column default value assignment error!\nColumn name %s not found in table %s\n",arg1,table->tableName);
		return 1;
	}
	else
	{
		memset(table->columns[colnum].defaultValue,'\0',MAX_FIELD_LEN + 1);
		strncpy(table->columns[colnum].defaultValue,arg2,MAX_FIELD_LEN + 1);
		return 0;
	}
}

// This function marks a column as required. Return 1 on error,
// return 0 otherwise.
int requireColumn(char *arg1, table_t *table)
{
	int colnum = -1;
	for(int i = 0; i < table->totalColumsn; i++)
	{
		if(strncmp(arg1,table->columns[i].columnName,MAX_COLUMN_NAME_LEN + 1) == 0)
		{
			colnum = i;
			break;
		}
	}
	if(colnum == -1)
	{
		printf("Column requirement spec error!\nColumn name %s not found in table %s\n",arg1,table->tableName);
		return 1;
	}
	else
	{
		table->columns[colnum].type = req;
		return 0;
	}
}
