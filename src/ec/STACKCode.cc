#include "STACKCode.hh"

STACKCode::STACKCode(int n, int k, int w, int opt, vector<string> param) {
    _n = n;
    _k = k;
    _w = w; // sub-packetization
    _opt = opt;
    _m = n - k;
    _num_groups = _w + 1; // num_groups = sub-packetization + 1

    // field width (default: 8)
    _fw = 8;
    if (param.size() == 1) {
        _fw = atoi(param[0].c_str());
    }

    if (_fw != 8) {
        cout << "STACKCode::STACKCode() Currently only supports fw=8" << endl;
        exit(1);
    }
    
    _e = getAvailPrimElements(_n, _k, _fw);

    if (_e == 0)
    {
        cout << "STACKCode::STACKCode() failed to find available primitive elements in GF(2^" << w << ")" << endl;
        exit(-1);
    }

    _order = (1 << _fw) - 1;
    getPrimElementsPower(_order, _e, _fw);
    genEncodingMatrix();

    initLayout();
}

ECDAG* STACKCode::Encode() {
    ECDAG *ecdag = new ECDAG();
    vector<int> data;
    vector<int> codes;

    // data: (k * w)
    for (int nodeId = _k; nodeId < _n; nodeId++) {
        for (int alpha = 0; alpha < _w; alpha++) {
            data.push_back(_layout[nodeId][alpha]);
        }
    }

    // parity: (m * w)
    for (int nodeId = _k; nodeId < _n; nodeId++) {
        for (int alpha = 0; alpha < _w; alpha++) {
            int parityId = nodeId - _k;
            int code = _layout[alpha][nodeId];
            vector<int> coef(_encodeMatrix[parityId * _k * _w], _encodeMatrix[parityId * _k * _w] + _k * _w);
            ecdag->Join(code, data, coef);
            codes.push_back(code);
        }
    }

    if (_m * _w > 1) {
        int vidx = ecdag->BindX(codes);
        ecdag->BindY(vidx, data[0]);

    }

    return ecdag;
}

ECDAG* STACKCode::Decode(vector<int> from, vector<int> to) {
    if (to.size() == 1) {
        return DecodeSingle(from, to);
    } else {
        return DecodeMultiple(from, to);
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
            int b = y / _w % _num_groups; // group id
            int j = y % _w; // the j-th sub-mtx in col
            int i = x / _m; // the i-th sub-mtx in row
            int t = x % _m;
            if (i == j || i == b)
            { // diagonal or 
                _pcMatrix[x * cols + y] = primitive_element_power[(y * t) % _order];
            }
        }
    }
}

void STACKCode::genEncodingMatrix() {
    genParityCheckMatrix();
    convertPCMatrix2EncMatrix(_n, _k, _w);
}

void STACKCode::convertPCMatrix2EncMatrix(int n, int k, int w) {
    int *from = new int[n * w];
    int *to = new int[n * w];

    // the first k*w symbols are data symbols, next (n-k)*w symbols are parity symbols
    memset(from, 1, k * w * sizeof(int));
    memset(from + k * w, 0, (n - k) * w * sizeof(int));

    memset(to, 0, k * w * sizeof(int));
    memset(to + k * w, 1, (n - k) * w * sizeof(int));

    _encodeMatrix = new int[k * w * n * w];
    convertPCMatrix2GenMatrix(n * w, k * w, w, _pcMatrix, _encodeMatrix, from, to);
    delete from;
    delete to;
}

bool STACKCode::convertPCMatrix2GenMatrix(int n, int k, int fw, const int* pcMatrix, int *genMatrix, const int* from, const int* to) {
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
        return tmpMatrix;
    }

    // only needs numFailedSymbols < m rows
    int *retMatrix = (int*) malloc(numFailedSymbols * k * sizeof(int));

    for (int x = 0, y = 0, z = 0; x < n; x++) {
        if (!from[x]) {
            if (to[x]) {
                memcpy(retMatrix + z * k, tmpMatrix + y * k, k * sizeof(int));
                z++;
            }
            y++;
        }
    }

    free(tmpMatrix);
    return true;
}

void STACKCode::initLayout() {
    int symbolId = 0;
    _layout.resize(_n, vector<int>(_w, 0));
    for (int nodeId = 0; nodeId < _n; nodeId++) {
        for (int alpha = 0; alpha < _w; alpha++) {
            _layout[alpha][nodeId] = symbolId++;
        }
    }
}

void STACKCode::genDecodingMatrix(vector<int> &availNodes, vector<int> &failedNodes) {

}

void STACKCode::repairSingle(vector<int> &availNodes, int failedNode) {
    int groupId = failedNode % _numGroups;
    int repairBW = getRepairBandwidth(failedNode);
    vector<int> helperNodeIds = getHelperNodes(failedNode);

    // TODO: resume here

    memset(from, 0, sizeof(int) * (n * alpha));
    memset(to, 0, sizeof(int) * (n * alpha));
    for (int j = 0; j < alpha; j++)
    {
        to[failedNode * alpha + j] = 1;
    }
    for (int i = 0; i < n; i++)
    {
        if (helperNodeIds[i])
        {
            if (i % g == b)
            {
                for (int j = 0; j < alpha; j++)
                {
                    from[i * alpha + j] = 1;
                }
            }
            else if (b < alpha)
            {
                from[i * alpha + b] = 1;
            }
            else
            {
                from[i * alpha + (i % g)] = 1;
            }
        }
    }

    int rows = r;
    // int cols = r + beta;
    int cols1 = r;
    int cols2 = beta;

    int before = 0;
    for (int j = 0; j < n; j++)
    {
        if (j >= failedNode)
            break;
        if (!helperNodeIds[j])
        {
            before += (((j % g) == b) ? (alpha) : (1));
        }
    }

    int *R1 = new int[rows * cols1];
    int *R2 = new int[rows * cols2];

    /**
     * get R1 and R2
     */
    int start_col1 = 0;
    int start_col2 = 0;
    for (int a = 0; a < n; a++)
    {
        if (helperNodeIds[a])
        {
            if (a % g == b)
            {
                for (int j = 0; j < alpha; j++)
                {
                    for (int t = 0; t < r; t++)
                    {
                        R2[t * cols2 + start_col2 + j] = primitive_element_power[((a * alpha + j) * t) % order];
                    }
                }
                start_col2 += alpha;
            }
            else if (b == alpha)
            {
                for (int t = 0; t < r; t++)
                {
                    R2[t * cols2 + start_col2] = primitive_element_power[((a * alpha + a % g) * t) % order];
                }
                start_col2 += 1;
            }
            else
            {
                for (int t = 0; t < r; t++)
                {
                    R2[t * cols2 + start_col2] = primitive_element_power[((a * alpha + b) * t) % order];
                }
                start_col2 += 1;
            }
        }
        else
        {
            if (a % g == b)
            {
                for (int j = 0; j < alpha; j++)
                {
                    for (int t = 0; t < r; t++)
                    {
                        R1[t * cols1 + start_col1 + j] = primitive_element_power[((a * alpha + j) * t) % order];
                    }
                }
                start_col1 += alpha;
            }
            else if (b == alpha)
            {
                for (int t = 0; t < r; t++)
                {
                    R1[t * cols1 + start_col1] = primitive_element_power[((a * alpha + a % g) * t) % order];
                }
                start_col1 += 1;
            }
            else
            {
                for (int t = 0; t < r; t++)
                {
                    R1[t * cols1 + start_col1] = primitive_element_power[((a * alpha + b) * t) % order];
                }
                start_col1 += 1;
            }
        }
    }

    // print_mat(R1, rows, cols1);
    // print_mat(R2, rows, cols2);

    int *R1_inverse = new int[rows * cols1];
    jerasure_invert_matrix(R1, R1_inverse, rows, w);
    int *temp = jerasure_matrix_multiply(R1_inverse, R2, rows, cols1, rows, cols2, w);
    // print_mat(temp, rows, cols2);
    delete[] R1;
    delete[] R2;
    delete[] R1_inverse;
    if (cols1 == alpha)
    {
        return temp;
    }
    else
    {
        int *R = (int *)malloc(sizeof(int) * alpha * cols2);
        for (int i = 0; i < alpha; i++)
        {
            memcpy(R + i * cols2, temp + (i + before) * cols2, sizeof(int) * cols2);
        }
        free(temp);
        return R;
    }
}

void STACKCode::repairMultiple(vector<int> &availNodes, vector<int> &failedNodes) {
    // TBD
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

int STACKCode::getAvailPrimElements(int n, int k, int fw) {
    // invalid cases
    if (k <= 0 || n - k < 2) {
        return 0;
    }
    else if (n * fw > 255) {
        return 0;
    }
    else if (fw > n - k) {
        return 0;
    }
    else if (n >= 100) {
        return 0;
    }

    /**
     * For the case of the number of parity nodes is 2,
     * all primitive elements of GF(2^fw) can be used to
     * generate the required n * fw elements.
     */
    if (n - k == 2) {
        return 2;
    }

    int f = 0;

    int flag = n * 10000 + k * 100 + fw;

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