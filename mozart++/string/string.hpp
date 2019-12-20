/**
 * Mozart++ Template Library: String
 * Licensed under MIT License
 * Copyright (c) 2019 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#pragma once

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <bitset>
#include <mozart++/core/iterator_range.hpp>
#include <mozart++/stream>
#include <mozart++/function>

namespace mpp {
    /**
     * Represent a constant reference to a string, i.e. a character
     * array and a length, which need not be null terminated.
     *
     * This class does not own the string data, it is expected to be used in
     * situations where the character data resides in some other buffer, whose
     * lifetime extends past that of the string_ref. For this reason, it is not in
     * general safe to store a string_ref.
     */
    class string_ref {
    public:
        static const size_t npos = ~size_t(0);

        using iterator = const char *;
        using const_iterator = const char *;
        using size_type = size_t;

    private:
        /**
         * The start of the string, in an external buffer.
         */
        const char *_data = nullptr;

        /**
         * The length of the string.
         */
        size_t _length = 0;

        /**
         * Workaround memcmp issue with null pointers (undefined behavior)
         * by providing a specialized version.
         *
         * @param lhs
         * @param rhs
         * @param length
         * @return
         */
        static int safe_memcmp(const char *lhs, const char *rhs, size_t length) {
            if (length == 0) { return 0; }
            return ::memcmp(lhs, rhs, length);
        }

        /**
         * Constexpr version of std::strlen.
         */
        static size_t string_length(const char *str) {
            return std::char_traits<char>::length(str);
        }

        static int ascii_strncasecmp(const char *lhs, const char *rhs, size_t Length) {
            for (size_t I = 0; I < Length; ++I) {
                unsigned char lw = std::tolower(lhs[I]);
                unsigned char rw = std::tolower(rhs[I]);
                if (lw != rw) {
                    return lw < rw ? -1 : 1;
                }
            }
            return 0;
        }

    public:
        /**
         * Wrap a string.
         * If data is nullptr, this function will wrap an empty string.
         *
         * @param data string
         * @return string_ref
         */
        static string_ref with(const char *data) {
            return string_ref{data ? data : ""};
        }

    public:
        char operator[](size_t Index) const {
            // TODO: replace with exception handling system.
            assert(Index < _length && "Invalid index!");
            return _data[Index];
        }

        /**
         * Disallow accidental assignment from a temporary std::string.
         * The declaration here is extra complicated so that `string_ref = {}`
         * and `string_ref = "abc"` continue to select the move assignment operator.
         *
         * @tparam T
         * @return
         */
        template <typename T>
        std::enable_if_t<std::is_same<T, std::string>::value, string_ref> &
        operator=(T &&) = delete;

        /*implicit*/ operator std::string() const {
            return str();
        }

    public:
        /*implicit*/ string_ref() = default;

        string_ref(std::nullptr_t) = delete;

        /*implicit*/ constexpr string_ref(const char *str)
            : _data(str), _length(str ? string_length(str) : 0) {}

        /*implicit*/ constexpr string_ref(const char *data, size_t length)
            : _data(data), _length(length) {}

        /*implicit*/ string_ref(const std::string &str)
            : _data(str.data()), _length(str.length()) {}

        iterator begin() const { return _data; }

        iterator end() const { return _data + _length; }

        const unsigned char *bytes_begin() const {
            return reinterpret_cast<const unsigned char *>(begin());
        }

        const unsigned char *bytes_end() const {
            return reinterpret_cast<const unsigned char *>(end());
        }

        mpp::iterator_range<const unsigned char *> bytes() const {
            return make_range(bytes_begin(), bytes_end());
        }

        /**
         * Get a pointer to the start of the string (which may not be null
         * terminated).
         *
         * @return char array
         */
        const char *data() const { return _data; }

        /**
         * Check if the string is empty.
         *
         * @return is empty?
         */
        bool empty() const { return _length == 0; }

        /**
         * Get the length of the string.
         *
         * @return length
         */
        size_t size() const { return _length; }

        /**
         * Get the first character in the string.
         *
         * @return the first char
         */
        char front() const {
            // TODO: replace with exception handling system.
            assert(!empty());
            return _data[0];
        }

        /**
         * Get the last character in the string.
         *
         * @return the last char
         */
        char back() const {
            // TODO: replace with exception handling system.
            assert(!empty());
            return _data[_length - 1];
        }

        /**
         * Allocate copy in Allocator and return string_ref to it.
         * @tparam Allocator Allocator Type
         * @param allocator allocator
         * @return copied string_ref(with data)
         */
        template <typename Allocator>
        string_ref copy(Allocator &allocator) const {
            if (empty()) {
                return string_ref{};
            }
            char *data = allocator.template allocate<char>(_length);
            std::copy(begin(), end(), data);
            return string_ref{data, _length};
        }

        /**
         * Check for string equality.
         *
         * @param rhs the other string
         * @return is equal?
         */
        bool equals(string_ref rhs) const {
            return (_length == rhs._length &&
                    safe_memcmp(_data, rhs._data, rhs._length) == 0);
        }

        /**
         * Check for string equality, case insensitively.
         *
         * @param rhs
         * @return is equal case insensitively?
         */
        bool equals_ignore_case(string_ref rhs) const {
            return _length == rhs._length && compare_ignore_case(rhs) == 0;
        }

        /**
         * Compare two strings.
         * The result is -1, 0, or 1 if this string is lexicographically
         * less than, equal to, or greater than the rhs.
         *
         * @param rhs
         * @return -1, 0 or 1
         */
        int compare(string_ref rhs) const {
            // Check the prefix for a mismatch.
            int r = safe_memcmp(_data, rhs._data, std::min(_length, rhs._length));
            if (r) {
                return r < 0 ? -1 : 1;
            }

            // Otherwise the prefixes match, so we only need to check the lengths.
            if (_length == rhs._length) {
                return 0;
            }
            return _length < rhs._length ? -1 : 1;
        }

        /**
         * Compare two strings, case insensitively.
         * @param rhs
         * @return {@see string_ref::compare(string_ref)}
         */
        int compare_ignore_case(string_ref rhs) const {
            int r = ascii_strncasecmp(_data, rhs._data, std::min(_length, rhs._length));
            if (r) {
                return r;
            }

            if (_length == rhs._length) {
                return 0;
            }
            return _length < rhs._length ? -1 : 1;
        }

        /**
         * Compare two strings, treating sequences of digits as numbers.
         * @param rhs
         * @return
         */
        int compare_numeric(string_ref rhs) const {
            for (size_t i = 0, end = std::min(_length, rhs._length); i != end; ++i) {
                // Check for sequences of digits.
                if (std::isdigit(_data[i]) && std::isdigit(rhs._data[i])) {
                    // The longer sequence of numbers is considered larger.
                    // This doesn't really handle prefixed zeros well.
                    size_t j = 0;
                    for (j = i + 1; j != end + 1; ++j) {
                        bool ld = j < _length && std::isdigit(_data[j]);
                        bool rd = j < rhs._length && std::isdigit(rhs._data[j]);
                        if (ld != rd) {
                            return rd ? -1 : 1;
                        }
                        if (!rd) {
                            break;
                        }
                    }

                    // The two number sequences have the same length (j-i), just memcmp them.
                    int r = safe_memcmp(_data + i, rhs._data + i, j - i);
                    if (r) {
                        return r < 0 ? -1 : 1;
                    }
                    // Identical number sequences, continue search after the numbers.
                    i = j - 1;
                    continue;
                }

                if (_data[i] != rhs._data[i]) {
                    return (unsigned char) _data[i] < (unsigned char) rhs._data[i] ? -1 : 1;
                }
            }

            if (_length == rhs._length) {
                return 0;
            }
            return _length < rhs._length ? -1 : 1;
        }

        /**
         * Get the contents as an std::string.
         * @return
         */
        std::string str() const {
            if (!_data) { return std::string(); }
            return std::string(_data, _length);
        }

        const char *printable() const {
            return str().c_str();
        }

        /**
         * Check if this string starts with the given prefix.
         * @param prefix
         * @return
         */
        bool startswith(string_ref prefix) const {
            return _length >= prefix._length &&
                   safe_memcmp(_data, prefix._data, prefix._length) == 0;
        }

        /**
         * Check if this string starts with the given prefix, case insensitively.
         * @param prefix
         * @return
         */
        bool startswith_ignore_case(string_ref prefix) const {
            return _length >= prefix._length &&
                   ascii_strncasecmp(_data, prefix._data, prefix._length) == 0;
        }

        /**
         * Check if this string ends with the given suffix.
         * @param suffix
         * @return
         */
        bool endswith(string_ref suffix) const {
            return _length >= suffix._length &&
                   safe_memcmp(end() - suffix._length, suffix._data, suffix._length) == 0;
        }

        /**
         * Check if this string ends with the given prefix, case insensitively.
         * @param prefix
         * @return
         */
        bool endswith_ignore_case(string_ref suffix) const {
            return _length >= suffix._length &&
                   ascii_strncasecmp(end() - suffix._length, suffix._data, suffix._length) == 0;
        }

        /**
         * Search for the first character c in the string.
         * @param c
         * @param start_index
         * @return index of the first c, or npos if not found
         */
        size_t find(char c, size_t start_index = 0) const {
            size_t pos = std::min(start_index, _length);
            if (pos < _length) {
                // Avoid calling memchr with nullptr.
                // Just forward to memchr, which is faster than a hand-rolled loop.
                if (const void *s = ::memchr(_data + pos, c, _length - pos)) {
                    return static_cast<const char *>(s) - _data;
                }
            }
            return npos;
        }

        /**
         * Search for the first character c in the string, case insensitively.
         * @param c
         * @param start_index
         * @return index of the first c, or npos if not found
         */
        size_t find_ignore_case(char c, size_t start_index = 0) const {
            int lc = std::tolower(c);
            return find_if(
                [lc](char d) { return std::tolower(d) == lc; },
                start_index
            );
        }

        /**
         * Search for the first character satisfying the predicate f
         * @param f
         * @param start_index
         * @return the position or npos
         */
        size_t find_if(const mpp::function<bool(char)> &f, size_t start_index = 0) const {
            string_ref s = drop_front(start_index);
            while (!s.empty()) {
                if (f(s.front())) {
                    return size() - s.size();
                }
                s = s.drop_front();
            }
            return npos;
        }

        /**
         * Search for the first character not satisfying the predicate f
         * @param f
         * @param start_index
         * @return
         */
        size_t find_if_not(const mpp::function<bool(char)> &f, size_t start_index = 0) const {
            return find_if(
                [&f](char c) { return !f(c); },
                start_index
            );
        }

        size_t find(string_ref str, size_t start_index = 0) const {
            if (start_index > _length) {
                return npos;
            }

            const char *start = _data + start_index;
            size_t size = _length - start_index;

            const char *needle = str.data();
            size_t N = str.size();
            if (N == 0) {
                return start_index;
            }
            if (size < N) {
                return npos;
            }
            if (N == 1) {
                const char *p = (const char *) ::memchr(start, needle[0], size);
                return p == nullptr ? npos : p - _data;
            }

            const char *end = start + (size - N + 1);

            // For short haystacks or unsupported needles fall back to the naive algorithm
            if (size < 16 || N > 255) {
                do {
                    if (std::memcmp(start, needle, N) == 0)
                        return start - _data;
                    ++start;
                } while (start < end);
                return npos;
            }

            // Build the bad char heuristic table, with uint8_t to reduce cache thrashing.
            uint8_t skipped[256];
            std::memset(skipped, N, 256);
            for (unsigned i = 0; i != N - 1; ++i) {
                skipped[(uint8_t) str[i]] = N - 1 - i;
            }

            do {
                uint8_t last = start[N - 1];
                if (last == (uint8_t) needle[N - 1]
                    && std::memcmp(start, needle, N - 1) == 0) {
                    return start - _data;
                }
                // Otherwise skip the appropriate number of bytes.
                start += skipped[last];
            } while (start < end);

            return npos;
        }

        size_t find_ignore_case(string_ref str, size_t start_index = 0) const {
            string_ref _this = substr(start_index);
            while (_this.size() >= str.size()) {
                if (_this.startswith_ignore_case(str)) {
                    return start_index;
                }
                _this = _this.drop_front();
                ++start_index;
            }
            return npos;
        }

        size_t rfind(char c, size_t start_index = npos) const {
            start_index = std::min(start_index, _length);
            size_t i = start_index;
            while (i != 0) {
                --i;
                if (_data[i] == c) {
                    return i;
                }
            }
            return npos;
        }

        size_t rfind_ignore_case(char c, size_t start_index = npos) const {
            start_index = std::min(start_index, _length);
            size_t i = start_index;
            while (i != 0) {
                --i;
                if (std::tolower(_data[i]) == std::tolower(c)) {
                    return i;
                }
            }
            return npos;
        }

        size_t rfind(string_ref str) const {
            size_t N = str.size();
            if (N > _length) {
                return npos;
            }
            for (size_t i = _length - N + 1; i != 0;) {
                --i;
                if (substr(i, N).equals(str)) {
                    return i;
                }
            }
            return npos;
        }

        size_t rfind_ignore_case(string_ref str) const {
            size_t N = str.size();
            if (N > _length) {
                return npos;
            }
            for (size_t i = _length - N + 1; i != 0;) {
                --i;
                if (substr(i, N).equals_ignore_case(str)) {
                    return i;
                }
            }
            return npos;
        }

        size_t find_first_of(char C, size_t From = 0) const {
            return find(C, From);
        }

        size_t find_first_of(string_ref chars, size_t start_index = 0) const {
            std::bitset<1 << CHAR_BIT> char_bits;
            for (size_type i = 0; i != chars.size(); ++i) {
                char_bits.set((unsigned char) chars[i]);
            }

            for (size_type i = std::min(start_index, _length); i != _length; ++i) {
                if (char_bits.test((unsigned char) _data[i])) {
                    return i;
                }
            }
            return npos;
        }

        size_t find_first_not_of(char c, size_t start_index = 0) const {
            for (size_type i = std::min(start_index, _length); i != _length; ++i) {
                if (_data[i] != c) {
                    return i;
                }
            }
            return npos;
        }

        size_t find_first_not_of(string_ref chars, size_t start_index = 0) const {
            std::bitset<1 << CHAR_BIT> char_bits;
            for (size_type i = 0; i != chars.size(); ++i) {
                char_bits.set((unsigned char) chars[i]);
            }

            for (size_type i = std::min(start_index, _length); i != _length; ++i) {
                if (!char_bits.test((unsigned char) _data[i])) {
                    return i;
                }
            }
            return npos;
        }

        size_t find_last_of(char c, size_t start_index = npos) const {
            return rfind(c, start_index);
        }

        size_t find_last_of(string_ref chars, size_t start_index = npos) const {
            std::bitset<1 << CHAR_BIT> char_bits;
            for (size_type i = 0; i != chars.size(); ++i) {
                char_bits.set((unsigned char) chars[i]);
            }

            for (size_type i = std::min(start_index, _length) - 1; i != -1; --i) {
                if (char_bits.test((unsigned char) _data[i])) {
                    return i;
                }
            }
            return npos;
        }

        size_t find_last_not_of(char c, size_t start_index = npos) const {
            for (size_type i = std::min(start_index, _length) - 1; i != -1; --i) {
                if (_data[i] != c) {
                    return i;
                }
            }
            return npos;
        }

        size_t find_last_not_of(string_ref chars, size_t start_index = npos) const {
            std::bitset<1 << CHAR_BIT> char_bits;
            for (size_type i = 0, e = chars.size(); i != e; ++i) {
                char_bits.set((unsigned char) chars[i]);
            }

            for (size_type i = std::min(start_index, _length) - 1; i != -1; --i) {
                if (!char_bits.test((unsigned char) _data[i])) {
                    return i;
                }
            }
            return npos;
        }

        bool contains(string_ref other) const { return find(other) != npos; }

        bool contains(char c) const { return find_first_of(c) != npos; }

        bool contains_ignore_case(string_ref other) const {
            return find_ignore_case(other) != npos;
        }

        bool contains_ignore_case(char c) const { return find_ignore_case(c) != npos; }

        size_t count(char c) const {
            size_t count = 0;
            for (size_t i = 0; i != _length; ++i) {
                if (_data[i] == c) {
                    ++count;
                }
            }
            return count;
        }

        size_t count(string_ref str) const {
            size_t count = 0;
            size_t N = str.size();
            if (N > _length) {
                return 0;
            }
            for (size_t i = 0, end = _length - N + 1; i != end; ++i) {
                if (substr(i, N).equals(str)) {
                    ++count;
                }
            }
            return count;
        }

        // Convert the given ASCII string to lowercase.
        std::string lower() const {
            std::string result(size(), char());
            for (size_type i = 0, e = size(); i != e; ++i) {
                result[i] = std::tolower(_data[i]);
            }
            return result;
        }

        /// Convert the given ASCII string to uppercase.
        std::string upper() const {
            std::string result(size(), char());
            for (size_type i = 0, e = size(); i != e; ++i) {
                result[i] = std::toupper(_data[i]);
            }
            return result;
        }

        /**
         * Return a reference to the substring from [start_index, start_index + N).
         *
         * @param start_index The index of the starting character in the substring; if
         * the index is npos or greater than the length of the string then the
         * empty substring will be returned.
         * @param N The number of characters to included in the substring. If N
         * exceeds the number of characters remaining in the string, the string
         * suffix (starting with start_index) will be returned.
         *
         * @return
         */
        string_ref substr(size_t start_index, size_t N = npos) const {
            start_index = std::min(start_index, _length);
            return string_ref{_data + start_index,
                              std::min(N, _length - start_index)};
        }

        /**
         * Return a string_ref equal to 'this' but with only the first N
         * elements remaining.  If N is greater than the length of the
         * string, the entire string is returned.
         *
         * @param N
         * @return
         */
        string_ref take_front(size_t N = 1) const {
            if (N >= size())
                return *this;
            return drop_back(size() - N);
        }

        /**
         * Return a string_ref equal to 'this' but with only the last \p N
         * elements remaining.  If N is greater than the length of the
         * string, the entire string is returned.
         *
         * @param N
         * @return
         */
        string_ref take_back(size_t N = 1) const {
            if (N >= size())
                return *this;
            return drop_front(size() - N);
        }

        /**
         * Return the longest prefix of 'this' such that every character
         * in the prefix satisfies the given predicate.
         *
         * @param f
         * @return
         */
        string_ref take_while(const mpp::function<bool(char)> &f) const {
            return substr(0, find_if_not(f));
        }

        /**
         * Return the longest prefix of 'this' such that no character in
         * the prefix satisfies the given predicate.
         *
         * @param f
         * @return
         */
        string_ref take_until(const mpp::function<bool(char)> &f) const {
            return substr(0, find_if(f));
        }

        /**
         * Return a string_ref equal to 'this' but with the first N elements
         * dropped.
         *
         * @param N
         * @return
         */
        string_ref drop_front(size_t N = 1) const {
            // TODO: replace with exception handling system.
            assert(size() >= N && "Dropping more elements than exist");
            return substr(N);
        }

        /**
         * Return a string_ref equal to 'this' but with the last N elements
         * dropped.
         *
         * @param N
         * @return
         */
        string_ref drop_back(size_t N = 1) const {
            // TODO: replace with exception handling system.
            assert(size() >= N && "Dropping more elements than exist");
            return substr(0, size() - N);
        }

        /**
         * Return a string_ref equal to 'this', but with all characters satisfying
         * the given predicate dropped from the beginning of the string.
         *
         * @param f
         * @return
         */
        string_ref drop_while(const mpp::function<bool(char)> &f) const {
            return substr(find_if_not(f));
        }

        /**
         * Return a string_ref equal to 'this', but with all characters not
         * satisfying the given predicate dropped from the beginning of the string.
         *
         * @param f
         * @return
         */
        string_ref drop_until(const mpp::function<bool(char)> &f) const {
            return substr(find_if(f));
        }

        /**
         * Return a reference to the substring from [start, end).
         * @param start The index of the starting character in the substring; if
         * the index is npos or greater than the length of the string then the
         * empty substring will be returned.
         *
         * @param end The index following the last character to include in the
         * substring. If this is npos or exceeds the number of characters
         * remaining in the string, the string suffix (starting with start)
         * will be returned. If this is less than start, an empty string will
         * be returned.
         *
         * @return
         */
        string_ref slice(size_t start, size_t end) const {
            start = std::min(start, _length);
            end = std::min(std::max(start, end), _length);
            return string_ref{_data + start, end - start};
        }

        /**
         * Split into two substrings around the first occurrence of a separator
         * character.
         *
         * If separator is in the string, then the result is a pair (LHS, RHS)
         * such that (*this == LHS + separator + RHS) is true and RHS is
         * maximal. If separator is not in the string, then the result is a
         * pair (LHS, RHS) where (*this == LHS) and (RHS == "").
         *
         * @param separator
         * @return
         */
        std::pair<string_ref, string_ref> split(char separator) const {
            return split(string_ref(&separator, 1));
        }

        std::pair<string_ref, string_ref> split(string_ref separator) const {
            size_t index = find(separator);
            if (index == npos) {
                return std::make_pair(*this, string_ref());
            }
            return std::make_pair(slice(0, index),
                slice(index + separator.size(), npos));
        }

        /**
         * Split into two substrings around the last occurrence of a separator
         * character.
         *
         * If separator is in the string, then the result is a pair (LHS, RHS)
         * such that (*this == LHS + separator + RHS) is true and RHS is
         * minimal. If separator is not in the string, then the result is a
         * pair (LHS, RHS) where (*this == LHS) and (RHS == "").
         *
         * @param separator
         * @return
         */
        std::pair<string_ref, string_ref> rsplit(char separator) const {
            return rsplit(string_ref(&separator, 1));
        }

        std::pair<string_ref, string_ref> rsplit(string_ref separator) const {
            size_t index = rfind(separator);
            if (index == npos) {
                return std::make_pair(*this, string_ref());
            }
            return std::make_pair(slice(0, index),
                slice(index + separator.size(), npos));
        }

        /**
         * Split into substrings around the occurrences of a separator string.
         *
         * Each substring is stored in result. If max_split is >= 0, at most
         * max_split splits are done and consequently <= max_split + 1
         * elements are added to result.
         * If keep_empty is false, empty strings are not added to result. They
         * still count when considering max_split
         * An useful invariant is that
         * separator.join(result) == *this if max_split == -1 and keep_empty == true
         *
         * @param result Where to put the substrings.
         * @param separator The string to split on.
         * @param max_split  The maximum number of times the string is split.
         * @param keep_empty True if empty substring should be added.
         */
        void split(std::vector<string_ref> &result,
                   string_ref separator, int max_split = -1,
                   bool keep_empty = true) const {
            string_ref str = *this;

            while (max_split-- != 0) {
                size_t index = str.find(separator);
                if (index == npos) {
                    break;
                }

                // Push this split.
                if (keep_empty || index > 0)
                    result.push_back(str.slice(0, index));

                // Jump forward.
                str = str.slice(index + separator.size(), npos);
            }

            // Push the tail.
            if (keep_empty || !str.empty()) {
                result.push_back(str);
            }
        }

        void split(std::vector<string_ref> &result, char separator, int max_split = -1,
                   bool keep_empty = true) const {
            string_ref str = *this;

            while (max_split-- != 0) {
                size_t index = str.find(separator);
                if (index == npos) {
                    break;
                }

                // Push this split.
                if (keep_empty || index > 0) {
                    result.push_back(str.slice(0, index));
                }

                // Jump forward.
                str = str.slice(index + 1, npos);
            }

            // Push the tail.
            if (keep_empty || !str.empty()) {
                result.push_back(str);
            }
        }

        string_ref ltrim(char chars) const {
            return drop_front(std::min(_length, find_first_not_of(chars)));
        }

        string_ref ltrim(string_ref chars = " \t\n\v\f\r") const {
            return drop_front(std::min(_length, find_first_not_of(chars)));
        }

        string_ref rtrim(char chars) const {
            return drop_back(_length - std::min(_length, find_last_not_of(chars) + 1));
        }

        string_ref rtrim(string_ref chars = " \t\n\v\f\r") const {
            return drop_back(_length - std::min(_length, find_last_not_of(chars) + 1));
        }

        string_ref trim(char chars) const {
            return ltrim(chars).rtrim(chars);
        }

        string_ref trim(string_ref chars = " \t\n\v\f\r") const {
            return ltrim(chars).rtrim(chars);
        }

        mpp::stream<char> stream() {
            std::deque<char> d;
            std::copy(begin(), end(), std::back_inserter(d));
            return mpp::stream<char>::of(std::move(d));
        }
    };
}
