#ifndef ICS_EXCEPTIONS_HPP_
#define ICS_EXCEPTIONS_HPP_

#include <string>

namespace ics {

class IcsError {
  public:
    IcsError(const std::string& message);
    virtual ~IcsError();
    virtual const std::string what () const;
  protected:
    const std::string message; //KLUDGE: why not &
};


class EmptyError : public IcsError {
  public:
    EmptyError(const std::string& message);
    virtual ~EmptyError();
    virtual const std::string what () const;
};


class ConcurrentModificationError : public IcsError {
  public:
    ConcurrentModificationError(const std::string& message);
    virtual ~ConcurrentModificationError();
    virtual const std::string what () const;
};


class ComparingDifferentIteratorsError : public IcsError {
  public:
    ComparingDifferentIteratorsError(const std::string& message);
    virtual ~ComparingDifferentIteratorsError();
    virtual const std::string what () const;
};


class IteratorTypeError : public IcsError {
  public:
    IteratorTypeError(const std::string& message);
    virtual ~IteratorTypeError();
    virtual const std::string what () const;
};


class CannotEraseError : public IcsError {
  public:
    CannotEraseError(const std::string& message);
    virtual ~CannotEraseError();
    virtual const std::string what () const;
};


class IteratorPositionIllegal : public IcsError {
  public:
    IteratorPositionIllegal(const std::string& message);
    virtual ~IteratorPositionIllegal();
    virtual const std::string what () const;
};

class KeyError : public IcsError {
  public:
  KeyError(const std::string& message);
    virtual ~KeyError();
    virtual const std::string what () const;
};


class EquivalenceError : public IcsError {
  public:
    EquivalenceError(const std::string& message);
    virtual ~EquivalenceError();
    virtual const std::string what () const;
};

class GraphError : public IcsError {
  public:
    GraphError(const std::string& message);
    virtual ~GraphError();
    virtual const std::string what () const;
};


}

#endif /* ICS_EXCEPTIONS_HPP_ */
