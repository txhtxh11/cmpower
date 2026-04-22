#!/bin/bash

# GitHub配置
GITHUB_USER="txhtxh11"
GITHUB_TOKEN="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"
REPO_NAME="cmpower"
REPO_URL="https://${GITHUB_USER}:${GITHUB_TOKEN}@github.com/${GITHUB_USER}/${REPO_NAME}.git"

# 设置本地Git配置（仅限当前仓库）
git config user.name "t"
git config user.email "t@sp"

# 检查是否已经是Git仓库
if [ ! -d ".git" ]; then
    echo "初始化Git仓库..."
    git init
    
    # 添加所有文件
    git add .
    
    # 提交
    git commit -m "Initial commit"
    
    # 添加远程仓库
    git remote add origin "$REPO_URL"
    
    # 推送并设置上游分支
    git push -u origin main || git push -u origin master
else
    echo "已经是Git仓库，直接推送..."
    git add .
    git commit -m "Update: $(date '+%Y-%m-%d %H:%M:%S')"
    git push origin main || git push origin master
fi

echo "推送完成！"