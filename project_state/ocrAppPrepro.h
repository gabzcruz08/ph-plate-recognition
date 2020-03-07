/***************************************************************
 * Name:      ocrAppPrepro.h
 * Purpose:   Defines Pre-Processing, Grayscale, Binarization, 
 *            Seggmentation, and Segmentation of Word
 * Author:    
 * Created:   2017-10-07
 * Copyright: 
 * License:
 **************************************************************/
//Preprocessing Functions
//Applies Grayscale Filter
void grayscale(wxImage &image); 
//Otsu's Binarization
void threshold(wxImage &image, bool isLetter); 
//Segments the Image
void segmentation(wxImage &image); 
//Segments the Words and Returns Number of Letters
int segmentation_word(wxImage &image, wxImage inputs [52] ); 

//Supplementary Code
//Used for Comparison
int compare (const void * a, const void * b);
