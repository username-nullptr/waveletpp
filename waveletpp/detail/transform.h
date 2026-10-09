// SPDX-FileCopyrightText: 2024-2026 Xiaoqiang <username_nullptr@163.com>
// SPDX-License-Identifier: MIT

#ifndef WAVELETPP_DETAIL_TRANSFORM_H
#define WAVELETPP_DETAIL_TRANSFORM_H

namespace waveletpp
{

template <typename T>
class WAVELETPP_TAPI basic_transform<T>::impl
{
public:
	using accumulator_t = std::common_type_t<value_t,double>;

	impl(vector_t src_data, const filter_arg &filter)
	{
		m_data.src = std::move(src_data);
		set_filter(filter);
	}
	explicit impl(const filter_arg &filter) {
		set_filter(filter);
	}

public:
	void set_filter(const filter_arg &filter)
	{
		m_filter = filter_arg_enum(filter);
		m_lo_d = lo_d_ref(m_filter);
		m_hi_d = hi_d_ref(m_filter);
		m_lo_r = lo_r_ref(m_filter);
		m_hi_r = hi_r_ref(m_filter);
	}

	void clear_decomposition_metadata() noexcept
	{
		m_reconstruction_size = 0;
		m_decomposition_low_size = 0;
		m_decomposition_high_size = 0;
		m_decomposition_extension = 0;

		m_has_decomposition_metadata = false;
		m_decomposition_was_extended = false;
		m_decomposition_is_passthrough = false;
	}

public:
	template <bool ApplyThreshold = false>
	void dwt(value_t threshold = type_tool<value_t>::zero)
	{
		const bool padded = m_data.src.size() % 2 != 0;
		const auto output_size = (m_data.src.size() + static_cast<size_t>(padded)) / 2;

		m_data.dec.high.resize(output_size);
		m_data.dec.low.reserve(output_size + output_size % 2);
		m_data.dec.low.resize(output_size);

		if( padded )
			m_data.src.emplace_back(m_data.src.back());

		const auto src_size = m_data.src.size();
		const auto filter_size = m_lo_d.size();

		if( filter_size == 2 )
		{
			for(size_t output=0; output<output_size; ++output)
			{
				const auto src_begin = output * 2;
				const auto first = m_data.src[src_begin];
				const auto second = m_data.src[src_begin + 1];

				m_data.dec.low[output] = first * m_lo_d[0] + second * m_lo_d[1];
				value_t high = first * m_hi_d[0] + second * m_hi_d[1];

				if constexpr( ApplyThreshold )
				{
					if( type_tool<value_t>::abs(high) < threshold )
						high = type_tool<value_t>::zero;
				}
				m_data.dec.high[output] = high;
			}
		}
		else if( filter_size <= src_size )
		{
			for(size_t output=0; output<output_size; ++output)
			{
				const auto src_begin = output * 2;
				const auto contiguous = std::min(filter_size, src_size - src_begin);

				accumulator_t d_low = 0;
				accumulator_t d_high = 0;

				for(size_t filter_index=0; filter_index<contiguous; ++filter_index)
				{
					const auto value = m_data.src[src_begin + filter_index];
					d_low += value * m_lo_d[filter_index];
					d_high += value * m_hi_d[filter_index];
				}
				for(size_t filter_index=contiguous; filter_index<filter_size; ++filter_index)
				{
					const auto value = m_data.src[filter_index - contiguous];
					d_low += value * m_lo_d[filter_index];
					d_high += value * m_hi_d[filter_index];
				}
				m_data.dec.low[output] = d_low;
				value_t high = d_high;

				if constexpr( ApplyThreshold )
				{
					if( type_tool<value_t>::abs(high) < threshold )
						high = type_tool<value_t>::zero;
				}
				m_data.dec.high[output] = high;
			}
		}
		else
		{
			for(size_t output=0; output<output_size; ++output)
			{
				auto src_index = output * 2;
				accumulator_t d_low = 0;
				accumulator_t d_high = 0;

				for(size_t filter_index=0; filter_index<filter_size; ++filter_index)
				{
					const auto value = m_data.src[src_index];
					d_low += value * m_lo_d[filter_index];
					d_high += value * m_hi_d[filter_index];

					if( ++src_index == src_size )
						src_index = 0;
				}
				m_data.dec.low[output] = d_low;
				value_t high = d_high;

				if constexpr( ApplyThreshold )
				{
					if( type_tool<value_t>::abs(high) < threshold )
						high = type_tool<value_t>::zero;
				}
				m_data.dec.high[output] = high;
			}
		}
		if( padded )
			m_data.src.pop_back();
	}

	void idwt()
	{
		auto &l_data = m_data.dec.low;
		if( l_data.empty() )
		{
			m_data.rec.clear();
			return ;
		}
		auto &h_data = m_data.dec.high;
		if( l_data.size() > h_data.size() )
			h_data.resize(l_data.size(), type_tool<value_t>::zero);

		const auto filter_size = m_lo_r.size();
		const auto rec_size = l_data.size() * 2;

		if( filter_size == 2 )
		{
			m_data.rec.resize(rec_size);
			for(size_t i=0; i<l_data.size(); ++i)
			{
				const auto low = l_data[i];
				const auto high = h_data[i];

				m_data.rec[i * 2] = low * m_lo_r[1] + high * m_hi_r[1];
				m_data.rec[i * 2 + 1] = low * m_lo_r[0] + high * m_hi_r[0];
			}
		}
		else if( filter_size <= rec_size )
		{
			m_data.rec.assign(rec_size, type_tool<value_t>::zero);
			for(size_t i=0; i<l_data.size(); ++i)
			{
				const auto dst_begin = i * 2;
				const auto contiguous = std::min(filter_size, rec_size - dst_begin);

				const auto low = l_data[i];
				const auto high = h_data[i];

				for(size_t j=0; j<contiguous; ++j)
				{
					const auto filter_index = filter_size - 1 - j;
					m_data.rec[dst_begin + j] += low * m_lo_r[filter_index] + high * m_hi_r[filter_index];
				}
				for(size_t j=contiguous; j<filter_size; ++j)
				{
					const auto filter_index = filter_size - 1 - j;
					m_data.rec[j - contiguous] += low * m_lo_r[filter_index] + high * m_hi_r[filter_index];
				}
			}
		}
		else
		{
			m_data.rec.assign(rec_size, type_tool<value_t>::zero);
			for(size_t i=0; i<l_data.size(); ++i)
			{
				auto dst = i * 2;
				const auto low = l_data[i];
				const auto high = h_data[i];

				for(size_t j=0; j<filter_size; ++j)
				{
					const auto filter_index = filter_size - 1 - j;
					m_data.rec[dst] += low * m_lo_r[filter_index] + high * m_hi_r[filter_index];

					if( ++dst == rec_size )
						dst = 0;
				}
			}
		}
	}

public:
	[[nodiscard]] static size_t default_extend(vector_t &data, size_t filter_size) // Symmetric
	{
		return default_extend_repeated(data, filter_size, 1);
	}

	[[nodiscard]] static size_t default_extend_repeated(vector_t &data, size_t filter_size, size_t repeats)
	{
		if( repeats == 0 )
			return 0;

		data = default_extended(data, filter_size, repeats);
		return filter_size * repeats;
	}

	[[nodiscard]] static vector_t default_extended(const vector_t &data, size_t filter_size, size_t repeats)
	{
		vector_t xdata;
		default_extend_into(data, filter_size, repeats, xdata);
		return xdata;
	}

	static void default_extend_into(const vector_t &data, size_t filter_size, size_t repeats, vector_t &xdata)
	{
		const auto src_size = data.size();
		if( filter_size > src_size )
			throw std::invalid_argument(
				"waveletpp: Boundary extension requires at least one filter length of data"
			);
		const auto ext_size = filter_size * repeats;

		const auto data_size = src_size + ext_size * 2;
		const auto level_scale = size_t {1} << repeats;

		const auto capacity = (data_size + level_scale - 1) / level_scale * level_scale;

		xdata.clear();
		xdata.reserve(capacity);
		xdata.resize(data_size, type_tool<value_t>::zero);

		std::copy(data.begin(), data.end(), xdata.begin() + ext_size);

		for(size_t block=0; block<repeats; ++block)
		{
			const auto left_begin = ext_size - (block + 1) * filter_size;
			const auto right_begin = ext_size + src_size + block * filter_size;
			const bool reversed = block % 2 == 0;

			for(size_t offset=0; offset<filter_size; ++offset)
			{
				xdata[left_begin + offset] = reversed ?
					data[filter_size - 1 - offset] : data[offset];

				xdata[right_begin + offset] = reversed ?
					data[src_size - 1 - offset] : data[src_size - filter_size + offset];
			}
		}
	}

	[[nodiscard]] size_t extend(vector_t &data, size_t filter_size, size_t repeats = 1)
	{
		if( m_default_ext )
			return default_extend_repeated(data, filter_size, repeats);

		if( not m_ext_func )
			throw std::invalid_argument("waveletpp: Empty boundary extension callback");

		size_t ext_size = 0;
		for(size_t i=0; i<repeats; ++i)
		{
			const auto previous_size = data.size();
			const auto added_per_side = m_ext_func(data, filter_size);

			if( added_per_side % 2 != 0 )
			{
				throw std::invalid_argument (
					"waveletpp: Boundary extension width must be even"
				);
			}
			if( added_per_side > (std::numeric_limits<size_t>::max() - previous_size) / 2 or
				data.size() != previous_size + added_per_side * 2 )
			{
				throw std::invalid_argument (
					"waveletpp: Boundary extension callback returned an inconsistent width"
				);
			}
			ext_size += added_per_side;
		}

		return ext_size;
	}

	[[nodiscard]] size_t extend_preserving
	(vector_t &data, vector_t &original, size_t filter_size, size_t repeats = 1, vector_t *workspace = nullptr)
	{
		if( m_default_ext )
		{
			original = std::move(data);
			if( workspace )
			{
				default_extend_into(original, filter_size, repeats, *workspace);
				data.swap(*workspace);
			}
			else
				data = default_extended(original, filter_size, repeats);
			return filter_size * repeats;
		}
		original = data;
		try
		{
			return extend(data, filter_size, repeats);
		}
		catch(...)
		{
			data = original;
			throw;
		}
	}

	void restore_preserved(vector_t &data, vector_t &original, vector_t *workspace = nullptr)
	{
		if( m_default_ext and workspace )
		{
			workspace->swap(data);
			workspace->clear();
		}
		data = std::move(original);
	}

	static void unexternd(vector_t &data, size_t ext_size)
	{
		if( ext_size == 0 )
			return;

		if( ext_size > data.size() / 2 )
			throw std::invalid_argument("waveletpp: Invalid boundary extension width");

		const auto src_size = data.size() - ext_size * 2;
		std::move (
			data.begin() + ext_size,
			data.begin() + ext_size + src_size,
			data.begin()
		);
		data.resize(src_size);
	}

public:
	filter_t m_filter {};

	detail::coefficient_view m_lo_d {};
	detail::coefficient_view m_hi_d {};

	detail::coefficient_view m_lo_r {};
	detail::coefficient_view m_hi_r {};

	data_t m_data {};
	std::array<vector_t,8> m_high_levels {};
	std::array<size_t,8> m_level_sizes {};
	std::array<vector_t,2> m_extension_buffers {};

	ext_method_t m_ext_func = default_extend;
	bool m_default_ext = true;

	size_t m_reconstruction_size = 0;
	size_t m_decomposition_low_size = 0;
	size_t m_decomposition_high_size = 0;
	size_t m_decomposition_extension = 0;

	bool m_has_decomposition_metadata = false;
	bool m_decomposition_was_extended = false;
	bool m_decomposition_is_passthrough = false;
};

template <typename T>
basic_transform<T>::basic_transform(vector_t src_data, const filter_arg &filter) :
	m_impl(new impl(std::move(src_data), filter))
{

}

template <typename T>
basic_transform<T>::basic_transform(const filter_arg &filter) :
	m_impl(new impl(filter))
{

}

template <typename T>
basic_transform<T>::~basic_transform()
{
	delete m_impl;
}

template <typename T>
basic_transform<T>::basic_transform(const basic_transform &other) :
	m_impl(new impl(*other.m_impl))
{

}

template <typename T>
basic_transform<T> &basic_transform<T>::operator=(const basic_transform &other)
{
	if( this != &other )
		*m_impl = *other.m_impl;
	return *this;
}

template <typename T>
basic_transform<T>::basic_transform(basic_transform &&other) noexcept :
	m_impl(new impl(std::move(*other.m_impl)))
{

}

template <typename T>
basic_transform<T> &basic_transform<T>::operator=(basic_transform &&other) noexcept
{
	if( this != &other )
		std::swap(m_impl, other.m_impl);
	return *this;
}

template <typename T>
basic_transform<T> &basic_transform<T>::set_filter(const filter_arg &filter)
{
	m_impl->set_filter(filter);
	m_impl->m_data.dec = {};
	m_impl->m_data.rec.clear();
	m_impl->clear_decomposition_metadata();
	return *this;
}

template <typename T>
basic_transform<T> &basic_transform<T>::set_src_data(vector_t src_data)
{
	m_impl->m_data.src = std::move(src_data);
	m_impl->m_data.dec = {};
	m_impl->m_data.rec.clear();
	m_impl->clear_decomposition_metadata();
	return *this;
}

template <typename T>
typename basic_transform<T>::decomposed_t &basic_transform<T>::dwt(bool ext)
{
	m_impl->m_data.rec.clear();
	m_impl->clear_decomposition_metadata();
	const auto reconstruction_size = m_impl->m_data.src.size();

	if( m_impl->m_data.src.size() < m_impl->m_lo_d.size() * 2 )
	{
		m_impl->m_data.dec.low = m_impl->m_data.src;
		m_impl->m_data.dec.high.clear();

		m_impl->m_reconstruction_size = reconstruction_size;
		m_impl->m_decomposition_low_size = m_impl->m_data.dec.low.size();
		m_impl->m_decomposition_high_size = 0;

		m_impl->m_has_decomposition_metadata = true;
		m_impl->m_decomposition_is_passthrough = true;

		return m_impl->m_data.dec;
	}
	size_t ext_size = 0;
	vector_t source;
	try {
		if( ext )
		{
			ext_size = m_impl->extend_preserving (
				m_impl->m_data.src, source, filter_size(), 1,
				&m_impl->m_extension_buffers[0]
			);
		}
		m_impl->template dwt<false>();

		if( ext )
		{
			m_impl->restore_preserved (
				m_impl->m_data.src, source, &m_impl->m_extension_buffers[0]
			);
		}
	}
	catch(...)
	{
		if( ext and not source.empty() )
		{
			m_impl->restore_preserved (
				m_impl->m_data.src, source, &m_impl->m_extension_buffers[0]
			);
		}
		m_impl->clear_decomposition_metadata();
		throw;
	}
	m_impl->m_reconstruction_size = reconstruction_size;
	m_impl->m_decomposition_low_size = m_impl->m_data.dec.low.size();

	m_impl->m_decomposition_high_size = m_impl->m_data.dec.high.size();
	m_impl->m_decomposition_extension = ext_size;

	m_impl->m_has_decomposition_metadata = true;
	m_impl->m_decomposition_was_extended = ext;

	return m_impl->m_data.dec;
}

template <typename T>
typename basic_transform<T>::vector_t &basic_transform<T>::idwt(bool ext)
{
	auto &low = m_impl->m_data.dec.low;
	auto &high = m_impl->m_data.dec.high;

	if( m_impl->m_has_decomposition_metadata )
	{
		if( low.size() != m_impl->m_decomposition_low_size or
			high.size() != m_impl->m_decomposition_high_size )
		{
			throw std::invalid_argument (
				"waveletpp::idwt: Decomposition coefficient sizes were changed"
			);
		}
		if( m_impl->m_decomposition_is_passthrough )
		{
			m_impl->m_data.rec = low;
			m_impl->m_data.rec.resize(m_impl->m_reconstruction_size);
			return m_impl->m_data.rec;
		}
	}
	else
	{
		if( low.empty() and high.empty() )
		{
			m_impl->m_data.rec.clear();
			return m_impl->m_data.rec;
		}
		if( low.empty() || high.empty() || low.size() != high.size() )
		{
			throw std::invalid_argument (
				"waveletpp::idwt: Low- and high-pass coefficient sizes must match"
			);
		}
		if( ext and low.size() < filter_size() )
		{
			throw std::invalid_argument (
				"waveletpp::idwt: Too few coefficients for boundary reconstruction"
			);
		}
	}
	m_impl->idwt();

	if( m_impl->m_has_decomposition_metadata )
	{
		const auto working_size = m_impl->m_reconstruction_size + m_impl->m_decomposition_extension * 2;

		if( m_impl->m_data.rec.size() < working_size )
			throw std::runtime_error("waveletpp::idwt: Reconstruction is shorter than expected");

		m_impl->m_data.rec.resize(working_size);
		if( m_impl->m_decomposition_was_extended )
			m_impl->unexternd(m_impl->m_data.rec, m_impl->m_decomposition_extension);

		m_impl->m_data.rec.resize(m_impl->m_reconstruction_size);
	}
	return m_impl->m_data.rec;
}

template <typename T>
typename basic_transform<T>::vector_t &basic_transform<T>::lpf(param_t param)
{
	if( not std::isfinite(static_cast<long double>(param.threshold)) or
		param.threshold < type_tool<value_t>::zero )
		throw std::invalid_argument("waveletpp::lpf: Threshold must be finite and non-negative");

	m_impl->clear_decomposition_metadata();
	if( m_impl->m_data.src.size() < m_impl->m_lo_d.size() * 2 or param.level == 0 )
	{
		m_impl->m_data.dec.high.clear();
		m_impl->m_data.dec.low.clear();

		m_impl->m_data.rec = m_impl->m_data.src;
		return m_impl->m_data.rec;
	}
	if( param.level > 8 )
		param.level = 8;

	vector_t source {};
	const auto ext_size = m_impl->extend_preserving (
		m_impl->m_data.src, source, filter_size(), param.level,
		&m_impl->m_extension_buffers[0]
	);
	const auto working_size = m_impl->m_data.src.size();
	auto &h_datas = m_impl->m_high_levels;
	auto &level_sizes = m_impl->m_level_sizes;

	for(level_t i=0; i<param.level; i++)
	{
		h_datas[i].clear();
		level_sizes[i] = m_impl->m_data.src.size();

		m_impl->m_data.dec.high.swap(h_datas[i]);
		m_impl->template dwt<true>(param.threshold);

		m_impl->m_data.dec.high.swap(h_datas[i]);
		m_impl->m_data.src.swap(m_impl->m_data.dec.low);
	}
	m_impl->m_data.dec.low.swap(m_impl->m_data.src);
	m_impl->restore_preserved (
		m_impl->m_data.src, source, &m_impl->m_extension_buffers[0]
	);
	m_impl->m_data.dec.low.reserve(working_size);
	m_impl->m_data.rec.reserve(working_size);

	for(size_t i=0; i<param.level; i++)
	{
		const auto level = param.level - 1 - i;

		m_impl->m_data.dec.high.swap(h_datas[level]);
		m_impl->idwt();

		if( m_impl->m_data.rec.size() < level_sizes[level] )
			throw std::runtime_error("waveletpp::lpf: Reconstruction is shorter than expected");

		m_impl->m_data.rec.resize(level_sizes[level]);
		m_impl->m_data.dec.high.swap(h_datas[level]);

		h_datas[level].clear();
		m_impl->m_data.dec.low.swap(m_impl->m_data.rec);
	}
	m_impl->m_data.rec.swap(m_impl->m_data.dec.low);
	m_impl->m_data.dec.low.clear();

	m_impl->m_data.dec.high.clear();
	m_impl->unexternd(m_impl->m_data.rec, ext_size);

	if( m_impl->m_data.rec.size() != m_impl->m_data.src.size() )
		throw std::runtime_error("waveletpp::lpf: Reconstruction size mismatch");

	return m_impl->m_data.rec;
}

template <typename T>
filter_t basic_transform<T>::filter() const noexcept
{
	return m_impl->m_filter;
}

template <typename T>
size_t basic_transform<T>::filter_size() const noexcept
{
	return m_impl->m_lo_d.size();
}

template <typename T>
const typename basic_transform<T>::data_t &basic_transform<T>::data() const noexcept
{
	return m_impl->m_data;
}

template <typename T>
typename basic_transform<T>::data_t &basic_transform<T>::data() noexcept
{
	return m_impl->m_data;
}

template <typename T>
basic_transform<T> &basic_transform<T>::on_extend(ext_method_t ext)
{
	if( not ext )
		throw std::invalid_argument("waveletpp::on_extend: Empty callback");

	m_impl->m_ext_func = std::move(ext);
	m_impl->m_default_ext = false;
	return *this;
}

template <typename T>
basic_transform<T> &basic_transform<T>::def_extend()
{
	m_impl->m_ext_func = impl::default_extend;
	m_impl->m_default_ext = true;
	return *this;
}

} //namespace waveletpp


#endif //WAVELETPP_DETAIL_TRANSFORM_H
