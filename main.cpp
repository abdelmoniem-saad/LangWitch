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
using namespace std;


void loadWordsFromFile(const string& filename, LanguageTrie* trie) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << "\n";
        return;
    }

    string word;
    while (getline(file, word)) {
        trie->insert(word);
    }

    file.close();
}

    pair<string, double> detectLanguage(
    const string& input,
    LanguageTrie* english,
    LanguageTrie* french,
    LanguageTrie* german,
    LanguageTrie* spanish,
    LanguageTrie* italian
    ) {



    bool hasAlphabetic = false;
    for (char ch : input) {
        if (isalpha(ch)) {
            hasAlphabetic = true;
            break;
        }
    }
    if (!hasAlphabetic) {

        return {"Unknown", 0.0};
    }

    istringstream stream(input);
    string word;

    map<string, map<string, double>> matrix;
    map<string, map<string, set<string>>> contributors;
    vector<string> langs = {"English", "French", "German", "Spanish", "Italian"};

    while (stream >> word) {
        set<string> detected;
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

        if (!detected.empty()) {
            for (const auto& lang : detected) {
                matrix[lang][lang] += 1;
                contributors[lang][lang].insert(word);
            }
            for (const auto& l1 : detected) {
                for (const auto& l2 : detected) {
                    if (l1 != l2) {
                        matrix[l1][l2] += 0.5;
                        contributors[l1][l2].insert(word);
                    }
                }
            }
        }
    }

    string bestLang;
    int maxDiagonal = -1;
    for (const string& lang : langs) {
        if (matrix[lang][lang] > maxDiagonal) {
            maxDiagonal = matrix[lang][lang];
            bestLang = lang;
        }
    }


    int total = 0;
    for (const string& row : langs) {
        for (const string& col : langs) {
            if (row == col || row < col) total += matrix[row][col];
        }
    }
    double confidence = (total > 0) ? static_cast<double>(matrix[bestLang][bestLang]) / total : 0.0;
    cout << "\n--- Language Word Match Square Matrix ---\n";
    cout << setw(10) << "";
    for (const auto& col : langs) {
        cout << setw(10) << col;
    }
    cout << "\n";
    for (const auto& row : langs) {
        cout << setw(10) << row;
        for (const auto& col : langs) {
            cout << setw(10) << matrix[row][col];
        }
        cout << "\n";
    }
    cout << "\n--- Word Contributors per Matrix Cell ---\n";
    for (const auto& row : langs) {
        for (const auto& col : langs) {
            if (!contributors[row][col].empty()) {
                cout << row << " " << col << " : ";
                for (const auto& w : contributors[row][col]) {
                    cout << w << " ";
                }
                cout << "\n";
            }
        }
    }
    cout << fixed << setprecision(2);
    return {bestLang, confidence};
}

void runTests(LanguageTrie* english, LanguageTrie* french, LanguageTrie* german, LanguageTrie* spanish, LanguageTrie* italian) {
    vector<pair<string, string>> testCases = {
        // 1. Exact matches
        {"hello world", "English"},
        {"bonjour le monde", "French"},
        {"hallo welt", "German"},
        {"hola mundo", "Spanish"},
        {"ciao mondo", "Italian"},
        // 2. Mixed input – all languages
        {"hello bonjour hallo hola ciao", "English"}, // Based on majority match
        // 3. Empty input
        {"", "Unknown"},
        // 4. Input with accents (French)
        {"je suis très content", "French"}, // includes accents (très)
        // 5. Input with accent-less equivalents
        {"je suis tres content", "French"}, // tests normalization fallback
        // 6. Shared word (English + French)
        {"content", "English"}, // Appears in both, should be low confidence
        // 7. German special character ß
        {"straße", "German"}, // exact German word with ß
        // 8. Normalized form of German ß
        {"strasse", "German"}, // normalization test if you support mapping ß -> ss or s
        // 9. Word that appears in multiple languages
        {"pizza", "Italian"}, // might exist in several dictionaries
        // 10. Input with numbers and punctuation
        {"12345! bonjour.", "French"}, // numbers/punct ignored
        // 11. Capital accented letter
        {"À la carte", "French"}, // capital À
        // 12. Out-of-vocabulary words
        {"flerbin schmaggle", "Unknown"}, // nonsense / OOV
        // 13. Tie situation
        {"world monde welt", "English"}, // 1 word per language, expect tie handling
        // 14. Minor typos (up to 2 modifications)
            {"helo wrld", "English"},              // typo for "hello world"
            {"bonjor le mnde", "French"},          // typo for "bonjour le monde"
            {"hallo weltz", "German"},             // extra char
            {"holaa mundo", "Spanish"},            // double 'a'
            {"cia mond", "Italian"},              // missing 'o'

            // 15. Typo in accented word
            {"tres contnet", "French"},           // typo in "très content"

            // 16. Garbage with 1 correct word
            {"flargle hallo blurt", "German"},
    };

    cout << "\n--- Running Test Cases ---\n";
    for (const auto& testCase : testCases) {
        const string& input = testCase.first;
        const string& expected = testCase.second;

        auto [detectedLang, confidence] = detectLanguage(input, english, french, german, spanish, italian);
        cout << "Input: \"" << input << "\" | Expected: " << expected
                  << " | Detected: " << detectedLang << endl;
    }
}

int main() {
    // Create tries
    LanguageTrie* english = new LanguageTrie("English");
    LanguageTrie* french = new LanguageTrie("French");
    LanguageTrie* german = new LanguageTrie("German");
    LanguageTrie* spanish = new LanguageTrie("Spanish");
    LanguageTrie* italian = new LanguageTrie("Italian");


    // Load from corpus files
    loadWordsFromFile("english.txt", english);
    loadWordsFromFile("french.txt", french);
    loadWordsFromFile("german.txt", german);
    loadWordsFromFile("spanish.txt", spanish);
    loadWordsFromFile("italian.txt", italian);


    runTests(english, french, german, spanish, italian);

    string input;

    while (true) {
        cout << "Enter a sentence to detect its language (or type 'Stop eating the processor' to quit):\n> ";
        getline(cin, input);

        if (input == "Stop eating the processor") {
            cout << "Just Kidding! Tries are leightwight on the processor";
            break;
        }

        auto [detectedLang, confidence] = detectLanguage(input, english, french, german, spanish, italian);
        cout << fixed << setprecision(2);
        cout << "Detected Language: " << detectedLang << endl;
    }

    // Clean up
    delete english;
    delete french;
    delete german;
    delete spanish;
    delete italian;

    return 0;
}
