#ifndef PARSER_HPP
# define PARSER_HPP

std::vector<std::string>	splitStringByEND(const std::string&);

int							commandToMacro(const std::string&);
std::string					macroToCommand(int command);
std::string					removeEOFCharacters(const std::string& str);
#endif
