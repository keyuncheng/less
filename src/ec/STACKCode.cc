#include "STACKCode.hh"

STACKCode::STACKCode(int n, int k, int w, int opt, vector<string> param) {
    _n = n;
    _k = k;
    _w = w; // sub-packetization
    _opt = opt;
    _m = n - k;
    _numGroups = _w + 1; // num_groups = sub-packetization + 1

    // field width (default: 8)
    _fw = 8;
    if (param.size() == 1) {
        if (param[0] != "-") {
            _fw = atoi(param[0].c_str());
        }
    }

    if (_fw != 8) {
        cout << "STACKCode::STACKCode() Currently only supports fw=8" << endl;
        exit(1);
    }

    _e = getAvailPrimElements(_n, _k, _w, _fw);

    if (_e == 0)
    {
        cout << "STACKCode::STACKCode() failed to find available primitive elements in GF(2^" << _fw << ")" << endl;
        exit(-1);
    }

    _order = (1 << _fw) - 1;
    getPrimElementsPower(_order, _e, _fw);
    genEncodingMatrix();

    cout << "STACKCode::STACKCode() Parity-check matrix:" << endl;
    jerasure_print_matrix(_pcMatrix, _m * _w, _n * _w, 8);

    cout << "STACKCode::STACKCode() Generator matrix:" << endl;
    jerasure_print_matrix(_encodeMatrix, _m * _w, _k * _w, 8);

    initLayout();
}

STACKCode::~STACKCode() {
    delete [] _pcMatrix;
    delete [] _encodeMatrix;
}

ECDAG* STACKCode::Encode() {
    ECDAG *ecdag = new ECDAG();
    vector<int> data;
    vector<int> codes;

    // data: (k * w)
    for (int nodeId = 0; nodeId < _k; nodeId++) {
        for (int alpha = 0; alpha < _w; alpha++) {
            data.push_back(_layout[alpha][nodeId]);
        }
    }

    for (int nodeId = _k; nodeId < _n; nodeId++) {
        for (int alpha = 0; alpha < _w; alpha++) {
            codes.push_back(_layout[alpha][nodeId]);
        }
    }

    for (int i = 0; i < codes.size(); i++) {
        int code = codes[i];
        vector<int> coef(_encodeMatrix + i * _k * _w, _encodeMatrix + (i + 1) * _k * _w);
        ecdag->Join(code, data, coef);
    }

    if (codes.size() > 1) {
        int vidx = ecdag->BindX(codes);
        ecdag->BindY(vidx, data[0]);

    }

    return ecdag;
}

ECDAG* STACKCode::Decode(vector<int> from, vector<int> to) {
    if (to.size() == _w) {
        int failedNodeId = to[0] / _w;
        return decodeSingle(failedNodeId);
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
                _pcMatrix[x * cols + y] = _primElementPower[(y * t) % _order];
            }
        }
    }
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
    jerasure_invert_matrix(H1, H1_inverse, m, fw);

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

    bool ret = getGenMatrixFromPCMatrix(_n * _w, _k * _w, _fw, _pcMatrix, decodeMatrix, from, to);

    delete [] from;
    delete [] to;

    return ret;
}

ECDAG *STACKCode::decodeSingle(int failedNode) {
    ECDAG *ecdag = new ECDAG();

    // symbols to repair
    int groupId = failedNode % _numGroups;
    int repairBW = getRepairBandwidth(failedNode);
    vector<int> helperNodeIds = getHelperNodes(failedNode);

    vector<int> availSymbols;
    vector<int> failedSymbols;
    for (int alpha = 0; alpha < _w; alpha++)
    {
        failedSymbols.push_back(_layout[alpha][failedNode]);
    }

    for (int nodeId = 0; nodeId < _n; nodeId++)
    {
        if (helperNodeIds[nodeId])
        {
            // put all nodes in the group
            if (nodeId % _numGroups == groupId)
            {
                for (int alpha = 0; alpha < _w; alpha++)
                {
                    availSymbols.push_back(_layout[alpha][nodeId]);
                }
            }
            else if (groupId < _w)
            {
                availSymbols.push_back(_layout[groupId][nodeId]);
            }
            else
            {
                availSymbols.push_back(_layout[nodeId % _numGroups][nodeId]);
            }
        }
    }

    int *repairMatrix = getRepairMatrix(failedNode);

    cout << "STACKCode::decodeSingle() Repair Matrix: " << endl;
    jerasure_print_matrix(repairMatrix, _m, repairBW, _fw);

    for (int i = 0; i < _w; i++) {
        vector<int> &data = availSymbols;
        vector<int> coef(repairMatrix + i * repairBW, repairMatrix + (i + 1) * repairBW);
        int code = failedSymbols[i];
        ecdag->Join(code, data, coef);
    }

    return ecdag;
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
    // TBD
    ECDAG *ecdag = new ECDAG();

    int *decodeMatrix = new int[_k * _w * _n * _w];
    if (genDecodingMatrix(availSymbols, failedSymbols, decodeMatrix) == false) {
        cout << "STACKCode::decodeMultiple() failed to generate decode matrix" << endl;
        return ecdag;
    }

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

int STACKCode::findRoot(int f, int fw)
{
    for (int root = 1; ; root++)
    {
        if (root == 0) {
            return 0;
        }
        if (polynomialAssignment(root, f, fw) != 0) {
            continue;
        }
        return root;
    }
}

int STACKCode::polynomialAssignment(int x, int f, int fw)
{
    int fx = 1;
    for (int i = fw - 1; i >= 0; i--)
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