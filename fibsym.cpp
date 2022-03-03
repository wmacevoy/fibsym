#include <iostream>
#include <memory>
#include <map>
#include <vector>
#include <sstream>
#include <ncurses.h>

using namespace std;

// generic to print vectors like [a,b,c]..
template <typename T>
ostream& operator<<(ostream &out, const vector<T> &v) {
  out << "[";
  bool comma = false;
  for (auto x : v) {
    if (comma) out << ",";
    out << x;
    comma = true;
  }
  out << "]";
  return out;
}

// base "any" var type with str() method

struct BaseVar {
  virtual std::string str() const = 0;
  virtual ~BaseVar() {}
};

// specialized by template to any type

template <typename T> struct Var : BaseVar {
  T value;
  Var(const T& _value) : value(_value) {}
  virtual std::string str() const {
    std::ostringstream oss;
    oss << value;
    return oss.str();
  }
};

// frame is a name->var map
// frame.map adds a new value to the frame
// frame.val returns a reference to a mapped value

struct Frame : std::map < std::string , std::shared_ptr < BaseVar > > {
  template <typename T> void map(const std::string &name, const T &value) {
    insert(std::pair < std::string, std::shared_ptr < BaseVar > >(name, std::shared_ptr<BaseVar>(new Var<T>(value))));
  }
  template <typename T> Var<T> &var(const std::string &name) {
    auto loc = find(name);
    if (loc != end()) {
      return (dynamic_cast < Var<T> & > (*loc->second));
    }
    throw std::out_of_range("name is not in frame");
  }
  template <typename T> T& val(const std::string &name) {
    return (var<T>(name)).value;
  }
  template <typename T> string str(const std::string &name) {
    return (var<T>(name)).str();
  }

  void print(ostream &out) {
    out << "{";
    bool comma = false;
    for (auto kv : *this) {
      if (comma) out << ",";
      out << kv.first << ":" << kv.second->str();
      comma=true;
    }
    out << "}";
  }
};

struct Stack : vector < shared_ptr < Frame > > {
  shared_ptr<Frame> top(int back = 0) {
    return at(size()-back-1);
  }
  
  void push(shared_ptr < Frame > frame) {
    push_back(frame);
  }
  
  shared_ptr<Frame> pop() {
    shared_ptr<Frame> ans = top();
    pop_back();
    return ans;
  }

  void print(std::ostream &out) {
    int d = 0;
    for (auto p: *this) {
      out << "frame " << d << ":";
      p->print(out);
      out << endl;
      ++d;
    }
  }
};

// stack simulation for ...
//
// return: exit to os...
//
// int main(vector<string> args) {
//
//   int n = atoi(args[1]);
//   int a = fib(n);
//   cout << "fib(" << n << ")=" << a << endl;
//
//   return 0;
// }
//
// int fib(int n) {
//    int rec1=0;
//    int rec2=0;
//
//    if (n <= 1) {
//      return n;
//    }
//
//    rec1 = fib(n-1);
//    rec2 = fib(n-2);
//
//    return rec1+rec2;
// }
//



struct Screen {
  int step;
  int maxConsole;
  vector<pair<int,string> > console;
  Screen() { step=0; initscr(); maxConsole = 10; }
  ~Screen() { endwin(); }
  void clear() { ::clear(); }
  void display(Stack &stack) {
    ostringstream out;
    int y=maxConsole+1,x=0;
    stack.print(out);
    mvprintw(y,x,"%s",out.str().c_str());
  }
  void display(string &address) {
    int y=0,x=40;    
    mvprintw(y,x,"address: %s",address.c_str());

    y=0,x=0;
    for (int i=0; i<console.size(); ++i) {
      mvprintw(y+i,x,"%04d> %s",console[i].first,console[i].second.c_str());
    }
  }
  void log(const std::string &str) {
    while (console.size() >= maxConsole) {
      console.erase(console.begin());
    }
    console.push_back(pair<int,string>(step,str));
  }
  void refresh() { ::refresh(); ++step; }
};

int main(int argc, const char *argv[]) {
  vector<string> args;
  for (int i=0; i<argc; ++i) args.push_back(argv[i]);
  
  Stack stack;
  string address;
  Screen screen;
  
  stack.push(shared_ptr<Frame>(new Frame()));
  stack.top()->map<vector<string>>("args",args);
  stack.top()->map<int>("return_value",0);
  stack.top()->map<string>("return_address","return");

  address="main";

  for (;;) {
    screen.clear();
    screen.display(stack);
    screen.display(address);
    screen.refresh();
    getch();

    if (address == "return") {
      shared_ptr<Frame> called = stack.pop();
      return called->val<int>("return_value");    
    }
    
    if (address.rfind("main",0) == 0) {
      if (address == "main") {
	screen.log("enter main");
	stack.top()->map<int>("n",0);
	stack.top()->map<int>("a",0);

	vector<string> &args=stack.top()->val<vector<string>>("args");
	int &n=stack.top()->val<int>("n");
	
        n = atoi(args[1].c_str());
        shared_ptr<Frame> call(new Frame());
        call->map<int>("n",n);
        call->map<int>("return_value",0);
        call->map<string>("return_address","main1");
        stack.push(call);
	screen.log("calling fib in main");
        address="fib";
        continue;
      }
      
      if (address == "main1") {
	screen.log("return from fib in main");
        shared_ptr<Frame> called = stack.pop();
	int &n=stack.top()->val<int>("n");
	int &a=stack.top()->val<int>("a");
	int &return_value = stack.top()->val<int>("return_value");
        a = called->val<int>("return_value");
        cout << "fib(" << n << ")=" << a << endl;
	return_value = 0;
	screen.log("return from main");
	address = "return";
	continue;
      }
    }
  
    if (address.rfind("fib",0) == 0) {
      if (address == "fib") {
	screen.log("enter fib");
	stack.top()->map<int>("rec1",0);
	stack.top()->map<int>("rec2",0);

	int &n = stack.top()->val<int>("n");

	if (n <= 1) {
	  int &return_value = stack.top()->val<int>("return_value");
	  string &return_address = stack.top()->val<string>("return_address");
	  
	  screen.log("return n in fib");
	  return_value = n;
	  address = return_address;
	  continue;
	}

	screen.log("call fib(n-1) in fib");
	shared_ptr<Frame> call(new Frame());
	call->map<int>("n",n-1);
	call->map<int>("return_value",0);
	call->map<string>("return_address","fib1");
	stack.push(call);
	address = "fib";
	continue;
      }

      if (address == "fib1") {

	screen.log("return from fib(n-1) in fib");
	shared_ptr<Frame> called = stack.pop();
	int &n = stack.top()->val<int>("n");
	int &rec1 = stack.top()->val<int>("rec1");
	rec1=called->val<int>("return_value");

	screen.log("call fib(n-2) in fib");	
	shared_ptr<Frame> call(new Frame());
	call->map<int>("n",n-2);
	call->map<int>("return_value",0);
	call->map<string>("return_address","fib2");
	stack.push(call);
	address = "fib";
	continue;
      }

      if (address == "fib2") {
	screen.log("returned from fib(n-2) in fib");
	shared_ptr<Frame> called = stack.pop();
	int &rec1 = stack.top()->val<int>("rec1");
	int &rec2 = stack.top()->val<int>("rec2");
	rec2=called->val<int>("return_value");

	int &return_value = stack.top()->val<int>("return_value");
	string &return_address = stack.top()->val<string>("return_address");

	screen.log("return rec1+rec2 in fib");
	return_value = rec1 + rec2;
	address = return_address;
	continue;
      }
    }
  }
}
