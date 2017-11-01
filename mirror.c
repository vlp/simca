#include	<stdio.h>
#include	<strings.h>

#define t_byte unsigned char

FILE	*infile=NULL;
FILE	*outfile=NULL;

void	close_files()
{
    fclose(infile);
    fclose(outfile);
}

unsigned int mirror (unsigned int ch)
{
	unsigned int k;
	unsigned int tmp=0;

	ch=ch^0xFF;
	for (k=0;k<8;k++) {
		tmp=(tmp << 1);
		tmp=tmp+((ch >> k)&1);
	}

	return (tmp);
}

t_byte	i_getchar()
{
    unsigned int	r;
    int	res;

    res=fscanf(infile,"%hX",&r);
    if (res==EOF) {
	fprintf(stderr,"[!!]: End of file.\n");
	close_files();
	exit(-1);
    }
    if (res!=1) {
	fprintf(stderr,"[!!]: Wrong infile format. Two char hex words expected.\n");
	close_files();
	exit(-1);
    }

    return	r;
}

int	main(int argc, char *argv[])
{
    t_byte	tmp;
    int	written=0;
    
    fprintf(stderr,"\nSSI Hex Dump Convention Mirrorer v1.0\n\n");
    fprintf(stderr,"[usage]: %s [infile] [outfile]\n\n",argv[0]);
    
    if (argc>1) {
	infile=fopen(argv[1],"r");
	if (infile==NULL) {
	    fprintf(stderr,"[!!]: Error opening \"%s\".\n\n",argv[1]);
	    exit(-1);
	}
    } else {
	fprintf(stderr,"[!]: Stdin used as infile\n");
	infile=stdin;
    }

    if (argc==3) {
	outfile=fopen(argv[2],"w");
	if (outfile==NULL) {
	    fprintf(stderr,"[!!]: Error opening \"%s\".\n\n",argv[1]);
	    exit(-1);
	}
    } else {
	fprintf(stderr,"[!]: Stdout used as outfile\n\n");
	outfile=stdout;
    }

    while (!(feof(infile))) {
	tmp=i_getchar();
	fprintf(outfile,"%02X ",mirror(tmp));
	written++;
	if (written==16) {
	    written=0; 
	    fprintf(outfile,"\n");
	}
    }
    
    close_files();
    return 0;
}
