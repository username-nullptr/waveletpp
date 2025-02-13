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
			double d_jg = 0;
			double d_gt = 0;
			for(int j=0; j<lo_d_a.size(); j++)
			{
				if( i + j - 3 < 0 )
				{
					d_jg += m_data.src[m_data.src.size() + i + j - 3] * lo_d_a[j];
					d_gt += m_data.src[m_data.src.size() + i + j - 3] * hi_d_a[j];
				}
				else if( i + j - 3 > m_data.src.size() - 1 )
				{
					d_jg += m_data.src[i + j - 3 - m_data.src.size()] * lo_d_a[j];
					d_gt += m_data.src[i + j - 3 - m_data.src.size()] * hi_d_a[j];
				}
				else
				{
					d_jg += m_data.src[i + j - 3] * lo_d_a[j];
					d_gt += m_data.src[i + j - 3] * hi_d_a[j];
				}
			}
			m_data.dec.high.emplace_back(d_gt);
			m_data.dec.low.emplace_back(d_jg);
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

		for(int i=0; i<l_data.size(); i++)
		{
			double d_jg = 0.0;
			double d_gt = 0.0;
			int j = 0;

			auto do_one = [&](int m)
			{
				if( m < 0 )
				{
					d_jg += l_data[m + l_data.size()] * lo_r_a[j];
					d_gt += h_data[m + h_data.size()] * hi_r_a[j];
				}
				else if( m > l_data.size() - 1 )
				{
					d_jg += l_data[m - l_data.size()] * lo_r_a[j];
					d_gt += h_data[m - h_data.size()] * hi_r_a[j];
				}
				else
				{
					d_jg += l_data[m] * lo_r_a[j];
					d_gt += h_data[m] * hi_r_a[j];
				}
			};
			for(j=0; j<lo_d_a.size(); j+=2)
				do_one(i + j / 2 - 2);
			m_data.rec.emplace_back(d_jg + d_gt);

			d_jg = d_gt = 0.0;
			for(j=1; j<lo_d_a.size(); j+=2)
				do_one(i + (j + 1) / 2 - 2);
			m_data.rec.emplace_back(d_jg + d_gt);
		}
	}

public:
	static void default_extend(vector_t &data) // Symmetric
	{
		auto src_front = data.front();
		auto src_back = data.back();
		auto src_size = data.size();

		std::vector xdata(src_size * 3 - 2, 0.0);
		for(int i=0; i<src_size; i++)
			xdata[src_size + i - 1] = data[i];

		for(int i=1; i<src_size; i++)
		{
			xdata[src_size - i] = 2 * src_front - data[i];
			xdata[src_size * 2 + i - 2] = 2 * src_back - data[src_size - 1 - i];
		}
		data = std::move(xdata);
	}

	static void default_unextend(vector_t &xdata) // Symmetric
	{
		auto tmp = std::move(xdata);
		auto offset = tmp.size() / 3;
		auto size = offset + static_cast<size_t>((tmp.size() - offset) / 2.0 + 0.5);
		while( offset < size )
			xdata.emplace_back(tmp[offset++]);
	}

public:
	filter_t m_filter;
	data_t m_data;
	ext_method_t m_ext_func = default_extend;
	uext_method_t m_uext_func = default_unextend;
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
	if( ext )
		m_impl->m_ext_func(m_impl->m_data.src);

	m_impl->dwt();
	if( ext )
	{
		m_impl->m_uext_func(m_impl->m_data.dec.high);
		m_impl->m_uext_func(m_impl->m_data.dec.low);
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
	if( ext )
	{
		m_impl->m_ext_func(m_impl->m_data.dec.high);
		m_impl->m_ext_func(m_impl->m_data.dec.low);
	}
	m_impl->idwt();

	if( ext )
		m_impl->m_uext_func(m_impl->m_data.rec);
	return m_impl->m_data.rec;
}

template <typename T>
typename basic_transform<T>::vector_t &basic_transform<T>::lpf(T threshold, level_t level)
{
	if( m_impl->m_data.src.size() < 3 && level == 0 )
		return m_impl->m_data.src;

	m_impl->m_ext_func(m_impl->m_data.src);
	if( level > 8 )
		level = 8;

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

	m_impl->m_uext_func(m_impl->m_data.rec);
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
basic_transform<T> &basic_transform<T>::on_extend(ext_method_t ext, uext_method_t uext)
{
	m_impl->m_ext_func = std::move(ext);
	m_impl->m_uext_func = std::move(uext);
	return *this;
}

template <typename T>
basic_transform<T> &basic_transform<T>::def_extend()
{
	m_impl->m_ext_func = impl::default_extend;
	m_impl->m_uext_func = impl::default_unextend;
	return *this;
}

} //namespace waveletpp


#endif //WAVELETPP_DETAIL_TRANSFORM_H