#include "IntercoDataMps.h"

/*!
 * \brief Initialize Candidates::MPS_LIST static attribute from file
 *
 * \param mps_filePath_p : file name to use to read mps list
 * \return void
 */
void initMPSList(std::string const & mps_filePath_p);

/*!
 * \fn void initializedCandidates(string rootPath, Candidates & candidates)
 * \brief Initialize the candidates structure with input data located in the directory given in argument
 *
 * \param rootPath :  String corresponding to the path where are located input data
 * \param candidates : Structure which is initialized
 * \return void
 */
void initializedCandidates(std::string const & rootPath, Candidates & candidates);
