#ifndef MyFs_h
#define MyFs_h
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <esp_log.h>
class MyFs : public fs::LittleFSFS
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
        ESP_LOGV("DEBUG", "this->begin_bool=%s", this->begin_bool ? "true" : "false");
        this->file_bool = this->exists(path);
        ESP_LOGV("DEBUG", "this->file_bool=%s", this->file_bool ? "true" : "false");
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

    void readFile(JsonDocument &doc)
    {
        File dataFile = this->open(this->path);
        DeserializationError error = deserializeJson(doc, dataFile);
        dataFile.close();
        if (error)
        {
            ESP_LOGE("ERROR", "%s", error.c_str());
        }
    }
    void readFile(JsonObject &obj)
    {
        File dataFile = this->open(this->path);
        DynamicJsonDocument doc(2000);
        DeserializationError error = deserializeJson(doc, dataFile);
        dataFile.close();
        if (error)
        {
            ESP_LOGE("ERROR", "%s", error.c_str());
        }
        else
        {
            JsonObject obj2 = doc.as<JsonObject>();
            obj.set(obj2);
            // String str;
            // serializeJson(obj, str);
            // ESP_LOGV("DEBUG", "%s", str);
            // str="";
            // serializeJson(obj2, str);
            // ESP_LOGV("DEBUG", "%s", str);
            // serializeJson(obj, Serial);
        }
    }
    int writeFile(JsonObject &obj)
    {
        File dataFile = this->open(path, "w");
        return serializeJson(obj, dataFile);
    }
};
#endif