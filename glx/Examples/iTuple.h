#ifndef _ITUPLE_H_
#define _ITUPLE_H_

#include <iostream>
#include <vector>

class iTuple {
 public:

  typedef void (*Func)(iTuple&, void*);

  iTuple();
  iTuple(std::vector<int>&);
  iTuple(const iTuple&);
  ~iTuple(void);
  iTuple& operator=(const iTuple&);

  operator size_t() const; // let iTuples index a std::vector<>, iTuple2size_t
  bool operator==(const iTuple&); 
  int& operator[](int);
  int const& operator[](int) const;

  static int card(const iTuple&, const iTuple&); // # pts [lo..hi] inclusive
  friend bool operator<(const iTuple&,const iTuple&);
  friend bool operator>(const iTuple&,const iTuple&);
  friend iTuple operator-(const iTuple&, const iTuple&);
  friend std::ostream& operator<<(std::ostream&, const iTuple&);
  friend iTuple index2tuple(size_t);

  static iTuple end(void);

  std::vector<int> coord;
  static std::vector<int> idims; // for operator size_t()

 private:
  explicit iTuple(int){}
  
};


#endif
