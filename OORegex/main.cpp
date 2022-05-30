#include <iostream>
#include <fstream>
#include <vector>
#include <regex>
#include <format>

bool GetNestedString(const char* str, const char open='{', const char close='}', const size_t offset=0, size_t* start_pos=nullptr, size_t* length=nullptr)
{
    int count = 0;
    size_t len = strlen(str);
    *start_pos = offset;
    *length = 0;

    for (size_t i = offset; i < len; i++)
    {
        if (str[i] == open)
        {
            if (count == 0)
                *start_pos = i;

            count++;
        }
        else if (str[i] == close)
        {
            count--;

            if (count == 0)
            {
                *length = i - *start_pos + 1;
                return true;
            }
        }
    }

    return false;
}

int main(int argc, char* argv[])
{
    std::vector<std::string> includes;
    std::string output;
    std::string source;

    for (int ndx{}; ndx != argc; ++ndx)
    {
        if (argv[ndx][0] == '-')
        {
            if (argv[ndx][1] == 'i')
                includes.push_back(argv[ndx] + 2); // push includes
            else if (argv[ndx][1] == 'o')
                output = argv[ndx] + 2; // set output
        }
        else
            source = argv[ndx]; // set source
    }

    std::ifstream file(source, std::ios::binary | std::ios::ate);
    if (!file)
    {
        std::cout << "Error opening file '" << source << "'" << std::endl;
        return EXIT_FAILURE;
    }

    // read file str into string 
    auto file_size = file.tellg();
    file.seekg(std::ios::beg);
    std::string str(file_size, 0);
    file.read(&str[0], file_size);

    std::regex blk_comment_r("\\/\\*(\\*(?!\\/)|[^*])*\\*\\/");
    std::regex line_comment_r("\\/\\/[^\\n\\r]+?(?:\\*\\)|[\\n\\r])");
    str = std::regex_replace(str, blk_comment_r, ""); // remove all block comments
    str = std::regex_replace(str, line_comment_r, ""); // remove all line comments

    std::regex class_r("^(\\s*)class\\s+([a-zA-Z_][a-zA-Z0-9_]*)(?:\\s+extends\\s+([a-zA-Z_][a-zA-Z0-9_]*))?");
    std::regex class_var_r("^(\\s*)var\\s+([a-zA-Z_][a-zA-Z0-9_]*\\s*:\\s*)?([a-zA-Z_][a-zA-Z0-9_]*)\\s*(\\[\\d+\\])?");
    std::regex class_method_r("^(\\s*)method\\s+((\\+|~)|([a-zA-Z_][a-zA-Z0-9_]*\\s*:\\s*))?([a-zA-Z_][a-zA-Z0-9_]*)");
    std::smatch match;

    // search for class declaration
    while (std::regex_search(str.cbegin(), str.cend(), match, class_r))
    {
        std::string class_name = match[2].str();
        std::string var_class = "_c_" + class_name;

        std::string replace;
        if (match[3].matched) // is inherited
            replace = std::format("{}new Class:{} = oo_class(\"{}\", \"{}\")", match[1].str(), var_class, class_name, match[3].str());
        else
            replace = std::format("{}new Class:{} = oo_class(\"{}\")", match[1].str(), var_class, class_name);

        // replace with new string
        str.erase(match.position(), match.length());
        str.insert(match.position(), replace);
        
        // get {} position and length
        size_t brace_start, brace_length;
        if (GetNestedString(str.c_str(), '{', '}', match.position() + replace.length(), &brace_start, &brace_length))
        {
            std::string class_str = str.substr(brace_start, brace_length);

            // search for variables
            while (std::regex_search(class_str.cbegin(), class_str.cend(), match, class_var_r))
            {
                std::string var_name = match[3].str();
                int var_size = match[4].matched ? std::stoi(match[4].str().substr(1, match[4].str().size() - 2)) : 1;
                if (var_size > 1) // array
                    replace = std::format("{}oo_var({}, DT_ARRAY[{}], \"{}\")", match[1].str(), var_class, var_size, var_name);
                else
                    replace = std::format("{}oo_var({}, DT_CELL, \"{}\")", match[1].str(), var_class, var_name);

                class_str.erase(match.position(), match.length());
                class_str.insert(match.position(), replace);
            }

            // search for methods
            while (std::regex_search(class_str.cbegin(), class_str.cend(), match, class_method_r))
            {
                int method_type = match[3].matched ? (match[3].str() == "+" ? 1 : match[3].str() == "-" ? 2 : 0) : 0;
                std::string tag_name = match[4].str();
                std::string method_name = match[5].str();

                size_t param_start, param_length;
                // get () position and length
                if (GetNestedString(class_str.c_str(), '(', ')', match.position() + match.length(), &param_start, &param_length))
                {

                }

                class_str.erase(match.position(), match.length());
            }

            str.erase(brace_start, brace_length);
            str.insert(brace_start, class_str);
        }
    }

    std::ofstream ofile(source + ".oo", std::ios::binary | std::ios::trunc);
    ofile << str;
}