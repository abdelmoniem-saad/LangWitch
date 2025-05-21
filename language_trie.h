#ifndef LANGUAGE_TRIE_H
#define LANGUAGE_TRIE_H

#include <string>
#include "trie_node.h"
#include "normalize.h"
#include <vector>
#include <algorithm>
using namespace std;
int levenshtein(const std::string& s1, const std::string& s2) {
    const size_t len1 = s1.size(), len2 = s2.size();
    std::vector<std::vector<int>> dp(len1 + 1, std::vector<int>(len2 + 1));

    for (size_t i = 0; i <= len1; ++i) dp[i][0] = static_cast<int>(i);
    for (size_t j = 0; j <= len2; ++j) dp[0][j] = static_cast<int>(j);

    for (size_t i = 1; i <= len1; ++i) {
        for (size_t j = 1; j <= len2; ++j) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            dp[i][j] = std::min({
                dp[i - 1][j] + 1,
                dp[i][j - 1] + 1,
                dp[i - 1][j - 1] + cost
            });
        }
    }

    return dp[len1][len2];
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

    bool fuzzySearch(TrieNode* node, const std::string& word, std::string current, int& minDist) const {
        if (!node || current.size() > word.size() + 2) return false;

        int dist = levenshtein(word, current);
        if (dist <= 1 && node->isEndOfWord) {
            minDist = std::min(minDist, dist);
            return true;
        }

        for (int i = 0; i < CHAR_SIZE; ++i) {
            if (node->children[i]) {
                fuzzySearch(node->children[i], word, current + node->children[i]->value, minDist);
            }
        }
        return false;
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

        // Fuzzy match (Levenshtein ≤ 2)
        int minDist = 3;
        if (fuzzySearch(root, normalized, "", minDist)) return 1;

        return 0;
    }

    string getLanguageName() const {
        return languageName;
    }
};

#endif
