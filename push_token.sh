#!/bin/bash
echo "=== GitHub 令牌推送脚本 ==="
echo ""

# 切换到脚本所在目录
cd "$(dirname "$0")"

# 检查是否在git仓库中
if ! git rev-parse --git-dir > /dev/null 2>&1; then
    echo "错误：当前目录不是Git仓库"
    exit 1
fi

# 显示当前状态
echo "当前分支: $(git branch --show-current)"
echo "远程仓库: $(git remote get-url origin)"
echo ""

# 检查是否有需要推送的提交
if git status | grep -q "领先"; then
    echo "检测到本地有未推送的提交"
    echo ""
    echo "🚀 开始推送..."
    echo ""
    echo "注意：如果提示输入密码，请输入你的GitHub个人访问令牌"
    echo "（以 ghp_ 开头的令牌）"
    echo ""
    
    # 尝试推送
    if git push; then
        echo ""
        echo "✅ 推送成功！"
    else
        echo ""
        echo "❌ 推送失败"
        echo "请检查："
        echo "1. 令牌是否有效（在 https://github.com/settings/tokens 查看）"
        echo "2. 令牌是否有 repo 权限"
        echo "3. 网络连接是否正常"
    fi
else
    echo "没有需要推送的提交"
fi

echo ""
read -p "按回车键退出..."