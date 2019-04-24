//
//  BigNumber.cpp
//  
//  Author:  Nick Gammon
//  Date:    22nd January 2013.
//  Contributors: Paul Stoffregen, S. Downey
//  Version: 3.5
//  Released into the public domain.
//  Added print function as suggested by Paul Stoffregen.
//  Changed printTo to return the length, as required.

#include "BigNumber.h"

int BigNumber::scale_ = 0;

// constructor
BigNumber::BigNumber () : num_ (NULL)
{
  bc_init_num (&num_);  // default to zero
} // end of constructor from string

// constructor
BigNumber::BigNumber (const char * s) : num_ (NULL)
{
  bc_str2num(&num_, s, scale_);
} // end of constructor from string

BigNumber::BigNumber (const int n) : num_ (NULL)  // constructor from int
{
  bc_int2num (&num_, n);
} // end of constructor from int

//flok99
BigNumber::BigNumber(const double n) : num_ (NULL) {  //constructor from double
   bc_double2num(&num_, n);
} //end of constructor from double

// copy constructor
BigNumber::BigNumber (const BigNumber & rhs)
  {
  if (this != &rhs)
    num_ = bc_copy_num (rhs.num_);
  }  // end of BigNumber::BigNumber
  
//operator=
BigNumber & BigNumber::operator= (const BigNumber & rhs)
{
  // gracefully handle self-assignment (eg. a = a;)
  if (this == &rhs )
    return *this;
  
  bc_free_num (&num_);  // get rid of old one
  num_ = bc_copy_num (rhs.num_);
  return *this;
} // end of BigNumber::BigNumber & operator=

// destructor - free memory used, if any
BigNumber::~BigNumber ()
{
  bc_free_num (&num_);
} // end of destructor

// set scale factor (number of places after the decimal point)
int BigNumber::setScale (const int scale)
{
  int old_scale = scale_;
  if (scale >= 0)
    scale_ = scale;
  else
    scale_ = 0;
  return old_scale;
}  // end of BigNumber::setScale

// initialize package
// supply scale (number of decimal places): default zero
void BigNumber::begin (const int scale)
{
  bc_init_numbers ();
  scale_ = scale;
} // end of BigNumber::begin

// finished with package
// free special numbers: zero, one, two
void BigNumber::finish ()
{
  bc_free_numbers ();
} // end of BigNumber::finish

// return a pointer to a string containing the number
// MUST FREE THIS after use!
// eg:  char * s = mynumber.toString ();
//      Serial.println (s);
//      free (s);
char * BigNumber::toString () const
{
  return bc_num2str(num_);
} // end of BigNumber::toString 

BigNumber::operator long () const
{
  return bc_num2long (num_);
} // end of BigNumber::operator long

BigNumber::operator double () const
{
  return bc_num2double (num_);
} // end of BigNumber::operator double

// Allow Arduino's Serial.print() to print BigNumber objects!
size_t BigNumber::printTo(Print& p) const
{
  char *buf = bc_num2str(num_);
  size_t len = p.write(buf);
  free(buf);
  return len;
}

// add
BigNumber & BigNumber::operator+= (const BigNumber & n)
{ 
  bc_num result = NULL;
  bc_add (num_, n.num_, &result, scale_);
  bc_free_num (&num_);
  num_ = result;
  return *this; 
} // end of BigNumber::operator+= 

// subtract
BigNumber & BigNumber::operator-= (const BigNumber & n)
{ 
  bc_num result = NULL;
  bc_sub (num_, n.num_, &result, scale_);
  bc_free_num (&num_);
  num_ = result;
  return *this; 
}  // end of BigNumber::operator-=

// divide
BigNumber & BigNumber::operator/= (const BigNumber & n)
{ 
  bc_num result = NULL;
  bc_init_num (&result);  // in case zero
  bc_divide (num_, n.num_, &result, scale_);
  bc_free_num (&num_);
  num_ = result;
  return *this; 
} // end of BigNumber::operator/= 

// multiply
BigNumber & BigNumber::operator*= (const BigNumber & n)
{ 
  bc_num result = NULL;
  bc_multiply (num_, n.num_, &result, scale_);
  bc_free_num (&num_);
  num_ = result;
  return *this; 
}  // end of BigNumber::operator*=

// modulo
BigNumber & BigNumber::operator%= (const BigNumber & n)
{ 
  bc_num result = NULL;
  bc_init_num (&result);  // in case zero
  bc_modulo (num_, n.num_, &result, scale_);
  bc_free_num (&num_);
  num_ = result;
  return *this; 
}  // end of BigNumber::operator%=


// ----------------------------- COMPARISONS ------------------------------

// compare less with another BigNumber
bool BigNumber::operator< (const BigNumber & rhs) const
{
  return bc_compare (num_, rhs.num_) < 0;
} // end of BigNumber::operator<


// compare greater with another BigNumber
bool BigNumber::operator> (const BigNumber & rhs) const
{
  return bc_compare (num_, rhs.num_) > 0;
} // end of BigNumber::operator>

// compare less-or-equal with another BigNumber
bool BigNumber::operator<= (const BigNumber & rhs) const
{
  return bc_compare (num_, rhs.num_) <= 0;
} // end of BigNumber::operator<=

// compare greater-or-equal with another BigNumber
bool BigNumber::operator>= (const BigNumber & rhs) const
{
  return bc_compare (num_, rhs.num_) >= 0;
} // end of BigNumber::operator>=

// compare not equal with another BigNumber
bool BigNumber::operator!= (const BigNumber & rhs) const
{
  return bc_compare (num_, rhs.num_) != 0;
} // end of BigNumber::operator!=

// compare equal with another BigNumber
bool BigNumber::operator== (const BigNumber & rhs) const
{
  return bc_compare (num_, rhs.num_) == 0;
} // end of BigNumber::operator==

// special comparisons
bool BigNumber::isNegative () const
{
  return bc_is_neg (num_) == true;
} // end of BigNumber::isNegative

bool BigNumber::isZero () const
{
  return bc_is_zero (num_) == true;
} // end of BigNumber::isZero

bool BigNumber::isNearZero () const
{
  return bc_is_near_zero (num_, scale_) == true;
} // end of BigNumber::isNearZero

// ----------------------------- OTHER OPERATIONS ------------------------------

// square root
BigNumber BigNumber::sqrt () const
{
  BigNumber result (*this);
  bc_sqrt (&result.num_, scale_);
  return result;
} // end of BigNumber::sqrt

// raise to power
BigNumber BigNumber::pow (const BigNumber power) const
{
  BigNumber result;
  bc_raise (num_, power.num_, &result.num_, scale_);
  return result;
} // end of BigNumber::pow

void BigNumber::divMod (const BigNumber divisor, BigNumber & quotient, BigNumber & remainder) const
{
  bc_divmod (num_, divisor.num_, &quotient.num_, &remainder.num_, scale_);
}

// raise number by power, modulus modulus
BigNumber BigNumber::powMod (const BigNumber power, const BigNumber & modulus) const
{
  BigNumber result;
  bc_raisemod (num_, power.num_, modulus.num_, &result.num_, scale_);
  return result;
}

BigNumber BigNumber::log() const
{
  BigNumber result;
  double numerus_d = bc_num2double(num_);
  double logres = log(numerus_d);
  bc_double2num(&result.num_, logres);
  return result;
}

BigNumber BigNumber::log(const BigNumber base) const
{
  BigNumber result;
  double numerus_d = bc_num2double(num_);
  double base_d = bc_num2double(base.num_);
  double log_n = log(numerus_d);
  double log_b = log(base_d);
  BigNumber a;
  BigNumber b;
  bc_double2num(&a.num_, log_n);
  bc_double2num(&b.num_, log_b);
  bc_divide(a.num_, b.num_, &result.num_, scale_);
  return result;
}


double BigNumber::log_d() const
{
  double numerus_d = bc_num2double(num_);
  return log(numerus_d);
}

double BigNumber::log_d(const BigNumber base) const
{
  BigNumber result;
  double numerus_d = bc_num2double(num_);
  double base_d = bc_num2double(base.num_);
  double log_n = log(numerus_d);
  double log_b = log(base_d);
  BigNumber a;
  BigNumber b;
  bc_double2num(&a.num_, log_n);
  bc_double2num(&b.num_, log_b);
  bc_divide(a.num_, b.num_, &result.num_, scale_);
  double res = bc_num2double(result.num_);
  return res;
}
