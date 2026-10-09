// SPDX-FileCopyrightText: 2024-2026 Xiaoqiang <username_nullptr@163.com>
// SPDX-License-Identifier: MIT

#ifndef WAVELETPP_DETAIL_FILTER_TYPES_H
#define WAVELETPP_DETAIL_FILTER_TYPES_H

namespace waveletpp::detail
{

constexpr std::size_t filter_count =
#define X_MACRO(e,n) + 1
	0 WAVELETPP_FILTER_LIST;
#undef X_MACRO

class WAVELETPP_VAPI coefficient_view
{
public:
	coefficient_view() = default;
	coefficient_view(const double *data, std::size_t size) :
		m_data(data), m_size(size) {}

	[[nodiscard]] const double *begin() const noexcept { return m_data; }
	[[nodiscard]] const double *end() const noexcept { return m_data + m_size; }
	[[nodiscard]] std::size_t size() const noexcept { return m_size; }

	[[nodiscard]] const double &operator[](std::size_t index) const noexcept {
		return m_data[index];
	}

private:
	const double *m_data = nullptr;
	std::size_t m_size = 0;
};

template <typename, filter>
struct filter_name {};

#define X_MACRO(e,n) \
	template <typename CharT> \
	struct filter_name<CharT, filter::e> { \
		static constexpr const CharT *get() { \
			if constexpr( std::is_same_v<CharT,char> ) \
				return n; \
			else \
				return L##n; \
		} \
	};
	WAVELETPP_FILTER_LIST
#undef X_MACRO

} //namespace waveletpp::detail


#endif //WAVELETPP_DETAIL_FILTER_TYPES_H
