
/************************************************************************************
*                                                                                   *
*   Copyright (c) 2024-2025 Xiaoqiang <username_nullptr@163.com>                    *
*                                                                                   *
*   This file is part of LIBGS                                                      *
*   License: MIT License                                                            *
*                                                                                   *
*   Permission is hereby granted, free of charge, to any person obtaining a copy    *
*   of this software and associated documentation files (the "Software"), to deal   *
*   in the Software without restriction, including without limitation the rights    *
*   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell       *
*   copies of the Software, and to permit persons to whom the Software is           *
*   furnished to do so, subject to the following conditions:                        *
*                                                                                   *
*   The above copyright notice and this permission notice shall be included in      *
*   all copies or substantial portions of the Software.                             *
*                                                                                   *
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR      *
*   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,        *
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE     *
*   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER          *
*   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,   *
*   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE   *
*   SOFTWARE.                                                                       *
*                                                                                   *
*************************************************************************************/

#ifndef WAVELETPP_DETAIL_TRANSFORM_H
#define WAVELETPP_DETAIL_TRANSFORM_H

namespace waveletpp
{

template <typename T>
class WAVELETPP_TAPI basic_transform<T>::impl
{
public:
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
		m_lo_d = lo_d(m_filter);
		m_hi_d = hi_d(m_filter);
		m_lo_r = lo_r(m_filter);
		m_hi_r = hi_r(m_filter);
	}

public:
	void dwt()
	{
		m_data.dec.high.clear();
		m_data.dec.low.clear();

		if( m_data.src.size() % 2 )
			m_data.src.emplace_back(m_data.src.back());

		for(int i=0; i<static_cast<int>(m_data.src.size()); i+=2)
		{
			double d_low = 0;
			double d_high = 0;
			for(int j=0; j<static_cast<int>(m_lo_d.size()); j++)
			{
				int idx = (i + j + static_cast<int>(m_data.src.size())) % m_data.src.size();
				d_low += m_data.src[idx] * m_lo_d[j];
				d_high += m_data.src[idx] * m_hi_d[j];
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

		for(int i=0; i<static_cast<int>(l_data.size()); i++)
		{
			double d_high = 0.0;
			double d_low = 0.0;
			int j = 0;

			auto do_one = [&](int m) mutable
			{
				int idx = (m + static_cast<int>(l_data.size())) % l_data.size();
				d_high += h_data[idx] * m_hi_r[j];
				d_low += l_data[idx] * m_lo_r[j];
			};
			int offset = static_cast<int>(m_lo_d.size()) / 2;
			for(j=0; j<static_cast<int>(m_lo_d.size()); j+=2)
				do_one(i + j / 2 - offset);
			m_data.rec.emplace_back(d_low + d_high);

			d_high = d_low = 0.0;
			for(j=1; j<static_cast<int>(m_lo_d.size()); j+=2)
				do_one(i + (j + 1) / 2 - offset);
			m_data.rec.emplace_back(d_low + d_high);
		}
	}

public:
	[[nodiscard]] static size_t default_extend(vector_t &data, size_t filter_size) // Symmetric
	{
		auto src_size = data.size();
		std::vector xdata(src_size + filter_size * 2, type_tool<value_t>::zero);

		for(int i=0; i<static_cast<int>(src_size); i++)
			xdata[filter_size + i] = data[i];

		for(int i=0; i<static_cast<int>(filter_size); i++)
			xdata[i] = data[filter_size - 1 - i];

		for(int i=0; i<static_cast<int>(filter_size); i++)
			xdata[ + src_size + i] = data[src_size - 1 - i];

		data = std::move(xdata);
		return filter_size;
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
	filter_t m_filter {};
	std::vector<double> m_lo_d {};
	std::vector<double> m_hi_d {};
	std::vector<double> m_lo_r {};
	std::vector<double> m_hi_r {};

	data_t m_data {};
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
	m_impl->set_filter(filter);
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
	if( m_impl->m_data.src.size() < m_impl->m_lo_d.size() * 2 )
	{
		m_impl->m_data.dec.low = m_impl->m_data.src;
		m_impl->m_data.dec.high.clear();
		return m_impl->m_data.dec;
	}
	size_t ext_size = 0;
	if( ext )
		ext_size = m_impl->m_ext_func(m_impl->m_data.src, filter_size());

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
typename basic_transform<T>::vector_t &basic_transform<T>::lpf(param_t param)
{
	if( m_impl->m_data.src.size() < m_impl->m_lo_d.size() * 2 or param.level == 0 )
	{
		m_impl->m_data.dec.high.clear();
		m_impl->m_data.dec.low.clear();
		return m_impl->m_data.src;
	}
	if( param.level > 8 )
		param.level = 8;

	size_t ext_size = 0;
	for(level_t i=0; i<param.level; i++)
		ext_size += m_impl->m_ext_func(m_impl->m_data.src, filter_size());

	auto tmp = m_impl->m_data.src;
	std::vector<vector_t> h_datas;

	for(level_t i=0; i<param.level; i++)
	{
		m_impl->dwt();
		for(auto &data : m_impl->m_data.dec.high)
		{
			if( type_tool<value_t>::abs(data) < param.threshold )
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