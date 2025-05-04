//
// Created by Basmala Kamal on 5/4/2025.
//

#include <iostream>
#include <sstream>
#include <map>
#include "language_trie.h"
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

void tokenizeAndDetect(
    const std::string& input,
    LanguageTrie* english,
    LanguageTrie* french,
    LanguageTrie* german,
    LanguageTrie* spanish,
    LanguageTrie* italian
) {
    std::istringstream stream(input);
    std::string word;

    int enCount = 0, frCount = 0, deCount = 0, spCount = 0, itCount = 0;

    while (stream >> word) {
        enCount += english->getMatchScore(word);
        frCount += french->getMatchScore(word);
        deCount += german->getMatchScore(word);
        spCount += spanish->getMatchScore(word);
        itCount += italian->getMatchScore(word);
    }

    std::cout << "\n--- Language Match Counts ---\n";
    std::cout << "English: " << enCount << "\n";
    std::cout << "French : " << frCount << "\n";
    std::cout << "German : " << deCount << "\n";
    std::cout << "Spanish : " << spCount << "\n";
    std::cout << "Italian : " << itCount << "\n";

    if (enCount == 0 && frCount == 0 && deCount == 0 && spCount == 0 && itCount == 0) {
        std::cout << "No match found. Unable to detect language.\n";
        return;
    }

    std::map<std::string, int> scores = {
        {"English", enCount},
        {"French", frCount},
        {"German", deCount},
        {"Spanish", spCount},
        {"Italian", itCount}
    };

    std::string bestLang;
    int maxScore = -1;
    for (auto& [lang, count] : scores) {
        if (count > maxScore) {
            bestLang = lang;
            maxScore = count;
        }
    }

    std::cout << "\n Detected Language: " << bestLang << "\n";
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

    tokenizeAndDetect(input, english, french, german, spanish, italian);

    // Clean up
    delete english;
    delete french;
    delete german;
    delete spanish;
    delete italian;

    return 0;
}
