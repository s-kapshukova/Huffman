#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <queue>
#include <unordered_map>


class Node
{   // A tree node
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
    return new Node( x.key + key , x.size + size, new Node(x), this);
  }
};

// Builds Huffman tree
Node * builder(std::priority_queue<Node, std::vector<Node>, Node> tree)
{
    while (tree.size() > 1)
    {
        Node *n = new Node(tree.top());
        tree.pop();
        std::cout << "Build: " << n->key << " " << tree.top().key << std::endl;
        tree.push(*n->join(*new Node(tree.top())));
        tree.pop();
    }
    return new Node(tree.top());
}

// Decoding function
void decoder(const char* input_name = "encoded.txt", const char* output_name = "output.txt")
{
    uint64_t * alfabet = new uint64_t[256];
    for (int i = 0; i < 256; i++)
    {
        alfabet[i] = 0;
    }
    FILE* input_file = fopen(input_name, "rb");  // Open input file
    if (input_file == nullptr)
    {
       throw std::invalid_argument("File not found.");
    }
    unsigned char col = 0;
    unsigned int col_letters = 0;
    col = fgetc(input_file);
    if (!feof(input_file))
    {
       col_letters =(unsigned int) col;
    }

    unsigned char character = 0;
    // Reading the letters used and their number
    for (int i = 0; i < col_letters; i++)
    {
        character = fgetc(input_file);
        if (!feof(input_file))
        {
            fread(reinterpret_cast<char*>(&alfabet[character]), sizeof(uint64_t), 1, input_file);
        }
        else
        {
            throw std::invalid_argument("Can't decompress file.");
        }
    }

    std::priority_queue<Node, std::vector<Node>, Node> tree;
    for (int i = 0; i < 256; i++)
    {
        if (alfabet[i] != 0)
        {
            std::string s(1, static_cast<char>(i));

            Node new_leaf(s, alfabet[i]);
            tree.push(new_leaf);
        }
    }
    character = 0;
    Node *n = builder(tree);

    FILE* output_file = fopen(output_name, "wb +");
    Node *nodes = n;
    unsigned char letter = 0;
    while (!feof(input_file))
    {  // Decompressing the file
        character = fgetc(input_file);
        if (!feof(input_file))
        {
            for (int i = 7; i > -1; i--)
            {
                if (((character >> i) & 1) == 1) // Right nodes
                {
                    if (nodes->R == NULL)
                    {
                        letter = nodes->key[0];
                        if (alfabet[letter] > 0)
                        {
                            alfabet[letter]--;
                            fputc(letter, output_file);
                            nodes = n->R;
                        }
                    }
                    else
                    {
                        nodes = nodes->R;
                    }
                }
                else if (((character >> i) & 1) == 0) //Left nodes
                {
                    if (nodes->L == NULL)
                    {
                        letter = nodes->key[0];
                        if (alfabet[letter] > 0)
                        {
                            fputc(letter, output_file);
                            nodes = n->L;
                            alfabet[letter]--;
                        }
                    }
                    else
                    {
                        nodes = nodes->L;
                    }
                }
            }
       }
    }

    fclose(input_file);
    fclose(output_file);
}

// Checking for file matches
unsigned int checker(const char* before_name = "input.txt", const char* after_name = "output.txt")
{
    unsigned int same = 0;
    FILE* before_file = fopen(before_name, "r");
    FILE* after_file = fopen(after_name, "r");

    unsigned char after_l = 0;
    unsigned char before_l = 0;
    while (!feof(after_file) && !feof(before_file))
    {
        after_l = fgetc(after_file);
        before_l = fgetc(before_file);
        if (!feof(after_file) && !feof(before_file))
        {
            if (after_l != before_l)
            {
                same++;
            }
        }
    }

    while (!feof(after_file))
    {
        after_l = fgetc(after_file);
        if (!feof(after_file))
        {
            same++;
        }
    }

    while (!feof(before_file))
    {
        before_l = fgetc(before_file);
        if (!feof(before_file))
        {
            same++;
        }
    }
    fclose(after_file);
    fclose(before_file);
    return same;
}

int main()
{
    decoder();
    std::cout << checker() << std::endl;
}