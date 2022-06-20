#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <queue>
#include <unordered_map>


class Node
{  // A tree node
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

// Builds Huffman tree function
Node * builder(std::priority_queue<Node, std::vector<Node>, Node> leafs)
{
    while (leafs.size() > 1)
    {
        Node *n = new Node(leafs.top());

        leafs.pop();
        std::cout << "Build: " << n->key << " " << leafs.top().key << std::endl;
        leafs.push(*n->join(*new Node(leafs.top())));
        leafs.pop();
    }
    return new Node(leafs.top());
}

// Generate Huffman codes function
void huffmanCodes(Node* root, std::string code, std::unordered_map <std::string, std::string >* huffmanCode)
{
    if (root == nullptr)
    {
        return;
    }
    if (!root->L && !root->R)
    {
        (*huffmanCode)[root->key] = code;
    }

    huffmanCodes(root->L, code + "0", huffmanCode);
    huffmanCodes(root->R, code + "1", huffmanCode);
}

// Coding function
double coder(const char* input_name = "input.txt", const char* output_name = "encoded.txt")
{
    uint64_t * alfabet = new uint64_t[256];
    for (int i = 0; i < 256; i++)
    {
        alfabet[i] = 0;
    }
    FILE* input_file = fopen(input_name, "rb");
    if (input_file == nullptr)
    {
        throw std::invalid_argument("File not found.");
    }

    unsigned char character = 0;
    while (!feof(input_file))
    {  // Read from input file
        character = fgetc(input_file);
        if (!feof(input_file))
        {
           alfabet[character]++;
        }
    }

    fclose(input_file);

    std::priority_queue<Node, std::vector<Node>, Node> leafs;
    for (int i = 0; i < 256; i++)
    {  // Create nodes
        if (alfabet[i] != 0)
        {
            std::string s(1, static_cast<char>(i));

            Node new_leaf(s, alfabet[i]);
            std::cout << s << " : " << alfabet[i] << " : " << new_leaf.size << std::endl;
            leafs.push(new_leaf);
        }
    }

    Node *tree = builder(leafs);  // Create tree

    std::unordered_map<std::string, std::string> huffmanCode;
    huffmanCodes(tree, "", &huffmanCode);  // Generate Huffman codes

    std::cout << "Huffman Codes are :\n" << '\n';  // Print Huffman codes
    for (auto pair : huffmanCode)
    {
        std::cout << pair.first << " " << pair.second << '\n';
    }

    FILE* output_file = fopen(output_name, "wb +");
    input_file = fopen(input_name, "rb");

    character = 0;
    unsigned char k = 0;
    unsigned int len = 0;

    unsigned int bit_len = 0;
    unsigned char letter = 0;
    char col_letters = leafs.size();
    fputc(col_letters, output_file);

    // Writing the letters used and their number
    for (int i = 0; i < 256; i++)
    {
        if (alfabet[i] != 0)
        {
            fputc(static_cast<char>(i), output_file);
            fwrite(reinterpret_cast<const char*>(&alfabet[i]), sizeof(uint64_t), 1, output_file);
        }
    }

    while (!feof(input_file))
    {  // Compressing the file
       character = fgetc(input_file);
        if (!feof(input_file))
        {
            std::string s(1, character);
            if (bit_len + huffmanCode[s].length() <= 8)
            {
                for (int i = 0; i < huffmanCode[s].length(); i++)
                {
                    letter = letter << 1 | (huffmanCode[s][i] - '0');
                }
                bit_len += huffmanCode[s].length();
            }
            else
            {
                for (int i = 0; i < 8-bit_len; i++)
                {
                    letter = letter << 1 | (huffmanCode[s][i] - '0');
                }
                if (huffmanCode[s].length() - 8 + bit_len >= 8)
                {
                    int i = 8 - bit_len;
                    while ( i + 7 < huffmanCode[s].length() )
                    {
                        k = 0;

                        for (int j = 0; j < 8; j++)
                        {
                            k = k << 1 | (huffmanCode[s][i+j] - '0');
                        }

                        i += 8;
                        fputc(letter, output_file);
                        letter = k;
                    }

                    k = 0;
                    len = 0;

                    for (int j = i; j < huffmanCode[s].length(); j++)
                    {
                        k = k << 1 | (huffmanCode[s][j] - '0');
                        len++;
                    }
                }
                else
                {
                    len = 0;
                    for (int i = 8-bit_len; i < huffmanCode[s].length(); i++)
                    {
                        k = k << 1 | (huffmanCode[s][i] - '0');
                        len++;
                    }
                }

                bit_len = 8;
            }

            if (bit_len == 8)
            {
                fputc(letter, output_file);

                letter = k;
                bit_len = len;
                k = 0;
                len = 0;
            }
        }
        else if (bit_len < 8) 
        {
           letter = letter << (8-bit_len);
           fputc(letter, output_file);
        }
    }

    fclose(input_file);
    fclose(output_file);

    uint64_t file_full_size = 0;
    uint64_t commpres_size = 0;
    struct stat sb{};
    struct stat se{};
    // Finding compression ratio
    if (!stat(input_name, &sb))
    {
        file_full_size = sb.st_size;
    }
    else 
    {
        perror("stat");
    }
    if (!stat(output_name, &se))
    {
        commpres_size = se.st_size;
    }
    else
    {
        perror("stat");
    }
    return (commpres_size + 0.0) / file_full_size;
}

int main()
{
    std::cout << coder() << std::endl;  // Print compression ratio
}