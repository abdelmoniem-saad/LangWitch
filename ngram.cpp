#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
using namespace std;

unordered_map<string, unordered_map<string, int>> lang_ngrams; // language -> ngram -> count
vector<string> languages = {"English", "French", "German", "Spanish", "Italian"};

string normalize(const string& s) {
    string result;
    for (char ch : s) {
        if (isalpha(static_cast<unsigned char>(ch))) {
            result += tolower(ch);
        }
    }
    return result;
}

vector<string> generate_ngrams(const string& word, int n = 3) {
    vector<string> result;
    if (word.length() < n) return result;
    for (size_t i = 0; i <= word.length() - n; ++i) {
        result.push_back(word.substr(i, n));
    }
    return result;
}

void load_ngram_profile(const string& filename, const string& language) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Could not open file: " << filename << endl;
        return;
    }
    string word;
    while (file >> word) {
        string normalized = normalize(word);
        for (const string& ngram : generate_ngrams(normalized)) {
            lang_ngrams[language][ngram]++;
        }
    }
    file.close();
}

string detect_by_ngram(const string& input) {
    unordered_map<string, int> scores;
    stringstream ss(input);
    string word;
    while (ss >> word) {
        string normalized = normalize(word);
        for (const string& ngram : generate_ngrams(normalized)) {
            for (const string& lang : languages) {
                scores[lang] += lang_ngrams[lang][ngram];
            }
        }
    }

    string best_lang = "Unknown";
    int max_score = 0;
    for (const auto& [lang, score] : scores) {
        if (score > max_score) {
            max_score = score;
            best_lang = lang;
        }
    }
    return best_lang;
}

int main() {
    for (const string& lang : languages) {
        string filename = lang + "_adjectives.txt"; // each file contains language-specific adjectives
        load_ngram_profile(filename, lang);
    }

    vector<pair<string, string>> testCases = {
         // 1. Exact matches – common words
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

    cout << "\n--- N-gram Test Cases ---\n";
    for (const auto& [input, expected] : testCases) {
        string detected = detect_by_ngram(input);
        cout << "Input: '" << input << "' | Expected: " << expected << " | Detected: " << detected << endl;
    }

    return 0;
}
