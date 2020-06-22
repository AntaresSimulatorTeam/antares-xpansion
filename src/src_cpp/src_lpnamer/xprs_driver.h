#pragma once

#include <xprs.h>
#include "common_lpnamer.h"

void XPRS_CC optimizermsg(XPRSprob prob, void* data, const char *sMsg, int nLen, int nMsgLvl);
void errormsg(const char *sSubName, int nLineNo, int nErrorCode);