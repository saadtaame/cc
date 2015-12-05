#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

enum {dummy, Z, plus, E, left, T, right, dollar, id, CNT_SYMBOLS};
char *names[] = {"dummy", "Z", "+", "E", "(", "T", ")", "$", "id", "CNT_SYMBOLS"};

/* non-terminals are odd, terminals are even */
#define isTerminal(x) (((x) >= 0) && ((x)%2 == 0))

/* max RHS size, number of symbols in grammar */
#define DEG 100
#define NRULES 500

/* end of inupt, empty string, end of rule marker */
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
{   int i, n;
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
{   int i, n, Y;

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
    	if(!visited2[Y])
            getFollow(Y);
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
{   int st = 0;
    if(isTerminal(0)) st = 1;

    memset(visited, 0, sizeof(visited));
    for(; st < nsymbols; st++)
    {   first(st);
    }
}

void collectFirstSets(int nsymbols)
{	int st = 0;
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
{	int st = 0;
    if(isTerminal(0)) st = 1;

    memset(visited, 0, sizeof(visited));
    derivesEoi[st] = 1; /* add end_of_input to start symbol */
    for(; st < nsymbols; st++)
    {	follow(st);
    }
}

void collectFollowSets(int nsymbols)
{	int st = 0;
    if(isTerminal(0)) st = 1;

	for(st = 0; st < nsymbols; st++)
    {	printf("FOLLOW(%s) = ", names[st]);
    	collectFollow(st);
    	puts("");
    }
}

/* lalr table construction */
struct State {
    int x, y;
    int unseen;
    struct State *next;
};

struct State *mkState( int a, int b, int c ) {
    struct State *s = 0;
    s = (struct State *)malloc(sizeof(struct State));
    assert(s);
    s->x = a, s->y = b;
    s->unseen = c;
    s->next = 0;
    return s;
}

#define MAX_STATES 5000
struct State *state[MAX_STATES];
int nstates;

/* stupid linear scan */
int exists( int a, int b, int c ) {
    struct State *sp;
    for(sp = state[nstates]; sp; sp = sp->next) {
        if(sp->x == a && sp->y == b && sp->unseen == c)
            return 1;
    }
    return 0;
}

void closure( struct State *s ) {
    int i, j, k;
    int cnt_eor;
    struct State *new_state;

    i = s->x;
    cnt_eor = 0;
    for(j = 0; j < deg[i]; j++) {
        if(cnt_eor == s->y) break;
        if(adj[i][j] == EOR) cnt_eor++;
    }
    j += s->unseen;
    if(adj[i][j] == EOR) return;
    if( isTerminal(adj[i][j]) ) return;

    cnt_eor = 0;
    for(k = 0; k < deg[ adj[i][j] ]; k++) {
        if(adj[ adj[i][j] ][k] == EOR) {
            cnt_eor++;
        }
        else if(k == 0 || adj[ adj[i][j] ][k-1] == EOR) {
            if( exists(adj[i][j], cnt_eor, 0) ) continue;
            new_state = mkState(adj[i][j], cnt_eor, 0);
            new_state->next = state[nstates];
            state[nstates] = new_state;
            closure(new_state);
        }
    }
}

void printState( struct State *s ) {
    int i, j;
    int flag;
    int cnt_eor;

    printf("\"");
    for(; s; s = s->next) {
        cnt_eor = 0;
        for(i = 0; i < deg[s->x]; i++) {
            if(cnt_eor == s->y) break;
            if(adj[s->x][i] == EOR) cnt_eor++;
        }
        j = i;
        flag = 0;
        printf("%s :", names[s->x]);
        for(; adj[s->x][i] != EOR; i++) {
            if(s->unseen + j == i)
                printf(" ."), flag = 1;
            printf(" %s", names[adj[s->x][i]]);
        }
        if(flag == 0) printf(" .");
        printf("\\l");
    }
    printf("\"");
}

int itemCmp( const void *A_, const void *B_ ) {
    struct State *A, *B;
    A = (struct State *)A_;
    B = (struct State *)B_;
    if(A->x < B->x) return -1;
    else if(A->x == B->x) {
        if(A->y < B->y) return -1;
        else if(A->y == B->y) {
            if(A->unseen < B->unseen) return -1;
            else if(A->unseen == B->unseen) return 0;
        }
    }
    return 1;
}

/* hash state for fast lookup */
unsigned hash( struct State *s ) {
    static struct State *arr[1000];
    int n = 0;
    int i;
    unsigned h = 0;
    unsigned b = 1;
    unsigned base = 127;
    unsigned mod = 1000000007;

    for(; s; s = s->next)
        arr[n++] = s;
    qsort(arr, n, sizeof(struct State *), itemCmp);

    for(i = 0; i < n; i++) {
        h += ((arr[i]->x * 3 ^ arr[i]->y) ^ (arr[i]->unseen * 5)) * b;
        h %= mod;
        b = (b * base) % mod;
    }

    return h;
}

/* state machine, state indexing starts at 1 */
int table[MAX_STATES][NRULES];

/* successor function */
void succ( struct State *s, int X ) {
    int i;
    int cnt_eor;
    struct State *new_state;

    for(; s; s = s->next) {
        cnt_eor = 0;
        for(i = 0; i < deg[s->x]; i++) {
            if(cnt_eor == s->y) break;
            if(adj[s->x][i] == EOR) cnt_eor++;
        }
        if(adj[s->x][s->unseen + i] == X) {
            if( exists( s->x, s->y, s->unseen+1) ) continue;
            new_state = mkState(s->x, s->y, s->unseen+1);
            new_state->next = state[nstates];
            state[nstates] = new_state;
            closure(new_state);
        }
    }
}

void freeState( struct State *s ) {
    struct State *tmp;
    while(s) {
        tmp = s;
        s = s->next;
        free(tmp);
    }
}

/* save DFA as dot graph */
void save( void ) {
    int i, j;

    freopen("dfa.txt", "w", stdout);
    puts("digraph {");
    puts("node [shape=Mrecord]");

    for(i = 1; i <= nstates; i++) {
        printf("%d [label=", i);
        printState(state[i]);
        puts("]");
    }

    for(i = 1; i <= nstates; i++) {
        for(j = 1; j < CNT_SYMBOLS; j++) {
            if(table[i][j]) {
                printf("%d -> %d [label=\"%s\"]\n", i, table[i][j], names[j]);
            }
        }
    }
    puts("}");
}

/* compute LALR parsing table,
   computes LR(0) states for now
 */
void lalr( void ) {
    static int hashes[MAX_STATES];
    int pos;

    int i, j, k, st;
    int flag;
    unsigned hh;
    struct State *cur;

    st = 0;
    if( isTerminal(0) ) st = 1;
    nstates = 1;
    state[nstates] = mkState(st, 0, 0);

    /* initial state */
    closure(state[nstates]);
    cur = state[nstates];
    k = pos = nstates;
    hashes[pos++] = hash(cur);

    while(k++ <= nstates) {
        cur = state[k-1];
        for(i = 1; i < CNT_SYMBOLS; i++) {
            nstates++;
            freeState(state[nstates]);
            state[nstates] = 0;
            succ(cur, i);
            if(state[nstates] == 0) {
                nstates--;
                continue;
            }
            hh = hash(state[nstates]);
            flag = 0;
            assert(pos == nstates);
            for(j = 1; !flag && j < pos; j++) {
                if(hh == hashes[j]) {
                    table[k-1][i] = j;
                    nstates--;
                    flag = 1;
                }
            }
            if(flag == 0) {
                hashes[pos++] = hh;
                table[k-1][i] = nstates;
            }
        }
    }
    save();
}

int main(void)
{   //freopen("ioutput.txt", "w", stdout);
    append(Z, E);
    append(Z, dollar);
    append(Z, EOR);

    append(E, E);
    append(E, plus);
    append(E, T);
    append(E, EOR);

    append(E, T);
    append(E, EOR);

    append(T, id);
    append(T, EOR);

    append(T, left);
    append(T, E);
    append(T, right);
    append(T, EOR);

	/*computeFirstSets(N);
	computeFollowSets(N);

	collectFirstSets(N);
	collectFollowSets(N);*/

	lalr();

    return 0;
}
