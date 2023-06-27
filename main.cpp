#include <iostream>
#include <format>
#include <filesystem>
#include <fstream>
#include <thread>

#include <boost/asio/thread_pool.hpp>

namespace fs = std::filesystem;
namespace ba = boost::asio;

static constexpr char WORD_DELIMITER = ' ';

[[nodiscard]]
std::unique_ptr<char[]> readNextChunk(std::ifstream & fin) {
    static constexpr size_t CHUNK_SIZE = 1024 * 1024;
    static constexpr size_t LONGEST_ENGLISH_WORD = 50;

    std::unique_ptr<char[]> buffer(new char[CHUNK_SIZE]);
    auto r = fin.read(buffer.get(), CHUNK_SIZE - LONGEST_ENGLISH_WORD);

    std::cout << "CHUNK_SIZE - LONGEST_ENGLISH_WORD = " << CHUNK_SIZE - LONGEST_ENGLISH_WORD << std::endl;
    std::cout << "r = " << r << ", CHUNK_SIZE - r = " << CHUNK_SIZE - r << std::endl;

    fin.get(buffer.get() + r, CHUNK_SIZE - r, WORD_DELIMITER);

    std::cout << "buffer - " << buffer.get() << std::endl;

    return buffer;
}

std::set<std::string> getUniqueWords(const std::unique_ptr<char[]> & words, std::uint64_t size) {
    static constexpr auto AVERAGE_ENGLISH_WORD = 5;
    if (!words) {
        return {};
    }

    std::set<std::string> splitedWords;
    splitedWords.reserve(size / AVERAGE_ENGLISH_WORD);

    boost::split(splitedWords, words.get(), WORD_DELIMITER);

    std::cout << "splitedWords.size() - " << splitedWords.size();
    return splitedWords;
}

[[nodiscard]]
uint64_t calculateUniqueWords(const std::filesystem::path & file) {
    std::ifstream userFile(file);
    ba::thread_pool pool(std::thread::hardware_concurrency());

    std::vector<std::future<decltype(getUniqueWords)>> futures;
    std::filesystem::file_size(p);

    while (userFile) {
        auto [chunk, size] = readNextChunk(userFile);
        if (!chunk) {
            break;
        }
        futures.push_back(ba::post(pool, std::bind(getUniqueWords, chunk, size)));
    }

    pool.join();

    std::set<std::string> uniqueWords;

    for (auto future & : futures) {
        uniqueWords.insert(future);
    }
    
    return uniqueWords.size();
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cout << "Wrond number of parameters" << std::endl;
        std::cout << std::format("Usage - {} <filename>", argv[0]) << std::endl;
        return -1;
    }

    std::error_code ec;
    if (!fs::exists(argv[1], ec)) {
        std::cout << std::format("The path {} you entered doesn't exist", argv[1]) << std::endl;
        return -2;
    }

    auto count = calculateUniqueWords(argv[1]);

    std::cout << fmt::format("Number of unique words is {}", count) << std::endl;

    return 0;
}