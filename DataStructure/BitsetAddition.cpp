// Bitset addition using long long
// Add two bitsets represented as arrays of long long
// Useful for subset sum DP optimization

template<int MAXN>
struct BitsetAdd {
    static const int BLOCK = 64;
    static const int SIZE = (MAXN + BLOCK - 1) / BLOCK;
    
    unsigned long long a[SIZE];
    
    BitsetAdd() {
        memset(a, 0, sizeof(a));
    }
    
    void set(int pos) {
        a[pos / BLOCK] |= (1ULL << (pos % BLOCK));
    }
    
    bool test(int pos) const {
        return (a[pos / BLOCK] >> (pos % BLOCK)) & 1;
    }
    
    void reset() {
        memset(a, 0, sizeof(a));
    }
    
    // Add value to all set bits (shift left by value positions)
    void add(int value) {
        if (value == 0) return;
        
        int block_shift = value / BLOCK;
        int bit_shift = value % BLOCK;
        
        if (bit_shift == 0) {
            // Simple block shift
            for (int i = SIZE - 1; i >= block_shift; --i) {
                a[i] = a[i - block_shift];
            }
            for (int i = 0; i < block_shift; ++i) {
                a[i] = 0;
            }
        } else {
            // Complex shift with carry
            for (int i = SIZE - 1; i > block_shift; --i) {
                a[i] = (a[i - block_shift] << bit_shift) | 
                       (a[i - block_shift - 1] >> (BLOCK - bit_shift));
            }
            a[block_shift] = a[0] << bit_shift;
            for (int i = 0; i < block_shift; ++i) {
                a[i] = 0;
            }
        }
    }
    
    // OR operation
    BitsetAdd operator|(const BitsetAdd &other) const {
        BitsetAdd result;
        for (int i = 0; i < SIZE; ++i) {
            result.a[i] = a[i] | other.a[i];
        }
        return result;
    }
    
    // OR assignment
    BitsetAdd& operator|=(const BitsetAdd &other) {
        for (int i = 0; i < SIZE; ++i) {
            a[i] |= other.a[i];
        }
        return *this;
    }
    
    // AND operation
    BitsetAdd operator&(const BitsetAdd &other) const {
        BitsetAdd result;
        for (int i = 0; i < SIZE; ++i) {
            result.a[i] = a[i] & other.a[i];
        }
        return result;
    }
    
    // XOR operation
    BitsetAdd operator^(const BitsetAdd &other) const {
        BitsetAdd result;
        for (int i = 0; i < SIZE; ++i) {
            result.a[i] = a[i] ^ other.a[i];
        }
        return result;
    }
    
    // Count set bits
    int count() const {
        int cnt = 0;
        for (int i = 0; i < SIZE; ++i) {
            cnt += __builtin_popcountll(a[i]);
        }
        return cnt;
    }
    
    // Find first set bit
    int first() const {
        for (int i = 0; i < SIZE; ++i) {
            if (a[i]) {
                return i * BLOCK + __builtin_ctzll(a[i]);
            }
        }
        return -1;
    }
};

// Example usage for subset sum
// BitsetAdd<100001> dp;
// dp.set(0);
// for (int i = 0; i < n; ++i) {
//     BitsetAdd<100001> tmp = dp;
//     tmp.add(arr[i]);
//     dp |= tmp;
// }

