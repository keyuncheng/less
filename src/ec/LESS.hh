#ifndef __LESS_HH__
#define __LESS_HH__

#include "ECBase.hh"
#include "Computation.hh"

#include <map>

class LESS : public ECBase {
private:

    int _m; // m = n - k
    int _fw; // field width
    uint32_t _e; // primitive root element
    uint32_t _f; // primitive root element (in uint32_t)
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
    ECDAG *decodeSingle(vector<int> &availSymbols, vector<int> &failedSymbols);
    ECDAG *decodeMultiple(vector<int> &availSymbols, vector<int> &failedSymbols);
    ECDAG *decodeMultipleWithSubStripes(vector<int> &availSymbols, vector<int> &failedSymbols);
    ECDAG *decodeMultipleWithPCMatrix(vector<int> &availSymbols, vector<int> &failedSymbols);

    void getPrimElementsPower(int order, int e, int fw); // get primitive elements power
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

    LESS(int n, int k, int w, int opt, vector<string> param);
    ~LESS();

    /**
     * @brief Get Available Prim Elements for LESS with (n,k,w).
     * 
     * @param n 
     * @param k 
     * @param w 
     * @param fw return field width
     * @param e return primitive element
     * @param f return polynomial f (in uint32_t)
     * @return true
     * @return false 
     */
    static bool getAvailPrimElements(int n, int k, int w, int &fw, uint32_t &e, uint32_t &f);

    /**
     * @brief find root with primitive polynomial f
     * 
     * @param f polynomial 
     * @param fw field width
     * @return uint32_t root
     */
    static uint32_t findRoot(uint32_t f, int fw);

    /**
     * @brief polynomial assignment
     * 
     * @param x 
     * @param f 
     * @param fw 
     * @return uint32_t 
     */
    static uint32_t polynomialAssignment(uint32_t x, uint32_t f, int fw);

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

#endif // __LESS_HH_