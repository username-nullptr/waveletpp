#ifndef WAVELETPP_DETAIL_TRANSFORM_H
#define WAVELETPP_DETAIL_TRANSFORM_H

#include <waveletpp/types.h>

namespace waveletpp
{

template <typename T>
class WAVELETPP_TAPI basic_transform<T>::impl
{
public:
	impl(vector_t src_data, const filter_arg &filter) :
		m_filter(filter_arg_enum(filter))
	{
		m_data.src = std::move(src_data);
	}

	explicit impl(const filter_arg &filter) :
		m_filter(filter_arg_enum(filter)) {}

public:
	void dwt()
	{
		m_data.dec.high.clear();
		m_data.dec.low.clear();

		if( m_data.src.size() % 2 )
			m_data.src.emplace_back(m_data.src.back());

		auto lo_d_a = lo_d(m_filter);
		auto hi_d_a = hi_d(m_filter);

		for(int i=0; i<m_data.src.size(); i+=2)
		{
			double d_low = 0;
			double d_high = 0;
			for(int j=0; j<lo_d_a.size(); j++)
			{
				int idx = (i + j + static_cast<int>(m_data.src.size())) % m_data.src.size();
				d_low += m_data.src[idx] * lo_d_a[j];
				d_high += m_data.src[idx] * hi_d_a[j];
			}
			m_data.dec.high.emplace_back(d_high);
			m_data.dec.low.emplace_back(d_low);
		}
		if( m_data.src.size() % 2 )
		{
			m_data.dec.high.pop_back();
			m_data.dec.low.pop_back();
		}
	}

	void idwt()
	{
		m_data.rec.clear();
		auto &l_data = m_data.dec.low;
		if( l_data.empty() )
			return ;

		auto &h_data = m_data.dec.high;
		while( l_data.size() > h_data.size() )
			h_data.emplace_back(0.0);

		auto lo_d_a = lo_d(m_filter);
		auto lo_r_a = lo_r(m_filter);
		auto hi_r_a = hi_r(m_filter);

		for(int i=0; i<static_cast<int>(l_data.size()); i++)
		{
			double d_high = 0.0;
			double d_low = 0.0;
			int j = 0;

			auto do_one = [&](int m) mutable
			{
				int idx = (m + static_cast<int>(l_data.size())) % l_data.size();
				d_high += h_data[idx] * hi_r_a[j];
				d_low += l_data[idx] * lo_r_a[j];
			};
			int offset = static_cast<int>(lo_d_a.size()) / 2;
			for(j=0; j<static_cast<int>(lo_d_a.size()); j+=2)
				do_one(i + j / 2 - offset);
			m_data.rec.emplace_back(d_low + d_high);

			d_high = d_low = 0.0;
			for(j=1; j<static_cast<int>(lo_d_a.size()); j+=2)
				do_one(i + (j + 1) / 2 - offset);
			m_data.rec.emplace_back(d_low + d_high);
		}
	}

public:
	[[nodiscard]] static size_t default_extend(vector_t &data, filter_t filter) // Symmetric
	{
		auto src_size = data.size();
		auto ext_size = lo_d(filter).size();

		std::vector xdata(src_size + ext_size * 2, type_tool<value_t>::zero);
		for(int i=0; i<static_cast<int>(src_size); i++)
			xdata[ext_size + i] = data[i];

		for(int i=0; i<static_cast<int>(ext_size); i++)
			xdata[i] = data[ext_size - 1 - i];
		for(int i=0; i<static_cast<int>(ext_size); i++)
			xdata[ext_size + src_size + i] = data[src_size - 1 - i];

		data = std::move(xdata);
		return ext_size;
	}

	static void unexternd(vector_t &data, size_t ext_size)
	{
		auto src_size = data.size() - ext_size * 2;
		std::vector xdata(src_size, type_tool<value_t>::zero);

		for(size_t i=0; i<src_size; i++)
			xdata[i] = data[ext_size + i];
		data = std::move(xdata);
	}

public:
	filter_t m_filter;
	data_t m_data;
	ext_method_t m_ext_func = default_extend;
};

template <typename T>
basic_transform<T>::basic_transform(vector_t src_data, const filter_arg &filter) :
	m_impl(new impl(src_data, filter))
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
	if( this == &other )
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
	if( this == &other )
		*m_impl = std::move(*other.m_impl);
	return *this;
}

template <typename T>
basic_transform<T> &basic_transform<T>::set_filter(const filter_arg &filter)
{
	m_impl->m_filter = filter_arg_enum(filter);
	return *this;
}

template <typename T>
basic_transform<T> &basic_transform<T>::set_src_data(vector_t src_data)
{
	m_impl->m_data.src = std::move(src_data);
	return *this;
}

template <typename T>
typename basic_transform<T>::decomposed_t &basic_transform<T>::dwt(bool ext)
{
	if( m_impl->m_data.src.size() < 3 )
	{
		m_impl->m_data.dec.low = m_impl->m_data.src;
		m_impl->m_data.dec.high.clear();
		return m_impl->m_data.dec;
	}
	size_t ext_size = 0;
	if( ext )
		ext_size = m_impl->m_ext_func(m_impl->m_data.src, m_impl->m_filter);

	m_impl->dwt();
	if( ext )
	{
		m_impl->unexternd(m_impl->m_data.dec.high, ext_size / 2);
		m_impl->unexternd(m_impl->m_data.dec.low, ext_size / 2);
	}
	return m_impl->m_data.dec;
}

template <typename T>
typename basic_transform<T>::vector_t &basic_transform<T>::idwt(bool ext)
{
	if( m_impl->m_data.dec.low.empty() )
	{
		m_impl->m_data.rec.clear();
		return m_impl->m_data.rec;
	}
	size_t ext_size[2] {0,0};
	if( ext )
	{
		ext_size[0] = m_impl->m_ext_func(m_impl->m_data.dec.high, m_impl->m_filter);
		ext_size[1] = m_impl->m_ext_func(m_impl->m_data.dec.low, m_impl->m_filter);
	}
	m_impl->idwt();

	if( ext )
		m_impl->unexternd(m_impl->m_data.rec, ext_size[0] + ext_size[1]);
	return m_impl->m_data.rec;
}

template <typename T>
typename basic_transform<T>::vector_t &basic_transform<T>::lpf(T threshold, level_t level)
{
	if( m_impl->m_data.src.size() < 3 && level == 0 )
		return m_impl->m_data.src;
	if( level > 8 )
		level = 8;

	size_t ext_size = 0;
	for(level_t i=0; i<level; i++)
		ext_size += m_impl->m_ext_func(m_impl->m_data.src, m_impl->m_filter);

	auto tmp = m_impl->m_data.src;
	std::vector<vector_t> h_datas;

	for(level_t i=0; i<level; i++)
	{
		m_impl->dwt();
		for(auto &data : m_impl->m_data.dec.high)
		{
			if( type_tool<value_t>::abs(data) < threshold )
				data = type_tool<value_t>::zero;
		}
		h_datas.emplace_back(std::move(m_impl->m_data.dec.high));
		m_impl->m_data.src = std::move(m_impl->m_data.dec.low);
	}
	m_impl->m_data.dec.low = std::move(m_impl->m_data.src);
	m_impl->m_data.src = std::move(tmp);

	for(size_t i=0; i<h_datas.size(); i++)
	{
		m_impl->m_data.dec.high = std::move(h_datas[h_datas.size() - 1 - i]);
		m_impl->idwt();
		m_impl->m_data.dec.low = std::move(m_impl->m_data.rec);
	}
	m_impl->m_data.rec = std::move(m_impl->m_data.dec.low);
	m_impl->m_data.dec.high.clear();

	m_impl->unexternd(m_impl->m_data.rec, ext_size);
	return m_impl->m_data.rec;
}

template <typename T>
typename basic_transform<T>::vector_t &basic_transform<T>::lpf(level_t level)
{
	return lpf(type_tool<value_t>::threshold, level);
}

template <typename T>
filter_t basic_transform<T>::filter() const noexcept
{
	return m_impl->m_filter;
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
	m_impl->m_ext_func = std::move(ext);
	return *this;
}

template <typename T>
basic_transform<T> &basic_transform<T>::def_extend()
{
	m_impl->m_ext_func = impl::default_extend;
	return *this;
}

} //namespace waveletpp


#endif //WAVELETPP_DETAIL_TRANSFORM_H
