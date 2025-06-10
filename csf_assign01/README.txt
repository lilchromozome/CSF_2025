TODO: list team members, document who did what, discuss
anything interesting about your implementation.

William Li 
Sichen Deng

In the hexcode implementation I used bitwise operators by & 0b1111 to get a chunk that could be converted easily into a base16 value.

William functions and corresponding tests:
constructors
destructor
get_bit_vector() member function
git_bits(unsigned) member function
is_negative() member function
operator-() member function (unary minus)
to_hex() member function (conversion to base-16)
operator<<(unsigned) member function (left shift)
operator/(const BigInt &) member function (division—challenging!)
to_dec() member function (conversion to base-10—challenging!)

Sichen functions and corresponding tests:
is_bit_set(unsigned) member function
operator+(const BigInt &) member function (addition)
operator-(const BigInt &) member function (two-operand subtraction)
operator*(const BigInt &) member function (multiplication—challenging!)
compare(const BigInt &) member function
