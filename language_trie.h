#ifndef LANGUAGE_TRIE_H
#define LANGUAGE_TRIE_H

#include <string>
#include "trie_node.h"
using namespace std;
class LanguageTrie {
private:
    TrieNode* root;
    string languageName;

public:
    // Constructor
    LanguageTrie(const string& languageName) {
        root = new TrieNode();
    }

    // Destructor
    ~LanguageTrie() {
        delete root;
    }

    // Insert a new word
    void insert(const string& word) {
        TrieNode* current = root;
        for (char ch : word) {
            unsigned char index = static_cast<unsigned char>(ch);
            if (current->children[index] == nullptr) {
                current->children[index] = new TrieNode(ch);
            }
            current = current->children[index];
        }
        current->isEndOfWord = true;
    }

    // Search for a matching word
    bool search(const string& word) const {
        TrieNode* current = root;
        for (char ch : word) {
            unsigned char index = static_cast<unsigned char>(ch);
            if (current->children[index] == nullptr) {
                return false;
            }
            current = current->children[index];
        }
        return current->isEndOfWord;
    }

    // Get the language
    string getLanguageName() const {
        return languageName;
    }
};

#endif
