#include <algorithm>
#include <bitset>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <ctype.h>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <shared_mutex>
#include <mutex>
#include <numeric>
#include <random>
#include <sstream>
#include <thread>
#include <vector>

using namespace std;
random_device rand_dev;
mt19937 gen(rand_dev());
int max_c_master;
shared_mutex X;

vector<int> C_master;
void randPermutation(vector<int> &O, int n) {
  if (O.size() != n)
    O.resize(n);
  iota(O.begin(), O.end(), 0);
  shuffle(O.begin(), O.end(), gen);
}
int greedyColoring(const vector<vector<int>> &A, vector<int> &O, vector<int> &C,
                   int n) {
  int max_c = (n > 0);
  fill(C.begin(), C.end(), 0);

  vector<bool> N_v(max_c_master + 1, false);
  for (int i = 0; (i < n) && (max_c <= max_c_master); i++) {
    int v = O[i];
    int j = 0;
    fill(N_v.begin(), N_v.begin() + max_c + 1, false);
    for (int x : A[v]) {
      if (C[x] > j) {
        N_v[C[x] - 1] = true;
        while (N_v[j])
          j++;
      }
    }
    max_c = max(max_c, C[v] = j + 1);
  }
  return max_c;
}

int SS(const vector<vector<int>> &A, int n) {
  vector<int> O(n), C(n);
  randPermutation(O, n);
  int max_c = greedyColoring(A, O, C, n);
  vector<int> Ot;

  time_t start = time(NULL);

  while (true) {
    randPermutation(Ot, max_c);
    transform(C.begin(), C.end(), C.begin(),
              [&](int val) { return Ot[val - 1]; });
    iota(O.begin(), O.end(), 0);
    sort(O.begin(), O.end(), [&C](int i, int j) { return C[i] < C[j]; });

    max_c = greedyColoring(A, O, C, n);

    X.lock();
    if (max_c < max_c_master) {
            max_c_master = max_c;
            C_master = C;
        cout << max_c << "\t " << (time(NULL) - start) << endl;
    } else if (max_c > max_c_master) {
        C = C_master;
    }

    X.unlock();

  }
    return max_c_master;
  }
  bool readfile(string fileName, vector<vector<int>> & A, int &n, int &e) {
    ifstream file(fileName);
    if (!(file.is_open()))
      return false;
    string st[2], line;
    while (getline(file, line)) {
      if (line[0] == 'p') {
        stringstream s(line);
        s >> st[0] >> st[1] >> n >> e;
        A.resize(n);
        for (int i = 0; i < n; i++)
          A[i].resize(0);
      } else if (line[0] == 'e') {
        stringstream s(line);
        int x, y;
        s >> st[0] >> x >> y;
        A[x - 1].push_back(y - 1);
        A[y - 1].push_back(x - 1);
      }
    }
    file.close();
    for(int i = 0; i < n;i++)
        A[i].shrink_to_fit();

    return true;
  }
  int main(int argc, char *argv[]) {
    if (argc < 2)
      return 1;

    int n, e, N = thread::hardware_concurrency();
    vector<vector<int>> A;
    if (readfile(argv[1], A, n, e) == 0) {
      return 1;
    }
    max_c_master = n;

    vector<thread> t;
    for (int i = 0; i < N; i++) {
      t.push_back(thread(SS, ref(A), n));
    }
    for (int i = 0; i < N; i++) {
      t[i].join();
    }

    return 0;
  }
