#include <stdint.h>
#include <iostream>

using namespace std;

int maxd=0;
int calls=0;

uint64_t fib(int n, int d=1) {
  ++calls;
  if (n <= 1) {
    if (d > maxd) maxd = d;
    return n;
  }
  return fib(n-1,d+1)+fib(n-2,d+1);
}

int main(int argc, const char *argv[]) {
  maxd=0;
  calls=0;
  
  int n = atoi(argv[1]);
  int a = fib(n);

  cout << "fib(" << n << ")=" << a << " in " << calls << " calls and max depth " << maxd << endl;

  return 0;
}
