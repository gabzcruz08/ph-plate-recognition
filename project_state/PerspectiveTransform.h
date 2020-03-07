//---------------------------------------------------------------------------
//
// Name:        PerspectiveTransform.h
// Author:      John Benedict Du
// Created:     14/02/2018 2:30:24 PM
// Description: 
//
//---------------------------------------------------------------------------

#include <vector>

class PerspectiveTransform{
  private:
    double a11, a12, a13;
    double a21, a22, a23;
    double a31, a32, a33;
	
  public:
    PerspectiveTransform(
      double a11Given, double a21Given, double a31Given,
      double a12Given, double a22Given, double a32Given,
      double a13Given, double a23Given, double a33Given
    );
    
    PerspectiveTransform squareToQuadrilateral(
      double newX0, double newY0,
      double newX1, double newY1,
      double newX2, double newY2,
      double newX3, double newY3
    );
    
    PerspectiveTransform reverseWarp(
      double x0, double y0,
      double x1, double y1,
      double x3, double y3
    );
    
    int** transformPoints(int** givenPixelArray, int givenWidth, int givenHeight);      
};
