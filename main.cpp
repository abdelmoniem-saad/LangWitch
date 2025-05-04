//
// Created by Basmala Kamal on 5/4/2025.
//

#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <set>
#include <iomanip>
#include "language_trie.h"
#include "normalize.h"
#include <fstream>

// Function to load words from a file into a LanguageTrie
void loadWordsFromFile(const std::string& filename, LanguageTrie* trie) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << "\n";
        return;
    }

    std::string word;
    while (std::getline(file, word)) {
        trie->insert(word);
    }

    file.close();
}

std::pair<std::string, double> detectLanguage(
    const std::string& input,
    LanguageTrie* english,
    LanguageTrie* french,
    LanguageTrie* german,
    LanguageTrie* spanish,
    LanguageTrie* italian
    ) {
    std::istringstream stream(input);
    std::string word;

    map<string, map<string, int>> matrix;
    map<string, map<string, set<string>>> contributors;
    vector<string> langs = {"English", "French", "German", "Spanish", "Italian"};

    while (stream >> word) {

        set<std::string> detected;
        string normalized = normalizeWord(word);

        int en = english->getMatchScore(normalized);
        int fr = french->getMatchScore(normalized);
        int de = german->getMatchScore(normalized);
        int sp = spanish->getMatchScore(normalized);
        int it = italian->getMatchScore(normalized);
        if (en) detected.insert("English");
        if (fr) detected.insert("French");
        if (de) detected.insert("German");
        if (sp) detected.insert("Spanish");
        if (it) detected.insert("Italian");

        if (detected.size() == 1) {
            std::string lang = *detected.begin();
            matrix[lang][lang] += 2;
            contributors[lang][lang].insert(word);
        } else if (detected.size() > 1) {
            for (const auto& l1 : detected) {
                for (const auto& l2 : detected) {
                    if (l1 != l2) {
                        matrix[l1][l2] += 1;
                        contributors[l1][l2].insert(word);
                    }
                }
            }
        }
    }

    std::string bestLang;
    int maxDiagonal = -1;
    for (const std::string& lang : langs) {
        if (matrix[lang][lang] > maxDiagonal) {
            maxDiagonal = matrix[lang][lang];
            bestLang = lang;
        }
    }
    int total = 0;
    for (const std::string& row : langs) {
        for (const std::string& col : langs) {
            if (row == col || row < col) total += matrix[row][col];
        }
    }
    double confidence = (total > 0) ? static_cast<double>(matrix[bestLang][bestLang]) / total : 0.0;
    std::cout << "\n--- Language Word Match Square Matrix ---\n";
    std::cout << std::setw(10) << "";
    for (const auto& col : langs) {
        std::cout << std::setw(10) << col;
    }
    std::cout << "\n";
    for (const auto& row : langs) {
        std::cout << std::setw(10) << row;
        for (const auto& col : langs) {
            std::cout << std::setw(10) << matrix[row][col];
        }
        std::cout << "\n";
    }
    std::cout << "\n--- Word Contributors per Matrix Cell ---\n";
    for (const auto& row : langs) {
        for (const auto& col : langs) {
            if (!contributors[row][col].empty()) {
                std::cout << row << " " << col << " : ";
                for (const auto& w : contributors[row][col]) {
                    std::cout << w << " ";
                }
                std::cout << "\n";
            }
        }
    }
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\nPredicted Language: " << bestLang << " | Confidence: " << (confidence * 100.0) << "%\n";
    return {bestLang, confidence};
}


int main() {
    // Create tries
    LanguageTrie* english = new LanguageTrie("English");
    LanguageTrie* french = new LanguageTrie("French");
    LanguageTrie* german = new LanguageTrie("German");
    LanguageTrie* spanish = new LanguageTrie("Spanish");
    LanguageTrie* italian = new LanguageTrie("Italian");


    // Load words from corpus files
    loadWordsFromFile("english.txt", english);
    loadWordsFromFile("french.txt", french);
    loadWordsFromFile("german.txt", german);
    loadWordsFromFile("spanish.txt", spanish);
    loadWordsFromFile("italian.txt", italian);

    std::string input;
    std::cout << "Enter a sentence to detect its language:\n> ";
    std::getline(std::cin, input);

    detectLanguage(input, english, french, german, spanish, italian);
    // Clean up
    delete english;
    delete french;
    delete german;
    delete spanish;
    delete italian;

    return 0;
}
