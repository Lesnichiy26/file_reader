#include <iostream>
#include <string>
#include <fstream>


struct Arguments {
    std::string filename;
    uint64_t lines = -1;
    char delimiter = '\n';
    bool tail = false;
    uint64_t size = 0;

    Arguments(const std::string& filename, uint64_t lines, char delimiter, bool tail, uint64_t size) :
        filename(filename),
        lines(lines),
        delimiter(delimiter),
        tail(tail),
        size(size) {}

    Arguments() {}
};


bool StartWith(const std::string_view& ideal, const std::string_view& comparable) { // the comparable string should start with ideal
    int i = 0;
    for (char a : ideal) {
        if (a != comparable[i]) {
            return false;
        }
        i++;
    }
    return true;
}


void ThrowError(const std::string_view& error) {
    std::cerr << error;
    exit(EXIT_FAILURE);
}


char SpecialChar(const char del) {
    switch (del) {
        case 'n':
            return '\n';
        case 'r':
            return '\r';
        case '0':
            return '\0';
        case 'a':
            return '\a';
        case 't':
            return '\t';
        case 'b':
            return '\b';
        case 'v':
            return '\v';
        case 'f':
            return '\f';
        case '\\':
            return '\\';
        default:
            ThrowError("Wrong delimiter");
    }
}


uint64_t CheckNumber(const std::string_view& str) {
    uint64_t st = 1;
    uint64_t res = 0;

    for (auto it = str.rbegin(); it != str.rend(); it++) {
        if (!isdigit(*it)) {
            ThrowError("Not a number.");
        }
        res += (*it - '0') * st;
        st *= 10;
    }

    return res;
}

void CountLines(Arguments& args) {
    int res = 0;
    char ch;
    std::ifstream file(args.filename);
    if (!file.is_open()) {
        ThrowError("Unknown command.");
    }
    while (file.good()) {
        file.get(ch);
        if (ch == args.delimiter) {
            res++;
        }
    }
    args.size = res + 1;
}


char Delimiter(std::string_view& arg) {
    std::string_view del = arg.substr(arg.find('=') + 1);

    if (del.size() == 1) {
        return del[0];

    } else if (del.size() == 2 && del[0] == '\\') {
        return SpecialChar(del[1]);

    } else {
        ThrowError("Wrong delimiter");
    }
}


uint64_t Lines(std::string_view& arg) {
    std::string_view num = arg.substr(arg.find('=') + 1);
    return CheckNumber(num);
}


void ReadFile(Arguments& args) {
    std::ifstream file(args.filename);
    char sym;

    if (!file.is_open()) {
        ThrowError("Can't open the file.");
    }

    if (args.lines == -1) {
        args.lines = args.size;
    }

    if (!args.tail) {
        int wait_for = 0;
        while (file.get(sym) && wait_for < args.lines) {
            if (sym == args.delimiter) {
                wait_for++;
            }
            std::cout << sym;
        }

    } else {
        uint64_t wait_for = 0;
        uint64_t border = args.size - args.lines;
        while (file.good() && wait_for < border) {
            file.get(sym);
            if (sym == args.delimiter) {
                wait_for++;
            }
        }
        while (file.get(sym)) {
            if (file.good()) {
                std::cout << sym;
            }
        }
    }
}


void Parser(Arguments& args, char *argv[], int argc) {

    const std::string_view expected_lines = "--lines=";
    const std::string_view expected_delimiter = "--delimiter=";

    if (argc < 2 || argc > 7) {
        ThrowError("Expected more than 1 argument and less than 8 arguments.");
    }
    for (int i = 1; i < argc; i++) {

        std::string_view arg;
        arg = argv[i];

        if (arg == "-l") {
            std::string_view num;

            if (i + 1 < argc) {
                num = argv[i + 1];

            } else {
                ThrowError("There isn't any number of lines.");
            }
            i++;
            args.lines = CheckNumber(num);

        } else if (arg == "-d") {
            std::string del;

            if (i + 1 < argc) {
                del = argv[i + 1];

            } else {
                ThrowError("There isn't any delimiter.");
            }

            if (del.size() == 1) {
                args.delimiter = del[0];
                i++;

            } else if (del.size() == 2 && del[0] == '\\') {
                char special_del = SpecialChar(del[1]);
                args.delimiter = special_del;
                i++;

            } else {
                ThrowError("Wrong delimiter");
            }

        } else if (arg == "-t" || arg == "--tail") {
            args.tail = true;

        } else if (StartWith(expected_lines, arg)) {
            args.lines = Lines(arg);

        } else if (StartWith(expected_delimiter, arg)) {
            args.delimiter = Delimiter(arg);

        } else {
            args.filename = arg;
        }
    }

    CountLines(args);
}


int main(int argc, char *argv[]) {

    Arguments args; // these values are default for the program

    Parser(args, argv, argc);

    ReadFile(args);

    return EXIT_SUCCESS;
}
