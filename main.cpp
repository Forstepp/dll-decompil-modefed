#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = std::filesystem;

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

std::vector<std::size_t> findOccurrences(const std::vector<unsigned char>& data, const std::string& needle) {
    std::vector<std::size_t> positions;
    if (needle.empty() || data.size() < needle.size()) return positions;
    for (std::size_t i = 0; i <= data.size() - needle.size(); ++i) {
        bool ok = true;
        for (std::size_t j = 0; j < needle.size(); ++j) {
            if (data[i + j] != static_cast<unsigned char>(needle[j])) {
                ok = false;
                break;
            }
        }
        if (ok) positions.push_back(i);
    }
    return positions;
}

std::string extractAsciiStrings(const std::vector<unsigned char>& data, std::size_t minLen) {
    std::string out;
    std::string cur;
    for (unsigned char b : data) {
        if (b >= 32 && b <= 126) {
            cur.push_back(static_cast<char>(b));
        } else {
            if (cur.size() >= minLen) out += cur + "\n";
            cur.clear();
        }
    }
    if (cur.size() >= minLen) out += cur + "\n";
    return out;
}

std::string extractUtf16LeStrings(const std::vector<unsigned char>& data, std::size_t minLen) {
    std::string out;
    std::string cur;
    for (std::size_t i = 0; i + 1 < data.size(); i += 2) {
        unsigned char lo = data[i];
        unsigned char hi = data[i + 1];
        if (hi == 0 && lo >= 32 && lo <= 126) {
            cur.push_back(static_cast<char>(lo));
        } else {
            if (cur.size() >= minLen) out += cur + "\n";
            cur.clear();
        }
    }
    if (cur.size() >= minLen) out += cur + "\n";
    return out;
}

fs::path getExecutableDir() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    DWORD len = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    if (len == 0 || len == MAX_PATH) return fs::current_path();
    return fs::path(buffer).parent_path();
#else
    return fs::current_path();
#endif
}

bool hasDllExtension(const fs::path& p) {
    return toLower(p.extension().string()) == ".dll";
}

int main() {
    const fs::path currentDir = getExecutableDir();
    const fs::path reportPath = currentDir / "scan_report.txt";
    const std::vector<std::string> indicators = {
        "AxelBB", "triger", "TriggerBot", "Reach", "Esp", "XRay", "Hitbox",
        "qEXPANDING_BUTTON", "EXPANDING_BUTTON", "EXPANDING_BUTTONq", "GishCode",
        R"(HARDWAREDeviceMapVideoy)", "prevScreenes}", "WindowsSelectorImpl.javay",
        "axisalignedbb", "0/qvaxisalignedbb", "+compileESP", "compileESP", "eKillAura", "YautoArmor",
        "_execute_onexit_table", "Module32Next>", "CreateToolhelp32Snapshot", "net/minecraft/util/math/AxisAlignedBB"
    };

    std::ofstream report(reportPath, std::ios::trunc);
    if (!report) {
        std::cerr << "Failed to create report: " << reportPath << "\n";
        return 1;
    }

    std::size_t dllCount = 0;
    std::size_t hitFiles = 0;
    std::size_t totalRawHits = 0;
    std::size_t totalStringHits = 0;

    for (const auto& entry : fs::directory_iterator(currentDir)) {
        if (!entry.is_regular_file()) continue;
        const fs::path filePath = entry.path();
        if (!hasDllExtension(filePath)) continue;

        ++dllCount;
        std::ifstream file(filePath, std::ios::binary);
        if (!file) {
            report << "[ERROR] Cannot open " << filePath.filename().string() << "\n\n";
            continue;
        }

        std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        std::string corpus = extractAsciiStrings(bytes, 4) + extractUtf16LeStrings(bytes, 4);
        std::string corpusLower = toLower(corpus);

        bool fileHasHit = false;
        report << "File: " << filePath.filename().string() << "\n";

        for (const auto& indicator : indicators) {
            auto raw = findOccurrences(bytes, indicator);
            bool inStrings = corpusLower.find(toLower(indicator)) != std::string::npos;

            if (!raw.empty() || inStrings) {
                fileHasHit = true;
                totalRawHits += raw.size();
                if (inStrings) ++totalStringHits;

                report << "  Indicator: " << indicator << "\n";
                report << "  Raw offsets: ";
                if (raw.empty()) {
                    report << "none";
                } else {
                    for (std::size_t i = 0; i < raw.size(); ++i) {
                        report << "0x" << std::hex << std::uppercase << raw[i] << std::dec;
                        if (i + 1 < raw.size()) report << ", ";
                    }
                }
                report << "\n";
                report << "  Found in extracted strings: " << (inStrings ? "yes" : "no") << "\n";
            }
        }

        if (!fileHasHit) {
            report << "  No indicators found.\n";
        } else {
            ++hitFiles;
        }
        report << "\n";
    }

    report << "Scanned DLL files: " << dllCount << "\n";
    report << "Files with indicators: " << hitFiles << "\n";
    report << "Total raw indicator hits: " << totalRawHits << "\n";
    report << "Total string-corpus hits: " << totalStringHits << "\n";

    std::cout << "Done. Report: " << reportPath << "\n";
    return 0;
}
