#include "STACKCode.hh"

STACKCode::STACKCode(int n, int k, int w, int opt, vector<string> param) {
    _n = n;
    _k = k;
    _w = w;
    _opt = opt;
    _m = n - k;
    _num_groups = _w + 1;

    if (_w != 8) {
        cout << "STACKCode::STACKCode() Currently only supports w=8" << endl;
        exit(1);
    }
    
    _e = getAvailPrimElements(_n, _k, _w);

    if (_e == 0)
    {
        cout << "STACKCode::STACKCode() failed to find available primitive elements in GF(2^" << w << ")" << endl;
        exit(-1);
    }

    _order = (1 << w) - 1;
    getPrimElementsPower(_order, _e, _w);
}

ECDAG* STACKCode::Encode() {

}

ECDAG* STACKCode::Decode(vector<int> from, vector<int> to) {

}

void STACKCode::Place(vector<vector<int>>& group) {

}

void STACKCode::genParityCheckMatrix() {

}

void STACKCode::genEncodingMatrix() {

}

void STACKCode::convertPCMatrix2EncMatrix() {

}

void STACKCode::genDecodingMatrix(vector<int> &availNodes, vector<int> &failedNodes) {

}

void STACKCode::repairSingle(vector<int> &availNodes, int failedNode) {

}

void STACKCode::repairMultiple(vector<int> &availNodes, vector<int> &failedNodes) {

}

void STACKCode::getPrimElementsPower(int order, int e, int w)
{
    _primElementPower.resize(order);
    _primElementPower[0] = 1;
    for (int i = 1; i < order; i++)
    {
        _primElementPower[i] = galois_single_multiply(_primElementPower[i - 1], e, w);
    }
}

int STACKCode::getAvailPrimElements(int n, int k, int w) {
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
     * all primitive elements of GF(256) can be used to
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

    return findRoot(f, w);
}

int STACKCode::findRoot(int f, int w)
{
    for (int root = 1;; root++)
    {
        if (root == 0)
            return 0;
        if (polynomialAssignment(root, f, w) != 0)
            continue;
        return root;
    }
}

int STACKCode::polynomialAssignment(int x, int f, int w)
{
    int fx = 1;
    for (int i = w - 1; i >= 0; i--)
    {
        fx = galois_single_multiply(fx, x, w);
        // fx *= x;
        // fx = gf_mult(fx, x, w);
        fx ^= ((f >> i) & (1));
    }
    return fx;
}