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

int rec(int d) {
  int a[10];
  if (d > 0) return rec(d-1);
  else return d;
}

int main() {
  rec(313156);  
  //  for (int d=313150; d<1'000'000; d += 1) {
  //    std::cout << d << std::endl;
///    rec(313155);
  //  }
	
  for (int n=1; n<30; ++n) {
    maxd=0;
    calls=0;
    
    uint64_t ans =fib(n);
    cout << "fib(" << n << ")=" << ans << " in " << calls << " calls and max depth " << maxd << endl;
  }
  return 0;
}
