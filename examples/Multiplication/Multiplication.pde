// BigNumber test: multiplication
#include "BigNumber.h"

void setup ()
{
  Serial.begin (115200);
  Serial.println ();
  Serial.println ();
  BigNumber::begin ();  // initialize library

  // test multiplication  
  BigNumber a = "564328376";
  BigNumber b = "18254546";
  BigNumber c = a * b;
  
  Serial.println (c);

}  // end of setup

void loop () { }