#include "finder.h"

namespace finder
{

SizeVector FindIndexesForKey(const uint8_t* buffer, const size_t bufferLength, const std::string& key)
{
    if (key.empty())
        return SizeVector();

    const auto& keyBytes = reinterpret_cast<const uint8_t*>(key.c_str());

    Uint8Vector keyBytesVector(keyBytes, keyBytes + key.size());

    return FindIndexesForBytesKey(buffer, bufferLength, keyBytesVector);
}

SizeVector FindIndexesForHexKey(const uint8_t* buffer, const size_t bufferLength, const std::string& hexKey,
                                size_t& bytesCountInHexKey)
{
    if (hexKey.empty())
        return SizeVector();

    StringVector tokens;
    tokens = split(tokens, hexKey, boost::algorithm::is_any_of(" "));
    bytesCountInHexKey = tokens.size();

    return FindIndexesForBytesKey(buffer, bufferLength, GetHexBytesFromStringVector(tokens));
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

// TODO if possible, use something like "search" from standard library
SizeVector FindIndexesForBytesKey(const uint8_t* buffer, const size_t bufferLength, const Uint8Vector& keyBytes)
{
    SizeVector resultIndexes;
    const size_t keyLength = keyBytes.size();

    size_t bufferIndex = 0;
    size_t previousBufferIndex = 0;
    while (bufferIndex < bufferLength)
    {
        size_t keyIndex = 0;
        size_t foundStringLength = 0;
        size_t indexOfFirstKeyLetter = bufferIndex;
        while (keyIndex < keyLength &&
               bufferIndex < bufferLength &&
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

Range FindRangeForPairOfKeys(uint8_t* buffer, size_t bufferLength, std::string hexKeyFrom, std::string hexKeyTill)
{
    SizeVector resultsFrom;
    SizeVector resultsTill;

    size_t countBytesInKeyFrom = 0;
    size_t countBytesInKeyTill = 0;

    FindIndexesForHexKey(buffer, bufferLength, hexKeyFrom, countBytesInKeyFrom);
    FindIndexesForHexKey(buffer, bufferLength, hexKeyTill, countBytesInKeyTill);

    utilities::PrintFoundKeyResults(hexKeyFrom, resultsFrom, 0);
    utilities::PrintFoundKeyResults(hexKeyTill, resultsTill, 0);

    Range range;
    range.begin = !resultsFrom.empty() ? resultsFrom.front() : 0;
    range.end = !resultsTill.empty() ? resultsTill.back() + countBytesInKeyTill - 1 : 0;

    return range;
}

} // namespace finder
