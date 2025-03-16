#include "comb.hh"
#include "progressBar.hh"
#include <vector>
#include "element.hh"
#include <iostream>
#include "LESSMDS.hh"
#include <algorithm>

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        fprintf(stderr, "usage: LESS_search n k alpha w \n \
                                -- A brute-force approach to find primitive element in GF(2^w) satisfying MDS property.\n");
        fprintf(stderr, "       w must be one of 8, 16, 32.\n");
        fprintf(stderr, "\n\n");
        return 0;
    }

    int n = 14;
    int k = 10;
    int alpha = 2;
    int w = 8;

    sscanf(argv[1], "%d", &n);
    sscanf(argv[2], "%d", &k);
    sscanf(argv[3], "%d", &alpha);
    sscanf(argv[4], "%d", &w);

    LESS_PCmat pcmat(n, k, alpha);
   
    uint32_t e = 0;
    pcmat.search_element(w, e);

    return 0;

    // generate_combination g(5, 3);
    // g.print();
    // while (g.next())
    // {
    //     g.print();
    // }

    // ProgressBar p(65536);
    // for (int i = 0; i < 65536; i++)
    // {
    //     p.increment();
    // }

    // std::vector<uint32_t> primitive_element_list_GF8;
    // primitive_element_generator pe8(8);
    // do
    // {
    //     primitive_element_list_GF8.push_back(pe8.get_element());
    // } while (pe8.next());
    // std::sort(primitive_element_list_GF8.begin(), primitive_element_list_GF8.end());
    // std::cout << "Primitive elements in GF(2^8):  " << primitive_element_list_GF8.size() << "-----\n";
    // for (auto i : primitive_element_list_GF8)
    // {
    //     std::cout << i << ",";
    // }
    // std::cout << std::endl;
    // std::cout << std::endl;
    // std::cout << std::endl;

    // std::vector<uint32_t> primitive_element_list_GF16;
    // primitive_element_generator pe16(16);
    // do
    // {
    //     primitive_element_list_GF16.push_back(pe16.get_element());
    // } while (pe16.next());
    // std::sort(primitive_element_list_GF16.begin(), primitive_element_list_GF16.end());
    // std::cout << "Primitive elements in GF(2^16): " << primitive_element_list_GF16.size() << "-----\n";
    // for (auto i : primitive_element_list_GF16)
    // {
    //     std::cout << i << ",";
    // }
    // std::cout << std::endl;
}

