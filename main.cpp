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

// Function to load words from a file into a LanguageTrie
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


    // Check if input is empty or contains only non-alphabetic characters
    bool hasAlphabetic = false;
    for (char ch : input) {
        if (isalpha(ch)) {
            hasAlphabetic = true;
            break;
        }
    }
    if (!hasAlphabetic) {
        // Handle empty or non-alphabetic input explicitly
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
        {"hello world", "English"},
        {"bonjour le monde", "French"},
        {"hallo welt", "German"},
        {"hola mundo", "Spanish"},
        {"ciao mondo", "Italian"},
        {"hello bonjour hallo hola ciao", "English"}, // Mixed input
        {"", "Unknown"} // Empty input
    };

    cout << "\n--- Running Test Cases ---\n";
    for (const auto& testCase : testCases) {
        const string& input = testCase.first;
        const string& expected = testCase.second;

        auto [detectedLang, confidence] = detectLanguage(input, english, french, german, spanish, italian);
        cout << "Input: \"" << input << "\" | Expected: " << expected
                  << " | Detected: " << detectedLang
                  << " | Confidence: " << (confidence * 100.0) << "%\n";
    }
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

    // Run test cases
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
        cout << "Detected Language: " << detectedLang;
    }

    // Clean up
    delete english;
    delete french;
    delete german;
    delete spanish;
    delete italian;

    return 0;
}
