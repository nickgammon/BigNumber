// BigNumber example: Calculate Pi using the Francois Viete formula

#include <BigNumber.h>

BigNumber PiViete ()
{
  // Francois Viete formula
  BigNumber two (2);       // used as a constant value of 2
  BigNumber s (0);         // used in iteration
  BigNumber t (1);         // used in iteration
 
  for (int i = 0; i < 115; i++) // number of iterations: 10 to 115
  {
    BigNumber r = s + two; // temporary value
    s = r.sqrt ();
    t *= s / two;
  }
 
  return two / t;          // calculate pi
} // end of function pi

void setup()
{
  Serial.begin (115200);
  Serial.println ();

  // Arduino Uno
  // -----------
  // The Arduino Uno has only 2k bytes of SRAM.
  // The maximum length of BigNumber is 74 for an Arduino Uno.
  // About 115 iterations in the Francois Viete formula is enough for an Arduino Uno.
  // That will take 34 seconds.
  //
  // Arduino Zero or Arduino MKR
  // ---------------------------
  // For an Arduino Zero, it is possible to set the length of BigNumber to a higher value.
  // For example, the length of BigNumber to 1000, and 1500 iterations.
  // That will take 8 hours.
  //
  BigNumber::begin (74);    // Length of the BigNumber. 50 is fast, 74 is slow.

  Serial.println (F("Please wait patiently (34 seconds with Arduino Uno)"));
  Serial.println (F("pi (as text)       = 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899862803482534211706"));

  unsigned long start = millis();

  BigNumber pi = PiViete ();

  unsigned long finish = millis();

  Serial.print (F("pi (Viete formula) = "));
  Serial.println (pi);
  Serial.print (F("It took "));
  Serial.print ((finish - start) / 1000UL);
  Serial.print (F(" seconds."));
  
} // end of setup

void loop () { }

