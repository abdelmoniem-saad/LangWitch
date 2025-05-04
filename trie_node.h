#ifndef TRIE_NODE_H
#define TRIE_NODE_H

// ASCII (0–127)
const int CHAR_SIZE = 128;

struct TrieNode {
    char value;
    bool isEndOfWord;
    bool isNormalizedWord;

    TrieNode* children[CHAR_SIZE];  // children nodes

    // Constructor
    TrieNode(char val = '\0') {
        isEndOfWord = false;
        isNormalizedWord = false;
        for (int i = 0; i < CHAR_SIZE; i++) children[i] = nullptr;
    }


    // Destructor
    ~TrieNode() {
        for (int i = 0; i < CHAR_SIZE; ++i) {
            if (children[i]) {
                delete children[i];
                children[i] = nullptr;
            }
        }
    }
};

#endif
