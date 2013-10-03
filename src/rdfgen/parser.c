// Travis Whitaker 2013
// twhitak@its.jnj.com

// Standard I/O functions:
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <rdfgen/limits.h>
#include <rdfgen/interface.h>
#include <rdfgen/parser.h>
#include <rdfgen/structure.h>
#include <rdfgen/color.h>



// This function seeks to the target table in the schema file, leaving the
// cursor on the tab after the newline. Return 1 upon error, 0 otherwise:
int schemaSeek(char *schemafile_map, char *tableName, unsigned long int *cursor)
{
	unsigned long int _cursor = 0; // Cursor for use in this scope.
	while(*(schemafile_map + _cursor) != '\0')
	{
		// Skip any lines that are comments or part of another table's spec:
		if(*(schemafile_map + _cursor) == '~' || *(schemafile_map + _cursor) == '\t')
		{
			while(*(schemafile_map + _cursor) != '\n')
			{
				if(*(schemafile_map + _cursor) == '\0')
				{
					printf(BOLD RED "Schema file error!\nUnexpected EOF at position 0x%x\n" RESET, (unsigned int)_cursor);
					return 1;
				}
				_cursor++;
			}
			_cursor++;
		}
		else if(*(schemafile_map + _cursor) == '#')
		{
			_cursor++;
			for(int i = 0; i < MAX_TABLE_NAME_LEN + 1; i++)
			{
				if(tableName[i] == '\0' && *(schemafile_map + _cursor) == '\n')
				{
					_cursor++;
					*cursor = _cursor;
					if(*(schemafile_map + *cursor) != '\t')
					{
						printf(BOLD RED "Schema file error!\nMalformed line at position 0x%x found by schemaSeek()\n" RESET, (unsigned int)(*cursor));
						return 1;
					}
					return 0;
				}
				else if(*(schemafile_map + _cursor) == tableName[i])
				{
					_cursor++;
				}
				else
				{
					while(*(schemafile_map + _cursor) != '\n')
					{
						if(*(schemafile_map + _cursor) == '\0')
						{
							printf(BOLD RED "Schema file error!\nUnexpected EOF at position 0x%x\n" RESET, (unsigned int)_cursor);
							break;
						}
						_cursor++;
					}
					_cursor++;
					break;
				}
			}
		}
		else
		{
			printf(BOLD RED "Schema file error!\nFound line that does not begin with ~, !, or tab\n" RESET);
			return 1;
		}
	}
	printf(BOLD RED "Schema file error!\nTable %s not found in schema file.\n" RESET, tableName);
	return 1;
}

// This function gets the primary key identifier. Cursor should be on the tab
// at the beginning of the line. Return 1 on error, 0 otherwise:
int schemaPI(char *schemafile_map, table_t *table, unsigned long int *cursor)
{
	int _cursor = *cursor;
	char PIname[MAX_COLUMN_NAME_LEN + 1] = {};
	char FKname[MAX_TABLE_NAME_LEN + 1] = {};

	table->primaryIdentifier = -2; // Set value OOR for error detection.
	_cursor++;

	if(*(schemafile_map + _cursor) != '!')
	{
		printf(BOLD RED "Schema file error!\nNo Primary Identifier defined in schema file for table %s\n" RESET, table->tableName);
		printf(BOLD YELLOW "Found [[[%c]]] instead...\n" RESET, *(schemafile_map + _cursor));
		return 1;
	}
	_cursor++;
	if(*(schemafile_map + _cursor) == '$')
	{
		table->primaryIdentifier = -1;
		_cursor++;
		if(*(schemafile_map + _cursor) != '\n')
		{
			printf(BOLD RED "Schema file error!\nUnexpected char at position 0x%x\n" RESET, (unsigned int)_cursor);
			return 1;
		}
		_cursor++;
		if(*(schemafile_map + _cursor) == '\0')
		{
			*cursor = _cursor;
			return 0;
		}
		*cursor = _cursor;
		return 0;
	}
	for(int i = 0; i < MAX_COLUMN_NAME_LEN + 1; i++)
	{
		if(*(schemafile_map + _cursor) == '@' || *(schemafile_map + _cursor) == '\n')
		{
			PIname[i] = '\0';
			break;
		}
		else if(*(schemafile_map + _cursor) == '\0')
		{
			printf(BOLD RED "Schema file error!\nUnexpected EOF at position 0x%x\n" RESET, _cursor);
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
		printf(BOLD RED "Schema file error!\nTable %s: Primary Identifier too long.\n" RESET, table->tableName);
		return 1;
	}
	for(int i = 0; i < MAX_COLUMNS; i++)
	{
		if(strncmp(PIname, table->columns[i].columnName, MAX_COLUMN_NAME_LEN + 1) == 0)
		{
			table->primaryIdentifier = i;
			table->columns[table->primaryIdentifier].type = req;
			break;
		}
	}
	if(table->primaryIdentifier == -2)
	{
		printf(BOLD RED "Schema file error!\nTable %s: Primary Identifier %s not found in table file.\n" RESET, table->tableName,PIname);
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
				printf(BOLD RED "Schema file error!\nUnexpected EOF at position 0x%x\n" RESET, (unsigned int)_cursor);
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
			printf(BOLD RED "Schema file error!\nTable %s: Foreign Key for Primary Identifier %s is too long.\n" RESET, table->tableName, table->columns[table->primaryIdentifier].columnName);
			return 1;
		}
		strncpy(table->columns[table->primaryIdentifier].FKtarget,FKname,MAX_TABLE_NAME_LEN + 1);
		_cursor++;
		*cursor = _cursor;
		return 0;
	}
	else
	{
	printf(BOLD RED "Schema file error!\nUnexpected char at position 0x%x\n" RESET, _cursor);
	return 1;
	}
}

// This function fetches the operator and arguments from one line of the schema
// file. Cursor should be on the tab preceeding the declaration. Return 1 on
// error, return 0 otherwise.
int schemaFetchLine(char *schemafile_map, char *op, char *arg1, char *arg2, unsigned long int *cursor)
{
	unsigned long int _cursor = *cursor;
	if(*(schemafile_map + _cursor) != '\t')
	{
		if(*(schemafile_map + _cursor) == '#' || *(schemafile_map + _cursor) == '\0')
		{
			*op = '#';
			arg1[0] = '\0';
			arg2[0] = '\0';
			return 0;
		}
		else
		{
			printf(BOLD RED "Schema file error!\nMalformed line at position 0x%x found by schemaFetchLine()\n" RESET, (unsigned int)_cursor);
			return 1;
		}
	}
	_cursor++;
	for(int i = 0; i < MAX_COLUMN_NAME_LEN; i++)
	{
		if(*(schemafile_map + _cursor) == '!')
		{
			printf(BOLD RED "Schema file error!\nUnexpected '!' at position 0x%x\n" RESET, (unsigned int)_cursor);
			return 1;
		}
		else if(*(schemafile_map + _cursor) == '\n')
		{
			printf(BOLD RED "Schema file error!\nUnexpected line feed at position 0x%x\n" RESET, (unsigned int)_cursor);
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
			printf(BOLD RED "Schema file error!\nInitial argument too long.\n" RESET);
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
			printf(BOLD RED "Schema file error!\nToo many arguments!\n" RESET);
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
			printf(BOLD RED "Schema file error!\nNew name for column %s is too long.\n" RESET, arg1);
			return 1;
		}
	}
	else if(*op == '?' || *op == '&')
	{
		if(arg2[MAX_FIELD_LEN] != '\0')
		{
			printf(BOLD RED "Schema file error!\nDefault value for column %s is too long.\n" RESET, arg1);
			return 1;
		}
	}
	else if(*op == '@')
	{
		if(arg2[MAX_TABLE_NAME_LEN] != '\0')
		{
			printf(BOLD RED "Schema file error!\nForeign Key target name for column %s is too long.\n" RESET, arg1);
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
		printf(BOLD RED "Column rename error!\nColumn name %s not found in table %s\n" RESET, arg1, table->tableName);
		return 1;
	}
	else
	{
		memset(table->columns[colnum].columnName, '\0', MAX_COLUMN_NAME_LEN + 1);
		strncpy(table->columns[colnum].columnName, arg2, MAX_COLUMN_NAME_LEN + 1);
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
		if(strncmp(arg1, table->columns[i].columnName, MAX_COLUMN_NAME_LEN + 1) == 0)
		{
			colnum = i;
			break;
		}
	}
	if(colnum == -1)
	{
		printf(BOLD RED "Column default value assignment error!\nColumn name %s not found in table %s\n" RESET, arg1, table->tableName);
		return 1;
	}
	else if(colnum == table->primaryIdentifier)
	{
		printf(BOLD RED "Column default value assignment error!\nColumn %s is the Primary Identifier and cannot have a default value.\n" RESET, arg1);
		return 1;
	}
	else
	{
		table->columns[colnum].type = req;
		memset(table->columns[colnum].defaultValue.data, '\0', MAX_FIELD_LEN + 1);
		strncpy(table->columns[colnum].defaultValue.data, arg2, MAX_FIELD_LEN + 1);
		return 0;
	}
}

// This function marks a column as required. Return 1 on error,
// return 0 otherwise.
int requireColumn(char *arg1, table_t *table)
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
		printf(BOLD RED "Column requirement spec error!\nColumn name %s not found in table %s\n" RESET, arg1, table->tableName);
		return 1;
	}
	else if(colnum == table->primaryIdentifier)
	{
		printf(BOLD RED "Column requirement spec error!\nColumn %s is the Primary Identifier and has a static type.\n" RESET, arg1);
		return 1;
	}
	else
	{
		table->columns[colnum].type = req;
		return 0;
	}
}

// This function sets a column's FK target. Return 1 on error, 0 otherwise.
int defineFK(char *arg1, char *arg2, table_t *table)
{
	int colnum = -1;
	for(int i = 0; i < table->totalColumns; i++)
	{
		if(strncmp(arg1, table->columns[i].columnName, MAX_COLUMN_NAME_LEN + 1) == 0)
		{
			colnum = i;
			break;
		}
	}
	if(colnum == -1)
	{
		printf(BOLD RED "Column FK target assignment error!\nColumn name %s not found in table %s\n" RESET, arg1, table->tableName);
		return 1;
	}
	else
	{
		memset(table->columns[colnum].FKtarget, '\0', MAX_TABLE_NAME_LEN + 1);
		strncpy(table->columns[colnum].FKtarget, arg2, MAX_TABLE_NAME_LEN + 1);
		return 0;
	}
}

// This function defines a new virtual column and it's default value. Return
// 1 on error, return 0 on success.
int defineVC(char *arg1, char *arg2, table_t *table)
{
	if((table->totalColumns + 1) > MAX_COLUMNS)
	{
		printf(BOLD RED "Virtual column definition error!\n" REVERSE BLINK "MAX_COLUMNS EXCEEDED!\n" RESET);
		return 1;
	}
	table->totalColumns = (table->totalColumns + 1);
	memset(&(table->columns[table->totalColumns - 1]) ,'\0', sizeof(column_t));
	table->columns[table->totalColumns - 1].type = virt;
	strncpy(table->columns[table->totalColumns - 1].columnName, arg1, MAX_COLUMN_NAME_LEN + 1);
	strncpy(table->columns[table->totalColumns - 1].defaultValue.data, arg2, MAX_TABLE_NAME_LEN + 1);
	return 0;
}
