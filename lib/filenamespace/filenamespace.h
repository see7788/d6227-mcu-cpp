// #ifndef filenamespace_h
// #define filenamespace_h
// #include <FS.h>
// #include <SPIFFS.h>
// #include <ArduinoJson.h>
// namespace filenamespace
// {
//     // listFilePrint::listFile(fileOs, "/", 0);
//     void listFilePrint(fs::FS &fs, const char *dirname, uint8_t levels)
//     {
//         Serial.printf("Listing directory: %s\r\n", dirname);

//         File root = fs.open(dirname);
//         if (!root)
//         {
//             Serial.println("- failed to open directory");
//             return;
//         }
//         if (!root.isDirectory())
//         {
//             Serial.println(" - not a directory");
//             return;
//         }

//         File file = root.openNextFile();
//         while (file)
//         {
//             if (file.isDirectory())
//             {
//                 Serial.print("  DIR : ");
//                 Serial.println(file.name());
//                 if (levels)
//                 {
//                     listFilePrint(fs, file.path(), levels - 1);
//                 }
//             }
//             else
//             {
//                 Serial.print("  FILE: ");
//                 Serial.print(file.name());
//                 Serial.print("\tSIZE: ");
//                 Serial.println(file.size());
//             }
//             file = root.openNextFile();
//         }
//     }
//     //readFile(SPIFFS, "/hello.txt");
//     void readFile(fs::FS &fs, const char *path)
//     {
//         Serial.printf("Reading file: %s\r\n", path);

//         File file = fs.open(path);
//         if (!file || file.isDirectory())
//         {
//             Serial.println("- failed to open file for reading");
//             return;
//         }

//         Serial.println("- read from file:");
//         while (file.available())
//         {
//             Serial.write(file.read());
//         }
//         file.close();
//     }
//     void writeFile(fs::FS &fs, const char *path, const char *message)
//     {
//         Serial.printf("Writing file: %s\r\n", path);

//         File file = fs.open(path, FILE_WRITE);
//         if (!file)
//         {
//             Serial.println("- failed to open file for writing");
//             return;
//         }
//         if (file.print(message))
//         {
//             Serial.println("- file written");
//         }
//         else
//         {
//             Serial.println("- write failed");
//         }
//         file.close();
//     }

// }
// #endif