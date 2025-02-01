#include "STACKCode.hh"

STACKCode::STACKCode(int n, int k, int w, int opt, vector<string> param) {
    _n = n;
    _k = k;
    _w = w; // sub-packetization
    _opt = opt;
    _m = n - k;
    _numGroups = _w + 1; // num_groups = sub-packetization + 1

    _f = 0b0; // init f to 0b0
    _fw = 8; // init fw to 8
    if (param.size() == 1) {
        if (param[0] != "-") {
            _fw = atoi(param[0].c_str());
        }
    }

    /**
     * @brief set default fw = 8
     * 
     */
    // if (_fw != 8) {
    //     cout << "STACKCode::STACKCode() Currently only supports fw=8" << endl;
    //     exit(1);
    // }
    // _e = getAvailPrimElements(_n, _k, _w, _fw);

    /**
     * @brief extend fw to 16 and 32
     * 
     */
    if (getAvailPrimElements(n, k, w, _fw, _e, _f) == false) {
        cout << "STACKCode::STACKCode() failed to find primitive element" << endl;
        exit(1);
    }

    if (_e == 0)
    {
        cout << "STACKCode::STACKCode() failed to find available primitive elements in GF(2^" << _fw << ")" << endl;
        exit(-1);
    }

    initLayout();

    // obtain GF elements
    _order = (1 << _fw) - 1;
    getPrimElementsPower(_order, _e, _fw);

    // element map (debug)
    for (int i = 0; i < 255; i++) {
        _elementMap[_primElementPower[i]] = i;
    }

    /**
     * @brief method 1: use augmented sub-stripes for encoding and decoding
     */
    
    // Step 1: assign nodes to node groups
    _nodeGroups = vector<vector<int>>(_numGroups, vector<int>());
    int minNumGroupElements = _n / _numGroups;
    int numMaxGroups = n % (_numGroups);

    for (int groupId = 0, nodeId = 0; groupId < _numGroups; groupId++) {
        if (groupId < numMaxGroups) {
            for (int i = 0; i < minNumGroupElements + 1; i++) {
                _nodeGroups[groupId].push_back(nodeId);
                nodeId++;
            }
        } else {
            for (int i = 0; i < minNumGroupElements; i++) {
                _nodeGroups[groupId].push_back(nodeId);
                nodeId++;
            }
        }
    }

    // print out the node groups
    cout << "STACKCode::STACKCode() Node groups:" << endl;
    for (int groupId = 0; groupId < _numGroups; groupId++) {
        cout << "Group " << groupId << ": ";
        for (auto nodeId : _nodeGroups[groupId]) {
            cout << nodeId << " ";
        }
        cout << endl;
    }

    // Step 2: assign symbols to symbol groups in augmented sub-stripes
    _symbolGroups = vector<vector<int>>(_numGroups, vector<int>());
    
    // handle the first _numGroups - 1 symbol groups
    for (int groupId = 0; groupId < _numGroups - 1; groupId++) {
        auto &nodeGroup = _nodeGroups[groupId];
        // add symbols from the same node group
        for (auto nodeId : nodeGroup) {
            for (int alpha = 0; alpha < _w; alpha++) {
                _symbolGroups[groupId].push_back(_layout[alpha][nodeId]);
            }
        }
        // add symbols from the <groupId>-th sub-stripe
        for (int nodeId = 0; nodeId < _n ; nodeId++) {
            // exclude the nodes from the same node group
            if (find(nodeGroup.begin(), nodeGroup.end(), nodeId) == nodeGroup.end()) {
                _symbolGroups[groupId].push_back(_layout[groupId][nodeId]);
            }
        }
    }

    // handle the last symbol group
    auto &lastNodeGroup = _nodeGroups[_numGroups - 1];

    // put all symbols diagonally
    for (int groupId = 0; groupId < _numGroups - 1; groupId++) {
        for (auto nodeId : _nodeGroups[groupId]) {
            _symbolGroups[_numGroups - 1].push_back(_layout[groupId][nodeId]);
        }
    }

    // put all symbols from the last node group
    for (auto nodeId : lastNodeGroup) {
        for (int alpha = 0; alpha < _w; alpha++) {
            _symbolGroups[_numGroups - 1].push_back(_layout[alpha][nodeId]);
        }
    }

    // sort the symbol groups
    for (int groupId = 0; groupId < _numGroups; groupId++) {
        sort(_symbolGroups[groupId].begin(), _symbolGroups[groupId].end());
    }

    // print out the symbol groups
    cout << "STACKCode::STACKCode() Symbol groups:" << endl;
    for (int groupId = 0; groupId < _numGroups; groupId++) {
        cout << "Group " << groupId << ": ";
        for (auto symbolId : _symbolGroups[groupId]) {
            cout << symbolId << " ";
        }
        cout << endl;
    }

    // Step 3: assign coefficients for each symbol for encoding

    // generate permutation (vertically)
    vector<vector<int>> nodeReordering(_numGroups, vector<int>());
    for (int nodeId = 0, rid = 0; nodeId < _n; nodeId++) {
        nodeReordering[rid].push_back(nodeId);
        rid = (rid + 1) % _numGroups;
    }
    for (int groupId = 0; groupId < _numGroups; groupId++) {
        for (auto nodeId : nodeReordering[groupId]) {
            _nodePermutation.push_back(nodeId);
        }
    }

    // print permutation
    cout << "STACKCode::STACKCode() Node permutation:" << endl;
    for (auto nodeId : _nodePermutation) {
        cout << nodeId << " ";
    }
    cout << endl;

    _coefs4Symbols = vector<vector<int>>(_w, vector<int>(_n, 0));

    for (int nodeId = 0; nodeId < _n; nodeId++) {
        for (int alpha = 0; alpha < _w; alpha++) {
            _coefs4Symbols[alpha][nodeId] = _primElementPower[_nodePermutation[nodeId] * _w + alpha];
        }
    }

    // print primitive element
    cout << "STACKCode::STACKCode() Primitive element (root): " << _e << endl;

    // // print primElementPower
    // cout << "STACKCode::STACKCode() Primitive elements power:" << endl;
    // for (int i = 0; i < _order; i++) {
    //     cout << _primElementPower[i] << " ";
    // }
    // cout << endl;

    // print out the coefficients
    cout << "STACKCode::STACKCode() Coefficients for encoding:" << endl;
    for (int i = 0; i < _w; i++) {
        for (int j = 0; j < _n; j++) {
            cout << _coefs4Symbols[i][j] << " ";
        }
        cout << endl;
    }

    // cout << "symbol element correspondence:" << endl;
    // for (int groupId = 0; groupId < _numGroups; groupId++) {
    //     cout << "Group " << groupId << ": ";
    //     for (auto symbolId : _symbolGroups[groupId]) {
    //         int nodeId = symbolId / _w;
    //         int alpha = symbolId % _w;
    //         cout << _elementMap[_coefs4Symbols[alpha][nodeId]] << " ";
    //     }
    //     cout << endl;
    // }

    // // print out the coefficients
    // cout << "STACKCode::STACKCode() Coefficients for encoding (corresponding):" << endl;
    // for (int i = 0; i < _w; i++) {
    //     for (int j = 0; j < _n; j++) {
    //         cout << _elementMap[_coefs4Symbols[i][j]] << " ";
    //     }
    //     cout << endl;
    // }

    /**
     * @brief method 2: directly construct parity check matrix for encoding and decoding
     */
    genEncodingMatrix();

    cout << "STACKCode::STACKCode() Parity-check matrix:" << endl;
    jerasure_print_matrix(_pcMatrix, _m * _w, _n * _w, 8);


    // cout << "STACKCode::STACKCode() Parity-check matrix (corresponding):" << endl;
    // for (int i = 0; i < _m * _w; i++) {
    //     if (i % _m != 1) {
    //         continue;
    //     }
    //     for (int j = 0; j < _n * _w; j++) {
    //         int a = _pcMatrix[i * (_n * _w) + j];
    //         if (a == 0)
    //         {
    //             printf("--- ");
    //         }
    //         else
    //             printf("%03d ", _elementMap[a]);
    //     }
    //     cout << endl;
    // }

    cout << "STACKCode::STACKCode() Generator matrix:" << endl;
    jerasure_print_matrix(_encodeMatrix, _m * _w, _k * _w, 8);
}

STACKCode::~STACKCode() {
    delete [] _pcMatrix;
    delete [] _encodeMatrix;
}

ECDAG* STACKCode::Encode() {
    ECDAG *ecdag = new ECDAG();

    /**
     * @brief method 1: use augmented sub-stripes for encoding
     */
    for (int alpha = 0; alpha < _w; alpha++) {
        vector<int> &symbolGroup = _symbolGroups[alpha];

        // construct RS(as_n, as_k) for the augmented sub-stripe
        int as_n = symbolGroup.size();
        int as_k = as_n - _m;

        // obtain coefficients for each symbol for encoding
        vector<int> coefs4Encoding(as_n, 0);
        for (int i = 0; i < as_n; i++) {
            int symbolNodeId = symbolGroup[i] / _w;
            int symbolAlpha = symbolGroup[i] % _w;
            coefs4Encoding[i] = _coefs4Symbols[symbolAlpha][symbolNodeId];
        }
        
        vector<int> data;
        vector<int> codes;
        for (int nodeId = _k; nodeId < _n; nodeId++) {
            codes.push_back(_layout[alpha][nodeId]);
        }

        for (auto symbolId : symbolGroup) {
            if (find(codes.begin(), codes.end(), symbolId) == codes.end()) {
                data.push_back(symbolId);
            }
        }
        
        // construct parity check matrix for the augmented sub-stripe
        int *pcMatrix4SubStripe = new int[_m * as_n];
        for (int rid = 0; rid < _m; rid++) {
            for (int cid = 0; cid < as_n; cid++) {
                if (rid == 0) {
                    pcMatrix4SubStripe[rid * as_n + cid] = 1;
                } else {
                    pcMatrix4SubStripe[rid * as_n + cid] = galois_single_multiply(pcMatrix4SubStripe[(rid - 1) * as_n + cid], coefs4Encoding[cid], _fw);
                }
            }
        }

        // print matrix
        cout << "STACKCode::Encode() Parity-check matrix for augmented sub-stripe " << alpha << ":" << endl;
        jerasure_print_matrix(pcMatrix4SubStripe, _m, as_n, _fw);

        // obtain encoding matrix
        int *from = new int[as_n];
        int *to = new int[as_n];
        memset(from, 0, as_n * sizeof(int));
        memset(to, 0, as_n * sizeof(int));
        for (int i = 0; i < as_n; i++) {
            if (find(data.begin(), data.end(), symbolGroup[i]) != data.end()) {
                from[i] = 1;
            } else {
                to[i] = 1;
            }
        }
        int *encodeMatrix4SubStripe = new int[as_k * _m];
        if (getGenMatrixFromPCMatrix(as_n, as_k, _fw, pcMatrix4SubStripe, encodeMatrix4SubStripe, from, to) == false) {
            cout << "STACKCode::Encode() failed to obtain encoding matrix for augmented sub-stripe " << alpha << endl;
            exit(-1);
        }

        // // print encoding matrix for augmented sub-stripe
        // cout << "STACKCode::Encode() Encoding matrix for augmented sub-stripe " << alpha << ":" << endl;
        // jerasure_print_matrix(encodeMatrix4SubStripe, _m, as_k, _fw);

        // encode the augmented sub-stripe
        for (int i = 0; i < codes.size(); i++) {
            int code = codes[i];
            vector<int> coef(encodeMatrix4SubStripe + i * as_k, encodeMatrix4SubStripe + (i + 1) * as_k);
            ecdag->Join(code, data, coef);
        }
    }

    /**
     * @brief method 2: use parity check matrix for encoding
     */
    // vector<int> data;
    // vector<int> codes;

    // // data: (k * w)
    // for (int nodeId = 0; nodeId < _k; nodeId++) {
    //     for (int alpha = 0; alpha < _w; alpha++) {
    //         data.push_back(_layout[alpha][nodeId]);
    //     }
    // }

    // for (int nodeId = _k; nodeId < _n; nodeId++) {
    //     for (int alpha = 0; alpha < _w; alpha++) {
    //         codes.push_back(_layout[alpha][nodeId]);
    //     }
    // }

    // for (int i = 0; i < codes.size(); i++) {
    //     int code = codes[i];
    //     vector<int> coef(_encodeMatrix + i * _k * _w, _encodeMatrix + (i + 1) * _k * _w);
    //     ecdag->Join(code, data, coef);
    // }

    // if (codes.size() > 1) {
    //     int vidx = ecdag->BindX(codes);
    //     ecdag->BindY(vidx, data[0]);
    // }

    return ecdag;
}

ECDAG* STACKCode::Decode(vector<int> from, vector<int> to) {
    if (to.size() == _w) {
        int failedNodeId = to[0] / _w;
        return decodeSingle(from, to);
    } else {
        return decodeMultiple(from, to);
    }
}

void STACKCode::Place(vector<vector<int>>& group) {

}

void STACKCode::genParityCheckMatrix() {
    int rows = _m * _w;
    int cols = _n * _w;

    _pcMatrix = new int[rows * cols];
    memset(_pcMatrix, 0, rows * cols * sizeof(int));

    int *pcMatrixWithoutPermutate = new int[rows * cols];
    memset(pcMatrixWithoutPermutate, 0, rows * cols * sizeof(int));

    for (int x = 0; x < rows; x++)
    {
        for (int y = 0; y < cols; y++)
        {
            // int a = y / _w / g;
            int b = y / _w % _numGroups; // group id
            int j = y % _w; // the j-th sub-mtx in col
            int i = x / _m; // the i-th sub-mtx in row
            int t = x % _m;
            if (i == j || i == b)
            { // diagonal or 
                pcMatrixWithoutPermutate[x * cols + y] = _primElementPower[(y * t) % _order];
            }
        }
    }

    // // print pcMatrixWithoutPermutate
    // cout << "STACKCode::genParityCheckMatrix() Parity-check matrix without permutation:" << endl;
    // jerasure_print_matrix(pcMatrixWithoutPermutate, rows, cols, _fw);

    // convert the parity check matrix based on node permutation
    for (int nodeId = 0; nodeId < _n; nodeId++) {
        int permNodeId = _nodePermutation[nodeId];
        for (int alpha = 0; alpha < _w; alpha++) {
            for (int rid = 0; rid < rows; rid++) {
                _pcMatrix[rid * cols + nodeId * _w + alpha] = pcMatrixWithoutPermutate[rid * cols + permNodeId * _w + alpha];
            }
        }
    }

    // // print pcmatrix after permute
    // cout << "STACKCode::genParityCheckMatrix() Parity-check matrix after permutation:" << endl;
    // jerasure_print_matrix(_pcMatrix, rows, cols, _fw);
}

void STACKCode::genEncodingMatrix() {
    genParityCheckMatrix();
    if (convertPCMatrix2EncMatrix(_n, _k, _w) == false) {
        cout << "STACKCode::genEncodingMatrix() failed to obtain encoding matrix" << endl;
        exit(-1);
    }
}

bool STACKCode::convertPCMatrix2EncMatrix(int n, int k, int w) {
    int *from = new int[n * w];
    int *to = new int[n * w];
    memset(from, 0, n * w * sizeof(int));
    memset(to, 0, n * w * sizeof(int));

    // the first k*w symbols are data symbols, next (n-k)*w symbols are parity
    // symbols
    for (int nodeId = 0; nodeId < k; nodeId++) {
        for (int alpha = 0; alpha < w; alpha++) {
            from[nodeId * w + alpha] = 1;
        }
    }

    for (int nodeId = k; nodeId < n; nodeId++) {
        for (int alpha = 0; alpha < w; alpha++) {
            to[nodeId * w + alpha] = 1;
        }
    }

    _encodeMatrix = new int[k * w * n * w];
    bool ret = getGenMatrixFromPCMatrix(n * w, k * w, _fw, _pcMatrix, _encodeMatrix, from, to);

    delete [] from;
    delete [] to;

    return ret;
}

bool STACKCode::getGenMatrixFromPCMatrix(int n, int k, int fw, const int* pcMatrix, int *genMatrix, const int* from, const int* to) {
    int m = n - k;
    int numFailedSymbols = 0;
    int numAvailSymbols = 0;
    for (int i = 0; i < n; i++)
    {
        if (to[i])
        {
            numFailedSymbols++;
        }
        if (from[i])
        {
            numAvailSymbols++;
        }
        if (to[i] == 1 && from[i] == 1)
        {
            cout << "STACKCode::convertPCMatrix2GenMatrix() numFailedSymbols node " << i << " cannot be used for decoding" << endl;
            return false;
        }
    }
    if (numFailedSymbols > m)
    {
        cout << "STACKCode::convertPCMatrix2GenMatrix() Too many failed symbols to decode: " << numFailedSymbols << ", max allowed: " << m << endl;
        return false;
    }
    if (numAvailSymbols != k)
    {
        cout << "STACKCode::convertPCMatrix2GenMatrix() Invalid number of available symbols: " << numAvailSymbols << ", k: " << k << std::endl;
        return false;
    }

    int *H1 = new int[m * m]; // number of failed nodes
    int *H2 = new int[m * k]; // number of helper nodes
    int *H1_inverse = new int[m * m];

    // copy the matrix to corresponding sub-matrix
    for (int j = 0, j1 = 0, j2 = 0; j < n; j++)
    {
        if (!from[j]) {
            for(int i = 0; i < m; i++){
                H1[i * m + j1] = pcMatrix[i * n + j];
            }
            j1++;
        } else{
            for (int i = 0; i < m; i++) {
                H2[i * k + j2] = pcMatrix[i * n + j];
            }
            j2++;
        }
    }

    // inverse H1
    if (jerasure_invert_matrix(H1, H1_inverse, m, fw) == -1) {
        cout << "STACKCode::convertPCMatrix2GenMatrix() failed to invert H1" << endl;
        exit(-1);
    }

    // multiply H1_inverse and H2
    int *tmpMatrix = jerasure_matrix_multiply(H1_inverse, H2, m, m, m, k, fw);
    delete[] H1;
    delete[] H2;
    delete[] H1_inverse;

    // tmpMatrix has exactly m rows
    if (numFailedSymbols == m)
    {
        memcpy(genMatrix, tmpMatrix, m * k * sizeof(int));
    } else {
        // only needs numFailedSymbols < m rows

        for (int x = 0, y = 0, z = 0; x < n; x++) {
            if (!from[x]) {
                if (to[x]) {
                    memcpy(genMatrix + z * k, tmpMatrix + y * k, k * sizeof(int));
                    z++;
                }
                y++;
            }
        }
    }

    free(tmpMatrix);
    return true;
}

void STACKCode::initLayout() {
    int symbolId = 0;
    _layout.resize(_w, vector<int>(_n, 0));
    for (int nodeId = 0; nodeId < _n; nodeId++) {
        for (int alpha = 0; alpha < _w; alpha++) {
            _layout[alpha][nodeId] = symbolId++;
        }
    }
}

bool STACKCode::genDecodingMatrix(vector<int> &availSymbols, vector<int> &failedSymbols, int *decodeMatrix) {
    int *from = new int[_n * _w];
    int *to = new int[_n * _w];
    memset(to, 0, _n * _w * sizeof(int));

    for (int symbolId = 0; symbolId < _n * _w; symbolId++) {
        from[symbolId] = 1;
    }

    // mark failed / available symbols
    for (auto failedSymbol : failedSymbols) {
        to[failedSymbol] = 1;
        from[failedSymbol] = 0;
    }

    // only needs to retrieve the first _k * _w available symbols
    int numRedundantSymbols = _m * _w - failedSymbols.size();
    for (int i = _n * _w - 1; (i >= 0) && (numRedundantSymbols > 0); i--) {
        if (from[i] == 1 && to[i] == 0) {
            from[i] = 0;
            numRedundantSymbols--;

            // in place update availSymbols
            availSymbols.erase(std::remove(availSymbols.begin(), availSymbols.end(), i), availSymbols.end());
        }
    }

    // // print from and to
    // cout << "STACKCode::genDecodingMatrix() from:" << endl;
    // for (int i = 0; i < _n * _w; i++) {
    //     cout << from[i] << " ";
    // }
    // cout << endl;

    // cout << "STACKCode::genDecodingMatrix() to:" << endl;
    // for (int i = 0; i < _n * _w; i++) {
    //     cout << to[i] << " ";
    // }
    // cout << endl;

    // cout << "STACKCode::STACKCode() Parity-check matrix (corresponding):" << endl;
    // for (int i = 0; i < _m * _w; i++) {
    //     if (i % _m != 1) {
    //         continue;
    //     }
    //     for (int j = 0; j < _n * _w; j++) {
    //         int a = _pcMatrix[i * (_n * _w) + j];
    //         if (a == 0)
    //         {
    //             printf("--- ");
    //         }
    //         else
    //             printf("%03d ", _elementMap[a]);
    //     }
    //     cout << endl;
    // }

    bool ret = getGenMatrixFromPCMatrix(_n * _w, _k * _w, _fw, _pcMatrix, decodeMatrix, from, to);

    delete [] from;
    delete [] to;

    return ret;
}

ECDAG *STACKCode::decodeSingle(vector<int> &availSymbols, vector<int> &failedSymbols) {
    /**
     * @brief method 1: use augmented sub-stripes for single failure repair
     */
    return decodeMultipleWithSubStripes(availSymbols, failedSymbols);

    // /**
    //  * @brief method 2: use parity check matrix for single failure repair
    //  */

    // ECDAG *ecdag = new ECDAG();

    // // symbols to repair
    // int failedNode = failedSymbols[0] / _w;
    // int groupId = failedNode % _numGroups;
    // int repairBW = getRepairBandwidth(failedNode);
    // vector<int> helperNodeIds = getHelperNodes(failedNode);

    // vector<int> availSymbols;
    // vector<int> failedSymbols;
    // for (int alpha = 0; alpha < _w; alpha++)
    // {
    //     failedSymbols.push_back(_layout[alpha][failedNode]);
    // }

    // for (int nodeId = 0; nodeId < _n; nodeId++)
    // {
    //     if (helperNodeIds[nodeId])
    //     {
    //         // put all nodes in the group
    //         if (nodeId % _numGroups == groupId)
    //         {
    //             for (int alpha = 0; alpha < _w; alpha++)
    //             {
    //                 availSymbols.push_back(_layout[alpha][nodeId]);
    //             }
    //         }
    //         else if (groupId < _w)
    //         {
    //             availSymbols.push_back(_layout[groupId][nodeId]);
    //         }
    //         else
    //         {
    //             availSymbols.push_back(_layout[nodeId % _numGroups][nodeId]);
    //         }
    //     }
    // }

    // int *repairMatrix = getRepairMatrix(failedNode);

    // cout << "STACKCode::decodeSingle() Repair Matrix: " << endl;
    // jerasure_print_matrix(repairMatrix, _m, repairBW, _fw);

    // for (int i = 0; i < _w; i++) {
    //     vector<int> &data = availSymbols;
    //     vector<int> coef(repairMatrix + i * repairBW, repairMatrix + (i + 1) * repairBW);
    //     int code = failedSymbols[i];
    //     ecdag->Join(code, data, coef);
    // }

    // return ecdag;
}

int *STACKCode::getRepairMatrix(int failedNode) {
    int groupId = failedNode % _numGroups;
    int repairBW = getRepairBandwidth(failedNode);
    vector<int> helperNodeIds = getHelperNodes(failedNode);

    int numRows = _m;
    // int cols = r + beta;
    int numCols1 = _m;
    int numCols2 = repairBW;

    int before = 0;
    for (int nodeId = 0; nodeId < _n; nodeId++)
    {
        if (nodeId >= failedNode) {
            break;
        }
        if (!helperNodeIds[nodeId])
        {
            before += (((nodeId % _numGroups) == groupId) ? (_w) : (1));
        }
    }

    int *R1 = new int[numRows * numCols1];
    int *R2 = new int[numRows * numCols2];

    /**
     * get R1 and R2
     */
    int start_col1 = 0;
    int start_col2 = 0;
    for (int a = 0; a < _n; a++)
    {
        if (helperNodeIds[a])
        {
            if (a % _numGroups == groupId)
            {
                for (int alpha = 0; alpha < _w; alpha++)
                {
                    for (int t = 0; t < _m; t++)
                    {
                        R2[t * numCols2 + start_col2 + alpha] = _primElementPower[((a * _w + alpha) * t) % _order];
                    }
                }
                start_col2 += _w;
            }
            else if (groupId == _w)
            {
                for (int t = 0; t < _m; t++)
                {
                    R2[t * numCols2 + start_col2] = _primElementPower[((a * _w + a % _numGroups) * t) % _order];
                }
                start_col2 += 1;
            }
            else
            {
                for (int t = 0; t < _m; t++)
                {
                    R2[t * numCols2 + start_col2] = _primElementPower[((a * _w + groupId) * t) % _order];
                }
                start_col2 += 1;
            }
        }
        else
        {
            if (a % _numGroups == groupId)
            {
                for (int alpha = 0; alpha < _w; alpha++)
                {
                    for (int t = 0; t < _m; t++)
                    {
                        R1[t * numCols1 + start_col1 + alpha] = _primElementPower[((a * _w + alpha) * t) % _order];
                    }
                }
                start_col1 += _w;
            }
            else if (groupId == _w)
            {
                for (int t = 0; t < _m; t++)
                {
                    R1[t * numCols1 + start_col1] = _primElementPower[((a * _w + a % groupId) * t) % _order];
                }
                start_col1 += 1;
            }
            else
            {
                for (int t = 0; t < _m; t++)
                {
                    R1[t * numCols1 + start_col1] = _primElementPower[((a * _w + groupId) * t) % _order];
                }
                start_col1 += 1;
            }
        }
    }

    // print_mat(R1, numRows, numCols1);
    // print_mat(R2, numRows, numCols2);

    int *R1_inverse = new int[numRows * numCols1];
    jerasure_invert_matrix(R1, R1_inverse, numRows, _fw);
    int *temp = jerasure_matrix_multiply(R1_inverse, R2, numRows, numCols1, numRows, numCols2, _fw);
    // print_mat(temp, numRows, numCols2);
    delete[] R1;
    delete[] R2;
    delete[] R1_inverse;
    if (numCols1 == _w)
    {
        return temp;
    }
    else
    {
        int *R = (int *)malloc(sizeof(int) * _w * numCols2);
        for (int i = 0; i < _w; i++)
        {
            memcpy(R + i * numCols2, temp + (i + before) * numCols2, sizeof(int) * numCols2);
        }
        free(temp);
        return R;
    }
}

ECDAG *STACKCode::decodeMultiple(vector<int> &availSymbols, vector<int> &failedSymbols) {
    bool canDecodeWithSubStripes = false;

    // identify failed nodes
    vector<int> failedNodes;
    for (auto symbolId : failedSymbols) {
        int nodeId = symbolId / _w;
        if (find(failedNodes.begin(), failedNodes.end(), nodeId) == failedNodes.end()) {
            failedNodes.push_back(nodeId);
        }
    }

    // identify failed groups
    vector<int> failedGroups;
    for (auto nodeId : failedNodes) {
        for (int i = 0; i < _numGroups; i++) {
            if (find(_nodeGroups[i].begin(), _nodeGroups[i].end(), nodeId) != _nodeGroups[i].end()) {
                if (find(failedGroups.begin(), failedGroups.end(), i) == failedGroups.end()) {
                    failedGroups.push_back(i);
                }
                break;
            }
        }
    }

    // print failed nodes
    cout << "STACKCode::decodeMultiple() failed nodes: ";
    for (auto nodeId : failedNodes) {
        cout << nodeId << " ";
    }
    cout << endl;
    
    // print failed groups
    cout << "STACKCode::decodeMultiple() failed groups: ";
    for (auto groupId : failedGroups) {
        cout << groupId << " ";
    }
    cout << endl;

    // can tolerate at most t = _m / _w failures within one group
    if (failedGroups.size() == 1 && failedNodes.size() <= (_m / _w)) {
        canDecodeWithSubStripes = true;
    }

    if (canDecodeWithSubStripes == true) {
        return decodeMultipleWithSubStripes(availSymbols, failedSymbols);
    } else {
        return decodeMultipleWithPCMatrix(availSymbols, failedSymbols);
    }
}

ECDAG *STACKCode::decodeMultipleWithSubStripes(vector<int> &availSymbols, vector<int> &failedSymbols) {
    ECDAG *ecdag = new ECDAG();

    // use augmented sub-stripes for multiple failures repair
    cout << "STACKCode::decodeMultipleWithSubStripes() Multiple failures repair using augmented sub-stripes" << endl;

    // identify failed nodes
    vector<int> failedNodes;
    for (auto symbolId : failedSymbols) {
        int nodeId = symbolId / _w;
        if (find(failedNodes.begin(), failedNodes.end(), nodeId) == failedNodes.end()) {
            failedNodes.push_back(nodeId);
        }
    }
    int residingGroupId = -1;
    for (int i = 0; i < _numGroups; i++)
    {
        if (find(_nodeGroups[i].begin(), _nodeGroups[i].end(), failedNodes[0]) != _nodeGroups[i].end())
        {
            residingGroupId = i;
            break;
        }
    }
    
    // obtain available data symbols
    vector<int> data;
    vector<int> codes;
    vector<int> symbolGroup = _symbolGroups[residingGroupId];
    int as_n = symbolGroup.size();
    int as_k = symbolGroup.size() - _m;

    // obtain coefficients for each symbol for decoding
    vector<int> coefs4Decoding(as_n, 0);

    int *from = new int[as_n];
    int *to = new int[as_n];
    memset(from, 0, as_n * sizeof(int));
    memset(to, 0, as_n * sizeof(int));

    for (int i = 0, count = 0; i < as_n; i++) {
        int symbol = symbolGroup[i];
        int symbolNodeId = symbol / _w;
        int symbolAlpha = symbol % _w;
        coefs4Decoding[i] = _coefs4Symbols[symbolAlpha][symbolNodeId];
        if (find(failedNodes.begin(), failedNodes.end(), symbolNodeId) != failedNodes.end()) {
            // failed node
            to[i] = 1;
            codes.push_back(symbol);
        } else {
            if (count < as_k) {
                // available node
                from[i] = 1;
                data.push_back(symbol);
                count++;
            }
        }
    }
        
    // construct parity check matrix for the augmented sub-stripe
    int *pcMatrix4SubStripe = new int[_m * as_n];
    for (int rid = 0; rid < _m; rid++) {
        for (int cid = 0; cid < as_n; cid++) {
            if (rid == 0) {
                pcMatrix4SubStripe[rid * as_n + cid] = 1;
            } else {
                pcMatrix4SubStripe[rid * as_n + cid] = galois_single_multiply(pcMatrix4SubStripe[(rid - 1) * as_n + cid], coefs4Decoding[cid], _fw);
            }
        }
    }

    // print matrix
    cout << "STACKCode::Encode() Parity-check matrix for augmented sub-stripe " << residingGroupId << ":" << endl;
    jerasure_print_matrix(pcMatrix4SubStripe, _m, as_n, _fw);

    int *decodeMatrix4SubStripe = new int[as_k * _m];
    if (getGenMatrixFromPCMatrix(as_n, as_k, _fw, pcMatrix4SubStripe, decodeMatrix4SubStripe, from, to) == false) {
        cout << "STACKCode::Encode() failed to obtain decoding matrix for augmented sub-stripe " << residingGroupId << endl;
        exit(-1);
    }

    // print encoding matrix for augmented sub-stripe
    cout << "STACKCode::DecodeSingle() Decoding matrix for augmented sub-stripe " << residingGroupId << ":" << endl;
    jerasure_print_matrix(decodeMatrix4SubStripe, _m, as_k, _fw);

    // encode the augmented sub-stripe
    for (int i = 0; i < codes.size(); i++) {
        int code = codes[i];
        vector<int> coef(decodeMatrix4SubStripe + i * as_k, decodeMatrix4SubStripe + (i + 1) * as_k);
        ecdag->Join(code, data, coef);
    }

    return ecdag;
}

ECDAG *STACKCode::decodeMultipleWithPCMatrix(vector<int> &availSymbols, vector<int> &failedSymbols) {
    ECDAG *ecdag = new ECDAG();

    int *decodeMatrix = new int[_m * _w * _k * _w];
    if (genDecodingMatrix(availSymbols, failedSymbols, decodeMatrix) == false) {
        cout << "STACKCode::decodeMultiple() failed to generate decode matrix" << endl;
        return ecdag;
    }

    // print decode matrix
    cout << "STACKCode::decodeMultiple() Decoding matrix:" << endl;
    jerasure_print_matrix(decodeMatrix, _m * _w, _k * _w, 8);

    // data: (k * w)
    vector<int> &data = availSymbols;
    vector<int> &codes = failedSymbols;

    for (int i = 0; i < codes.size(); i++) {
        int code = codes[i];
        vector<int> coef(decodeMatrix + i * _k * _w, decodeMatrix + (i + 1) * _k * _w);
        ecdag->Join(code, data, coef);
    }

    if (codes.size() > 1) {
        int vidx = ecdag->BindX(codes);
        ecdag->BindY(vidx, data[0]);

    }
    
    delete [] decodeMatrix;

    return ecdag;
}

void STACKCode::getPrimElementsPower(int order, int e, int fw)
{
    _primElementPower.resize(order);
    _primElementPower[0] = 1;
    for (int i = 1; i < order; i++)
    {
        _primElementPower[i] = galois_single_multiply(_primElementPower[i - 1], e, fw);
    }
}

int STACKCode::getAvailPrimElements(int n, int k, int w, int fw) {
    // invalid cases
    if (k <= 0 || n - k < 2) {
        return 0;
    }
    else if (n * w > 255) {
        return 0;
    }
    else if (w > n - k) {
        return 0;
    }
    else if (n >= 100) {
        return 0;
    }

    /**
     * For the case of the number of parity nodes is 2,
     * all primitive elements of GF(2^8) can be used to
     * generate the required n * w elements.
     */
    if (n - k == 2) {
        return 2;
    }

    int f = 0;

    int flag = n * 10000 + k * 100 + w;

    switch (flag)
    {
    case 80402:         // n = 8, k = 4, w = 2
        f = 0b00101011; // f = x^8 + x^5 + x^3 + x + 1
        break;

    case 80403:         // n = 8, k = 4, w = 3
        f = 0b00011101; // f = x^8 + x^4 + x^3 + x^2 + 1
        break;

    case 80404:         // n = 8, k = 4, w = 4
        f = 0b00011101; // f = x^8 + x^4 + x^3 + x^2 + 1
        break;

    case 90602:         // n = 9, k = 6, w = 2
        f = 0b00011101; // f = x^8 + x^4 + x^3 + x^2 + 1
        break;

    case 90603:         // n = 9, k = 6, w = 3
        f = 0b00101011; // f = x^8 + x^5 + x^3 + x + 1
        break;

    case 100602:        // n = 10, k = 6, w = 2
        f = 0b00101011; // f = x^8 + x^5 + x^3 + x + 1
        break;

    case 100603:        // n = 10, k = 6, w = 3
        f = 0b00011101; // f = x^8 + x^4 + x^3 + x^2 + 1
        break;

    case 100604:        // n = 10, k = 6, w = 4
        f = 0b00011101; // f = x^8 + x^4 + x^3 + x^2 + 1
        break;

    case 110802:        // n = 11, k = 8, w = 2
        f = 0b00011101; // f = x^8 + x^4 + x^3 + x^2 + 1
        break;

    case 110803:        // n = 11, k = 8, w = 3
        f = 0b00101011; // f = x^8 + x^5 + x^3 + x + 1
        break;

    case 120802:        // n = 12, k = 8, w = 2
        f = 0b00101011; // f = x^8 + x^5 + x^3 + x + 1
        break;

    case 120803:        // n = 12, k = 8, w = 3
        f = 0b00011101; // f = x^8 + x^4 + x^3 + x^2 + 1
        break;

    case 120804:        // n = 12, k = 8, w = 4
        f = 0b00101101; // f = x^8 + x^5 + x^3 + x^2 + 1
        break;

    case 141002:        // n = 14, k = 10, w = 2
        f = 0b00101011; // f = x^8 + x^5 + x^3 + x + 1
        break;

    case 141003:        // n = 14, k = 10, w = 3
        f = 0b00011101; // f = x^8 + x^4 + x^3 + x^2 + 1
        break;

    case 141004:        // n = 14, k = 10, w = 4
        f = 0b00101101; // f = x^8 + x^5 + x^3 + x^2 + 1
        break;

    case 161202:        // n = 16, k = 12, w = 2
        f = 0b00101011; // f = x^8 + x^5 + x^3 + x + 1
        break;

    case 161203:        // n = 16, k = 12, w = 3
        f = 0b00011101; // f = x^8 + x^4 + x^3 + x^2 + 1
        break;

    case 161204:        // n = 16, k = 12, w = 4
        f = 0b00101101; // f = x^8 + x^5 + x^3 + x^2 + 1
        break;

    case 181502:        // n = 18, k = 15, w = 2
        f = 0b00011101; // f = x^8 + x^4 + x^3 + x^2 + 1
        break;

    case 181503:        // n = 18, k = 15, w = 3
        f = 0b00101101; // f = x^8 + x^5 + x^3 + x^2 + 1
        break;

    case 201702:        // n = 20, k = 17, w = 2
        f = 0b00011101; // f = x^8 + x^4 + x^3 + x^2 + 1
        break;

    case 201703:        // n = 20, k = 17, w = 3
        f = 0b00101101; // f = x^8 + x^5 + x^3 + x^2 + 1
        break;

    default:
        f = 0;
    }

    return findRoot(f, fw);
}

bool STACKCode::getAvailPrimElements(int n, int k, int w, int &fw, uint32_t &e, uint32_t &f) {
    
    // no available primitive elements: k <= 1; n - k == 1
    if (k <= 1 || n <= k + 1)
    {
        f = 0;
        fw = 0;
        e = 0;
        return false;
    }

    // reset fw, f, e
    fw = 0;
    f = 0;
    e = 0;

    if (n - k == 2)
    {
        if (n * w <= ((1 << 8) - 1))
        {
            fw = 8;
            f = 0b100011101; // x^8 + x^4 + x^3 + x^2 + 1
            e = 2;
        }
        else if (n * w <= ((1 << 16) - 1))
        {
            fw = 16;
            f = 0b10000000000101101; // x^{16} + x^5 + x^3 + x^2 + 1
            e = 2;
        }
    }
    else if (n - k == 3)
    {
        if (w == 2)
        {
            if (n <= 44)
            {
                f = 0b111001111; // x^8 + x^7 + x^6 + x^3 + x^2 + x + 1
                fw = 8;
            }
            else if (n <= 127)
            {
                f = 0b10000000000101101; // x^{16} + x^5 + x^3 + x^2 + 1
                fw = 16;
            }
        }
        else if (w == 3)
        {
            if (n <= 40)
            {
                f = 0b100101101; // x^8 + x^5 + x^3 + x^2 + 1
                fw = 8;
            }
            else if (n <= 127)
            {
                f = 0b10000000000101101; // x^{16} + x^5 + x^3 + x^2 + 1
                fw = 16;
            }
        }
    }
    else if (n - k == 4)
    {
        if (w == 2)
        {
            if (n <= 23)
            {
                f = 0b100101011; // x^8 + x^5 + x^3 + x + 1
                fw = 8;
            }
            else if (n <= 127)
            {
                f = 0b10000001101110001; // x^{16} + x^9 + x^8 + x^6 + x^5 + x^4 + 1
                fw = 16;
            }
        }
        else if (w == 3)
        {
            if (n <= 17)
            {
                f = 0b100011101; // x^8 + x^4 + x^3 + x^2 + 1
                fw = 8;
            }
            else if (n <= 127)
            {
                f = 0b10001001011110011; // x^{16} + x^{12} + x^9 + x^7 + x^6 + x^5 + x^4 + x + 1
                fw = 16;
            }
        }
        else if (w == 4)
        {
            if (n <= 16)
            {
                f = 0b100101101; // x^8 + x^5 + x^3 + x^2 + 1
                fw = 8;
            }
            else if (n <= 127)
            {
                f = 0b10000011101111001; // x^{16} + x^{10} + x^9 + x^8 + x^6 + x^5 + x^4 + x^3 + 1
                fw = 16;
            }
        }
    }

    if (f)
    {
        e = findRoot(f, fw);
        //printf("r = %d, w = %d:\n   n = %d\n    w = %2d, f(x) = ", r, w, n, w);
        //print_polynomial(f);
        //printf("    available element: %d\n\n", e);
    }

    return e != 0;
}

uint32_t STACKCode::findRoot(uint32_t f, int fw)
{
    for (uint32_t root = 1; ; root++)
    {
        if (root == 0 || root > ((1 << fw) - 1)) {
            return 0;
        }
        if (polynomialAssignment(root, f, fw) != 0) {
            continue;
        }
        return root;
    }
}

uint32_t STACKCode::polynomialAssignment(uint32_t x, uint32_t f, int fw)
{
    uint32_t fx = 0;
    for (int i = 31; i >= 0; i--)
    {
        fx = galois_single_multiply(fx, x, fw);
        // fx *= x;
        // fx = gf_mult(fx, x, w);
        fx ^= ((f >> i) & (1));
    }
    return fx;
}

vector<int> STACKCode::getNodeSubPackets(int nodeid) {
    vector<int> symbols;
    for (int i = 0; i < _w; i++) {
        symbols.push_back(_layout[i][nodeid]);
    }

    return symbols;
}

vector<vector<int>> STACKCode::GetSubPackets() {
    return _layout;
}

int STACKCode::getRepairBandwidth(int failedNode)
{
    int groupId = failedNode % _numGroups;
    if (groupId < _n % _numGroups)
    {
        return (_n / _numGroups + 1) * (_w - 1) + _k;
    }
    else
    {
        return (_n / _numGroups) * (_w - 1) + _k;
    }
}

vector<int> STACKCode::getHelperNodes(int failedNode)
{
    int groupId = failedNode % _numGroups;
    vector<int> helperNodeIds(_n, 1);
    for (int i = 0; i < _n; i++)

    helperNodeIds[failedNode] = 0;
    if (_w < _m)
    {
        int count = _m - _w;
        int single = (groupId < (_n % _numGroups)) ? (_n - _n / _numGroups - 1) : (_n - _n / _numGroups);

        for (int i = 0; (i < _n) && (single < count); i++)
        {
            if (i % _numGroups == groupId)
            {
                helperNodeIds[i] = 0;
                count -= _w;
            }
        }

        for (int i = 0; (i < _n) && (count > 0); i++)
        {
            if (i % _numGroups != groupId)
            {
                helperNodeIds[i] = 0;
                count -= 1;
            }
        }
    }
    
    return helperNodeIds;
}