<#
.SYNOPSIS
    Robocopy 定时覆盖备份脚本
.DESCRIPTION
    使用 Robocopy 实现源目录到目标目录的完全覆盖备份
    自动创建日志文件并记录备份结果
#>

param(
    [string]$SourcePath = "C:\要备份的文件夹",
    [string]$BackupPath = "D:\备份文件夹",
    [string]$LogPath = "D:\备份日志"
)

# 创建日志目录（如果不存在）
if (-not (Test-Path -Path $LogPath)) {
    New-Item -ItemType Directory -Path $LogPath | Out-Null
}

# 生成带日期的日志文件名
$LogFile = Join-Path -Path $LogPath -ChildPath ("BackupLog_" + (Get-Date -Format "yyyyMMdd_HHmmss") + ".log")

# Robocopy 参数说明：
# /MIR - 镜像模式（完全同步，删除目标中多余的文件）
# /NP  - 不显示进度百分比
# /NDL - 不记录目录名
# /NJH - 不显示作业头
# /NJS - 不显示作业摘要
# /R:3 - 失败重试3次
# /W:5 - 重试等待5秒
# /XD "System Volume Information" - 排除系统目录
# /COPY:DAT - 复制数据、属性和时间戳
# /DCOPY:T  - 复制目录时间戳
# /LOG+:$LogFile - 追加日志到文件
# /TEE       - 同时在控制台显示输出

# 执行备份
robocopy $SourcePath $BackupPath /MIR /NP /NDL /NJH /NJS /R:3 /W:5 /XD "System Volume Information" /COPY:DAT /DCOPY:T /LOG+:$LogFile /TEE

# 检查返回码
switch ($LASTEXITCODE) {
    { $_ -le 7 } { 
        Write-Host "备份成功完成" -ForegroundColor Green
        Add-Content -Path $LogFile -Value "`n[$(Get-Date)] 备份成功完成"
    }
    default {
        Write-Host "备份过程中出现错误 (错误代码: $LASTEXITCODE)" -ForegroundColor Red
        Add-Content -Path $LogFile -Value "`n[$(Get-Date)] 备份失败 (错误代码: $LASTEXITCODE)"
    }
}

# 保留最近30天的日志
Get-ChildItem -Path $LogPath -Filter "BackupLog_*.log" | 
    Where-Object { $_.LastWriteTime -lt (Get-Date).AddDays(-30) } | 
    Remove-Item -Force