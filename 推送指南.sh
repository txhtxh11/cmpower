#!/bin/bash
# 使用方法：运行此脚本，然后按照提示输入GitHub用户名和令牌

echo "=== ESPHome HomeKit项目推送脚本 ==="
echo ""
echo "这个脚本会帮你将项目推送到GitHub仓库。"
echo "你需要准备好以下信息："
echo "1. GitHub用户名：txhtxh11"
echo "2. GitHub个人访问令牌"
echo ""
echo "按Enter键开始..."
read

cd /home/t/桌面/esphome-homekit-6b

echo "检查Git状态..."
git status

echo ""
echo "设置Git用户信息..."
git config user.name "t"
git config user.email "t@sp"

echo ""
echo "添加所有文件..."
git add .

echo ""
echo "提交更改..."
git commit -m "Update: $(date '+%Y-%m-%d %H:%M:%S')"

echo ""
echo "设置远程仓库..."
git remote remove origin 2>/dev/null
git remote add origin https://github.com/txhtxh11/cmpower.git

echo ""
echo "正在推送到GitHub..."
echo "当系统提示时，请输入："
echo "用户名：txhtxh11"
echo "密码：你的GitHub个人访问令牌"
echo ""
echo "注意：如果看到'推送保护'警告，请访问以下链接允许推送："
echo "https://github.com/txhtxh11/cmpower/security/secret-scanning/unblock-secret/3CihWrPhYfx1GaAt9qQzi8C2QgG"
echo ""
git push -u origin master

echo ""
echo "=== 完成 ==="
echo "如果推送成功，你的项目现在应该在："
echo "https://github.com/txhtxh11/cmpower"
echo ""
echo "如果遇到问题，请检查："
echo "1. 令牌是否正确且未过期"
echo "2. 网络连接是否正常"
echo "3. 是否访问了上面的链接允许推送"