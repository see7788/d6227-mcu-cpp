import os
Import("env")


if (len(BUILD_TARGETS) == 0 or "upload" in BUILD_TARGETS):
    try:
        mcuPath = env.get("PROJECT_DIR")
        mcudataPath = os.path.abspath(mcuPath+"/data")
        uiPath = os.path.abspath(mcuPath+"/../d6227-mcu-ts")
        print("当前工作目录 : " + mcuPath)
        os.chdir(uiPath)
        print("当前工作目录 : " + os.getcwd())
        env.Execute("pnpm run build --mode app-spiffs --outDir "+mcudataPath)
    except Exception as e:
        print(e)
    finally:
        os.chdir(mcuPath)
        print("当前工作目录 : " + mcuPath)
    try:
        p = "C:\\Users\\13520\\.platformio\\penv\\Scripts\\platformio.exe run --target"
        os.system('"' + p + ' buildfs"')
        os.system('"'+p+' size"')
        os.system('"' + p + ' uploadfs"')
    except Exception as e:
        print(e + 'uploadFs error')
    finally:
        print("uploadfs success")
