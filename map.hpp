#ifndef MAP_HPP_
#define MAP_HPP_

#include <iostream>
#include "iterator.hpp"
#include "pair.hpp"


namespace ics {

template<class KEY,class T> class Map  {
  public:
  typedef ics::pair<KEY,T> Entry;
    virtual ~Map();

    virtual bool empty      () const = 0;
    virtual int  size       () const = 0;
    virtual bool has_key    (const KEY& key) const = 0;
    virtual bool has_value  (const T& value) const = 0;
    virtual std::string str () const = 0;

    virtual T    put   (const KEY& key, const T& value) = 0;
    virtual T    erase (const KEY& key) = 0;
    virtual void clear () = 0;

    virtual int put (ics::Iterator<Entry>& start, const ics::Iterator<Entry>& stop) = 0;

    virtual T&       operator [] (const KEY&) = 0;
    virtual const T& operator [] (const KEY&) const = 0;
    virtual bool operator == (const Map<KEY,T>& rhs) const = 0;
    virtual bool operator != (const Map<KEY,T>& rhs) const = 0;

    template<class KEY2,class T2>
    friend std::ostream& operator << (std::ostream& outs, const Map<KEY2,T2>& m);

    virtual Iterator<Entry>& ibegin () const = 0;
    virtual Iterator<Entry>& iend   () const = 0;

    //KLUDGE: could define
    //virtual Iterator<KEY>&  begin_key   () const = 0;
    //virtual Iterator<KEY>&  end_key     () const = 0;
    //virtual Iterator<T>&    begin_value () const = 0;
    //virtual Iterator<T>&    end_value   () const = 0;
  };





template<class KEY,class T>
Map<KEY,T>::~Map(){}

template<class KEY2,class T2>
std::ostream& operator << (std::ostream& outs, const Map<KEY2,T2>& m) {
  outs << "map[";

  if (!m.empty()) {
    ics::Iterator<T2>& i = m.ibegin();
    outs << i->first << "->" << i->second;
    ++i;
    for (; i != m.iend(); ++i)
      outs << "," << i->first << "->" << i->second;
  }

  outs <<"]";
  return outs;
}


}

#endif /* MAP_HPP_ */
