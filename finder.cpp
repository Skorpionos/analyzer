#include "finder.h"

namespace finder
{

SizeVector FindIndexesForKey(const uint8_t* buffer, const size_t start, const size_t length, const std::string& key)
{
    if (key.empty())
        return SizeVector();

    const auto& keyBytes = reinterpret_cast<const uint8_t*>(key.c_str());

    Uint8Vector keyBytesVector(keyBytes, keyBytes + key.size());

    return FindIndexesForBytesKey(buffer, start, length, keyBytesVector);
}

SizeVector FindIndexesForHexKey(const uint8_t* buffer, const size_t start, const size_t legth,
                                const std::string& hexKey, size_t& bytesCountInHexKey)
{
    if (hexKey.empty())
        return SizeVector();

    StringVector tokens;
    split(tokens, hexKey, boost::algorithm::is_any_of(" "));
    bytesCountInHexKey = tokens.size();

    return FindIndexesForBytesKey(buffer, start, legth, GetHexBytesFromStringVector(tokens));
}

Uint8Vector GetHexBytesFromStringVector(const StringVector& words)
{
    Uint8Vector hexKeyBytes;
    for (const auto& word : words)
    {
        uint16_t byte;
        std::stringstream stream;
        stream << std::hex << word;

        stream >> byte;

        hexKeyBytes.push_back(static_cast<uint8_t>(byte));
    }
    return hexKeyBytes;
}

// TODO if possible, use functions from standard library
SizeVector FindIndexesForBytesKey(const uint8_t* buffer, const size_t startIndex, const size_t length,
                                  const Uint8Vector& keyBytes)
{
    SizeVector resultIndexes;
    const size_t keyLength = keyBytes.size();

    const size_t endIndex =  startIndex + length;
    size_t previousBufferIndex = 0;
    for (size_t bufferIndex = startIndex; bufferIndex < endIndex;)
    {
        size_t keyIndex = 0;
        size_t foundStringLength = 0;
        size_t indexOfFirstKeyLetter = bufferIndex;
        while (keyIndex < keyLength &&
               bufferIndex < endIndex &&
               buffer[bufferIndex] == keyBytes[keyIndex])
        {
            ++foundStringLength;
            ++bufferIndex;
            ++keyIndex;
        }
        if (foundStringLength == keyLength)
            resultIndexes.push_back(indexOfFirstKeyLetter);

        if (bufferIndex == previousBufferIndex)
            ++bufferIndex;

        previousBufferIndex = bufferIndex;
    }
    return resultIndexes;
}

Range FindRangeForPairOfKeys(uint8_t* buffer, const size_t startIndex, size_t length, std::string hexKeyFrom,
                             std::string hexKeyTill)
{
    size_t countBytesInKeyFrom = 0;
    size_t countBytesInKeyTill = 0;

    SizeVector resultsFrom = FindIndexesForHexKey(buffer, startIndex, length, hexKeyFrom, countBytesInKeyFrom);
    SizeVector resultsTill = FindIndexesForHexKey(buffer, startIndex, length, hexKeyTill, countBytesInKeyTill);

    utilities::PrintFoundKeyResults(hexKeyFrom, resultsFrom, 0, false);
    utilities::PrintFoundKeyResults(hexKeyTill, resultsTill, 0, false);

    Range range;
    range.begin = resultsFrom.empty() ? startIndex
                                      : resultsFrom.front();
    range.end = resultsTill.empty() ? startIndex + length
                                    : resultsTill.back() + countBytesInKeyTill - 1;
    return range;
}

} // namespace finder
