#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

enum {plus, E, times, T, left, X, right, Y, int_lit, N};
char *names[] = {"plus", "E", "times", "T", "left", "X", "right", "Y", "int_lit", "N"};

/* non-terminals are odd, terminals are even */
#define isTerminal(x) (((x) >= 0) && ((x)%2 == 0))

/* max RHS size, number of rules in grammar */
#define DEG 100
#define NRULES 6000

/* empty string, end of rule marker */
#define EOI -1
#define EPS -3
#define EOR -4

struct Set
{	int derivesEps, derivesEoi;
	int first[NRULES];
	int follow[NRULES];
	int deg[NRULES][2];
};

struct Set s[NRULES];

/* grammar graph */
int deg[NRULES];
int adj[NRULES][DEG];

/* first sets graph */
int fDeg[NRULES];
int fAdj[NRULES][DEG];

/* follow sets graph */
int ffDeg[NRULES];
int ffAdj[NRULES][DEG];

/* append Y to the RHS of rule X */
void append(int X, int Y)
{   assert(isTerminal(X) == 0);
    adj[X][ deg[X]++ ] = Y;
}

int visited[NRULES];
int visited2[NRULES];
int derivesEps[NRULES]; /* does rule derive empty string */
int derivesEoi[NRULES]; /* does rule derive end of input ($) */

void first(int X)
{   int n = deg[X];
    int i, eps;

    visited[X] = 1;
    
    if(isTerminal(X))
        fAdj[X][ fDeg[X]++ ] = X;
    else
    {   eps = 1; /* curent prefix derives empty string */
        for(i = 0; i < n; i++)
        {   if(adj[X][i] == EOR)   
            {   derivesEps[X] |= eps;
                eps = 1;
                continue;
            }
        
            if(eps == 0) continue;
            if(adj[X][i] == EPS) continue;
            
            fAdj[X][ fDeg[X]++ ] = adj[X][i];

            if(!visited[ adj[X][i] ])
                first(adj[X][i]);

            if(derivesEps[ adj[X][i] ] == 0)
                eps = 0;
        }
    }
}

/* set */
int S[NRULES];

void getFirst(int X)
{   int i, j, n;
    visited[X] = 1;
    if(isTerminal(X))
        S[X] = 1;
    else
    {   n = fDeg[X];
        for(i = 0; i < n; i++)
        {   if(!visited[ fAdj[X][i] ])
                getFirst(fAdj[X][i]);
        }
    }
}

void getFollow(int X)
{   int i, j, n, Y;

	if(X < 0)
	{	getFirst(-X-10);
		return;
	}

    visited2[X] = 1;
    if(isTerminal(X))
        S[X] = 1;

    n = ffDeg[X];
    for(i = 0; i < n; i++)
    {   Y = ffAdj[X][i];
    	if(!visited2[ ffAdj[X][i] ])
            getFollow( ffAdj[X][i] );
    }
}

void collectFirst(int X)
{   int i;
    memset(visited, 0, sizeof(visited));
    memset(S, 0, sizeof(S));
    getFirst(X);
    for(i = 0; i < NRULES; i++)
        if(S[i])
        {	printf("%s ", names[i]);
        	s[X].first[ s[X].deg[X][0]++ ] = i;
        }
    if(derivesEps[X])
    {	printf(" eps");
    	s[X].derivesEps = 1;
    }
}

void collectFollow(int X)
{   int i, n;

    memset(visited, 0, sizeof(visited));
    memset(visited2, 0, sizeof(visited2));
    memset(S, 0, sizeof(S));

	n = ffDeg[X];
    for(i = 0; i < n; i++)
	{	getFollow( ffAdj[X][i] );
	}
    
    for(i = 0; i < NRULES; i++)
        if(S[i])
        {	printf("%s ", names[i]);
        	s[X].follow[ s[X].deg[X][1]++ ] = i;
        }
    if(derivesEoi[X])
    {	printf(" eoi");
    	s[X].derivesEoi = 1;
    }
}

void computeFirstSets(int nsymbols)
{   int st = 0, i, j;
    if(isTerminal(0)) st = 1;

    memset(visited, 0, sizeof(visited));
    for(; st < nsymbols; st++)
    {   first(st);
    }
}

void collectFirstSets(int nsymbols)
{	int st = 0, i, j;
    if(isTerminal(0)) st = 1;

	for(st = 0; st < nsymbols; st++)
    {	printf("FIRST(%s) = ", names[st]);
    	collectFirst(st);
    	puts("");
    }
}

void follow(int X)
{	int i, n;
	int eps;

	n = deg[X]-1;
	for(i = n; i >= 0; i--)
	{	if(adj[X][i] == EPS) continue;
	
		if(adj[X][i] == EOR)
		{	eps = 1;
		}
		else
		{	if(eps)
			{	ffAdj[ adj[X][i] ][ ffDeg[ adj[X][i] ]++ ] = X;
				derivesEoi[ adj[X][i] ] |= derivesEoi[ X ];
			}
			
			eps &= derivesEps[ adj[X][i] ];
			
			if((i > 0) && (adj[X][i-1] != EOR) && (adj[X][i-1] != EPS))
			{	ffAdj[ adj[X][i-1] ][ ffDeg [ adj[X][i-1] ]++ ] = -adj[X][i]-10;
			}
		}
	}
}

void computeFollowSets(int nsymbols)
{	int st = 0, i, j;
    if(isTerminal(0)) st = 1;

    memset(visited, 0, sizeof(visited));
    derivesEoi[st] = 1; /* add end_of_input to start symbol */
    for(; st < nsymbols; st++)
    {	follow(st);
    }
}

void collectFollowSets(int nsymbols)
{	int st = 0, i, j;
    if(isTerminal(0)) st = 1;

	for(st = 0; st < nsymbols; st++)
    {	printf("FOLLOW(%s) = ", names[st]);
    	collectFollow(st);
    	puts("");
    }
}

/* Simple hash table implementation */
struct Bucket
{   struct Bucket *next;
    char *s;
};

typedef struct Bucket Bucket;

Bucket *mkBucket(char *s)
{   int n;
    Bucket *b;
    
    n = strlen(s);
    b = (Bucket *)malloc(sizeof(Bucket));
    assert(b);

    b->s = (char *)calloc(n+1, sizeof(char));
    assert(b->s);
    strncpy(b->s, s, n);
    b->s[n] = '\0';

    b->next = 0;
    return b;
}

Bucket *buckets[2000001];

int hash(char *s)
{   static int MOD = 2000001;
    static int B = 26;
    int h = 0;
    int b = 1;
    char *c;
    for(c = s; *c; c++)
    {   h += (*c - 'a' + 1) * b;
        h %= MOD;
        b *= B;
        b %= MOD;
    }
    return h;
}

void put(char *s)
{   Bucket *b = mkBucket(s);
    int i = hash(s); 
    b->next = buckets[i];
    buckets[i] = b;
}

Bucket *get(char *s)
{   int i = hash(s);
    Bucket *b = buckets[i];
    for(; b; b=b->next)
        if(strcmp(b->s, s) == 0)
            return b;
    return 0;
}

int main(void)
{   append(E, T);
	append(E, X);
	append(E, EOR);

	append(T, int_lit);
	append(T, Y);
	append(T, EOR);
	append(T, left);
	append(T, E);
	append(T, right);
	append(T, EOR);

	append(X, EPS);
	append(X, EOR);
	append(X, plus);
	append(X, E);
	append(X, EOR);

	append(Y, EPS);
	append(Y, EOR);
	append(Y, times);
	append(Y, T);
	append(Y, EOR);

	computeFirstSets(N);
	computeFollowSets(N);

	collectFirstSets(N);
	collectFollowSets(N);
	
    return 0;
}
