/************************************************s***************
 * Name:      ocrAppMain.cpp
 * Purpose:   Code for Application Class
 * Author:    
 * Created:   2017-09-17
 * Copyright: 
 * License:
 **************************************************************/
#include "ocrAppMain.h"
#include "ocrAppPrepro.h"
#include <iostream>
#include <fstream>

using namespace std;

//GUI Operations
void MyFrame::OnExit(wxCommandEvent& event)
{
    Close( true );
}
void MyFrame::OnSave(wxCommandEvent& event)
{
    if (loaded == 1 && edited == 1)
    {
        ofstream myfile ("output.txt");
        if (myfile.is_open())
        {
            myfile << word + "\n";
            myfile.close();
            label->SetLabel("Saved");
        }  
    }
}
void MyFrame::OnAbout(wxCommandEvent& event)
{
    string msg1 = "This is an Optical Character Recognition that ";
    string msg2 = "uses Template Matching for Feature Extraction. ";
    string msg3 = "To use, load an image using Ctrl + L. ";
    string msg4 = "After this, execute identification by pressing ";
    string msg5 = "Ctrl + I. To save the result, press Ctrl + S";
    wxMessageBox( msg1 + msg2 + "\n\n" + msg3 + msg4 + msg5
        , "Help", wxOK | wxICON_INFORMATION );
}
void MyFrame::Clear(wxCommandEvent& event)
{   
    //Clears the GUI
    wxImage temp;
    temp.Create(W,H);
    for (int x = 0; x < W; x++)
    {
        for (int y = 0; y < H; y++)
        {
          temp.SetRGB(x, y, 0, 0, 0);
        }
    }
    WxStaticBitmap1->SetBitmap(temp);
    edited = 0;
    loaded = 0;
}

//Image Panel Functions
void MyFrame::viewnext(wxCommandEvent& event)
{
    if (loaded == 1 && edited == 1)
    {
        viewnow++;
        if(viewnow == num_letters)
            viewnow = 0;
        WxStaticBitmap1->SetBitmap(theinputs[viewnow].Scale(W,H));
    }
}
void MyFrame::viewprev(wxCommandEvent& event)
{
    if (loaded == 1 && edited == 1)
    {
        viewnow--;
        if(viewnow == -1)
            viewnow = num_letters - 1;
        WxStaticBitmap1->SetBitmap(theinputs[viewnow].Scale(W,H));
    }
}
void MyFrame::backtoimg(wxCommandEvent& event)
{
    if (loaded == 1)
    {
        WxStaticBitmap1->SetBitmap(input.Scale(W,H));
    }
}

//Loading Functions
void MyFrame::OnLoad(wxCommandEvent& event)
{   
    //File Path Acquisition
    FileDialog1->ShowModal();
    if (FileDialog1->GetPath().IsEmpty()) return;
    
    //File Loading
    input.LoadFile(FileDialog1->GetPath(), wxBITMAP_TYPE_ANY);
    //display the image 
    WxStaticBitmap1->SetBitmap(input.Scale(W,H)); 

    //For Error Handling
    loaded = 1;
    edited = 0;   
}
void MyFrame::train()
{
    int count = 11;
    //Loads Images
    for (int i=0; i<52; i++)
    {
        //Converts int to string
        string cnt;         
        stringstream cnter;
        cnter << count;
        cnt = cnter.str();
        
        //Loads Image
        trainset[i].LoadFile("trainset\\img0" + cnt + "-00986.png",
            wxBITMAP_TYPE_ANY);
        count++;
        
        //Apply Corresponding Filters
        grayscale(trainset[i]);
        threshold(trainset[i], 1);
        segmentation(trainset[i]); 
        trainset[i].Rescale(W/5,H/5);
    }
}

//Recognition Functions
void MyFrame::Identify(wxCommandEvent& event)
{
    word = "";
    if (loaded == 1)
    {
        //Apply Filters and Manipulations on Image
        if (edited == 0)
        {
            grayscale(input);
            threshold(input, 0);
            segmentation(input);
            num_letters = segmentation_word(input, theinputs);
            for (int a = 0; a< num_letters; a++)
            {
                segmentation(theinputs[a]);
                theinputs[a].Rescale(W/5,H/5);
            }
            WxStaticBitmap1->SetBitmap(input.Scale(W,H));
            edited = 1;
        }
        
        //Identification
        for (int i = 0; i < num_letters; i++)
        {
            stringstream ss;
            string str;
            char ch = identifier(theinputs[i]);
            ss << ch;
            ss >> str;
            word.append(str);
        }
        label->SetLabel(word);
    }
}

char MyFrame::identifier(wxImage &image)
{
    int red, red1;
    int stat[52];
    int sort_stat[52];
    int maxindent = 0;
    
    //Generation of Histogram
    for(int i=0; i<52; i++)
    {
        stat[i] = 0;
        for (int x = 0; x < image.GetWidth(); x++)
        {
            for (int y = 0; y < image.GetHeight(); y++)
            {
                red = image.GetRed(x,y);
                red1 = trainset[i].GetRed(x,y);
                if(red == red1)
                {
                    stat[i]++;
                }
            }
        }
    }
    
    //Determination of Peak
    for (int i=0; i<52; i++)
    {
        sort_stat[i] = stat[i];
    }
    qsort(sort_stat, 52, sizeof(int), compare);
    
    //Interpretation of Histogram Peak    
    char cnt; 
    for (int i = 0; i < 52; i++)
    {
        if(sort_stat[51] == stat[i])
        {
            maxindent = i;
        }
    }
    cnt = converter(maxindent);
    return cnt;
}

char MyFrame::converter(int value)
{
    char val;
    //Numbers
    if(value > 25)
    {
        val = value+22; 
    }
    //Uppercase Letter
    else
    {
        val = value+65;  
    }
    return val;
}
