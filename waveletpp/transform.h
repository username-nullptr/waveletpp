#ifndef WAVELETPP_TRANSFORM_H
#define WAVELETPP_TRANSFORM_H

#include <waveletpp/filter.h>
#include <functional>

namespace waveletpp
{

template <typename T>
class WAVELETPP_TAPI basic_transform
{
public:
	using value_t = T;
	using vector_t = std::vector<value_t>;

	using level_t = size_t;
	using ext_method_t = std::function<size_t(vector_t&,filter_t)>;

	struct decomposed_t
	{
		vector_t high {};
		vector_t low {};
	};
	struct data_t
	{
		vector_t src {};
		vector_t rec {};
		decomposed_t dec {};
	};

public:
	explicit basic_transform(vector_t src_data, const filter_arg &filter = filter_t::db1);
	explicit basic_transform(const filter_arg &filter = filter_t::db1);
	~basic_transform();

	basic_transform(const basic_transform &other);
	basic_transform &operator=(const basic_transform &other);

	basic_transform(basic_transform &&other) noexcept;
	basic_transform &operator=(basic_transform &&other) noexcept;

public:
	basic_transform &set_filter(const filter_arg &filter);
	basic_transform &set_src_data(vector_t src_data);

	decomposed_t &dwt(bool ext = true);
	vector_t &idwt(bool ext = true);

	vector_t &lpf(T threshold, level_t level = 1);
	vector_t &lpf(level_t level = 1);

public:
	[[nodiscard]] filter_t filter() const noexcept;
	[[nodiscard]] const data_t &data() const noexcept;
	[[nodiscard]] data_t &data() noexcept;

public:
	basic_transform &on_extend(ext_method_t ext);
	basic_transform &def_extend();

private:
	class impl;
	impl *m_impl;
};

using int_transform = basic_transform<int>;
using double_transform = basic_transform<double>;
using transform = double_transform;

} //namespace waveletpp
#include <waveletpp/detail/transform.h>


#endif //WAVELETPP_TRANSFORM_H