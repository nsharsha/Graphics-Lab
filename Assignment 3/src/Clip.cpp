#include <windows.h>
#include "Line.h"
#include "ClipUtil.h"
#include <algorithm>

using namespace std;

int code(double x, double y)
{
  return ( ((x < gDrawData.clipMin.x) << 3) +
           ((x > gDrawData.clipMax.x) << 2) +
           ((y < gDrawData.clipMin.y) << 1) +
            (y > gDrawData.clipMax.y));
}

//void scLineClip(POINT start, POINT end)
//{
//  bool bVerticalLine;
//  float m;
//  double x1Clip, y1Clip, x2Clip, y2Clip;
//  int c1,c2;  
//  POINT clipStart, clipEnd;
//
//  // groundwork before drawing contiguous line segments for clipping
//  setupLineSegmentDrawing(gDrawData.hdcMem, start, end);
//  calculateSlope(start, end, bVerticalLine, m);
//
//  x1Clip = start.x;
//  y1Clip = start.y;
//  x2Clip = end.x;
//  y2Clip = end.y;
//
//  c1 = code(x1Clip, y1Clip); /* region code of start point*/ 
//  c2 = code(x2Clip, y2Clip); /* region code of end point*/
//
//  while (c1 | c2)  
//  {
//    /* two points are not inside the rectangle */
//    if(c1 & c2) 
//    {
//      // line is completely outside, erase entire line
//      drawNextLineSegment(end, CLR_BG);
//      return;
//    }
//    if(c1) 
//    {
//      if(c1&8)
//      {
//        y1Clip += m * (gDrawData.clipMin.x - x1Clip);
//        x1Clip = gDrawData.clipMin.x;                 
//      }
//      else if(c1&4)
//      {
//        y1Clip += m * (gDrawData.clipMax.x - x1Clip);
//        x1Clip = gDrawData.clipMax.x;           
//      }
//      else if(c1&2)
//      {
//        if (!bVerticalLine)
//          x1Clip += (gDrawData.clipMin.y - y1Clip)/m;
//        y1Clip = gDrawData.clipMin.y;           
//      }
//      else if(c1&1)
//      {
//        if (!bVerticalLine)
//          x1Clip += (gDrawData.clipMax.y - y1Clip)/m;
//        y1Clip = gDrawData.clipMax.y;           
//      }
//      c1 = code(x1Clip, y1Clip);
//    }//End OF if(c1)
//    else // c1 is false, so c2 must be true
//    {
//      if(c2&8)
//      {
//        y2Clip += m * (gDrawData.clipMin.x - x2Clip);
//        x2Clip = gDrawData.clipMin.x;                
//      }
//      else if(c2&4)
//      {
//        y2Clip += m * (gDrawData.clipMax.x - x2Clip);
//        x2Clip = gDrawData.clipMax.x;
//      }
//      else if(c2&2)
//      {
//        if (!bVerticalLine)
//          x2Clip += (gDrawData.clipMin.y - y2Clip)/m;
//        y2Clip = gDrawData.clipMin.y;           
//      }
//      else if(c2&1) 
//      {
//        if (!bVerticalLine)
//          x2Clip += (gDrawData.clipMax.y - y2Clip)/m;
//        y2Clip = gDrawData.clipMax.y;           
//      }
//      c2 = code(x2Clip, y2Clip);
//    }//End OF else
//  }//End of while
//  clipStart.x = (long) (x1Clip);
//  clipStart.y = (long) (y1Clip);
//  clipEnd.x = (long) (x2Clip);
//  clipEnd.y = (long) (y2Clip);
//
//  drawNextLineSegment(clipStart, CLR_BG);
//  drawNextLineSegment(clipEnd, CLR_LINE);
//  drawNextLineSegment(end, CLR_BG);
//  performCorrectionAtClipPts(gDrawData.hdcMem, clipStart,
//                             CLR_LINE, CLR_BG);
//  performCorrectionAtClipPts(gDrawData.hdcMem, clipEnd,
//                             CLR_LINE, CLR_BG); 
//}

//function for calculating dotproduct of two vectors
double dotProduct(POINT p1, POINT p2)
{
    return p1.x*p2.x + p1.y*p2.y;
}

POINT getOutsideNormal(int i)
{  POINT ret;
     if (i==0){   /*left clip line*/
                 ret.x = -1;
                 ret.y=0;  
               } 
     else if(i==1){
          /*right clip line*/
           ret.x = 1;
           ret.y = 0;
          }  
     else if(i==2){
          /*bottom clip line*/
           ret.x = 0;
           ret.y = -1;
          } 
     else if(i==3){
          /*top clip line*/
           ret.x = 0;
           ret.y = 1;
          }                   
    
    return ret;
}

POINT getPoint(int i)
{
      POINT P;
      if(i==0){
               P.x = gDrawData.clipMin.x;
               P.y = (gDrawData.clipMin.y + gDrawData.clipMax.y)/2;
      }
      else if(i==1){
               P.x = gDrawData.clipMax.x;
               P.y = (gDrawData.clipMin.y + gDrawData.clipMax.y)/2;
      }
      else if(i==2){
              P.x=  (gDrawData.clipMin.x + gDrawData.clipMax.x)/2;
              P.y = gDrawData.clipMin.y;
      }
      else if(i==3){
              P.x=  (gDrawData.clipMin.x + gDrawData.clipMax.x)/2;
              P.y = gDrawData.clipMax.y;
      }                  
      return P;     
}

void cbLineClip(POINT start, POINT end)
{
   // groundwork before drawing contiguous line segments for clipping
   setupLineSegmentDrawing(gDrawData.hdcMem, start, end);
   double x1Clip, y1Clip, x2Clip, y2Clip; //endpoints of the line to keep after clipping
   POINT clipStart, clipEnd;
   double te=0,tl=1;  //parameters for the intersection points
   POINT d;            //P1-P0  
   d.x = end.x - start.x;
   d.y = end.y - start.y;
   POINT N[4]; //array of normal vector to left,right,bottom and top clip edge respectively
   POINT P[4]; //array of apoint on left,right,bottom and top clip edge respectively
   int l[4],m[4],flag=1;  //denominator and numerator for calculating t parameter 

   for(int i=0;i<4;i++)
   {
	    POINT Q;
        N[i]=getOutsideNormal(i);
        P[i] = getPoint(i);
        Q.x = (int)start.x - (int)P[i].x;
        Q.y = (int)start.y - (int)P[i].y;
        l[i] = dotProduct(N[i],d);
        m[i] = -dotProduct(N[i],Q);
   }
   if(l[0]==0){
               //line is parallel to the y axis
             if( m[0]*m[1]<0)
             {
                 //line is outside the clip window
                  drawNextLineSegment(end, CLR_BG);
                  return;
             }
             else 
             {     // when part of line lies on / inside the clip wintdow
                  clipStart.x = (long) (start.x);
                  clipStart.y = (long) (max( min(start.y,end.y),gDrawData.clipMin.y));
                  clipEnd.x = (long) (start.x);
                  clipEnd.y = (long) (min(max(start.y,end.y),gDrawData.clipMax.y));
                  drawLineSegment(gDrawData.hdcMem,start,end, CLR_BG);
                  drawLineSegment(gDrawData.hdcMem,clipStart,clipEnd, RGB(0,0,0));
                  return; 
              }  
               
   }     
   if(l[2]==0){
               //line is parallel to the x axis
             if( m[2]*m[3]<0)
             { 
                 //line is outside the clip window
                  drawNextLineSegment(end, CLR_BG);
                  return;
             } 
              else 
             {    // when part of line lies on/inside the clip wintdow
                     
                  clipStart.x = (long) (max(min(start.x,end.x),gDrawData.clipMin.x));
                  clipStart.y = (long) (start.y);
                  clipEnd.x = (long) (min(max(start.x,end.x),gDrawData.clipMax.x));
                  clipEnd.y = (long) (start.y);
                  drawLineSegment(gDrawData.hdcMem,start,end, CLR_BG); 
                  drawLineSegment(gDrawData.hdcMem,clipStart,clipEnd, RGB(0,0,0));
                  return;
              } 

        }          
   if( l[0]!=0 && l[2]!=0)
   {               
                   
       for(int i=0;i<4;i++)
       {           
           if(l[i]<0 && flag)
           {
               //potentially entering  
               double t= (double)m[i]/(double)l[i];
               if(t > tl)
                 flag=0;
               else
               if( t>te) 
                 te = t;
                    
                       
           }
           else if(l[i]>0 && flag)
           {
               //potentially leaving
               double t= (double)m[i]/(double)l[i];
               if(t < te)
                 flag=0;
               else
               if (t < tl) 
                 tl= t;
           }
       }
     }
           
    if( te-tl >= 0 || flag==0 ) {
          
           drawLineSegment(gDrawData.hdcMem,start,end, CLR_BG);
           return;
      
    }
    else
    {    
         /*x and y coordintes of the portion to keep*/
         x1Clip = start.x + (end.x - start.x)*te;
         y1Clip = start.y + (end.y - start.y)*te;
         x2Clip = start.x + (end.x - start.x)*tl;
         y2Clip = start.y + (end.y - start.y)*tl;
        
    
          clipStart.x = (long) (x1Clip);
          clipStart.y = (long) (y1Clip);
          clipEnd.x = (long) (x2Clip);
          clipEnd.y = (long) (y2Clip);
         // drawLineSegment(gDrawData.hdcMem,start,end, CLR_BG);
          drawNextLineSegment(clipStart, CLR_BG);
           drawNextLineSegment(clipEnd, RGB(0,0,0));
            drawNextLineSegment(end, CLR_BG);
           //drawLineSegment(gDrawData.hdcMem,start,clipStart, CLR_BG);
          //drawLineSegment(gDrawData.hdcMem,clipStart,clipEnd, RGB(0,0,0));
         // drawLineSegment(gDrawData.hdcMem,clipEnd,end ,CLR_BG);  
         performCorrectionAtClipPts(gDrawData.hdcMem, clipStart,
                                     RGB(0,0,0), CLR_BG);
          performCorrectionAtClipPts(gDrawData.hdcMem, clipEnd,
                                 RGB(0,0,0), CLR_BG); 
         
    }                            
}
void clip(HWND hwnd)
{
  cbLineClip(gDrawData.lineEndPts[0], gDrawData.lineEndPts[1]);   /* Cyrus Beck */
//  scLineClip(gDrawData.lineEndPts[0], gDrawData.lineEndPts[1]);	/* Sohen Cutherland*/
  reDraw(hwnd);
  setDrawMode(CLIPPED_MODE, hwnd);
}
