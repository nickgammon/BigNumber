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

#ifdef STD_INCLUDED
BigNumber::BigNumber(std::string s) {
  *this = s.c_str();
}

std::ostream& operator<<(std::ostream &out, BigNumber num) {
  out << num.toString();
  return out;
}

std::istream& operator>>(std::istream &in, BigNumber &num) {
  std::string a;
  in >> a;
  num = a.c_str();
  return in;
}
#endif

// constructor
BigNumber::BigNumber () : num_ (NULL)
{
  bc_init_num (&num_);  // default to zero
} // end of constructor from string

// constructor
BigNumber::BigNumber (const char * s) : num_ (NULL)
{
  begin();
  bool isZero = 1;
  int64_t i = 0;
  while (s[i] != 0) {
    if (s[i] != '0' || s[i] == '.') {
      isZero = 0;
      break;
    }
    i++;
  }
  if (isZero) {
    *this = 0;
    return;
  }
#ifdef AUTO_SCALING
  uint64_t size = 0;
  uint64_t dotPoint = 0;
  bool setScale = false;
  for(; s[size] != 0; size++){
    if(s[size] == '.'){
      dotPoint = size;
      setScale = true;
    }
  }
  while (s[size - 1] == '0') {
    size--;
  }
  if(setScale)_setScale(size - dotPoint - 1);
#endif
  bc_str2num(&num_, s, scale_);
} // end of constructor from string

BigNumber::BigNumber (const int n) : num_ (NULL)  // constructor from int
{
  bc_int2num (&num_, n);
} // end of constructor from int

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
  _setScale(scale);
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
  char* a = bc_num2str(num_);
  int64_t size = 0;
  bool removeZerosAfterPoint = false;
  while(a[size] != 0) {
    if(a[size] == '.') removeZerosAfterPoint = true;
    size++;
  }
  if (removeZerosAfterPoint) {
    while (a[size - 1] == '0') {
      size--;
    }
    if (a[size - 1] == '.') size--;
    a[size] = 0;
  }
  return a;
} // end of BigNumber::toString

BigNumber::operator long () const
{
  return bc_num2long (num_);
} // end of BigNumber::operator long

// Allow Arduino's Serial.print() to print BigNumber objects!
#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) || defined(Arduino_h)
size_t BigNumber::printTo(Print& p) const
{
  char *buf = toString();
  size_t len = p.write(buf);
  free(buf);
  return len;
}
#endif

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

long BigNumber::toLong() {
  return bc_num2long (num_);
}

double BigNumber::toDouble() {
  char* a = toString();
  int8_t c = 0;
  int8_t pointLocation = 0;
  long res = 0;
  double ret = 0;
  while (a[c] != 0) {
    if (a[c] != '.') {
      res = res * 10 + a[c] - 48;
    } else {
      pointLocation = c;
    }
    c++;
  }
  if (pointLocation) {
    for (int8_t i = 1; i < (c - pointLocation); i++) {
      float tmp = 1;
      for (int8_t j = (c - pointLocation); j > i; j--) {
        tmp *= 10;
      }
      ret += (res % 10) / tmp;
      res /= 10;
    }
  }
  return ret + res;
}

#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) || defined(Arduino_h)
BigNumber::BigNumber(String s) {
  *this = s.c_str();
}
#endif

BigNumber::BigNumber(double d) : num_(NULL) {
  begin();
  if (d == ((int)d)) {
    *this = (int)d;
    return;
  }
  char r[20];
	int number;
  char tmp2 = 0, tmp = 0, counter = 0;
  bool isNegative;
	double number2;
	isNegative = 0;
	if (d < 0) {
		isNegative = 1;
		d = -d;
	}
	number2 = d;
	number = d;
	while ((number2 - (double)number) != 0 && !((number2 - (double)number) < 0)) {
    int count = tmp2 + 1;
    int result = 1;
    while(count-- > 0)
      result *= 10;
      number2 = d * result;
      number = number2;
      tmp2++;
	}
  for (tmp = (d > 1) ? 0 : 1; d > 1; tmp++)
    d /= 10;
	counter = tmp;
	tmp = tmp + 1 + tmp2;
	number = number2;
	if (isNegative) {
    tmp++;
    counter++;
	}
	for (int i = tmp; i >= 0 ; i--) {
    if (i == tmp) {
      r[i] = '\0';
    } else if(i == counter) {
      r[i] = '.';
    } else if(isNegative && i == 0) {
      r[i] = '-';
    } else {
      r[i] = (number % 10) + '0';
      number /=10;
		}
	}
#ifdef AUTO_SCALING
  uint64_t size = 0;
  uint64_t dotPoint = 0;
  bool setScale = false;
  for(; r[size] != 0; size++){
    if(r[size] == '.'){
      dotPoint = size;
      setScale = true;
    }
  }
  if(setScale)_setScale(size - dotPoint - 1);
#endif
  bc_str2num(&num_, r, scale_);
}

BigNumber::BigNumber(long data) : num_ (NULL) {
  char res[25] = {'0', '0', '0', '0', '0', '0',
												'0', '0', '0', '0', '0', '0',
												'0', '0', '0', '0', '0', '0',
												'0', '0', '0', '0', '0', '0'
												, '0'};
			if (data < 0) { res[0] = '-'; data = -data; }
			uint8_t c = 0;
			while(data != 0) {
				res[23 - c++] = (data % 10) + '0';
				data /= 10;
			}
			res[24] = 0;
      *this = res;
}

BigNumber::BigNumber(long long data) : num_ (NULL) {
  char res[25] = {'0', '0', '0', '0', '0', '0',
												'0', '0', '0', '0', '0', '0',
												'0', '0', '0', '0', '0', '0',
												'0', '0', '0', '0', '0', '0'
												, '0'};
			if (data < 0) { res[0] = '-'; data = -data; }
			uint8_t c = 0;
			while(data != 0) {
				res[23 - c++] = (data % 10) + '0';
				data /= 10;
			}
			res[24] = 0;
      *this = res;
}

#ifdef AUTO_SCALING
void BigNumber::_setScale(int newScale) {
  if (newScale > scale_){
    setScale(newScale);
  }
}
#endif
