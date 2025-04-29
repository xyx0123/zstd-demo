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

    // 获取解压后大小（或者猜一个足够大）
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

    // 写入解压后的文件
    fs::path outputPath = inputPath;
    outputPath.replace_extension();  // 去掉 .zst 后缀

    std::ofstream output(outputPath, std::ios::binary);
    if (!output.write(decompressed.data(), actualSize)) {
        std::cerr << "Write error: " << outputPath << "\n";
        return false;
    }

    std::cout << "✔️ Decompressed: " << inputPath.filename() << " → " << outputPath.filename() << "\n";
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: zstd_decompress_dir <directory_path>\n";
        return 1;
    }

    fs::path dirPath = argv[1];
    if (!fs::is_directory(dirPath)) {
        std::cerr << "Not a directory: " << dirPath << "\n";
        return 1;
    }

    for (const auto& entry : fs::directory_iterator(dirPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".zst") {
            decompressFile(entry.path());
        }
    }

    return 0;
}
