#ifndef HASH_EQUIVALENCE_HPP_
#define HASH_EQUIVALENCE_HPP_

#include <sstream>
#include "ics_exceptions.hpp"
#include "hash_map.hpp"
#include "array_set.hpp"



namespace ics {

template<class T>
class HashEquivalence {
  public:
    //Fundamental methods
    HashEquivalence(int (*ahash)(const T& element));
    void add_singleton    (const T& a);
    bool in_same_class    (const T& a, const T& b);
    void merge_classes_of (const T& a, const T& b);

    //Other methods
    int size        () const;
    int class_count () const;
    ics::ArraySet<ics::ArraySet<T>> classes ();

    //Useful for debugging (bassed on the implementation)
    int max_height  () const;
    ics::HashMap<T,int> heights () const;
    void show_equivalence () const;
  private:
    int (*hash)(const T& element);
    ics::HashMap<T,T>   parent;
    ics::HashMap<T,int> root_size;
    T compress_to_root (T a);
    //To collect statistics
    int max = 0;
    ics::HashMap<int,int> compress_size;
};



template<class T>
HashEquivalence<T>::HashEquivalence (int (*ahash)(const T& element)) : hash(ahash), parent(ahash), root_size(ahash), compress_size(ahash) {
}


template<class T>
void HashEquivalence<T>::add_singleton (const T& a) {
  if (parent.has_key(a)) {
    std::ostringstream exc;
    exc << "HashEquivalence.add_singleton: a(" << a << ") already in an equivalence class";
    throw EquivalenceError(exc.str());
  }
  parent[a] = a;    //its own parent
  root_size[a] = 1; //its equivalence class has 1 value in it
}

//Use compress_to_root in in_same_class and merge_classes_of
//When finished, a and all its ancestors should refer
//  (in the parent map) directly to the root of a's equivalence class
template<class T>
T HashEquivalence<T>::compress_to_root (T a) {
  if (!parent.has_key(a)) {
    std::ostringstream exc;
    exc << "HashEquivalence.compress_to_root: a(" << a << ") not in an equivalence class";
    throw EquivalenceError(exc.str());
  }
  ics::ArraySet<T> compress_set;
  T to_root;
  while ( (to_root=parent[a]) != a) {
    compress_set.insert(a);
    a = to_root;
  }

  for (auto x : compress_set)
    parent[x] = to_root;

  //To collect statistics
  int set_size = compress_set.size();
  compress_size[set_size] += 1;
  if (set_size > max)
    max = set_size;

  return to_root;
}


//Two values are in the same class if their equivalence trees have the
//  same roots
//In the process of finding the roots, compress all the values on the
//  path to the root: make the parents of a and all its ancestors directly
//  refer to the root of a's equivlance class (same for b).
template<class T>
bool HashEquivalence<T>::in_same_class (const T& a, const T& b) {
  if (!parent.has_key(a)) {
    std::ostringstream exc;
    exc << "HashEquivalence.in_same_class: a(" << a << ") not in an equivalence class";
    throw EquivalenceError(exc.str());
  }
  if (!parent.has_key(b)) {
    std::ostringstream exc;
    exc << "HashEquivalence.in_same_class: b(" << b << ") not in an equivalence class";
    throw EquivalenceError(exc.str());
  }

  return compress_to_root(a) == compress_to_root(b);
}


//Compress a and b to their roots.
//If they are in different equivalence classes, make the parent of the
//  root of the smaller equivalence class refer to the root of the larger
//  equivalence class; update the size of the root of the larger equivalence
//  class and remove the root of the smaller equivalance class from the root_size.
//Throw an exception immediately, if a or b are not in any equivalence
//  classes (were never added as singletons).
template<class T>
void HashEquivalence<T>::merge_classes_of (const T& a, const T& b) {
  if (!parent.has_key(a)) {
    std::ostringstream exc;
    exc << "HashEquivalence.merge_classes_of: a(" << a << ") not in an equivalence class";
    throw EquivalenceError(exc.str());
  }
  if (!parent.has_key(b)) {
    std::ostringstream exc;
    exc << "HashEquivalence.merge_classes_of: b(" << b << ") not in an equivalence class";
    throw EquivalenceError(exc.str());
  }

  T a_root = compress_to_root(a);
  T b_root = compress_to_root(b);
  if (a_root == b_root)
    return;   //Already in same equivalence class! Don't execute code below

  if (root_size[a_root] < root_size[b_root]) {
    parent[a_root] = b_root;
    root_size[b_root] = root_size[a_root]+root_size[b_root];
    root_size.erase(a_root);
  }else{
    parent[b_root] = a_root;
    root_size[a_root] = root_size[a_root]+root_size[b_root];
    root_size.erase(b_root);
  }
}


template<class T>
int HashEquivalence<T>::size () const{
  return parent.size();
}

template<class T>
int HashEquivalence<T>::class_count () const{
  return root_size.size();
}

template<class T>
int HashEquivalence<T>::max_height () const{
  int mh = 0;
  for (auto h : heights())
    if (h.second > mh)
      mh = h.second;
  return mh;
}



template<class T>
ics::ArraySet<ics::ArraySet<T>> HashEquivalence<T>::classes () {
  ics::HashMap<T,ics::ArraySet<T>> answer_map;
  for (auto np : parent) {
    T root = compress_to_root(np.first);
    answer_map[root].insert(np.first);
  }

  ics::ArraySet<ics::ArraySet<T>> answer;
  for (auto rs : answer_map)
    answer.insert(rs.second);

  return answer;
}


template<class T>
ics::HashMap<T,int> HashEquivalence<T>::heights () const {
  ics::HashMap<T,int> answer(hash);
  for (auto np : parent) {
    T e = np.first;
    int depth = 0;
    while (parent[e] != e) {
      e = parent[e];
      depth++;
    }
    int so_far = answer[e];
    if (so_far < depth)
      answer[e] = depth;
  }
  return answer;
}


template<class T>
void HashEquivalence<T>::show_equivalence () const {
  //To compute/print collected statistics
  std::cout << "max=" << max << std::endl;
  int compressed = 0, times = 0;
  for (int i=0; i<=max; ++i) {
    std::cout << "times set size was " << i << " = " << compress_size[i] << std::endl;
    compressed += i*compress_size[i];
    times      += compress_size[i];
  }
  std::cout << "sum of set sizes/times called = " << compressed << "/" << times << "=" << (double)compressed/times << std::endl;

  //  std::cout << "  parent map:    " << parent       << std::endl;
//  std::cout << "  root_size map: " << root_size    << std::endl;
//  std::cout << "  heights map:   " << heights()    << std::endl;
//  std::cout << "  max height:    " << max_height() << std::endl;
}



}

#endif /* HASH_EQUIVALENCE_HPP_ */
