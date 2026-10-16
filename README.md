# Online Examination Management System

A console-based examination management system written in C. The application
supports administrator, teacher, and student workflows, with records stored
in local text files.

> This is an educational capstone project. It is not intended for production
> use or for storing real user credentials or examination data.

## Features

- **Administrator:** view users and exams, inspect system statistics, and
  search for users by username.
- **Teacher:** register, create exams, add multiple-choice questions, review
  exam questions, and view exam statistics.
- **Student:** register, browse exams not yet attempted, take an exam once,
  view results, and review a profile summary.
- **Local persistence:** users, exams, questions, and results are saved between
  runs in plain-text files.

## Requirements

- A C compiler such as GCC
- A terminal or command prompt

## Build and run

Run these commands from the repository root. The program reads and writes its
data files in the current working directory.

### Windows (GCC / MinGW)

```powershell
gcc -std=c11 -Wall -Wextra -pedantic src/exam_system.c -o exam_system.exe
.\exam_system.exe
```

### Linux or macOS

```sh
gcc -std=c11 -Wall -Wextra -pedantic src/exam_system.c -o exam_system
./exam_system
```

## Getting started

On first run, the program creates an administrator account if no administrator
is already present:

| Username | Password |
| --- | --- |
| `admin` | `admin123` |

Log in as the administrator to explore user, exam, and statistics views.
Teachers and students can create accounts from the registration menu.

**Change or remove this demo credential before sharing any copy of the
application.** Passwords are stored as plain text in `users.txt`; this project
is for learning and demonstration, not real accounts.

## Data files

The program creates these files in the directory from which it is run:

| File | Contents |
| --- | --- |
| `users.txt` | User ID, username, password, and role |
| `exams.txt` | Exam ID, name, subject, teacher ID, and duration |
| `questions.txt` | Question ID, exam ID, prompt, four options, and answer |
| `results.txt` | Result ID, student ID, exam ID, score, and question count |

Records are space-delimited, so text fields cannot contain spaces. Use
underscores in exam names, questions, and answer options instead. These runtime
files are excluded from version control because they can contain personal
records and plain-text passwords.

## Project layout

```text
.
├── README.md
├── .gitignore
└── src/
    └── exam_system.c
```

## Known limitations

- Exam duration is displayed but not enforced.
- Input fields are single words; spaces are not supported.
- Data is stored locally in plain-text files without password hashing or
  concurrent-user support.
- This project is a console-based learning example, not a production exam
  platform.
