#include <iostream>
#include <vector>
#include <optional>
#include <fstream>
#include <iomanip>
#include <windows.h>
#include <cstdlib>
#include <conio.h>
#  include <fcntl.h>
/*

x  x  5  3  x  x  x  x  x
8  x  x  x  x  x  x  2  x
x  7  x  x  1  x  5  x  x
4  x  x  x  x  5  3  x  x
x  1  x  x  7  x  x  x  6
x  x  3  2  x  x  x  8  x
x  6  x  5  x  x  x  x  9
x  x  4  x  x  x  x  3  x
x  x  x  x  x  9  7  x  x

 */

#ifndef MS_STDLIB_BUGS // Allow overriding the autodetection.
/* The Microsoft C and C++ runtime libraries that ship with Visual Studio, as
 * of 2017, have a bug that neither stdio, iostreams or wide iostreams can
 * handle Unicode input or output.  Windows needs some non-standard magic to
 * work around that.  This includes programs compiled with MinGW and Clang
 * for the win32 and win64 targets.
 *
 * NOTE TO USERS OF TDM-GCC: This code is known to break on tdm-gcc 4.9.2. As
 * a workaround, "-D MS_STDLIB_BUGS=0" will at least get it to compile, but
 * Unicode output will still not work.
 */
#  if ( _MSC_VER || __MINGW32__ || __MSVCRT__ )
/* This code is being compiled either on MS Visual C++, or MinGW, or
 * clang++ in compatibility mode for either, or is being linked to the
 * msvcrt (Microsoft Visual C RunTime) library.
 */
#    define MS_STDLIB_BUGS 1
#  else
#    define MS_STDLIB_BUGS 0
#  endif
#endif

#if MS_STDLIB_BUGS
#  include <io.h>
#endif

void init_locale() {
    // Does magic so that wcout can work.
    #if MS_STDLIB_BUGS
    constexpr char cp_utf16le[] = ".1200";
    setlocale(LC_ALL, cp_utf16le);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stdin), _O_WTEXT);
    #endif
}
void fontSettings() {
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(cfi);
    cfi.nFont = 0;
    cfi.dwFontSize.X = 0;                   // Width of each character in the font
    cfi.dwFontSize.Y = 32;                  // Height
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_BOLD;
    std::wcscpy(cfi.FaceName, L"Lucida Console"); // Choose your font
    SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);
}
void sizePosSettings() {
    struct {
        int x, y;
    } resulution = { GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };
    
    RECT r = {
        .left   = (resulution.x - 580)/2,
        .top    = resulution.y/3 - 250,
        .right  = 580, // Width
        .bottom = 850, // Height
    };
    MoveWindow(GetConsoleWindow(), r.left, r.top, r.right, r.bottom, false);
}
void codePageSettings(unsigned code_page_id) {
    SetConsoleOutputCP(code_page_id);
    SetConsoleCP(code_page_id);
}
void consoleSettings() {
    fontSettings();
    sizePosSettings();
    codePageSettings(65001);
    init_locale();
    system("color 0B && cls");
}

class Sudoku {
  public:
    struct UserInput {
        friend std::wistream& operator>> (std::wistream& in, UserInput& u) {
            wchar_t a, b, c;
            in >> a >> b >> c;
            u.i = static_cast<size_t>(a - L'0' - 1);
            u.j = static_cast<size_t>(b - L'0' - 1);
            u.number = c;
            return in;
        }
        
        size_t i, j;
        wchar_t number;
    };
  public:
    Sudoku(): field{9, std::wstring(9, L'•')} { }
    
    void menu() {
        while(true) {
            system("cls");
            printMessage({
                L"1. Загрузить поле",
                L"2. Ввести поле   "
            });
            std::wcout << L"\nВыбор: ";
            int choice;
            std::wcin >> choice;
        
            bool is_continue_choice = false;
            if(std::wcin && 1 <= choice && choice <= 2) {
                switch(choice) {
                    case 1:
                        if(!load() || !checkStartInitialize()) {
                            printMessage({L"Загрузить поле", L"не удалось :("});
                            _getch();
                            is_continue_choice = true;
                        }
                        break;
                        
                    case 2:
                        if(keyboardInput(); !checkStartInitialize()) {
                            printMessage({L"Поле невозможно :("});
                            _getch();
                            is_continue_choice = true;
                        }
                        break;
                        
                    default:
                        break;
                }
            } else {
                is_continue_choice = true;
                std::wcin.clear();
                std::wcin.sync();
            }
            
            if(!is_continue_choice) {
                break;
            }
        }
        
        play();
    }
    bool load() {
        std::wifstream in("field.txt");
        if(!in) {
            return false;
        }
        
        return input(in);
    }
    void save() {
        std::ofstream out("field.txt");
        for(size_t i = 0; i != 9; ++i) {
            for(size_t j = 0; j != 9; ++j) {
                if(isdigit(field[i][j])) {
                    out << field[i][j] - '0';
                } else {
                    out << ".";
                }
            }
            out << "\n";
        }
    }
    
    void keyboardInput() {
        printMessage({
            L"Введите поле 9х9,",
            L"состоящее из цифр и",
            L"символов-пропусков"
        });
        std::wcout << "\n";
        
        input(std::wcin);
        std::wcin.clear();
        std::wcin.sync();
        
        system("cls");
        printField();
    }
    bool input(std::wistream& in) {
        for(size_t i = 0; i != 9; ++i) {
            for(size_t j = 0; j != 9; ++j) {
                if(!in) {
                    return false;
                }
                wchar_t c;
                in >> c;
                if(isdigit(c)){
                    field[i][j] = c;
                }
            }
        }
        return true;
    }
    bool checkStartInitialize() {
        for(size_t i = 0; i != 9; ++i) {
            for(size_t j = 0; j != 9; ++j) {
                wchar_t c = field[i][j];
                field[i][j] = 'x';
                if(isdigit(c) && !isCorrect({i, j, c})) {
                    return false;
                }
                field[i][j] = c;
            }
        }
        return true;
    }
    
    void play() {
        std::optional<UserInput> user_input;
        
        while(!isFieldFilled()) {
            save();
            printField();
            do {
                user_input = getUserNumber();
    
            } while(!user_input || !isCorrect(*user_input));
            
            field[user_input->i][user_input->j] = user_input->number;
        }
    
        save();
        printField();
        printMessage(0[messages]);
        _getch();
        clearInput();
    }
    bool isFieldFilled() {
        for(auto& row: field) {
            for(auto& cell: row) {
                if(!isdigit(cell)) {
                    return false;
                }
            }
        }
        return true;
    }
    
    void printField() {
        setPos(0, 0);
        
        // Первая строка
        std::wcout << L"♪| ";
        for(size_t i = 0; i != 9; ++i) {
            std::wcout << i+1 << " " << ((i+1) % 3 == 0 ? "| " : "");
        }
        std::wcout << "\b|\n";
        
        // Вторая строка
        auto line = [] {
            std::wcout << "-+";
            for(size_t i = 0; i != 9 * 2 + 3; ++i) {
                std::wcout << "-" << ((i + 1) % 7 == 0 ? "+" : "");
            }
            std::wcout << "\n";
        };
        line();
        
        // Поле
        for(size_t i = 0; i != 9; ++i) {
            std::wcout << i+1 << "|";
            for(size_t j = 0; j != 9; ++j) {
                wchar_t cell = (isdigit(field[i][j]) ? field[i][j] : L'•');
                std::wcout << " " << cell << ((j+1) % 3 == 0 ? " |" : "");
            }
            std::wcout << "\n";
            
            if((i+1) % 3 == 0) {
                line();
            }
        }
    }
    
    static std::optional<UserInput> getUserNumber() {
        std::wcout << L"\nФормат хода: a b c\n";
        std::wcout << L"a - Номер ряда\n";
        std::wcout << L"b - Номер точки в ряду\n";
        std::wcout << L"c - Значение точки\n";
        std::wcout << L"a b c: ";
    
        UserInput user_input;
        if(!(std::wcin >> user_input)) {
            std::wcin.clear();
            std::wcin.sync();
            std::wcout << "\nОшибка ввода\n";
            _getch();
            return std::nullopt;
        }
        
        clearInput();
        
        return user_input;
    }
    
    static void clearInput() {
        static std::wstring blank_lines(512, ' ');
        
        setPos(7, 14);
        std::wcout << blank_lines;
        setPos(0, 0);
    
        setPos(7, 14);
    }
    
    bool isCorrect(UserInput user_input) {
        return
            checkRange(user_input) &&
            checkSquare(user_input) &&
            checkHorizontal(user_input) &&
            checkVertical(user_input);
    }
    bool checkRange(UserInput user_input) {
        bool i_in_range = user_input.i <= 8;
        bool j_in_range = user_input.j <= 8;
        bool n_in_range = '1' <= user_input.number && user_input.number <= '9';
        if(i_in_range && j_in_range && n_in_range) {
            return true;
        }
    
        printMessage({
            L"Все числа",
            L"должны быть из",
            L"интервала [1 ... 9]"
        });
        _getch();
        clearInput();
        return false;
    }
    bool checkSquare(UserInput user_input) {
        struct { size_t i, j; } square_start;
        square_start.i = (user_input.i) / 3; // 0 1 2
        square_start.j = (user_input.j) / 3; // 0 1 2
    
        square_start.i *= 3; // 0 3 6
        square_start.j *= 3; // 0 3 6
        
        for(size_t i = 0; i != 3; ++i) {
            for(size_t j = 0; j != 3; ++j) {
                if(field[square_start.i + i][square_start.j + j] == user_input.number) {
                    printMessage({
                        L"Квадратик  (" +
                        std::to_wstring(square_start.i/3+1) + L", " +
                        std::to_wstring(square_start.j/3+1) + L")",
                        L"не получается"
                    });
                    _getch();
                    clearInput();
                    return false;
                }
            }
        }

        return true;
    }
    bool checkHorizontal(UserInput user_input) {
        for(size_t j = 0; j != 9; ++j) {
            if(field[user_input.i][j] == user_input.number) {
                printMessage(
                    {
                        L"Строка " + std::to_wstring(user_input.i+1),
                        L"не получается"
                    }
                );
                _getch();
                clearInput();
                return false;
            }
        }
        return true;
    }
    bool checkVertical(UserInput user_input) {
        for(size_t i = 0; i != 9; ++i) {
            if(field[i][user_input.j] == user_input.number) {
                printMessage(
                    {
                        L"Столбец " + std::to_wstring(user_input.j+1),
                        L"не получается"
                    }
                );
                _getch();
                clearInput();
                return false;
            }
        }
        return true;
    }
    
    void printMessage(std::vector<std::wstring> messeges) {
        char const* m = reinterpret_cast<char const*>(messeges[0].data());
        if(int(m[0]) == 101 && int(m[1]) == 38 && !isFieldFilled()) {
            printMessage(1[messages]);
            return;
        }
        for(std::wstring const& message: messeges) {
            size_t length = message.size();
            std::wcout
                << "\n >>"
                << std::setw(21/2 - length/2) << "" << message
                << std::setw(21/2+1 - length/2 - length%2) << "" << "<<";
        }
    }
    static void setPos(short x, short y) {
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), {x, y});
    }
    
  private:
    std::vector<std::wstring> field;
    std::vector<std::wstring> messages[2] {
        {
            (wchar_t const*) "\x65\x26\x17\x04\x30\x04\x3f\x04\x3e\x04\x3b\x04\x3d\x04\x35\x04\x3d\x04\x3e\x04\x21\x00\x65\x26\x00\x00",
            (wchar_t const*) "\x2e\x00\x2e\x00\x2e\x00\x20\x00\x34\x04\x3e\x04\x20\x00\x20\x00\x3a\x04\x40\x04\x30\x04\x51\x04\x32\x04\x20\x00\x2e\x00\x2e\x00\x2e\x00\x00\x00",
            (wchar_t const*) "\x2e\x00\x20\x00\x2e\x00\x20\x00\x2e\x00\x20\x00\x2e\x00\x2e\x00\x20\x00\x2e\x00\x2e\x00\x20\x00\x2e\x00\x20\x00\x2e\x00\x20\x00\x2e\x00\x00\x00",
            (wchar_t const*) "\x13\x04\x3e\x04\x40\x04\x4f\x04\x47\x04\x38\x04\x3c\x04\x20\x00\x20\x00\x41\x04\x47\x04\x30\x04\x41\x04\x42\x04\x4c\x04\x35\x04\x3c\x04\x00\x00",
            (wchar_t const*) "\x32\x04\x41\x04\x51\x04\x20\x00\x32\x04\x3e\x04\x3a\x04\x40\x04\x43\x04\x33\x04\x2e\x00\x2e\x00\x2e\x00\x00\x00",
            (wchar_t const*) "\x1f\x04\x3e\x04\x42\x04\x3e\x04\x3c\x04\x43\x04\x20\x00\x20\x00\x47\x04\x42\x04\x3e\x04\x00\x00",
            (wchar_t const*) "\x65\x26\x20\x00\x3b\x04\x38\x04\x41\x04\x38\x04\x47\x04\x3a\x04\x30\x04\x20\x00\x65\x26\x00\x00",
            (wchar_t const*) "\x65\x26\x20\x00\x3d\x04\x35\x04\x3d\x04\x30\x04\x33\x04\x3b\x04\x4f\x04\x34\x04\x3d\x04\x30\x04\x4f\x04\x20\x00\x65\x26\x00\x00",
            (wchar_t const*) "\x41\x04\x47\x04\x30\x04\x41\x04\x42\x04\x4c\x04\x35\x04\x20\x00\x3c\x04\x3e\x04\x51\x04\x00\x00",
            (wchar_t const*) "\x35\x04\x34\x04\x38\x04\x3d\x04\x41\x04\x42\x04\x32\x04\x35\x04\x3d\x04\x3d\x04\x3e\x04\x35\x04\x00\x00",
            (wchar_t const*) "\x3d\x04\x35\x04\x3f\x04\x3e\x04\x32\x04\x42\x04\x3e\x04\x40\x04\x38\x04\x3c\x04\x3e\x04\x35\x04\x00\x00",
            (wchar_t const*) "\x38\x04\x20\x00\x42\x04\x51\x04\x3f\x04\x3b\x04\x3e\x04\x35\x04\x2c\x00\x20\x00\x43\x04\x4e\x04\x42\x04\x3d\x04\x3e\x04\x35\x04\x00\x00",
            (wchar_t const*) "\x2e\x00\x2e\x00\x2e\x00\x00\x00",
            (wchar_t const*) "\x18\x04\x20\x00\x42\x04\x30\x04\x3a\x04\x3e\x04\x35\x04\x20\x00\x40\x04\x3e\x04\x34\x04\x3d\x04\x3e\x04\x35\x04\x00\x00",
            (wchar_t const*) "\x65\x26\x65\x26\x65\x26\x20\x00\x41\x04\x3e\x04\x20\x00\x3c\x04\x3d\x04\x3e\x04\x39\x04\x20\x00\x65\x26\x65\x26\x65\x26\x00\x00"
        },
        {
            (wchar_t const*)"\x11\x04\x40\x04\x30\x04\x32\x04\x3e\x04\x2c\x00\x20\x00\x3b\x04\x38\x04\x41\x04\x38\x04\x47\x04\x3a\x04\x30\x04\x21\x00\x00\x00",
            (wchar_t const*)"\x22\x04\x4b\x04\x20\x00\x3f\x04\x3e\x04\x3f\x04\x40\x04\x3e\x04\x31\x04\x3e\x04\x32\x04\x30\x04\x3b\x04\x30\x04\x00\x00",
            (wchar_t const*)"\x32\x04\x37\x04\x3b\x04\x3e\x04\x3c\x04\x30\x04\x42\x04\x4c\x04\x20\x00\x3c\x04\x3e\x04\x51\x04\x00\x00",
            (wchar_t const*)"\x3f\x04\x3e\x04\x41\x04\x3b\x04\x30\x04\x3d\x04\x38\x04\x35\x04\x20\x00\x2d\x00\x00\x00",
            (wchar_t const*)"\x41\x04\x3d\x04\x3e\x04\x32\x04\x30\x04\x21\x00\x00\x00",
            (wchar_t const*)"\x1e\x04\x34\x04\x3d\x04\x30\x04\x3a\x04\x3e\x04\x2e\x00\x2e\x00\x2e\x00\x00\x00",
            (wchar_t const*)"\x1d\x04\x35\x04\x3b\x04\x4c\x04\x37\x04\x4f\x04\x20\x00\x3c\x04\x43\x04\x45\x04\x3b\x04\x35\x04\x32\x04\x30\x04\x42\x04\x4c\x04\x21\x00\x00\x00",
            (wchar_t const*)"\x21\x04\x4b\x04\x33\x04\x40\x04\x30\x04\x39\x04\x20\x00\x47\x04\x35\x04\x41\x04\x42\x04\x3d\x04\x3e\x04\x00\x00",
            (wchar_t const*)"\x38\x04\x20\x00\x43\x04\x32\x04\x38\x04\x34\x04\x38\x04\x48\x04\x4c\x04\x00\x00",
            (wchar_t const*)"\x47\x04\x42\x04\x3e\x04\x20\x00\x4f\x04\x20\x00\x3d\x04\x30\x04\x3f\x04\x38\x04\x41\x04\x30\x04\x3b\x04\x20\x00\x3b\x00\x29\x00\x00\x00",
        }
    };
};


int main() {
    consoleSettings();
    Sudoku().menu();
    return 0;
}
