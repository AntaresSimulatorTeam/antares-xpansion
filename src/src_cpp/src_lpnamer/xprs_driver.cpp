#include "xprs_driver.h"



/**********************************************************************************\
* Name:         optimizermsg                                                           *
* Purpose:      Display Optimizer error messages and warnings.                         *
* Arguments:    const char *sMsg       Message string                                  *
*               int nLen               Message length                                  *
*               int nMsgLvl            Message type                                    *
* Return Value: None                                                                   *
\**********************************************************************************/
void XPRS_CC optimizermsg(XPRSprob prob, void* data, const char *sMsg, int nLen, int nMsgLvl)
{
	switch (nMsgLvl) {

		/* Print Optimizer error messages and warnings */
	case 4:       /* error */
	case 3:       /* warning */
	case 2:       /* dialogue */
	case 1:       /* information */
		printf("%*s\n", nLen, sMsg);
		break;
		/* Exit and flush buffers */
	default:
		fflush(NULL);
		break;
	}
}


/**************************************************************************************\
* Name:         errormsg                                                               *
* Purpose:      Display error information about failed subroutines.                    *
* Arguments:    const char *sSubName   Subroutine name                                 *
*               int nLineNo            Line number                                     *
*               int nErrCode           Error code                                      *
* Return Value: None                                                                   *
\**************************************************************************************/
void errormsg(XPRSprob & probg, const char *sSubName, int nLineNo, int nErrCode)
{
	int nErrNo;   /* Optimizer error number */
				  /* Print error message */
	printf("The subroutine %s has failed on line %d\n", sSubName, nLineNo);

	/* Append the error code, if it exists */
	if (nErrCode != -1) printf("with error code %d.\n\n", nErrCode);

	/* Append Optimizer error number, if available */
	if (nErrCode == 32) {
		XPRSgetintattrib(probg, XPRS_ERRORCODE, &nErrNo);
		printf("The Optimizer error number is: %d.\n\n", nErrNo);
	}

	/* Free memory, close files and exit */
	XPRSdestroyprob(probg);
	XPRSfree();
	std::exit(nErrCode);
}



