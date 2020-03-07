/***************************************************************
 * Name:      ocrAppPrepro.h
 * Purpose:   Code for Pre-Processing, Grayscale, Binarization,
 *            Segmentation, and Segmentation of Word
 * Author:    
 * Created:   2017-10-07
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

//Functions
int compare (const void * a, const void * b);
void color_inversion (wxImage &image3);

//Definitions
#define W 500
#define H 500

void grayscale(wxImage &image1)
{
    //Grayscale Filter
    int red, green, blue, lum;
    
    for (int x = 0; x < image1.GetWidth(); x++)
    {
        for (int y = 0; y < image1.GetHeight(); y++)
        { 
          red = image1.GetRed(x,y)*0.21;
          green = image1.GetGreen(x,y)*0.72;
          blue = image1.GetBlue(x,y)*0.07;
          lum = red + green + blue;
          image1.SetRGB(x, y, lum, lum, lum);
        }
    }
}
 
void threshold(wxImage &image2, bool isLetter)
{
    //Otsu's Binarization
    //For identification purposes, is is assumed that the image has
    //already been applied with a grayscale filter
    int windowx = image2.GetWidth();
    int windowy = image2.GetHeight();
    int histoarray[256], sort_histoarray[256];
    
    //Generation of Histogram    
    for (int x = 0; x < windowx ; x++)
    {
        for (int y = 0; y < windowy ; y++)
        {
            histoarray[image2.GetRed(x,y)]++; 
        }
    }
    
    //Determination of Peaks
    for (int i = 0; i < 256; i++)
    {
        sort_histoarray[i] = histoarray[i];
    }
    qsort(sort_histoarray, 256, sizeof(int), compare);
    
    int peak1, peak2;
    int num_of_maxa = 1;
    int num_of_maxb = 1;
    
    for (int i = 0; i < 255; i++)
    {
        if (sort_histoarray[255] == sort_histoarray[i])
            num_of_maxa++;        
    }
    for (int i = 0; i < 254; i++)
    {
        if (sort_histoarray[254] == sort_histoarray[i])
            num_of_maxb++;        
    }
    
    if(num_of_maxa == 1 && num_of_maxb == 1)
    {
        for (int i = 0; i < 256; i++)
        {
            if(sort_histoarray[255] == histoarray[i])
                peak1 = i;
            if(sort_histoarray[254] == histoarray[i])
                peak2 = i;
        }
    }
    
    //Determination of Threshold
    int thr = (peak1 + peak2)/2;

    //Variables for Detection of Background and Foreground Luminosity    
    int black = 0;
    int white = 0;
    
    //Binarization Proper
    for (int x = 0; x < windowx ; x++)
    {
        for (int y = 0; y < windowy ; y++)
        {   
            if(image2.GetRed(x,y) <= thr)
            {
                image2.SetRGB(x, y, 0, 0, 0);
                black++;
            }   
            else if(image2.GetRed(x,y) > thr)
            {
                image2.SetRGB(x, y, 255, 255, 255);
                white++;
            }
        }
    }
    
    //Color Inversion
    if (black > white && isLetter == 0)
    {
        color_inversion(image2);
    } 
}

void segmentation(wxImage &image3)
{
    int windowx = image3.GetWidth();
    int windowy = image3.GetHeight();
    /*
     Variable for storing the x and y coordinate of oorners where 
     the row indicates the point number and the column indicates 
     whether it is the x coordinate y-coordinate
     
     [0][0],[0][1] --------- [1][0],[1][1]
             ...                   ....
     [3][0],[3][1] --------- [2][0],[2][1]
    */
    int edge[4][2]; 
    
    //Corner Detection
    for (int i = 0; i < windowx ; i++)
    {
        for (int j = 0; j < windowy ; j++)
        {
            if(image3.GetRed(i,j) == 0)
            {
                edge[0][0] = i; //x-minimum
                i = windowx;
                j = windowy;          
            }
        }
    }
    
    for (int i = windowx-1; i >= 0 ; i--)
    {
        for (int j = 0; j < windowy ; j++)
        {
            if(image3.GetRed(i,j) == 0)
            {   
                edge[1][0] = i; //x-maximum
                j = windowy;
                i = 0;  
            }
        }
    }
    
    for (int j = 0; j < windowy ; j++)
    {
        for (int i = 0; i < windowx ; i++)
        {
            if(image3.GetRed(i,j) == 0)
            {
                edge[0][1] = j; //y-minimum
                j = windowy;
                i = windowx;  
            }
        }
    }
    for (int j = windowy-1; j >= 0 ; j--)
    {
        for (int i = 0; i < windowx ; i++)
        {
            if(image3.GetRed(i,j) == 0)
            {
                edge[2][1] = j; //y-maximum
                j = 0;
                i = windowx; 
            }
        }
    }
    
    //Width and Height of the Character
    int n_width = edge[1][0] - edge[0][0] + 1;
    int n_height = edge[2][1] - edge[0][1] + 1;
    
    if(n_width == windowx && n_height == windowy)
    {
        return;
    }
    
    //Isolates the character given the corners of the character
    wxImage temp;
    temp.Create(n_width,n_height);
    int lums;
    
    for (int x = edge[0][0]; x < edge[0][0] + n_width; x++)
    {
        for (int y = edge[0][1]; y < edge[0][1] + n_height; y++)
        {
          lums = image3.GetRed(x,y);
          temp.SetRGB(x-edge[0][0], y-edge[0][1], lums, lums, lums);
        }
    }
    image3 = temp;
}

int segmentation_word(wxImage &image3, wxImage inputs [52] )
{
    //Level 2 Segmentation
    //Separates the letters on a per column basis
    int windowx = image3.GetWidth();
    int windowy = image3.GetHeight();
    int columnState[windowx]; //whether a column has black or not
    int sum;

    //Color Inversion
    color_inversion(image3);
    
    //Checks column whether it has a black pixel or not 
    for (int i = 0; i < windowx ; i++)
    {
        sum = 0;
        for (int j = 0; j < windowy ; j++)
        {
            sum += image3.GetRed(i,j);
        }
        
        if (sum == 0) //Black
            columnState[i] = 1; //Part of the Background
        else          //White
            columnState[i] = 0; //Part of the Character
    }
    
    //Reverse Color Inversion
    color_inversion(image3);
    
    //Segmentation Proper: The Concept
    //The principle is that a wxImage variable is created only when 
    //it is the first column with a black, and determines the column 
    //as to which it is all white. Given this, the variable for the 
    //letter is created with the determined width and length. The code 
    //then toggles between foreground and background, as to which it 
    //will only copy values, when it has been determined to be the 
    //foreground.
    int n_width;
    int h = 0;    //Counter for Number of Segmented Letters
    int val = 0;  //Variable for Luminance
    int l = 0;    //Counter for x-axis location in letter
    
    int init_state_fg = 0;
    int init_state_bg = 0;
    
    for (int i = 0; i < windowx; i++)
    {
        //For Columns with a Black Pixel
        if(columnState[i] == 0)
        {
            //If Initial Column, Initialize Values
            if (init_state_fg == 0)
            {
                //Computation for Width
                for(int k = i; k < windowx; k++)
                {
                    if(columnState[k] == 1 || k == windowx - 1)
                    {
                         n_width = k - i;
                         if (k == windowx - 1)
                            n_width++;
                         k = windowx;    
                    }
                }
                inputs[h].Create(n_width, windowy);
                init_state_fg = 1;
                init_state_bg = 0;
            }
            
            //Copy Proper
            for(int j = 0; j < windowy; j++)
            {
                val = image3.GetRed(i,j);
                inputs[h].SetRGB(l,j,val,val,val);
            }
            l++;
            
        }
        
        //For Columns with no Black Pixel
        //Initiates a Letter Switch
        else if(columnState[i] == 1 && init_state_bg == 0 )
        {
            inputs[h].Rescale(W/5, H/5);  
            init_state_fg = 0;
            init_state_bg = 1;
            h++;
            l = 0; 
        }
        
        //For Last Column
        if (i == windowx - 1)
        { 
            inputs[h].Rescale(W/5, H/5);
        }
    }
    //Sets the maximum value of used variables based on h
    return ++h;
}

//Supplemetary Code
int compare (const void * a, const void * b)
{
    return ( *(int*)a - *(int*)b );
}

void color_inversion (wxImage &image3)
{
    int windowx = image3.GetWidth();
    int windowy = image3.GetHeight();
    
    for (int i = 0; i < windowx ; i++)
    {
        for (int j = 0; j < windowy ; j++)
        {
            if(image3.GetRed(i,j) == 255) //If white, turn black
                image3.SetRGB(i, j, 0, 0, 0);
            else if(image3.GetRed(i,j) == 0) //If black, turn white
                image3.SetRGB(i, j, 255, 255, 255);
        }
    }
}
