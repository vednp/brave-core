/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "base/feature_list.h"

#define SetupBasicInterceptions SetupBasicInterceptions_ChromiumImpl

#include "src/sandbox/win/src/policy_broker.cc"

#undef SetupBasicInterceptions

#include "brave/sandbox/win/src/module_file_name_interception.h"

#if !defined(PSAPI_VERSION)
#error <Psapi.h> should be included.
#endif

namespace sandbox {

namespace {
bool SetupModuleFilenameInterceptions(InterceptionManager* manager) {
  if (!base::FeatureList::IsEnabled(kModuleFileNamePatch)) {
    return true;
  }
  if (!INTERCEPT_EAT(manager, kKerneldllName, GetModuleFileNameA,
                     GET_MODULE_FILENAME_A_ID, 3)) {
    return false;
  }

  if (!INTERCEPT_EAT(manager, kKerneldllName, GetModuleFileNameW,
                     GET_MODULE_FILENAME_W_ID, 3)) {
    return false;
  }

#if (PSAPI_VERSION > 1)
  if (!INTERCEPT_EAT(manager, kKerneldllName, K32GetModuleFileNameExA,
                     GET_MODULE_FILENAME_EX_A_ID, 4)) {
    return false;
  }
  if (!INTERCEPT_EAT(manager, kKerneldllName, K32GetModuleFileNameExW,
                     GET_MODULE_FILENAME_EX_W_ID, 4)) {
    return false;
  }
#else
  if (!INTERCEPT_EAT(manager, kKerneldllName, GetModuleFileNameExA,
                     GET_MODULE_FILENAME_EX_A_ID, 4)) {
    return false;
  }
  if (!INTERCEPT_EAT(manager, kKerneldllName, GetModuleFileNameExW,
                     GET_MODULE_FILENAME_EX_W_ID, 4)) {
    return false;
  }
#endif

  return true;
}
}  // namespace

bool SetupBasicInterceptions(InterceptionManager* manager,
                             bool is_csrss_connected) {
  if (!SetupBasicInterceptions_ChromiumImpl(manager, is_csrss_connected)) {
    return false;
  }

  if (!SetupModuleFilenameInterceptions(manager)) {
    return false;
  }

  return true;
}

}  // namespace sandbox
