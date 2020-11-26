#include "SubtitleShaker.h"

SubtitleShaker::SubtitleShaker(){
    srand(time(NULL));
}

namespace fs = std::experimental::filesystem;

static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

void SubtitleShaker::tryFindFiles(){
    std::wstring directory_name = fs::current_path().wstring();
    std::wstring extensionAss = L".ass";
    std::wstring extensionTxt = L".txt";
    std::vector<std::wstring> fileNames;
    std::vector<std::wstring> settingsFileNames;
    try // Может быть исключение, например, если папки не существует
    {
        for (auto &p : fs::directory_iterator(directory_name)) // Для всех файлов в папке
        {
            if (!fs::is_regular_file(p.status()))
                continue; // Пропускаем, если это не простой файл, а папка или что-то другое

            std::wstring name(p.path().filename().wstring());

            // Проверяем, что имя заканчивается нужным расширением
            // В С++20 можно будет просто `bool match = name.ends_with(extension);`
            bool matchTxt = !name.compare(name.size() - extensionAss.size(), extensionAss.size(), extensionTxt);
            if (matchTxt)
                settingsFileNames.push_back(name);

            bool match = !name.compare(name.size() - extensionAss.size(), extensionAss.size(), extensionAss);
            if (!match)
                continue;

            // if starts MOD_ skip
            if (name.compare(0, fileOutPrefix.size(), fileOutPrefix) == 0)
                continue;

            fileNames.push_back(name);
        }
    }
    catch (std::exception &e)
    {
        //std::cout << "Error: " << e.what() << '\n';
    }

    if (fileIn.empty()){
        if (fileNames.size() == 0){
            debug.error(Lang::en, L"Can't find .ass files\n");
            debug.error(Lang::ru, L"Файлы .ass не найдены\n");
            throw 1;
        }
        debug.out(Lang::en, L"Write number subtitles file:\n");
        debug.out(Lang::ru, L"Напиши номер файла субтитров:\n");
        for (uint i = 0; i < fileNames.size(); i++){
            std::wcout << std::to_wstring(i + 1) << L" " << fileNames[i] << std::endl;
        }
        int assNumber;
        std::cin >> assNumber;
        if (assNumber - 1 < (int)fileNames.size() && assNumber >= 1){
            fileIn = fileNames[assNumber - 1];
            readFile(fileIn, file);
            loadSubtitleFileInfo();
        }
        else{
            debug.error(Lang::en, L"No input file\n");
            debug.error(Lang::ru, L"Нет файла на вход\n");
            throw 1;
        }
    }

    /*if (settingsFile.empty()){
        for (int i = 0; i < settingsFileNames.size()-1; i++){
            bool find = false;
            for (int j = 0; j < settingsFileNames[i].size() - extensionTxt.size(); i++){
                if (j >= fileIn.size() - extensionAss.size()){
                    find = false;
                    break;
                }
                if (fileIn.empty() || fileIn[j] != settingsFileNames[i][j]){
                    find = false;
                    break;
                }
                find = true;
            }
            for (int j = 0; j < fileIn.size() - extensionAss.size(); i++){
                if (fileIn.empty() || j >= settingsFileNames[i].size() - extensionTxt.size() || fileIn[j] != settingsFileNames[i][j]){
                    find = false;
                    break;
                }
                find = true;
            }*/
            /*if (find){
                debug.out(Lang::en, "Auto settings file:");
                debug.out(Lang::ru, "Автоматически выбранный файл настроек:");
                settingsFile = settingsFileNames[i];
                debug.out(settingsFile);
                loadSettings(settingsFile);
                break;
            }
        }
    }*/
    if (settingsFile.empty()){
        for (auto& it : settingsFileNames){
             if (it == L"settings.txt"){
                 debug.out(Lang::en, L"Auto settings file:");
                 debug.out(Lang::ru, L"Автоматически выбранный файл настроек:");
                 settingsFile = it;
                 debug.out(settingsFile);
                 loadSettings(settingsFile);
             }
        }
    }
    if (settingsFile.empty() && settingsFileNames.size() != 0){
        debug.out(Lang::en, L"Write number settings file:\n0 if don't use settings file\n");
        debug.out(Lang::ru, L"Напиши номер файла настроек:\n0 если не использовать файл настроек\n");
        for (uint i = 0; i < settingsFileNames.size(); i++){
            std::wcout << std::to_wstring(i + 1) << ' ' << settingsFileNames[i] << std::endl;
        }
        int txtNumber;
        std::cin >> txtNumber;
        if (txtNumber != 0 && txtNumber - 1 < (int)settingsFileNames.size() && txtNumber >= 1){
            settingsFile = settingsFileNames[txtNumber - 1];
            loadSettings(settingsFile);
        }
    }

    if (fileOut.empty()){
        auto it = fileIn.find(L"/");
        if (it != std::string::npos){
            fileOut = fileIn;
            fileOut.insert(it + 1, fileOutPrefix);
        }
        else{
            fileOut = fileOutPrefix + fileIn;
        }
        debug.out(Lang::en, L"Auto output file:");
        debug.out(Lang::ru, L"Автоматически выбранный файл для вывода:");
        debug.error(fileOut);
    }
}

void SubtitleShaker::help(){
    #define out(x) std::wcout<<L##x<<std::endl;
    #define outOption(shortOption, option, description) std::wcout<<std::setw(4)<<std::right<<L##shortOption<<std::setw(16)<<L##option<<L"   "<<L##description<<std::endl;
    #define outMode(mode, description) std::wcout<<L"        "<<std::setw(16)<<std::left<<L##mode<<L" "<<L##description<<std::endl;
    #define outModeO(shortOption, option, description) std::wcout<<L"           "<<std::setw(4)<<L##shortOption<<std::setw(16)<<L##option<<L"   "<<L##description<<std::endl;
    #define next std::wcout<<std::endl;
    Lang lang = debug.getLang();
    if (lang == Lang::en){
        out("==============================================================")
        out("SubtitleShaker [options]")
        outOption("-h", "--help", "Help")
        outOption("-i", "--input", "Input file")
        outOption("-o", "--output", "Output file")
        outOption("-sf", "--settings_file", "Settings file")
        outOption("-v", "--verbose", "More verbose output")
        outOption("-s", "--seed", "Seed for random")
        out("==============================================================")
        out("Strings process options:")
        outOption("-s", "--shake", "Shake every n ms")
        outOption("-i", "--intensity", "Intensity in pixels")
        outOption("-ix", "--intensity_x", "Intensity in pixels at x")
        outOption("-iy", "--intensity_y", "Intensity in pixels at y")
        outOption("-t", "--time", "Time between two phrases (need to division)")
        outOption("-m", "--mode", "Select mode")
        out("   Modes:")
        outMode("add_str", "Add string in begin or end phrase")
        outModeO("-b", "--begin", "Add in begin")
        outModeO("-e", "--end", "Add in end")
        next
        outMode("translate", "Translate dialogues")
        outModeO("-f", "--from", "Source language")
        outModeO("-to", "--to", "End language WORK ONLY ru")
        next
        outOption("-q", "--quiet", "Deleted")
        out("==============================================================")
    }
    if (lang == Lang::ru){
        out("==============================================================")
        out("SubtitleShaker [опции]")
        outOption("-h", "--help", "Помощь")
        outOption("-i", "--input", "Исходный файл")
        outOption("-o", "--output", "Файл для вывода")
        outOption("-sf", "--settings_file", "Файл настроек")
        outOption("-v", "--verbose", "Более подробный вывод")
        outOption("-s", "--seed", "Семя для генерации случайных чисел")
        out("==============================================================")
        out("Опции обработки строк:")
        outOption("-s", "--shake", "Трясти каждые n миллисекунд")
        outOption("-i", "--intensity", "Интенсивность в пикселях")
        outOption("-ix", "--intensity_x", "Интенсивность в пикселях по x")
        outOption("-iy", "--intensity_y", "Интенсивность в пикселях по y")
        outOption("-m", "--mode", "Выбор режима")
        out("   Режимы:")
        outMode("add_str", "Добавить строку в начало или конец фразы")
        outModeO("-b", "--begin", "В начало")
        outModeO("-e", "--end", "В конец")
        next
        outMode("translate", "Перевод диалогов")
        outModeO("-f", "--from", "С языка")
        outModeO("-to", "--to", "На язык РАБОТАЕТ ТОЛЬКО ru")
        next
        outOption("-q", "--quiet", "Удален")
        out("==============================================================")
    }
    #undef outMode
    #undef out
    #undef outOption
}

void SubtitleShaker::parseSettings(int argc, char *argv[]){
    std::vector<std::string> temp1(argv, argv + argc);
    std::vector<std::wstring> temp;
    temp.reserve(argc);
    for (auto &it : temp1){
        temp.push_back(std::wstring(it.begin(), it.end()));
    }
    parseArg(temp);
}

void SubtitleShaker::parseSettings(std::vector<std::wstring> argv){
    parseArg(argv);
}

void SubtitleShaker::readFile(const std::wstring& fileName, std::vector<std::wstring>& file){
    file.resize(0);
    file.reserve(100);
    std::wifstream fin;
    fin.open(std::string(fileName.begin(), fileName.end()));
    if(!fin.is_open()){
        debug.error(fileName);
        debug.error(Lang::en, L"file not found\n");
        debug.error(Lang::ru, L"файл не найден\n");
        throw 1;
    }
    std::wstring str;
    while(getline(fin, str)){
        file.push_back(str);
    }
}

void SubtitleShaker::writeFile(const std::wstring& fileName, const std::vector<std::wstring>& file){
    std::wofstream fout;
    fout.open(std::string(fileName.begin(), fileName.end()));
    if(fileName.empty()){
        debug.error(Lang::en, L"Not setted output file\n");
        debug.error(Lang::ru, L"Не назначен файл вывода\n");
        throw 1;
    }
    if(!fout.is_open()){
        debug.error(fileName);
        debug.error(Lang::en, L"file not found\n");
        debug.error(Lang::ru, L"файл не найден\n");
        throw 1;
    }
    for (auto& it : file){
        fout<<it<<"\n";
    }
}

void SubtitleShaker::loadSubtitleFileInfo(){
    if (file.empty()){
        debug.error(Lang::en, L"Input file empty\n");
        debug.error(Lang::ru, L"Входной файл пуст\n");
        throw 1;
    }
    for (uint i = 0; i < file.size(); i++){
        const auto strFull = file[i];
        auto str = file[i];
        /*if (str.compare(0, 9, comparePlayResX) == 0){
            auto elems = split(str, ' ');
            playResX = std::stoi(elems[1]);
        }
        if (str.compare(0, 9, comparePlayResY) == 0){
            auto elems = split(str, ' ');
            playResY = std::stoi(elems[1]);
        }*/
        #define ifCompare(x) if (str.compare(0, x.size(), x) == 0)
        #define strErase(x) str.erase(str.begin(), str.begin() + x.size() + 1)

        ifCompare(textScriptInfo){
            std::vector<std::wstring> temp;
            temp.reserve(30);
            for (uint j = i; j < file.size(); j++){
                if (!(file[j].compare(0, textV4.size(), textV4) == 0)){
                    temp.push_back(file[j]);
                }
                else{
                    assHeader.setHeader(temp, std::wstring(version.begin(), version.end()));
                    i = j - 1;
                    break;
                }
            }
        }
        else
        ifCompare(textV4){
            auto str2 = file[i + 1];
            str2.erase(str2.begin(), str2.begin() + textFormat.size() + 1);
            styles.insert(std::make_pair(operation.split(str2, L',')[0], Style(strFull + L'\n' + textFormat + L' ', str2)));
            stylesVector.push_back(Style(strFull + L'\n' + textFormat + L' ', str2));
            i++;
        }
        else
        ifCompare(textStyle){
            strErase(textStyle);
            styles.insert(std::make_pair(operation.split(str, L',')[0], Style(textStyle + L' ', str)));
            stylesVector.push_back(Style(textStyle + L' ', str));
        }
        else
        ifCompare(textEvents){
            auto str2 = file[i + 1];
            str2.erase(str2.begin(), str2.begin() + textFormat.size() + 1);
            dialogues.push_back(Dialog(strFull + L'\n' + textFormat + L' ', str2));
            i++;
        }
        else
        ifCompare(textDialogue){
            strErase(textDialogue);
            dialogues.push_back(Dialog(textDialogue + L' ',str));
        }
        else
        ifCompare(textComment){
            strErase(textComment);
            dialogues.push_back(Dialog(textComment + L' ',str));
        }
    }
}

void SubtitleShaker::loadSettings(std::wstring fileName){
    std::vector<std::wstring> file;
    std::wstring temp;
    readFile(fileName, file);
    temp.reserve(file.size() * 7);
    for (auto& it : file){
        temp += it + L"\n ";
    }
    parseArg(operation.split(temp, L' '));
}

void SubtitleShaker::startProccesing(){
    std::vector<std::wstring> out;
    out.reserve(400);
    out = assHeader.getHeader();
    for (auto& it : stylesVector){
        out.push_back(it.getString());
        //out.push_back(it.second.getString()); // when for it : styles
    }

    if (dialogues.size() == 0){
        debug.error(Lang::en, L"Can't parse dialogs or file not contains they\n");
        debug.error(Lang::ru, L"Невозможно получить диалоги или файл не содержит их\n");
        throw 1;
    }
    if (needToProcess.size() == 0){
        debug.error(Lang::en, L"Not setted dialogs to process\n");
        debug.error(Lang::ru, L"На обработку не выбраны диалоги\n");
        throw 1;
    }
    out.push_back(dialogues[0].getString());
    std::wstring latestDGet;
    try{
        for (uint i = 1; i < dialogues.size(); i++){
            if ((int)i == needToProcess.back()){
                auto& s = settings[i];
                #define dGet(x) (s[x])
                auto dGetInt{[&s, &latestDGet](int x) -> int{
                        latestDGet = dGet(x);
                        return (int)(dGet(x) == L"" ? 0 : std::stoi(dGet(x)));
                    }
                };
                const int time = dGetInt(Time) - dGetInt(Time) % 10;
                const int intensityX = dGetInt(IntensityX) * 2; // !
                const int intensityY = dGetInt(IntensityY) * 2; // !
                auto dialog = dialogues[i];
                #define INT(x) (x == L"" ? 0 : std::stoi(x))
                const auto marginL = INT(dialog.format[Dialog::MarginL]);
                const auto marginR = INT(dialog.format[Dialog::MarginR]);
                const auto marginV = INT(dialog.format[Dialog::MarginV]);
                const auto start = dialog.getStartMs();
                const auto end = dialog.getEndMs();
                if (assHeader.format[ASSHeader::PlayResX].empty() || assHeader.format[ASSHeader::PlayResY].empty()){
                    debug.error(Lang::en, L"Can't find PlayRes please add video file to subtitles\n");
                    debug.error(Lang::ru, L"Невозможно определить PlayRes добавьте видео файл к субтитрам\n");
                    throw 1;
                }
                int x, y;
                const auto& parseTagPos = dialog.parseAssTag(L"\\pos", true);
                if (parseTagPos.first == 0){
                    x = INT(parseTagPos.second[0]);
                    y = INT(parseTagPos.second[1]);
                    dialogues[i] = dialog;
                }
                else{
                    auto pos = styles.at(dialog.format[Dialog::Style]).calculatePosition(
                        std::stoi(assHeader.format[ASSHeader::PlayResX]),
                        std::stoi(assHeader.format[ASSHeader::PlayResY]),
                        marginL || marginR || marginV,
                        marginL, marginR, marginV
                    );
                    x = pos.first;
                    y = pos.second;
                }
                auto newStart = start;
                auto newEnd = start + time;

                int prevX = x, prevY = y;

                bool breakWhile = false;
                while (true){
                    if (newEnd >= end){
                        newEnd = end;
                        breakWhile = true;
                    }
                    for (uint i = FirstMode; i < s.size(); i++)
                    {
                        struct ArgParse{
                            ArgParse(const std::wstring& arg, const std::wstring& argLong) : arg(arg), argLong(argLong){;}
                            std::wstring getParse(){return parse;}
                            int getInt(){if (!parse.empty()) return std::stoi(parse); else return 0;}
                            std::wstring arg, argLong, parse;
                        };
                        std::vector<ArgParse> parseVector;
                        #define addParse(arg,argLong) parseVector.push_back(ArgParse(L##arg, L##argLong));
                        #define parse \
                        for (uint j = i + 1; j < s.size(); j++){\
                            if (dGet(j)[0] != '-'/*IS DIGIT CHECK*/) break;\
                            for (auto& it : parseVector){\
                                if (dGet(j) == it.arg || dGet(j) == it.argLong) {it.parse = dGet(j + 1); j++;}\
                            }\
                        }
                        if (time != 0){
                            dialog.setStartMs(newStart);
                            dialog.setEndMs(newEnd);
                        }
                        if (dGet(i) == L"shake"){
                            //addParse("-f", "--factor")
                            //addParse("-b", "--begin")
                            //addParse("-e", "--end")
                            //parse;
                            int xRand, yRand;
                            do{
                                xRand = intensityX == 0 ? 0 : (rand() % (intensityX + 1)) - intensityX / 2;
                                yRand = intensityY == 0 ? 0 : (rand() % (intensityY + 1)) - intensityY / 2;
                            } while (sqrt((0 - xRand) * (0 - xRand) + (0 - yRand) * (0 - yRand)) > (intensityX + intensityY) / 2); // "circle" random

                            std::vector<int> numbers = {prevX, prevY, x + xRand, y + yRand};
                            dialog.addAssTag(L"move", numbers);
                            prevX = x + xRand;
                            prevY = y + yRand;
                        } else
                        if (dGet(i) == L"translate"){
                            addParse("-f", "--from")
                            addParse("-to", "--to")
                            parse;
                            std::wstring from = parseVector[0].getParse();
                            std::wstring to = parseVector[1].getParse();
                            translator.setLang(from, to);
                            auto get = translator.translate(dialog.format[Dialog::Text]);
                            dialog.format[Dialog::Text] = get;
                        } else
                        if (dGet(i) == L"rand_symbol"){
                             addParse("-a", "--add")
                             addParse("-te", "--time_end")
                             addParse("-b", "--begin")
                             parse;

                             const auto text = parseVector[0].getParse();
                             const auto timeEnd = parseVector[1].getInt();
                             const auto begin = L"{" + parseVector[2].getParse() + L"}";

                             // find position letters without tags
                             std::vector<int> pos;
                             struct AddSettings{
                                 AddSettings(){};
                                 AddSettings(int start, int end, std::wstring text) : start(start), end(end), text(text){;}
                                 int start, end;
                                 std::wstring text;
                             };
                             std::map<int, AddSettings> add;
                             pos.reserve(dialog.format[Dialog::Text].size());
                             int posi = 0;
                             for (auto& it : dialog.format[Dialog::Text]){
                                 if (std::isspace(it) == 0) {pos.push_back(posi);}
                                 posi++;
                             } // if not space add pos

                             dialog.format[Dialog::Text].reserve(dialog.format[Dialog::Text].size() + pos.size() * text.size());
                             int delay = timeEnd / pos.size(); // find delay
                             int tstart = 0;
                             int tend;
                             auto posCopy = pos;
                             while (posCopy.size()){ // setup settings every letter
                                 int random = rand() % posCopy.size();
                                 tend = tstart + delay;
                                 add.insert(std::make_pair(posCopy[random], AddSettings(tstart, tend, text)));
                                 posCopy.erase(posCopy.begin() + random);
                                 tstart = tend;
                             }
                             for (auto it = pos.rbegin(); it != pos.rend(); ++it){ // apply from end
                                 const AddSettings& as = add.at(*it);
                                 dialog.addAssTag(L"t", {as.start, as.end}, as.text, *it);
                                 dialog.insert(*it, begin);
                                 dialog.addAssTag(L"r", {L""}, *it);
                             }
                        } else
                        if (dGet(i) == L"add_str"){
                            addParse("-b", "--begin")
                            addParse("-e", "--end")
                            parse;
                            std::wstring begin = parseVector[0].getParse();
                            std::wstring end = parseVector[1].getParse();
                            dialog.addTextInBegin(begin);
                            dialog.addTextInEnd(end);
                        }
                    }

                    newStart = newEnd;
                    newEnd += time;

                    out.push_back(dialog.getString());
                    dialog = dialogues[i];
                    if (time == 0 || breakWhile) break;
                }
                while (needToProcess.back() == (int)i){
                    needToProcess.pop_back();
                }
                #undef parse
                #undef addParse
                #undef INT
                #undef dGetInt
                #undef dGet
            }
            else{
                out.push_back(dialogues[i].getString());
            }
        }
    }
    /*catch(std::invalid_argument &e){
        debug.exception(e);
        debug.error(Lang::en, L"Check input, maybe invalid argument:");
        debug.error(Lang::ru, L"Проверьте входные данные, возможный неправильный аргумент:");
        debug.error(latestDGet);
        return 1;
    }*/
    catch(std::out_of_range &e){
        debug.exception(e);
        debug.error(Lang::en, L"Out of range, maybe invalid argument:");
        debug.error(Lang::ru, L"Вне диапозона, возможный неправильный аргумент:");
        debug.error(latestDGet);
        throw 1;
    }
    /*catch(std::exception &e){
        std::cerr << "Caught " << e.what() << std::endl;
        std::cerr << "Type " << typeid(e).name() << std::endl;
        return 1;
    }*/
    writeFile(fileOut, out);
}

SubtitleShaker::~SubtitleShaker()
{
    //dtor
}

/*bool sortPairback(const std::pair<int, std::vector<std::wstring>> &a, const std::pair<int, std::vector<std::wstring>> &b)
{
    return (a.first > b.first);
}*/

void SubtitleShaker::parseArg(std::vector<std::wstring> argv){
    if (!fileIn.empty()) inputFileLoaded = true;
    int currentString = -1;
    int currentMode = -1;
    int start, end = 0;
    for (uint i = 0; i < argv.size(); i++){
        auto it = argv[i];
        auto it2 = i + 1 < argv.size() ? argv[i + 1] : L"";
        auto it3 = i + 2 < argv.size() ? argv[i + 2] : L"";

        #define toInt(x) std::stoi(x)
        if (it == L" " || it.empty() || it == L"\n"){
            continue;
        }
        if (it == L"-h" || it == L"--help"){
            help();
            throw 0;
        }

        if (it.back() == '\n') it.erase(it.end() - 1); //don't erase argv[i]
        if (it2.back() == '\n') it2.erase(it2.end() - 1);
        if (it3.back() == '\n') it3.erase(it3.end() - 1);

        if ( it == L"-q" || it == L"--quiet"){
            continue;
        }

        if (it[0] == L'#'){
            for (uint j = i + 1; j < argv.size(); j++){
                if (std::count(argv[j].begin(), argv[j].end(), '\n')){
                    i = j;
                    break;
                }
            }
            continue;
        }
        if (it[0] != L'-' && std::isdigit(it[0])){
            auto numbers = operation.split(it, '-');
            if (numbers.size() == 1){
                int number = toInt(it);
                if (number == 0) continue;
                currentString = number;
                needToProcess.push_back(currentString);
                settings[currentString].resize(settingsToProcess::End);
                //stringToProcess.push_back(make_pair(currentString, settings));
                //stringToProcess.back().second.resize(settingsToProcess::End);
                end = 0;
                continue;
            }
            if (numbers.size() == 2){
                start = toInt(numbers[0]);
                end = toInt(numbers[1]);
                currentString = start;
                for (int i = start; i <= end; i++){
                    needToProcess.push_back(i);
                    settings[i].resize(settingsToProcess::End);
                }
                /*for (int i = start; i <= end; i++){
                    stringToProcess.push_back(make_pair(currentString, settings));
                    stringToProcess.back().second.resize(settingsToProcess::End);
                }*/
                continue;
            }
        }
        else
        if (it == L"ALL"){
            end = dialogues.size() - 1;
            if (end == -1){
                debug.error(Lang::en, L"Macros \"ALL\" don't work, because input file not loaded\n");
                debug.error(Lang::ru, L"Макрос \"ALL\" не работает, потому что входной файл не загруен\n");
            }
            start = 1;
            currentString = start;
            for (int i = start; i <= end; i++){
                needToProcess.push_back(i);
                settings[i].resize(settingsToProcess::End);
            }
            continue;
        }
        else
        {
            if (std::isdigit(it[1])){
                auto s = it;
                s.erase(s.begin());
                auto numbers = operation.split(s, '-');
                int start, end = 0;
                start = std::stoi(numbers[0]);
                if (numbers.size() == 1) end = start;
                if (numbers.size() == 2) end = std::stoi(numbers[1]);
                for (int i = start; i <= end; i++){
                    auto del = std::find_if(needToProcess.cbegin(), needToProcess.cend(), [&i](const auto &p) { return p == i; });
                    if (del != needToProcess.end()){
                        needToProcess.erase(del);
                    }
                }
                continue;
            }
        }

        if (it == L"-sf" || it == L"--settings_file"){
            settingsFile = argv[i + 1];
            if (argv[i + 1] != L"0"){
                loadSettings(settingsFile);
            }
            i++;
        } else
        if (it == L"-o" || it == L"--output"){
            fileOut = argv[i + 1];
            i++;
        } else
        if ((it == L"-i" || it == L"--input") && !inputFileLoaded){
            inputFileLoaded = true;
            fileIn = argv[i + 1];
            readFile(fileIn, file);
            loadSubtitleFileInfo();
            i++;
        } else
        if (it == L"--seed"){
            srand(std::stoll(argv[i + 1]));
            i++;
        } else
        if (it == L"-v" || it == L"--verbose"){
            verbose = true;
        } else
        if (it == L"--version"){
            std::wcout << std::wstring(version.begin(), version.end()) << std::endl;
            throw 0;
        } else
        if (currentString != -1){
            auto& settingsCurrent = SubtitleShaker::settings[currentString];
            if (settingsCurrent.size() < settingsToProcess::End) settingsCurrent.resize(settingsToProcess::End);
            #define settings(x, set)\
            if(end == 0){\
                settings[currentString][x] = set;\
            }\
            else{\
                for (int j = start; j <= end; j++){\
                    auto& s = settings[j];\
                    s.at(x) = set;\
                }\
            }
            #define settingsPushBack(set)\
            if(end == 0){\
                settingsCurrent.push_back(set);\
            }\
            else{\
                for (int j = start; j <= end; j++){\
                    auto& s = settings[j];\
                    s.push_back(set);\
                }\
            }
            if (it == L"-t" || it == L"--time"){
                settings(Time, it2);
            } else
            if ((it == L"-i" || it == L"--intensity") && inputFileLoaded){
                settings(IntensityX, it2)
                settings(IntensityY, it2)
            } else
            if (it == L"-ix" || it == L"--intensity_x"){
                settings(IntensityX, it2)
            } else
            if (it == L"-iy" || it == L"--intensity_y"){
                settings(IntensityY, it2)
            } else
            if (it == L"-s" || it == L"--shake"){
                settings(Time, it2)
                argv.insert(argv.begin() + (i + 2), L"-m");
                argv.insert(argv.begin() + (i + 3), L"shake");
            } else
            if (it == L"-m" || it == L"--mode"){
                if (it2 == L"add_str"){  //support old style
                    if (it3[0] != '-'){
                        argv.insert(argv.begin() + (i + 2), L"-b");
                    }
                }
                if (settingsCurrent[FirstMode] == L""){
                    settings(FirstMode, it2)
                    currentMode = FirstMode;
                }
                else{
                    settingsPushBack(it2);
                    currentMode = settingsCurrent.size() - 1;
                }
            } else
            if (it[0] == L'-'){  //DANGER only in end and only after all else !!!
                if (currentMode != -1)
                {
                    settingsPushBack(it);
                    settingsPushBack(it2);
                }
                else{
                    debug.error(Lang::en, L"Invalid argument");
                    debug.error(Lang::ru, L"Невалиден аргумент");
                    debug.error(it);
                    debug.error(Lang::en, L" or not setted mode\n");
                    debug.error(Lang::ru, L" или не установлен режим\n");
                }
            }
            else i--;

            i++;
        }

        #undef last
        #undef it
        #undef it2
        #undef toInt
    }

    std::sort(needToProcess.begin(), needToProcess.end(), std::greater<>());
}
