#include	<stdio.h>
#include	<strings.h>

#define	program_version		"v0.1"

#define t_byte unsigned char

typedef	struct	t_atr {
     t_byte TS;
     t_byte T0;
     t_byte TA1;
     t_byte TB1;
     t_byte TC1;
     t_byte TD1;
     t_byte TA2;
     t_byte TB2;
     t_byte TC2;
     t_byte TDi;
     // ...
     t_byte historical_ch[15];
     t_byte TCK; 
     
} t_atr;

FILE	*infile=NULL;
FILE	*outfile=NULL;

t_byte	last_response_data_command=0;
t_byte	selected_file[2];

void	close_files()
{
    fclose(infile);
    fclose(outfile);
}

void	print_line()
{
    fprintf(outfile,"---============-----------------------------------------------------------------\n");
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

void	ReadATR()
{
    int	x;
    int	historical_ch_count;
    int interface_character_present[4];
    t_atr	atr;

    
    atr.TS=i_getchar();
    if ((atr.TS!=0x3F)&&(atr.TS!=0x3B)) {
	fprintf(outfile,"Garbage:");
	while ((atr.TS!=0x3F)&&(atr.TS!=0x3B)) {
	    fprintf(outfile," %02X",atr.TS);
	    atr.TS=i_getchar();
	}
	fprintf(outfile,"\n\n");
    }
    atr.T0=i_getchar();
    interface_character_present[0]=((atr.T0 &  16)== 16);	//TA1
    interface_character_present[1]=((atr.T0 &  32)== 32);	//TB1
    interface_character_present[2]=((atr.T0 &  64)== 64);	//TC1
    interface_character_present[3]=((atr.T0 & 128)==128);	//TD1
    historical_ch_count=( atr.T0 & 15 );
    
    if (interface_character_present[0]) atr.TA1=i_getchar();
    if (interface_character_present[1]) atr.TB1=i_getchar();
    if (interface_character_present[2]) atr.TC1=i_getchar();
    if (interface_character_present[3]) atr.TD1=i_getchar();
    //TODO-FIXME: more characters may be present
    
    for(x=0;x<historical_ch_count;x++) {
	atr.historical_ch[x]=i_getchar();
    }

//TODO-FIXME    atr.TCK=i_getchar();
    
    print_line();
    fprintf(outfile,"ATR: %02X %02X",atr.TS,atr.T0);
    if (interface_character_present[0]) fprintf (outfile," %02X",atr.TA1);
    if (interface_character_present[1]) fprintf (outfile," %02X",atr.TB1);
    if (interface_character_present[2]) fprintf (outfile," %02X",atr.TC1);
    if (interface_character_present[3]) fprintf (outfile," %02X",atr.TD1);
    for(x=0;x<historical_ch_count;x++) {
	fprintf (outfile," %02X",atr.historical_ch[x]);
    }
    fprintf(outfile,"\n\n(*) ATR analyze\n");
//    fprintf (outfile," %02X\n\n",atr.TCK);
    fprintf(outfile,"\tInitial character TS=%02X\n",atr.TS);
    if (atr.TS==0x3F) fprintf(outfile,"\t\tInverse convention\n"); else fprintf(outfile,"\t\tDirect convention\n");
    fprintf(outfile,"\tFormat character T0=%02X\n",atr.T0);
    fprintf(outfile,"\t\t");
    if (interface_character_present[0]) fprintf (outfile,"TA1 ");
    if (interface_character_present[1]) fprintf (outfile,"TB1 ");
    if (interface_character_present[2]) fprintf (outfile,"TC1 ");
    if (interface_character_present[3]) fprintf (outfile,"TD1 ");
    fprintf(outfile,"global interface character(s) defined\n");
    fprintf(outfile,"\t\t%i historical characters present\n",historical_ch_count);
    if (interface_character_present[0]) {
	fprintf(outfile,"\tGlobal interface character TA1=%02X\n",atr.TA1);
    }
    if (interface_character_present[1]) {
	fprintf(outfile,"\tGlobal interface character TB1=%02X\n",atr.TB1);
    }

    if (interface_character_present[2]) {
	fprintf(outfile,"\tGlobal interface character TC1=%02X\n",atr.TC1);
    }

    if (interface_character_present[3]) {
	fprintf(outfile,"\tGlobal interface character TD1=%02X\n",atr.TD1);
    }
    fprintf(outfile,"\tHistorical characters:");
    for(x=0;x<historical_ch_count;x++) {
	fprintf (outfile," %02X",atr.historical_ch[x]);
    }
    fprintf(outfile,"\n");
    //TODO: TCK
    
    fprintf(outfile,"\n");
    print_line();
}

char	*get_cmd_name(int ins)
{
    switch (ins) {
	case	0xA4	:	return "SELECT";
	case	0xF2	:	return "STATUS";
	case	0xB0	:	return "READ BINARY";
	case	0xD6	:	return "UPDATE BINARY";
	case	0xB2	:	return "READ RECORD";
	case	0xDC	:	return "UPDATE RECORD";
	case	0xA2	:	return "SEEK";
	case	0x32	:	return "INCREASE";
	case	0x20	:	return "VERIFY CHV";
	case	0x24	:	return "CHANGE CHV";
	case	0x26	:	return "DISABLE CHV";
	case	0x28	:	return "ENABLE CHV";
	case	0x2C	:	return "UNBLOCK CHV";
	case	0x04	:	return "INVALIDATE";
	case	0x44	:	return "REHABILITATE";
	case	0x88	:	return "RUN GSM ALGORITHM";
	case	0xFA	:	return "SLEEP";
	case	0xC0	:	return "GET RESPONSE";
	case	0x10	:	return "TERMINAL PROFILE";
	case	0xC2	:	return "ENVELOPE";
	case	0x12	:	return "FETCH";
	case	0x14	:	return "TERMINAL RESPONSE";
    }
    return "UNKNOWN COMMAND";
}
void	print_status(int sw1, int sw2)
{
    switch (sw1) {
	case	0x90	:
	    fprintf(outfile,"Normal ending of the command");
	    break;
	case	0x9F	:
	    fprintf(outfile,"SIM has response data with length %02X",sw2);
	    break;
	case	0x94	:
	    switch (sw2) {
		case	0x00	:
		    fprintf(outfile,"No EF selected");
		    break;
		case	0x02	:
		    fprintf(outfile,"Out of range / invalid address");
		    break;
		case	0x04	:
		    fprintf(outfile,"File ID not found / Pattern not found");
		    break;
		case	0x08	:
		    fprintf(outfile,"File is inconsistent with the command");
		    break;
	    }
	    break;
	case	0x98	:
	    switch (sw2) {
		case	0x02	:
		    fprintf(outfile,"No CHV initialized");
		    break;
		case	0x04	:
		    fprintf(outfile,"Access condition not fillfulled");
		    break;
		case	0x08	:
		    fprintf(outfile,"Error in contradiction with CHV status");
		    break;
		case	0x10	:
		    fprintf(outfile,"Error in contradiction with invalidation status");
		    break;
		case	0x40	:
		    fprintf(outfile,"Access condition not fillfulled, no attempts left");
		    break;
		case	0x50	:
		    fprintf(outfile,"Increase cannot be performed, Max value reached");
		    break;		
	    }
	    break;
	case	0x67	:
	    fprintf(outfile,"Incorrect parameter P3, correct length %02X",sw2);
	    break;
	case	0x6B	:
	    fprintf(outfile,"Incorrect parameter P1 or P2");
	    break;
	case	0x6D	:
	    fprintf(outfile,"Unknown instruction code given in the command");
	    break;
	case	0x6E	:
	    fprintf(outfile,"Wrong instruction class given in the command");
	    break;
	case	0x6F	:
	    fprintf(outfile,"Technical problem with no diagnostic given");
	    break;
    }
}

int	bs(t_byte byt,int bit)
{
    const t_byte	bubu[8] = {1,2,4,8,16,32,64,128};

    return	((byt & bubu[bit]) == bubu[bit]);
}

void	Analyze_TP(t_byte par[255],int p3)
{
    fprintf(outfile,"\n");
    if (p3>=1) {
	if (bs(par[0],0)) fprintf(outfile,"\tProfile download\n");
	if (bs(par[0],1)) fprintf(outfile,"\tSMS-PP data download\n");
	if (bs(par[0],2)) fprintf(outfile,"\tCell Broadcast data download\n");
	if (bs(par[0],3)) fprintf(outfile,"\tMenu selection\n");
	if (bs(par[0],4)) fprintf(outfile,"\t'9EXX' response code for SIM data download error\n");
	if (bs(par[0],5)) fprintf(outfile,"\tTimer expiration\n");
	if (bs(par[0],6)) fprintf(outfile,"\tUSSD string object supported in Call control\n");
	if (bs(par[0],7)) fprintf(outfile,"\tEnvelope Call Control always sent to SIM during automatic redial mode\n");
    }

    if (p3>=2) {
	if (bs(par[1],0)) fprintf(outfile,"\tCommand result\n");
	if (bs(par[1],1)) fprintf(outfile,"\tCall control by SIM\n");
	if (bs(par[1],2)) fprintf(outfile,"\tCell identity included in Call Control by SIM\n");
	if (bs(par[1],3)) fprintf(outfile,"\tMO short message control by SIM\n");
	if (bs(par[1],4)) fprintf(outfile,"\tHandling of the alpha identifier according to 9.1.3 in GSM 11.14\n");
	if (bs(par[1],5)) fprintf(outfile,"\tUCS2 Entry supported\n");
	if (bs(par[1],6)) fprintf(outfile,"\tUCS2 Display supported\n");
	if (bs(par[1],7)) fprintf(outfile,"\tDisplay of the extension text\n");
    }

    if (p3>=3) {
	if (bs(par[2],0)) fprintf(outfile,"\tProactive SIM: DISPLAY TEXT\n");
	if (bs(par[2],1)) fprintf(outfile,"\tProactive SIM: GET INKEY\n");
	if (bs(par[2],2)) fprintf(outfile,"\tProactive SIM: GET INPUT\n");
	if (bs(par[2],3)) fprintf(outfile,"\tProactive SIM: MORE TIME\n");
	if (bs(par[2],4)) fprintf(outfile,"\tProactive SIM: PLAY TONE\n");
	if (bs(par[2],5)) fprintf(outfile,"\tProactive SIM: POLL INTERVAL\n");
	if (bs(par[2],6)) fprintf(outfile,"\tProactive SIM: POLLING OFF\n");
	if (bs(par[2],7)) fprintf(outfile,"\tProactive SIM: REFRESH\n");
    }

    if (p3>=4) {
	if (bs(par[3],0)) fprintf(outfile,"\tProactive SIM: SELECT ITEM\n");
	if (bs(par[3],1)) fprintf(outfile,"\tProactive SIM: SEND SHORT MESSAGE\n");
	if (bs(par[3],2)) fprintf(outfile,"\tProactive SIM: SEND SS\n");
	if (bs(par[3],3)) fprintf(outfile,"\tProactive SIM: SEND USSD\n");
	if (bs(par[3],4)) fprintf(outfile,"\tProactive SIM: SET UP CALL\n");
	if (bs(par[3],5)) fprintf(outfile,"\tProactive SIM: SET UP MENU\n");
	if (bs(par[3],6)) fprintf(outfile,"\tProactive SIM: PROVIDE LOCAL INFORMATION (MCC,MNC,LAC,Cell ID & IMEI)\n");
	if (bs(par[3],7)) fprintf(outfile,"\tProactive SIM: PROVIDE LOCAL INFORMATION (NMR)\n");
    }
    if (p3>=5) {
	if (bs(par[4],0)) fprintf(outfile,"\tProactive SIM: SET UP EVENT LIST\n");
	if (bs(par[4],1)) fprintf(outfile,"\tEvent: MT call\n");
	if (bs(par[4],2)) fprintf(outfile,"\tEvent: Call connected\n");
	if (bs(par[4],3)) fprintf(outfile,"\tEvent: Call disconnected\n");
	if (bs(par[4],4)) fprintf(outfile,"\tEvent: Location status\n");
	if (bs(par[4],5)) fprintf(outfile,"\tEvent: User activity\n");
	if (bs(par[4],6)) fprintf(outfile,"\tEvent: Idle screen available\n");
	if (bs(par[4],7)) fprintf(outfile,"\tEvent: Card reader status\n");
    }
    if (p3>=6) {
	if (bs(par[5],0)) fprintf(outfile,"\tEvent: Language selection\n");
	if (bs(par[5],1)) fprintf(outfile,"\tEvent: Browser Termination\n");
	if (bs(par[5],2)) fprintf(outfile,"\tEvent: Data available\n");
	if (bs(par[5],3)) fprintf(outfile,"\tEvent: Channel status\n");
	//others are RFU in GSM 11.14 v8.3.0
    }
    //TODO
}


void	read_block(t_byte par[255],int count,int offset)
{
    int x;

    for(x=0;x<count;x++) {
    	par[offset+x]=i_getchar();
	fprintf(outfile," %02X",par[offset+x]);
    }
}

void	ReadCommand()
{
    t_byte	cla;
    t_byte	ins;
    t_byte	p1;
    t_byte	p2;
    t_byte	p3;
    t_byte	sim_ins_rply;
    t_byte	params[255];
    t_byte	sw1;
    t_byte	sw2;
    
    cla=i_getchar();
    ins=i_getchar();
    p1=i_getchar();
    p2=i_getchar();
    p3=i_getchar();
    
    if (cla!=0xA0) {
	fprintf(stderr,"[!!]: Invalid command CLA \"%02X\". \"A0\" expected.\n",cla);
	close_files();
	exit(-1);
    }

    fprintf(outfile,"ME: %02X %02X %02X %02X %02X - (%s command)\n",cla,ins,p1,p2,p3,get_cmd_name(ins));
    sim_ins_rply=i_getchar();
    if (sim_ins_rply==ins) 
	fprintf(outfile,"SIM: %02X - (Ins echo)\n",sim_ins_rply);
    else {
	fprintf(outfile,"SIM: %02X - (Wrong command response)\n",sim_ins_rply);
	fprintf(stderr,"[!!]: Invalid SIM command reply \"%02X\". \"%02X\" expected.\n",sim_ins_rply,ins);
	close_files();
	exit(-1);
    }
    fprintf(outfile,"\n(Processing command %s)\n\n",get_cmd_name(ins));
    
    switch (ins) {
	case	0xA4	:
	    fprintf(outfile,"ME:");
	    read_block(params,2,0);
	    fprintf(outfile," - (File %02X%02X)\n",params[0],params[1]);
	    selected_file[0]=params[0];
	    selected_file[1]=params[1];
	    break;
	case	0xF2	:
	    fprintf(outfile,"ME:");
	    read_block(params,p3,0);
	    fprintf(outfile," - (Status info)\n",params[0],params[1]);
	    break;
	case	0xB0	:
	    fprintf(outfile,"SIM:");
	    read_block(params,p3,0);
	    fprintf(outfile," - (Data of file %02X%02X at offset %02X%02X)\n",selected_file[0],selected_file[1],p1,p2);
	    break;
	case	0xD6	:
	    fprintf(outfile,"ME:");
	    read_block(params,p3,0);
	    fprintf(outfile," - (Data to be written at offset %02X%02X of file %02X%02X)\n",p1,p2,selected_file[0],selected_file[1]);
	    break;
	case	0xB2	:
	    fprintf(outfile,"SIM:");
	    read_block(params,p3,0);
	    switch (p2) {
		case	0x02	:
		    fprintf(outfile," - (Data from next record of file %02X%02X)\n",selected_file[0],selected_file[1]);
		    break;
		case	0x03	:
		    fprintf(outfile," - (Data from previous record of file %02X%02X)\n",selected_file[0],selected_file[1]);
		    break;
		case	0x04	:
		    if (p1==0)
			fprintf(outfile," - (Data from current record of file %02X%02X)\n",selected_file[0],selected_file[1]);
		    else
			fprintf(outfile," - (Data of record %i of file %02X%02X)\n",p1,selected_file[0],selected_file[1]);
		    break;
	    }
	    break;
	case	0xDC	:
	    fprintf(outfile,"ME:");
	    read_block(params,p3,0);
	    switch (p2) {
		case	0x02	:
		    fprintf(outfile," - (Data to be written to next record of file %02X%02X)\n",selected_file[0],selected_file[1]);
		    break;
		case	0x03	:
		    fprintf(outfile," - (Data to be written to previous record of file %02X%02X)\n",selected_file[0],selected_file[1]);
		    break;
		case	0x04	:
		    if (p1==0)
			fprintf(outfile," - (Data to be written to current record of file %02X%02X)\n",selected_file[0],selected_file[1]);
		    else
			fprintf(outfile," - (Data to be written to record %i of file %02X%02X)\n",p1,selected_file[0],selected_file[1]);
		    break;
	    }
	    break;
	case	0xA2	:	//SEEK - fully implemented
	    fprintf(outfile,"ME:");
	    read_block(params,p3,0);
	    fprintf(outfile," - (Pattern to be seeked in  file %02X%02X)\n",selected_file[0],selected_file[1]);
	    fprintf(outfile,"\t(Seek type: %i - ",((p2 & 0xF0)+1));
	    if ((p2 & 0xF0)==0) fprintf(outfile,"Record pointer is set to found record");
	    if ((p2 & 0xF0)==1) fprintf(outfile,"Record pointer is set to found record. Returns record number.");
	    fprintf(outfile,")\n\t(Mode: %i - ",(p2 & 0x0F));
	    switch (p2 & 0x0F) {
		case	0 :	fprintf(outfile,"from teh beggining forward)\n");
		case	1 :	fprintf(outfile,"from the end backward)\n");
		case	2 :	fprintf(outfile,"from the next location forward)\n");
		case	3 :	fprintf(outfile,"from the previous location backward)\n");
	    }
	    break;
	case	0x32	:
	    fprintf(outfile,"ME:");
	    read_block(params,p3,0);
	    fprintf(outfile," - (Value to be added)\n");
	    break;
	case	0x20	:
	    fprintf(outfile,"ME:");
	    read_block(params,p3,0);
	    fprintf(outfile," - (CHV%02X value to be validated)\n",p2);
	    break;
	case	0x24	:	//CHANGE CHV VALUE - fully implemented
	    fprintf(outfile,"ME:");
	    read_block(params,p3,0);
	    fprintf(outfile," - (Change CHV data)\n",p2);
	    fprintf(outfile,"\t(Target CHV: CHV%i)\n",p2);
	    fprintf(outfile,"\t(Old CHV value: %02X %02X %02X %02X %02X %02X %02X %02X)\n",params[0],params[1],params[2],params[3],params[4],params[5],params[6],params[7]);
	    fprintf(outfile,"\t(New CHV value: %02X %02X %02X %02X %02X %02X %02X %02X)\n",params[0],params[1],params[2],params[3],params[4],params[5],params[6],params[7]);
	    break;
	case	0x26	:	//DISABLE CHV - fully implemented
	case	0x28	:	//ENABLE CHV - fully implemented
	    fprintf(outfile,"ME:");
	    read_block(params,p3,0);
	    fprintf(outfile," - (CHV1 value)\n",p2);
	    break;
	case	0x2C	:	//UNBLOCK CHV - fully implemented
	    fprintf(outfile,"ME:");
	    read_block(params,p3,0);
	    fprintf(outfile," - (Unblock CHV data)\n");
	    fprintf(outfile,"\t(Target CHV: CHV%i)\n", (p2==0) ? 1 : p2);
	    fprintf(outfile,"\t(UNBLOCK CHV value: %02X %02X %02X %02X %02X %02X %02X %02X)\n",params[0],params[1],params[2],params[3],params[4],params[5],params[6],params[7]);
	    fprintf(outfile,"\t(New CHV value: %02X %02X %02X %02X %02X %02X %02X %02X)\n",params[0],params[1],params[2],params[3],params[4],params[5],params[6],params[7]);
	    break;
	case	0x04	:
	case	0x44	:
	    break;
	case	0x88	:	//RUN GSM ALGORITHM - fully implemented
	    fprintf(outfile,"ME:");
	    read_block(params,p3,0);
	    fprintf(outfile," - (RAND)");
	case	0xFA	:	//SLEEP - fully implemented
	    break;
	case	0xC0	:
	    fprintf(outfile,"SIM:");
	    read_block(params,p3,0);
	    fprintf(outfile," - (%s response data)\n",get_cmd_name(last_response_data_command));
	    break;    
	case	0x10	:
	    fprintf(outfile,"ME:");
	    read_block(params,p3,0);
	    fprintf(outfile," - (Terminal profile of ME)");
	    Analyze_TP(params,p3);
	    break;
	case	0xC2	:
	    fprintf(outfile,"ME:");
	    read_block(params,p3,0);
	    fprintf(outfile," - (Envelope data)");
	    break;
	case	0x12	:
	    fprintf(outfile,"ME:");
	    read_block(params,p3,0);
	    fprintf(outfile," - (Proactive SIM command)");
	    break;
	case	0x14	:
	    fprintf(outfile,"ME:");
	    read_block(params,p3,0);
	    fprintf(outfile," - (Terminal response)");
	    break;

    }
    do {
	sw1=i_getchar();
	if (sw1==0x60)
	    fprintf(outfile,"\nSIM: 60 - (SIM needs more time)",sw1,sw2);
    } while (sw1==0x60);
    if (sw1==0x9F) last_response_data_command=ins;
    sw2=i_getchar();
    fprintf(outfile,"\nSIM: %02X %02X - (",sw1,sw2);
    print_status(sw1,sw2);
    fprintf(outfile,")\n\n");
    
    print_line();
}

int	main(int argc, char *argv[])
{
    fprintf(stderr,"\nSSI Sim Communication Analyzer "program_version"\n\n");
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

    fprintf(outfile,"File generated by SSI Sim Communication Analyzer "program_version"\n\n");
    fprintf(outfile,"Infile: %s\n\n", (argc>1) ? argv[1] : "<stdin>");
    
    print_line();
    ReadATR();
    while (!(feof(infile)))
	ReadCommand();
    print_line();
    close_files();
    return 0;
}
