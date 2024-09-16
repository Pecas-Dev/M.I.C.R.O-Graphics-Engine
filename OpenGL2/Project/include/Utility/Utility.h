#pragma once

#include <string>
#include <vector>
#include <deque>


class Utility
{
public:
	static void ParseString(std::string& string, std::vector<std::string>& subStrings, char token);

	static void AddMessage(const std::string& message);
	static std::string ReadMessage();

private:
	static std::deque<std::string> m_messages;
};
