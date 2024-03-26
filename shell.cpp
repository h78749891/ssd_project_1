#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

// by abin7989

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <queue>
#include <ctime>
#include <cstdio>

#define LBA_START 0
#define LBA_END 99
//#define LOG_MAX_SIZE 100 //TEST용
#define LOG_MAX_SIZE 10240
using namespace std;

class DataSet {
private:
    unordered_map<string, int> findCommand;
    queue<string> CommandQueue;
    string nowFuncName;
    string nowMessage;
    bool exitFlag;
    static DataSet* dataSet;
    DataSet() { 
        exitFlag = false;
    }
    DataSet& operator=(DataSet& ref) {}
    ~DataSet() {}
protected:

    DataSet(DataSet& ref) = delete;
    void operator=(const DataSet&) = delete;

public:
    static DataSet* getIncetance() {
        if (dataSet == nullptr)
            dataSet = new DataSet();
        return dataSet;
    }
    static bool getExitFlag() { return dataSet->exitFlag; }
    static string getNowMessage() { return dataSet->nowMessage; }
    static string getNowFuncName() { return dataSet->nowFuncName; }
    static string getCommandQueue() {
        string temp = dataSet->CommandQueue.front();
        dataSet->CommandQueue.pop();
        return temp;
    }
    static bool getEmptyCommandQueue() { return dataSet->CommandQueue.empty(); }
    static int getSizeCommandQueue() { return dataSet->CommandQueue.size(); }
    static unordered_map<string, int>::iterator getIteratorFindCommand(string key) { return dataSet->findCommand.find(key); }
    static unordered_map<string, int>::iterator getFindCommandEnd() { return dataSet->findCommand.end(); }

    static void setNowMessage(string m_nowMessage) { dataSet->nowMessage = m_nowMessage; }
    static void setNowFuncName(string m_nowFuncName) { dataSet->nowFuncName = m_nowFuncName; }
    static void setFindCommand(string key, int value) { dataSet->findCommand.insert({ key,value }); }
    static void setExitFlag(bool m_exitFlag) { dataSet->exitFlag = m_exitFlag; }
   
    static void setCommandQueue(string key) { dataSet->CommandQueue.push(key); }
};

DataSet* DataSet::dataSet = nullptr;

enum Command
{
    write,
    read,
    Exit,
    help,
    fullwrite,
    fullread,
    erase,
    erase_range,
    testapp1,
    testapp2
};

bool errorPrint();
bool isValid_between_value(int left, int rigth, int num);
bool isValid_write_value(string num);
bool helpCommand();
bool isValid_lba_num(string num);
bool isCheck_stream(int Qsize);
bool isValid_erase_size(string lba_pos, string erase_size);
int getTotalLine(string name);
string getAllText(string fileName);

bool getFile(string filePos);
bool readFile(string ssd_command, string read_file, string buf_pos);
bool writeFile(string ssd_command, string buf_pos, string buf_value);
bool eraseFile(string ssd_command, string buf_pos, string erase_size);

bool command_write(string ssd_command);
bool command_read(string ssd_command, string read_file);
bool command_fullwrite(string ssd_command);
bool command_fullread(string ssd_command, string read_file);
bool command_erase(string ssd_command);
bool command_erase_range(string ssd_command);
void command_init();

bool test_app1(string ssd_command, string read_file);
bool test_app2(string ssd_command, string read_file);
void logger_print(string func_name, string message);
void logger_manager(string logger);

bool solution() {
    if (DataSet::getEmptyCommandQueue()) {
        DataSet::setNowMessage("존재하지 않는 명령어 입력");
        return errorPrint();
    }
    string command = DataSet::getCommandQueue();

    auto it_c = DataSet::getIteratorFindCommand(command);
    if (it_c == DataSet::getFindCommandEnd()) {
        DataSet::setNowMessage("존재하지 않는 명령어 입력");
        return errorPrint();
    }


    //string ssd_command = "";
    string ssd_command = ".\\";
    string read_file = ".\\result.txt";
    //string read_file = "result.txt";
    DataSet::setNowFuncName("");
    DataSet::setNowMessage("");
    if (!isCheck_stream(it_c->second))
        return errorPrint();

    DataSet::setNowMessage("해당 Command 정상 작동");
    switch (it_c->second)
    {
    case Command::write: //write
        DataSet::setNowFuncName("command_write()");
        return command_write(ssd_command);
    case Command::read: //read
        DataSet::setNowFuncName("command_read()");
        return command_read(ssd_command, read_file);
    case Command::help: //help
        DataSet::setNowFuncName("helpCommand()");
        return helpCommand();
    case Command::fullwrite: //fullwrite
        DataSet::setNowFuncName("command_fullwrite()");
        return command_fullwrite(ssd_command);
    case Command::fullread: //fullread
        DataSet::setNowFuncName("command_fullread()");
        return command_fullread(ssd_command, read_file);
    case Command::testapp1: //testapp1
        DataSet::setNowFuncName("test_app1()");
        return test_app1(ssd_command, read_file);
    case Command::testapp2: //testapp2
        DataSet::setNowFuncName("test_app2()");
        return test_app2(ssd_command, read_file);
    case Command::erase:
        DataSet::setNowFuncName("nowFuncName()");
        return command_erase(ssd_command);
    case Command::erase_range:
        DataSet::setNowFuncName("command_erase_range()");
        return command_erase_range(ssd_command);
        break;
    case Command::Exit: //Exit
        DataSet::setNowFuncName("Exit");
        DataSet::setNowMessage("나가기");
        DataSet::setExitFlag(false);
        return false;
    default:
        return errorPrint();
    }
}

int main() {
    command_init();
    while (DataSet::getExitFlag()) {
        string command;
        getline(cin, command);
        istringstream iss(command);
        string token;
        while (getline(iss, token, ' ')) {
            DataSet::setCommandQueue(token);

        }
        solution();
        logger_print(DataSet::getNowFuncName(), DataSet::getNowMessage());
    }

    return 0;

}

void command_init() {
    DataSet::getIncetance();
    DataSet::setFindCommand("write", Command::write);
    DataSet::setFindCommand("read", Command::read);
    DataSet::setFindCommand("exit", Command::Exit);
    DataSet::setFindCommand("help", Command::help);
    DataSet::setFindCommand("fullwrite", Command::fullwrite);
    DataSet::setFindCommand("fullread", Command::fullread);
    DataSet::setFindCommand("testapp1", Command::testapp1);
    DataSet::setFindCommand("testapp2", Command::testapp2);
    DataSet::setFindCommand("erase", Command::erase);
    DataSet::setFindCommand("erase_range", Command::erase_range);
    DataSet::setExitFlag(true);
}
bool readFile(string ssd_command, string read_file, string buf_pos) {
    ssd_command.append("ssd.exe R ")
        .append(buf_pos);
    if (system(ssd_command.c_str()))
    {
        DataSet::setNowMessage("ssd.exe 실행 실패");
        return false;
    }
    if (!getFile(read_file))
    {
        DataSet::setNowMessage("파일에서 값을 읽어오지 못했습니다.");
        return false;
    }
}
bool writeFile(string ssd_command, string buf_pos, string buf_value) {
    ssd_command.append("ssd.exe W ")
        .append(buf_pos)
        .append(" ")
        .append(buf_value).append("\n");
    if (system(ssd_command.c_str()))//ssd.exe 실행
    {
        DataSet::setNowMessage("ssd.exe 실행 실패");
        return false;
    }

    return true;
}
bool eraseFile(string ssd_command, string buf_pos, string erase_size) {
    ssd_command.append("ssd.exe E ")
        .append(buf_pos)
        .append(" ")
        .append(erase_size).append("\n");
    if (system(ssd_command.c_str()))//ssd.exe 실행
    {
        DataSet::setNowMessage("ssd.exe 실행 실패");
        return false;
    }
    return true;
}
bool command_write(string ssd_command) {
    string buf_pos = DataSet::getCommandQueue();
    string buf_value = DataSet::getCommandQueue();
    if (!(isValid_lba_num(buf_pos)))
        return errorPrint();
    if (!isValid_write_value(buf_value))
        return errorPrint();
    if (!writeFile(ssd_command, buf_pos, buf_value))
        return errorPrint();

    return true;
}
bool command_read(string ssd_command, string read_file) {
    string buf_pos = DataSet::getCommandQueue();
    if (!isValid_lba_num(buf_pos))
        return errorPrint();
    if (!readFile(ssd_command, read_file, buf_pos))
        return errorPrint();

    return true;
}
bool command_fullwrite(string ssd_command) {
    string buf_value = DataSet::getCommandQueue();
    string temp_ssd_command = ssd_command;
    if (!isValid_write_value(buf_value))
        return errorPrint();
    for (int i = LBA_START; i <= LBA_END; i++) {
        if (!writeFile(ssd_command, to_string(i), buf_value))
            return errorPrint();
    }
    return true;
}
bool command_fullread(string ssd_command, string read_file) {

    string temp_ssd_command = ssd_command;
    for (int i = LBA_START; i <= LBA_END; i++) {
        if (!readFile(ssd_command, read_file, to_string(i)))
            return errorPrint();

    }
    return true;
}

bool test_app1(string ssd_command, string read_file) {
    string buf_value = "0xAAAABBBB";
    string temp_ssd_command = ssd_command;

    if (!isValid_write_value(buf_value))
        return errorPrint();
    for (int i = LBA_START; i <= LBA_END; i++) {
        if (!writeFile(ssd_command, to_string(i), buf_value))
            return errorPrint();
    }

    for (int i = LBA_START; i <= LBA_END; i++) {
        if (!readFile(ssd_command, read_file, to_string(i)))
            return errorPrint();

    }
    return true;
}
bool test_app2(string ssd_command, string read_file) {
    string buf_value = "0xAAAABBBB";
    string temp_ssd_command = ssd_command;
    if (!isValid_write_value(buf_value))
        return errorPrint();

    for (int k = 0; k < 30; k++)
        for (int i = 0; i <= 5; i++)
            if (!writeFile(ssd_command, to_string(i), buf_value))
                return errorPrint();

    buf_value = "0x12345678";

    for (int i = 0; i <= 5; i++)
        if (!writeFile(ssd_command, to_string(i), buf_value))
            return errorPrint();

    for (int i = 0; i <= 5; i++)
        if (!readFile(ssd_command, read_file, to_string(i)))
            return errorPrint();
    return true;
}
bool command_erase(string ssd_command) {
    string buf_pos = DataSet::getCommandQueue();
    string erase_size = DataSet::getCommandQueue();
    if (!(isValid_lba_num(buf_pos)))
        return errorPrint();
    if (!isValid_erase_size(buf_pos, erase_size))
        return errorPrint();
    if (!eraseFile(ssd_command, buf_pos, erase_size))
        return errorPrint();

    return true;
}
bool command_erase_range(string ssd_command) {
    string erase_start_pos = DataSet::getCommandQueue();
    string erase_end_pos = DataSet::getCommandQueue();
    if (!(isValid_lba_num(erase_start_pos)))
        return errorPrint();
    if (!(isValid_lba_num(erase_end_pos)))
        return errorPrint();
    int startPos = atoi(erase_start_pos.c_str());
    int endPos = atoi(erase_end_pos.c_str());
    if (startPos > endPos)
    {
        DataSet::setNowFuncName("command_erase_range()");
        DataSet::setNowMessage("시작지점이 도착지점보다 큽니다.");
        return errorPrint();
    }
    int erase_size = endPos - startPos + 1; //30
    for (int i = 0; i < erase_size / 10; i++) {
        if (!eraseFile(ssd_command, to_string(startPos), to_string(10)))
            return errorPrint();
        startPos += 10;
    }
    if (erase_size % 10 != 0)
        if (!eraseFile(ssd_command, to_string(startPos), to_string(erase_size % 10)))
            return errorPrint();


    return true;

}

bool isCheck_stream(int Qsize) {
    switch (Qsize)
    {
    case Command::Exit:
    case Command::help:
    case Command::fullread:
    case Command::testapp1:
    case Command::testapp2:
        if (!DataSet::getEmptyCommandQueue())
        {
            DataSet::setNowFuncName("isCheck_stream()");
            DataSet::setNowMessage("입력 인자의 개수가 알맞지 않습니다.");
            return false;
        }
        break;
    case Command::read:
    case Command::fullwrite:
        if (DataSet::getSizeCommandQueue() != 1)
        {
            DataSet::setNowFuncName("isCheck_stream()");
            DataSet::setNowMessage("입력 인자의 개수가 알맞지 않습니다.");
            return false;
        }
        break;
    case Command::write:
    case Command::erase:
    case Command::erase_range:
        if (DataSet::getSizeCommandQueue() != 2)
        {
            DataSet::setNowFuncName("isCheck_stream()");
            DataSet::setNowMessage("입력 인자의 개수가 알맞지 않습니다.");
            return false;
        }
        break;
    default:
        return false;
        break;
    }
    return true;
}

bool errorPrint() {
    cout << "INVALID COMMAND\n";
    return false;
}
bool isValid_between_value(int left, int rigth, int num) {
    if (num < left || num>rigth)
        return false;

    return true;
}
bool isValid_write_value(string num) {
    if (num.size() != 10)
    {
        DataSet::setNowMessage("입력값이 유효하지 않은 값입니다.");
        return false;
    }
    if (num[0] != '0' || num[1] != 'x')
    {
        DataSet::setNowMessage("입력값이 유효하지 않은 값입니다.");
        return false;
    }
    bool flag = true;
    for (int i = 2; i < 10; i++) {
        flag = false;
        if (num[i] >= 'A' && num[i] <= 'F')
            flag = true;
        else if (num[i] >= '0' && num[i] <= '9')
            flag = true;

        if (!flag)
        {
            DataSet::setNowMessage("입력값이 유효하지 않은 값입니다.");
            return false;
        }
    }

    return true;
}
bool isValid_lba_num(string num) {
    if (!isValid_between_value(1, 2, num.size()))
    {
        DataSet::setNowMessage("해당 위치가 LBA상에 존재하지 않습니다.");
        return false;
    }
    for (int i = 0; i < num.size(); i++) {
        if (num[i] < '0' || num[i]>'9')
        {
            DataSet::setNowMessage("해당 위치가 LBA상에 존재하지 않습니다.");
            return false;
        }
    }
    int tempNum = atoi(num.c_str());
    if (!isValid_between_value(LBA_START, LBA_END, tempNum))
    {
        DataSet::setNowMessage("해당 위치가 LBA상에 존재하지 않습니다.");
        return false;
    }

    return true;
}
bool helpCommand() {
    cout << "write     LBA위치에 입력값을 SSD에 작성한다. \n";
    cout << "read      LBA위치에 입력된 값을 SSD에서 가져온다. \n";
    cout << "fullwrite 모든 LBA위치에 입력값을 SSD에 작성한다. \n";
    cout << "fullread  모든 LBA위치에 입련된 값을 SSD에서 가져온다. \n";
    cout << "Exit      프로그램을 종료한다. \n";
    return true;
}
bool getFile(string filePos) {
    string line;
    ifstream file(filePos);
    if (file.is_open()) {
        while (getline(file, line)) {
            cout << line << "\n";
        }
        file.close();
    }
    else {
        DataSet::setNowMessage("파일에서 읽어오기 실패.");
        return false;
    }
    return true;
}
bool isValid_erase_size(string lba_pos, string erase_size) {
    if (!isValid_between_value(1, 2, erase_size.size()))
    {
        DataSet::setNowMessage("삭제 범위가 존재하지 않습니다.");
        return false;
    }
    for (int i = 0; i < erase_size.size(); i++) {
        if (erase_size[i] < '0' || erase_size[i]>'9')
            return false;
    }
    int esnum = atoi(erase_size.c_str());
    int posnum = atoi(lba_pos.c_str());
    if (!isValid_between_value(1, 10, esnum))
    {
        DataSet::setNowMessage("삭제 범위가 존재하지 않습니다.");
        return false;
    }
    if (!isValid_between_value(0, 99, posnum + esnum))
    {
        DataSet::setNowMessage("삭제 범위가 존재하지 않습니다.");
        return false;
    }

    return true;

}

void logger_print(string func_name, string message) {
    time_t timer = time(NULL);
    struct tm t;
    localtime_s(&t, &timer);
    string timeTamp;
    timeTamp.append("[");
    if ((t.tm_year - 100) / 10 == 0) timeTamp.append("0");
    timeTamp.append(to_string(t.tm_year - 100)).append(".");
    if ((t.tm_mon + 1) / 10 == 0) timeTamp.append("0");
    timeTamp.append(to_string(t.tm_mon + 1)).append(".");
    if (t.tm_mday / 10 == 0)timeTamp.append("0");
    timeTamp.append(to_string(t.tm_mday)).append(" ");
    if (t.tm_hour / 10 == 0) timeTamp.append("0");
    timeTamp.append(to_string(t.tm_hour)).append(":");
    if (t.tm_min / 10 == 0) timeTamp.append("0");
    timeTamp.append(to_string(t.tm_min)).append("]").append(func_name);
    for (int i = func_name.size(); i <= 30; i++) timeTamp.append(" ");
    timeTamp.append(": ").append(message).append("\n");
    cout << timeTamp;
    logger_manager(timeTamp);
}

int getTotalLine(string name)
{
    FILE* fp;
    int line = 0;
    char c;
    fp = fopen(name.c_str(), "r");
    while ((c = fgetc(fp)) != EOF) {
        if (c == '\n')
            line++;
    }

    fclose(fp);
    return line;
}

string getAllText(string fileName) {
    ifstream inLogManager(fileName);
    string sLogManager;
    if (inLogManager.is_open()) {
        inLogManager.seekg(0, std::ios::end);
        int size = inLogManager.tellg();
        sLogManager.resize(size);
        inLogManager.seekg(0, std::ios::beg);
        inLogManager.read(&sLogManager[0], size);
        inLogManager.close();
    }
    else {
        DataSet::setNowMessage("파일을 찾을 수 없습니다.");
    }
    return sLogManager;
}

void logger_manager(string logger) {
    ifstream in("latest.log");
    streamoff logSize;
    in.seekg(0, ios_base::end);
    logSize = in.tellg();
    in.close();
    if (logSize <= LOG_MAX_SIZE) {
        ofstream loggerFile("latest.log", std::ios::out | std::ios::app);
        loggerFile << logger;
        loggerFile.close();
    }
    else {
        time_t timer = time(NULL);
        struct tm t;
        localtime_s(&t, &timer);
        string timeTamp = "until_";
        if ((t.tm_year - 100) / 10 == 0) timeTamp.append("0");
        timeTamp.append(to_string(t.tm_year - 100));
        if ((t.tm_mon + 1) / 10 == 0) timeTamp.append("0");
        timeTamp.append(to_string(t.tm_mon + 1));
        if (t.tm_mday / 10 == 0)timeTamp.append("0");
        timeTamp.append(to_string(t.tm_mday)).append("_");
        if (t.tm_hour / 10 == 0) timeTamp.append("0");
        timeTamp.append(to_string(t.tm_hour)).append("h_");
        if (t.tm_min / 10 == 0) timeTamp.append("0");
        timeTamp.append(to_string(t.tm_min)).append("m_");
        if (t.tm_sec / 10 == 0) timeTamp.append("0");
        timeTamp.append(to_string(t.tm_sec)).append("s").append(".log");

        rename("latest.log", timeTamp.c_str());

        if (getTotalLine("logManager.log") == 2) {
            string sLogManager = getAllText("logManager.log");
            string oneLineName = "";
            string twoLineName = "";
            bool flag = false;
            bool flagLog = false;
            for (char el : sLogManager) {

                if (el == '\n') {
                    flag = true;
                }
                else if (flag) {
                    twoLineName += el;
                }
                else {
                    if (el == '.')
                        flagLog = true;
                    if (!flagLog)
                        oneLineName += el;
                }
            }
            string orginName = oneLineName + ".log";
            string newName = oneLineName + ".zip";
            rename(orginName.c_str(), newName.c_str());


            twoLineName += '\n';
            twoLineName += timeTamp;
            ofstream logManagerFile("logManager.log", std::ios::out);
            logManagerFile << twoLineName << "\n";
            logManagerFile.close();
        }
        else {
            ofstream logManagerFile("logManager.log", std::ios::out | std::ios::app);
            logManagerFile << timeTamp << "\n";
            logManagerFile.close();
        }
        ofstream loggerFile("latest.log", std::ios::out);
        loggerFile << logger;
        loggerFile.close();
    }

}

