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
			printf("Found newline, breaking...\n");
			break;
		}
		else if(*(firstline+j) == ',' || *(firstline+j) == '\n')
		{
			printf("Writing null at i=%d",i);
			*(columnames+i) = '\0';
			printf("Found comma or newline, jumping.\n");
			printf("Before jump, i = %d, j = %d, jmp = %d\n",i,j,jmp);
			i = i+jmp;
			jmp = 30;
			printf("Afer jump, i = %d, j = %d, jmp = %d\n",i,j,jmp);
		}
		else
		{
			char current = *(firstline+j);
			printf("Writing %c at i=%d.\n",current,i);
			*(columnames+i) = *(firstline+j);
		}
		j++;
		jmp--;
	}
	return;

}
