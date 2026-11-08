# submission/

本目录只放 PR 模板。

真正的 `my_submission.txt`（含邮箱、电话、指导老师）**不入库**，
放在提交包的 `01-提交材料/my_submission.txt`。

流程：
1. Fork https://github.com/OpenXiangShan/XiangShanLab
2. 把 `my_submission.txt` 放进 `2026-CIE-RISC-V-Contest-Application-Track/`
3. 运行 `source ./submit.sh my_submission.txt` 生成 `submission.asc`
4. 只把 `submission.asc` 推到自己的 Fork，不推明文
5. 向 `OpenXiangShan/XiangShanLab:main` 发 PR
