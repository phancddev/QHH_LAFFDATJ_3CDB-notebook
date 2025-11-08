// 2D Sparse Table for Range Minimum/Maximum Query
// Preprocessing: O(N*M*log(N)*log(M))
// Query: O(1)

template<typename T>
struct SparseTable2D {
    vector<vector<vector<vector<T>>>> st;
    vector<int> log_table;
    int n, m;
    
    // Function to combine two values (min, max, gcd, etc.)
    T combine(T a, T b) {
        return min(a, b); // Change to max(a, b) for RMQ
    }
    
    void build_log(int maxn) {
        log_table.resize(maxn + 1);
        log_table[1] = 0;
        for (int i = 2; i <= maxn; ++i) {
            log_table[i] = log_table[i / 2] + 1;
        }
    }
    
    SparseTable2D(vector<vector<T>> &a) {
        n = a.size();
        m = a[0].size();
        
        build_log(max(n, m));
        
        int log_n = log_table[n] + 1;
        int log_m = log_table[m] + 1;
        
        st.assign(log_n, vector<vector<vector<T>>>(log_m, 
                  vector<vector<T>>(n, vector<T>(m))));
        
        // Copy original array
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                st[0][0][i][j] = a[i][j];
            }
        }
        
        // Build for rows (k1 varies, k2 = 0)
        for (int k1 = 1; k1 < log_n; ++k1) {
            for (int i = 0; i + (1 << k1) <= n; ++i) {
                for (int j = 0; j < m; ++j) {
                    st[k1][0][i][j] = combine(st[k1-1][0][i][j], 
                                              st[k1-1][0][i + (1 << (k1-1))][j]);
                }
            }
        }
        
        // Build for columns (k1 varies, k2 varies)
        for (int k1 = 0; k1 < log_n; ++k1) {
            for (int k2 = 1; k2 < log_m; ++k2) {
                for (int i = 0; i + (1 << k1) <= n; ++i) {
                    for (int j = 0; j + (1 << k2) <= m; ++j) {
                        st[k1][k2][i][j] = combine(st[k1][k2-1][i][j], 
                                                   st[k1][k2-1][i][j + (1 << (k2-1))]);
                    }
                }
            }
        }
    }
    
    // Query rectangle [r1, r2] x [c1, c2] (0-indexed, inclusive)
    T query(int r1, int c1, int r2, int c2) {
        int k1 = log_table[r2 - r1 + 1];
        int k2 = log_table[c2 - c1 + 1];
        
        int len1 = 1 << k1;
        int len2 = 1 << k2;
        
        T res = st[k1][k2][r1][c1];
        res = combine(res, st[k1][k2][r2 - len1 + 1][c1]);
        res = combine(res, st[k1][k2][r1][c2 - len2 + 1]);
        res = combine(res, st[k1][k2][r2 - len1 + 1][c2 - len2 + 1]);
        
        return res;
    }
};

// Example usage:
// vector<vector<int>> a(n, vector<int>(m));
// // Fill array a
// SparseTable2D<int> st(a);
// int result = st.query(r1, c1, r2, c2);

