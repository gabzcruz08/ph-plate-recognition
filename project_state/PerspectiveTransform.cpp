//---------------------------------------------------------------------------
//
// Name:        PerspectiveTransform.cpp
// Author:      John Benedict Du
// Created:     14/02/2018 2:07:33 PM
// Description: 
//
//---------------------------------------------------------------------------

#include <vector>
#include <fstream>
#include <iostream>
#include "PerspectiveTransform.h"

/*
 * The Perspective Transform Object contains the values of the matrix 
 * that would be used to transform coordinates of the given pixels.
 * The matrix is given as follows:
 *      | a11 a12 a13 |
 *      | a21 a22 a33 |
 *      | a31 a32 a33 |
 */
PerspectiveTransform::PerspectiveTransform(
                                double a11Given, double a21Given, double a31Given,
                                double a12Given, double a22Given, double a32Given,
                                double a13Given, double a23Given, double a33Given)
{
  a11 = a11Given;
  a12 = a12Given;
  a13 = a13Given;
  a21 = a21Given;
  a22 = a22Given;
  a23 = a23Given;
  a31 = a31Given;
  a32 = a32Given;
  a33 = a33Given;					 
}

/*
 * Source: George Wolberg-Digital Image Warping-IEEE (1990) pp 52-56
 *
 * This function computes for the matrix values of the PerspectiveTransform 
 * object. The inputs are the 4 points of the corners of the desired 
 * quadrilateral where (x0, y0), (x1, y1), (x2, y2) and (x3, y3) are the
 * top-left, top-right, bottom-right and bottom-left corners respectively.
 * 
 * Original four corners        Result
 *      (0, 0)                 (newX0, newY0)
 *      (1, 0)                 (newX1, newY1)
 *      (1, 1)                 (newX2, newY2)
 *      (0, 1)                 (newX3, newY3)
 *
 * According to our source the general reperesentation Perespective Transform is
 *                          | a11 a12 a13 |
 *  [x', y', w'] = [x, y, w]| a21 a22 a23 |
 *                          | a31 a32 a33 |
 * 
 * Since the images that we were using are 2 dimensional, only x and y are
 * variables while w would be 1 by default. Thus the values of the newX and
 * the newY could be computed as follows.
 *  newX = x'/w'
 *  newY = y'/w'
 *
 * For this function, the given would be 4 points or 8 values. Thus, the 
 * 9th coefficeint, a33, of the matrix could be normalized to 1 so that 
 * a minimum of 8 degrees of freedom could be acheived for the algorithm
 * without making it too complex.
 *
 * Leting a33 = 1 and w = 1 and solving to x' and y':
 *  newX = (a11 * x + a21 * y + a31)/(a13 * x + a23 * y)
 *  newY = (a12 * x + a22 * y + a32)/(a13 * x + a23 * y)
 *
 * Simplifing where i = 0, 1, 2, 3:
 *  newXi = a11 * xi + a21 * yi + a31 - a13 * xi * newXi - a23 * yi * newXi
 *  newYi = a12 * xi + a22 * yi + a32 - a13 * xi * newYi - a23 * yi * newYi
 *
 * Expanding where i = 0, 1, 2, 3:
 *  newXi =  a11 * (xi)            + a21 * (yi)              + a31 * (1)
 *        +  a12 * (0)             + a22 * (0)               + a32 * (0)
 *        +  a13 * (- xi * newXi)  + a23 * (- yi * newXi)    + a33 * (0)
 
 *  newYi =  a11 * (0)             + a21 * (0)               + a31 * (0)
 *        +  a12 * (xi)            + a22 * (yi)              + a32 * (1)
 *        +  a13 * (- xi * newYi)  + a23 * (- yi * newYi)    + a33 * (0)
 * 
 * This would result in having 8 equations with 16 unknown values namely the 
 * coordinates of the 4 points newX0, newY0, newX1, newY1, newX2, newY2, newX3 
 * and newY3 and the matrix values a11, a21, a31, a12, a22, a32, a13 and a23.
 * All 8 equations could be represented into the following matrix equation.
 *  |x0 y0 1 0  0  0 -x0*newX0 -y0*newX0||a11| = |newX0| 
 *  |x1 y1 1 0  0  0 -x1*newX1 -y1*newX1||a21| = |newX1|
 *  |x2 y2 1 0  0  0 -x2*newX2 -y2*newX2||a31| = |newX2|
 *  |x3 y3 1 0  0  0 -x3*newX3 -y3*newX3||a12| = |newX3|
 *  |0  0  0 x0 y0 1 -x0*newY0 -y0*newY0||a22| = |newY0|
 *  |0  0  0 x1 y1 1 -x1*newY1 -y1*newY1||a32| = |newY1|
 *  |0  0  0 x2 y2 1 -x2*newY2 -y2*newY2||a13| = |newY2|
 *  |0  0  0 x3 y3 1 -x3*newY3 -y3*newY3||a23| = |newY3|
 *
 * Since the image would originally start as a 
 * square, the following values can be used.
 *  (x0, y0) = (0, 0)
 *  (x1, y1) = (1, 0)
 *  (x2, y2) = (1, 1)
 *  (x3, y3) = (0, 1)
 *
 * Plugging in the values into the matrix:
 *  |0 0 1 0 0 0 0      0     ||a11| = |newX0| 
 *  |1 0 1 0 0 0 -newX1 0     ||a21| = |newX1|
 *  |1 1 1 0 0 0 -newX2 -newX2||a31| = |newX2|
 *  |0 1 1 0 0 0 0      -newX3||a12| = |newX3|
 *  |0 0 0 0 0 1 0      0     ||a22| = |newY0|
 *  |0 0 0 1 0 1 -newY1 0     ||a32| = |newY1|
 *  |0 0 0 1 1 1 -newY2 -newY2||a13| = |newY2|
 *  |0 0 0 0 1 1 0      -newY3||a23| = |newY3|
 *
 * This result into the following equations:
 *  a31 = newX0
 *  a11 + a31 - a13*newX1= newX1
 *  a11 + a21 + a31 - a13*newX2 - a23*newX2 = newX2
 *  a21 + a31 - a23*newX3 = newX3
 *  a32 = newY0
 *  a12 + a32 - a13*newY1 = newY1
 *  a12 + a22 + a32 - a13*newY2 - a23*newY2 = newY2
 *  a22 + a32 - a23*newY3 = newY3
 *
 * With the power of calculators, solving for the 
 * matrix values in terms of the 4 coordinates:
 *  a11 = newX1 - newX0 + a13*newX1
 *  a21 = newX3 - newX0 + a23*newX3
 *  a31 = newX0
 *  a12 = newY1 - newY0 + a13*newY1
 *  a22 = newY3 - newY0 + a23*newY3
 *  a32 = newY0
 *  a13 = ((newX0 - newX1 + newX2 - newX3)*(newY3 - newY2) 
 *          - (newY0 - newY1 + newY2 - newY3)*(newX3 - newX2))
 *          /((newX1 - newX2)*(newY3 - newY2) 
 *          - (newY1 - newY2)*(newX3 - newX2))
 *  a23 = ((newY0 - newY1 + newY2 - newY3)*(newX1 - newX2)
 *          - (newX0 - newX1 + newX2 - newX3)*(newY1 - newY2))
 *          /((newX1 - newX2)*(newY3 - newY2) 
 *          - (newY1 - newY2)*(newX3 - newX2))
 */
PerspectiveTransform PerspectiveTransform::squareToQuadrilateral(
                                                        double newX0, double newY0,
                                                        double newX1, double newY1,
                                                        double newX2, double newY2,
                                                        double newX3, double newY3)
{
  double dx3 = newX0 - newX1 + newX2 - newX3;
  double dy3 = newY0 - newY1 + newY2 - newY3;
  
  /*
   * If dx3 and dy3 results in a 0, the algorithm could 
   * be simplified to make the computation faster.
   */
  if(dx3 == 0.0 && dy3 == 0.0)
  {            
    a11 = newX1 - newX0;
    a21 = newX3 - newX0;
    a31 = newX0;
    a12 = newY1 - newY0;
    a22 = newY3 - newY0;
    a32 = newY0;
    a13 = 0.0;
    a23 = 0.0;
    a33 = 1.0;
  }
  /*
   * The code as follows implements the equations
   * as obtained in the description ablove.
   */
  else
  {
    double dx1 = newX1 - newX2;
    double dx2 = newX3 - newX2;
    double dy1 = newY1 - newY2;
    double dy2 = newY3 - newY2;
    double denominator = dx1 * dy2 - dx2 * dy1;
    a13 = (dx3 * dy2 - dx2 * dy3) / denominator;
    a23 = (dx1 * dy3 - dx3 * dy1) / denominator;
            
    a11 = newX1 - newX0 + a13 * newX1;
    a21 = newX3 - newX0 + a23 * newX3;
    a31 = newX0;
    a12 = newY1 - newY0 + a13 * newY1;
    a22 = newY3 - newY0 + a23 * newY3;
    a32 = newY0;
    a33 = 1.0;
  }
  /*
   * The results of the matrix values are then returned.
   */
  PerspectiveTransform result(
    a11, a21, a31,
    a12, a22, a32,
    a13, a23, 1.0
  );
        
  //Use this code to check if the matrix is generated correctly
  /*
  std::ofstream myfile ("result.txt");
  if (myfile.is_open())
  {
    myfile << a11 <<"    " << a21 << "   " << a31 << "\n";
    myfile << a12 <<"    " << a22 << "   " << a32 << "\n";
    myfile << a13 <<"    " << a23 << "   " << a33 << "\n";
    myfile.close();
  }
  else std::cout << "Unable to open file";
  */
        
  return result;
}

/*
 * This function transforms the points of a given pixel array using the
 * PerspectiveTransform object's matrix values. The returned object is a
 * new pixel array containing the transformed image.
 */
int** PerspectiveTransform::transformPoints(int** givenPixelArray, 
                                            int givenWidth, 
                                            int givenHeight)
{

  /*
   * Creates a new pixel array with the same 
   * width and height of the given pixel array.
   */
  int** imgPixelArray = new int*[givenHeight];
  for(int dynamicIndex = 0; dynamicIndex < givenHeight; ++dynamicIndex)
  {
    imgPixelArray[dynamicIndex] = new int[givenWidth];
  }

  /*
   * The new pixel array's default form is a blank white image.
   */
  for (int heightIndex=0; heightIndex<givenHeight; ++heightIndex)
  {
    for (int widthIndex=0; widthIndex<givenWidth; ++widthIndex)
    {
      imgPixelArray[heightIndex][widthIndex] = 1;
    }
  }
  
  /*
   * Each pixel in the given pixel array is 
   * then transformed using the matrix values.
   */
  for (int heightIndex=0; heightIndex<givenHeight; ++heightIndex)
  {
    for (int widthIndex=0; widthIndex<givenWidth; ++widthIndex)
    {
        
      /*
       * Given that the general reperesentation Perespective Transform
       *                          | a11 a12 a13 |
       *  [x', y', w'] = [x, y, w]| a21 a22 a23 |
       *                          | a31 a32 a33 |
       * where w = 1 for 2 dimensional coordinates
       *
       * Solving the matrix gives the following formulas for x' and y'
       *    newX = x'/w' = (a11 * x + a21 * y + a31)/(a13 * x + a23 * y + a33)
       *    newY = y'/w' = (a12 * x + a22 * y + a32)/(a13 * x + a23 * y + a33)
       * where newX and newY are the new coordinates of the pixel.
       */
      double x = widthIndex;
      double y = heightIndex;
      double denominator = a13 * x + a23 * y + a33;
      double doubleNewX = ((a11 * x + a21 * y + a31) / denominator);
      double doubleNewY = ((a12 * x + a22 * y + a32) / denominator);
      
      /*
       * Since the pixel array does not have any decimal coordinates,
       * the new coordinate is converted into an integer.
       */
      int newX = doubleNewX;
      int newY = doubleNewY;
         
      /*
       * Sometimes when the image is streched too much the image goes out
       * of bounds of the avaliable size of the image. Thus, new coordinates
       * that are out of bounds would be ignored.
       */
      int value = givenPixelArray[heightIndex][widthIndex];
      if(newX < givenWidth && newY < givenHeight && newX > 0 && newY > 0)
      {
        /*
         * The pixel value of the given pixel array is then 
         * stored in the new pixel array at its new coordinates.
         */
        imgPixelArray[newY][newX] = value;
      }
    }
  }    
        
  //Use this code to check if the matrix is generated correctly
  /*    
  std::ofstream myfile ("matrix.txt");
  if (myfile.is_open())
  {
    myfile << a11 <<"    " << a21 << "   " << a31 << "\n";
    myfile << a12 <<"    " << a22 << "   " << a32 << "\n";
    myfile << a13 <<"    " << a23 << "   " << a33 << "\n";
    for (int heightIndex=0; heightIndex<givenHeight; ++heightIndex)
    {
      for (int widthIndex=0; widthIndex<givenWidth; ++widthIndex)
      {
        myfile << imgPixelArray[heightIndex][widthIndex] << " ";
      }
      myfile << "\n";
    }    
    myfile.close();
  }
  else std::cout << "Unable to open file";
  */
  return imgPixelArray;
}

/*
 * This function returns a custom PerspectiveTransform object which adapts
 * to the many limitations of the finder pattern's results and the 
 * squareToQuadrilateral function. The inputs are the three points from the 
 * finder pattern algorithm with the 4th point being estimated based on the
 * three points.
 */
PerspectiveTransform PerspectiveTransform::reverseWarp(
                                                        double x0, double y0,
                                                        double x1, double y1,
                                                        double x3, double y3)
{
  /*
   * It has been observed that the squareToQuadrilateral function had the
   * limitation of only accepting rhombuses and parallelograms. Fortunately,
   * only 3 points are given from the finder pattern which allows the 4th point
   * could be generated by taking the differences of the 3 coordinates and
   * thus, allowing the 4 points to create a parallogram.
   */
  x0 *= 1.0;
  x1 *= 1.0;         
  x3 *= 1.0;
  double x2 = (x3 + (x1 - x0))*1.0;
        
  y0 *= 1.0;
  y1 *= 1.0;
  y3 *= 1.0;
  double y2 = (y3 + (y1 - y0))*1.0;

  double scaleY = 0;
  double scaleX = 0;
  
  /*
   * One thing to note was that the given points most likely won't be in a form
   * of a square. This makes using the squareToQuadrilateral function won't
   * work because what we want would be to make the Quadrilateral into a square
   * instead of the other way around. Thus, in order for it to work, instead of 
   * plugging the given 4 points in, we create the reverse of the shape formed 
   * by the given 4 points and plugin the points of the reversed shape instead.
   */  
  double x0p = 0; 
  double x1p = 0;
  double x2p = 0;
  double x3p = 0;
    
  double y0p = 0; 
  double y1p = 0;
  double y2p = 0;
  double y3p = 0;
  if(y2 > y3)
  {
    y0p = y0 + (y2 - y3);
    y1p = y0;
    y2p = y2 - (y1 - y0);
    y3p = y2;
    scaleY = y3p - y1p;
  }
  
  else
  {
    y0p = y1;
    y1p = y1 + (y3 - y2);
    y2p = y3;
    y3p = y3 - (y0 - y1);
    scaleY = y2p - y0p;
  }
    
  if(x1 > x2)
  {
    x0p = x3; 
    x1p = x1 - (x0 - x3);
    x2p = x1;
    x3p = x3 + (x1 - x2);
    scaleX = x1 - x3;
  }
  
  else
  {
    x0p = x0 + (x2 - x1); 
    x1p = x2;
    x2p = x2 - (x3 - x0);
    x3p = x0;
    scaleX = x2 - x0;
  }

  /*
   * Another limitation of the squareToQuadrilateral would be that it assumes
   * that the image is a 1x1 square image. Since the 4 points would most likely
   * for a quadrilateral bigger than a 1x1 square, the point would then have to 
   * be scaled such that the matrix could be generated properly. The scale was
   * computed based on the length from corner to corner of the points.
   */
 
  x0p /= scaleX;
  x1p /= scaleX;
  x2p /= scaleX;
  x3p /= scaleX;

  y0p /= scaleY;
  y1p /= scaleY;
  y2p /= scaleY;
  y3p /= scaleY;        

  //Use this code to check if the coordinates were generated correctly
  /*
  std::ofstream myfile ("center.txt");
  if (myfile.is_open())
  {
      myfile << x0p << "\t" << y0p << "\n";
      myfile << x1p << "\t" << y1p << "\n";
      myfile << x2p << "\t" << y2p << "\n";
      myfile << x3p << "\t" << y3p << "\n";
      myfile.close();
  }
  else std::cout << "Unable to open file";
  */
  
  /*
   * The results of the matrix values are then returned.
   */
  PerspectiveTransform result = squareToQuadrilateral(
    x0p, y0p,
    x1p, y1p,
    x2p, y2p,
    x3p, y3p
  );                            
  
  return result;                                                                          
}
