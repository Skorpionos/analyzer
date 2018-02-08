#include "finder.h"

namespace finder
{

SizeVector FindIndexesForKey(const uint8_t* buffer, const Range range, TheKey& key)
{
    if (key.value.empty())
        return SizeVector();

    key.length = key.value.size();

    const auto& keyBytes = reinterpret_cast<const uint8_t*>(key.value.c_str());

    Uint8Vector keyBytesVector(keyBytes, keyBytes + key.value.size());

    return FindIndexesForBytesKey(buffer, range, keyBytesVector);
}

SizeVector FindIndexesForHexKey(const uint8_t* buffer, const Range range, TheKey& hexKey)
{
    if (hexKey.value.empty())
        return SizeVector();

    StringVector tokens;
    split(tokens, hexKey.value, boost::algorithm::is_any_of(" "));
    hexKey.length = tokens.size();

    return FindIndexesForBytesKey(buffer, range, GetHexBytesFromStringVector(tokens));
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

// TODO use functions from standard library
SizeVector FindIndexesForBytesKey(const uint8_t* buffer, const Range range, const Uint8Vector& keyBytes)
{
    SizeVector resultIndexes;
    const size_t keyLength = keyBytes.size();

    size_t previousBufferIndex = 0;
    for (size_t bufferIndex = range.begin; bufferIndex < range.end;)
    {
        size_t keyIndex = 0;
        size_t foundStringLength = 0;
        size_t indexOfFirstKeyLetter = bufferIndex;
        while (keyIndex < keyLength &&
               bufferIndex < range.end &&
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

Range FindRangeForPairOfKeys(uint8_t* buffer, const Range range, TheKey& hexKeyFrom, TheKey& hexKeyTill)
{
    hexKeyFrom.results = FindIndexesForHexKey(buffer, range, hexKeyFrom);
    hexKeyTill.results = FindIndexesForHexKey(buffer, range, hexKeyTill);

    Range resultRange;
    resultRange.begin = hexKeyFrom.results.empty() ? range.begin : hexKeyFrom.results.front();
    resultRange.end   = hexKeyTill.results.empty() ? range.end   : hexKeyTill.results.back() + hexKeyTill.length - 1;

    return resultRange;
}

} // namespace finder
