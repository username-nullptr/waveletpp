#include <waveletpp.hpp>
#include <fstream>
#include <string>

int main(int argc, char *argv[])
{
	constexpr auto file_name = "C:/Users/Administrator/Desktop/test.wavelet.data/r.csv.200/tttt_0.csv";
	std::ifstream ifile(file_name);

	waveletpp::transform wavelet(waveletpp::filter::sym5);
	std::string buf;

	while( std::getline(ifile,buf) )
		wavelet.data().src.emplace_back(std::stod(buf));
	ifile.close();

	auto &wdata = wavelet.lpf(1.0);
	std::ofstream ofile("C:/Users/Administrator/Desktop/test.wavelet.data/w.csv.200/tttt_0.csv");

	for(auto &data : wdata)
		ofile << data << "\n";
	ofile.close();

	// auto &ddata = wavelet.dwt();

	// std::ofstream ofile_l("C:/Users/Administrator/Desktop/test.wavelet.data/w.csv.200/tttt_0_l.csv");
	// for(auto data : ddata.low)
	// 	ofile_l << data << "\n";
	// ofile_l.close();
	//
	// std::ofstream ofile_h("C:/Users/Administrator/Desktop/test.wavelet.data/w.csv.200/tttt_0_h.csv");
	// for(auto data : ddata.high)
	// 	ofile_h << data << "\n";
	// ofile_h.close();

	return 0;
}