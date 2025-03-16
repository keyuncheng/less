#ifndef _WASLRC_HH_
#define _WASLRC_HH_

#include "Computation.hh"
#include "ECBase.hh"

using namespace std;

#define RS_N_MAX (32)

class WASLRC : public ECBase {
  private:
    int _l;
    int _r;
    int _encode_matrix[RS_N_MAX * RS_N_MAX];     

    void generate_matrix(int* matrix, int k, int l, int r, int w);
  public:
    WASLRC(int n, int k, int cps, int opt, vector<string> param);

    ECDAG* Encode();
    ECDAG* Decode(vector<int> from, vector<int> to);
    void Place(vector<vector<int>>& group);

    /**
     * @brief Get all sub-packets
     * N1 N2 ... Nn
     * 0 2 4 6 8 ...
     * 1 3 5 7 9 ... 
     * 
     * @return vector<vector<int>> 
     */
    vector<vector<int>> GetSubPackets();
};

#endif