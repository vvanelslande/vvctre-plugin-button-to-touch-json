// Copyright 2013 Dolphin Emulator Project / 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#ifdef _WIN32

#include <string>

namespace Common {

std::wstring UTF8ToUTF16W(const std::string& input);

} // namespace Common

#endif
