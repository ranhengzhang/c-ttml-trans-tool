//
// Created by LEGION on 2025/12/14.
//

#ifndef LYRIC_PARSER_LYRICTIME_H
#define LYRIC_PARSER_LYRICTIME_H

#include <compare>

#include <QString>

class LyricTime {
public:
    [[nodiscard]] static LyricTime max() {
        return {5999999};
    }

    [[nodiscard]] static LyricTime min() {
        return {0};
    }

    [[nodiscard]] static std::pair<LyricTime, bool> parse(const QString &str);

    LyricTime(int64_t count = 0);

    template<typename T>
    requires std::constructible_from<int64_t, T>
    [[nodiscard]] LyricTime operator-(const T &rhs) const {
        LyricTime time{};
        time._count = this->_count - static_cast<int64_t>(rhs);
        return time;
    }

    template<typename T>
    requires std::constructible_from<int64_t, T>
    LyricTime operator-=(const T &rhs) {
        this->_count -= static_cast<int64_t>(rhs);
        return *this;
    }

    template<typename T>
    requires std::constructible_from<int64_t, T>
    [[nodiscard]] LyricTime operator+(const T rhs) const {
        LyricTime time{};
        time._count = this->_count + static_cast<int64_t>(rhs);
        return time;
    }

    template<typename T>
    requires std::constructible_from<int64_t, T>
    LyricTime operator+=(const T rhs) {
        this->_count += static_cast<int64_t>(rhs);
        return *this;
    }

    template<typename T>
    requires std::constructible_from<int64_t, T>
    [[nodiscard]] LyricTime operator/(const T &rhs) const {
        LyricTime time{};
        time._count = this->_count / static_cast<int64_t>(rhs);
        return time;
    }

    template<typename T>
    requires std::constructible_from<int64_t, T>
    [[nodiscard]] LyricTime operator*(const T rhs) const {
        LyricTime time{};
        time._count = this->_count * static_cast<int64_t>(rhs);
        return time;
    }

    auto operator<=>(const LyricTime &) const = default;

    template<typename T>
    requires std::constructible_from<int64_t, T>
    std::strong_ordering operator<=>(const T& other) const {
        return this->_count <=> static_cast<int64_t>(other);
    }

    template<typename T>
    requires std::constructible_from<int64_t, T>
    bool operator==(const T& other) const {
        return this->_count == static_cast<int64_t>(other);
    }

    void offset(int64_t count);

    /**
     * @brief 格式化输出
     * @param pre_length 前置补零长度
     * @param to_centi 末尾输出为厘秒
     * @param to_dot 使用 <code>.</code> 分隔，否则使用 <code>:</code> 分隔
     * @return 格式化后的时间戳
     */
    [[nodiscard]] QString toString(uint8_t pre_length = 0, bool to_centi = false, bool to_dot = true) const;

    explicit operator int64_t() const;

    [[nodiscard]] LyricTime toShort() const;

    [[nodiscard]] int64_t abs() const;

private:
    int64_t _count{};
};


#endif //LYRIC_PARSER_LYRICTIME_H
