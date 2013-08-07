// Travis Whitaker 2013
// twhitak@its.jnj.com

#include <stdio.h>
#include <string.h>

#include <rdfgen.h>

void getColNames(char *firstline, char *columnames)
{
	unsigned register int j=0;
	unsigned register int jmp=30;
	for(unsigned register int i=0;i<31000;i++)
	{
		if(*(firstline+j) == '\0')
		{
			break;
		}
		else if(*(firstline+j) == ',' || *(firstline+j) == '\n')
		{
			*(columnames+i) = '\0';
			i = i+jmp;
			jmp = 30;
		}
		else
		{
			*(columnames+i) = *(firstline+j);
		}
		j++;
		jmp--;
	}
	return;

}
