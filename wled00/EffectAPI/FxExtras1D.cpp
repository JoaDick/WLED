/**
 * (c) 2026 Joachim Dick
 * Licensed under the EUPL v. 1.2 or later
 */

#include "wled.h"
#include "FxExtras1D.h"

//--------------------------------------------------------------------------------------------------

void colorLine(PxArray &pxa, AIndex firstPos, AIndex lastPos, ColorSource &colorSource)
{
  // We must NOT constrain! Off-strip positions have to be treated like regular positions.
  AIndex colorIndex = 0;
  while (firstPos < lastPos)
    pxa.setColor(firstPos++, colorSource.get(colorIndex++));
  while (firstPos >= lastPos)
    pxa.setColor(firstPos--, colorSource.get(colorIndex++));
}

//--------------------------------------------------------------------------------------------------
