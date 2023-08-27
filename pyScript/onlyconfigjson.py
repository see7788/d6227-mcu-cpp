import shutil
import os
import subprocess
Import("env")
# print(env.Dump())
if (len(BUILD_TARGETS) == 0 or "upload" in BUILD_TARGETS):
    try:
        # mcuPath = env.get("PROJECT_DIR")
        # srcFilePath=os.path.abspath(mcuPath+"/../d6227-mcu-ts/src/config.json")
        # saveFilePath=os.path.abspath(mcuPath+"/data/config.json")
        # if os.path.exists(saveFilePath):
        #     os.remove(saveFilePath)
        # msg=shutil.copyfile(srcFilePath,saveFilePath)
        result = subprocess.run(['where', 'platformio'], capture_output=True, text=True)
        output_lines = result.stdout.splitlines()
        p= output_lines[0]+" run --target "
        # os.system('"' + p + 'buildfs" && "' + p + 'size" && "' + p + 'uploadfs"')
        os.system('"' + p + 'buildfs"')
        os.system('"'+p+'size"')
        os.system('"' + p + 'uploadfs"') 
    except Exception as e:
        print(e)
    # finally:
    #     print("\n\n\nuploadfs success;srcFilePath=%s;saveFilePath=%s;copy msg=%s;exe path=%s\n\n\n" % (srcFilePath, saveFilePath,msg,p))
