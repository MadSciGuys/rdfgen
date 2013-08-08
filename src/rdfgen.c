// Travis Whitaker 2013
// twhitak@its.jnj.com

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rdfgen.h>

int getColNames(char *firstline, char *columnames)
{
	int colnum = 1;
	unsigned register int j=0;
	int jmp=29;
	for(unsigned register int i=0;i<31000;i++)
	{
		if(*(firstline+j) == '\0')
		{
			break;
		}
		else if(*(firstline+j) == ',' || *(firstline+j) == '\n')
		{
			i = i+jmp;
			jmp = 30;
			colnum++;
		}
		else
		{
			*(columnames+i) = *(firstline+j);
		}
		j++;
		if(jmp < 0)
		{
			printf("ERROR: Name of column %d greater than 30 chars.\n",colnum);
			free(columnames);
			columnames = NULL;
			return -1;
		}
		jmp--;
	}
	return colnum;

}
