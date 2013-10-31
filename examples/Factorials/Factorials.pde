// BigNumber test: factorials
#include "BigNumber.h"

void setup ()
{
  Serial.begin (115200);
  while (!Serial) ;
  delay(500);
  Serial.println ();
  BigNumber::begin ();  // initialize library
 
  //factorials
  BigNumber fact = 1;

  for (int i = 2; i <= 200; i++)
  {
    Serial.print(i);
    Serial.print("! = ");
    fact *= i;
    Serial.println(fact);
  }

}  // end of setup

void loop () { }