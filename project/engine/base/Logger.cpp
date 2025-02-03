#include "Logger.h"
#include "Windows.h"

namespace Logger 
{
	void Logger::Log(const std::string& message){
		OutputDebugStringA("文字列リテラルを出力するよ\n");

		std::string a("stringに埋め込んだ文字列を出力するよ\n");
		OutputDebugStringA(message.c_str());
	}
}