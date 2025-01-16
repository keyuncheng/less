#ifndef __STACKCode_HH__
#define __STACKCode_HH__

#include "ECBase.hh"
#include "Computation.hh"

#include <map>

class STACKCode : public ECBase {
private:

    int _m; // m = n - k
    int _fw; // field width
    int _e; // primitive element
    int _order; // the number of nonzero elements in GF(2^fw)
    vector<int> _primElementPower; // primitive elements power in GF(2^fw)

    int _numVirtualSymbols; // total number of virtual symbols
    vector<int> _virtualSymbols; // virtual symbols
    vector<vector<int>> _layout; // layout (w * n)

    int _numGroups; // number of groups
    int *_encodeMatrix; // encoding matrix
    int *_pcMatrix; // parity check matrix

    /**
     * @brief Implementation 2: use augmented sub-stripes to construct the code
     * 
     */
    vector<vector<int>> _nodeGroups; // node groups (with _numGroups)
    vector<vector<int>> _symbolGroups; // symbol groups (with _numGroups)
    vector<vector<int>> _coefs4Symbols; // coefficient for each symbol for encoding
    vector<int> _nodePermutation; // node permutation

    map<int, int> _elementMap; // element map (for debugging)

    void genParityCheckMatrix();
    void genEncodingMatrix();
    bool genDecodingMatrix(vector<int> &availSymbols, vector<int> &failedSymbols, int *decodeMatrix);
    ECDAG *decodeSingle(int failedNode);
    ECDAG *decodeMultiple(vector<int> &availSymbols, vector<int> &failedSymbols);
    ECDAG *decodeMultipleWithSubStripes(vector<int> &availSymbols, vector<int> &failedSymbols);
    ECDAG *decodeMultipleWithPCMatrix(vector<int> &availSymbols, vector<int> &failedSymbols);

    void getPrimElementsPower(int order, int e, int fw); // get primitive elements power
    int getAvailPrimElements(int n, int k, int w, int fw); // get available primitive elements for STACKCode with w for GF(2^fw)
    int findRoot(int f, int fw); // find root with primitive polynomial f
    int polynomialAssignment(int x, int f, int fw); // polynomial assignment
    bool convertPCMatrix2EncMatrix(int n, int k, int w); // convert parity check matrix to encoding matrix
    /**
     * @brief convert parity check matrix to generator matrix, based on the
     * avialable ndoes and failed nodes
     * 
     * @param n 
     * @param k 
     * @param fw field width
     * @param pcMatrix a matrix in size of (n - k)*w * n*w
     * @param genMatrix a matrix in size of k*w * n*w
     * @param from length of n * w vector, consisting of 0 and 1s. 0 means
     * failed symbol id, 1 means available symbol id
     * @param to length of n * w vector, consisting of 0 and 1s. 0 means
     * failed symbol id, 1 means available symbol id
     */
    bool getGenMatrixFromPCMatrix(int n, int k, int fw, const int* pcMatrix, int *genMatrix, const int* from, const int* to);

    void initLayout(); // init code layout
    int getRepairBandwidth(int failedNode); // get repair bandwidth
    vector<int> getHelperNodes(int failedNode); // get helper nodes
    int *getRepairMatrix(int failedNode); // get repair matrix for single node failure

public:

    STACKCode(int n, int k, int w, int opt, vector<string> param);
    ~STACKCode();
    ECDAG* Encode();
    ECDAG* Decode(vector<int> from, vector<int> to);
    void Place(vector<vector<int>>& group);

    /**
     * @brief Get sub-packets in nodeid
     * 
     * @param nodeid 
     * @return vector<int> 
     */
    vector<int> getNodeSubPackets(int nodeid);

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

#endif // __STACKCode_HH_