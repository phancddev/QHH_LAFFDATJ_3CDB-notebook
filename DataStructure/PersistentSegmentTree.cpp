// Persistent Segment Tree
// Supports point update and range query with version control
// Space: O(N + Q*log(N)) where Q is number of updates
// Time: O(log(N)) per operation

struct PersistentSegmentTree {
    struct Node {
        int val;
        int left, right;
        Node() : val(0), left(-1), right(-1) {}
        Node(int v, int l, int r) : val(v), left(l), right(r) {}
    };
    
    vector<Node> tree;
    vector<int> roots;
    int n;
    
    PersistentSegmentTree(int _n) : n(_n) {
        tree.reserve(2 * n + 1000000); // Reserve space for nodes
        roots.push_back(build(0, n - 1));
    }
    
    PersistentSegmentTree(vector<int> &a) : n(a.size()) {
        tree.reserve(2 * n + 1000000);
        roots.push_back(build(a, 0, n - 1));
    }
    
    int build(int l, int r) {
        int node = tree.size();
        tree.push_back(Node());
        if (l == r) {
            tree[node].val = 0;
        } else {
            int mid = (l + r) / 2;
            tree[node].left = build(l, mid);
            tree[node].right = build(mid + 1, r);
            tree[node].val = tree[tree[node].left].val + tree[tree[node].right].val;
        }
        return node;
    }
    
    int build(vector<int> &a, int l, int r) {
        int node = tree.size();
        tree.push_back(Node());
        if (l == r) {
            tree[node].val = a[l];
        } else {
            int mid = (l + r) / 2;
            tree[node].left = build(a, l, mid);
            tree[node].right = build(a, mid + 1, r);
            tree[node].val = tree[tree[node].left].val + tree[tree[node].right].val;
        }
        return node;
    }
    
    // Update position pos to value val, creates new version
    int update(int node, int l, int r, int pos, int val) {
        int new_node = tree.size();
        tree.push_back(tree[node]); // Copy current node
        
        if (l == r) {
            tree[new_node].val = val;
        } else {
            int mid = (l + r) / 2;
            if (pos <= mid) {
                tree[new_node].left = update(tree[node].left, l, mid, pos, val);
            } else {
                tree[new_node].right = update(tree[node].right, mid + 1, r, pos, val);
            }
            tree[new_node].val = tree[tree[new_node].left].val + tree[tree[new_node].right].val;
        }
        return new_node;
    }
    
    void update(int pos, int val) {
        int new_root = update(roots.back(), 0, n - 1, pos, val);
        roots.push_back(new_root);
    }
    
    // Query sum in range [ql, qr] for specific version
    int query(int node, int l, int r, int ql, int qr) {
        if (node == -1 || qr < l || r < ql) return 0;
        if (ql <= l && r <= qr) return tree[node].val;
        
        int mid = (l + r) / 2;
        return query(tree[node].left, l, mid, ql, qr) + 
               query(tree[node].right, mid + 1, r, ql, qr);
    }
    
    int query(int version, int ql, int qr) {
        return query(roots[version], 0, n - 1, ql, qr);
    }
    
    // Get value at position pos for specific version
    int get(int node, int l, int r, int pos) {
        if (l == r) return tree[node].val;
        int mid = (l + r) / 2;
        if (pos <= mid) return get(tree[node].left, l, mid, pos);
        else return get(tree[node].right, mid + 1, r, pos);
    }
    
    int get(int version, int pos) {
        return get(roots[version], 0, n - 1, pos);
    }
    
    int version_count() {
        return roots.size();
    }
};

// Example usage for k-th smallest in range [l, r]:
// Build PST on sorted positions
// For each element, update its position in sorted order
// Query difference between version[r+1] and version[l]
// Binary search on answer

// K-th smallest number in range [l, r] using PST
struct KthSmallest {
    PersistentSegmentTree pst;
    int n;
    
    KthSmallest(vector<int> &a) : n(a.size()), pst(n) {
        vector<pair<int, int>> sorted_a;
        for (int i = 0; i < n; ++i) {
            sorted_a.push_back({a[i], i});
        }
        sort(sorted_a.begin(), sorted_a.end());
        
        for (int i = 0; i < n; ++i) {
            int pos = sorted_a[i].second;
            pst.update(pos, 1);
        }
    }
    
    // Find k-th smallest (1-indexed) in range [l, r]
    int kth_smallest(int l, int r, int k) {
        return query_kth(pst.roots[0], pst.roots[r - l + 1], 0, n - 1, k);
    }
    
    int query_kth(int node_l, int node_r, int l, int r, int k) {
        if (l == r) return l;
        
        int mid = (l + r) / 2;
        int left_count = pst.tree[pst.tree[node_r].left].val - 
                        pst.tree[pst.tree[node_l].left].val;
        
        if (k <= left_count) {
            return query_kth(pst.tree[node_l].left, pst.tree[node_r].left, l, mid, k);
        } else {
            return query_kth(pst.tree[node_l].right, pst.tree[node_r].right, 
                           mid + 1, r, k - left_count);
        }
    }
};

