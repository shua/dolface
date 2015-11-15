#define CMD 1
#if (CMD==0)
char childcmd[] = \
	"while read f; do \
		if [ -d $f ]; then \
			echo $f/..; \
			for g in $(ls $f); do \
				echo $f/$g; \
			done; \
		else echo; \
		fi; \
	done";
char primer[] = ".";
char *nextinput(char *selection) {
	char *ret;
	fprintf(stderr, "%s -", selection);
	ret = malloc(strlen(selection)+1);
	strcpy(ret, selection);
	return ret;
}
#elif (CMD==1)
char childcmd[] = "./datbody PlPpNr.dat";
char primer[] = "0 hdr ";
char *nextinput(char *selection) {
	char *ret;

	logwin("heyyyy");
	if(selection[9] != 'p' && selection[9] != '?')
		return NULL;

	selection += 9;
	while(*selection && *selection != ':') {
		++selection;
	}
	selection += 2;
	ret = malloc(strlen(selection) + 1);
	strcpy(ret, selection);
	return ret;
}
#elif (CMD==2)
char childcmd[] = "while read input; do echo $(($input * 2)); echo $(($input * 3)); echo $(($input * 5)); echo; done";
char primer[] = "1";
char *nextinput(char *selection) {
	char *ret;
	ret = malloc(strlen(selection));
	strcpy(ret, selection);
	return ret;
}
#elif (CMD==3)
char childcmd[] = "dc";
char primer[] = "7 6 5 4 n10P n10P n10P n10P";
char *nextinput(char *selection) {
	char *ret;
	char add2[] = " 2 + n10P ";
	char mul3[] = " 3 * n10P ";
	char sub4[] = " 4 - n10P ";
	char nl[]   = "10P";
	int sellen = strlen(selection);
	int len = 3*sellen + 3*(sizeof(add2)-1) + sizeof(nl)-1;
	ret = malloc(len+1);
	len = 0;
	strcpy(ret + len, selection);   len += sellen;
	strcpy(ret + len, add2);        len += sizeof(add2)-1;
	strcpy(ret + len, selection);   len += sellen;
	strcpy(ret + len, mul3);        len += sizeof(mul3)-1;
	strcpy(ret + len, selection);   len += sellen;
	strcpy(ret + len, sub4);        len += sizeof(sub4)-1;
	strcpy(ret + len, nl);          len += sizeof(nl)-1;
	return ret;
}
#endif
