# Library Management System（图书馆管理系统）

## 项目简介
本项目是一个基于 **C++11** 的控制台图书馆管理系统，覆盖图书、会员、借阅交易、预约队列、推荐、报表和数据备份/恢复等核心业务。

系统采用 CSV 文件持久化数据，支持跨平台（Windows / Linux）构建，使用 OpenSSL 实现密码哈希认证。

## 核心功能
- 账号认证与角色分级（管理员 / 普通会员）
- 图书管理：新增、修改、删除、检索、可借状态管理
- 会员管理：新增、修改、删除、检索、管理员账号识别
- 借阅管理：借书、还书、续借、历史记录、逾期识别、罚金计算
- 预约管理：预约创建、取消、查询、FIFO 队列位置
- 推荐系统：基于 KNN + 余弦相似度，结合偏好与历史行为推荐
- 报表系统：汇总、库存、会员、交易、预约、热门图书报表
- 备份系统：数据快照、清单管理、恢复、旧备份清理
- UI 模式：简单 / 高级显示模式切换

## 技术栈
- 语言标准：C++11
- 构建系统：CMake
- 加密库：OpenSSL（PKCS5_PBKDF2_HMAC + SHA-256）
- 存储：CSV 文件

## 项目结构
```text
.
├─CMakeLists.txt
├─data/
│  ├─books.csv
│  ├─members.csv
│  ├─transactions.csv
│  ├─reservations.csv
│  ├─settings.csv
│  └─backup/
├─reports/
└─src/
   ├─main.cpp
   ├─authentication/
   ├─config/
   ├─models/
   ├─managers/
   ├─ui/
   └─utils/
```

## 模块说明
- `models/`：领域对象（Book、Member、Transaction、Reservation）及 CSV 序列化
- `managers/`：业务管理器（CRUD、业务规则、持久化）
- `authentication/`：密码哈希与校验
- `config/`：系统常量与运行时设置
- `ui/`：控制台 UI 与菜单流程
- `utils/`：日期、文件与校验工具

## 构建与运行
### 依赖
- C++11 编译器（GCC/Clang/MSVC）
- CMake
- OpenSSL 开发库

### 构建示例
```bash
cmake -S . -B build
cmake --build build
```

### 运行
```bash
./build/Library_Management_System
```

在 Windows 上，可能为：
```powershell
.\build\Debug\Library_Management_System.exe
```

## 首次启动行为
当 `data/*.csv` 为空时，系统在启动流程中会自动创建默认管理员与默认会员，以及一批示例图书。

默认账号：
- 管理员：`A20261001` / `admin123`
- 普通会员：`M20261001` / `user123`

## 数据文件说明
- `books.csv`：`ISBN,Title,Author,Publisher,Genre,TotalCopies,AvailableCopies,IsReserved`
- `members.csv`：`MemberID,Name,PhoneNumber,Preference,RegistrationDate,ExpiryDate,MaxBooksAllowed,IsAdmin,PasswordHash`
- `transactions.csv`：`TransactionID,MemberID,ISBN,BorrowDate,DueDate,ReturnDate,RenewCount,Fine,IsReturned`
- `reservations.csv`：`ReservationID,MemberID,ISBN,ReservationDate,IsActive`
- `settings.csv`：系统运行参数（UI 模式、借期、罚金等）

## 业务规则（代码层）
- 借阅默认期限 14 天
- 续借每次 +7 天，总借期上限 30 天
- 逾期罚金：`2.0 / 天`，封顶 `14.0`
- 预约队列按日期 FIFO
- 交易 ID / 预约 ID 按“年份+季度+序号”生成

## 报表输出
报表由 `ReportManager` 生成，默认输出到 `reports/`，包括：
- SummaryReport
- InventoryReport
- MemberReport
- TransactionReport
- ReservationReport
- TopBorrowedBooksReport

## 备份与恢复
`BackupManager` 默认使用 `data/backup/` 存储备份目录及清单文件。
可执行：
- 创建备份
- 按备份 ID 恢复
- 列出备份
- 自动清理旧备份（保留最近 N 份）

## 已知注意事项
- `CMakeLists.txt` 中 `cmake_minimum_required(VERSION 4.1)` 版本要求较高，请按实际环境调整。
- 建议使用 UTF-8 格式。
- `data/*.csv` 当前为空是正常状态，首次成功运行后会自动填充基础数据。
