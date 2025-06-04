#include <cassert>
#include "bigint.h"

BigInt::BigInt()
  // TODO: initialize member variables
  :   magnitude{0}, sign(false)
{
}

BigInt::BigInt(uint64_t val, bool negative)
  // TODO: initialize member variables
  :   magnitude{val}, sign(negative)
{
}

BigInt::BigInt(std::initializer_list<uint64_t> vals, bool negative)
  // TODO: initialize member variables
  : magnitude{vals}, sign(negative)
{
}

BigInt::BigInt(const BigInt &other)
  // TODO: initialize member variables
  : magnitude(other.magnitude), sign(other.sign)
{
}

BigInt::~BigInt()
{
}

BigInt &BigInt::operator=(const BigInt &rhs)
{
  // TODO: implement
  if (this != &rhs) {
    magnitude = rhs.magnitude;
    sign = rhs.sign;
  }
  return *this;
}

bool BigInt::is_negative() const
{
  // TODO: implement
  return sign;
}

const std::vector<uint64_t> &BigInt::get_bit_vector() const {
  // TODO: implement
  return magnitude;
}

uint64_t BigInt::get_bits(unsigned index) const
{
  // TODO: implement
  if (index < magnitude.size()) {
    return magnitude[index];
  }
  return 0;
}

BigInt BigInt::operator+(const BigInt &rhs) const
{
  // TODO: implement
}

BigInt BigInt::operator-(const BigInt &rhs) const
{
  // TODO: implement
  // Hint: a - b could be computed as a + -b
}

BigInt BigInt::operator-() const
{
  // TODO: implement
  BigInt num = *this;

  if(num.magnitude.empty() || (num.magnitude.size() == 1 && num.magnitude[0] == 0)) {
    return num;
  }
  num.sign = !sign;
  return num;
}

bool BigInt::is_bit_set(unsigned n) const
{
  // TODO: implement
}

BigInt BigInt::operator<<(unsigned n) const
{
  // TODO: implement
}

BigInt BigInt::operator*(const BigInt &rhs) const
{
  // TODO: implement
}

BigInt BigInt::operator/(const BigInt &rhs) const
{
  // TODO: implement
}

int BigInt::compare(const BigInt &rhs) const
{
  // TODO: implement
}


std::string BigInt::to_hex() const
{
  // TODO: implement
  if (magnitude.empty()) {
    return "0";
  }
  std::stringstream hex;
  std::string hexcodes = "0123456789abcdef";

  for (auto i = magnitude.rbegin(); i != magnitude.rend(); ++i){
    uint64_t val64 = *i;
    for(int chunk4 = 60; chunk4 >= 0; chunk4 -= 4){
      uint8_t bit_base16 = (val64 >> chunk4) & 0b1111;       //0xf doesn't work?
      hex << hexcodes[bit_base16];
    }  
  }

  std::string hex_str = hex.str();
  int zero_counter = 0;
  while (zero_counter < hex_str.length()-1 && hex_str[zero_counter] == '0') {
    ++zero_counter;
  }
  std::string result = hex_str.substr(zero_counter);


  if(sign){
    return "-" + result;
  }
  return result;


}

std::string BigInt::to_dec() const
{
  // TODO: implement
}

