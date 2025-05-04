//
// Created by Basmala Kamal on 5/4/2025.
//

#ifndef NORMALIZE_H
#define NORMALIZE_H

#include <string>

char normalizeChar(char ch) {
    switch (ch) {
        case 'à': case 'á': case 'â': case 'ã': case 'ä': case 'å': return 'a';
        case 'ç': return 'c';
        case 'è': case 'é': case 'ê': case 'ë': return 'e';
        case 'ì': case 'í': case 'î': case 'ï': return 'i';
        case 'ñ': return 'n';
        case 'ò': case 'ó': case 'ô': case 'õ': case 'ö': return 'o';
        case 'ù': case 'ú': case 'û': case 'ü': return 'u';
        case 'ý': case 'ÿ': return 'y';
        case 'ß': return 's'; // or 'ss'
        default:
            if (ch >= 'a' && ch <= 'z') return ch;
            else return '\0';
    }
}

std::string normalizeWord(const std::string& word) {
    std::string result;
    for (char ch : word) {
        ch = tolower(ch);
        char norm = normalizeChar(ch);
        if (norm != '\0') result += norm;
    }
    return result;
}

#endif //NORMALIZE_H
