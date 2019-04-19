#include <stdio.h>

#include "sbcast_output.h"

class sbcast_output{
public:
	void tracehead();


};

FILE * eventtraceFile=fopen ("trace2.tr","wt");
void sbcast_output::tracehead() {
	fprintf(eventtraceFile,"event\t");
	fprintf(eventtraceFile,"node\t");
}



int test(int a, int b){
	int c;
	fprintf(stdout,"sample%d,%d\n",a,b);
	c=a+b;	
	return c;
}


int test2(char event, int node){
	fprintf(eventtraceFile,"%c\t",event);
	fprintf(eventtraceFile,"%d\t",node);
}
