// Copyright 2013 Dolphin Emulator Project / 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#ifdef _WIN32

#include <cstdint>
#include <windows.h>
#include "string_util.h"

namespace Common {

static std::wstring CPToUTF16(std::uint32_t code_page, const std::string& input) {
    const auto size =
        MultiByteToWideChar(code_page, 0, input.data(), static_cast<int>(input.size()), nullptr, 0);

    if (size == 0) {
        return L"";
    }

    std::wstring output(size, L'\0');

    if (size != MultiByteToWideChar(code_page, 0, input.data(), static_cast<int>(input.size()),
                                    &output[0], static_cast<int>(output.size()))) {
        output.clear();
    }

    return output;
}

std::wstring UTF8ToUTF16W(const std::string& input) {
    return CPToUTF16(CP_UTF8, input);
}

} // namespace Common

#endif
