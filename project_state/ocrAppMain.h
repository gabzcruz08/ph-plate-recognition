/***************************************************************
 * Name:      ocrAppMain.h
 * Purpose:   Defines Application Frame
 * Author:    
 * Created:   2017-09-17
 * Copyright: 
 * License:
 **************************************************************/
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <string.h>
#include <sstream>
#include <stdlib.h>

using namespace std;

class MyApp: public wxApp
{
public:
    virtual bool OnInit();
};

class MyFrame: public wxFrame
{
public:
    MyFrame(const wxString& title, const wxPoint& pos, 
        const wxSize& size);
    
private:
    //GUI Functions
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void Clear(wxCommandEvent& event); 
    
    //Image Panel Functions
    void viewnext(wxCommandEvent& event);
    void viewprev(wxCommandEvent& event);
    void backtoimg(wxCommandEvent& event);
    
    //Loading Functions
    void OnLoad(wxCommandEvent& event);
    void train();
    
    //Saving Functions
    void OnSave(wxCommandEvent& event);

    //Recognition Functions
    void Identify(wxCommandEvent& event);
    char identifier(wxImage &image);
    char converter(int value);

    //Definitions
    #define W 500
    #define H 500

    //Variables
    wxImage input;       //Initial Image
    wxImage theinputs[52];//Letters
    wxImage trainset[52];//Training Set
    string word;         //Interpretation
    bool edited;         //Indicates whether an image has been edited
    bool loaded;         //Indicates whether an image has been loaded
    int viewnow;
    int num_letters;
    
    //Declarations
	wxStaticBitmap *WxStaticBitmap1;
    wxFileDialog* FileDialog1;
    wxStaticText *label;

    wxDECLARE_EVENT_TABLE();
};

enum
{
    ID_Load = 2,
    ID_Clear = 3,
    ID_Save = 4,
    ID_Identify = 5,
    ID_WxBitmap1 = 6,
    ID_Next = 7,
    ID_Prev = 8,
    ID_Img = 9
};

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(ID_Load,    MyFrame::OnLoad)
    EVT_MENU(ID_Clear,   MyFrame::Clear)
    EVT_MENU(ID_Save,    MyFrame::OnSave)
    EVT_MENU(ID_Identify,    MyFrame::Identify)
    EVT_MENU(wxID_EXIT,  MyFrame::OnExit)
    EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
    
    EVT_MENU(ID_Next, MyFrame::viewnext)  
    EVT_MENU(ID_Prev, MyFrame::viewprev)  
    EVT_MENU(ID_Img, MyFrame::backtoimg)     
wxEND_EVENT_TABLE()

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
    MyFrame *frame = new MyFrame( "AlphabeticOCR", wxPoint(25, 25), 
        wxSize(W+15, H+150) );
    frame->Show( true );
    return true;
}

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, 
        const wxSize& size)
        : wxFrame(NULL, wxID_ANY, title, pos, size)
{
    wxInitAllImageHandlers();   //Initializes All Image Handlers
    
    //Load Menu Bar Contents
    //Menu Bar - File   
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(ID_Load, "&Load...\tCtrl-L", "Load Images");;
    menuFile->Append(ID_Clear, "&Clear", "Clears the Screen");;
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
    
    //Menu Bar - Execute
    wxMenu *menuExecute = new wxMenu;
    menuExecute->Append(ID_Identify, "&Identify...\tCtrl-I",
                     "Identifies the character on screen");
    menuExecute->Append(ID_Save, "&Save Text...\tCtrl-S", 
                     "Save Text to a .txt File");;
    
    //Menu Bar - Help                 
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
    
    //Menu Bar 
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append( menuFile, "&File" );
    menuBar->Append( menuExecute, "&Execute");
    menuBar->Append( menuHelp, "&Help" );
    SetMenuBar( menuBar );
    CreateStatusBar();
    SetStatusText( "Welcome to AlphabeticOCR!" );
    
    //Bitmap
    WxStaticBitmap1 = new wxStaticBitmap(
        this, wxID_ANY, wxBitmap("pattern.jpg", wxBITMAP_TYPE_JPEG),
        wxPoint(0,0), wxSize(W, H));
    WxStaticBitmap1->SetBackgroundColour(wxColour(000,000,000));
    ClearBackground();
    
    //Dialog Boxes
    FileDialog1 = new wxFileDialog(this, _("Select file"), 
    wxEmptyString, wxEmptyString, 
    _("BMP, GIF, JPEG and PNG files|*.bmp;*.gif;*.jpg;*.jpeg;*.png"), 
    wxFD_DEFAULT_STYLE, wxDefaultPosition, 
    wxDefaultSize, _T("wxFileDialog"));
    
    //Initial Functions
    train();
    
    //Error Handling
    loaded = 0;
    edited = 0;
    
    //Text
    int w, h, h1, h2, h3, w1, w2;
    GetClientSize(&w, &h);
    h1 = 0.6*h; h2 = 0.1*h; h3 = h-h1-h2;
    w1 = 0.5*w; w2 = (w-w1)*0.5;
    
    SetFont(wxFont(32, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, 
        wxFONTWEIGHT_NORMAL));
    
    input.LoadFile("pattern.jpg", wxBITMAP_TYPE_ANY);
    WxStaticBitmap1->SetBitmap(input.Scale(W,H));
    
    label = new wxStaticText(this, wxID_ANY, "AlphabeticOCR", 
        wxPoint(0,h1), wxSize(w, h2), 
        wxALIGN_CENTER | wxST_NO_AUTORESIZE);
    
    SetFont(wxFont(12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, 
        wxFONTWEIGHT_NORMAL));
    
    //Button
    wxButton *next = new wxButton(this, wxID_ANY, "View Next Letter",
        wxPoint(w2*0.5,510), wxSize(w2,25));
    wxButton *prev = new wxButton(this, wxID_ANY, "View Prev Letter",
        wxPoint(w1-w1/2+w2*0.5, 510), wxSize(w2,25));
    wxButton *loadimg = new wxButton(this, wxID_ANY, "View Image",
        wxPoint(w1+w2*0.5 , 510), wxSize(w2,25));
    
    next->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(MyFrame::viewnext), NULL, this);
    prev->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(MyFrame::viewprev), NULL, this);
    loadimg->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(MyFrame::backtoimg), NULL, this);
}
