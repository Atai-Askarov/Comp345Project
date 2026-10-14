#include "Map.h"
#include <iostream>


#ifdef MAP_STANDALONE
int main() {
    Graph g(4);
    g.addEdge(0, 1);
    g.addEdge(0, 2);
    g.addEdge(1, 3);
    g.addEdge(2, 3);
    g.print();
    return 0;
}
#endif
