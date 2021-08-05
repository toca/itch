#pragma once
#include <string>
#include <stdexcept>

void printerr(const wchar_t* fmt, ...);

//void on_error(const wchar_t* pos, const wchar_t* message);

class parse_error : public std::exception
{
public:
	/***/
	parse_error(const std::wstring& filename, const std::wstring& source, const wchar_t* pos, const std::wstring& message);
	/***/
	parse_error(const std::wstring& filename, const std::wstring& source, const wchar_t* pos, const wchar_t* message);
	/***/
	parse_error(const wchar_t* file, const wchar_t* source, const wchar_t* pos, const wchar_t* message);
	/***/
	parse_error(const wchar_t* file, const wchar_t* source, const wchar_t* pos, const std::wstring& message);

	const char* what() const override;
private:
	void set_message(const wchar_t* file, const wchar_t* source, const wchar_t* pos, const wchar_t* message);
	std::wstring message;
	std::string narrow_message;
};

class tokenize_error : public std::exception
{
public:
	tokenize_error(const wchar_t* file, const wchar_t* source, const wchar_t* pos, const wchar_t* message);
	tokenize_error(const wchar_t* file, const wchar_t* source, const wchar_t* pos, const std::wstring& message);

	const char* what() const override;
private:
	std::wstring message;
	std::string narrow_message;
};
