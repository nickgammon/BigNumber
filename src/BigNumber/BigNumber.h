//
//  BigNumber.h
//
//  Author:  Nick Gammon
//  Date:    22nd January 2013.
//  Contributors: Paul Stoffregen, S. Downey
//  Version: 3.5
//  Released into the public domain.
//  Added print function as suggested by Paul Stoffregen.

#ifndef _BigNumber_h
#define _BigNumber_h

#define AUTO_SCALING

#include <stddef.h>
#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) || defined(Arduino_h)
#include <Arduino.h>
#endif

#ifdef STD_INCLUDED
#include <iostream>
#include <string.h>
#include <sstream>
#endif

extern "C"
{
 #include "number.h"
}

class BigNumber
#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) || defined(Arduino_h)
 : public Printable
#endif
{

  // the current scaling amount - shared amongst all BigNumbers
  static int scale_;

  // member variable (the big number)
  bc_num        num_;

#ifdef AUTO_SCALING
  void _setScale(int newScale);
#endif
public:

  // constructors
  BigNumber ();  // default constructor
  BigNumber (const char * s);   // constructor from string
  BigNumber (const int n);  // constructor from int
  // copy constructor
  BigNumber (const BigNumber & rhs);

  // destructor
  ~BigNumber ();

  // static methods: initialize package, and set global scaling factor
  static void begin (const int scale = 0);
  static void finish ();  // free memory used by 'begin' method
  static int setScale (const int scale = 0);

  // for outputting purposes ...
  char * toString () const;  // returns number as string, MUST FREE IT after use!
  operator long () const;
#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) || defined(Arduino_h)
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
#ifdef STD_INCLUDED
  BigNumber(std::string s);
#endif
  long toLong();
#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) || defined(Arduino_h)
  BigNumber(String s);
#endif

};  // end class declaration

#ifdef STD_INCLUDED
std::ostream& operator<<(std::ostream &out, BigNumber num);
std::istream& operator>>(std::istream &in, BigNumber &num);
#endif

#endif
