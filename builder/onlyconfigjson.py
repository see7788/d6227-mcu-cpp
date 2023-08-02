import shutil
import os
import runpy
Import("env")


if (len(BUILD_TARGETS) == 0 or "upload" in BUILD_TARGETS):
    try:
        mcuPath = env.get("PROJECT_DIR")
        srcFilePath=os.path.abspath(mcuPath+"/../d6227-mcu-ts/src/config.json")
        saveFilePath=os.path.abspath(mcuPath+"/data/config.json")
        print("srcFilePath=%s;saveFilePath=%s" % (srcFilePath, saveFilePath))
        if os.path.exists(saveFilePath):
            os.remove(saveFilePath)
        msg=shutil.copyfile(srcFilePath,saveFilePath)
        print(msg)
        p = "C:\\Users\\13520\\.platformio\\penv\\Scripts\\platformio.exe run --target"
        os.system('"' + p + ' buildfs"')
        os.system('"'+p+' size"')
        os.system('"' + p + ' uploadfs"')
    except Exception as e:
        print(e + 'uploadFs error')
    finally:
        print("uploadfs success")
