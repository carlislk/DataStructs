#include "ics_exceptions.hpp"

namespace ics {

IcsError::IcsError(const std::string& message) : message(message) {};
IcsError::~IcsError(){};
const std::string IcsError::what () const {
  return "IcsError " + message;
};

EmptyError::EmptyError(const std::string& message) : IcsError(message) {};
EmptyError::~EmptyError() {};
const std::string EmptyError::what () const {
  return "EmptyError " + message;
};

ConcurrentModificationError::ConcurrentModificationError(const std::string& message) : IcsError(message) {};
ConcurrentModificationError::~ConcurrentModificationError() {};
const std::string ConcurrentModificationError::what () const {
  return "ConcurrentModificationError " + message;
};

ComparingDifferentIteratorsError::ComparingDifferentIteratorsError(const std::string& message) : IcsError(message) {};
ComparingDifferentIteratorsError::~ComparingDifferentIteratorsError() {};
const std::string ComparingDifferentIteratorsError::what () const {
  return "ComparingDifferentIteratorsError " + message;
};


IteratorTypeError::IteratorTypeError(const std::string& message) : IcsError(message) {};
IteratorTypeError::~IteratorTypeError() {};
const std::string IteratorTypeError::what () const {
  return "IteratorTypeError " + message;
};


CannotEraseError::CannotEraseError(const std::string& message) : IcsError(message) {};
CannotEraseError::~CannotEraseError() {};
const std::string CannotEraseError::what () const {
    return "CannotEraseError " + message;
};


IteratorPositionIllegal::IteratorPositionIllegal(const std::string& message) : IcsError(message) {};
IteratorPositionIllegal::~IteratorPositionIllegal() {};
const std::string IteratorPositionIllegal::what () const {
  return "IteratorPositionIllegal " + message;
};


KeyError::KeyError(const std::string& message) : IcsError(message) {};
KeyError::~KeyError() {};
const std::string KeyError::what () const {
  return "KeyError " + message;
};


EquivalenceError::EquivalenceError(const std::string& message) : IcsError(message) {};
EquivalenceError::~EquivalenceError() {};
const std::string EquivalenceError::what () const {
  return "EquivalenceError " + message;
};


GraphError::GraphError(const std::string& message) : IcsError(message) {};
GraphError::~GraphError() {};
const std::string GraphError::what () const {
  return "GraphError " + message;
};


}
