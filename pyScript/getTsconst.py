import subprocess
import os
Import("env")
try:
    result = subprocess.run(['where', 'platformio'], capture_output=True, text=True)
    output_lines = result.stdout.splitlines()
    mcuPath = env.get("PROJECT_DIR")
    srcFilePath=os.path.abspath(mcuPath+"/../d6227-mcu-ts/src/useStore.ts")
    saveFilePath=os.path.abspath(mcuPath+"/data/config.json")
    if os.path.exists(saveFilePath):
        os.remove(saveFilePath)
    print('srcFilePath:',srcFilePath,";\nsaveFilePath:",saveFilePath)
    # 构建命令字符串，使用console.log输出mcuConfig的值
    command = f'npx ts-node -e "process.chdir(\'../d6227-mcu-ts\');console.log(process.cwd(),require.cache)"'
    output = subprocess.check_output(command, shell=True, encoding='utf-8')
    print(output)
    # 使用 ts-node 运行 TypeScript 文件并将变量值写入到输出文件
    # subprocess.check_output(["npx", "ts-node", "-e", f"require('fs').writeFileSync('{saveFilePath}', require('{srcFilePath}').mcuConfig)"])
    # p= output_lines[0]+" run --target "
    # os.system('"' + p + 'buildfs"')
    # os.system('"'+p+'size"')
    # os.system('"' + p + 'uploadfs"') 
except Exception as e:
        print(e)
finally:
        print("\npy success\n")