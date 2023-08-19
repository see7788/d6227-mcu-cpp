import subprocess

# 指定 TypeScript 文件路径
ts_file = "example.ts"

# 定义输出文件路径
output_file = "output.txt"

# 使用 ts-node 运行 TypeScript 文件并将变量值写入到输出文件
subprocess.check_output(["npx", "ts-node", "-e", f"require('fs').writeFileSync('{output_file}', require('{ts_file}').myVariable)"])