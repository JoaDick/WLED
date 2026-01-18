/**
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#include "wled.h"
#include "FxExtras2D.h"

//--------------------------------------------------------------------------------------------------

void box(PxMatrix &pxm, APoint p1, APoint p2, PxColor color)
{
  {
    auto row = pxm.row(p1.y);
    line(row, p1.x, p2.x, color);
  }
  {
    auto row = pxm.row(p2.y);
    line(row, p1.x, p2.x, color);
  }
  {
    auto col = pxm.col(p1.x);
    line(col, p1.y, p2.y, color);
  }
  {
    auto col = pxm.col(p2.x);
    line(col, p1.y, p2.y, color);
  }
}

//--------------------------------------------------------------------------------------------------
