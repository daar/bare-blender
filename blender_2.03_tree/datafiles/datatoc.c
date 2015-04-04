 #include <stdio.h>

int main(int argc, char**argv) {
	FILE *fpin,  *fpout;
	char cname[256];
	char sizest[256];
	long size;
	int i;
	
	if (argc<1) {
			printf ("Usage: datatoc <data_file>\n");
			exit(1);
	}
	
	fpin= fopen(argv[1], "rb");
	if (!fpin) {
			printf ("Unable to open input <%s>\n", argv[1]);
			exit(1);
	}
	
	fseek (fpin, 0L,  SEEK_END);
	size= ftell(fpin);
	fseek (fpin, 0L,  SEEK_SET);

	if (argv[1][0]=='.') argv[1]++;

	sprintf(cname, "%s.c", argv[1]);
	printf ("Making C file <%s>\n", cname);

	for (i=0; i < strlen(argv[1]); i++)
		if (argv[1][i]=='.') argv[1][i]='_';

	sprintf(sizest, "%d", (int)size);
	printf ("Input filesize is %d, Output size should be %d\n", size, ((int)size)*4 + strlen("/* DataToC output of file <> */\n\n") + strlen("char datatoc_[]= {\"") + strlen ("\"};\n") + (strlen(argv[1])*3) + strlen(sizest) + strlen("int datatoc__size= ;\n") +(((int)(size/256)+1)*5));
	
	fpout= fopen(cname, "w");
	if (!fpout) {
			printf ("Unable to open output <%s>\n", cname);
			exit(1);
	}
	
	fprintf (fpout, "/* DataToC output of file <%s> */\n\n",argv[1]);
	fprintf (fpout, "int datatoc_%s_size= %s;\n", argv[1], sizest);
	/*
	fprintf (fpout, "char datatoc_%s[]= {\"", argv[1]);

	while (size--) {
		if(size%256==0)
			fprintf(fpout, "\" \\\n\"");
			
		fprintf (fpout, "\\x%02x", getc(fpin));
	}

	fprintf (fpout, "\"};\n");
	*/
	
	fprintf (fpout, "char datatoc_%s[]= {\n", argv[1]);
	while (size--) {
		if(size%32==31)
			fprintf(fpout, "\n");
			
		/* fprintf (fpout, "\\x%02x", getc(fpin)); */
		fprintf (fpout, "%3d,", getc(fpin));
	}
	
	fprintf (fpout, "\n};\n");
	
	fclose(fpin);
	fclose(fpout);
	
	return 0;
}
