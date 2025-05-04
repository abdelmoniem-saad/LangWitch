#ifndef LANGUAGE_TRIE_H
#define LANGUAGE_TRIE_H

#include <string>
#include "trie_node.h"
#include "normalize.h"

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

    // Insert a new word // file with path
    void insert(const string& word) {
        TrieNode* current = root;
        for (char ch : word) {
            unsigned char index = static_cast<unsigned char>(ch);
            if (index >= CHAR_SIZE) continue;
            if (current->children[index] == nullptr) {
                current->children[index] = new TrieNode(ch);
            }
            current = current->children[index];
        }
        current->isEndOfWord = true;
    }

    // Search for a matching word
    bool search(const string& word) const {
      // Try accented first
        TrieNode* current = root;
        for (char ch : word) {
            unsigned char index = static_cast<unsigned char>(ch);
            if (current->children[index] == nullptr || index >= CHAR_SIZE){
              current == nullptr;
              break;
            }
            current = current->children[index];
        }
        if (current && current->isEndOfWord) return true;

       	// Try normalized
        string normalized = normalizeWord(word);
        current = root;
        for (char ch : normalized) {
            unsigned char index = static_cast<unsigned char>(ch);
            if (index >= CHAR_SIZE || current->children[index] == nullptr)
                return false;
            current = current->children[index];
        }
        return current && current->isEndOfWord;
    }

    // Get the language
    string getLanguageName() const {
        return languageName;
    }
};

#endif
