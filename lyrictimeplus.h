//
// Created by LEGION on 2025/11/21.
//

#ifndef TTMLX_LYRICTIMEPLUS_H
#define TTMLX_LYRICTIMEPLUS_H

#include <QString>
#include <compare>

class LyricTimePlus {
public:
    [[nodiscard]] static std::pair<LyricTimePlus, bool> parse(const QString &str);

    template<typename T>
    requires std::constructible_from<int64_t, T>
    [[nodiscard]] LyricTimePlus operator-(const T &rhs) const {
        LyricTimePlus time{};
        time._count = this->_count - static_cast<int64_t>(rhs);
        return time;
    }

    template<typename T>
    requires std::constructible_from<int64_t, T>
    [[nodiscard]] LyricTimePlus operator+(const T rhs) const {
        LyricTimePlus time{};
        time._count = this->_count + static_cast<int64_t>(rhs);
        return time;
    }

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

    template<typename T>
    requires std::constructible_from<int64_t, T>
    bool operator!=(const T& other) const {
        return this->_count != static_cast<int64_t>(other);
    }

    void offset(int64_t count);

    [[nodiscard]] QString toString(bool to_long, bool to_centi, bool to_dot) const;

    explicit operator int64_t() const;

    [[nodiscard]] LyricTimePlus toShort() const;

private:
    int64_t _count{0};
};


#endif //TTMLX_LYRICTIMEPLUS_H
