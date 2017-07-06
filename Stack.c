//
// Created by Leozki on 2017/7/6.
//

#include "Stack.h"
#include "Script.h"


void PushFrame(int iThreadIndex, int iSize) {
    g_Scripts[iThreadIndex].Stack.iTopIndex += iSize;
    g_Scripts[iThreadIndex].Stack.iFrameIndex = g_Scripts[g_iCurrThread].Stack.iTopIndex;
}

