#include <stdio.h>
#include <string.h>

#include <fileIO.h>

void OutputFilename(char *inputfilename, char *outputfilename, char *tablename)
{
	for(unsigned register int i=0; i<1000;i++)
	{
		if(*(inputfilename+i) == '.' || *(inputfilename+i) == '\0')
		{
			*(tablename+i) = '\0';
			break;
		}
		*(tablename+i) = *(inputfilename+i);
	}
	for(unsigned register int i=0;i<1000;i++)
	{
		if(*(tablename+i) == '\0')
		{
			strcpy(outputfilename+i,".rdf");
			break;
		}
		*(outputfilename+i) = *(tablename+i);
	}
	return;
}
