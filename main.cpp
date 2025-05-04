//
// Created by Basmala Kamal on 5/4/2025.
//

#include <iostream>
#include <sstream>
#include <map>
#include "language_trie.h"

void tokenizeAndDetect(
    const std::string& input,
    LanguageTrie* english,
    LanguageTrie* french,
    LanguageTrie* german
) {
    std::istringstream stream(input);
    std::string word;

    int enCount = 0, frCount = 0, deCount = 0;

    while (stream >> word) {
        enCount += english->getMatchScore(word);
        frCount += french->getMatchScore(word);
        deCount += german->getMatchScore(word);
    }

    std::cout << "\n--- Language Match Counts ---\n";
    std::cout << "English: " << enCount << "\n";
    std::cout << "French : " << frCount << "\n";
    std::cout << "German : " << deCount << "\n";

    if (enCount == 0 && frCount == 0 && deCount == 0) {
        std::cout << "No match found. Unable to detect language.\n";
        return;
    }

    std::map<std::string, int> scores = {
        {"English", enCount},
        {"French", frCount},
        {"German", deCount}
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

    // Insert some sample words
    english->insert("hello");
    english->insert("world");
    english->insert("computer");
    english->insert("language");
    english->insert("dejavu");
    english->insert("content");

    french->insert("bonjour");
    french->insert("monde");
    french->insert("ordinateur");
    french->insert("langue");
    french->insert("complément");
    french->insert("déjàvu");
    french->insert("content");
    french->insert("je");
    french->insert("suis");
    french->insert("trés");

    german->insert("hallo");
    german->insert("welt");
    german->insert("sprache");
    german->insert("computer");
    german->insert("straße");
    german->insert("hallo");

    std::string input;
    std::cout << "Enter a sentence to detect its language:\n> ";
    std::getline(std::cin, input);

    tokenizeAndDetect(input, english, french, german);

    // Clean up
    delete english;
    delete french;
    delete german;


    return 0;
    return 0;
}
