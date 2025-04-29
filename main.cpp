#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <zstd.h>

namespace fs = std::filesystem;

bool decompressFile(const fs::path& inputPath) {
    // 打开 .zst 或 .zstd 文件
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
        std::cerr << "Unknown decompressed size: " << inputPath << "\n";
        return false;
    }

    std::vector<char> decompressed(decompressedSize);
    size_t actualSize = ZSTD_decompress(decompressed.data(), decompressedSize,
                                        compressed.data(), size);

    if (ZSTD_isError(actualSize)) {
        std::cerr << "ZSTD decompress error: " << ZSTD_getErrorName(actualSize) << "\n";
        return false;
    }

    // 写入解压后的文件，并加上 .zstd 后缀
    fs::path outputPath = inputPath;
    outputPath.replace_extension(".zstd");

    std::ofstream output(outputPath, std::ios::binary);
    if (!output.write(decompressed.data(), actualSize)) {
        std::cerr << "Write error: " << outputPath << "\n";
        return false;
    }

    std::cout << "✔️ Decompressed: " << inputPath.filename() << " → " << outputPath.filename() << "\n";

    // 打印解压出来的内容到终端
    std::cout << "----- Content Start -----\n";
    std::cout.write(decompressed.data(), actualSize);
    std::cout << "\n----- Content End -----\n";

    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: zstd_decompress <file_path>\n";
        return 1;
    }

    fs::path filePath = argv[1];

    if (!fs::exists(filePath)) {
        std::cerr << "File does not exist: " << filePath << "\n";
        return 1;
    }

    // 检查文件扩展名是 .zst 或 .zstd
    if (filePath.extension() != ".zst" && filePath.extension() != ".zstd") {
        std::cerr << "The file is neither a .zst nor a .zstd file: " << filePath << "\n";
        return 1;
    }

    if (!decompressFile(filePath)) {
        std::cerr << "Failed to decompress: " << filePath << "\n";
        return 1;
    }

    return 0;
}
