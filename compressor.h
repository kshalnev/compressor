#pragma once

#include "icompressor.h"

#include <memory>

std::shared_ptr<ICompressor> GetCompressor(const char* compressorName);

void Compress(ICompressor* compressor, const char* sourceFile, const char* destFile);

void Decompress(ICompressor* compressor, const char* sourceFile, const char* destFile);

void Compress(const char* compressorName, const char* sourceFile, const char* destFile);

void Decompress(const char* compressorName, const char* sourceFile, const char* destFile);
