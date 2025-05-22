#ifndef LANGUAGE_TRIE_H
#define LANGUAGE_TRIE_H

#include <string>
#include "trie_node.h"
#include "normalize.h"
#include <vector>
#include <algorithm>
#include <climits>
using namespace std;

int levenshtein(const std::string& s1, const std::string& s2) {
    const size_t len1 = s1.size(), len2 = s2.size();
    vector<int> col(len2 + 1), prevCol(len2 + 1);

    for (size_t i = 0; i <= len2; i++)
        prevCol[i] = static_cast<int>(i);

    for (size_t i = 0; i < len1; i++) {
        col[0] = static_cast<int>(i + 1);
        for (size_t j = 0; j < len2; j++) {
            col[j + 1] = std::min({
                prevCol[1 + j] + 1,
                col[j] + 1,
                prevCol[j] + (s1[i] == s2[j] ? 0 : 1)
            });
        }
        prevCol.swap(col);
    }
    return prevCol[len2];
}

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

    bool fuzzySearch(TrieNode* node, const std::string& target, std::string current, int& minDist) const {
        if (!node || current.size() > target.size() + 1) return false;

        bool found = false;

        if (node->value != '\0') current += node->value;

        if (node->isEndOfWord) {
            int dist = levenshtein(target, current);
            if (dist <= 1) {
                minDist = std::min(minDist, dist);
                found = true;
            }
        }

        for (int i = 0; i < CHAR_SIZE; ++i) {
            if (node->children[i]) {
                found |= fuzzySearch(node->children[i], target, current, minDist);
            }
        }

        return found;
    }

    int getMatchScore(const std::string& word) const {
        // Exact match
        TrieNode* current = root;
        for (char ch : word) {
            unsigned char index = static_cast<unsigned char>(ch);
            if (index >= CHAR_SIZE || !current->children[index]) {
                current = nullptr;
                break;
            }
            current = current->children[index];
        }
        if (current && current->isEndOfWord) return 2;

        // Normalized match
        std::string normalized = normalizeWord(word);
        TrieNode* normNode = root;
        for (char ch : normalized) {
            unsigned char index = static_cast<unsigned char>(ch);
            if (index >= CHAR_SIZE || !normNode->children[index]) {
                normNode = nullptr;
                break;
            }
            normNode = normNode->children[index];
        }
        if (normNode && normNode->isNormalizedWord) return 1;

        // Fuzzy match
        int minDist = INT_MAX;
        if (fuzzySearch(root, normalized, "", minDist) && minDist <= 1)
            return 1;

        return 0;
    }

    string getLanguageName() const {
        return languageName;
    }
};

#endif
