#include <fstream>
#include <string_view>
#include <nlohmann/json.hpp>
#include <simdutf.h>

class Parser
{
public:
    using char_type = char16_t;
    using string_type = std::basic_string<char_type>;
    using string_view_type = std::basic_string_view<char_type>;

    constexpr static auto length_from_utf8_func = simdutf::utf16_length_from_utf8;
    constexpr static auto convert_utf8_func = simdutf::convert_utf8_to_utf16;

private:
    struct string_hash
    {
        using hash_type = std::hash<string_view_type>;
        using is_transparent = void;

        std::size_t operator()(const char_type *str) const { return hash_type{}(str); }
        std::size_t operator()(const string_type &str) const { return hash_type{}(str); }
        std::size_t operator()(const string_view_type str) const { return hash_type{}(str); }
    };

    using group_type = std::unordered_map<string_type, int, string_hash, std::equal_to<>>;
    using model_type = std::unordered_map<string_type, group_type, string_hash, std::equal_to<>>;

public:
    Parser &loadByFileName(const std::string &modelFileName)
    {
        std::ifstream fs(modelFileName);

        std::unordered_map<std::string, std::unordered_map<std::string, int>> internal;
        nlohmann::json::parse(fs).get_to(internal);

        for (const auto &[u8name, u8group] : internal)
        {
            string_type name(
                length_from_utf8_func(u8name.data(), u8name.length()),
                u'\0');
            convert_utf8_func(u8name.data(), u8name.length(), name.data());

            group_type group;
            group.reserve(u8group.size());
            for (const auto &[u8key, point] : u8group)
            {
                string_type key(
                    length_from_utf8_func(u8key.data(), u8key.length()),
                    u'\0');
                convert_utf8_func(u8key.data(), u8key.length(), key.data());

                group.emplace(std::move(key), point);
            }

            _model.emplace(
                std::move(name),
                std::move(group));
        }

        return *this;
    }

    std::vector<string_view_type> parse(const string_view_type sentence)
    {
        if (sentence.empty())
        {
            return {};
        }

        std::vector<string_view_type> result;
        result.emplace_back(sentence.substr(0, 1));

        int totalScore = 0;
        for (auto &[_, group] : _model)
        {
            for (auto &[__, s] : group)
            {
                totalScore += s;
            }
        }

        for (int i = 1; i < sentence.length(); i++)
        {
            int score = -totalScore;

            if (i - 2 > 0)
            {
                score += 2 * getScore(u"UW1", sentence.substr(i - 3, 1));
            }

            if (i - 1 > 0)
            {
                score += 2 * getScore(u"UW2", sentence.substr(i - 2, 1));
            }

            score += 2 * getScore(u"UW3", sentence.substr(i - 1, 1));
            score += 2 * getScore(u"UW4", sentence.substr(i, 1));

            if (i + 1 < sentence.length())
            {
                score += 2 * getScore(u"UW5", sentence.substr(i + 1, 1));
            }

            if (i + 2 < sentence.length())
            {
                score += 2 * getScore(u"UW6", sentence.substr(i + 2, 1));
            }

            if (i > 1)
            {
                score += 2 * getScore(u"BW1", sentence.substr(i - 2, 2));
            }

            score += 2 * getScore(u"BW2", sentence.substr(i - 1, 2));

            if (i + 1 < sentence.length())
            {
                score += 2 * getScore(u"BW3", sentence.substr(i, 2));
            }

            if (i - 2 > 0)
            {
                score += 2 * getScore(u"TW1", sentence.substr(i - 3, 3));
            }

            if (i - 1 > 0)
            {
                score += 2 * getScore(u"TW2", sentence.substr(i - 2, 3));
            }

            if (i + 1 < sentence.length())
            {
                score += 2 * getScore(u"TW3", sentence.substr(i - 1, 3));
            }

            if (i + 2 < sentence.length())
            {
                score += 2 * getScore(u"TW4", sentence.substr(i, 3));
            }

            if (score > 0)
            {
                result.push_back(sentence.substr(i, 1));
            }
            else
            {
                auto &back = result.back();

                assert(back.end() != sentence.end());
                back = string_view_type{back.data(), back.length() + 1};
            }
        }

        return result;
    }

private:
    model_type _model;

    int getScore(const string_view_type featureKey, const string_view_type sequence)
    {
        auto groupItr = _model.find(featureKey);
        if (groupItr == _model.cend())
        {
            return 0;
        }

        auto seqItr = groupItr->second.find(sequence);
        if (seqItr == groupItr->second.cend())
        {
            return 0;
        }

        return seqItr->second;
    }
};
