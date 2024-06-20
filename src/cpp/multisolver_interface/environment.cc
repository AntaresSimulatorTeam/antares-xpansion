// This this file is an adaptation of OR-Tools for Antares-Xpansion needs
//  see orginal at https://github.com/google/or-tools
// Copyright 2010-2021 Google LLC
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "multisolver_interface/environment.h"

#include <filesystem>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

namespace Solver {
class DynamicLibrary;
}
namespace LoadXpress {

#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)

// This was generated with the parse_header_xpress.py script.
// See the comment at the top of the script.

std::string StringJoin(const std::vector<std::string>& vec) {
  std::string ret;
  for (const auto& str : vec) ret += str + "', '";
  return ret;
}
// This is the 'define' section.
std::function<int(const char* path)> XPRSinit = nullptr;
std::function<int(XPRSprob dest, XPRSprob src, const char* name)> XPRScopyprob =
    nullptr;
std::function<int(XPRSprob prob, const char* filename, const char* flags)>
    XPRSwritebasis = nullptr;
std::function<int(XPRSprob prob, const char* filename, const char* flags)>
    XPRSreadprob = nullptr;
std::function<int(XPRSprob prob, const char* filename, const char* flags)>
    XPRSreadbasis = nullptr;
std::function<int(XPRSprob prob, int start[], int colind[], double colcoef[],
                  int maxcoefs, int* p_ncoefs, int first, int last)>
    XPRSgetrows = nullptr;
std::function<int(XPRSprob prob, int type, const char* name, int* p_index)>
    XPRSgetindex = nullptr;
std::function<int(XPRSprob prob, int type, char names[], int first, int last)>
    XPRSgetnames = nullptr;
std::function<int(XPRSprob prob, int type, const char names[], int first,
                  int last)>
    XPRSaddnames = nullptr;
std::function<int(XPRSprob prob, const char* flags)> XPRSlpoptimize = nullptr;
std::function<int(XPRSprob prob, const char* flags)> XPRSmipoptimize = nullptr;
std::function<int(void)> XPRSfree = nullptr;
std::function<int(XPRSprob* p_prob)> XPRScreateprob = nullptr;
std::function<int(XPRSprob prob, const char* probname, int ncols, int nrows,
                  const char rowtype[], const double rhs[], const double rng[],
                  const double objcoef[], const int start[], const int collen[],
                  const int rowind[], const double rowcoef[], const double lb[],
                  const double ub[])>
    XPRSloadlp = nullptr;
std::function<int(XPRSprob prob)> XPRSdestroyprob = nullptr;

std::function<int(XPRSprob prob, const char* filename, const char* flags)>
    XPRSwriteprob = nullptr;
std::function<int(XPRSprob prob, int control, int* p_value)> XPRSgetintcontrol =
    nullptr;
std::function<int(XPRSprob prob, double objcoef[], int first, int last)>
    XPRSgetobj = nullptr;
std::function<int(XPRSprob prob, char rowtype[], int first, int last)>
    XPRSgetrowtype = nullptr;
std::function<int(XPRSprob prob, double rhs[], int first, int last)>
    XPRSgetrhs = nullptr;
std::function<int(XPRSprob prob, double rng[], int first, int last)>
    XPRSgetrhsrange = nullptr;
std::function<int(XPRSprob prob, char coltype[], int first, int last)>
    XPRSgetcoltype = nullptr;
std::function<int(XPRSprob prob, double lb[], int first, int last)> XPRSgetlb =
    nullptr;
std::function<int(XPRSprob prob, double ub[], int first, int last)> XPRSgetub =
    nullptr;
std::function<int(XPRSprob prob, int nrows, const int rowind[])> XPRSdelrows =
    nullptr;
std::function<int(XPRSprob prob, int nrows, int ncoefs, const char rowtype[],
                  const double rhs[], const double rng[], const int start[],
                  const int colind[], const double rowcoef[])>
    XPRSaddrows = nullptr;
std::function<int(XPRSprob prob, int ncols, int ncoefs, const double objcoef[],
                  const int start[], const int rowind[], const double rowcoef[],
                  const double lb[], const double ub[])>
    XPRSaddcols = nullptr;

std::function<int(XPRSprob prob, int ncols, const int colind[],
                  const double objcoef[])>
    XPRSchgobj = nullptr;
std::function<int(XPRSprob prob, int objsense)> XPRSchgobjsense = nullptr;
std::function<int(XPRSprob prob, int nbounds, const int colind[],
                  const char bndtype[], const double bndval[])>
    XPRSchgbounds = nullptr;
std::function<int(XPRSprob prob, int ncols, const int colind[],
                  const char coltype[])>
    XPRSchgcoltype = nullptr;
std::function<int(XPRSprob prob, int nrows, const int rowind[],
                  const double rhs[])>
    XPRSchgrhs = nullptr;

std::function<int(XPRSprob prob, int row, int col, double coef)> XPRSchgcoef =
    nullptr;
std::function<int(XPRSprob prob, int rowstat[], int colstat[])> XPRSgetbasis =
    nullptr;
std::function<int(XPRSprob prob, int attrib, double* p_value)>
    XPRSgetdblattrib = nullptr;
std::function<int(XPRSprob prob, double x[], double slack[], double duals[],
                  double djs[])>
    XPRSgetlpsol = nullptr;
std::function<int(XPRSprob prob, double x[], double slack[])> XPRSgetmipsol =
    nullptr;
std::function<int(
    XPRSprob prob,
    void(XPRS_CC* f_message)(XPRSprob cbprob, void* cbdata, const char* msg,
                             int msglen, int msgtype),
    void* p)>
    XPRSsetcbmessage = nullptr;
std::function<int(XPRSprob prob, int control, int value)> XPRSsetintcontrol =
    nullptr;
std::function<int(XPRSprob prob, int control, double value)> XPRSsetdblcontrol =
    nullptr;
std::function<int(char* banner)> XPRSgetbanner = nullptr;

std::function<int(char* buffer, int maxbytes)> XPRSgetlicerrmsg = nullptr;
std::function<int(int* p_i, char* p_c)> XPRSlicense = nullptr;
std::function<int(char* version)> XPRSgetversion = nullptr;
std::function<int(XPRSprob prob, int attrib, int* p_value)> XPRSgetintattrib =
    nullptr;

XpressLoader::XpressLoader(std::shared_ptr<ILoggerXpansion> logger)
    : logger_(std::move(logger)) {}

bool XpressLoader::LoadXpressFunctions(Solver::DynamicLibrary* xpress_dynamic_library) {
  // This was generated with the parse_header_xpress.py script.
  // See the comment at the top of the script.

  // This is the 'assign' section.
  xpress_dynamic_library->GetFunction(&XPRSinit, "XPRSinit");
  xpress_dynamic_library->GetFunction(&XPRScopyprob, "XPRScopyprob");
  xpress_dynamic_library->GetFunction(&XPRSwritebasis, "XPRSwritebasis");
  xpress_dynamic_library->GetFunction(&XPRSreadprob, "XPRSreadprob");
  xpress_dynamic_library->GetFunction(&XPRSreadbasis, "XPRSreadbasis");
  xpress_dynamic_library->GetFunction(&XPRSgetrows, "XPRSgetrows");
  xpress_dynamic_library->GetFunction(&XPRSgetindex, "XPRSgetindex");
  xpress_dynamic_library->GetFunction(&XPRSgetnames, "XPRSgetnames");
  xpress_dynamic_library->GetFunction(&XPRSaddnames, "XPRSaddnames");
  xpress_dynamic_library->GetFunction(&XPRSlpoptimize, "XPRSlpoptimize");
  xpress_dynamic_library->GetFunction(&XPRSmipoptimize, "XPRSmipoptimize");
  xpress_dynamic_library->GetFunction(&XPRSfree, "XPRSfree");
  xpress_dynamic_library->GetFunction(&XPRSloadlp, "XPRSloadlp");
  xpress_dynamic_library->GetFunction(&XPRScreateprob, "XPRScreateprob");
  xpress_dynamic_library->GetFunction(&XPRSdestroyprob, "XPRSdestroyprob");
  xpress_dynamic_library->GetFunction(&XPRSwriteprob, "XPRSwriteprob");
  xpress_dynamic_library->GetFunction(&XPRSgetintcontrol, "XPRSgetintcontrol");
  xpress_dynamic_library->GetFunction(&XPRSgetintattrib, "XPRSgetintattrib");
  xpress_dynamic_library->GetFunction(&XPRSgetobj, "XPRSgetobj");
  xpress_dynamic_library->GetFunction(&XPRSgetrowtype, "XPRSgetrowtype");
  xpress_dynamic_library->GetFunction(&XPRSgetrhsrange, "XPRSgetrhsrange");
  xpress_dynamic_library->GetFunction(&XPRSgetrhs, "XPRSgetrhs");
  xpress_dynamic_library->GetFunction(&XPRSgetcoltype, "XPRSgetcoltype");
  xpress_dynamic_library->GetFunction(&XPRSgetlb, "XPRSgetlb");
  xpress_dynamic_library->GetFunction(&XPRSgetub, "XPRSgetub");
  xpress_dynamic_library->GetFunction(&XPRSdelrows, "XPRSdelrows");
  xpress_dynamic_library->GetFunction(&XPRSaddrows, "XPRSaddrows");
  xpress_dynamic_library->GetFunction(&XPRSchgobj, "XPRSchgobj");
  xpress_dynamic_library->GetFunction(&XPRSaddcols, "XPRSaddcols");
  xpress_dynamic_library->GetFunction(&XPRSchgobjsense, "XPRSchgobjsense");
  xpress_dynamic_library->GetFunction(&XPRSchgbounds, "XPRSchgbounds");
  xpress_dynamic_library->GetFunction(&XPRSchgcoltype, "XPRSchgcoltype");
  xpress_dynamic_library->GetFunction(&XPRSchgrhs, "XPRSchgrhs");
  xpress_dynamic_library->GetFunction(&XPRSchgcoef, "XPRSchgcoef");
  xpress_dynamic_library->GetFunction(&XPRSgetbasis, "XPRSgetbasis");
  xpress_dynamic_library->GetFunction(&XPRSgetlpsol, "XPRSgetlpsol");
  xpress_dynamic_library->GetFunction(&XPRSgetdblattrib, "XPRSgetdblattrib");
  xpress_dynamic_library->GetFunction(&XPRSgetmipsol, "XPRSgetmipsol");
  xpress_dynamic_library->GetFunction(&XPRSsetcbmessage, "XPRSsetcbmessage");
  xpress_dynamic_library->GetFunction(&XPRSsetintcontrol, "XPRSsetintcontrol");
  xpress_dynamic_library->GetFunction(&XPRSsetdblcontrol, "XPRSsetdblcontrol");
  xpress_dynamic_library->GetFunction(&XPRSgetbanner, "XPRSgetbanner");
  xpress_dynamic_library->GetFunction(&XPRSgetlicerrmsg, "XPRSgetlicerrmsg");
  xpress_dynamic_library->GetFunction(&XPRSlicense, "XPRSlicense");
  xpress_dynamic_library->GetFunction(&XPRSgetversion, "XPRSgetversion");

  auto notFound = xpress_dynamic_library->FunctionsNotFound();
  if (!notFound.empty()) {
    std::string msg(
        "Could not find the following functions (list may not be "
        "exhaustive). [" +
        StringJoin(notFound) +
        "]. Please make sure that your XPRESS install is "
        "up-to-date (>= 8.13.0).\n");
    logger_->display_message(msg);
    return false;
  }
  return true;
}

void XpressLoader::printXpressBanner() {
  char banner[XPRS_MAXBANNERLENGTH];
  XPRSgetbanner(banner);
  std::ostringstream msg;

  msg << "Xpress banner :\n" << banner << "\n";
  logger_->display_message(msg);
}

std::string XpressLoader::GetXpressVarFromEnvironmentVariables(
    const char* XPRESS_var, bool verbose) {
  std::ostringstream msg;
  // Look for libraries pointed by XPRESSDIR first.
  std::string xpress_home_from_env = "";
#ifdef _MSC_VER
  size_t requiredSize;

  getenv_s(&requiredSize, NULL, 0, XPRESS_var);
  if (requiredSize != 0) {
    xpress_home_from_env.resize(requiredSize);

    // Get the value of the LIB environment variable.
    getenv_s(&requiredSize, xpress_home_from_env.data(), requiredSize,
             XPRESS_var);
    xpress_home_from_env = xpress_home_from_env.substr(0, requiredSize - 1);

  } else {
    if (verbose) {
      msg.str("");
      msg << "[Windows getenv_s function]: " << XPRESS_var
          << " doesn't exist!\n";
      logger_->display_message(msg);
    }
  }
#else
  char* path = nullptr;
  path = getenv(XPRESS_var);
  if (path) {
    xpress_home_from_env = path;
  }
#endif
  return xpress_home_from_env;
}

std::vector<std::string> XpressLoader::XpressDynamicLibraryPotentialPaths() {
  std::ostringstream msg;
  std::vector<std::string> potential_paths;

  const char* XPRESSDIR = "XPRESSDIR";
  std::string xpress_home_from_env =
      GetXpressVarFromEnvironmentVariables(XPRESSDIR);

  if (xpress_home_from_env != "") {
    std::filesystem::path prefix(xpress_home_from_env);
#if defined(_MSC_VER)  // Windows
    potential_paths.push_back((prefix / "bin" / "xprs.dll").string());
#elif defined(__APPLE__)  // OS X
    potential_paths.push_back((prefix / "lib" / "libxprs.dylib").string());
#elif defined(__GNUC__)   // Linux
    potential_paths.push_back((prefix / "lib" / "libxprs.so").string());
#else
    msg << "OS Not recognized by xpress/environment.cc."
        << " You won't be able to use Xpress.";
    logger_->display_message(msg);
#endif
  } else {
    msg.str("");
    msg << "Warning: "
        << "Environment variable " << XPRESSDIR << " undefined.\n";
    logger_->display_message(msg);
  }

  // Search for canonical places.
#if defined(_MSC_VER)  // Windows
  potential_paths.push_back("C:\\xpressmp\\bin\\xprs.dll");
  potential_paths.push_back("C:\\Program Files\\xpressmp\\bin\\xprs.dll");
#elif defined(__APPLE__)  // OS X
  potential_paths.push_back("/Library/xpressmp/lib/libxprs.dylib");
#elif defined(__GNUC__)   // Linux
  potential_paths.push_back("/opt/xpressmp/lib/libxprs.so");
#else
  msg.str("");
  msg << "OS Not recognized by environment.cc."
      << " You won't be able to use Xpress.";
  logger_->display_message(msg);
#endif
  return potential_paths;
}

bool XpressLoader::LoadXpressDynamicLibrary(std::string& xpresspath) {
  static std::string xpress_lib_path;
  static std::once_flag xpress_loading_done;
  static bool ret;
  static Solver::DynamicLibrary xpress_library;
  // static std::mutex mutex;

  // mutex.lock();
  std::ostringstream msg;
  std::call_once(xpress_loading_done, [this, &msg]() {
    const std::vector<std::string> canonical_paths =
        XpressDynamicLibraryPotentialPaths();
    for (const std::string& path : canonical_paths) {
      msg.str("");
      if (xpress_library.TryToLoad(path)) {
        msg << "Info: "
            << "Found the Xpress library in " << path << ".\n";
        logger_->display_message(msg);
        xpress_lib_path.clear();
        std::filesystem::path p(path);
        // p.remove_filename();
        xpress_lib_path.append(p.parent_path().string());
        break;
      }
    }

    if (xpress_library.LibraryIsLoaded()) {
      ret = LoadXpressFunctions(&xpress_library);
    } else {
      msg.str("");
      msg << "Could not find the Xpress shared library. Looked in: ["
          << StringJoin(canonical_paths)
          << "]. Please check environment variable XPRESSDIR\n";
      msg << std::endl;
      logger_->display_message(msg);
      ret = false;
    }
  });
  xpresspath.clear();
  xpresspath.append(xpress_lib_path);
  return ret;
}

int XpressLoader::loadLicence(const std::string& lib_path, bool verbose) {
  std::ostringstream msg;

  //-----first let xpress find the licence
  int code = XPRSinit(nullptr);
  if (!code) {
    return code;
  }

  // search for XPAUTH_PATH env var
  const auto XPAUTH_PATH = "XPAUTH_PATH";
  std::string xpauth_path_env_var =
      GetXpressVarFromEnvironmentVariables(XPAUTH_PATH, false);
  if (!xpauth_path_env_var.empty()) {
    auto xpauth_path_parent_path =
        std::filesystem::path(xpauth_path_env_var).parent_path().string();
    code = XPRSinit(xpauth_path_parent_path.c_str());
    if (!code) {
      return code;
    }
  } else {
    if (verbose) {
      msg.str("");
      msg << "Warning: Environment variable " << XPAUTH_PATH << " undefined.\n";
      logger_->display_message(msg);
    }
  }

  //-- XPRESS env var
  const auto XPRESS = "XPRESS";

  std::string xpress_env_var =
      GetXpressVarFromEnvironmentVariables(XPRESS, false);
  if (!xpress_env_var.empty()) {
    code = XPRSinit(xpress_env_var.c_str());
    if (!code) {
      return code;
    }
  } else {
    if (verbose) {
      msg.str("");
      msg << "Warning: Environment variable " << XPRESS << " undefined.\n";
      logger_->display_message(msg);
    }
  }
  // --- in xpress bin dir
  auto xpress_bin_dir =
      (std::filesystem::path(lib_path).parent_path() / "bin").string();
  code = XPRSinit(xpress_bin_dir.c_str());
  return code;
}

/** init XPRESS environment */
bool XpressLoader::initXpressEnv(bool verbose, int xpress_oem_license_key) {
  std::ostringstream msg;

  std::string xpresspath;
  bool status = LoadXpressDynamicLibrary(xpresspath);
  if (!status) {
    return false;
  }

  int code;

  // if not an OEM key
  if (xpress_oem_license_key == 0) {
    if (verbose) {
      msg.str("");
      msg << "Initialising xpress-MP with parameter " << xpresspath << "\n";
      logger_->display_message(msg);
    }

    code = loadLicence(xpresspath, false);

    if (!code) {
      // XPRSbanner informs about Xpress version, options and error messages
      if (verbose) {
        printXpressBanner();
        char version[16];
        XPRSgetversion(version);
        msg.str("");
        msg << "Warning: "
            << "Optimizer version: " << version
            << " (Antares-Xpansion was compiled with version " << XPVERSION
            << ").\n";
        logger_->display_message(msg);
      }
      return true;
    } else {
      msg.str("");
      msg << "XpressInterface: Xpress found at " << xpresspath << "\n";

      char errmsg[256];
      XPRSgetlicerrmsg(errmsg, 256);

      msg << "Xpress License error : " << errmsg << " (XPRSinit returned code "
          << code << "). Please check"
          << " environment variable XPRESS.\n";
      logger_->display_message(msg);

      return false;
    }
  } else {
    // if OEM key
    if (verbose) {
      msg.str("");
      msg << "Warning: "
          << "Initialising xpress-MP with OEM key " << xpress_oem_license_key
          << "\n";
      logger_->display_message(msg);
    }

    int nvalue = 0;
    int ierr;
    char slicmsg[256] = "";
    char errmsg[256];

    XPRSlicense(&nvalue, slicmsg);
    if (verbose) {
      msg.str("");
      msg << "First message from XPRSLicense : " << slicmsg << "\n";
      logger_->display_message(msg);
    }

    nvalue = xpress_oem_license_key - ((nvalue * nvalue) / 19);
    ierr = XPRSlicense(&nvalue, slicmsg);

    if (verbose) {
      msg.str("");
      msg << "Second message from XPRSLicense : " << slicmsg << "\n";
      logger_->display_message(msg);
    }
    if (ierr == 16) {
      if (verbose) {
        msg.str("");
        msg << "Optimizer development software detected\n";
        logger_->display_message(msg);
      }
    } else if (ierr != 0) {
      // get the license error message
      XPRSgetlicerrmsg(errmsg, 256);

      msg.str("");
      msg << "Xpress Error Message: " << errmsg << "\n";
      logger_->display_message(msg);
      return false;
    }

    code = XPRSinit(NULL);

    if (!code) {
      return true;
    } else {
      msg.str("");
      msg << "XPRSinit returned code : " << code << "\n";
      logger_->display_message(msg);
      return false;
    }
  }
}

bool XpressLoader::XpressIsCorrectlyInstalled(bool verbose) {
  bool correctlyInstalled = initXpressEnv(verbose);
  if (correctlyInstalled) {
    XPRSfree();
  }
  return correctlyInstalled;
}

}  // namespace LoadXpress
