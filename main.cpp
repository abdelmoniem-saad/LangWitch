#include <wx/wx.h>
#include <wx/textctrl.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
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
#include <wx/notebook.h>
#include <iomanip>
#include <sstream>
#include <wx/config.h>


using namespace std;

// Structure to hold detection results including matrix and contributors
struct DetectionResult {
    std::string language;
    double confidence;
    std::map<std::string, std::map<std::string, int>> matrix;
    std::map<std::string, std::map<std::string, std::set<std::string>>> contributors;
};

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
DetectionResult detectLanguageWithMatrix(
    const std::string& input,
    LanguageTrie* english,
    LanguageTrie* french,
    LanguageTrie* german,
    LanguageTrie* spanish,
    LanguageTrie* italian
) {
    DetectionResult result;

    // Check for empty or non-alphabetic input
    bool hasAlphabetic = false;
    for (char ch : input) {
        if (isalpha(ch)) {
            hasAlphabetic = true;
            break;
        }
    }

    if (!hasAlphabetic) {
        result.language = "Unknown";
        result.confidence = 0.0;
        return result;
    }

    // Process words and build matrix
    std::istringstream stream(input);
    std::string word;
    std::vector<std::string> langs = {"English", "French", "German", "Spanish", "Italian"};

    while (stream >> word) {
        std::set<std::string> detected;
        std::string normalized = normalizeWord(word);

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
                result.matrix[lang][lang] += 1;
                result.contributors[lang][lang].insert(word);
            }

            for (const auto& l1 : detected) {
                for (const auto& l2 : detected) {
                    if (l1 != l2) {
                        result.matrix[l1][l2] += 0.5;
                        result.contributors[l1][l2].insert(word);
                    }
                }
            }
        }
    }

    // Find best language
    std::string bestLang;
    int maxDiagonal = -1;

    for (const std::string& lang : langs) {
        if (result.matrix[lang][lang] > maxDiagonal) {
            maxDiagonal = result.matrix[lang][lang];
            bestLang = lang;
        }
    }

    // Calculate total for confidence
    int total = 0;
    for (const std::string& row : langs) {
        for (const std::string& col : langs) {
            if (row == col || row < col) total += result.matrix[row][col];
        }
    }

    result.language = bestLang;
    result.confidence = (total > 0) ? static_cast<double>(result.matrix[bestLang][bestLang]) / total : 0.0;

    return result;
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
    ~LangWitchFrame();

    void OnSave(wxCommandEvent& event);

private:
    // Add notebook reference
    wxNotebook* notebook;

    // Panel for each tab
    wxPanel* mainPanel;
    wxPanel* testPanel;

    // Controls for main tab
    wxTextCtrl* inputField;
    wxTextCtrl* outputField;

    // Controls for test tab
    wxTextCtrl* testOutputField;

    // Existing members
    LanguageTrie* english;
    LanguageTrie* french;
    LanguageTrie* german;
    LanguageTrie* spanish;
    LanguageTrie* italian;

    void OnDetectLanguage(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnToggleDarkMode(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);
    void OnRunTests(wxCommandEvent& event); // New handler
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
    EVT_MENU(wxID_SAVE, LangWitchFrame::OnSave)
    EVT_BUTTON(1002, LangWitchFrame::OnRunTests) // Add button handler for test tab
wxEND_EVENT_TABLE()


wxIMPLEMENT_APP(LangWitchApp);

bool LangWitchApp::OnInit() {
    LangWitchFrame* frame = new LangWitchFrame("LangWitch Language Detector");
    frame->Show(true);
    return true;
}

LangWitchFrame::LangWitchFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(600, 500)),
      english(nullptr), french(nullptr), german(nullptr), spanish(nullptr), italian(nullptr) {

    // Menu Bar (keep existing menu code unchanged)
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

    // Preferences menu
    wxMenu* prefsMenu = new wxMenu;
    prefsMenu->AppendCheckItem(1003, "Dark Mode", "Toggle dark mode");
    menuBar->Append(prefsMenu, "&Preferences");

    // Create the notebook control - this will hold both tabs
    notebook = new wxNotebook(this, wxID_ANY);

    // Create the main detection tab
    mainPanel = new wxPanel(notebook, wxID_ANY);
    wxBoxSizer* mainVbox = new wxBoxSizer(wxVERTICAL);

    wxStaticText* inputLabel = new wxStaticText(mainPanel, wxID_ANY, "Enter Text:");
    mainVbox->Add(inputLabel, 0, wxALL, 10);

    inputField = new wxTextCtrl(mainPanel, wxID_ANY, "", wxDefaultPosition, wxSize(400, 30));
    mainVbox->Add(inputField, 0, wxALL | wxEXPAND, 10);

    wxButton* detectButton = new wxButton(mainPanel, 1001, "Detect Language");
    mainVbox->Add(detectButton, 0, wxALL | wxALIGN_CENTER, 10);

    wxStaticText* outputLabel = new wxStaticText(mainPanel, wxID_ANY, "Detected Language:");
    mainVbox->Add(outputLabel, 0, wxALL, 10);

    outputField = new wxTextCtrl(mainPanel, wxID_ANY, "", wxDefaultPosition, wxSize(400, 300),
                                 wxTE_MULTILINE | wxTE_READONLY);
    mainVbox->Add(outputField, 1, wxALL | wxEXPAND, 10);

    // Set monospaced font for better matrix display
    wxFont monoFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    outputField->SetFont(monoFont);

    mainPanel->SetSizer(mainVbox);

    // Create the test tab
    testPanel = new wxPanel(notebook, wxID_ANY);
    wxBoxSizer* testVbox = new wxBoxSizer(wxVERTICAL);

    wxStaticText* testLabel = new wxStaticText(testPanel, wxID_ANY, "Test Cases:");
    testVbox->Add(testLabel, 0, wxALL, 10);

    wxButton* runTestsButton = new wxButton(testPanel, 1002, "Run Test Cases");
    testVbox->Add(runTestsButton, 0, wxALL | wxALIGN_CENTER, 10);

    testOutputField = new wxTextCtrl(testPanel, wxID_ANY, "", wxDefaultPosition, wxSize(400, 350),
                                     wxTE_MULTILINE | wxTE_READONLY);
    testVbox->Add(testOutputField, 1, wxALL | wxEXPAND, 10);

    // Set monospaced font for test output display
    testOutputField->SetFont(monoFont);

    testPanel->SetSizer(testVbox);

    // Add the tabs to the notebook
    notebook->AddPage(mainPanel, "Language Detection");
    notebook->AddPage(testPanel, "Test Cases");

    // Set up the main sizer for the frame
    wxBoxSizer* frameSizer = new wxBoxSizer(wxVERTICAL);
    frameSizer->Add(notebook, 1, wxEXPAND | wxALL, 5);
    SetSizer(frameSizer);

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

    // Use the new function that returns more detailed results
    DetectionResult result = detectLanguageWithMatrix(input, english, french, german, spanish, italian);

    std::ostringstream output;
    std::vector<std::string> langs = {"English", "French", "German", "Spanish", "Italian"};

    // Format the basic detection result
    output << "Language: " << result.language << "\n";
    output << "Confidence: " << std::fixed << std::setprecision(2) << result.confidence * 100 << "%\n\n";

    // Add the matrix display
    output << "--- Language Word Match Square Matrix ---\n";
    output << std::setw(10) << "";
    for (const auto& col : langs) {
        output << std::setw(10) << col;
    }
    output << "\n";

    for (const auto& row : langs) {
        output << std::setw(10) << row;
        for (const auto& col : langs) {
            output << std::setw(10) << result.matrix[row][col];
        }
        output << "\n";
    }

    // Add word contributors information
    output << "\n--- Word Contributors per Matrix Cell ---\n";
    for (const auto& row : langs) {
        for (const auto& col : langs) {
            if (!result.contributors[row][col].empty()) {
                output << row << " " << col << " : ";
                for (const auto& w : result.contributors[row][col]) {
                    output << w << " ";
                }
                output << "\n";
            }
        }
    }

    outputField->SetValue(output.str());
    SetStatusText("Detection complete");
}


void LangWitchFrame::OnExit(wxCommandEvent& event) {
    Close(true);
}

void LangWitchFrame::OnAbout(wxCommandEvent& event) {
    wxMessageBox("LangWitch Language Detector\nDeveloped by Abd El-Moniem Saad and Basmala Kamal", "About LangWitch", wxOK | wxICON_INFORMATION, this);
}

void LangWitchFrame::OnToggleDarkMode(wxCommandEvent& event) {
    bool darkMode = event.IsChecked();
    // Define colors for dark and light modes
    wxColour textColor = darkMode ? wxColour(220, 220, 220) : wxColour(10, 10, 10);
    wxColour bgColor = darkMode ? wxColour(40, 40, 40) : wxColour(255, 255, 255);
    wxColour controlBgColor = darkMode ? wxColour(60, 60, 60) : wxColour(250, 250, 250);
    wxColour buttonBgColor = darkMode ? wxColour(80, 80, 80) : wxColour(225, 225, 225);

    // Set frame and notebook background
    SetBackgroundColour(bgColor);
    notebook->SetBackgroundColour(bgColor);

    // Apply to both tabs
    mainPanel->SetBackgroundColour(bgColor);
    testPanel->SetBackgroundColour(bgColor);

    // Update main panel controls
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

    // Update test panel controls
    for (wxWindow* control : testPanel->GetChildren()) {
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

    // Update status bar
    if (wxStatusBar* statusBar = GetStatusBar()) {
        statusBar->SetBackgroundColour(darkMode ? wxColour(50, 50, 50) : wxColour(240, 240, 240));
        statusBar->SetForegroundColour(textColor);
    }

    wxFont tabFont = notebook->GetFont();
    notebook->SetForegroundColour(textColor);

    // Force refresh for notebook tabs
    notebook->Refresh();


    // Force refresh
    Refresh();
    Update();

    // Inform user about menu bar changes
    wxString message = "Menu bar styling requires a restart to fully apply the theme.";
    GetStatusBar()->SetStatusText(message);
}


void LangWitchFrame::OnRunTests(wxCommandEvent& event) {
    SetStatusText("Running test cases...");

    // Clear test output
    testOutputField->Clear();

    // Define test cases
    std::vector<std::pair<std::string, std::string>> testCases = {
        {"hello world", "English"},
        {"bonjour le monde", "French"},
        {"hallo welt", "German"},
        {"hola mundo", "Spanish"},
        {"ciao mondo", "Italian"},
        {"hello bonjour hallo hola ciao", "English"}, // Mixed input
        {"", "Unknown"} // Empty input
    };

    std::ostringstream output;
    output << "=== Running Test Cases ===\n\n";

    for (const auto& testCase : testCases) {
        const std::string& input = testCase.first;
        const std::string& expected = testCase.second;

        // Use the detectLanguageWithMatrix function to get detailed results
        DetectionResult result = detectLanguageWithMatrix(input, english, french, german, spanish, italian);

        output << "Input: \"" << input << "\"\n"
               << "Expected: " << expected << "\n"
               << "Detected: " << result.language << "\n"
               << "Confidence: " << std::fixed << std::setprecision(2)
               << result.confidence * 100 << "%\n\n";

        // Add matrix for this test case
        std::vector<std::string> langs = {"English", "French", "German", "Spanish", "Italian"};

        output << "--- Language Matrix ---\n";
        output << std::setw(10) << "";
        for (const auto& col : langs) {
            output << std::setw(10) << col;
        }
        output << "\n";

        for (const auto& row : langs) {
            output << std::setw(10) << row;
            for (const auto& col : langs) {
                output << std::setw(10) << result.matrix[row][col];
            }
            output << "\n";
        }

        output << "\n----------------------------------------\n\n";
    }

    testOutputField->SetValue(output.str());
    SetStatusText("Test cases completed");
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