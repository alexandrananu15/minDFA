#include "Dsu.h"

//initializare 

Dsu::Dsu(int n) {
    parent.resize(n);
    weight.resize(n);
    for (int i = 0; i < n; ++i) {
        parent[i] = i;
        weight[i] = 1;
    }
}

int Dsu::getParent(int x) {
    int ret = x;
    while (parent[ret] != ret) ret = parent[ret];
    //compresia drumurilor => reduce complexitatea de la O(log n) la O(log *), inversa functiei lui Ackermann
    while (x != parent[x]) {
        int aux = parent[x];
        parent[x] = ret;
        x = aux;
    }
    return ret;
}

bool Dsu::link(int x, int y)
{
    x = getParent(x);
    y = getParent(y);
    if (x == y) return false;
    if (weight[x] < weight[y]) std::swap(x, y); 
    //reuniune dupa rang (greutate in cazul nostru)
    weight[x] += weight[y];
    parent[y] = x;
    return true;
}