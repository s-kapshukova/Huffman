#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <queue>
#include <unordered_map>


class Node {  // A tree node
    public:
    std::string key;
    uint64_t size;
    Node *R;  // Right node
    Node *L;  // Left node

    // Comparison function to be used to order the heap
    bool operator() (const Node& x, const Node& y)
    {
        return x.size >= y.size;
    }

    // Node constructor
    Node(const std::string& value = "", uint64_t amount = 0, Node * left = NULL, Node * right = NULL)
    {
        key = value;
        size = amount;
        L = left;
        R = right;
    }


    Node * join(Node x)
    {  // Node pooling function
        return new Node( x.key+key , x.size + size, new Node(x), this);
    }
};


int main()
{}