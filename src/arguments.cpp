#include "arguments.h"
#include <iostream>

compact_list_c::compact_list_c(){}

compact_list_c::compact_list_c(int dummy_value, ...) {
    va_list vl;
    char *value;
    va_start(vl, dummy_value);
    do {
        value = va_arg(vl, char*);
        if (value) {
            items.push_back(value);
        }
    } while (value);
}
size_t compact_list_c::size() const {
    return items.size();
}
std::vector<std::string> compact_list_c::vector() const {
    return items;
}
std::string compact_list_c::operator[] (size_t index) {
    return items[index];
}
compact_list_c::operator std::vector<std::string>() const {
    return vector();
}


void abstract_parser_c::add_parameter(const std::vector<std::string> &params, std::function<bool(const std::string)> callback) {
    for (auto i = params.begin(); i != params.end(); i++) {
        parameters[*i] = callback;
    }
}


bool settings_parser_c::is_program() {
    return (position < arg_c && arg_v[position][0] != '-');
}

void settings_parser_c::parse_program() {
    program = get_next_argument();
    while (position < arg_c && arg_v[position] != separator) {
        program_arguments.push_back(get_next_argument());
    }
}

void settings_parser_c::add_parser(abstract_parser_c *parser) {
    parsers.push_back(parser);
}

void settings_parser_c::set_dividers(const std::vector<std::string> &d) {
    dividers.clear();
    dividers.insert(dividers.begin(), d.begin(), d.end());
}

settings_parser_c::settings_parser_c() : position(0), stopped(0) {
}
int settings_parser_c::current_position() {
    return position;
}
void settings_parser_c::fetch_current_position() {
    fetched_position = position;
}
void settings_parser_c::restore_position() {
    position = fetched_position;
}
size_t settings_parser_c::saved_count() {
    return saved_positions.size();
}
void settings_parser_c::stop() {
    stopped = true;
}
char *settings_parser_c::get_next_argument() {
    if (position >= arg_c) {
        return NULL;
    }
    return arg_v[position++];
}
void settings_parser_c::save_current_position(abstract_parser_c *associated_parser) {
    saved_positions.push_back(std::pair<int, abstract_parser_c*>(current_position(), associated_parser));
}
abstract_parser_c *settings_parser_c::pop_saved_parser() {
    auto value = saved_positions.front();//std::pair<int, void*>
    position = value.first;
    saved_positions.erase(saved_positions.begin(), saved_positions.begin()+1);
    return value.second;
}
void settings_parser_c::clear_parsers() {
    for (auto i = parsers.begin(); i != parsers.end(); i++) {
        delete (*i);
    }
    parsers.clear();
}

settings_parser_c::~settings_parser_c() {
    clear_parsers();
}

std::string settings_parser_c::get_program() {
    return program;
}
std::vector<std::string> settings_parser_c::get_program_arguments() {
    return program_arguments;
}
void settings_parser_c::set_separator(const std::string &s) {
    separator = long_arg(s);
}
void settings_parser_c::reset_program() {
    program = "";
    program_arguments.clear();
}
void settings_parser_c::parse(int argc, char *argv[]) {
    arg_c = argc;
    arg_v = argv;
    position = 1;

    while (current_position() < argc && !stopped) {
        for (auto parser = parsers.begin(); parser != parsers.end(); parser++) {
            fetch_current_position();
            if ((*parser)->parse(*this)) {
                save_current_position((*parser));
            }
            restore_position();
        }
        if (saved_count() == 1) {
            if (!pop_saved_parser()->invoke(*this)) {
                std::cerr << "some error " << arg_v[position - 1] << std::endl;
                throw "";
            }
        } else if (saved_count() > 1) {
            //ambiguous arguments
            //throw
            std::cerr << "ambiguous arguments" << std::endl;
        } else {
            if (is_program()) {
                parse_program();
                //parse_program//until separator detected
            } else {
                //unknown_argument
                std::cerr << "unknown argument " << arg_v[position] << std::endl;
                throw "";
            }
        }
    }
}
size_t settings_parser_c::parsers_count() {
    return parsers.size();
}

void settings_parser_c::pop_back() {
    if (!parsers_count()) {
        return;
    }
    parsers.pop_back();
}






console_argument_parser_c::console_argument_parser_c() {
}
bool console_argument_parser_c::parse(abstract_settings_parser_c &parser_object) {
    console_argument_parser_c::parser_object = &parser_object;
    last_state = argument_error_state;
    if (process_argument(parser_object.get_next_argument()) == argument_error_state) {
        return false;
    }
    while (last_state == argument_started_state) {
        process_value(parser_object.get_next_argument());
    }
    return last_state == argument_ok_state;
}
void console_argument_parser_c::set_flag(const std::vector<std::string> &v) {
    for (auto i = v.begin(); i != v.end(); i++) {
        is_flag[*i] = true;
    }
}
console_argument_parser_c::parsing_state_e console_argument_parser_c::process_argument(const char *argument) {
    //check if in the dictionary
    if (!argument) {
        return last_state = argument_error_state;
    }
    std::string s = argument;
    std::string left, right;
    if (is_flag.find(std::string(argument)) != is_flag.end()) {
        value = "1";
        callback = parameters[argument];
        last_state = argument_ok_state;
        return last_state;
    }
    size_t length = s.length();
    size_t divider_length = 0;
    for (auto i = parser_object->dividers.begin(); i != parser_object->dividers.end(); i++) {
        size_t pos = s.find(*i);
        length = min_def(length, pos);
        if (length == pos) {
            divider_length = max_def(divider_length, (*i).length());
        }
    }
    left = s.substr(0, length);
    //check left
    if (parameters.find(left) == parameters.end()) {
        return argument_error_state;
    } else if (is_flag.find(left) != is_flag.end()) {
        return argument_error_state;
    }
    callback = parameters[left];
    last_state = argument_started_state;
    if (length < s.length()) {
        right = s.substr(length + divider_length, s.length() - length - divider_length);
        return process_value(right.c_str());
    }
    return last_state;
}
console_argument_parser_c::parsing_state_e console_argument_parser_c::process_value(const char *argument) {
    if (last_state == argument_ok_state) {
        return last_state;
    }
    if (!argument) {
        return last_state = argument_error_state;
    }
    value = argument;
    last_state = argument_ok_state;
    return last_state;
}
bool console_argument_parser_c::invoke(abstract_settings_parser_c &parser_object) {
    if (!callback(value)) {
        return false;//not good
    }
    return true;
    //((settings_parser_c&)parser_object).set_value(argument_name, value);
}
std::string console_argument_parser_c::help() { return ""; }



environment_variable_parser_c::environment_variable_parser_c() {
}

bool environment_variable_parser_c::exists_environment_variable(const std::string &variable) {
    last_variable_name = variable;
    return GetEnvironmentVariable(variable.c_str(), buffer, sizeof(buffer));
}
std::string environment_variable_parser_c::get_environment_variable(const std::string &variable) {
    if (last_variable_name != variable) {
        GetEnvironmentVariable(variable.c_str(), buffer, sizeof(buffer));
    }
    return buffer;
}
bool environment_variable_parser_c::invoke_initialization(abstract_settings_parser_c &parser_object) {
    std::map<std::string, std::function<bool(const std::string&)> > m;
    for (auto i = parameters.begin(); i != parameters.end(); i++) {
        if (exists_environment_variable(i->first)) {
            m[i->first] = i->second;
        }
    }
    for (auto i = m.begin(); i != m.end(); i++) {
        if (!i->second(get_environment_variable(i->first))) {
        }
    }
    return true;
}