#include "OwnLogger.hpp"

OwnLogger::OwnLogger(std::string consoleName, bool wine, bool inConsole, bool inFile, bool createBackup, std::string logFileName) :
    m_attach_console(inConsole), m_did_console_exist(false), write_in_file(inFile), should_create_backup(createBackup), m_console_logger(&OwnLogger::format_console), m_console_title(consoleName), m_original_console_mode(0), m_console_handle(nullptr), log_file_name(logFileName) {
	if (const auto env_no_color = std::getenv("NO_COLOR"); wine || (env_no_color && strlen(env_no_color))) {
		LOG(VERBOSE) << "Using simple logger.";
		m_console_logger = &OwnLogger::format_console_simple;
	}

	// g_logger = this;
}

OwnLogger::~OwnLogger() {
	// g_logger = nullptr;
}

void OwnLogger::init(std::filesystem::path folderPath, int alpha, bool showDate, bool showFile) {
	if (m_attach_console) {
		if (m_did_console_exist = ::AttachConsole(GetCurrentProcessId()); !m_did_console_exist)
			AllocConsole();

		if (m_console_handle = GetStdHandle(STD_OUTPUT_HANDLE); m_console_handle != nullptr) {
			SetLayeredWindowAttributes(GetConsoleWindow(), NULL, alpha, LWA_ALPHA);
			SetConsoleTitleA(m_console_title.data());
			SetConsoleOutputCP(CP_UTF8);

			DWORD console_mode;
			GetConsoleMode(m_console_handle, &console_mode);
			m_original_console_mode = console_mode;

			// terminal like behaviour enable full color support
			console_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
			// prevent clicking in terminal from suspending our main thread
			console_mode &= ~(ENABLE_QUICK_EDIT_MODE);

			SetConsoleMode(m_console_handle, console_mode);
		}
	}
	if (this->should_create_backup)
		create_backup(folderPath);
	if (this->write_in_file)
		open_outstreams(folderPath);

	Logger::Init();
	Logger::AddSink([ this, showDate, showFile ](LogMessagePtr msg) {
		(this->*m_console_logger)(std::move(msg), showDate, showFile);
	});
	Logger::AddSink([ this, showDate, showFile ](LogMessagePtr msg) {
		format_file(std::move(msg), showDate, showFile);
	});
}

void OwnLogger::destroy() {
	Logger::Destroy();
	close_outstreams();

	if (m_did_console_exist)
		SetConsoleMode(m_console_handle, m_original_console_mode);

	if (!m_did_console_exist && m_attach_console)
		FreeConsole();
}

void OwnLogger::create_backup(std::filesystem::path folderPath) {
	std::filesystem::path m_file_path(folderPath);
	std::filesystem::path m_backup_path = m_file_path;
	m_backup_path /= "Backup";
	m_file_path /= this->log_file_name;

	if (std::filesystem::exists(m_file_path)) {
		auto file_time  = std::filesystem::last_write_time(m_file_path);
		auto time_t     = to_time_t(file_time);
		auto local_time = std::localtime(&time_t);
		auto timestamp = std::format("{:0>2}-{:0>2}-{}-{:0>2}-{:0>2}-{:0>2} {}",
		                             local_time->tm_mon + 1,
		                             local_time->tm_mday,
		                             local_time->tm_year + 1900,
		                             local_time->tm_hour,
		                             local_time->tm_min,
		                             local_time->tm_sec,
		                             this->log_file_name);

		if (!std::filesystem::exists(m_backup_path))
			std::filesystem::create_directory(m_backup_path);
		std::filesystem::copy_file(m_file_path, m_backup_path / timestamp);
	}
}

void OwnLogger::open_outstreams(std::filesystem::path folderPath) {
	std::filesystem::path m_file_path(folderPath);
	m_file_path /= this->log_file_name;
	if (m_attach_console)
		m_console_out.open("CONOUT$", std::ios_base::out | std::ios_base::app);

	m_file_out.open(m_file_path.c_str(), std::ios_base::out | std::ios_base::trunc);
}

void OwnLogger::close_outstreams() {
	if (m_attach_console)
		m_console_out.close();

	m_file_out.close();
}

const LogColor get_color(const eLogLevel level) {
	switch (level) {
	case PROGRESS: return LogColor::BRIGHT_GREEN;
	case AUTH: return LogColor::BRIGHT_BLUE;
	case INFO: return LogColor::CYAN;
	case WARNING: return LogColor::BRIGHT_YELLOW;
	case INFO_DL: return LogColor::CYAN;
	case WARNING_DL: return LogColor::BRIGHT_YELLOW;
	case FATAL: return LogColor::BRIGHT_RED;
	case DEBUG: return LogColor::BLUE;
	case VERBOSE: return LogColor::YELLOW;
	}
	return LogColor::WHITE;
}

const std::string get_level_string(const eLogLevel level) {
	std::vector<std::string> levelStrings = {
	    {""},
	    {"LOADING"},
	    {"AUTH"},
	    {"INFO"},
	    {"WARN"},
	    {"FATAL"},
	    {"DEBUG"},
	    {"VERBOSE"},
	    {"INFO"},
	    {"WARN"},
	};

	return levelStrings [ level ];
}

void OwnLogger::format_console(const LogMessagePtr msg, bool showDate, bool showFile) {
	static std::ostringstream buffer;
	const auto color = get_color(msg->Level());

	const auto timestamp = showDate ? std::format("{0:%H:%M:%S}", std::chrono::time_point_cast<std::chrono::seconds>(msg->Timestamp())) : "";
	const auto& location = msg->Location();
	const auto level     = msg->Level();

	const auto file =
	    showFile ? std::filesystem::path(location.file_name()).filename().string() : "";

	const auto timestamp_stream = (showDate ? timestamp : "");
	const auto level_stream     = get_level_string(level);
	const auto file_stream = (showFile || level == eLogLevel::FATAL || level == eLogLevel::WARNING || level == eLogLevel::VERBOSE ?
	                              file + ":" + std::to_string(location.line()) :
	                              "");

	if (msg->Level() == eLogLevel::WARNING_DL || msg->Level() == eLogLevel::INFO_DL) {
		buffer << timestamp_stream << (timestamp_stream.length() > 0 ? " | " : "") << ADD_COLOR_TO_STREAM(color) << level_stream << RESET_STREAM_COLOR << " | " << file_stream << (file_stream.length() > 0 ? " | " : "") << msg->Message() << std::flush;
		return;
	}

	if (msg->Level() == eLogLevel::PROGRESS) {
		m_console_out << "\033[s" << timestamp_stream << (timestamp_stream.length() > 0 ? " | " : "") << ADD_COLOR_TO_STREAM(color) << level_stream << RESET_STREAM_COLOR << " | " << file_stream << (file_stream.length() > 0 ? " | " : "") << msg->Message() << std::flush;
		return;
	}
	if (buffer.str().length() != 0) {
		m_console_out << buffer.str() << std::flush;
		buffer.clear();
	}
	m_console_out << timestamp_stream << (timestamp_stream.length() > 0 ? " | " : "") << ADD_COLOR_TO_STREAM(color) << level_stream << RESET_STREAM_COLOR << " | " << file_stream << (file_stream.length() > 0 ? " | " : "") << msg->Message() << std::flush;
}

void OwnLogger::format_console_simple(const LogMessagePtr msg, bool showDate, bool showFile) {
	static std::string buffer = "";
	const auto color          = get_color(msg->Level());

	const auto timestamp = showDate ? std::format("{0:%H:%M:%S}", std::chrono::time_point_cast<std::chrono::seconds>(msg->Timestamp())) : "";
	const auto& location = msg->Location();
	const auto level     = msg->Level();

	const auto file =
	    showFile ? std::filesystem::path(location.file_name()).filename().string() : "";

	const auto timestamp_stream = (showDate ? timestamp : "");
	const auto level_stream     = get_level_string(level);
	const auto file_stream = (showFile || level == eLogLevel::FATAL || level == eLogLevel::WARNING || level == eLogLevel::VERBOSE ?
	                              file + ":" + std::to_string(location.line()) :
	                              "");

	if (msg->Level() == eLogLevel::WARNING_DL || msg->Level() == eLogLevel::INFO_DL) {
		buffer += timestamp_stream + (timestamp_stream.length() > 0 ? " | " : "") + level_stream + " | " + file_stream + (file_stream.length() > 0 ? " | " : "") + msg->Message() + "\n";
		return;
	}

	if (msg->Level() == eLogLevel::PROGRESS) {
		m_console_out << "\033[s" << level_stream << " | " << msg->Message() << std::flush;
		return;
	}

	if (!buffer.empty()) {
		m_console_out << buffer;
		buffer.clear();
		buffer = "";
	}
	m_console_out << timestamp_stream << (timestamp_stream.length() > 0 ? " | " : "") << level_stream << " | " << file_stream << (file_stream.length() > 0 ? " | " : "") << msg->Message() << std::flush;
}

void OwnLogger::format_console_progress(std::string filled, std::string unfilled, int progress) {
	if (progress == 100)
		m_console_out << "\033[K" << ADD_COLOR_TO_STREAM(LogColor::BRIGHT_GREEN) << filled << RESET_STREAM_COLOR << ADD_COLOR_TO_STREAM(LogColor::BRIGHT_BLACK) << unfilled << RESET_STREAM_COLOR << " " << progress << "%\r" << std::endl
		              << std::flush;
	else
		m_console_out << "\033[K" << ADD_COLOR_TO_STREAM(LogColor::BRIGHT_GREEN) << filled << RESET_STREAM_COLOR << ADD_COLOR_TO_STREAM(LogColor::BRIGHT_BLACK) << unfilled << RESET_STREAM_COLOR << " " << progress << "%\r" << std::flush;
}

void OwnLogger::format_file(const LogMessagePtr msg, bool showDate, bool showFile) {
	if (!m_file_out.is_open())
		return;

	const auto timestamp = showDate ? std::format("{0:%H:%M:%S}", std::chrono::time_point_cast<std::chrono::seconds>(msg->Timestamp())) : "";
	const auto& location = msg->Location();
	const auto level     = msg->Level();

	const auto file = std::filesystem::path(location.file_name()).filename().string();

	const auto timestamp_stream = (showDate ? timestamp : "");
	const auto level_and_file_stream = (showFile || level == eLogLevel::FATAL || level == eLogLevel::WARNING || level == eLogLevel::VERBOSE ? get_level_string(level) + " " + file + ":"
	                                            + std::to_string(location.line()) :
	                                                                                                                                          "" + get_level_string(level));

	m_file_out << timestamp_stream << (timestamp_stream.length() > 0 ? " | " : "") << level_and_file_stream
	           << (level_and_file_stream.length() > 0 ? " | " : "") << msg->Message() << std::flush;
}