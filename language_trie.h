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
    LanguageTrie(const string& languageName) : languageName(languageName) {
        root = new TrieNode();
    }

    // Destructor
    ~LanguageTrie() {
        delete root;
    }

    // Insert normalized version of word
    void insert(const string& word) {
        // Insert exact form
        TrieNode* current = root;
        for (char ch : word) {
            unsigned char index = static_cast<unsigned char>(ch);
            if (index >= CHAR_SIZE) continue;
            if (!current->children[index])
                current->children[index] = new TrieNode(ch);
            current = current->children[index];
        }
        current->isEndOfWord = true;

        // Insert normalized only if different
        std::string normalized = normalizeWord(word);
        if (normalized != word) {
            current = root;
            for (char ch : normalized) {
                unsigned char index = static_cast<unsigned char>(ch);
                if (index >= CHAR_SIZE) continue;
                if (!current->children[index])
                    current->children[index] = new TrieNode(ch);
                current = current->children[index];
            }
            current->isNormalizedWord = true;
        }
    }


    int getMatchScore(const std::string& word) const {
        // Try exact match
        TrieNode* current = root;
        for (char ch : word) {
            unsigned char index = static_cast<unsigned char>(ch);
            if (index >= CHAR_SIZE || !current->children[index]) {
                current = nullptr;
                break;
            }
            current = current->children[index];
        }
        if (current && current->isEndOfWord)
            return 2;

        // Try normalized match
        std::string normalized = normalizeWord(word);

        // Do NOT skip if normalized == word (important!)
        TrieNode* normNode = root;
        for (char ch : normalized) {
            unsigned char index = static_cast<unsigned char>(ch);
            if (index >= CHAR_SIZE || !normNode->children[index]) {
                normNode = nullptr;
                break;
            }
            normNode = normNode->children[index];
        }
        return (normNode && normNode->isNormalizedWord) ? 1 : 0;
    }


    string getLanguageName() const {
        return languageName;
    }
};

#endif
