#include <bigint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

static void *xrealloc(void *ptr, size_t size) {
    void *p = realloc(ptr, size);
    if (!p && size > 0) {
        abort();
    }
    return p;
}

void biguint_normalize(BigUInt *a) {
    while (a->len > 0 && a->words[a->len - 1] == 0) {
        a->len--;
    }
}

BigUInt *biguint_new(void) {
    BigUInt *a = malloc(sizeof(BigUInt));
    if (!a) return NULL;
    a->words = NULL;
    a->len = 0;
    a->cap = 0;
    return a;
}

BigUInt *biguint_from_uint64(uint64_t n) {
    BigUInt *a = biguint_new();
    if (!a) return NULL;
    if (n != 0) {
        a->words = malloc(sizeof(uint64_t));
        if (!a->words) { free(a); return NULL; }
        a->words[0] = n;
        a->len = 1;
        a->cap = 1;
    }
    return a;
}

BigUInt *biguint_from_slice(const uint64_t *words, size_t len) {
    BigUInt *a = biguint_new();
    if (!a) return NULL;
    if (len == 0) return a;
    
    a->words = malloc(len * sizeof(uint64_t));
    if (!a->words) { free(a); return NULL; }
    
    memcpy(a->words, words, len * sizeof(uint64_t));
    a->len = len;
    a->cap = len;
    biguint_normalize(a);
    return a;
}

BigUInt *biguint_clone(const BigUInt *a) {
    return biguint_from_slice(a->words, a->len);
}

void biguint_free(BigUInt *a) {
    if (a) {
        free(a->words);
        free(a);
    }
}

bool biguint_is_zero(const BigUInt *a) {
    return a->len == 0;
}

size_t biguint_bit_length(const BigUInt *a) {
    if (biguint_is_zero(a)) return 0;
    uint64_t msb = a->words[a->len - 1];
    return 64 * (a->len - 1) + (64 - __builtin_clzll(msb));
}

int biguint_cmp(const BigUInt *a, const BigUInt *b) {
    if (a->len != b->len) {
        return a->len < b->len ? -1 : 1;
    }
    for (size_t i = a->len; i > 0; i--) {
        if (a->words[i-1] != b->words[i-1]) {
            return a->words[i-1] < b->words[i-1] ? -1 : 1;
        }
    }
    return 0;
}

BigUInt *biguint_add(const BigUInt *a, const BigUInt *b) {
    size_t max_len = MAX(a->len, b->len);
    BigUInt *res = biguint_new();
    if (!res) return NULL;
    
    res->words = calloc(max_len + 1, sizeof(uint64_t));
    if (!res->words) { biguint_free(res); return NULL; }
    res->cap = max_len + 1;
    
    uint64_t carry = 0;
    for (size_t i = 0; i < max_len; i++) {
        uint64_t av = i < a->len ? a->words[i] : 0;
        uint64_t bv = i < b->len ? b->words[i] : 0;
        __uint128_t sum = (__uint128_t)av + bv + carry;
        res->words[i] = (uint64_t)sum;
        carry = (uint64_t)(sum >> 64);
    }
    res->words[max_len] = carry;
    res->len = max_len + (carry ? 1 : 0);
    biguint_normalize(res);
    
    return res;
}

BigUInt *biguint_sub(const BigUInt *a, const BigUInt *b) {
    if (biguint_cmp(a, b) < 0) {
        return NULL;
    }
    
    BigUInt *res = biguint_new();
    if (!res) return NULL;
    
    res->words = malloc(a->len * sizeof(uint64_t));
    if (!res->words) { biguint_free(res); return NULL; }
    res->cap = a->len;
    
    uint64_t borrow = 0;
    for (size_t i = 0; i < a->len; i++) {
        uint64_t av = a->words[i];
        uint64_t bv = i < b->len ? b->words[i] : 0;
        uint64_t diff = av - bv - borrow;
        borrow = (av < bv + borrow) ? 1 : 0;
        res->words[i] = diff;
    }
    res->len = a->len;
    biguint_normalize(res);
    return res;
}

static void fft_inplace(double *re, double *im, size_t n, bool inverse) {
    size_t j = 0;
    for (size_t i = 0; i < n - 1; i++) {
        if (i < j) {
            double temp;
            temp = re[i]; re[i] = re[j]; re[j] = temp;
            temp = im[i]; im[i] = im[j]; im[j] = temp;
        }
        size_t k = n / 2;
        while (k <= j) {
            j -= k;
            k /= 2;
        }
        j += k;
    }
    
    for (size_t len = 2; len <= n; len <<= 1) {
        double angle = (inverse ? -2.0 : 2.0) * M_PI / len;
        double wlen_re = cos(angle);
        double wlen_im = sin(angle);
        
        for (size_t i = 0; i < n; i += len) {
            double w_re = 1.0;
            double w_im = 0.0;
            
            for (size_t k = 0; k < len / 2; k++) {
                size_t idx = i + k;
                size_t idx2 = i + k + len / 2;
                
                double u_re = re[idx];
                double u_im = im[idx];
                double v_re = re[idx2] * w_re - im[idx2] * w_im;
                double v_im = re[idx2] * w_im + im[idx2] * w_re;
                
                re[idx] = u_re + v_re;
                im[idx] = u_im + v_im;
                re[idx2] = u_re - v_re;
                im[idx2] = u_im - v_im;
                
                double new_w_re = w_re * wlen_re - w_im * wlen_im;
                double new_w_im = w_re * wlen_im + w_im * wlen_re;
                w_re = new_w_re;
                w_im = new_w_im;
            }
        }
    }
    
    if (inverse) {
        for (size_t i = 0; i < n; i++) {
            re[i] /= n;
            im[i] /= n;
        }
    }
}

BigUInt *biguint_mul(const BigUInt *a, const BigUInt *b) {
    if (biguint_is_zero(a) || biguint_is_zero(b)) {
        return biguint_new();
    }
    
    size_t n = next_power_of_two(a->len + b->len - 1);
    
    double *re_a = calloc(n, sizeof(double));
    double *im_a = calloc(n, sizeof(double));
    double *re_b = calloc(n, sizeof(double));
    double *im_b = calloc(n, sizeof(double));
    
    if (!re_a || !im_a || !re_b || !im_b) {
        free(re_a); free(im_a); free(re_b); free(im_b);
        return NULL;
    }
    
    for (size_t i = 0; i < a->len; i++) re_a[i] = (double)a->words[i];
    for (size_t i = 0; i < b->len; i++) re_b[i] = (double)b->words[i];
    
    fft_inplace(re_a, im_a, n, false);
    fft_inplace(re_b, im_b, n, false);
    
    for (size_t i = 0; i < n; i++) {
        double r = re_a[i] * re_b[i] - im_a[i] * im_b[i];
        double im = re_a[i] * im_b[i] + im_a[i] * re_b[i];
        re_a[i] = r;
        im_a[i] = im;
    }
    
    fft_inplace(re_a, im_a, n, true);
    
    BigUInt *res = biguint_new();
    if (!res) {
        free(re_a); free(im_a); free(re_b); free(im_b);
        return NULL;
    }
    
    __uint128_t carry = 0;
    for (size_t i = 0; i < n; i++) {
        __uint128_t val = (__uint128_t)(re_a[i] + 0.5) + carry;
        res->words = xrealloc(res->words, (i + 1) * sizeof(uint64_t));
        res->words[i] = (uint64_t)val;
        carry = val >> 64;
    }
    
    while (carry > 0) {
        res->words = xrealloc(res->words, (res->len + 1) * sizeof(uint64_t));
        res->words[res->len++] = (uint64_t)carry;
        carry >>= 64;
    }
    
    res->len = n;
    res->cap = n;
    biguint_normalize(res);
    
    free(re_a); free(im_a); free(re_b); free(im_b);
    return res;
}


BigUInt *biguint_div(const BigUInt *a, const BigUInt *b) {
    if (biguint_is_zero(b)) {
        return NULL;
    }
    if (biguint_cmp(a, b) < 0) {
        return biguint_new();
    }
    
    
    size_t b_bits = biguint_bit_length(b);
    
    if (b_bits == 1) {
        uint64_t d = b->words[0];
        BigUInt *res = biguint_new();
        res->words = malloc(a->len * sizeof(uint64_t));
        res->cap = a->len;
        
        __uint128_t carry = 0;
        for (size_t i = a->len; i > 0; i--) {
            __uint128_t cur = (carry << 64) | a->words[i-1];
            res->words[i-1] = (uint64_t)(cur / d);
            carry = cur % d;
        }
        res->len = a->len;
        biguint_normalize(res);
        return res;
    }
    
    BigUInt *res = biguint_new();
    res->words = calloc(a->len - b->len + 1, sizeof(uint64_t));
    res->len = a->len - b->len + 1;
    res->cap = res->len;
    
    BigUInt *remainder = biguint_new();
    remainder->words = calloc(b->len + 1, sizeof(uint64_t));
    remainder->len = b->len + 1;
    remainder->cap = remainder->len;
    
    for (size_t i = a->len; i > 0; i--) {
        for (size_t j = remainder->len - 1; j > 0; j--) {
            remainder->words[j] = remainder->words[j-1];
        }
        remainder->words[0] = a->words[i-1];
        remainder->len = b->len + 1;
        biguint_normalize(remainder);
        
        size_t r_bits = biguint_bit_length(remainder);
        size_t d_bits = biguint_bit_length(b);
        
        if (r_bits < d_bits) {
            continue;
        }
        
        __uint128_t q;
        if (remainder->len == b->len + 1) {
            q = ((__uint128_t)remainder->words[remainder->len-1] << 64) | remainder->words[remainder->len-2];
        } else if (remainder->len == b->len) {
            q = remainder->words[remainder->len-1];
        } else {
            q = 0;
        }
        
        uint64_t d = b->words[b->len-1];
        uint64_t est = (uint64_t)(q / d);
        if (est > UINT64_MAX) est = UINT64_MAX;
        
        BigUInt *sub = biguint_mul(b, biguint_from_uint64(est));
        while (biguint_cmp(remainder, sub) < 0) {
            est--;
            biguint_free(sub);
            sub = biguint_mul(b, biguint_from_uint64(est));
        }
        
        BigUInt *tmp = biguint_sub(remainder, sub);
        biguint_free(remainder);
        remainder = tmp;
        biguint_free(sub);
        
        size_t res_idx = i - b->len;
        if ((int)res_idx >= 0) {
            res->words[res_idx] = est;
        }
    }
    
    biguint_normalize(res);
    biguint_free(remainder);
    return res;
}

BigUInt *biguint_mod(const BigUInt *a, const BigUInt *b) {
    BigUInt *div = biguint_div(a, b);
    if (!div) return NULL;
    BigUInt *prod = biguint_mul(div, b);
    biguint_free(div);
    BigUInt *res = biguint_sub(a, prod);
    biguint_free(prod);
    return res;
}

BigUInt *biguint_and(const BigUInt *a, const BigUInt *b) {
    size_t len = MIN(a->len, b->len);
    BigUInt *res = biguint_new();
    if (!res) return NULL;
    
    res->words = malloc(len * sizeof(uint64_t));
    if (!res->words) { biguint_free(res); return NULL; }
    
    for (size_t i = 0; i < len; i++) {
        res->words[i] = a->words[i] & b->words[i];
    }
    res->len = len;
    biguint_normalize(res);
    return res;
}

BigUInt *biguint_or(const BigUInt *a, const BigUInt *b) {
    size_t len = MAX(a->len, b->len);
    BigUInt *res = biguint_new();
    if (!res) return NULL;
    
    res->words = malloc(len * sizeof(uint64_t));
    if (!res->words) { biguint_free(res); return NULL; }
    
    for (size_t i = 0; i < len; i++) {
        uint64_t av = i < a->len ? a->words[i] : 0;
        uint64_t bv = i < b->len ? b->words[i] : 0;
        res->words[i] = av | bv;
    }
    res->len = len;
    biguint_normalize(res);
    return res;
}

BigUInt *biguint_xor(const BigUInt *a, const BigUInt *b) {
    size_t len = MAX(a->len, b->len);
    BigUInt *res = biguint_new();
    if (!res) return NULL;
    
    res->words = malloc(len * sizeof(uint64_t));
    if (!res->words) { biguint_free(res); return NULL; }
    
    for (size_t i = 0; i < len; i++) {
        uint64_t av = i < a->len ? a->words[i] : 0;
        uint64_t bv = i < b->len ? b->words[i] : 0;
        res->words[i] = av ^ bv;
    }
    res->len = len;
    biguint_normalize(res);
    return res;
}

BigUInt *biguint_not(const BigUInt *a) {
    BigUInt *res = biguint_new();
    if (!res) return NULL;
    
    res->words = malloc((a->len + 1) * sizeof(uint64_t));
    if (!res->words) { biguint_free(res); return NULL; }
    
    for (size_t i = 0; i < a->len; i++) {
        res->words[i] = ~a->words[i];
    }
    res->words[a->len] = UINT64_MAX;
    res->len = a->len + 1;
    biguint_normalize(res);
    return res;
}

BigUInt *biguint_shl(const BigUInt *a, size_t bits) {
    if (bits == 0) return biguint_clone(a);
    
    size_t word_shift = bits / 64;
    size_t bit_shift = bits % 64;
    
    BigUInt *res = biguint_new();
    if (!res) return NULL;
    
    size_t new_len = a->len + word_shift + (bit_shift ? 1 : 0);
    res->words = calloc(new_len, sizeof(uint64_t));
    if (!res->words) { biguint_free(res); return NULL; }
    
    if (bit_shift == 0) {
        for (size_t i = 0; i < a->len; i++) {
            res->words[i + word_shift] = a->words[i];
        }
    } else {
        for (size_t i = 0; i < a->len; i++) {
            res->words[i + word_shift] |= a->words[i] << bit_shift;
            res->words[i + word_shift + 1] |= a->words[i] >> (64 - bit_shift);
        }
    }
    
    res->len = new_len;
    biguint_normalize(res);
    return res;
}

BigUInt *biguint_shr(const BigUInt *a, size_t bits) {
    if (bits == 0) return biguint_clone(a);
    
    size_t word_shift = bits / 64;
    size_t bit_shift = bits % 64;
    
    if (word_shift >= a->len) {
        return biguint_new();
    }
    
    BigUInt *res = biguint_new();
    if (!res) return NULL;
    
    size_t new_len = a->len - word_shift;
    res->words = malloc(new_len * sizeof(uint64_t));
    if (!res->words) { biguint_free(res); return NULL; }
    
    for (size_t i = word_shift; i < a->len; i++) {
        res->words[i - word_shift] = a->words[i] >> bit_shift;
        if (bit_shift > 0 && i + 1 < a->len) {
            res->words[i - word_shift] |= a->words[i + 1] << (64 - bit_shift);
        }
    }
    
    res->len = new_len;
    biguint_normalize(res);
    return res;
}

BigUInt *biguint_gcd(BigUInt *a, BigUInt *b) {
    while (!biguint_is_zero(b)) {
        BigUInt *r = biguint_mod(a, b);
        biguint_free(a);
        a = b;
        b = r;
    }
    return a;
}

size_t next_power_of_two(size_t n) {
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    return n + 1;
}

bool is_power_of_two(size_t n) {
    return n != 0 && (n & (n - 1)) == 0;
}

size_t bit_length(size_t n) {
    return n == 0 ? 0 : 64 - __builtin_clzll(n);
}

size_t reverse_bits(size_t x, size_t len) {
    if (len <= 1) return x;
    size_t result = 0;
    for (size_t i = 0; i < len; i++) {
        result = (result << 1) | (x & 1);
        x >>= 1;
    }
    return result;
}
