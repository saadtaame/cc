#include <stdio.h>
#include <assert.h>
#include <string.h>

enum {a, A, b, B, c, C, N};

/* non-terminals are odd, terminals are even */
#define isTerminal(x) (((x) >= 0) && ((x)%2 == 0))

/* max RHS size, number of rules in grammar */
#define DEG 500
#define NRULES 5000

/* empty string, end of rule marker */
#define EPS -3
#define EOR -4

/* grammar graph */
int deg[NRULES];
int adj[NRULES][DEG];

/* first sets graph */
int fDeg[NRULES];
int fAdj[NRULES][DEG];

/* append Y to the RHS of rule X */
void append(int X, int Y)
{   assert(isTerminal(X) == 0);
    adj[X][ deg[X]++ ] = Y;
}

int visited[NRULES];
int derivesEps[NRULES]; /* does rule derive empty string */

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

void printFirst(int X)
{   int i;
    memset(visited, 0, sizeof(visited));
    memset(S, 0, sizeof(S));
    getFirst(X);
    for(i = 0; i < NRULES; i++)
        if(S[i]) printf("%d ", i);
    if(derivesEps[X])
        printf(" eps");
}

void computeFirstSets(int nsymbols)
{   int st = 0, i, j;
    if(isTerminal(0)) st = 1;

    memset(visited, 0, sizeof(visited));
    for(; st < nsymbols; st++)
    {   first(st);
    }

    for(st = 0; st < nsymbols; st++)
    {	printf("FIRST(%d) = ", st);
    	printFirst(st);
    	puts("");
    }
}

int main(void)
{
	append(A, a);
	append(A, EOR);
	append(A, EPS);
	append(A, EOR);
	
	append(B, b);
	append(B, EOR);

	append(C, c);
	append(C, EOR);
	append(C, A);
	append(C, EOR);
	append(C, B);
	append(C, EOR);

	computeFirstSets(N);
	
    return 0;
}
