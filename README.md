BigNumber
=========

BigNumber library for Arduino devices.

Use 256-bit numbers on tiny platforms.


ported from https://github.com/nickgammon/BigNumber

## Requirements

- Any Arduino Device: (recommended model) [Adafruit Feather HUZZAH with ESP8266 WiFi](https://www.adafruit.com/product/2821)
- Arduino IDE: https://www.arduino.cc/en/Main/Software

## Usage

### Initialize the BigNumber library
```cpp
  BigNumber.begin();
```

### create your BigNumber

```cpp
  BigNumber bigNum;
  bigNum = "67411411944031530524562029520200239450929984281572279077458355238873731477537";
```

```cpp
  BigNumber bigNum = "67411411944031530524562029520200239450929984281572279077458355238873731477537";
```

### modify your BigNumber


```cpp
  BigNumber bigNum = "67411411944031530524562029520200239450929984281572279077458355238873731477537";
  BigNumber base = 16;
  BigNumber multiplier = base.pow(8);
  bigNum *= multiplier;
```

### Serial.print your BigNumber

```cpp
  BigNumber bigNum = "67411411944031530524562029520200239450929984281572279077458355238873731477537";
  Serial.println(bigNum);
```
> 67411411944031530524562029520200239450929984281572279077458355238873731477537

#

# More documentation and examples:

http://www.gammon.com.au/forum/?id=11519


