#ifndef GAME_EXCEPTION_H
#define GAME_EXCEPTION_H

// The game's C++ exception hierarchy (MSL): std::exception is the polymorphic root; logic_error and its
// length_error/out_of_range children derive from it. Only the root is defined here: the derived classes keep
// their constructors and message storage in the units that throw them.
namespace std {
class exception {
public:
    virtual ~exception();
    virtual const char* what() const;
};
class logic_error;
class length_error;
}

#endif
