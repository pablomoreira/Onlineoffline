#include "ds18b20.h"

Ds18b20::Ds18b20(uint8_t pin)
{
  _pin = pin;
  //OneWire _ds(_pin);
  _ds.begin(pin);
  _num = 0;
  _index = 0;
  this->isSetTempLimit = false;
  //pinMode(pin, OUTPUT);
}
void Ds18b20::search(){
  uint8 i = 0, j = 0;
    if(_ds.search(_addr)){
      _num++;
      this->__addr2str();
    }
    else{
        _ds.reset_search();
        //_num = 0;

        while (i < 8){
          j += sprintf(this->_addr2str + j,"%02hx",0);
          i++;
        }
    }
}

uint8_t Ds18b20::getNum(){
  return _num;
}

char* Ds18b20::addr2str(){
  return this->_addr2str;
}

void Ds18b20::__addr2str(){
  uint8 i = 0, j = 0;

  while (i < 8){
    j += sprintf(this->_addr2str + j,"%02hx",this->_addr[i]);
    i++;
  }
}

bool Ds18b20::crc8(){
  if ( OneWire::crc8( this->_addr, 7) == this->_addr[7]) {
    this->_ds.reset();
    this->_ds.select(this->_addr);
    this->_ds.write(0x44,0);
    return true;
  }
  return false;
}

bool Ds18b20::update(){
  uint8 i;

  if (millis() - this->_mark_time > DELAY_DS){
    this->_mark_time = millis();
    //present =
    _ds.reset();
    _ds.select(_addr);
    _ds.write(0xBE);         // Read Scratchpad


    for ( i = 0; i < 9; i++) {           // we need 9 bytes
      _data[i] = _ds.read();
      //Serial.print(data[i], HEX);
      //Serial.print(" ");
    }
    //Serial.print(" CRC=");
    //Serial.print(
    OneWire::crc8( _data, 8);
    int16_t raw = (_data[1] << 8) | _data[0];
    byte cfg = (_data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time

    this->temp = (float)raw / 16.0;
    return true;
  }
  return false;
}


float Ds18b20::getTemp(){
  return this->temp;
}

uint32_t Ds18b20::getMark(){
 return  this->_mark_time;
}

bool Ds18b20::setTempLimit(String s){
  float temp;
  char buf[7];
  if (s.length() < 6){
     temp = s.toFloat();
     if(temp < 40 && temp > 10){
        Serial.printf("%f\n",temp);
        this->tempLimit = temp;
        LittleFS.begin();
        File f = LittleFS.open(PATH_TEMP, "w");
        s.toCharArray(buf,7);
        f.printf("%s\n",buf);
        f.close();
        LittleFS.end();
        this->isSetTempLimit = true;
        return true;
    }
  }
  return false;
}

float Ds18b20::getTempLimit(){
  String temp;
  if(this->isSetTempLimit){
    return this->tempLimit;
  }
  else{
    LittleFS.begin();
    if(LittleFS.exists(PATH_TEMP)){
      File f = LittleFS.open(PATH_TEMP, "r");
      temp = f.readString();
      LittleFS.end();
      this->isSetTempLimit = true;
      this->tempLimit = temp.toFloat();
      f.close();
      LittleFS.end();
    }
    else{
        LittleFS.begin();
        File f = LittleFS.open(PATH_TEMP, "w");
        f.printf("%s\n","23" );
        f.close();
        LittleFS.end();
        this->tempLimit = temp.toFloat();
    }
  }

  return this->tempLimit;

}
