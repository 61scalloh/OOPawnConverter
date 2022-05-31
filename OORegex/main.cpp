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

            if (count < 0)
            {
                return false;
            }

            if (count == 0)
            {
                *length = i - *start_pos + 1;
                return true;
            }
        }
        else if (count == 0 && str[i] != ' ' && str[i] != '\n' && str[i] != '\t')
        {
            return false;
        }
    }

    return false;
}

int main(int argc, char* argv[])
{
    std::string source = argv[1];

    std::ifstream file(source, std::ios::binary | std::ios::ate);
    if (!file)
    {
        std::cout << "Error opening file '" << source << "'" << std::endl;
        return EXIT_FAILURE;
    }

    // read the entire file content into string 
    auto file_size = file.tellg();
    file.seekg(std::ios::beg);
    std::string str(file_size, 0);
    file.read(&str[0], file_size);

    std::regex blk_comment_r("\\/\\*(\\*(?!\\/)|[^*])*\\*\\/");
    std::regex line_comment_r("\\/\\/[^\\n\\r]+?(?:\\*\\)|[\\n\\r])");
    str = std::regex_replace(str, blk_comment_r, ""); // remove all block comments
    str = std::regex_replace(str, line_comment_r, ""); // remove all line comments

    std::regex class_r("^(\\s*)class\\s+([a-zA-Z_][a-zA-Z0-9_]*)(?:\\s+extends\\s+([a-zA-Z_][a-zA-Z0-9_]*))?");
    std::regex class_var_r("^(\\s*)var\\s+(?:[a-zA-Z_][a-zA-Z0-9_]*\\s*:\\s*)?([a-zA-Z_][a-zA-Z0-9_]*)\\s*(?:\\[([a-zA-Z0-9@_]+)\\])?");
    std::regex class_method_r("^(\\s*)method\\s+(?:(\\+|~)\\s*|(?:([a-zA-Z_][a-zA-Z0-9_]*)\\s*:\\s*))?([a-zA-Z_][a-zA-Z0-9_]*)\\s*(\\()");
    std::regex method_param_r("(?:(const)\\s+|(&)\\s*)?(?:(char)\\s+|([a-zA-Z_@][a-zA-Z0-9_@]*)\\s*:)?([a-zA-Z_@][a-zA-Z0-9_@]*)\\s*(?:\\[([a-zA-z0-9_@]*)\\])?\\s*(?:,|\\))");
    std::regex char_r("(const\\s+)?(char\\s+)([a-zA-Z@_][a-zA-Z0-9@_]*\\s*\\[)");
    std::regex open_r("\\s*\\{$");
    std::string bstr; // build string
    std::vector<std::string> method_list;
    size_t last_pos = 0;

    // search for class declaration
    for (auto it = std::sregex_iterator(str.begin(), str.end(), class_r); it != std::sregex_iterator(); ++it)
    {
        auto m = *it;
        std::string class_name = m[2].str();
        std::string var_class = "_c_" + class_name;

        std::string new_str;
        if (m[3].matched) // is inherited
            new_str = std::format("{}new Class:{} = oo_class(\"{}\", \"{}\")", m[1].str(), var_class, class_name, m[3].str());
        else
            new_str = std::format("{}new Class:{} = oo_class(\"{}\")", m[1].str(), var_class, class_name);

        // replace with new string
        bstr += str.substr(last_pos, m.position()-last_pos) + new_str;
        last_pos = m.position() + m.length();

        // get {} position and length
        size_t brace_start, brace_length;
        if (GetNestedString(str.c_str(), '{', '}', m.position() + m.length() + 1, &brace_start, &brace_length))
        {

            std::cout << "haha" << std::endl;
            bstr += str.substr(last_pos, brace_start - last_pos);

            size_t last_pos2 = 0;
            std::string class_str = str.substr(brace_start, brace_length);
            std::string class_bstr;

            // search for variables
            for (auto itv = std::sregex_iterator(class_str.begin(), class_str.end(), class_var_r); itv != std::sregex_iterator(); ++itv)
            {
                auto mv = *itv;
                std::string var_name = mv[2].str();
                if (mv[3].matched) // array
                    new_str = std::format("{}oo_var({}, DT_ARRAY[{}], \"{}\")", mv[1].str(), var_class, mv[3].str(), var_name);
                else
                    new_str = std::format("{}oo_var({}, DT_CELL, \"{}\")", mv[1].str(), var_class, var_name);

                class_bstr += class_str.substr(last_pos2, mv.position() - last_pos2) + new_str; // build up the string
                last_pos2 = mv.position() + mv.length();
            }

            class_str = class_bstr + class_str.substr(last_pos2);
            class_bstr.clear();
            last_pos2 = 0;

            // search for methods
            for (auto itm = std::sregex_iterator(class_str.begin(), class_str.end(), class_method_r); 
                itm != std::sregex_iterator(); ++itm)
            {
                auto mm = *itm;
                std::string method_type = !mm[2].matched ? "MT_METHOD" : mm[2].str() == "+" ? "MT_CTOR" : "MT_DTOR";
                //std::string tag_name = mm[3].str();
                std::string method_name = mm[4].str();
                size_t param_start, param_length;

                // get () position and length
                if (GetNestedString(class_str.c_str(), '(', ')', mm.position() + mm.length() - 1, &param_start, &param_length))
                {
                    std::string param_str = class_str.substr(param_start, param_length);
                    new_str = std::format("{}oo_method({}, {}, \"{}\"", mm[1].str(), var_class, method_type, method_name);

                    // search for method parameters
                    for (auto itp = std::sregex_iterator(param_str.begin(), param_str.end(), method_param_r); 
                        itp != std::sregex_iterator(); ++itp)
                    {
                        auto mp = *itp;
                        if (mp[6].matched) // is array or string
                        {
                            if (mp[3].str() == "char")
                                new_str += ", FP_STRING";
                            else
                                new_str += ", FP_ARRAY";
                        }
                        else
                        {
                            if (!mp[2].matched) // by value
                            {
                                if (mp[4].str() == "Float")
                                    new_str += ", FP_FLOAT";
                                else
                                    new_str += ", FP_CELL";
                            }
                            else // by ref
                            {
                                new_str += ", FP_VAL_BYREF";
                            }
                        }
                    }

                    new_str += ")";
                    size_t mstart, mlen;

                    // check if this method has declaration
                    if (GetNestedString(class_str.c_str(), '{', '}', param_start + param_length + 1, &mstart, &mlen))
                    {
                        std::string method_str = std::format("\n{}public {}{}@{}", mm[1].str(), mm[3].matched ? mm[3].str() + ":" : "", class_name, method_name);
                        param_str = std::regex_replace(param_str, char_r, "$1$3");
                        method_str += param_str + mm[1].str();
                        method_str += class_str.substr(mstart, mlen);

                        std::string s = mm[1].str().substr(2);
                        method_str = std::regex_replace(method_str, std::regex("^" + s), "");
                        std::cout << "[" << s << "]" << std::endl;
                        method_list.push_back(method_str); // add to the method list for later use
                    }
                    else
                    {
                        mstart = param_start;
                        mlen = param_length;
                    }

                    class_bstr += class_str.substr(last_pos2, mm.position() - last_pos2) + new_str; // build up the string
                    last_pos2 = mstart + mlen;
                }
            }

            class_bstr += class_str.substr(last_pos2);
            bstr += class_bstr;
            last_pos = brace_start + brace_length;
        }
    }
    bstr += str.substr(last_pos);

    // append extra code from the method list
    for (const auto& s : method_list)
    {
        bstr += s;
    }

    std::ofstream ofile(source + ".sma", std::ios::binary | std::ios::trunc);
    ofile << bstr;
}