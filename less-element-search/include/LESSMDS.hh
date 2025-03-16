#ifndef __LESSMDS__
#define __LESSMDS__

#include <stdlib.h>

class LESS_PCmat
{
public:
    LESS_PCmat(int n, int k, int alpha);

    ~LESS_PCmat();

    void get_pcmat_log();

    // transpose of pcmat
    void get_pcmat(int w, uint32_t e);

    uint32_t* get_square_mat(int *subset);

    bool is_invertible(uint32_t *square_mat, int w);

    void print_pcmat_log();
    void print_pcmat();
    void print_square_mat(int *subset);
    bool verify_MDS_property(int w , uint32_t e);

    bool search_element(int w, uint32_t &e);

private:
    int g(int i);
    int h(int i);

    int _n;
    int _k;
    int _alpha;
    int _r;

    int s;
    int t;
    int rows;
    int cols;
    int order;

    int *pcmat_log;
    uint32_t *primitive_element_power;
    uint32_t *pcmat; // transpose of pcmat
    uint32_t *square_mat;

    int length;
};

#endif // #ifndef __LESSMDS__