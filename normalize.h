#ifndef NORMALIZE_H
#define NORMALIZE_H

#include <string>
#include <unordered_map>
#include <cctype>

// Map common UTF-8 accented characters (2-byte sequences) to ASCII
const std::unordered_map<std::string, char> utf8_accent_map = {
    {"\xC3\xA0", 'a'}, {"\xC3\xA1", 'a'}, {"\xC3\xA2", 'a'}, {"\xC3\xA3", 'a'}, {"\xC3\xA4", 'a'}, {"\xC3\xA5", 'a'},
    {"\xC3\x80", 'a'}, {"\xC3\x81", 'a'}, {"\xC3\x82", 'a'}, {"\xC3\x83", 'a'}, {"\xC3\x84", 'a'}, {"\xC3\x85", 'a'},
    {"\xC3\xA7", 'c'},  {"\xC3\x87", 'c'},
    {"\xC3\xA8", 'e'}, {"\xC3\xA9", 'e'}, {"\xC3\xAA", 'e'}, {"\xC3\xAB", 'e'},
    {"\xC3\x88", 'e'}, {"\xC3\x89", 'e'}, {"\xC3\x8A", 'e'}, {"\xC3\x8B", 'e'},
    {"\xC3\xAC", 'i'}, {"\xC3\xAD", 'i'}, {"\xC3\xAE", 'i'}, {"\xC3\xAF", 'i'},
    {"\xC3\x8C", 'i'}, {"\xC3\x8D", 'i'}, {"\xC3\x8E", 'i'}, {"\xC3\x8F", 'i'},
    {"\xC3\xB1", 'n'}, {"\xC3\x91", 'n'},
    {"\xC3\xB2", 'o'}, {"\xC3\xB3", 'o'}, {"\xC3\xB4", 'o'}, {"\xC3\xB5", 'o'}, {"\xC3\xB6", 'o'},
    {"\xC3\x92", 'o'}, {"\xC3\x93", 'o'}, {"\xC3\x94", 'o'}, {"\xC3\x95", 'o'}, {"\xC3\x96", 'o'},
    {"\xC3\x99", 'u'}, {"\xC3\x9A", 'u'}, {"\xC3\x9B", 'u'}, {"\xC3\x9C", 'u'},
    {"\xC3\xB9", 'u'}, {"\xC3\xBA", 'u'}, {"\xC3\xBB", 'u'}, {"\xC3\xBC", 'u'},
    {"\xC3\xBD", 'y'}, {"\xC3\xBF", 'y'},
    {"\xC3\x9F", 's'}
};

std::string normalizeWord(const std::string& word) {
    std::string normalized;
    for (size_t i = 0; i < word.size(); ) {
        // If it's a 2-byte UTF-8 sequence
        if ((unsigned char)word[i] == 0xC3 && i + 1 < word.size()) {
            std::string seq = word.substr(i, 2);
            if (utf8_accent_map.count(seq)) {
                normalized += utf8_accent_map.at(seq);
                i += 2;
                continue;
            }
        }

        // Handle ASCII chars
        char ch = word[i];
        if ((unsigned char)ch < 128 && std::isalpha(ch)) {
            normalized += std::tolower(ch);
        }
        i++;
    }
    return normalized;
}

#endif
