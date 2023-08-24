#ifndef MyFs_h
#define MyFs_h
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <esp_log.h>
class MyFs : public fs::SPIFFSFS
{
private:
    const char *path;

public:
    bool begin_bool;
    bool file_bool;
    MyFs(const char *config)
        : path(config)
    {
        this->begin_bool = this->begin(true);
        if (!this->begin_bool)
        {
            ESP_LOGV("ERROR", "%s", "!this->begin_bool");
        }
        File file = this->open(path);
        this->file_bool = file && !file.isDirectory();
        if (!this->file_bool)
        {
            ESP_LOGV("ERROR", "%s", "!this->file_bool");
        }
    }
    void listFilePrint(const char *dirname, uint8_t levels)
    {
        File root = this->open(dirname);
        if (!root)
        {
            Serial.println("- failed to open directory");
            return;
        }
        if (!root.isDirectory())
        {
            Serial.println(" - not a directory");
            return;
        }

        File file = root.openNextFile();
        while (file)
        {
            if (file.isDirectory())
            {
                Serial.print("  DIR : ");
                Serial.println(file.name());
                if (levels)
                {
                    listFilePrint(file.path(), levels - 1);
                }
            }
            else
            {
                Serial.print("  FILE: ");
                Serial.print(file.name());
                Serial.print("\tSIZE: ");
                Serial.println(file.size());
            }
            file = root.openNextFile();
        }
    }
    DeserializationError readFile(JsonDocument &doc)
    {
        File dataFile = this->open(this->path);
        DeserializationError error = deserializeJson(doc, dataFile);
        dataFile.close();
        return error;
    }

    // 返回写入到文件的字符串长度，0表示失败没写入
    int writeFile(JsonObject &obj)
    {
        File dataFile = this->open(path, "w");
        return serializeJson(obj, dataFile);
    }
};
#endif