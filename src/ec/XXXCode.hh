#ifndef __XXXCODE_HH__
#define __XXXCODE_HH__

#include "ECBase.hh"

class XXXCode : public ECBase {
public:

    XXXCode(int n, int k, int w, int opt, vector<string> param);
    ECDAG* Encode();
    ECDAG* Decode(vector<int> from, vector<int> to);
    void Place(vector<vector<int>>& group);


private:

    int _m; // m = n - k
    int *_encodeMatrix; // encoding matrix
    int *_pcMatrix; // parity check matrix

    void genParityCheckMatrix();
    void genEncodingMatrix();
    void convertPCMatrix2EncMatrix();
    void genDecodingMatrix(vector<int> &availNodes, vector<int> &failedNodes);
    void repairSingle(vector<int> &availNodes, int failedNode);
    void repairMultiple(vector<int> &availNodes, vector<int> &failedNodes);


};

#endif // __XXXCODE_HH_