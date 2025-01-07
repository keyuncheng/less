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
}

ECDAG* STACKCode::Encode() {

}

ECDAG* STACKCode::Decode(vector<int> from, vector<int> to) {

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
    int numFailedNodes = 0;
    int numHelperNodes = 0;
    for (int i = 0; i < n; i++)
    {
        if (to[i])
        {
            numFailedNodes++;
        }
        if (from[i])
        {
            numHelperNodes++;
        }
        if (to[i] == 1 && from[i] == 1)
        {
            cout << "STACKCode::convertPCMatrix2GenMatrix() numFailedNodes node " << i << " cannot be used for decoding" << endl;
            return false;
        }
    }
    if (numFailedNodes > m)
    {
        cout << "STACKCode::convertPCMatrix2GenMatrix() Too many nodes for decoding: " << numFailedNodes << endl;
        return false;
    }
    if (numHelperNodes != k)
    {
        cout << "STACKCode::convertPCMatrix2GenMatrix() The number of numHelperNodes nodes should be k" << std::endl;
        return false;
    }
    // TODO

    const int *H = pcmat;
    int *H1 = new int[r * r]; // numFailedNodes
    int *H2 = new int[r * k]; // numHelperNodes
    int *H1_inverse = new int[r * r];

    for (int j = 0, j1 = 0, j2 = 0; j < n; j++)
    {
        if (!from[j]){
            for(int i = 0; i < r; i++){
                H1[i * r + j1] = H[i * n + j];
            }
            j1++;
        }else{
            for(int i = 0; i < r; i++){
                H2[i * k + j2] = H[i * n + j];
            }
            j2++;
        }
    }

    jerasure_invert_matrix(H1, H1_inverse, r, w);
    int *bigmat = jerasure_matrix_multiply(H1_inverse, H2, r, r, r, k, w);
    delete[] H1;
    delete[] H2;
    delete[] H1_inverse;
    if (numFailedNodes == r)
    {
        return bigmat;
    }
    int *result = (int*)malloc(sizeof(int)*(numFailedNodes  * k));

    for(int x = 0, y = 0, z = 0; x < n; x++){
        if(!from[x]){
            if(to[x]){
                memcpy(result + z * k, bigmat + y * k, sizeof(int)*k);
                z++;
            }
            y++;
        }
    }

    free(bigmat);
    return result;
}

void STACKCode::genDecodingMatrix(vector<int> &availNodes, vector<int> &failedNodes) {

}

void STACKCode::repairSingle(vector<int> &availNodes, int failedNode) {

}

void STACKCode::repairMultiple(vector<int> &availNodes, vector<int> &failedNodes) {

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