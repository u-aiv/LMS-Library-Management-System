# Library Management System Design Document (English - US)

## 1. Purpose and Scope
This document captures the current system design based on the repository implementation (C++11 console edition), including:
- System goals and boundaries
- Architecture and module responsibilities
- Core data model and persistence strategy
- Key business workflows
- Security/config/report/backup mechanisms
- Known design risks and improvement directions

## 2. System Overview
### 2.1 Product Positioning
The system is a local, console-based library management application for two roles (Admin and Member), supporting:
- Authentication and role-based access
- Book/Member/Transaction/Reservation management
- Recommendation and reporting
- Data backup and restore

### 2.2 Tech Stack
- Language: C++11
- Build: CMake (`CMakeLists.txt`)
- Crypto: OpenSSL (PBKDF2-HMAC-SHA256)
- Storage: CSV files
- Interface: CLI menus

### 2.3 Runtime Shape
- Single-process, local machine, no network services needed
- In-memory state with CSV write-through on operations
- First-run seeding for default users and sample books

## 3. Structure and Layering
```
src/
  authentication/   Auth and password hashing
  config/           Global constants and runtime settings
  models/           Domain entities + CSV serialization
  managers/         Business managers (CRUD + rules + persistence)
  ui/               Console UI and menu orchestration
  utils/            File/date/validation utilities
```

Layer interaction:
- `UI/MenuHandler` calls `managers`
- `managers` depend on `models + utils`
- `authentication` is used by `Member/MemberManager`
- `Config` is shared by startup and managers

## 4. Core Module Design
### 4.1 Config Module (`Config`)
Responsibilities:
- File paths, constants, and UI parameters
- Mutable runtime settings (advanced UI, borrow days, fine settings, default max books)
- Load/save settings from `data/settings.csv`

Highlights:
- Singleton (`Config::getInstance()`)
- Default initialization with optional overrides
- Utility helpers for IDs and genre checks

### 4.2 Authentication Module (`auth`)
Responsibilities:
- Password hashing: random salt + PBKDF2 (100,000 iterations)
- Verification: parse stored salt/hash and recompute

Stored hash format:
- `salt(16 bytes => 32 hex chars) + hash(32 bytes => 64 hex chars)`
- Total stored string length: 96

### 4.3 Domain Models (`models`)
- `Book`: ISBN, title, author, publisher, genre, total/available copies, reserved flag
- `Member`: ID, name, phone, preference genres, registration/expiry date, max-books, admin flag, password hash
- `Transaction`: transaction ID, member ID, ISBN, borrow/due/return date, renew count, fine, returned flag
- `Reservation`: reservation ID, member ID, ISBN, reservation date, active flag

Shared capability:
- `toCSV()/fromCSV()` for persistence mapping

### 4.4 Utilities (`utils`)
- `FileHandler`: CSV read/write, file existence, directory creation, simple cache
- `DateUtils`: date/timestamp conversion, current date/time, day arithmetic
- `Validator`: basic validation helpers

### 4.5 Business Managers (`managers`)
#### BookManager
- Owns in-memory book collection and persistence
- CRUD, field search, borrow/return copy updates
- `BatchOperation` RAII for deferred save

#### MemberManager
- Owns member collection and persistence
- CRUD, field search, admin filtering
- `authenticateUser` for login
- Batch save support

#### TransactionManager
- Owns transaction collection and persistence
- Borrow/return/renew operations, active/overdue queries
- ID format: `T + year + quarter + 5-digit sequence`
- Borrow prechecks: valid member, borrow limit, book availability
- Fine rule: fixed 2.0/day capped at 14.0 in model logic

#### ReservationManager
- Owns reservation collection and persistence
- FIFO queue per ISBN (`map<string, deque<string>>`)
- ID format: `R + year + quarter + 5-digit sequence`
- Syncs book `isReserved` on reserve/cancel
- Exposes queue position/length/next APIs

#### RecommendationManager
Hybrid recommendation flow:
1. Build member vectors from preferences + history
2. Compute cosine similarity for KNN neighbors
3. Score candidate ISBNs (neighbor signal + popularity)
4. Cold-start fallback (content preference + popularity)

Supports TopN and available-only filtering.

#### ReportManager
Generates text reports under `reports/`:
1. Summary
2. Inventory
3. Member
4. Transaction
5. Reservation
6. TopBorrowedBooks

#### BackupManager
- Backs up data files into `data/backup/<backupID>/`
- Maintains `backup_manifest.txt`
- Restore/list/get-latest/validate/auto-clean APIs

### 4.6 UI and Flow Control
- `UI`: rendering + input abstraction for simple/advanced display modes
- `MenuHandler`:
- Login/logout lifecycle
- Member flows: search, borrow/return/renew, reserve, recommendations, profile
- Admin flows: entity management, reports, backup/restore, system settings

## 5. Data Persistence Design
### 5.1 CSV Files
- `data/books.csv`
- `data/members.csv`
- `data/transactions.csv`
- `data/reservations.csv`
- `data/settings.csv`

### 5.2 Read/Write Strategy
- Managers load file data on initialization
- Default mode is immediate save per mutation (`autoSave=true`)
- Batch mode defers save and flushes once

### 5.3 Consistency Behavior
- No true transaction manager across multiple files
- Partial rollback exists in a few flows (for example, failed borrow path)

## 6. Key Business Rules
- Default borrow period: 14 days
- Renewal: +7 days each, total borrow period <= 30 days
- Borrow limit: member-level max
- Overdue fine: 2.0/day, capped at 14.0
- Reservation queue: FIFO per ISBN
- Duplicate active reservation for same member+ISBN is blocked
- Expired members cannot borrow/reserve

## 7. Key Workflows
### 7.1 Startup
1. Configure console and locate working data path
2. Load settings and persist defaults
3. Initialize managers
4. Seed defaults if key datasets are empty
5. Enter login/menu loop

### 7.2 Borrow Book
1. Validate member status and limit
2. Validate book availability
3. Create transaction record
4. Decrement available copies
5. Persist changes

### 7.3 Return Book
1. Locate active transaction
2. Increment available copies
3. Mark transaction returned and compute fine
4. Persist changes

### 7.4 Reserve Book
1. Validate member/book and member expiry
2. Prevent duplicate active reservation
3. Create reservation and enqueue
4. Update book reserved flag
5. Persist changes

### 7.5 Backup/Restore
- Backup: create folder -> copy data files -> append manifest
- Restore: validate backup integrity -> overwrite current data files

## 8. Security and Reliability
### 8.1 Implemented
- No plaintext password storage
- Role-based menu separation

### 8.2 Gaps
- No account lockout/rate limiting
- No file locks or concurrency controls

## 9. Identified Design Risks (Code-Based)
1. `cmake_minimum_required(VERSION 4.1)` is relatively high and may break compatibility.
2. Multi-manager write operations are not atomic.

## 10. Non-Functional Assessment
- Portability: basic Windows/Linux support exists
- Maintainability: good module boundaries
- Extensibility: manager-oriented design is suitable for future DB/service migration
- Performance: acceptable for small datasets; CSV + linear scans limit scale
