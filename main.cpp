#include <wx/wx.h>
#include <wx/textctrl.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include "language_trie.h"
#include "normalize.h"
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <iomanip>
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

// Function to detect the language of a given input
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
    return {bestLang, confidence};
}

// Main Application Class
class LangWitchApp : public wxApp {
public:
    virtual bool OnInit();
};

// Main Frame Class
class LangWitchFrame : public wxFrame {
public:
    LangWitchFrame(const wxString& title);

private:
    wxTextCtrl* inputField;
    wxTextCtrl* outputField;

    LanguageTrie* english;
    LanguageTrie* french;
    LanguageTrie* german;
    LanguageTrie* spanish;
    LanguageTrie* italian;

    void OnDetectLanguage(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

    void LoadLanguageTries();
    wxDECLARE_EVENT_TABLE();
};

// Event Table
wxBEGIN_EVENT_TABLE(LangWitchFrame, wxFrame)
    EVT_MENU(wxID_EXIT, LangWitchFrame::OnExit)
    EVT_MENU(wxID_ABOUT, LangWitchFrame::OnAbout)
    EVT_BUTTON(1001, LangWitchFrame::OnDetectLanguage)
wxEND_EVENT_TABLE()

wxIMPLEMENT_APP(LangWitchApp);

bool LangWitchApp::OnInit() {
    LangWitchFrame* frame = new LangWitchFrame("LangWitch Language Detector");
    frame->Show(true);
    return true;
}

LangWitchFrame::LangWitchFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(600, 400)) {
    // Menu Bar
    wxMenu* fileMenu = new wxMenu;
    fileMenu->Append(wxID_EXIT, "E&xit\tAlt-X", "Quit this program");

    wxMenu* helpMenu = new wxMenu;
    helpMenu->Append(wxID_ABOUT, "&About\tF1", "Show about dialog");

    wxMenuBar* menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, "&File");
    menuBar->Append(helpMenu, "&Help");
    SetMenuBar(menuBar);

    // Input and Output Fields
    wxPanel* panel = new wxPanel(this, wxID_ANY);
    wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);

    wxStaticText* inputLabel = new wxStaticText(panel, wxID_ANY, "Enter Text:");
    vbox->Add(inputLabel, 0, wxALL, 10);

    inputField = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(400, 30));
    vbox->Add(inputField, 0, wxALL | wxEXPAND, 10);

    wxButton* detectButton = new wxButton(panel, 1001, "Detect Language");
    vbox->Add(detectButton, 0, wxALL | wxALIGN_CENTER, 10);

    wxStaticText* outputLabel = new wxStaticText(panel, wxID_ANY, "Detected Language:");
    vbox->Add(outputLabel, 0, wxALL, 10);

    outputField = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(400, 100), wxTE_MULTILINE | wxTE_READONLY);
    vbox->Add(outputField, 1, wxALL | wxEXPAND, 10);

    panel->SetSizer(vbox);

    // Initialize Language Tries
    LoadLanguageTries();
}

void LangWitchFrame::LoadLanguageTries() {
    english = new LanguageTrie("English");
    french = new LanguageTrie("French");
    german = new LanguageTrie("German");
    spanish = new LanguageTrie("Spanish");
    italian = new LanguageTrie("Italian");

    loadWordsFromFile("english.txt", english);
    loadWordsFromFile("french.txt", french);
    loadWordsFromFile("german.txt", german);
    loadWordsFromFile("spanish.txt", spanish);
    loadWordsFromFile("italian.txt", italian);
}

void LangWitchFrame::OnDetectLanguage(wxCommandEvent& event) {
    std::string input = inputField->GetValue().ToStdString();
    auto [detectedLang, confidence] = detectLanguage(input, english, french, german, spanish, italian);

    std::ostringstream result;
    result << "Language: " << detectedLang << "\nConfidence: " << confidence * 100 << "%";
    outputField->SetValue(result.str());
}

void LangWitchFrame::OnExit(wxCommandEvent& event) {
    Close(true);
}

void LangWitchFrame::OnAbout(wxCommandEvent& event) {
    wxMessageBox("LangWitch Language Detector\nDeveloped with wxWidgets", "About LangWitch", wxOK | wxICON_INFORMATION, this);
}