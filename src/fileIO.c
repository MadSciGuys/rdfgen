// Travis Whitaker 2013
// twhitak@its.jnj.com

#include <stdio.h>
#include <string.h>

#include <fileIO.h>

void OutputFilename(char *inputfilename, char *outputfilename, char *tablename)
{
	for(unsigned register int i=0; i<30;i++)
	{
		if(*(inputfilename+i) == '.' || *(inputfilename+i) == '\0')
		{
			*(tablename+i) = '\0';
			break;
		}
		*(tablename+i) = *(inputfilename+i);
	}
	*(tablename+30) = '\0';
	for(unsigned register int i=0;i<31;i++)
	{
		if(*(tablename+i) == '\0')
		{
			strcpy(outputfilename+i,".rdf");
			break;
		}
		*(outputfilename+i) = *(tablename+i);
	}
	*(outputfilename+34) = '\0';
	return;
}

void getLine(FILE *file, char *output, unsigned int maxlength)
{
	char cursor;
	for(unsigned register int i=0;i<maxlength;i++)
	{
		cursor = fgetc(file);
		if(cursor == '\n' || cursor == '\0' || cursor == EOF)
		{
			*(output+i) = '\0';
			break;
		}
		*(output+i) = cursor;
	}
	return;
}
