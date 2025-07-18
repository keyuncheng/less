#ifndef _RSCONV_HH_
#define _RSCONV_HH_

#include "../inc/include.hh"
#include "Computation.hh"

#include "ECBase.hh"

using namespace std;

#define RS_N_MAX (255)

#define RSCONV_DEBUG_ENABLE false

class RSCONV : public ECBase {
  private:
    int _encode_matrix[RS_N_MAX * RS_N_MAX];
    int _m;

    void generate_matrix(int* matrix, int rows, int cols, int w);  // This w is for galois field, which is different from our sub-packetization level _w

  public:
    RSCONV(int n, int k, int w, int opt, vector<string> param);
 
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
    virtual vector<vector<int>> GetSubPackets();
    
};

#endif
