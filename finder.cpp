#include "finder.h"

namespace finder
{

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

Range Finder::FindRange(uint8_t* buffer, Range range, const std::string& hexFromValue, const std::string& hexTillValue)
{
    Range resultRange;

    resultRange.begin = FindFirst(buffer, range, hexFromValue);
    resultRange.end   = FindLast(buffer, range, hexTillValue);

    return resultRange;
}

size_t Finder::FindFirstForBytesKey(const uint8_t* buffer, Range range, const Uint8Vector& keyBytes)
{
    auto index = std::search(buffer + range.begin, buffer + range.end + 1, std::begin(keyBytes), std::end(keyBytes));

    return index - buffer;
}

size_t Finder::FindLastForBytesKey(const uint8_t* buffer, Range range, const Uint8Vector& keyBytes)
{
    const auto bufferBegin = buffer + range.begin;
    const auto bufferEnd = buffer + range.end + 1;

    auto index = std::find_end(bufferBegin, bufferEnd, std::begin(keyBytes), std::end(keyBytes));

    auto result = index - buffer;

    if (index != bufferEnd)
        result += keyBytes.size() - 1;

    return result;
}

SizeVector KeyFinder::Find(const uint8_t* buffer, Range range, Key& key)
{
    if (key.value.empty())
        return SizeVector();

    key.length = key.value.size();

    const auto& keyBytes = reinterpret_cast<const uint8_t*>(key.value.c_str());

    Uint8Vector keyBytesVector(keyBytes, keyBytes + key.value.size());

    return KeyFinder::FindIndexesForBytesKey(buffer, range, keyBytesVector);
}

SizeVector Finder::FindIndexesForBytesKey(const uint8_t* buffer, Range range, const Uint8Vector& keyBytes)
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

size_t HexKeyFinder::FindFirst(const uint8_t* buffer, Range range, std::string hexKeyValue)
{
    if (hexKeyValue.empty())
        return range.begin;

    StringVector tokens;
    split(tokens, hexKeyValue, boost::algorithm::is_any_of(" "));

    return FindFirstForBytesKey(buffer, range, GetHexBytesFromStringVector(tokens));
}

size_t HexKeyFinder::FindLast(const uint8_t* buffer, Range range, std::string hexKeyValue)
{
    if (hexKeyValue.empty())
        return range.end;

    StringVector tokens;
    split(tokens, hexKeyValue, boost::algorithm::is_any_of(" "));


    return FindLastForBytesKey(buffer, range, GetHexBytesFromStringVector(tokens));
}

SizeVector HexKeyFinder::Find(const uint8_t* buffer, Range range, Key& hexKey)
{
    if (hexKey.value.empty())
        return SizeVector();

    StringVector tokens;
    split(tokens, hexKey.value, boost::algorithm::is_any_of(" "));
    hexKey.length = tokens.size();

    return FindIndexesForBytesKey(buffer, range, GetHexBytesFromStringVector(tokens));
}

} // namespace finder
