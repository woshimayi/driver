/*
 * @*************************************: 
 * @FilePath: /user/C/string/ball_dynamic.c
 * @version: 
 * @Author: dof
 * @Date: 2024-02-08 14:45:35
 * @LastEditors: dof
 * @LastEditTime: 2024-03-05 14:23:59
 * @Descripttion: 
 * @**************************************: 
 */
#include<graphics.h>
#include<math.h>
#include<stdio.h>
#include<conio.h>

#define PI acos(-1.0)
void HideSphere(float R,int alfa,int beta,int HideFlag);

int main()
{
  initgraph(640, 480);
  BeginBatchDraw(); 
  while (true)
  {
    for (int i = 0; i < 180; i++)
    {
      HideSphere(200, 45 + i, 30 + i, 1);
      Sleep(20);
      
      FlushBatchDraw();
      cleardevice();
    }
  }
  EndBatchDraw();
  _getch();
}

void HideSphere(float R, int alfa, int beta, int HideFlag)
{
  int i, j, k;
  float x[4], y[4], z[4], x1[4], y1[4], z1[4], sx[4], sy[4];
  double a1, a2, b1, b2, c, d, xn, yn, zn, vn;
  c = alfa * PI / 180.0;
  d = beta * PI / 180.0;
  for (j = 0; j < 180; j = j + 5)
  {
    a1 = j * PI / 180.0;
    a2 = (j + 5) * PI / 180.0;
    for (i = 0; i < 360; i = i + 5)
    {
      b1 = i * PI / 180.0;
      b2 = (i + 5) * PI / 180.0;
      x[0] = R * sin(a1) * cos(b1); y[0] = R * sin(a1) * sin(b1); z[0] = R * cos(a1);
      x[1] = R * sin(a2) * cos(b1); y[1] = R * sin(a2) * sin(b1); z[1] = R * cos(a2);
      x[2] = R * sin(a2) * cos(b2); y[2] = R * sin(a2) * sin(b2); z[2] = R * cos(a2);
      x[3] = R * sin(a1) * cos(b2); y[3] = R * sin(a1) * sin(b2); z[3] = R * cos(a1);
      for (k = 0; k < 4; k++)
      {
        x1[k] = x[k] * cos(c) - y[k] * sin(c);
        y1[k] = x[k] * sin(c) * cos(d) + y[k] * cos(c) * sin(d) + z[k] * sin(d);
        z1[k] = -x[k] * sin(c) * sin(d) - y[k] * cos(c) * sin(d) + z[k] * cos(d);
        sx[k] = 320 - x1[k];
        sy[k] = 240 - z1[k];
      }
      xn = (y1[2] - y1[0]) * (z1[3] - z1[1]) - (y1[3] - y1[1]) * (z1[2] - z1[0]);
      yn = -(x1[2] - x1[0]) * (z1[3] - z1[1]) + (x1[3] - x1[1]) * (z1[2] - z1[0]);
      zn = (x1[2] - x1[0]) * (y1[3] - y1[1]) - (x1[3] - x1[1]) * (y1[2] - y1[0]);
      vn = sqrt(xn * xn + yn * yn + zn * zn);
      xn = xn / vn;
      yn = yn / vn;
      zn = zn / vn;
      if (!HideFlag || yn >= 0.0)
      {
        moveto(sx[0],sy[0]);
        lineto(sx[1],sy[1]);
        lineto(sx[2],sy[2]);
        lineto(sx[3],sy[3]);
        lineto(sx[0],sy[0]);
      }
    }
  }
}
