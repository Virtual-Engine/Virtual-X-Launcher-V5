#pragma once

#include "Logger/Logger.hpp"

#include <array>
#include <filesystem>
#include <fstream>
#include <windows.h>

using namespace al;

#define ADD_COLOR_TO_STREAM(color) "\x1b[" << int(color) << "m"
#define RESET_STREAM_COLOR "\x1b[" << int(LogColor::RESET) << "m"
#define HEX_TO_UPPER(value) \
	"0x" << std::hex << std::uppercase << (DWORD64)value << std::dec << std::nouppercase

template<typename TP>
inline std::time_t to_time_t(TP tp) {
	using namespace std::chrono;
	auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now() + system_clock::now());
	return system_clock::to_time_t(sctp);
}

enum class LogColor {
	RESET,
	BLACK          = 30,
	RED            = 31,
	GREEN          = 32,
	YELLOW         = 33,
	BLUE           = 34,
	MAGENTA        = 35,
	CYAN           = 36,
	DARK_WHITE     = 37,
	BRIGHT_BLACK   = 90,
	BRIGHT_RED     = 91,
	BRIGHT_GREEN   = 92,
	BRIGHT_YELLOW  = 93,
	BRIGHT_BLUE    = 94,
	BRIGHT_MAGENTA = 95,
	BRIGHT_CYAN    = 96,
	WHITE          = 97,
};

class OwnLogger final {
public:
	OwnLogger(std::string consoleName, bool wine, bool inConsole = true, bool inFile = true, bool createBackup = true, std::string logFileName = "Log.txt");
	virtual ~OwnLogger();

	void init(std::filesystem::path folderPath, int alpha = 255, bool showDate = true, bool showFile = true);
	void destroy();

	void format_console_progress(std::string filled, std::string unfilled, int progress);

private:
	void create_backup(std::filesystem::path folderPath);

	void open_outstreams(std::filesystem::path folderPath);
	void close_outstreams();

	void format_console(const LogMessagePtr msg, bool showDate = true, bool showFile = true);
	void format_console_simple(const LogMessagePtr msg, bool showDate = true, bool showFile = true);
	void format_file(const LogMessagePtr msg, bool showDate = true, bool showFile = true);

public:
	bool m_attach_console;
	bool write_in_file;
	bool should_create_backup;
	bool m_did_console_exist;
	std::string log_file_name;

	void (OwnLogger::*m_console_logger)(const LogMessagePtr msg, bool showDate, bool showFile);

	std::string m_console_title;
	DWORD m_original_console_mode;
	HANDLE m_console_handle;

	std::ofstream m_console_out;
	std::ofstream m_file_out;
};