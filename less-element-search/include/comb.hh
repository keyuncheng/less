#ifndef __COMBINATION__
#define __COMBINATION__

/**
 * @brief Calculate the combination of \binom{n}{r}
 * @param n The total number of elements
 * @param r The number of elements to choose
 */

unsigned long long comb(int n, int r);

class generate_combination
{
public:
    generate_combination(int n, int r) : _n(n), _r(r)
    {
        subset = new int[r];
        clear();
        count = 1;
        ncr = comb(n, r);
    }

    ~generate_combination()
    {
        delete[] subset;
    }

    int *cur_subset()
    {
        return subset;
    }

    void clear();
    bool next();
    void print();

private:
    int _n;
    int _r;
    int *subset;
    unsigned long long ncr;
    unsigned long long count;
};

#endif // #ifndef __COMBINATION__