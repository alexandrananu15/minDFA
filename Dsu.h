#pragma once

#include <iostream>
#include <vector>

//clasa separata ptr ca ne place POO :)

//Disjoint set union : paduri de multimi disjuncte

class Dsu {
public:

    Dsu(int);
    int getParent(int);
    bool link(int, int);

private:
    std::vector < int > parent, weight;
};