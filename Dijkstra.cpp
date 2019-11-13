#include <limits.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <omp.h>

#define INF INT_MAX
#define NO_CONN 111
#define NOT_VISITED 1
#define VISITED 2
#define MAX_EDGE_WEIGTH 25

// Number of vertices in the graph 
#define V 5000 

typedef struct {
	long N; //Number of nodes
	char** node; //Matrix of Nodes and connections
	int* D; //Here we will store the shortest path length for each node.
	char* visited; //Used to flag which nodes have been visted yet or not.
} graph;

// Function to find and update the distance array
int minDistance(int dist[], bool visited[]){ 
	int min = INT_MAX;
    int min_index; 

	for (int v = 0; v < V; v++){ 
		if (visited[v] == false && dist[v] <= min){ 
			min = dist[v];
            min_index = v;
        } 
    }
	return min_index; 
} 

// Function to print the distance array 
void printSolution(int dist[], int n) 
{ 
	printf("Vertex Distance from Source\n"); 
	for (int i = 0; i < V; i++){ 
		printf("%d tt %d\n", i, dist[i]);
    }
} 

// Function that implements Dijkstra's single source shortest path algorithm 
// for a graph represented using adjacency matrix representation 
void dijkstra(graph *G, int src) 
{ 
    //char graph[V][V] = G->node;
	int dist[V]; // The output array. dist[i] will hold the shortest distance from src to i 
	bool visited[V]; // Array to hold true if the vertex has been visited or false otherwise 
    int numVisited = 0;

	// Initialize all distances as INFINITE and visited[] as false 
	for (int i = 0; i < V; i++){ 
		dist[i] = INT_MAX; 
        visited[i] = false; 
    	}

	// Distance of source vertex from itself is always 0 
	dist[src] = 0; 

	// Find shortest path for all vertices
	int v = 0; 
	int u = 0;
	#pragma omp parallel private(v,u)
	{
    	while(numVisited < V) { 
		u = minDistance(dist, visited); 
		visited[u] = true;
        	numVisited++; 
		#pragma omp parallel for
		for (v = 0; v < V; v++){
			// Update dist[v] only if total weight of path from src to v through u is smaller than current value of dist[v] 
			if (!visited[v] && G->node[u][v] && dist[u] != INT_MAX && dist[u] + G->node[u][v] < dist[v]){
				dist[v] = dist[u] + G->node[u][v];
            		} 
        	}
	} 
	}
	// print the constructed distance array 
	printSolution(dist, V); 
} 

////////////////////////////////////////////////GRAPH GENERATOR CODE STARTS HERE//////////////////////////////////////////////////


void generateEmptyGraph(long N, graph *G)
{
	int i;

	assert((N > 0) && "N has to be bigger than zero!" );

	G->N = N;
	G->D = (int*)malloc(sizeof(int) * G->N );
	if(G->D == NULL) { perror("malloc"); exit(EXIT_FAILURE); }
	G->visited = (char*)malloc(sizeof(char) * G->N);
	if(G->visited == NULL) { perror("malloc"); exit(EXIT_FAILURE); }

	char* temp;
	temp = (char*) malloc(G->N*G->N);
	if(temp == NULL) { perror("malloc"); exit(EXIT_FAILURE); }
	G->node = (char**)malloc(sizeof(char*) * G->N);
	if(G->node == NULL) { perror("malloc"); exit(EXIT_FAILURE); }
	for(i = 0; i < G->N; i++){
		G->node[i] = &temp[i*G->N];
	}
}

void resetGraph(graph* G)
{
	long i;
	for(i=0; i < G->N; i++){
		G->D[i] = INF;
		G->visited[i] = NOT_VISITED;
	}
}

void generateGraph(long N, graph *G, int seed)
{
	long linkCnt;
	long i,j;
	long t;
	printf("Generating Graph of size %ldx%ld with seed [%d]!\n", N,N, seed);

	assert((N > 0) && "N has to be bigger than zero!" );
	assert((seed >= 0) && "seed has to be bigger than zero or equal!" );

	srand(seed);

	generateEmptyGraph(N, G);
	resetGraph(G);

	//Initialize Matrix
	for(i = 0; i< G->N; i++)
	{
		linkCnt=0; //Keep track of # of outgoing edges
		for(j = 0; j < G->N; j++)
		{
			if(i == j){
				t = 0;
			} else {
				t = (rand() % ((MAX_EDGE_WEIGTH-1) * 2)+1); //50% of having no connection
				if(t > MAX_EDGE_WEIGTH){
					//t = INF; //Like no connection
					t = NO_CONN; //Like no connection
				} else {
					linkCnt++;
					G->visited[j] = VISITED; //Do this to find isolated nods that have no incomming edge
				}
			}

			G->node[i][j] = t;
		}

		//Be sure to only generate fully connected graphs by each node having at least one edge to someone else!
		if(linkCnt == 0)
		{
			printf("Adding outgoing link for [%ld]\n", i);
			t = rand() % (G->N);

			if(t == i) //NO self loops
				t = (t*t)%G->N;

			G->node[i][t] = rand() % (MAX_EDGE_WEIGTH);
		}
	}

	//To nods that have no incoming edge, add one randomly
	for(i=0;i < G->N; i++){
		if(G->visited[i] != VISITED){
			t = rand() % (G->N);

			if(t == i) //NO self loops
				t = (t+1)%G->N;

			printf("Adding incomming link for %ld -> %ld\n", t, i);
			G->node[t][i] = rand() % (MAX_EDGE_WEIGTH);
		}

		//No set it to unvisited for initialization
		G->visited[i] = NOT_VISITED;
    }
}

// Option to Print the generated Matrix

//void printGraph(graph* G, int N)
//{
//    char *b = (char*)malloc(10 * (N*N) + 400);
//	long i,j;
//	//Debug Print
//
//	if(b == NULL)
//		return;
//
//	b[0] = 0;
//	printf("       ");
//	for(i = 0; i<G->N; i++){
//		sprintf(&b[strlen(b)], " [%03ld] ", i);
//	}
//	sprintf(&b[strlen(b)], "\n\n");
//	for(i = 0; i<G->N; i++)
//	{
//		sprintf(&b[strlen(b)], "[%03ld]  ",i);
//		for(j = 0; j < G->N; j++)
//		{
//			sprintf(&b[strlen(b)], "  %03d  ", G->node[i][j]);
//		}
//		sprintf(&b[strlen(b)], "  [%03ld]\n",i);
//	}
//	sprintf(&b[strlen(b)], "\n       ");
//	for(i = 0; i<G->N; i++){
//		sprintf(&b[strlen(b)], " [%03ld] ", i);
//	}
//	sprintf(&b[strlen(b)], "\n");
//	printf("%s", b);
//}


int main() 
{ 
    graph G;
    int nodes = V; // How big the graph will be
    int seed = 10; // To generate random graphs
    printf("This creates a graph of size %d x %d\n", nodes, nodes);
    generateGraph(nodes, &G, seed);
    printf("DONE!\n");
    //printGraph(&G, nodes);
    
    double start_time = omp_get_wtime();
    dijkstra(&G, 0);
    double end_time = omp_get_wtime();
    printf("Time elapsed: %f\n", end_time - start_time);
    return 0;

} 
