//
//  BigNumber.h
//  
//
//  Author:  Nick Gammon
//  Date:    22nd January 2013.
//  Version: 3.0
//  Released into the public domain.
//  Added print function as suggested by Paul Stoffregen.

#ifndef _BigNumber_h
#define _BigNumber_h

#include <stddef.h>
#ifndef NOT_ARDUINO
#include <Arduino.h>
#endif

#include "number.h" 

class BigNumber
#ifndef NOT_ARDUINO
: public Printable
#endif
{
  // the current scaling amount - shared amongst all BigNumbers
  static uint16_t scale_;

  // member variable (the big number)
  bc_num        num_;

public:
 
  // constructors
  BigNumber ();  // default constructor
  BigNumber (const char *const s);   // constructor from string
  BigNumber (const int32_t n);  // constructor from int
  BigNumber (const uint32_t n);  // constructor from unsigned int
  BigNumber (const int64_t n);  // constructor from long long int
  BigNumber (const uint64_t n);  // constructor from long long unsigned int
  BigNumber (const double n);  // constructor from double
  // copy constructor
  BigNumber (const BigNumber & rhs); 
 
  // destructor  
  ~BigNumber ();
  
  // static methods: initialize package, and set global scaling factor
  static void begin (const uint16_t scale = 0);
  static void finish ();  // free memory used by 'begin' method
  static int setScale (const uint16_t scale = 0);
  
  // for outputting purposes ...
  char * toString () const;  // returns number as string, MUST FREE IT after use!
  operator long () const;
#ifndef NOT_ARDUINO
  virtual size_t printTo(Print& p) const; // for Arduino Serial.print()
#endif

  // operators ... assignment
  BigNumber & operator= (const BigNumber & rhs);
  
  // operations on the number which change it (eg. a += 5; )
  BigNumber & operator+= (const BigNumber & n);
  BigNumber & operator-= (const BigNumber & n);
  BigNumber & operator/= (const BigNumber & n);
  BigNumber & operator*= (const BigNumber & n);
  BigNumber & operator%= (const BigNumber & n);  // modulo

  // operations on the number which do not change it (eg. a = b + 5; )
  BigNumber operator+ (const BigNumber & n) const { BigNumber temp = *this; temp += n; return temp; };
  BigNumber operator- (const BigNumber & n) const { BigNumber temp = *this; temp -= n; return temp; };
  BigNumber operator/ (const BigNumber & n) const { BigNumber temp = *this; temp /= n; return temp; };
  BigNumber operator* (const BigNumber & n) const { BigNumber temp = *this; temp *= n; return temp; };
  BigNumber operator% (const BigNumber & n) const { BigNumber temp = *this; temp %= n; return temp; };
  
  // prefix operations
  BigNumber & operator++ () { *this += 1; return *this; }
  BigNumber & operator-- () { *this -= 1; return *this; }
  
  // postfix operations (cannot return by reference)
  // we make a temporary object, change our current object, return the temporary one
  // if we returned a reference it would cease to exist, so we have to return a copy
  BigNumber operator++ (int) { BigNumber temp = *this; *this += 1; return temp; }
  BigNumber operator-- (int) { BigNumber temp = *this; *this -= 1; return temp; }
  
  // comparisons
  bool operator<  (const BigNumber & rhs) const;
  bool operator<  (const int rhs) const { return *this < BigNumber (rhs); }
  bool operator>  (const BigNumber & rhs) const;
  bool operator>  (const int rhs) const { return *this > BigNumber (rhs); }
  bool operator<= (const BigNumber & rhs) const;
  bool operator<= (const int rhs) const { return *this <= BigNumber (rhs); }
  bool operator>= (const BigNumber & rhs) const;
  bool operator>= (const int rhs) const { return *this >= BigNumber (rhs); }
  bool operator!= (const BigNumber & rhs) const;
  bool operator!= (const int rhs) const { return *this != BigNumber (rhs); }
  bool operator== (const BigNumber & rhs) const;
  bool operator== (const int rhs) const { return *this == BigNumber (rhs); }

  // quick sign test
  bool isNegative () const;
  // quick zero test
  bool isZero () const;
  
  // In some places we need to check if the number is almost zero.
  // Specifically, all but the last digit is 0 and the last digit is 1.
  // Last digit is defined by scale.
  bool isNearZero () const;
  
  // other mathematical operations
  BigNumber sqrt () const;
  BigNumber pow (const BigNumber power) const;
  // divide number by divisor, give quotient and remainder
  void divMod (const BigNumber divisor, BigNumber & quotient, BigNumber & remainder) const;
  // raise number by power, modulus modulus
  BigNumber powMod (const BigNumber power, const BigNumber & modulus) const;
};  // end class declaration

#endif
