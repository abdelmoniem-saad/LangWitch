#include <wx/wx.h>
#include <wx/textctrl.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/wfstream.h>  // Add this include for file operations
#include <wx/txtstrm.h>   // Add this include for text stream
#include "language_trie.h"
#include "normalize.h"
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <iomanip>
#include <fstream>
#include <functional>
#include <queue>
#include <wx/config.h>


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
    ~LangWitchFrame(); // Add this line

    // ... rest of your class
    // Add this method to the class declaration
    void OnSave(wxCommandEvent& event);

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
    void OnToggleDarkMode(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);  // Add this declaration

    void LoadLanguageTries();
    wxDECLARE_EVENT_TABLE();
};

// Event Table
wxBEGIN_EVENT_TABLE(LangWitchFrame, wxFrame)
    EVT_MENU(wxID_EXIT, LangWitchFrame::OnExit)
    EVT_MENU(wxID_ABOUT, LangWitchFrame::OnAbout)
    EVT_BUTTON(1001, LangWitchFrame::OnDetectLanguage)
    EVT_MENU(1003, LangWitchFrame::OnToggleDarkMode)
    EVT_MENU(wxID_OPEN, LangWitchFrame::OnOpen)
    // Add to event table
    EVT_MENU(wxID_SAVE, LangWitchFrame::OnSave)
wxEND_EVENT_TABLE()

wxIMPLEMENT_APP(LangWitchApp);

bool LangWitchApp::OnInit() {
    LangWitchFrame* frame = new LangWitchFrame("LangWitch Language Detector");
    frame->Show(true);
    return true;
}

LangWitchFrame::LangWitchFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(600, 400)),
      english(nullptr), french(nullptr), german(nullptr), spanish(nullptr), italian(nullptr) {
    // Rest of constructor
    // Menu Bar
    wxMenu* fileMenu = new wxMenu;
    fileMenu->Append(wxID_OPEN, "&Open\tCtrl+O", "Open a file");
    fileMenu->Append(wxID_SAVE, "&Save\tCtrl+S", "Save current text");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, "E&xit\tAlt+X", "Quit this program");

    wxMenu* helpMenu = new wxMenu;
    helpMenu->Append(wxID_ABOUT, "&About\tF1", "Show about dialog");



    wxMenuBar* menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, "&File");
    menuBar->Append(helpMenu, "&Help");
    SetMenuBar(menuBar);

    // Add a preferences menu
    wxMenu* prefsMenu = new wxMenu;
    prefsMenu->AppendCheckItem(1003, "Dark Mode", "Toggle dark mode");
    menuBar->Append(prefsMenu, "&Preferences");

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

wxAcceleratorEntry entries[1];
entries[0].Set(wxACCEL_CTRL, (int)'D', 1001); // Ctrl+D for detection

wxAcceleratorTable accel(1, entries);
SetAcceleratorTable(accel);

CreateStatusBar();
SetStatusText("Ready");
}

void LangWitchFrame::LoadLanguageTries() {
    english = new LanguageTrie("English");
    french = new LanguageTrie("French");
    german = new LanguageTrie("German");
    spanish = new LanguageTrie("Spanish");
    italian = new LanguageTrie("Italian");

    loadWordsFromFile("/home/mnm/auc/uni/sem/spring25/CSCE2211/project/LangWitch/english.txt", english);
    loadWordsFromFile("/home/mnm/auc/uni/sem/spring25/CSCE2211/project/LangWitch/french.txt", french);
    loadWordsFromFile("/home/mnm/auc/uni/sem/spring25/CSCE2211/project/LangWitch/german.txt", german);
    loadWordsFromFile("/home/mnm/auc/uni/sem/spring25/CSCE2211/project/LangWitch/spanish.txt", spanish);
    loadWordsFromFile("/home/mnm/auc/uni/sem/spring25/CSCE2211/project/LangWitch/italian.txt", italian);
}

void LangWitchFrame::OnDetectLanguage(wxCommandEvent& event) {
    SetStatusText("Detecting language...");

    // Use proper UTF-8 conversion
    wxString wxInput = inputField->GetValue();
    std::string input = wxInput.ToUTF8().data();

    auto [detectedLang, confidence] = detectLanguage(input, english, french, german, spanish, italian);

    std::ostringstream result;
    result << "Language: " << detectedLang << "\nConfidence: " << confidence * 100 << "%";
    outputField->SetValue(result.str());

    SetStatusText("Detection complete");
}

void LangWitchFrame::OnExit(wxCommandEvent& event) {
    Close(true);
}

void LangWitchFrame::OnAbout(wxCommandEvent& event) {
    wxMessageBox("LangWitch Language Detector\nDeveloped with wxWidgets", "About LangWitch", wxOK | wxICON_INFORMATION, this);
}

   void LangWitchFrame::OnToggleDarkMode(wxCommandEvent& event) {
    bool darkMode = event.IsChecked();

    // Define colors for dark and light modes
    wxColour textColor = darkMode ? wxColour(220, 220, 220) : wxColour(10, 10, 10);
    wxColour bgColor = darkMode ? wxColour(40, 40, 40) : wxColour(255, 255, 255);
    wxColour controlBgColor = darkMode ? wxColour(60, 60, 60) : wxColour(250, 250, 250);
    wxColour buttonBgColor = darkMode ? wxColour(80, 80, 80) : wxColour(225, 225, 225);

    // Apply GTK theme at system level (most natural approach for KDE)
    #ifdef __WXGTK__
    if (darkMode) {
        wxExecute("gsettings set org.gnome.desktop.interface gtk-theme 'Adwaita-dark'", wxEXEC_ASYNC);
    } else {
        wxExecute("gsettings set org.gnome.desktop.interface gtk-theme 'Adwaita'", wxEXEC_ASYNC);
    }
    #endif

    // Set frame and panel background
    SetBackgroundColour(bgColor);

    // Apply to controls (text fields, buttons, labels)
    wxPanel* mainPanel = nullptr;
    for (wxWindow* child : GetChildren()) {
        if (wxPanel* panel = dynamic_cast<wxPanel*>(child)) {
            mainPanel = panel;
            panel->SetBackgroundColour(bgColor);
            break;
        }
    }

    if (mainPanel) {
        for (wxWindow* control : mainPanel->GetChildren()) {
            if (wxTextCtrl* textCtrl = dynamic_cast<wxTextCtrl*>(control)) {
                textCtrl->SetBackgroundColour(controlBgColor);
                textCtrl->SetForegroundColour(textColor);
            }
            else if (wxButton* button = dynamic_cast<wxButton*>(control)) {
                button->SetBackgroundColour(buttonBgColor);
                button->SetForegroundColour(textColor);
            }
            else if (wxStaticText* label = dynamic_cast<wxStaticText*>(control)) {
                label->SetForegroundColour(textColor);
            }
        }
    }

    // Update status bar
    if (wxStatusBar* statusBar = GetStatusBar()) {
        statusBar->SetBackgroundColour(darkMode ? wxColour(50, 50, 50) : wxColour(240, 240, 240));
        statusBar->SetForegroundColour(textColor);
    }

    // Force refresh
    Refresh();
    Update();

    // Inform user about menu bar changes
    wxString message = "Menu bar styling requires a restart to fully apply the theme.";
    GetStatusBar()->SetStatusText(message);
}

// Then implement the handlers:
void LangWitchFrame::OnOpen(wxCommandEvent& event) {
    wxFileDialog openDialog(this, "Open Text File", "", "",
                           "Text files (*.txt)|*.txt|All files (*.*)|*.*",
                           wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (openDialog.ShowModal() == wxID_CANCEL)
        return;

    wxFileInputStream input(openDialog.GetPath());
    if (!input.IsOk()) {
        wxLogError("Cannot open file '%s'.", openDialog.GetPath());
        return;
    }

    wxTextInputStream text(input);
    wxString line;
    wxString content;

    while (!input.Eof()) {
        line = text.ReadLine();
        content.Append(line);
        content.Append("\n");
    }

    inputField->SetValue(content);
    SetStatusText("File loaded.");
}

// Implementation
void LangWitchFrame::OnSave(wxCommandEvent& event) {
    wxFileDialog saveDialog(this, "Save Text File", "", "",
                          "Text files (*.txt)|*.txt|All files (*.*)|*.*",
                          wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (saveDialog.ShowModal() == wxID_CANCEL)
        return;

    wxFileOutputStream output(saveDialog.GetPath());
    if (!output.IsOk()) {
        wxLogError("Cannot save to file '%s'.", saveDialog.GetPath());
        return;
    }

    wxTextOutputStream text(output);

    // Save the input text
    text.WriteString("=== INPUT TEXT ===\n");
    text.WriteString(inputField->GetValue());

    // Add a separator
    text.WriteString("\n\n=== LANGUAGE DETECTION RESULT ===\n");

    // Save the detection result
    text.WriteString(outputField->GetValue());

    SetStatusText("Input text and detection result saved.");
}

LangWitchFrame::~LangWitchFrame() {
    delete english;
    delete french;
    delete german;
    delete spanish;
    delete italian;
}