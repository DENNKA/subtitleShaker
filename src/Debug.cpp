#include "Debug.h"

Debug::Debug(){
    #ifdef OS_LINUX
        //русский не нужон!
    #elif OS_WINDOWS
        int ret;
        ret = GetSystemDefaultLangID();
        if (ret == 1049){
            lang = Lang::ru;
        }
    #endif

    std::locale current_locale("");
    std::locale::global(current_locale);
    std::ios::sync_with_stdio(false);
}

void Debug::out(Lang lang, std::wstring msg){
    output(std::wcout, lang, msg);
}

Lang Debug::getLang(){return lang;}

void Debug::error(Lang lang, std::wstring msg){
    output(std::wcerr, lang, msg);
}

void Debug::output(std::wostream& out, Lang lang, std::wstring msg){
    if (this->lang == lang){
        out << msg;
    }
}

void Debug::output(std::wostream& out, std::string msg){
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    out << " \"";
    out << converter.from_bytes(msg);
    out << "\"";
    out << std::endl;
}
