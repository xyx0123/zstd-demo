#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <zstd.h>

namespace fs = std::filesystem;

bool decompressFile(const fs::path& inputPath) {
    // 打开 .zst 文件
    std::ifstream input(inputPath, std::ios::binary | std::ios::ate);
    if (!input) {
        std::cerr << "Failed to open: " << inputPath << "\n";
        return false;
    }

    // 读取内容
    std::streamsize size = input.tellg();
    input.seekg(0, std::ios::beg);
    std::vector<char> compressed(size);
    if (!input.read(compressed.data(), size)) {
        std::cerr << "Read error: " << inputPath << "\n";
        return false;
    }

    // 获取解压后大小
    unsigned long long decompressedSize = ZSTD_getFrameContentSize(compressed.data(), size);
    if (decompressedSize == ZSTD_CONTENTSIZE_ERROR || decompressedSize == ZSTD_CONTENTSIZE_UNKNOWN) {
        std::cerr << "Unknown decom
