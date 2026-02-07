#include "DatasetLoader.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>

namespace fs = std::filesystem;

#define MAX_SUDOKU_COUNT 250

static std::string mergedPath(const std::string& folder)
{
    return folder + "/merged.txt";
}

std::vector<std::vector<Sudoku>>
DatasetLoader::loadAllDatasets(const std::string& rootFolder)
{
    std::vector<std::vector<Sudoku>> all;

    for (int d = 0; d <= 5; ++d)
    {
        std::string folder =
            rootFolder + "/dataset" + std::to_string(d);

        std::cout << "[INFO] Loading " << folder << std::endl;
        all.push_back(loadSingleDataset(folder));
    }

    return all;
}

std::vector<Sudoku>
DatasetLoader::loadSingleDataset(const std::string& datasetFolder)
{
    std::vector<Sudoku> sudokus;
    std::string merged = mergedPath(datasetFolder);

    // ==================================================
    // CASE 1: merged.txt VAR
    // ==================================================
    if (fs::exists(merged))
    {
        std::ifstream in(merged);
        if (!in)
            throw std::runtime_error("Cannot open merged.txt");

        size_t count;
        in >> count;

        sudokus.reserve(count);

        for (size_t i = 0; i < count && i < MAX_SUDOKU_COUNT; ++i)
        {
            try {
                Sudoku s;
                in >> s;                 // <<< operator>> kullanılıyor
                sudokus.push_back(s);
            }
            catch (const std::runtime_error& e) {
                std::cerr << "[WARN] Skipping invalid sudoku in merged.txt: "
                    << "(" << e.what() << ")"
                    << std::endl;
            }
        }
        std::cout << "[INFO] Read " << sudokus.size()
			<< " sudokus from merged.txt"
            << std::endl;
        return sudokus;
    }

    // ==================================================
    // CASE 2: merged.txt YOK
    // ==================================================
    std::cout << "[INFO] Creating merged.txt in "
              << datasetFolder << std::endl;

    std::vector<fs::path> files;
    for (const fs::directory_entry& e :
         fs::directory_iterator(datasetFolder))
    {
        if (e.is_regular_file() &&
            e.path().extension() == ".txt" &&
            e.path().filename() != "merged.txt")
        {
            files.push_back(e.path());
        }
    }

    std::sort(files.begin(), files.end());

    // 1️⃣ Önce RAM’e oku
    for (size_t i = 0; i < files.size(); ++i)
    {
        if((i+1) % 1000 == 0)
            std::cout << "[INFO] Reading file " << (i + 1)
			<< " / " << files.size() << "\r" << std::flush;

        std::ifstream in(files[i]);
        if (!in)
            continue;
        try{
            Sudoku s;
            in >> s;                     // <<< operator>>
            sudokus.push_back(s);
        }
        catch (const std::runtime_error& e) {
            std::cerr << "[WARN] Skipping invalid file: "
                << files[i] << " (" << e.what() << ")"
                << std::endl;
        }
    }
    std::cout << "[INFO] Read " << sudokus.size()
              << " sudokus from " << files.size() << " files."
		<< std::endl;
    // 2️⃣ Sonra tek seferde merged.txt yaz
    std::ofstream out(merged);
    if (!out)
        throw std::runtime_error("Cannot create merged.txt");

    out << sudokus.size() << '\n';

    for (size_t i = 0; i < sudokus.size(); ++i)
        sudokus[i].writeRaw(out);

    return sudokus;
}