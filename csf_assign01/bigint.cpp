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
  // two different sign
  if (sign != rhs.sign){
    if(sign){
      return rhs - (-*this); // -a+b == b-a
    } else{
      return *this - (-rhs); // a+(-b) == a-b
    }
  }

  //same sign
  BigInt result;
  result.sign = sign; // result have the same sign

  const size_t max_size = std::max(magnitude.size(), rhs.magnitude.size());
  uint64_t carry = 0;

  for (size_t i = 0; i < max_size; ++i){
    uint64_t a = (i<magnitude.size()) ? magnitude[i] : 0;
    uint64_t b = (i<rhs.magnitude.size()) ? magnitude[i] : 0;

    uint64_t sum = a + b + carry;

    if (sum < a || sum < b){
      carry = 1;
    }else if (carry ==1 && sum == a + b){
      carry = 1;
    }else{
      carry = 0;
    }

    result.magnitude.push_back(sum);
  }

  if(carry > 0){
    result.magnitude.push_back(1);
  }

  return result;
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
  size_t word_index = n / 64;
  size_t bit_offset = n % 64;

  if (word_index >= magnitude.size()){
    return false;
  }

  uint64_t word = magnitude[word_index];
  return (word >> bit_offset) & 1;
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

