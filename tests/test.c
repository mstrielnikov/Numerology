#include <bigint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define ASSERT(cond, msg) do { \
    if (cond) { \
        tests_passed++; \
    } else { \
        tests_failed++; \
        printf("FAIL: %s\n", msg); \
    } \
} while(0)

#define ASSERT_EQ(a, b, msg) do { \
    int cmp = biguint_cmp(a, b); \
    if (cmp == 0) { \
        tests_passed++; \
    } else { \
        tests_failed++; \
        printf("FAIL: %s (got != expected)\n", msg); \
    } \
} while(0)

void print_biguint(const BigUInt *a) {
    if (biguint_is_zero(a)) {
        printf("0");
        return;
    }
    for (size_t i = a->len; i > 0; i--) {
        printf("%016lx", a->words[i-1]);
    }
}

char *biguint_to_hex(const BigUInt *a) {
    if (biguint_is_zero(a)) {
        return strdup("0");
    }
    char *buf = malloc(a->len * 16 + 1);
    buf[0] = '\0';
    for (size_t i = a->len; i > 0; i--) {
        char tmp[17];
        snprintf(tmp, sizeof(tmp), "%016lx", a->words[i-1]);
        strcat(buf, tmp);
    }
    return buf;
}

void test_add() {
    printf("=== ADD TESTS ===\n");
    
    BigUInt *a = biguint_from_uint64(10);
    BigUInt *b = biguint_from_uint64(20);
    BigUInt *c = biguint_add(a, b);
    ASSERT_EQ(c, biguint_from_uint64(30), "10 + 20");
    biguint_free(c);
    
    a = biguint_from_uint64(0);
    b = biguint_from_uint64(42);
    c = biguint_add(a, b);
    ASSERT_EQ(c, biguint_from_uint64(42), "0 + 42");
    biguint_free(c);
    
    a = biguint_from_uint64(UINT64_MAX);
    b = biguint_from_uint64(1);
    c = biguint_add(a, b);
    ASSERT(c->len == 2 && c->words[0] == 0 && c->words[1] == 1, "UINT64_MAX + 1");
    biguint_free(c);
    
    uint64_t words1[] = {UINT64_MAX, UINT64_MAX};
    uint64_t words2[] = {1, 0};
    a = biguint_from_slice(words1, 2);
    b = biguint_from_slice(words2, 2);
    c = biguint_add(a, b);
    ASSERT(c->len == 3 && c->words[0] == 0 && c->words[1] == 0 && c->words[2] == 1, "max+max + 1 = 2^128");
    biguint_free(c);
    
    a = biguint_from_uint64(0xFFFFFFFF);
    b = biguint_from_uint64(1);
    c = biguint_add(a, b);
    ASSERT_EQ(c, biguint_from_uint64(0x100000000ULL), "0xFFFFFFFF + 1");
    biguint_free(c);
    
    printf("add: %d passed, %d failed\n\n", tests_passed, tests_failed);
}

void test_sub() {
    printf("=== SUB TESTS ===\n");
    
    BigUInt *a = biguint_from_uint64(20);
    BigUInt *b = biguint_from_uint64(10);
    BigUInt *c = biguint_sub(a, b);
    ASSERT_EQ(c, biguint_from_uint64(10), "20 - 10");
    biguint_free(c);
    
    a = biguint_from_uint64(42);
    b = biguint_from_uint64(0);
    c = biguint_sub(a, b);
    ASSERT_EQ(c, biguint_from_uint64(42), "42 - 0");
    biguint_free(c);
    
    a = biguint_from_uint64(100);
    b = biguint_from_uint64(100);
    c = biguint_sub(a, b);
    ASSERT(biguint_is_zero(c), "100 - 100 = 0");
    biguint_free(c);
    
    uint64_t words1[] = {10, 0};
    uint64_t words2[] = {5};
    a = biguint_from_slice(words1, 2);
    b = biguint_from_slice(words2, 1);
    c = biguint_sub(a, b);
    ASSERT(c->len == 1 && c->words[0] == 5, "(10<<64) - 5");
    biguint_free(c);
    
    printf("sub: %d passed, %d failed\n\n", tests_passed, tests_failed);
}

void test_mul() {
    printf("=== MUL TESTS ===\n");
    
    BigUInt *a = biguint_from_uint64(17);
    BigUInt *b = biguint_from_uint64(19);
    BigUInt *c = biguint_mul(a, b);
    ASSERT_EQ(c, biguint_from_uint64(323), "17 * 19");
    biguint_free(c);
    
    a = biguint_from_uint64(0);
    b = biguint_from_uint64(999);
    c = biguint_mul(a, b);
    ASSERT(biguint_is_zero(c), "0 * n = 0");
    biguint_free(c);
    
    a = biguint_from_uint64(1);
    b = biguint_from_uint64(12345);
    c = biguint_mul(a, b);
    ASSERT_EQ(c, biguint_from_uint64(12345), "1 * n");
    biguint_free(c);
    
    a = biguint_from_uint64(123);
    b = biguint_from_uint64(456);
    c = biguint_mul(a, b);
    ASSERT_EQ(c, biguint_from_uint64(56088), "123 * 456");
    biguint_free(c);
    
    a = biguint_from_uint64(1000);
    b = biguint_from_uint64(2000);
    c = biguint_mul(a, b);
    ASSERT_EQ(c, biguint_from_uint64(2000000), "1000 * 2000");
    biguint_free(c);
    
    a = biguint_from_uint64(0xFFFF);
    b = biguint_from_uint64(0xFFFF);
    c = biguint_mul(a, b);
    ASSERT_EQ(c, biguint_from_uint64(0xFFFE0001ULL), "0xFFFF * 0xFFFF");
    biguint_free(c);
    
    printf("mul: %d passed, %d failed\n\n", tests_passed, tests_failed);
}

void test_div() {
    printf("=== DIV TESTS ===\n");
    
    BigUInt *a = biguint_from_uint64(100);
    BigUInt *b = biguint_from_uint64(3);
    BigUInt *c = biguint_div(a, b);
    ASSERT_EQ(c, biguint_from_uint64(33), "100 / 3");
    biguint_free(c);
    
    a = biguint_from_uint64(10);
    b = biguint_from_uint64(2);
    c = biguint_div(a, b);
    ASSERT_EQ(c, biguint_from_uint64(5), "10 / 2");
    biguint_free(c);
    
    a = biguint_from_uint64(1);
    b = biguint_from_uint64(1);
    c = biguint_div(a, b);
    ASSERT_EQ(c, biguint_from_uint64(1), "1 / 1");
    biguint_free(c);
    
    a = biguint_from_uint64(100);
    b = biguint_from_uint64(100);
    c = biguint_div(a, b);
    ASSERT_EQ(c, biguint_from_uint64(1), "n / n");
    biguint_free(c);
    
    a = biguint_from_uint64(7);
    b = biguint_from_uint64(8);
    c = biguint_div(a, b);
    ASSERT(biguint_is_zero(c), "7 / 8 = 0");
    biguint_free(c);
    
    printf("div: %d passed, %d failed\n\n", tests_passed, tests_failed);
}

void test_mod() {
    printf("=== MOD TESTS ===\n");
    
    BigUInt *a = biguint_from_uint64(100);
    BigUInt *b = biguint_from_uint64(3);
    BigUInt *c = biguint_mod(a, b);
    ASSERT_EQ(c, biguint_from_uint64(1), "100 % 3");
    biguint_free(c);
    
    a = biguint_from_uint64(50);
    b = biguint_from_uint64(10);
    c = biguint_mod(a, b);
    ASSERT(biguint_is_zero(c), "50 % 10 = 0");
    biguint_free(c);
    
    a = biguint_from_uint64(17);
    b = biguint_from_uint64(17);
    c = biguint_mod(a, b);
    ASSERT(biguint_is_zero(c), "n % n = 0");
    biguint_free(c);
    
    printf("mod: %d passed, %d failed\n\n", tests_passed, tests_failed);
}

void test_cmp() {
    printf("=== CMP TESTS ===\n");
    
    BigUInt *a = biguint_from_uint64(10);
    BigUInt *b = biguint_from_uint64(20);
    ASSERT(biguint_cmp(a, b) < 0, "10 < 20");
    ASSERT(biguint_cmp(b, a) > 0, "20 > 10");
    ASSERT(biguint_cmp(a, a) == 0, "n == n");
    biguint_free(a);
    biguint_free(b);
    
    a = biguint_from_uint64(0);
    b = biguint_from_uint64(1);
    ASSERT(biguint_cmp(a, b) < 0, "0 < 1");
    biguint_free(a);
    biguint_free(b);
    
    printf("cmp: %d passed, %d failed\n\n", tests_passed, tests_failed);
}

void test_shl() {
    printf("=== SHL TESTS ===\n");
    
    BigUInt *a = biguint_from_uint64(1);
    BigUInt *b = biguint_shl(a, 1);
    ASSERT_EQ(b, biguint_from_uint64(2), "1 << 1");
    biguint_free(b);
    
    a = biguint_from_uint64(1);
    b = biguint_shl(a, 64);
    ASSERT(b->len == 2 && b->words[0] == 0 && b->words[1] == 1, "1 << 64");
    biguint_free(b);
    
    a = biguint_from_uint64(1);
    b = biguint_shl(a, 0);
    ASSERT_EQ(b, biguint_from_uint64(1), "1 << 0");
    biguint_free(b);
    
    a = biguint_from_uint64(3);
    b = biguint_shl(a, 2);
    ASSERT_EQ(b, biguint_from_uint64(12), "3 << 2");
    biguint_free(b);
    
    a = biguint_from_uint64(0xFF);
    b = biguint_shl(a, 8);
    ASSERT_EQ(b, biguint_from_uint64(0xFF00ULL), "0xFF << 8");
    biguint_free(b);
    
    printf("shl: %d passed, %d failed\n\n", tests_passed, tests_failed);
}

void test_shr() {
    printf("=== SHR TESTS ===\n");
    
    BigUInt *a = biguint_from_uint64(8);
    BigUInt *b = biguint_shr(a, 1);
    ASSERT_EQ(b, biguint_from_uint64(4), "8 >> 1");
    biguint_free(b);
    
    a = biguint_from_uint64(1);
    b = biguint_shr(a, 1);
    ASSERT(biguint_is_zero(b), "1 >> 1 = 0");
    biguint_free(b);
    
    a = biguint_from_uint64(100);
    b = biguint_shr(a, 0);
    ASSERT_EQ(b, biguint_from_uint64(100), "n >> 0");
    biguint_free(b);
    
    a = biguint_from_uint64(0xFF00);
    b = biguint_shr(a, 8);
    ASSERT_EQ(b, biguint_from_uint64(0xFF), "0xFF00 >> 8");
    biguint_free(b);
    
    printf("shr: %d passed, %d failed\n\n", tests_passed, tests_failed);
}

void test_bitwise() {
    printf("=== BITWISE TESTS ===\n");
    
    BigUInt *a = biguint_from_uint64(0xF0F0);
    BigUInt *b = biguint_from_uint64(0x0F0F);
    BigUInt *c = biguint_and(a, b);
    ASSERT_EQ(c, biguint_from_uint64(0x0000), "and");
    biguint_free(c);
    
    c = biguint_or(a, b);
    ASSERT_EQ(c, biguint_from_uint64(0xFFFF), "or");
    biguint_free(c);
    
    c = biguint_xor(a, b);
    ASSERT_EQ(c, biguint_from_uint64(0xFFFF), "xor");
    biguint_free(c);
    
    a = biguint_from_uint64(0xAAAAAAAA);
    b = biguint_from_uint64(0x55555555);
    c = biguint_and(a, b);
    ASSERT(biguint_is_zero(c), "A & 5 = 0");
    biguint_free(c);
    
    c = biguint_or(a, b);
    ASSERT_EQ(c, biguint_from_uint64(0xFFFFFFFF), "A | 5 = F");
    biguint_free(c);
    
    c = biguint_xor(a, b);
    ASSERT_EQ(c, biguint_from_uint64(0xFFFFFFFF), "A ^ 5 = F");
    biguint_free(c);
    
    biguint_free(a);
    biguint_free(b);
    
    printf("bitwise: %d passed, %d failed\n\n", tests_passed, tests_failed);
}

void test_not() {
    printf("=== NOT TESTS ===\n");
    
    BigUInt *a = biguint_from_uint64(0);
    BigUInt *b = biguint_not(a);
    ASSERT(b->len == 1 && b->words[0] == UINT64_MAX, "~0 = max");
    biguint_free(b);
    
    a = biguint_from_uint64(1);
    b = biguint_not(a);
    ASSERT(b->len == 2, "~1 has 2 words");
    biguint_free(b);
    
    printf("not: %d passed, %d failed\n\n", tests_passed, tests_failed);
}

void test_bit_length() {
    printf("=== BIT_LENGTH TESTS ===\n");
    
    BigUInt *a = biguint_from_uint64(0);
    ASSERT(biguint_bit_length(a) == 0, "bit_length(0) = 0");
    biguint_free(a);
    
    a = biguint_from_uint64(1);
    ASSERT(biguint_bit_length(a) == 1, "bit_length(1) = 1");
    biguint_free(a);
    
    a = biguint_from_uint64(255);
    ASSERT(biguint_bit_length(a) == 8, "bit_length(255) = 8");
    biguint_free(a);
    
    a = biguint_from_uint64(256);
    ASSERT(biguint_bit_length(a) == 9, "bit_length(256) = 9");
    biguint_free(a);
    
    a = biguint_from_uint64(UINT64_MAX);
    ASSERT(biguint_bit_length(a) == 64, "bit_length(UINT64_MAX) = 64");
    biguint_free(a);
    
    uint64_t words[] = {UINT64_MAX, 1};
    a = biguint_from_slice(words, 2);
    ASSERT(biguint_bit_length(a) == 65, "bit_length(1<<64) = 65");
    biguint_free(a);
    
    printf("bit_length: %d passed, %d failed\n\n", tests_passed, tests_failed);
}

void test_gcd() {
    printf("=== GCD TESTS ===\n");
    
    BigUInt *a = biguint_from_uint64(48);
    BigUInt *b = biguint_from_uint64(18);
    BigUInt *c = biguint_gcd(a, b);
    ASSERT_EQ(c, biguint_from_uint64(6), "gcd(48, 18) = 6");
    biguint_free(c);
    
    a = biguint_from_uint64(17);
    b = biguint_from_uint64(13);
    c = biguint_gcd(a, b);
    ASSERT_EQ(c, biguint_from_uint64(1), "gcd(17, 13) = 1 (coprime)");
    biguint_free(c);
    
    a = biguint_from_uint64(100);
    b = biguint_from_uint64(25);
    c = biguint_gcd(a, b);
    ASSERT_EQ(c, biguint_from_uint64(25), "gcd(100, 25) = 25");
    biguint_free(c);
    
    a = biguint_from_uint64(0);
    b = biguint_from_uint64(42);
    c = biguint_gcd(a, b);
    ASSERT_EQ(c, biguint_from_uint64(42), "gcd(0, 42) = 42");
    biguint_free(c);
    
    printf("gcd: %d passed, %d failed\n\n", tests_passed, tests_failed);
}

void test_utils() {
    printf("=== UTILS TESTS ===\n");
    
    ASSERT(next_power_of_two(1) == 1, "next_pow2(1) = 1");
    ASSERT(next_power_of_two(2) == 2, "next_pow2(2) = 2");
    ASSERT(next_power_of_two(3) == 4, "next_pow2(3) = 4");
    ASSERT(next_power_of_two(1023) == 1024, "next_pow2(1023) = 1024");
    ASSERT(next_power_of_two(1024) == 1024, "next_pow2(1024) = 1024");
    ASSERT(next_power_of_two(1025) == 2048, "next_pow2(1025) = 2048");
    
    ASSERT(is_power_of_two(1), "is_pow2(1)");
    ASSERT(is_power_of_two(2), "is_pow2(2)");
    ASSERT(is_power_of_two(1024), "is_pow2(1024)");
    ASSERT(!is_power_of_two(0), "!is_pow2(0)");
    ASSERT(!is_power_of_two(100), "!is_pow2(100)");
    ASSERT(!is_power_of_two(3), "!is_pow2(3)");
    
    ASSERT(bit_length(0) == 0, "bit_len(0) = 0");
    ASSERT(bit_length(1) == 1, "bit_len(1) = 1");
    ASSERT(bit_length(255) == 8, "bit_len(255) = 8");
    ASSERT(bit_length(256) == 9, "bit_len(256) = 9");
    
    ASSERT(reverse_bits(1, 3) == 4, "reverse(1,3) = 4");
    ASSERT(reverse_bits(0, 1) == 0, "reverse(0,1) = 0");
    ASSERT(reverse_bits(3, 2) == 3, "reverse(3,2) = 3");
    
    printf("utils: %d passed, %d failed\n\n", tests_passed, tests_failed);
}

void test_large_mul() {
    printf("=== LARGE MUL TESTS ===\n");
    
    BigUInt *a = biguint_from_uint64(1000000);
    BigUInt *b = biguint_from_uint64(2000000);
    BigUInt *c = biguint_mul(a, b);
    ASSERT_EQ(c, biguint_from_uint64(2000000000000ULL), "1M * 2M");
    biguint_free(a);
    biguint_free(b);
    biguint_free(c);
    
    printf("large_mul: %d passed, %d failed\n\n", tests_passed, tests_failed);
}

void test_normalize() {
    printf("=== NORMALIZE TESTS ===\n");
    
    uint64_t words[] = {0, 0, 0};
    BigUInt *a = biguint_from_slice(words, 3);
    ASSERT(biguint_is_zero(a), "all zeros = zero");
    ASSERT(a->len == 0, "normalized length = 0");
    biguint_free(a);
    
    words[0] = 5;
    words[1] = 0;
    a = biguint_from_slice(words, 3);
    ASSERT(a->len == 1 && a->words[0] == 5, "trailing zeros stripped");
    biguint_free(a);
    
    printf("normalize: %d passed, %d failed\n\n", tests_passed, tests_failed);
}

int main() {
    printf("\n");
    test_add();
    test_sub();
    test_mul();
    test_div();
    test_mod();
    test_cmp();
    test_shl();
    test_shr();
    test_bitwise();
    test_not();
    test_bit_length();
    test_gcd();
    test_utils();
    test_large_mul();
    test_normalize();
    
    printf("====================\n");
    printf("TOTAL: %d passed, %d failed\n", tests_passed, tests_failed);
    printf("====================\n");
    
    return tests_failed > 0 ? 1 : 0;
}
