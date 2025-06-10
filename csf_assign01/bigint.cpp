#include <cassert>
#include "bigint.h"

BigInt::BigInt()
  // TODO: initialize member variables
  :   magnitude(), sign(false)
{
}

BigInt::BigInt(uint64_t val, bool negative)
  // TODO: initialize member variables
  :   magnitude{val}, sign(negative)
{
}

BigInt::BigInt(std::initializer_list<uint64_t> vals, bool negative)
  // TODO: initialize member variables
  : magnitude(vals), sign(negative)
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
  // two different sign, convert addition into subtraction
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
    uint64_t b = (i<rhs.magnitude.size()) ? rhs.magnitude[i] : 0;

    uint64_t sum = a + b;
    uint64_t sum_with_carry = sum + carry;

    //check the overflow, if yes, need to determine new carry
    if (sum < a || sum < b || sum_with_carry < sum){
      carry = 1;
    }else{
      carry = 0;
    }

    result.magnitude.push_back(sum_with_carry);
  }

  //add carry if there's left
  if(carry > 0){
    result.magnitude.push_back(1);
  }


  return result;
}

BigInt BigInt::operator-(const BigInt &rhs) const
{
  // TODO: implement
  // Hint: a - b could be computed as a + -b

  if(is_zero()) return -rhs;

  if(rhs.is_zero()) return *this;

  //different sign, can convert as addition
  if (sign != rhs.sign) {
    return *this + (-rhs); // a - (-b) or (-a) - b
  }
  
  //same sign
  BigInt result;
  int compare_two = compare_magnitudes(*this, rhs);

  //determine which is larger
  const std::vector<uint64_t> *larger;
  const std::vector<uint64_t> *smaller;
  bool result_sign;

  if (compare_two >= 0){ //normal subtraction
    larger = &magnitude;
    smaller = &rhs.magnitude;
    result_sign = sign;
  } else{               //ned to flip sign
    larger = &rhs.magnitude;
    smaller = &magnitude;
    result_sign = !sign;
  }


  uint64_t borrow = 0;
  for (size_t i = 0; i < larger->size(); ++i){
    uint64_t a = (*larger)[i];
    uint64_t b = (i<smaller->size()) ? (*smaller)[i] : 0;

    uint64_t sub;
    if (a >= b + borrow) {
      sub = a - b - borrow;
      borrow = 0;
    } else{
      sub = (1ULL << 64) + a - b - borrow;
      borrow = 1;
    }
    result.magnitude.push_back(sub);
  }

  //remove leading zeros
  while (result.magnitude.size() > 1 && result.magnitude.back() == 0) {
    result.magnitude.pop_back();
  }

  //zero's sign need to be false, positivezero 
  result.sign = result.is_zero() ? false : result_sign;
  return result;
}

BigInt BigInt::operator-() const
{
  // TODO: implement
  if(is_zero()){
    return *this;
  }
  BigInt result = *this;
  result.sign = !sign;

  // // weather 0 or not
  // bool allZero = true;
  // for (uint64_t word : result.magnitude) {
  //   if (word != 0) {
  //     allZero = false;
  //     break;
  //   }
  // }

  // result.sign = allZero ? false : !sign;

  // //move 0UL from end to start
  // if (!result.magnitude.empty() && result.magnitude.back() == 0) {
  //   result.magnitude.pop_back();                  
  //   result.magnitude.insert(result.magnitude.begin(), 0); 
  // }

  return result;
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

// WL
BigInt BigInt::operator<<(unsigned n) const
{
  if (is_negative()) {
    throw std::invalid_argument("Left shift is not allowed on negative values");
  }
  // TODO: implement

  unsigned int words_shifted = n / 64;
  unsigned int bits_shifted = n % 64;
  uint64_t buffer = 0;
  BigInt result;
  result.magnitude = std::vector<uint64_t>(magnitude.size() + words_shifted + 1, 0);
  result.sign = false;

  //  shift bits
  for (size_t i = 0; i < magnitude.size(); ++i) {
    uint64_t word = magnitude[i];
    result.magnitude[words_shifted + i] |= (word << bits_shifted) | buffer;
    buffer = (bits_shifted == 0) ? 0 : (word >> (64 - bits_shifted));
  }

  if (buffer != 0) {
    result.magnitude[words_shifted + magnitude.size()] = buffer;
  }

  //  delete empty uint_64
  while (result.magnitude.size() > 1 && result.magnitude.back() == 0) {
    result.magnitude.pop_back();
  }

  return result;
}

// Helper to copy the first n bits of word
uint64_t BigInt::copy_bits(uint64_t word, unsigned n){
  if (n >= 64) return word;
  return word & ((1ULL << n) - 1);
}

BigInt BigInt::operator*(const BigInt &rhs) const
{
  // TODO: implement
  BigInt result (0, false);
  if (is_zero() || rhs.is_zero()){
    return result;
  }

  result.magnitude.resize(magnitude.size() +rhs.magnitude.size(), 0);

  for (size_t i=0; i< magnitude.size(); ++i){
    uint64_t a = magnitude[i];
    uint64_t carry = 0;

    for (size_t j = 0; j < rhs.magnitude.size(); ++j){
      __uint128_t product = (__uint128_t)a * rhs.magnitude[j];
      product += result.magnitude[i + j];
      product += carry;

      result.magnitude[i + j] = (uint64_t)(product & 0xFFFFFFFFFFFFFFFFULL);
      carry = (uint64_t)(product >> 64);
    }

    result.magnitude[i + rhs.magnitude.size() ] += carry;
  }

  while (result.magnitude.size() > 1 && result.magnitude.back() == 0) {
    result.magnitude.pop_back();
  }

  result.sign = (sign != rhs.sign);   //XOR
  return result;
}

// WL
BigInt BigInt::operator/(const BigInt &rhs) const
{
  // TODO: implement
  if(rhs.is_zero()){
    throw std::invalid_argument("Divide by 0");
  } 

  BigInt top = *this;
  BigInt bottom = rhs;
  top.sign = false;
  bottom.sign = false;

  BigInt min(0, false);
  BigInt max = top;
  BigInt mid;
  BigInt product;

  bool first = true;

  while (compare_magnitudes(min, max) <= 0) {
    mid = (min + max).div_by_2();
    product = mid * bottom;
    
    int cmp = compare_magnitudes(product, top);

    if(first){
      std::cout << "min: " << min.to_hex() << std::endl; 
      std::cout << "mid: " << mid.to_hex() << std::endl; 
      std::cout << "max: " << max.to_hex() << std::endl; 
      std::cout << "cmp: " << cmp << std::endl; 
      first = false;
    }

    if (cmp == 0) {
      mid.sign = (sign != rhs.sign);  //XOR
      // std::cout << mid.to_hex() << std::endl;
      return mid;
    } else if (cmp < 0) {     //top bigger than product
      min = mid + BigInt(1, false);
    } else {
      max = mid - BigInt(1, false);
    }
  }

  std::cout << max.to_hex() << std::endl;
  max.sign = (sign != rhs.sign);
  return max;
}


BigInt BigInt::div_by_2() const {
  BigInt result;
  result.sign = sign;
  result.magnitude.resize(magnitude.size(), 0);

  uint64_t carry = 0;
  for (size_t i = magnitude.size(); i-- > 0;) {
    uint64_t current = magnitude[i];
    result.magnitude[i] = (current >> 1) | (carry << 63);
    carry = current & 1;
  }

  // remove leading zeroes
  while (result.magnitude.size() > 1 && result.magnitude.back() == 0) {
    result.magnitude.pop_back();
  }

  return result;
}


// WL
int BigInt::compare(const BigInt &rhs) const
{
  // TODO: implement

  if (is_zero() && rhs.is_zero()){
    return 0;
  }

  // compare signs
  if(is_negative()){
    if (rhs.is_negative())
    {
      return compare_magnitudes(*this, rhs);
    } else return -1;
  } else {
    if(!rhs.is_negative()){
      return compare_magnitudes(*this, rhs);
    } else return 1;
  } 
}

// int BigInt::compare_magnitudes(const BigInt &lhs, const BigInt &rhs){

//   const auto &left = lhs.magnitude;
//   const auto &right = rhs.magnitude;

//   size_t left_size = kill_leading_zeros(left);
//   size_t right_size = kill_leading_zeros(right);

//   // Diff length
//   if (left_size > right_size) return 1;
//   if (left_size < right_size) return -1;

//   // Same length
//   for (size_t i = left_size; i-- > 0;) {
//     return left[i] - right[i];
//   }

//   return 0;
// }

int BigInt::compare_magnitudes(const BigInt &lhs, const BigInt &rhs){

  const auto &left = lhs.magnitude;
  const auto &right = rhs.magnitude;

  size_t left_size = kill_leading_zeros(left);
  size_t right_size = kill_leading_zeros(right);

  // Diff length
  if (left_size > right_size) return 1;
  if (left_size < right_size) return -1;

  for (auto i = left_size-1; i > 0; --i){
    uint64_t lword = left[i];
    uint64_t rword = right[i];
    if(lword > rword) return 1;
    if(rword < lword) return -1;
  }

  return 0;
}


// Returns length of vector without leading zeroes
size_t BigInt::kill_leading_zeros(const std::vector<uint64_t> &vec) {
  size_t i = vec.size();
  while (i > 0 && vec[i - 1] == 0) {
    --i;
  }
  return i;
}

bool BigInt::is_zero() const{
  for (uint64_t word : magnitude) {
    if (word > 0) return false;
  }
  return true;
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

//  WL
std::string BigInt::to_dec() const
{
  // TODO: implement
  if (is_zero()) return 0;

  std::stringstream digits;

  BigInt num = *this;
  num.sign = false;

  BigInt divided = num;
  BigInt remainder;

  while(!divided.is_zero()){
    divided = divided/10;
    remainder = num - BigInt(10, false) * divided;
    digits << std::to_string(remainder.magnitude[0]);
  }

  std::string output = digits.str();
  std::reverse(output.begin(), output.end());

  if(sign){
    return "-" + output;
  }
  return output;

}