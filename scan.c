#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>

enum {NONE, NAME, NUMBER};
#define maxLen ((size_t)256)

long number;
char name[maxLen];
char *string;

int token;
size_t len;

int comment;
int isSpace(const char *c)
{	if((*c == '\0') || (*c == EOF))
		return 0;
	if((isspace(*c) || (comment & 1)) && (*c != '\n'))
		return 1;
	if((*c == '*') && (*(c+1) == '/') && (comment & 2))
	{	comment = 0;
		return 2;
	}
	if(comment & 2)
		return 1;
	if(*c == '/')
	{	if(*(c+1) == '/')
			comment = 1;
		else if(*(c+1) == '*')
			comment = 2;
		return 2;
	}
	return 0;
}

/* TODO: adjacent string literals are concatenated
   TODO: encodings
   TODO: fix 0x and base
   TODO: macro expansion
*/
int scan(const char *buf)
{	static const char *c = 0;
	int cnt;
	if(buf) c = buf;
	comment = 0;
	while((cnt = isSpace(c)))
		c += cnt;
	if(isalpha(*c) || (*c == '_'))
	#define ALPHA_NUM "abcdefghijklmnopqrstuvwxyz" \
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
		"_1234567890"
	{	len = strspn(c, ALPHA_NUM);
		strncpy(name, c, (len<maxLen ? len : maxLen-1));
		name[(len<maxLen ? len : maxLen-1)] = '\0';
		c += len;
		token = NAME;
	}
	else if(isdigit(*c))
	{	number = strtol(c, (char **)&c, 0);
		if(errno == ERANGE)
			perror(0);
		token = NUMBER;
	}
	else
		token = (c && (*c != '\0') ? *c++ : 0);
	return token;
}

char *readSrc(const char *fname)
{	char *buf;
	FILE *fp;
  	size_t size;

 	fp = fopen(fname, "rb");
  	if(fp == 0)
  		perror("Error opening source file");
	else
	{	fseek(fp, 0, SEEK_END); /* non-portable */
		size = ftell(fp);
		rewind(fp);
		buf = calloc(size+2, sizeof(char));
		if(buf)
		{	fread(buf, 1, size, fp);
			buf[size] = '\n';
			buf[size+1] = '\0';
		}
		fclose(fp);
		return buf;
    }
	return 0;
}

/* test */
int main(void)
{	char *buf;
	int i;
	assert((buf = readSrc("scan.c")));
	//freopen("tokens.txt", "w", stdout);
	scan(buf);
	while(scan(0))
	{	if(token == NAME)
			printf("%d %s\n", len, name);
		else if(token == NUMBER)
			printf("%ld\n", number);
		else printf("'%c'\n", token);
	}
	return 0;
}
