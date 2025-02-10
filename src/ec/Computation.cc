#include "Computation.hh"

mutex Computation::_cLock;

int Computation::singleMulti(int a, int b, int w) {
  Computation::_cLock.lock();
  int res = galois_single_multiply(a, b, w);
  Computation::_cLock.unlock();
  return res;
}

void Computation::Multi(char** dst, char** src, int* mat, int rowCnt, int colCnt, int len, string lib, int fw) {
  if (lib == "Jerasure") {
    Computation::_cLock.lock();
    jerasure_matrix_encode(colCnt, rowCnt, fw, mat, src, dst, len);
    Computation::_cLock.unlock();
  } else {
    if (fw != 8) {
      cout << "Computation::Multi ISA-L library only supports GF(2^8), required fw: " << fw << endl;
      exit(-1);
    }
    // first transfer the mat into char*
    char* imatrix;
    imatrix = (char*)calloc(rowCnt * colCnt, sizeof(char));
    for (int i=0; i<rowCnt * colCnt; i++) {
      char tmpc = mat[i];
      imatrix[i] = tmpc;
    }
    // call isa-l library
    unsigned char itable[32 * rowCnt * colCnt];
    ec_init_tables(colCnt, rowCnt, (unsigned char*)imatrix, itable);
    ec_encode_data(len, colCnt, rowCnt, itable, (unsigned char**)src, (unsigned char**)dst);
  }
}
