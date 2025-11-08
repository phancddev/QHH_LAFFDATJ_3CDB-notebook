// Trie (Prefix Tree) implementation
// Supports insert, search, and prefix matching
// Time: O(L) per operation where L is string length
// Space: O(ALPHABET_SIZE * N * L) where N is number of strings

const int ALPHABET_SIZE = 26;

struct TrieNode {
    TrieNode* children[ALPHABET_SIZE];
    bool isEndOfWord;
    int count; // Number of words ending at this node
    int prefixCount; // Number of words with this prefix
    
    TrieNode() {
        isEndOfWord = false;
        count = 0;
        prefixCount = 0;
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            children[i] = nullptr;
        }
    }
};

struct Trie {
    TrieNode* root;
    
    Trie() {
        root = new TrieNode();
    }
    
    // Insert a word into the trie
    void insert(const string& word) {
        TrieNode* node = root;
        for (char c : word) {
            int index = c - 'a'; // Change to c - 'A' for uppercase
            if (node->children[index] == nullptr) {
                node->children[index] = new TrieNode();
            }
            node = node->children[index];
            node->prefixCount++;
        }
        node->isEndOfWord = true;
        node->count++;
    }
    
    // Search for exact word
    bool search(const string& word) {
        TrieNode* node = root;
        for (char c : word) {
            int index = c - 'a';
            if (node->children[index] == nullptr) {
                return false;
            }
            node = node->children[index];
        }
        return node != nullptr && node->isEndOfWord;
    }
    
    // Check if any word starts with given prefix
    bool startsWith(const string& prefix) {
        TrieNode* node = root;
        for (char c : prefix) {
            int index = c - 'a';
            if (node->children[index] == nullptr) {
                return false;
            }
            node = node->children[index];
        }
        return true;
    }
    
    // Count words with given prefix
    int countWordsWithPrefix(const string& prefix) {
        TrieNode* node = root;
        for (char c : prefix) {
            int index = c - 'a';
            if (node->children[index] == nullptr) {
                return 0;
            }
            node = node->children[index];
        }
        return node->prefixCount;
    }
    
    // Delete a word from trie
    bool deleteWord(const string& word) {
        return deleteHelper(root, word, 0);
    }
    
    bool deleteHelper(TrieNode* node, const string& word, int depth) {
        if (node == nullptr) return false;
        
        if (depth == word.length()) {
            if (!node->isEndOfWord) return false;
            node->isEndOfWord = false;
            node->count--;
            return isEmpty(node);
        }
        
        int index = word[depth] - 'a';
        if (deleteHelper(node->children[index], word, depth + 1)) {
            delete node->children[index];
            node->children[index] = nullptr;
            node->prefixCount--;
            return !node->isEndOfWord && isEmpty(node);
        }
        
        return false;
    }
    
    bool isEmpty(TrieNode* node) {
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            if (node->children[i] != nullptr) {
                return false;
            }
        }
        return true;
    }
    
    // Get all words with given prefix
    void getAllWordsWithPrefix(const string& prefix, vector<string>& result) {
        TrieNode* node = root;
        for (char c : prefix) {
            int index = c - 'a';
            if (node->children[index] == nullptr) {
                return;
            }
            node = node->children[index];
        }
        getAllWordsHelper(node, prefix, result);
    }
    
    void getAllWordsHelper(TrieNode* node, string current, vector<string>& result) {
        if (node->isEndOfWord) {
            result.push_back(current);
        }
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            if (node->children[i] != nullptr) {
                getAllWordsHelper(node->children[i], current + char('a' + i), result);
            }
        }
    }
    
    // Find longest common prefix
    string longestCommonPrefix() {
        string prefix = "";
        TrieNode* node = root;
        
        while (node != nullptr && !node->isEndOfWord && countChildren(node) == 1) {
            for (int i = 0; i < ALPHABET_SIZE; i++) {
                if (node->children[i] != nullptr) {
                    prefix += char('a' + i);
                    node = node->children[i];
                    break;
                }
            }
        }
        return prefix;
    }
    
    int countChildren(TrieNode* node) {
        int count = 0;
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            if (node->children[i] != nullptr) {
                count++;
            }
        }
        return count;
    }
};

// Alternative: Trie using map for dynamic alphabet
struct TrieNodeMap {
    map<char, TrieNodeMap*> children;
    bool isEndOfWord;
    int count;
    
    TrieNodeMap() : isEndOfWord(false), count(0) {}
};

struct TrieMap {
    TrieNodeMap* root;
    
    TrieMap() {
        root = new TrieNodeMap();
    }
    
    void insert(const string& word) {
        TrieNodeMap* node = root;
        for (char c : word) {
            if (node->children.find(c) == node->children.end()) {
                node->children[c] = new TrieNodeMap();
            }
            node = node->children[c];
        }
        node->isEndOfWord = true;
        node->count++;
    }
    
    bool search(const string& word) {
        TrieNodeMap* node = root;
        for (char c : word) {
            if (node->children.find(c) == node->children.end()) {
                return false;
            }
            node = node->children[c];
        }
        return node != nullptr && node->isEndOfWord;
    }
    
    bool startsWith(const string& prefix) {
        TrieNodeMap* node = root;
        for (char c : prefix) {
            if (node->children.find(c) == node->children.end()) {
                return false;
            }
            node = node->children[c];
        }
        return true;
    }
};

// Example usage:
// Trie trie;
// trie.insert("hello");
// trie.insert("world");
// bool found = trie.search("hello"); // true
// bool hasPrefix = trie.startsWith("hel"); // true
// int count = trie.countWordsWithPrefix("hel"); // 1

