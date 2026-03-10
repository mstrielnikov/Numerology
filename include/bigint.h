#ifndef BIGINT_H
#define BIGINT_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint64_t *words;
    size_t len;
    size_t cap;
} BigUInt;

BigUInt *biguint_new(void);
BigUInt *biguint_from_uint64(uint64_t n);
BigUInt *biguint_from_slice(const uint64_t *words, size_t len);
void biguint_free(BigUInt *a);

BigUInt *biguint_add(const BigUInt *a, const BigUInt *b);
BigUInt *biguint_sub(const BigUInt *a, const BigUInt *b);
BigUInt *biguint_mul(const BigUInt *a, const BigUInt *b);
BigUInt *biguint_div(const BigUInt *a, const BigUInt *b);
BigUInt *biguint_mod(const BigUInt *a, const BigUInt *b);
BigUInt *biguint_and(const BigUInt *a, const BigUInt *b);
BigUInt *biguint_or(const BigUInt *a, const BigUInt *b);
BigUInt *biguint_xor(const BigUInt *a, const BigUInt *b);
BigUInt *biguint_not(const BigUInt *a);
BigUInt *biguint_shl(const BigUInt *a, size_t bits);
BigUInt *biguint_shr(const BigUInt *a, size_t bits);

int biguint_cmp(const BigUInt *a, const BigUInt *b);
size_t biguint_bit_length(const BigUInt *a);
bool biguint_is_zero(const BigUInt *a);
BigUInt *biguint_clone(const BigUInt *a);
void biguint_normalize(BigUInt *a);

BigUInt *biguint_gcd(BigUInt *a, BigUInt *b);
BigUInt *biguint_extended_gcd(const BigUInt *a, const BigUInt *b, BigUInt *x, BigUInt *y);

size_t next_power_of_two(size_t n);
bool is_power_of_two(size_t n);
size_t bit_length(size_t n);
size_t reverse_bits(size_t x, size_t len);

#endif
